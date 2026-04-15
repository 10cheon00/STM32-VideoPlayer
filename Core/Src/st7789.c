#include "st7789.h"

#define TIMEOUT_MS 100
#define WAIT_MS 150

typedef enum {
    SWRESET = 0x01,
    SLPOUT = 0x11,
    NORON = 0x13,
    INVON = 0x21,
    DISPON = 0x29,
    CASET = 0x2A,
    RASET = 0x2B,
    RAMWR = 0x2C,
    MADCTL = 0x36,
    COLMOD = 0x3A,
} st7789_command_t;

typedef enum {
    DISPLAY_DATA_LATCH_ORDER_LEFT_TO_RIGHT = 0x0,
    DISPLAY_DATA_LATCH_ORDER_RIGHT_TO_LEFT = 0x4,
    RGB_ORDER_RGB = 0x0,
    RGB_ORDER_BGR = 0x8,
    LINE_ADDRESS_ORDER_TOP_TO_BOTTOM = 0x0,
    LINE_ADDRESS_ORDER_BOTTOM_TO_TOP = 0x10,
    PAGE_COLUMN_ORDER_NORMAL_MODE = 0x0,
    PAGE_COLUMN_ORDER_REVERSE_MODE = 0x20,
    COLUMN_ADDRESS_ORDER_LEFT_TO_RIGHT = 0x0,
    COLUMN_ADDRESS_ORDER_RIGHT_TO_LEFT = 0x40,
    PAGE_ADDRESS_ORDER_TOP_TO_BOTTOM = 0x0,
    PAGE_ADDRESS_ORDER_BOTTOM_TO_TOP = 0x80,
} st7789_madctl_parameter_t;

typedef enum {
    RGB565_INTERFACE = 0x50,
    RGB666_INTERFACE = 0x60,
} st7789_rgb_interface_color_format_t;

typedef enum {
    COLOR_FORMAT_12BIT = 0x3,
    COLOR_FORMAT_16BIT = 0x5,
    COLOR_FORMAT_18BIT = 0x6,
    COLOR_FORMAT_16M_TRUNCATED = 0x7
} st7789_control_interface_color_format_t;

#define NO_PARAMETER 0

#define IS_SPI_DMA_ENABLED(hspi) (hspi->hdmatx != NULL)

static st7789_status_t st7789_send_command(st7789_handle_t *handle,
                                           uint8_t command, uint8_t *parameters,
                                           uint16_t parameter_length) {
    st7789_status_t status = STATUS_OK;
    HAL_StatusTypeDef hal_status = HAL_OK;

    // spi가 dma를 사용해 전송했다면, 직전 전송이 끝나지 않았을 수 있으므로,
    // 완료될 때까지 기다림
    while (HAL_SPI_GetState(handle->hspi) != HAL_SPI_STATE_READY)
        ;

    // 모듈에게 데이터를 전송할 것이므로 CS를 low로 설정함.
    HAL_GPIO_WritePin(handle->GPIO_Port_CS, handle->GPIO_Pin_CS,
                      GPIO_PIN_RESET);
    // 명령어를 보낼 땐 DC를 low로 만들어야함.
    HAL_GPIO_WritePin(handle->GPIO_Port_DC, handle->GPIO_Pin_DC,
                      GPIO_PIN_RESET);

    // 인자로 받은 명령어를 전송함
    hal_status = HAL_SPI_Transmit(handle->hspi, &command, 1, TIMEOUT_MS);
    if (hal_status != HAL_OK) {
        status = STATUS_TRANSMIT_FAILED;
    }

    if (parameter_length > 0) {
        // 파라미터를 전송함
        // 램에 쓰기를 할 땐 DC를 high로 설정
        HAL_GPIO_WritePin(handle->GPIO_Port_DC, handle->GPIO_Pin_DC,
                          GPIO_PIN_SET);

        if (handle->is_dma_enabled) {
            // spi가 dma를 지원하면 dma 방식으로 전송 후,
            // 등록된 콜백함수에 의해 명령어 전송을 끝냄
            hal_status = HAL_SPI_Transmit_DMA(handle->hspi, parameters,
                                              parameter_length);
            if (hal_status != HAL_OK) {
                status = STATUS_TRANSMIT_FAILED;
            }
        } else {
            // spi가 dma를 지원하지 않는다면 폴링으로 전송 후 명령어 전송을 끝냄
            hal_status = HAL_SPI_Transmit(handle->hspi, parameters,
                                          parameter_length, TIMEOUT_MS);
            if (hal_status != HAL_OK) {
                status = STATUS_TRANSMIT_FAILED;
            }
            // 전송이 끝난 후에는 DC, CS핀을 초기화함
            HAL_GPIO_WritePin(handle->GPIO_Port_CS, handle->GPIO_Pin_CS,
                              GPIO_PIN_SET);
            HAL_GPIO_WritePin(handle->GPIO_Port_DC, handle->GPIO_Pin_DC,
                              GPIO_PIN_RESET);
        }
    } else {
        // 전송이 끝난 후에는 DC, CS핀을 초기화함
        HAL_GPIO_WritePin(handle->GPIO_Port_CS, handle->GPIO_Pin_CS,
                          GPIO_PIN_SET);
        HAL_GPIO_WritePin(handle->GPIO_Port_DC, handle->GPIO_Pin_DC,
                          GPIO_PIN_RESET);
    }

    return status;
}

st7789_status_t st7789_dma_tx_cplt_callback(st7789_handle_t *handle) {
    // 파라미터 전송을 위해 사용했던 DMA 전송이 끝난 후에는 DC, CS핀을 초기화함
    HAL_GPIO_WritePin(handle->GPIO_Port_CS, handle->GPIO_Pin_CS, GPIO_PIN_SET);
    HAL_GPIO_WritePin(handle->GPIO_Port_DC, handle->GPIO_Pin_DC,
                      GPIO_PIN_RESET);

    return STATUS_OK;
}

static st7789_status_t st7789_send_image(st7789_handle_t *handle,
                                         st7789_rgb565_t *image, uint16_t sx,
                                         uint16_t sy, uint16_t ex,
                                         uint16_t ey) {
    st7789_status_t status = STATUS_OK;
    uint8_t parameters[4] = {
        (uint8_t)((sx & 0xFF00) >> 8),
        (uint8_t)(sx & 0x00FF),
        (uint8_t)((ex & 0xFF00) >> 8),
        (uint8_t)(ex & 0x00FF),
    };
    uint16_t image_length = (ex - sx) * (ey - sy);

    // 1. CASET 전송
    // TODO:램에 쓰는거니까 DC를 1로 설정
    // 파라미터는 시작 주소를 먼저, 종료 주소를 나중에 전송
    status = st7789_send_command(handle, CASET, parameters, 4);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 2. RASET 전송
    // 파라미터는 시작 주소를 먼저, 종료 주소를 나중에 전송
    parameters[0] = (uint8_t)((sy & 0xFF00) >> 8);
    parameters[1] = (uint8_t)(sy & 0x00FF);
    parameters[2] = (uint8_t)((ey & 0xFF00) >> 8);
    parameters[3] = (uint8_t)(ey & 0x00FF);
    status = st7789_send_command(handle, RASET, parameters, 4);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 3. RAMWR 전송
    status =
        st7789_send_command(handle, RAMWR, (uint8_t *)image, 2 * image_length);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    return STATUS_OK;
}

st7789_status_t
st7789_init_handle(st7789_handle_t *handle, SPI_HandleTypeDef *hspi,
                   GPIO_TypeDef *GPIO_Port_CS, GPIO_TypeDef *GPIO_Port_DC,
                   GPIO_TypeDef *GPIO_Port_RST, GPIO_TypeDef *GPIO_Port_SCL,
                   GPIO_TypeDef *GPIO_Port_SDA, uint16_t GPIO_Pin_CS,
                   uint16_t GPIO_Pin_DC, uint16_t GPIO_Pin_RST,
                   uint16_t GPIO_Pin_SCL, uint16_t GPIO_Pin_SDA,
                   uint8_t enable_dma) {
    handle->hspi = hspi;
    handle->GPIO_Port_CS = GPIO_Port_CS;
    handle->GPIO_Port_DC = GPIO_Port_DC;
    handle->GPIO_Port_RST = GPIO_Port_RST;
    handle->GPIO_Port_SCL = GPIO_Port_SCL;
    handle->GPIO_Port_SDA = GPIO_Port_SDA;
    handle->GPIO_Pin_CS = GPIO_Pin_CS;
    handle->GPIO_Pin_DC = GPIO_Pin_DC;
    handle->GPIO_Pin_RST = GPIO_Pin_RST;
    handle->GPIO_Pin_SCL = GPIO_Pin_SCL;
    handle->GPIO_Pin_SDA = GPIO_Pin_SDA;

    if (enable_dma && IS_SPI_DMA_ENABLED(handle->hspi)) {
        handle->is_dma_enabled = 1;
    }

    return STATUS_OK;
}

st7789_status_t st7789_init_display(st7789_handle_t *handle) {
    st7789_status_t status = STATUS_OK;
    uint8_t parameter = 0;

    // RST핀에 엣지를 발생시킴
    HAL_GPIO_WritePin(handle->GPIO_Port_RST, handle->GPIO_Pin_RST,
                      GPIO_PIN_RESET);
    HAL_GPIO_WritePin(handle->GPIO_Port_RST, handle->GPIO_Pin_RST,
                      GPIO_PIN_SET);

    // 1. SWRESET 전송 후, 120ms만큼 기다림
    status = st7789_send_command(handle, SWRESET, NO_PARAMETER, 0);
    HAL_Delay(WAIT_MS);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 2. SLPOUT 전송 후, 150ms 만큼 기다림
    status = st7789_send_command(handle, SLPOUT, NO_PARAMETER, 0);
    HAL_Delay(WAIT_MS);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 3. NORON 전송
    status = st7789_send_command(handle, NORON, NO_PARAMETER, 0);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 4. INVON 전송
    status = st7789_send_command(handle, INVON, NO_PARAMETER, 0);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 5. DISPON 전송
    status = st7789_send_command(handle, DISPON, NO_PARAMETER, 0);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 6. MADCTL 전송
    parameter =
        PAGE_ADDRESS_ORDER_TOP_TO_BOTTOM | COLUMN_ADDRESS_ORDER_LEFT_TO_RIGHT |
        PAGE_COLUMN_ORDER_NORMAL_MODE | LINE_ADDRESS_ORDER_TOP_TO_BOTTOM |
        RGB_ORDER_RGB | DISPLAY_DATA_LATCH_ORDER_LEFT_TO_RIGHT;
    status = st7789_send_command(handle, MADCTL, &parameter, 1);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 7. COLMOD 전송
    parameter = RGB565_INTERFACE | COLOR_FORMAT_16BIT;
    status = st7789_send_command(handle, COLMOD, &parameter, 1);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    return status;
}

st7789_status_t st7789_print_sample_display(st7789_handle_t *handle) {
    uint16_t image[240] = {0};

    for (uint16_t y = 0; y < 80; y++) {
        for (uint16_t i = 0; i < 240; i++) {
            image[i] = 0x00F8;
        }
        st7789_send_image(handle, image, 0, y, 240, y + 1);
    }

    for (uint16_t y = 80; y < 160; y++) {
        for (uint16_t i = 0; i < 240; i++) {
            image[i] = 0xE007;
        }
        st7789_send_image(handle, image, 0, y, 240, y + 1);
    }

    for (uint16_t y = 160; y < 240; y++) {
        for (uint16_t i = 0; i < 240; i++) {
            image[i] = 0x1F00;
        }
        st7789_send_image(handle, image, 0, y, 240, y + 1);
    }
    return STATUS_OK;
}

st7789_status_t st7789_print_pixels_with_range(st7789_handle_t *handle,
                                               void *buffer,
                                               uint16_t sx, uint16_t sy,
                                               uint16_t ex, uint16_t ey) {
    st7789_status_t status =
        st7789_send_image(handle, (st7789_rgb565_t *)buffer, sx, sy, ex, ey);
    return status;
}
