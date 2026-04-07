#include "micro_sd.h"

typedef struct {
    uint32_t hal_spi_prescaler;
    uint16_t div;
} micro_sd_spi_prescaler_entry_t;

#define SPI_MIN_KHZ 100000U
#define SPI_MAX_KHZ 400000U

static micro_sd_status_t micro_sd_reduce_spi_clock_to_range_of_100khz_to_400khz(
    micro_sd_handle_t *handle) {
    static const micro_sd_spi_prescaler_entry_t prescaler_entries[8] = {
        {SPI_BAUDRATEPRESCALER_2, 2},     {SPI_BAUDRATEPRESCALER_4, 4},
        {SPI_BAUDRATEPRESCALER_8, 8},     {SPI_BAUDRATEPRESCALER_16, 16},
        {SPI_BAUDRATEPRESCALER_32, 32},   {SPI_BAUDRATEPRESCALER_64, 64},
        {SPI_BAUDRATEPRESCALER_128, 128}, {SPI_BAUDRATEPRESCALER_256, 256},
    };

    const micro_sd_spi_bus_clock_t bus_clock =
        handle->micro_sd_get_spi_bus_clock_callback();

    micro_sd_status_t status = STATUS_FAILED_TO_REDUCE_SPI_CLOCK;

    if (micro_sd_is_valid_handle(handle)) {
        for (uint8_t i = 0; i < 8; i++) {
            uint32_t spi_clock = bus_clock / prescaler_entries[i].div;
            if (spi_clock >= SPI_MIN_KHZ && spi_clock <= SPI_MAX_KHZ) {
                handle->hspi->Init.BaudRatePrescaler =
                    prescaler_entries[i].hal_spi_prescaler;
                if (HAL_SPI_Init(handle->hspi) == HAL_OK) {
                    status = STATUS_OK;
                    break;
                } else {
                    status = STATUS_FAILED_TO_REDUCE_SPI_CLOCK;
                }
            }
        }
    }

    return status;
}

micro_sd_status_t micro_sd_init_handle(
    micro_sd_handle_t *handle, SPI_HandleTypeDef *hspi,
    GPIO_TypeDef *GPIO_Port_CS, uint16_t GPIO_Pin_CS,
    micro_sd_spi_bus_clock_t (*micro_sd_get_spi_bus_clock_callback)()) {

    handle->hspi = hspi;
    handle->GPIO_Port_CS = GPIO_Port_CS;
    handle->GPIO_Pin_CS = GPIO_Pin_CS;
    handle->micro_sd_get_spi_bus_clock_callback =
        micro_sd_get_spi_bus_clock_callback;

    return STATUS_OK;
}

micro_sd_status_t micro_sd_is_valid_handle(micro_sd_handle_t *handle) {
    micro_sd_status_t status = STATUS_OK;

    if (handle == NULL) {
        status = STATUS_HANDLE_NOT_VALID;
    } else {
        if (handle->hspi == NULL || handle->GPIO_Port_CS == NULL ||
            handle->GPIO_Pin_CS == NULL ||
            handle->micro_sd_get_spi_bus_clock_callback == NULL) {
            status = STATUS_HANDLE_NOT_VALID;
        }
    }

    return status;
}

micro_sd_status_t micro_sd_init_card(micro_sd_handle_t *handle) {
    micro_sd_status_t status = STATUS_OK;
    // 1. spi 클럭 낮추기
    // 나중에 spi의 클럭을 되돌려야 하므로, 원본 클럭 정보값을 기억해두기
    uint32_t original_BaudRatePrescaler = handle->hspi->Init.BaudRatePrescaler;
    if (status == STATUS_OK &&
        micro_sd_reduce_spi_clock_to_range_of_100khz_to_400khz(handle) !=
            HAL_OK) {
    }

    // 2. sd카드를 spi모드로 전환하기

    // 3. sd카드의 정보 획득하기

    // 4. 낮추었던 spi 클럭 되돌리기
    if (status == STATUS_OK &&
        micro_sd_restore_spi_clock(handle, original_BaudRatePrescaler) !=
            HAL_OK) {
    }

    return status;
}

micro_sd_status_t micro_sd_get_status(micro_sd_handle_t *handle) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_read_block(micro_sd_handle_t *handle, BYTE *buffer,
                                      DWORD sector, UINT count) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_write_block(micro_sd_handle_t *handle, BYTE *buffer,
                                       DWORD sector, UINT count) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_ioctl(micro_sd_handle_t *handle, BYTE cmd,
                                 BYTE *buffer) {
    return STATUS_OK;
}
