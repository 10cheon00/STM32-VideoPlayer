#ifndef _VIDEO_READER_H_
#define _VIDEO_READER_H_

#include "video_context.h"

typedef struct {
    FATFS *sd_fatfs;
    SD_HandleTypeDef *hsd;
    FIL file;
    UINT bytes_read;
    DWORD file_bytes;
    uint32_t max_frame_index;
} video_reader_context_t;

void video_reader_init(video_reader_context_t *reader_context,
                       FATFS *sd_fatfs,
                       SD_HandleTypeDef *hsd);

video_context_status_t video_reader_mount(video_reader_context_t *reader_context,
                                          const TCHAR *sd_path);

video_context_status_t video_reader_open_file(video_reader_context_t *reader_context,
                                              video_shared_context_t *shared_context,
                                              const TCHAR *file_path);

video_context_status_t
video_reader_close_file(video_reader_context_t *reader_context);

video_context_status_t
video_reader_read_file(video_reader_context_t *reader_context,
                       video_shared_context_t *shared_context);

#endif
