#include "video_reader.h"

static void video_reader_switch_buffer_address(video_context_t *context);

video_context_status_t video_reader_mount(video_context_t *context,
                                          const TCHAR *sd_path) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    if (f_mount(context->sd_fatfs, sd_path, 1) != FR_OK) {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_MOUNT;
    }

    return status;
}

video_context_status_t video_reader_open_file(video_context_t *context,
                                              const TCHAR *file_path) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    if (f_open(&context->file, file_path, FA_READ) != FR_OK) {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_OPEN_FILE;
    }
    return status;
}

video_context_status_t video_reader_close_file(video_context_t *context) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;
    if (f_close(&context->file) != FR_OK) {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_CLOSE_FILE;
    }
    return status;
}

video_context_status_t video_reader_read_file(video_context_t *context) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    video_reader_switch_buffer_address(context);
    if (f_read(&context->file, context->buffer, VIDEO_CONTEXT_BUFFER_SIZE,
               &context->read_size) != FR_OK) {
        // TODO: 읽기에 실패할 경우 에러 핸들링 구현
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_READ_FILE;
    }

    return status;
}

static void video_reader_switch_buffer_address(video_context_t *context) {
    context->use_front_buffer = !context->use_front_buffer;

    if (context->use_front_buffer) {
        context->buffer = context->double_buffer;
    } else {
        context->buffer = context->double_buffer + VIDEO_CONTEXT_BUFFER_OFFSET;
    }
}