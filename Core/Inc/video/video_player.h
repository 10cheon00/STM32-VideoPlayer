#ifndef _VIDEO_PLAYER_H_
#define _VIDEO_PLAYER_H_

#include "video_context.h"
#include "lcd/st7789.h"

struct video_player_context {
    st7789_handle_t *st7789_handle;
    uint16_t sx;
    uint16_t sy;
    uint16_t ex;
    uint16_t ey;
    uint32_t current_frame_rate;
    uint32_t last_tick;
    uint32_t next_frame_tick;
    uint32_t target_frame_rate;
    uint8_t deadline_missed_count;
};

video_context_status_t
video_player_init(video_player_context_t *player_context,
                  video_shared_context_t *shared_context,
                  st7789_handle_t *st7789_handle,
                  uint32_t target_frame_rate);

video_context_status_t
video_player_print_video_buffer(video_player_context_t *player_context,
                                video_shared_context_t *shared_context);

#endif
