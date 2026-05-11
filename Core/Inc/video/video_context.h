#ifndef _VIDEO_CONTEXT_H_
#define _VIDEO_CONTEXT_H_
/**
 * 비디오 데이터에 대한 구조체 정보를 명시하는 헤더파일
 */
#include "fatfs.h"

typedef uint16_t video_buffer_t;
typedef struct video_player_context video_player_context_t;

#define VIDEO_CONTEXT_LINE_LENGTH 240
#define VIDEO_CONTEXT_CHUNK_OFFSET 16
#define VIDEO_CONTEXT_BUFFER_SIZE                                              \
    (VIDEO_CONTEXT_LINE_LENGTH * VIDEO_CONTEXT_CHUNK_OFFSET)

typedef enum {
    VIDEO_CONTEXT_STATUS_OK = 0,
    VIDEO_CONTEXT_STATUS_BUSY,
    VIDEO_CONTEXT_STATUS_FAILED_TO_INIT_DISPLAY,
    VIDEO_CONTEXT_STATUS_FAILED_TO_PRINT_VIDEO_BUFFER_TO_DISPLAY,
    VIDEO_CONTEXT_STATUS_FAILED_TO_MOUNT,
    VIDEO_CONTEXT_STATUS_FAILED_TO_OPEN_FILE,
    VIDEO_CONTEXT_STATUS_FAILED_TO_CLOSE_FILE,
    VIDEO_CONTEXT_STATUS_FAILED_TO_READ_FILE,
    VIDEO_CONTEXT_STATUS_FAILED_TO_PROCESS_TIMING,
} video_context_status_t;

typedef struct {
    // reader와 player가 공유하는 버퍼 및 프레임 경계 정보
    video_buffer_t *buffer;
    video_buffer_t *first_buffer;
    video_buffer_t *second_buffer;
    uint8_t use_first_buffer;
    DWORD frame_bytes;
    DWORD total_bytes_read;
    DWORD next_frame_start_byte;
} video_shared_context_t;

void video_context_init(video_shared_context_t *shared_context);

void video_context_wait_for_dma_and_sdio_idle(
    video_player_context_t *player_context);

void video_context_step_next_range(video_player_context_t *player_context);

video_context_status_t
video_context_process_frame_timing(video_player_context_t *player_context,
                                   video_shared_context_t *shared_context);

#endif
