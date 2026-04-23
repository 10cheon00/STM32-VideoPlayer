#include "error_handler.h"

#define RGB565_COLOR_RED 0x00F8
#define RGB565_COLOR_YELLOW 0xE0FF
#define RGB565_COLOR_GREEN 0xE007
#define RGB565_COLOR_BLUE 0x1F00

static uint16_t pixels[240];

static void error_handler_enable_led(error_handler_handle_t *handle);
static void error_handler_disable_led(error_handler_handle_t *handle);
static void error_handler_toggle_led(error_handler_handle_t *handle);

void error_handler_init_handle(error_handler_handle_t *handle,
                               GPIO_TypeDef *GPIO_Port_LED,
                               uint16_t GPIO_Pin_LED,
                               st7789_handle_t *st7789_handle) {
    handle->GPIO_Port_LED = GPIO_Port_LED;
    handle->GPIO_Pin_LED = GPIO_Pin_LED;
    handle->st7789_handle = st7789_handle;

    error_handler_disable_led(handle);
}

error_handler_error_code_t
error_handler_get_error_code(video_context_status_t status) {
    error_handler_error_code_t error_code = ERROR_HANDLER_NO_ERROR;
    switch (status) {
    case VIDEO_CONTEXT_STATUS_FAILED_TO_INIT_DISPLAY:
        error_code = ERROR_HANDLER_ST7789_INIT_FAILURE;
        break;
    case VIDEO_CONTEXT_STATUS_FAILED_TO_PRINT_VIDEO_BUFFER_TO_DISPLAY:
        error_code = ERROR_HANDLER_ST7789_DISPLAY_FAILURE;
        break;
    case VIDEO_CONTEXT_STATUS_FAILED_TO_MOUNT:
        error_code = ERROR_HANDLER_MICROSD_INIT_FAILURE;
        break;
    case VIDEO_CONTEXT_STATUS_FAILED_TO_OPEN_FILE:
        error_code = ERROR_HANDLER_MICROSD_FILE_SEARCH_FAILURE;
        break;
    case VIDEO_CONTEXT_STATUS_FAILED_TO_READ_FILE:
        error_code = ERROR_HANDLER_MICROSD_FILE_READ_FAILURE;
        break;
    case VIDEO_CONTEXT_STATUS_FAILED_TO_PROCESS_TIMING:
        error_code = ERROR_HANDLER_MICROSD_FILE_POINTER_FAILURE;
        break;
    case VIDEO_CONTEXT_STATUS_FAILED_TO_CLOSE_FILE:
    default:
        error_code = ERROR_HANDLER_NO_ERROR;
    }
    return error_code;
}

void error_handler_handle_error(error_handler_handle_t *handle,
                                error_handler_error_code_t error_code) {
    uint32_t last_tick = HAL_GetTick();
    uint32_t next_toggle_tick = last_tick;

    error_handler_disable_led(handle);

    while (1) {
        last_tick = HAL_GetTick();

        if (error_code == ERROR_HANDLER_INTERNAL_LOGIC_FAILURE) {
            error_handler_enable_led(handle);
        } else if (error_code == ERROR_HANDLER_RESOURCE_UNAVAILABLE) {
            if (next_toggle_tick <= last_tick) {
                next_toggle_tick = last_tick + 5000;
                error_handler_toggle_led(handle);
            }
        } else if (error_code == ERROR_HANDLER_ST7789_INIT_FAILURE) {
            if (next_toggle_tick <= last_tick) {
                next_toggle_tick = last_tick + 500;
                error_handler_toggle_led(handle);
            }
        } else if (error_code == ERROR_HANDLER_ST7789_DISPLAY_FAILURE) {
            if (next_toggle_tick <= last_tick) {
                next_toggle_tick = last_tick + 1000;
                error_handler_toggle_led(handle);
            }
        } else if (error_code == ERROR_HANDLER_MICROSD_INIT_FAILURE) {
            if (next_toggle_tick <= last_tick) {
                next_toggle_tick = last_tick + 2000;
                error_handler_toggle_led(handle);
                // TODO:
                // 아래 코드는 ST7789가 초기화되었음을 전제하고 진행했음
                // 초기화 되지 않은 상황을 가정하기
                for (uint8_t i = 0; i < 240; i++) {
                    pixels[i] = RGB565_COLOR_RED;
                }
                for (uint8_t y = 0; y < 240; y++) {
                    st7789_print_pixels_with_range(
                        handle->st7789_handle, pixels, 0, y,
                        handle->st7789_handle->screen_width, y + 1);
                }
            }
        } else if (error_code == ERROR_HANDLER_MICROSD_FILE_SEARCH_FAILURE) {
            if (next_toggle_tick <= last_tick) {
                next_toggle_tick = last_tick + 2000;
                error_handler_toggle_led(handle);
                // TODO:
                // 아래 코드는 ST7789가 초기화되었음을 전제하고 진행했음
                // 초기화 되지 않은 상황을 가정하기
                for (uint8_t i = 0; i < 240; i++) {
                    pixels[i] = RGB565_COLOR_YELLOW;
                }
                for (uint8_t y = 0; y < 240; y++) {
                    st7789_print_pixels_with_range(
                        handle->st7789_handle, pixels, 0, y,
                        handle->st7789_handle->screen_width, y + 1);
                }
            }
        } else if (error_code == ERROR_HANDLER_MICROSD_FILE_READ_FAILURE) {
            if (next_toggle_tick <= last_tick) {
                next_toggle_tick = last_tick + 2000;
                error_handler_toggle_led(handle);
                // TODO:
                // 아래 코드는 ST7789가 초기화되었음을 전제하고 진행했음
                // 초기화 되지 않은 상황을 가정하기
                for (uint8_t i = 0; i < 240; i++) {
                    pixels[i] = RGB565_COLOR_GREEN;
                }
                for (uint8_t y = 0; y < 240; y++) {
                    st7789_print_pixels_with_range(
                        handle->st7789_handle, pixels, 0, y,
                        handle->st7789_handle->screen_width, y + 1);
                }
            }
        } else if (error_code == ERROR_HANDLER_MICROSD_FILE_POINTER_FAILURE) {
            if (next_toggle_tick <= last_tick) {
                next_toggle_tick = last_tick + 2000;
                error_handler_toggle_led(handle);
                // TODO:
                // 아래 코드는 ST7789가 초기화되었음을 전제하고 진행했음
                // 초기화 되지 않은 상황을 가정하기
                for (uint8_t i = 0; i < 240; i++) {
                    pixels[i] = RGB565_COLOR_BLUE;
                }
                for (uint8_t y = 0; y < 240; y++) {
                    st7789_print_pixels_with_range(
                        handle->st7789_handle, pixels, 0, y,
                        handle->st7789_handle->screen_width, y + 1);
                }
            }
        }
    }
}

void error_handler_enable_led(error_handler_handle_t *handle) {
    HAL_GPIO_WritePin(handle->GPIO_Port_LED, handle->GPIO_Pin_LED,
                      GPIO_PIN_RESET);
}

void error_handler_disable_led(error_handler_handle_t *handle) {
    HAL_GPIO_WritePin(handle->GPIO_Port_LED, handle->GPIO_Pin_LED,
                      GPIO_PIN_SET);
}

void error_handler_toggle_led(error_handler_handle_t *handle) {
    HAL_GPIO_TogglePin(handle->GPIO_Port_LED, handle->GPIO_Pin_LED);
}
