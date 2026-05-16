#include "video/video_reader.h"

static uint8_t
video_context_is_reached_end_of_file(video_reader_context_t *reader_context);
static video_context_status_t video_context_read_restart_from_beginning(
    video_reader_context_t *reader_context,
    video_shared_context_t *shared_context,
    video_buffer_t *buffer);

void video_reader_init(video_reader_context_t *reader_context,
                       DWORD frame_bytes, FATFS *sd_fatfs,
                       SD_HandleTypeDef *hsd) {
    reader_context->sd_fatfs = sd_fatfs;
    reader_context->hsd = hsd;
    reader_context->frame_bytes = frame_bytes;
    reader_context->bytes_read = 0;
    reader_context->file_bytes = 0;
    reader_context->max_frame_index = 0;
}

video_context_status_t
video_reader_mount(video_reader_context_t *reader_context,
                   const TCHAR *sd_path) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    if (f_mount(reader_context->sd_fatfs, sd_path, 1) != FR_OK) {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_MOUNT;
    }

    return status;
}

video_context_status_t
video_reader_open_file(video_reader_context_t *reader_context,
                       const TCHAR *file_path) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    if (f_open(&reader_context->file, file_path, FA_READ) != FR_OK) {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_OPEN_FILE;
    }

    if (status == VIDEO_CONTEXT_STATUS_OK) {
        reader_context->file_bytes = f_size(&reader_context->file);
        reader_context->max_frame_index =
            reader_context->file_bytes / reader_context->frame_bytes;
    }
    return status;
}

video_context_status_t
video_reader_close_file(video_reader_context_t *reader_context) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;
    if (f_close(&reader_context->file) != FR_OK) {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_CLOSE_FILE;
    }
    return status;
}

video_context_status_t
video_reader_read_file(video_reader_context_t *reader_context,
                       video_shared_context_t *shared_context,
                       video_buffer_t *buffer) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    FRESULT fresult = f_read(&reader_context->file, (uint8_t *)buffer,
                             VIDEO_CONTEXT_BUFFER_SIZE *
                                 sizeof(video_buffer_t),
                             &reader_context->bytes_read);
    if (fresult == FR_OK) {
        shared_context->total_bytes_read += reader_context->bytes_read;
    } else {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_READ_FILE;
    }

    if (status == VIDEO_CONTEXT_STATUS_OK) {
        if (video_context_is_reached_end_of_file(reader_context)) {
            status = video_context_read_restart_from_beginning(
                reader_context, shared_context, buffer);
        }
    }

    return status;
}

static uint8_t
video_context_is_reached_end_of_file(video_reader_context_t *reader_context) {
    return reader_context->bytes_read <= VIDEO_CONTEXT_BUFFER_SIZE;
}

static video_context_status_t video_context_read_restart_from_beginning(
    video_reader_context_t *reader_context,
    video_shared_context_t *shared_context,
    video_buffer_t *buffer) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;
    FRESULT fresult = f_lseek(&reader_context->file, 0);

    if (fresult == FR_OK) {
        shared_context->total_bytes_read = 0;
        shared_context->next_frame_start_byte = reader_context->frame_bytes;
        fresult = f_read(&reader_context->file, (uint8_t *)buffer,
                         VIDEO_CONTEXT_BUFFER_SIZE *
                             sizeof(video_buffer_t),
                         &reader_context->bytes_read);
        if (fresult == FR_OK) {
            shared_context->total_bytes_read += reader_context->bytes_read;
        } else {
            status = VIDEO_CONTEXT_STATUS_FAILED_TO_READ_FILE;
        }
    }
    return status;
}
