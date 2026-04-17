#ifndef _VIDEO_CONTEXT_H_
#define _VIDEO_CONTEXT_H_
/**
 * 비디오 데이터에 대한 구조체 정보를 명시하는 헤더파일
 */
#include "fatfs.h"

#include "st7789.h"

typedef uint16_t video_buffer_t;

#define VIDEO_CONTEXT_LINE_LENGTH 240
#define VIDEO_CONTEXT_CHUNK_OFFSET 16
#define VIDEO_CONTEXT_BUFFER_SIZE                                              \
    (VIDEO_CONTEXT_LINE_LENGTH * VIDEO_CONTEXT_CHUNK_OFFSET)

typedef enum {
    VIDEO_CONTEXT_STATUS_OK = 0,
    VIDEO_CONTEXT_STATUS_BUSY,
    VIDEO_CONTEXT_STATUS_FAILED_TO_MOUNT,
    VIDEO_CONTEXT_STATUS_FAILED_TO_OPEN_FILE,
    VIDEO_CONTEXT_STATUS_FAILED_TO_CLOSE_FILE,
    VIDEO_CONTEXT_STATUS_FAILED_TO_READ_FILE,
    VIDEO_CONTEXT_STATUS_FAILED_TO_PRINT_VIDEO_BUFFER_TO_SCREEN,
    VIDEO_CONTEXT_STATUS_FAILED_TO_PROCESS_TIMING,
} video_context_status_t;

typedef struct {
    // 비디오 파일 읽기를 위한 멤버들
    FATFS *sd_fatfs;
    SD_HandleTypeDef *hsd;
    FIL file;
    video_buffer_t *buffer;
    video_buffer_t *first_buffer;
    video_buffer_t *second_buffer;
    uint8_t use_first_buffer;
    UINT bytes_read;
    DWORD file_bytes;
    DWORD frame_bytes;
    DWORD total_bytes_read;
    DWORD next_frame_start_byte;

    // 비디오 파일 재생을 위한 멤버들
    st7789_handle_t *st7789_handle;
    uint16_t sx;
    uint16_t sy;
    uint16_t ex;
    uint16_t ey;

    // 비디오 재생에 관련된 메타 데이터들
    uint32_t current_frame_rate;
    uint32_t last_tick;
    uint32_t next_frame_tick;
    uint32_t target_frame_rate;

    uint32_t max_frame_index;
} video_context_t;

video_context_status_t video_context_init(video_context_t *context,
                                          FATFS *sd_fatfs,
                                          SD_HandleTypeDef *hsd,
                                          st7789_handle_t *st7789_handle,
                                          uint32_t target_frame_rate);

void video_context_wait_for_dma_and_sdio_idle(video_context_t *context);

void video_context_step_next_range(video_context_t *context);

video_context_status_t
video_context_process_frame_timing(video_context_t *context);

#endif
