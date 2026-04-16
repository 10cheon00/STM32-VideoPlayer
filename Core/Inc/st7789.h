#ifndef _ST7789_H_
#define _ST7789_H_

/**
 * 참고한 데이터 시트
 * https://cdn-learn.adafruit.com/downloads/pdf/adafruit-1-3-and-1-54-240-x-240-wide-angle-tft-lcd-displays.pdf
 */

#include "stm32f4xx_hal.h"

typedef enum {
    ST7789_STATUS_OK = 0,
    ST7789_STATUS_TRANSMIT_FAILED,
} st7789_status_t;

typedef enum {
    ST7789_DMA_DISABLE = 0,
    ST7789_DMA_ENABLE,
} st7789_dma_status_t;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *GPIO_Port_CS;
    GPIO_TypeDef *GPIO_Port_DC;
    GPIO_TypeDef *GPIO_Port_RST;
    uint16_t GPIO_Pin_CS;
    uint16_t GPIO_Pin_DC;
    uint16_t GPIO_Pin_RST;
    st7789_dma_status_t dma_status;
    uint8_t is_dma_tx_done;
    uint16_t screen_width;
    uint16_t screen_height;
} st7789_handle_t;

typedef uint16_t st7789_rgb565_t;

/**
 * 데이터 시트에 따르면 통신에 앞서 인터페이스 선택이 필요함. 시리얼 통신의 경우
 * 4가지 인터페이스가 존재하는데, SPI를 사용하므로 4-line serial interface I를
 * 선택함.(출력 선이 필요없음)
 * CS = CS
 * WRX = DC
 * DCX = SCL
 * SDA = SDA
 * DC가 low면 명령어를 받는 것으로, high면 메모리에 쓰는 것으로 이해한다.
 * CS가 falling edge면 해당 모듈을 선택한 것으로 판단하고 SDA와 SCL에 전달되는
 * 데이터를 받고, 처리한다.
 *
 * SWRESET 명령을 보내면 디스플레이가 내부를 초기화한다. 명령을 보내기 전 5ms,
 * 명령을 보낸 후 120ms를 기다려야 한다.
 * Sleep in 상태에 돌입하면 절전 모드에 진입한다.
 * Sleep out 명령을 보내면 Sleep in 상태를 해제하고 내부를 초기화한다.
 *
 * Sleep out 명령어가 실행되면 다음 항목들을 설정해야한다.
 * - Normal / Partial display mode
 * - Display Inversion On/Off  -> 색상값 뒤집기 (rgb가 cmyk로 바뀜)
 * - Display On/Off
 * - Memory Data Access Control
 * - Interface Pixel Format
 *
 *
 */

st7789_status_t st7789_init_handle(st7789_handle_t *handle,
                                   SPI_HandleTypeDef *hspi,
                                   GPIO_TypeDef *GPIO_Port_CS,
                                   GPIO_TypeDef *GPIO_Port_DC,
                                   GPIO_TypeDef *GPIO_Port_RST,
                                   uint16_t GPIO_Pin_CS, uint16_t GPIO_Pin_DC,
                                   uint16_t GPIO_Pin_RST, uint16_t screen_width,
                                   uint16_t screen_height, st7789_dma_status_t dma_status);

st7789_status_t st7789_init_display(st7789_handle_t *handle);

st7789_status_t st7789_print_sample_display(st7789_handle_t *handle);

st7789_status_t st7789_print_pixels_with_range(st7789_handle_t *handle,
                                               void *buffer, uint16_t sx,
                                               uint16_t sy, uint16_t ex,
                                               uint16_t ey);

/**
 * 이 함수는 LCD 출력용으로 사용하는 SPI가 DMA를 사용하여 전송하는 경우,
 * 사용자가 정의한 DMA 콜백 함수에서 호출해주어야 할 함수입니다.
 */
st7789_status_t st7789_dma_tx_cplt_callback(st7789_handle_t *handle);

#endif
