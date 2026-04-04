#include "st7789.h"

#define TIMEOUT_MS 100
#define WAIT_MS 150

typedef enum {
    SWRESET = 0x01,
    SLPOUT = 0x11,
    NORON = 0x13,
    INVOFF = 0x20,
    DISPON = 0x29,
    CASET = 0x2A,
    RASET = 0x2B,
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

static st7789_status_t st7789_send_command(st7789_handle_t *handle,
                                           uint16_t command,
                                           uint16_t parameter) {
    st7789_status_t status = STATUS_OK;
    HAL_StatusTypeDef hal_status = HAL_OK;
    uint8_t buffer[4] = {(command & 0xFF00) >> 8, (command & 0x00FF),
                         (parameter & 0xFF00) >> 8, (parameter & 0xFF)};

    // 모듈에게 데이터를 전송할 것이므로 CS를 low로 설정함.
    HAL_GPIO_WritePin(handle->GPIO_Port_CS, handle->GPIO_Pin_CS,
                      GPIO_PIN_RESET);
    // 명령어를 보낼 땐 DC를 high로 만들어야함.
    HAL_GPIO_WritePin(handle->GPIO_Port_DC, handle->GPIO_Pin_DC, GPIO_PIN_SET);
    // 인자로 받은 명령어를 전송함

    hal_status = HAL_SPI_Transmit(handle->hspi, buffer, 4, TIMEOUT_MS);
    if (hal_status != HAL_OK) {
        status = STATUS_TRANSMIT_FAILED;
    }

    // 전송이 끝난 후에는 DC, CS핀을 초기화함
    HAL_GPIO_WritePin(handle->GPIO_Port_CS, handle->GPIO_Pin_CS, GPIO_PIN_SET);
    HAL_GPIO_WritePin(handle->GPIO_Port_DC, handle->GPIO_Pin_DC,
                      GPIO_PIN_RESET);
    return status;
}

st7789_status_t
st7789_init_handle(st7789_handle_t *handle, SPI_HandleTypeDef *hspi,
                   GPIO_TypeDef *GPIO_Port_CS, GPIO_TypeDef *GPIO_Port_SDA,
                   GPIO_TypeDef *GPIO_Port_SCL, GPIO_TypeDef *GPIO_Port_DC,
                   GPIO_TypeDef *GPIO_Port_RST, uint16_t GPIO_Pin_CS,
                   uint16_t GPIO_Pin_SDA, uint16_t GPIO_Pin_SCL,
                   uint16_t GPIO_Pin_DC, uint16_t GPIO_Pin_RST) {
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
    return STATUS_OK;
}

st7789_status_t st7789_init_display(st7789_handle_t *handle) {
    st7789_status_t status = STATUS_OK;
    uint16_t command = SWRESET;
    uint16_t parameter = 0;

    // 1. SWRESET 전송 후, 120ms만큼 기다림
    status = st7789_send_command(handle, command, NO_PARAMETER);
    HAL_Delay(WAIT_MS);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 2. SLPOUT 전송 후, 150ms 만큼 기다림
    command = SLPOUT;
    status = st7789_send_command(handle, command, NO_PARAMETER);
    HAL_Delay(WAIT_MS);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 3. NORON 전송
    command = NORON;
    status = st7789_send_command(handle, command, NO_PARAMETER);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 4. INVOFF 전송
    command = INVOFF;
    status = st7789_send_command(handle, command, NO_PARAMETER);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 5. DISPON 전송
    command = DISPON;
    status = st7789_send_command(handle, command, NO_PARAMETER);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 6. MADCTL 전송
    command = MADCTL;
    parameter =
        PAGE_ADDRESS_ORDER_TOP_TO_BOTTOM | COLUMN_ADDRESS_ORDER_LEFT_TO_RIGHT |
        PAGE_COLUMN_ORDER_NORMAL_MODE | LINE_ADDRESS_ORDER_TOP_TO_BOTTOM |
        RGB_ORDER_BGR | DISPLAY_DATA_LATCH_ORDER_LEFT_TO_RIGHT;
    status = st7789_send_command(handle, command, parameter);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    // 7. COLMOD 전송
    command = COLMOD;
    parameter = RGB565_INTERFACE | COLOR_FORMAT_16BIT;
    status = st7789_send_command(handle, command, parameter);
    if (status != STATUS_OK) {
        // TODO: st7789 라이브러리의 오류에 대한 문서가 없으므로 예외 처리에
        // 대한 구현은 미룸
    }

    return STATUS_OK;
}

st7789_status_t st7789_print_sample_display(st7789_handle_t *handle) {
    return STATUS_OK;
}
