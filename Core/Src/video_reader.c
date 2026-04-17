#include "video_reader.h"

static void video_context_switch_buffer_address(video_context_t *context);
static uint8_t video_context_is_reached_end_of_file(video_context_t *context);
static video_context_status_t
video_context_read_restart_from_beginning(video_context_t *context);

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

    if (status == VIDEO_CONTEXT_STATUS_OK) {
        context->file_size = f_size(&context->file);
        uint32_t frame_size = context->st7789_handle->screen_width *
                              context->st7789_handle->screen_width;
        context->max_frame_index = context->file_size / frame_size;
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
    // TODO: 현재 프레임 출력을 마쳤다면 다음 프레임 출력 시간까지 기다리기
    // 또는 현재 프레임 출력 중에 다음 프레임 출력 시작 시간이 도래하면 다음
    // 프레임으로 넘어가기

    video_context_switch_buffer_address(context);
    FRESULT fresult = f_read(&context->file, context->buffer,
                             VIDEO_CONTEXT_BUFFER_SIZE * sizeof(video_buffer_t),
                             &context->read_size);
    if (fresult == FR_OK) {
        context->total_read_size += context->read_size;
    } else {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_READ_FILE;
    }

    if (status == VIDEO_CONTEXT_STATUS_OK) {
        // 파일 끝에 도달할 경우 파일 포인터를 이동시킨 후 다시 파일 읽기 수행
        if (video_context_is_reached_end_of_file(context)) {
            status = video_context_read_restart_from_beginning(context);
        }
    }

    return status;
}

static void video_context_switch_buffer_address(video_context_t *context) {
    context->use_first_buffer = !context->use_first_buffer;

    if (context->use_first_buffer) {
        context->buffer = context->first_buffer;
    } else {
        context->buffer = context->second_buffer;
    }
}

static uint8_t video_context_is_reached_end_of_file(video_context_t *context) {
    return context->read_size <= VIDEO_CONTEXT_BUFFER_SIZE;
}

static video_context_status_t
video_context_read_restart_from_beginning(video_context_t *context) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;
    FRESULT fresult = f_lseek(&context->file, 0);

    if (fresult == FR_OK) {
        context->total_read_size = 0;
        fresult = f_read(&context->file, context->buffer,
                         VIDEO_CONTEXT_BUFFER_SIZE * sizeof(video_buffer_t),
                         &context->read_size);
        if (fresult == FR_OK) {
            context->total_read_size += context->read_size;
        } else {
            status = VIDEO_CONTEXT_STATUS_FAILED_TO_READ_FILE;
        }
    }
    return status;
}