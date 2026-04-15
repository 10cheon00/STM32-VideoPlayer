#ifndef _VIDEO_CONTEXT_H_
#define _VIDEO_CONTEXT_H_
/**
 * 비디오 데이터에 대한 구조체 정보를 명시하는 헤더파일
 */
#include "fatfs.h"

typedef uint16_t video_buffer_t;

#define VIDEO_CONTEXT_LINE_LENGTH 240
#define VIDEO_CONTEXT_CHUNK_OFFSET 16
#define VIDEO_CONTEXT_BUFFER_OFFSET                                            \
    (VIDEO_CONTEXT_LINE_LENGTH * VIDEO_CONTEXT_CHUNK_OFFSET)
#define VIDEO_CONTEXT_BUFFER_SIZE                                              \
    (VIDEO_CONTEXT_LINE_LENGTH * VIDEO_CONTEXT_CHUNK_OFFSET *                  \
     sizeof(video_buffer_t))

typedef enum {
    VIDEO_CONTEXT_STATUS_OK = 0,
    VIDEO_CONTEXT_STATUS_FAILED_TO_MOUNT,
    VIDEO_CONTEXT_STATUS_FAILED_TO_OPEN_FILE,
    VIDEO_CONTEXT_STATUS_FAILED_TO_CLOSE_FILE,
    VIDEO_CONTEXT_STATUS_FAILED_TO_READ_FILE,
} video_context_status_t;

typedef struct {
    FATFS *sd_fatfs;
    FIL file;
    video_buffer_t double_buffer[VIDEO_CONTEXT_BUFFER_SIZE];
    uint16_t *buffer;
    uint8_t use_front_buffer;
    UINT read_size;
    uint32_t frame_per_milliseconds;
    uint32_t last_tick;
} video_context_t;

void video_context_init(video_context_t *context, FATFS *sd_fatfs);

#endif