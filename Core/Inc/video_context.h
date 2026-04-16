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
} video_context_status_t;

typedef struct {
    // 비디오 파일 읽기를 위한 멤버들
    FATFS *sd_fatfs;
    SD_HandleTypeDef *hsd;
    FIL file;
    video_buffer_t *buffer;
    uint8_t use_buffer_a;
    UINT read_size;
    uint32_t frame_per_milliseconds;
    uint32_t last_tick;

    // 비디오 파일 재생을 위한 멤버들
    st7789_handle_t *st7789_handle;
    uint16_t sx;
    uint16_t sy;
    uint16_t ex;
    uint16_t ey;
} video_context_t;

video_context_status_t video_context_init(video_context_t *context, FATFS *sd_fatfs,
                        SD_HandleTypeDef *hsd, st7789_handle_t *st7789_handle);

void video_context_switch_buffer_address(video_context_t *context);

void video_context_calculate_frame_per_milliseconds(video_context_t *context);

void video_context_wait_for_dma_and_sdio_idle(video_context_t *context);

#endif