#include "error_handler.h"


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

void error_handler_handle_error(error_handler_error_code_t error_code) {
    if (error_code == ERROR_HANDLER_ST7789_INIT_FAILURE) {
    }
}
