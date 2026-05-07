#ifndef _VIDEO_READER_H_
#define _VIDEO_READER_H_

#include "video_context.h"

video_context_status_t video_reader_mount(video_context_t *context,
                                          const TCHAR *sd_path);

video_context_status_t video_reader_open_file(video_context_t *context,
                                              const TCHAR *file_path);

video_context_status_t video_reader_close_file(video_context_t *context);

video_context_status_t video_reader_read_file(video_context_t *context);

#endif
