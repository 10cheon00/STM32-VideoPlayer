#ifndef _ERROR_HANDLER_H_
#define _ERROR_HANDLER_H_

#include "stm32f4xx.h"

#include "video_context.h"

typedef enum {
    ERROR_HANDLER_NO_ERROR = 0,
    ERROR_HANDLER_INTERNAL_LOGIC_FAILURE,
    ERROR_HANDLER_RESOURCE_UNAVAILABLE,
    ERROR_HANDLER_ST7789_INIT_FAILURE,
    ERROR_HANDLER_ST7789_DISPLAY_FAILURE,
    ERROR_HANDLER_MICROSD_INIT_FAILURE,
    ERROR_HANDLER_MICROSD_FILE_SEARCH_FAILURE,
    ERROR_HANDLER_MICROSD_FILE_READ_FAILURE,
    ERROR_HANDLER_MICROSD_FILE_POINTER_FAILURE,
} error_handler_error_code_t;

typedef struct {
    GPIO_TypeDef* GPIO_Port_LED;
    uint16_t GPIO_Pin_LED;

    st7789_handle_t* st7789_handle;
} error_handler_handle_t;

void error_handler_init_handle(error_handler_handle_t* handle, GPIO_TypeDef *GPIO_Port_LED, uint16_t GPIO_Pin_LED, st7789_handle_t *st7789_handle); 

error_handler_error_code_t error_handler_get_error_code(video_context_status_t status);

void error_handler_handle_error(error_handler_handle_t* handle, error_handler_error_code_t error_code);

#endif
