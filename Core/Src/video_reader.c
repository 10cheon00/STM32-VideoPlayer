#include "video_reader.h"

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

    video_context_switch_buffer_address(context);
    FRESULT fresult;
    if ((fresult = f_read(&context->file, context->buffer,
                          VIDEO_CONTEXT_BUFFER_SIZE * sizeof(video_buffer_t),
                          &context->read_size)) != FR_OK) {
        // TODO: 읽기에 실패할 경우 에러 핸들링 구현
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_READ_FILE;
    }

    return status;
}
