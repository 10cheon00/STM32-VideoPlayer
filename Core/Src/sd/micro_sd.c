#include "sd/micro_sd.h"

#include "diskio.h"

typedef struct {
    uint32_t hal_spi_prescaler;
    uint16_t div;
} micro_sd_spi_prescaler_entry_t;

#define SPI_MIN_HZ 100000U
#define SPI_MAX_HZ 200000U
#define TIMEOUT_MS 500U

#define MICRO_SD_BLOCK_SIZE 512U
#define MICRO_SD_DUMMY_BYTE 0xFFU
#define MICRO_SD_DATA_TOKEN 0xFEU
#define MICRO_SD_DATA_ACCEPTED 0x05U
#define MICRO_SD_DATA_RESPONSE_MASK 0x1FU

#define MICRO_SD_CMD0_GO_IDLE_STATE 0U
#define MICRO_SD_CMD8_SEND_IF_COND 8U
#define MICRO_SD_CMD9_SEND_CSD 9U
#define MICRO_SD_CMD16_SET_BLOCKLEN 16U
#define MICRO_SD_CMD17_READ_SINGLE_BLOCK 17U
#define MICRO_SD_CMD24_WRITE_BLOCK 24U
#define MICRO_SD_CMD55_APP_CMD 55U
#define MICRO_SD_CMD58_READ_OCR 58U
#define MICRO_SD_ACMD41_SD_SEND_OP_COND 41U

#define MICRO_SD_CMD0_CRC 0x95U
#define MICRO_SD_CMD8_CRC 0x87U
#define MICRO_SD_DEFAULT_CRC 0x01U
#define MICRO_SD_CMD8_ARGUMENT 0x000001AAUL
#define MICRO_SD_ACMD41_HCS_ARGUMENT 0x40000000UL
#define MICRO_SD_RESPONSE_RETRY_COUNT 0xFFU

#define MICRO_SD_R1_IDLE_STATE 0x01U
#define MICRO_SD_R1_READY_STATE 0x00U
#define MICRO_SD_R1_ILLEGAL_COMMAND 0x04U
#define MICRO_SD_OCR_CCS 0x40U

static micro_sd_handle_t *active_handle = NULL;

static micro_sd_status_t micro_sd_reduce_spi_clock_to_range_of_100khz_to_400khz(
    micro_sd_handle_t *handle);
static micro_sd_status_t micro_sd_enter_spi_mode(micro_sd_handle_t *handle);
static micro_sd_status_t micro_sd_find_card_version(micro_sd_handle_t *handle);
static micro_sd_status_t
micro_sd_restore_spi_clock(micro_sd_handle_t *handle,
                           uint32_t original_BaudRatePrescaler);
static micro_sd_status_t micro_sd_read_csd(micro_sd_handle_t *handle,
                                           BYTE *csd);
static micro_sd_status_t micro_sd_parse_csd(micro_sd_handle_t *handle,
                                            const BYTE *csd);
static micro_sd_status_t micro_sd_read_single_block(micro_sd_handle_t *handle,
                                                    BYTE *buffer, DWORD sector);
static micro_sd_status_t micro_sd_write_single_block(micro_sd_handle_t *handle,
                                                     const BYTE *buffer,
                                                     DWORD sector);
static micro_sd_status_t
micro_sd_send_app_command(micro_sd_handle_t *handle, uint8_t command,
                          uint32_t argument, uint8_t crc, uint8_t *response);
static micro_sd_status_t
micro_sd_send_command_packet(micro_sd_handle_t *handle, uint8_t command,
                             uint32_t argument, uint8_t crc, uint8_t *response);
static micro_sd_status_t
micro_sd_send_command_packet_and_read_bytes(micro_sd_handle_t *handle,
                                            uint8_t command, uint32_t argument,
                                            uint8_t crc, uint8_t *response, BYTE *data, uint8_t length);
static micro_sd_status_t micro_sd_wait_ready(micro_sd_handle_t *handle);
static micro_sd_status_t micro_sd_wait_data_token(micro_sd_handle_t *handle);
static micro_sd_status_t micro_sd_read_bytes(micro_sd_handle_t *handle,
                                             BYTE *buffer, UINT length);
static micro_sd_status_t micro_sd_write_bytes(micro_sd_handle_t *handle,
                                              const BYTE *buffer, UINT length);
static micro_sd_status_t micro_sd_read_byte(micro_sd_handle_t *handle,
                                            BYTE *value);
static micro_sd_status_t micro_sd_write_byte(micro_sd_handle_t *handle,
                                             BYTE value);
static DWORD micro_sd_get_sector_address(micro_sd_handle_t *handle,
                                         DWORD sector);
static void micro_sd_select_card(micro_sd_handle_t *handle);
static void micro_sd_deselect_card(micro_sd_handle_t *handle);
static micro_sd_handle_status_t
micro_sd_get_handle_status(micro_sd_handle_t *handle);

micro_sd_status_t micro_sd_init_handle(micro_sd_handle_t *handle,
                                       SPI_HandleTypeDef *hspi,
                                       GPIO_TypeDef *GPIO_Port_CS,
                                       uint16_t GPIO_Pin_CS,
                                       micro_sd_spi_bus_clock_t spi_bus_clock) {
    micro_sd_status_t status = MICRO_SD_STATUS_HANDLE_NOT_VALID;

    if (handle != NULL) {
        handle->hspi = hspi;
        handle->GPIO_Port_CS = GPIO_Port_CS;
        handle->GPIO_Pin_CS = GPIO_Pin_CS;
        handle->spi_bus_clock_max = spi_bus_clock;
        handle->card_type = MICRO_SD_CARD_TYPE_UNKNOWN;
        handle->sector_count = 0;
        handle->is_initialized = 0;
        active_handle = handle;
        status = MICRO_SD_HANDLE_STATUS_OK;
    }

    return status;
}

micro_sd_handle_t *micro_sd_get_active_handle(void) { return active_handle; }

micro_sd_status_t micro_sd_init_card(micro_sd_handle_t *handle) {
    micro_sd_status_t status = MICRO_SD_STATUS_HANDLE_NOT_VALID;

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK) {
        handle->card_type = MICRO_SD_CARD_TYPE_UNKNOWN;
        handle->sector_count = 0;
        handle->is_initialized = 0;

        uint32_t original_BaudRatePrescaler =
            handle->hspi->Init.BaudRatePrescaler;

        status = micro_sd_reduce_spi_clock_to_range_of_100khz_to_400khz(handle);
        // status = MICRO_SD_STATUS_OK;
        if (status == MICRO_SD_STATUS_OK) {
            status = micro_sd_enter_spi_mode(handle);
        }

        if (status == MICRO_SD_STATUS_OK) {
            status = micro_sd_find_card_version(handle);
        }

        if (status == MICRO_SD_STATUS_OK) {
            BYTE csd[16] = {0};
            status = micro_sd_read_csd(handle, csd);

            if (status == MICRO_SD_STATUS_OK) {
                status = micro_sd_parse_csd(handle, csd);
            }
        }

        if (micro_sd_restore_spi_clock(handle, original_BaudRatePrescaler) !=
            MICRO_SD_STATUS_OK) {
            status = MICRO_SD_STATUS_FAILED_TO_RESTORE_SPI_CLOCK;
        }

        if (status == MICRO_SD_STATUS_OK) {
            handle->is_initialized = 1;
        }
    }

    return status;
}

static micro_sd_status_t micro_sd_reduce_spi_clock_to_range_of_100khz_to_400khz(
    micro_sd_handle_t *handle) {
    static const micro_sd_spi_prescaler_entry_t prescaler_entries[8] = {
        {SPI_BAUDRATEPRESCALER_2, 2},     {SPI_BAUDRATEPRESCALER_4, 4},
        {SPI_BAUDRATEPRESCALER_8, 8},     {SPI_BAUDRATEPRESCALER_16, 16},
        {SPI_BAUDRATEPRESCALER_32, 32},   {SPI_BAUDRATEPRESCALER_64, 64},
        {SPI_BAUDRATEPRESCALER_128, 128}, {SPI_BAUDRATEPRESCALER_256, 256},
    };

    micro_sd_status_t status = MICRO_SD_STATUS_FAILED_TO_REDUCE_SPI_CLOCK;

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK) {
        for (uint8_t i = 0; i < 8; i++) {
            uint32_t spi_clock =
                handle->spi_bus_clock_max / prescaler_entries[i].div;
            if (spi_clock >= SPI_MIN_HZ && spi_clock <= SPI_MAX_HZ) {
                handle->hspi->Init.BaudRatePrescaler =
                    prescaler_entries[i].hal_spi_prescaler;
                if (HAL_SPI_Init(handle->hspi) == HAL_OK) {
                    status = MICRO_SD_STATUS_OK;
                    break;
                } else {
                    status = MICRO_SD_STATUS_FAILED_TO_REDUCE_SPI_CLOCK;
                }
            }
        }
    }

    return status;
}

static micro_sd_status_t micro_sd_enter_spi_mode(micro_sd_handle_t *handle) {
    micro_sd_status_t status = MICRO_SD_STATUS_FAILED_TO_ENTER_SPI_MODE;
    uint8_t response = MICRO_SD_DUMMY_BYTE;

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK) {
        status = MICRO_SD_STATUS_OK;
        HAL_Delay(1);

        micro_sd_deselect_card(handle);

        // 1. 80클럭 정도 high 신호를 전송.
        for (uint8_t i = 0; i < 10 && status == MICRO_SD_STATUS_OK; i++) {
            status = micro_sd_write_byte(handle, MICRO_SD_DUMMY_BYTE);
        }

        // 2. CMD0 명령을 보내고 응답이 올바른지 확인.
        if (status == MICRO_SD_STATUS_OK) {
            status = micro_sd_send_command_packet(
                handle, MICRO_SD_CMD0_GO_IDLE_STATE, 0, MICRO_SD_CMD0_CRC,
                &response);

            if (status == MICRO_SD_STATUS_OK &&
                response != MICRO_SD_R1_IDLE_STATE) {
                status = MICRO_SD_STATUS_FAILED_TO_ENTER_SPI_MODE;
            }
        }
    }

    return status;
}

static micro_sd_status_t micro_sd_find_card_version(micro_sd_handle_t *handle) {
    micro_sd_status_t status = MICRO_SD_STATUS_FAILED_TO_FIND_CARD_VERSION;
    uint8_t response = MICRO_SD_DUMMY_BYTE;
    BYTE r7[4] = {0};

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK) {

        // 1. CMD8 명령을 전송.
        status = micro_sd_send_command_packet_and_read_bytes(
            handle, MICRO_SD_CMD8_SEND_IF_COND, MICRO_SD_CMD8_ARGUMENT,
            MICRO_SD_CMD8_CRC, &response, r7, 4);

        // 2. 응답이 0x1AA인지 확인 후, ACMD41명령을 성공하거나 타임아웃될
        // 때까지 전송
        if (status == MICRO_SD_STATUS_OK &&
            response == MICRO_SD_R1_IDLE_STATE && r7[2] == 0x01 &&
            r7[3] == 0xAA) {
            for (uint32_t retry = 0;
                 retry < 10 && response != MICRO_SD_R1_READY_STATE;
                 retry++) {
                status = micro_sd_send_app_command(
                    handle, MICRO_SD_ACMD41_SD_SEND_OP_COND,
                    MICRO_SD_ACMD41_HCS_ARGUMENT, MICRO_SD_DEFAULT_CRC,
                    &response);
            }

            HAL_Delay(1);

            if (status == MICRO_SD_STATUS_OK &&
                response != MICRO_SD_R1_READY_STATE) {
                status = MICRO_SD_STATUS_FAILED_TO_FIND_CARD_VERSION;
            }

            if (status == MICRO_SD_STATUS_OK &&
                response == MICRO_SD_R1_READY_STATE) {
                status = micro_sd_send_command_packet(
                    handle, MICRO_SD_CMD58_READ_OCR, 0, MICRO_SD_DEFAULT_CRC,
                    &response);
            }

            if (status == MICRO_SD_STATUS_OK) {
                BYTE ocr[4] = {0};
                status = micro_sd_read_bytes(handle, ocr, 4);

                if (status == MICRO_SD_STATUS_OK) {
                    if ((ocr[0] & MICRO_SD_OCR_CCS) != 0) {
                        handle->card_type = MICRO_SD_CARD_TYPE_SDHC;
                    } else {
                        handle->card_type = MICRO_SD_CARD_TYPE_SDSC;
                    }
                }
            }
            micro_sd_deselect_card(handle);
        } else if (status == MICRO_SD_STATUS_OK &&
                   (response & MICRO_SD_R1_ILLEGAL_COMMAND) != 0) {
            response = MICRO_SD_R1_IDLE_STATE;
            micro_sd_select_card(handle);

            for (uint32_t retry = 0;
                 retry < TIMEOUT_MS && response != MICRO_SD_R1_READY_STATE &&
                 status == MICRO_SD_STATUS_OK;
                 retry++) {
                status = micro_sd_send_app_command(
                    handle, MICRO_SD_ACMD41_SD_SEND_OP_COND, 0,
                    MICRO_SD_DEFAULT_CRC, &response);
                HAL_Delay(1);
            }

            if (status == MICRO_SD_STATUS_OK &&
                response == MICRO_SD_R1_READY_STATE) {
                handle->card_type = MICRO_SD_CARD_TYPE_SDSC;
            }
        } else {
            status = MICRO_SD_STATUS_FAILED_TO_FIND_CARD_VERSION;
        }

        if (status == MICRO_SD_STATUS_OK &&
            handle->card_type == MICRO_SD_CARD_TYPE_SDSC) {
            status = micro_sd_send_command_packet(
                handle, MICRO_SD_CMD16_SET_BLOCKLEN, MICRO_SD_BLOCK_SIZE,
                MICRO_SD_DEFAULT_CRC, &response);

            if (status == MICRO_SD_STATUS_OK &&
                response != MICRO_SD_R1_READY_STATE) {
                status = MICRO_SD_STATUS_FAILED_TO_FIND_CARD_VERSION;
            }
        }

        micro_sd_deselect_card(handle);
    }
    return status;
}

static micro_sd_status_t
micro_sd_restore_spi_clock(micro_sd_handle_t *handle,
                           uint32_t original_BaudRatePrescaler) {
    micro_sd_status_t status = MICRO_SD_STATUS_OK;

    handle->hspi->Init.BaudRatePrescaler = original_BaudRatePrescaler;

    if (HAL_SPI_Init(handle->hspi) != HAL_OK) {
        status = MICRO_SD_STATUS_FAILED_TO_RESTORE_SPI_CLOCK;
    }

    return status;
}

static micro_sd_status_t micro_sd_read_csd(micro_sd_handle_t *handle,
                                           BYTE *csd) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;
    uint8_t response = MICRO_SD_DUMMY_BYTE;

    if (csd != NULL) {
        status = micro_sd_send_command_packet(handle, MICRO_SD_CMD9_SEND_CSD, 0,
                                              MICRO_SD_DEFAULT_CRC, &response);

        if (status == MICRO_SD_STATUS_OK &&
            response == MICRO_SD_R1_READY_STATE) {
            status = micro_sd_wait_data_token(handle);
        } else {
            status = MICRO_SD_STATUS_FAILED_TO_READ_BLOCK;
        }

        if (status == MICRO_SD_STATUS_OK) {
            status = micro_sd_read_bytes(handle, csd, 16);
        }

        if (status == MICRO_SD_STATUS_OK) {
            BYTE crc[2] = {0};
            status = micro_sd_read_bytes(handle, crc, 2);
        }

        micro_sd_deselect_card(handle);
    }

    return status;
}

static micro_sd_status_t micro_sd_parse_csd(micro_sd_handle_t *handle,
                                            const BYTE *csd) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;

    if (handle != NULL && csd != NULL) {
        BYTE csd_structure = (csd[0] >> 6) & 0x03;

        if (csd_structure == 1) {
            DWORD c_size =
                ((DWORD)(csd[7] & 0x3F) << 16) | ((DWORD)csd[8] << 8) | csd[9];
            handle->sector_count = (c_size + 1) * 1024;
            status = MICRO_SD_STATUS_OK;
        } else if (csd_structure == 0) {
            DWORD read_bl_len = csd[5] & 0x0F;
            DWORD c_size = ((DWORD)(csd[6] & 0x03) << 10) |
                           ((DWORD)csd[7] << 2) | ((DWORD)(csd[8] & 0xC0) >> 6);
            DWORD c_size_mult =
                ((DWORD)(csd[9] & 0x03) << 1) | ((DWORD)(csd[10] & 0x80) >> 7);
            DWORD block_count = (c_size + 1) << (c_size_mult + 2);
            DWORD block_length = 1UL << read_bl_len;

            handle->sector_count =
                (block_count * block_length) / MICRO_SD_BLOCK_SIZE;
            status = MICRO_SD_STATUS_OK;
        } else {
            status = MICRO_SD_STATUS_FAILED_TO_IOCTL;
        }
    }

    return status;
}

micro_sd_status_t micro_sd_get_status(micro_sd_handle_t *handle) {
    micro_sd_status_t status = MICRO_SD_STATUS_HANDLE_NOT_VALID;

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK) {
        if (handle->is_initialized != 0) {
            status = MICRO_SD_STATUS_OK;
        } else {
            status = MICRO_SD_STATUS_FAILED_TO_ENTER_SPI_MODE;
        }
    }

    return status;
}

micro_sd_status_t micro_sd_read_block(micro_sd_handle_t *handle, BYTE *buffer,
                                      DWORD sector, UINT count) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;

    if (buffer != NULL && count > 0) {
        status = micro_sd_get_status(handle);
    }

    for (UINT i = 0; i < count && status == MICRO_SD_STATUS_OK; i++) {
        status = micro_sd_read_single_block(
            handle, &buffer[i * MICRO_SD_BLOCK_SIZE], sector + i);
    }

    return status;
}

micro_sd_status_t micro_sd_write_block(micro_sd_handle_t *handle, BYTE *buffer,
                                       DWORD sector, UINT count) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;

    if (buffer != NULL && count > 0) {
        status = micro_sd_get_status(handle);
    }

    for (UINT i = 0; i < count && status == MICRO_SD_STATUS_OK; i++) {
        status = micro_sd_write_single_block(
            handle, &buffer[i * MICRO_SD_BLOCK_SIZE], sector + i);
    }

    return status;
}

micro_sd_status_t micro_sd_ioctl(micro_sd_handle_t *handle, BYTE cmd,
                                 BYTE *buffer) {
    micro_sd_status_t status = micro_sd_get_status(handle);

    if (status == MICRO_SD_STATUS_OK) {
        switch (cmd) {
        case CTRL_SYNC:
            status = micro_sd_wait_ready(handle);
            break;
        case GET_SECTOR_COUNT:
            if (buffer != NULL) {
                *(DWORD *)buffer = handle->sector_count;
            } else {
                status = MICRO_SD_STATUS_INVALID_PARAMETER;
            }
            break;
        case GET_SECTOR_SIZE:
            if (buffer != NULL) {
                *(WORD *)buffer = MICRO_SD_BLOCK_SIZE;
            } else {
                status = MICRO_SD_STATUS_INVALID_PARAMETER;
            }
            break;
        case GET_BLOCK_SIZE:
            if (buffer != NULL) {
                *(DWORD *)buffer = 1;
            } else {
                status = MICRO_SD_STATUS_INVALID_PARAMETER;
            }
            break;
        case MMC_GET_TYPE:
            if (buffer != NULL) {
                *buffer = (BYTE)handle->card_type;
            } else {
                status = MICRO_SD_STATUS_INVALID_PARAMETER;
            }
            break;
        case MMC_GET_CSD:
            if (buffer != NULL) {
                status = micro_sd_read_csd(handle, buffer);
            } else {
                status = MICRO_SD_STATUS_INVALID_PARAMETER;
            }
            break;
        default:
            status = MICRO_SD_STATUS_FAILED_TO_IOCTL;
            break;
        }
    }

    return status;
}

static micro_sd_status_t micro_sd_read_single_block(micro_sd_handle_t *handle,
                                                    BYTE *buffer,
                                                    DWORD sector) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;
    uint8_t response = MICRO_SD_DUMMY_BYTE;

    if (buffer != NULL) {
        status = micro_sd_send_command_packet(
            handle, MICRO_SD_CMD17_READ_SINGLE_BLOCK,
            micro_sd_get_sector_address(handle, sector), MICRO_SD_DEFAULT_CRC,
            &response);

        if (status == MICRO_SD_STATUS_OK &&
            response == MICRO_SD_R1_READY_STATE) {
            status = micro_sd_wait_data_token(handle);
        } else {
            status = MICRO_SD_STATUS_FAILED_TO_READ_BLOCK;
        }

        if (status == MICRO_SD_STATUS_OK) {
            status = micro_sd_read_bytes(handle, buffer, MICRO_SD_BLOCK_SIZE);
        }

        if (status == MICRO_SD_STATUS_OK) {
            BYTE crc[2] = {0};
            status = micro_sd_read_bytes(handle, crc, 2);
        }

        micro_sd_deselect_card(handle);
    }

    return status;
}

static micro_sd_status_t micro_sd_write_single_block(micro_sd_handle_t *handle,
                                                     const BYTE *buffer,
                                                     DWORD sector) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;
    uint8_t response = MICRO_SD_DUMMY_BYTE;
    uint8_t token = MICRO_SD_DATA_TOKEN;
    uint8_t crc[2] = {MICRO_SD_DUMMY_BYTE, MICRO_SD_DUMMY_BYTE};

    if (buffer != NULL) {
        status = micro_sd_send_command_packet(
            handle, MICRO_SD_CMD24_WRITE_BLOCK,
            micro_sd_get_sector_address(handle, sector), MICRO_SD_DEFAULT_CRC,
            &response);

        if (status != MICRO_SD_STATUS_OK ||
            response != MICRO_SD_R1_READY_STATE) {
            status = MICRO_SD_STATUS_FAILED_TO_WRITE_BLOCK;
        }

        if (status == MICRO_SD_STATUS_OK) {
            status = micro_sd_write_bytes(handle, &token, 1);
        }

        if (status == MICRO_SD_STATUS_OK) {
            status = micro_sd_write_bytes(handle, buffer, MICRO_SD_BLOCK_SIZE);
        }

        if (status == MICRO_SD_STATUS_OK) {
            status = micro_sd_write_bytes(handle, crc, 2);
        }

        if (status == MICRO_SD_STATUS_OK) {
            status = micro_sd_read_byte(handle, &response);
        }

        if (status == MICRO_SD_STATUS_OK &&
            (response & MICRO_SD_DATA_RESPONSE_MASK) !=
                MICRO_SD_DATA_ACCEPTED) {
            status = MICRO_SD_STATUS_FAILED_TO_WRITE_BLOCK;
        }

        if (status == MICRO_SD_STATUS_OK) {
            status = micro_sd_wait_ready(handle);
        }

        micro_sd_deselect_card(handle);
    }

    return status;
}

static micro_sd_status_t
micro_sd_send_app_command(micro_sd_handle_t *handle, uint8_t command,
                          uint32_t argument, uint8_t crc, uint8_t *response) {
    micro_sd_status_t status = micro_sd_send_command_packet(
        handle, MICRO_SD_CMD55_APP_CMD, 0, MICRO_SD_DEFAULT_CRC, response);

    if (status == MICRO_SD_STATUS_OK) {
        status = micro_sd_send_command_packet(handle, command, argument, crc,
                                              response);
    }

    return status;
}

static micro_sd_status_t micro_sd_send_command_packet(micro_sd_handle_t *handle,
                                                      uint8_t command,
                                                      uint32_t argument,
                                                      uint8_t crc,
                                                      uint8_t *response) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;
    BYTE packet[6] = {0};

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK &&
        response != NULL) {
        status = MICRO_SD_STATUS_OK;
        while (HAL_SPI_GetState(handle->hspi) != HAL_SPI_STATE_READY)
            ;

        micro_sd_select_card(handle);
        packet[0] = 0x40U | command;
        packet[1] = (BYTE)(argument >> 24);
        packet[2] = (BYTE)(argument >> 16);
        packet[3] = (BYTE)(argument >> 8);
        packet[4] = (BYTE)argument;
        packet[5] = crc;

        for (uint8_t i = 0; i < sizeof(packet) && status == MICRO_SD_STATUS_OK;
             i++) {
            status = micro_sd_write_byte(handle, packet[i]);
        }

        if (status == MICRO_SD_STATUS_OK) {
            *response = MICRO_SD_DUMMY_BYTE;

            for (uint32_t i = 0;
                 i < MICRO_SD_RESPONSE_RETRY_COUNT &&
                 (*response & 0x80U) != 0 && status == MICRO_SD_STATUS_OK;
                 i++) {
                status = micro_sd_read_byte(handle, response);
            }

            if (status == MICRO_SD_STATUS_OK && (*response & 0x80U) != 0) {
                status = MICRO_SD_STATUS_FAILED_TO_READ_RESPONSE;
            }
        }
        micro_sd_deselect_card(handle);
    }

    return status;
}

static micro_sd_status_t micro_sd_send_command_packet_and_read_bytes(
    micro_sd_handle_t *handle, uint8_t command, uint32_t argument, uint8_t crc,
    uint8_t *response, BYTE *data, uint8_t length) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;
    BYTE packet[6] = {0};

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK &&
        response != NULL) {
        status = MICRO_SD_STATUS_OK;
        while (HAL_SPI_GetState(handle->hspi) != HAL_SPI_STATE_READY)
            ;

        micro_sd_select_card(handle);
        packet[0] = 0x40U | command;
        packet[1] = (BYTE)(argument >> 24);
        packet[2] = (BYTE)(argument >> 16);
        packet[3] = (BYTE)(argument >> 8);
        packet[4] = (BYTE)argument;
        packet[5] = crc;

        for (uint8_t i = 0; i < sizeof(packet) && status == MICRO_SD_STATUS_OK;
             i++) {
            status = micro_sd_write_byte(handle, packet[i]);
        }

        if (status == MICRO_SD_STATUS_OK) {
            *response = MICRO_SD_DUMMY_BYTE;

            for (uint32_t i = 0;
                 i < MICRO_SD_RESPONSE_RETRY_COUNT &&
                 (*response & 0x80U) != 0 && status == MICRO_SD_STATUS_OK;
                 i++) {
                status = micro_sd_read_byte(handle, response);
            }
            if (status == MICRO_SD_STATUS_OK) {
                status = micro_sd_read_bytes(handle, data, length);
            }

            if (status == MICRO_SD_STATUS_OK && (*response & 0x80U) != 0) {
                status = MICRO_SD_STATUS_FAILED_TO_READ_RESPONSE;
            }
        }
        micro_sd_deselect_card(handle);
    }

    return status;
}

static micro_sd_status_t micro_sd_wait_ready(micro_sd_handle_t *handle) {
    micro_sd_status_t status = MICRO_SD_STATUS_FAILED_TO_WAIT_READY;
    BYTE response = 0;

    for (uint32_t retry = 0; retry < TIMEOUT_MS && status != MICRO_SD_STATUS_OK;
         retry++) {
        if (micro_sd_read_byte(handle, &response) == MICRO_SD_STATUS_OK &&
            response == MICRO_SD_DUMMY_BYTE) {
            status = MICRO_SD_STATUS_OK;
        }
    }

    return status;
}

static micro_sd_status_t micro_sd_wait_data_token(micro_sd_handle_t *handle) {
    micro_sd_status_t status = MICRO_SD_STATUS_FAILED_TO_READ_BLOCK;
    BYTE token = 0;

    for (uint32_t retry = 0; retry < TIMEOUT_MS && status != MICRO_SD_STATUS_OK;
         retry++) {
        if (micro_sd_read_byte(handle, &token) == MICRO_SD_STATUS_OK &&
            token == MICRO_SD_DATA_TOKEN) {
            status = MICRO_SD_STATUS_OK;
        }
    }

    return status;
}

static micro_sd_status_t micro_sd_read_bytes(micro_sd_handle_t *handle,
                                             BYTE *buffer, UINT length) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK &&
        buffer != NULL && length > 0) {
        for (UINT i = 0; i < length; i++) {
            status = micro_sd_read_byte(handle, &buffer[i]);

            if (status != MICRO_SD_STATUS_OK) {
                break;
            }
        }
    }

    return status;
}

static micro_sd_status_t micro_sd_write_bytes(micro_sd_handle_t *handle,
                                              const BYTE *buffer, UINT length) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK &&
        buffer != NULL && length > 0) {
        for (UINT i = 0; i < length; i++) {
            status = micro_sd_write_byte(handle, buffer[i]);

            if (status != MICRO_SD_STATUS_OK) {
                break;
            }
        }
    }

    return status;
}

static micro_sd_status_t micro_sd_read_byte(micro_sd_handle_t *handle,
                                            BYTE *value) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;
    uint8_t request = MICRO_SD_DUMMY_BYTE;

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK &&
        value != NULL) {
        while (HAL_SPI_GetState(handle->hspi) != HAL_SPI_STATE_READY)
            ;

        if (HAL_SPI_TransmitReceive(handle->hspi, &request, value, 1,
                                    TIMEOUT_MS) == HAL_OK) {
            status = MICRO_SD_STATUS_OK;
        } else {
            status = MICRO_SD_STATUS_FAILED_TO_READ_RESPONSE;
        }
    }

    return status;
}

static micro_sd_status_t micro_sd_write_byte(micro_sd_handle_t *handle,
                                             BYTE value) {
    micro_sd_status_t status = MICRO_SD_STATUS_INVALID_PARAMETER;

    if (micro_sd_get_handle_status(handle) == MICRO_SD_HANDLE_STATUS_OK) {
        while (HAL_SPI_GetState(handle->hspi) != HAL_SPI_STATE_READY)
            ;

        if (HAL_SPI_Transmit(handle->hspi, &value, 1, TIMEOUT_MS) == HAL_OK) {
            status = MICRO_SD_STATUS_OK;
        } else {
            status = MICRO_SD_STATUS_FAILED_TO_SEND_COMMAND;
        }
    }

    return status;
}

static DWORD micro_sd_get_sector_address(micro_sd_handle_t *handle,
                                         DWORD sector) {
    DWORD address = sector;

    if (handle->card_type != MICRO_SD_CARD_TYPE_SDHC) {
        address = sector * MICRO_SD_BLOCK_SIZE;
    }

    return address;
}

static void micro_sd_select_card(micro_sd_handle_t *handle) {
    HAL_GPIO_WritePin(handle->GPIO_Port_CS, handle->GPIO_Pin_CS,
                      GPIO_PIN_RESET);
}

static void micro_sd_deselect_card(micro_sd_handle_t *handle) {
    HAL_GPIO_WritePin(handle->GPIO_Port_CS, handle->GPIO_Pin_CS, GPIO_PIN_SET);
    (void)micro_sd_write_byte(handle, MICRO_SD_DUMMY_BYTE);
}

static micro_sd_handle_status_t
micro_sd_get_handle_status(micro_sd_handle_t *handle) {
    micro_sd_handle_status_t handle_status = MICRO_SD_HANDLE_STATUS_OK;

    if (handle == NULL) {
        handle_status = MICRO_SD_HANDLE_STATUS_INVALID;
    } else {
        if (handle->hspi == NULL || handle->GPIO_Port_CS == NULL ||
            handle->GPIO_Pin_CS == 0 || handle->spi_bus_clock_max == 0) {
            handle_status = MICRO_SD_HANDLE_STATUS_INVALID;
        }
    }

    return handle_status;
}
