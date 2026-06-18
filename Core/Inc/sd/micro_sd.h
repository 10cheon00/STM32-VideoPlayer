#ifndef _MICRO_SD_H_
#define _MICRO_SD_H_

#include "integer.h"

#include "stm32f4xx_hal.h"

typedef uint32_t micro_sd_spi_bus_clock_t;

typedef enum {
    MICRO_SD_CARD_TYPE_UNKNOWN = 0,
    MICRO_SD_CARD_TYPE_SDSC,
    MICRO_SD_CARD_TYPE_SDHC,
} micro_sd_card_type_t;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *GPIO_Port_CS;
    uint16_t GPIO_Pin_CS;
    micro_sd_spi_bus_clock_t spi_bus_clock_max;
    micro_sd_card_type_t card_type;
    DWORD sector_count;
    uint8_t is_initialized;
} micro_sd_handle_t;

typedef enum {
    MICRO_SD_HANDLE_STATUS_OK = 0,
    MICRO_SD_HANDLE_STATUS_INVALID,
} micro_sd_handle_status_t;

typedef enum {
    MICRO_SD_STATUS_OK = 0,
    MICRO_SD_STATUS_HANDLE_NOT_VALID,
    MICRO_SD_STATUS_FAILED_TO_REDUCE_SPI_CLOCK,
    MICRO_SD_STATUS_FAILED_TO_SEND_COMMAND,
    MICRO_SD_STATUS_FAILED_TO_ENTER_SPI_MODE,
    MICRO_SD_STATUS_FAILED_TO_FIND_CARD_VERSION,
    MICRO_SD_STATUS_FAILED_TO_RESTORE_SPI_CLOCK,
    MICRO_SD_STATUS_FAILED_TO_WAIT_READY,
    MICRO_SD_STATUS_FAILED_TO_READ_RESPONSE,
    MICRO_SD_STATUS_FAILED_TO_READ_BLOCK,
    MICRO_SD_STATUS_FAILED_TO_WRITE_BLOCK,
    MICRO_SD_STATUS_FAILED_TO_IOCTL,
    MICRO_SD_STATUS_INVALID_PARAMETER,
} micro_sd_status_t;

micro_sd_status_t micro_sd_init_handle(
    micro_sd_handle_t *handle, SPI_HandleTypeDef *hspi,
    GPIO_TypeDef *GPIO_Port_CS, uint16_t GPIO_Pin_CS,
    micro_sd_spi_bus_clock_t spi_bus_clock_max);

micro_sd_handle_t *micro_sd_get_active_handle(void);

micro_sd_status_t micro_sd_init_card(micro_sd_handle_t *handle);

micro_sd_status_t micro_sd_get_status(micro_sd_handle_t *handle);

micro_sd_status_t micro_sd_read_block(micro_sd_handle_t *handle, BYTE *buffer,
                                      DWORD sector, UINT count);

micro_sd_status_t micro_sd_write_block(micro_sd_handle_t *handle, BYTE *buffer,
                                       DWORD sector, UINT count);

micro_sd_status_t micro_sd_ioctl(micro_sd_handle_t *handle, BYTE cmd,
                                 BYTE *buffer);

#endif
