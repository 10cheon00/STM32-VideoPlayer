#ifndef _MICRO_SD_H_
#define _MICRO_SD_H_

#include "integer.h"

#include "stm32f4xx_hal.h"

typedef uint32_t micro_sd_spi_bus_clock_t;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *GPIO_Port_CS;
    uint16_t GPIO_Pin_CS;

    micro_sd_spi_bus_clock_t (*micro_sd_get_spi_bus_clock_callback)();
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
} micro_sd_status_t;

typedef struct {

} micro_sd_file_t;

typedef struct {

} micro_sd_file_header_t;

/**
 * @brief 보드별 micro SD 핸들을 반환합니다.
 *
 * @note 사용자는 이 함수를 애플리케이션/보드 포팅 계층에서 반드시
 *       구현해야 합니다. 이 함수가 구현되지 않으면 micro SD 미들웨어가
 *       FatFs diskio 계층에서 사용할 핸들을 얻을 수 없으며, 최종 링크
 *       단계에서 undefined reference 오류가 발생합니다.
 *
 * @brief Returns the board-specific micro SD handle.
 *
 * @note The user must implement this function in the application/board port
 *       layer. If this function is not implemented, the micro SD middleware
 *       cannot obtain the handle used by the FatFs diskio layer, and the final
 *       link step will fail with an undefined reference error.
 */
micro_sd_handle_t *micro_sd_get_handle();

micro_sd_status_t micro_sd_init_handle(
    micro_sd_handle_t *handle, SPI_HandleTypeDef *hspi,
    GPIO_TypeDef *GPIO_Port_CS, uint16_t GPIO_Pin_CS,
    micro_sd_spi_bus_clock_t (*micro_sd_get_spi_bus_clock_callback)());

micro_sd_handle_status_t micro_sd_get_handle_status(micro_sd_handle_t *handle);

micro_sd_status_t micro_sd_init_card(micro_sd_handle_t *handle);

micro_sd_status_t micro_sd_get_status(micro_sd_handle_t *handle);

micro_sd_status_t micro_sd_read_block(micro_sd_handle_t *handle, BYTE *buffer,
                                      DWORD sector, UINT count);

micro_sd_status_t micro_sd_write_block(micro_sd_handle_t *handle, BYTE *buffer,
                                       DWORD sector, UINT count);

micro_sd_status_t micro_sd_ioctl(micro_sd_handle_t *handle, BYTE cmd,
                                 BYTE *buffer);

#endif
