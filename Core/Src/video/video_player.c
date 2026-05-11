#include "video/video_player.h"

video_context_status_t
video_player_init(video_player_context_t *player_context,
                  video_shared_context_t *shared_context,
                  st7789_handle_t *st7789_handle,
                  uint32_t target_frame_rate) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    player_context->st7789_handle = st7789_handle;
    player_context->sx = 0;
    player_context->sy = 0;
    player_context->ex = player_context->st7789_handle->screen_width;
    player_context->ey = VIDEO_CONTEXT_CHUNK_OFFSET;
    player_context->last_tick = HAL_GetTick();
    player_context->current_frame_rate = 0;
    player_context->next_frame_tick = 0;
    player_context->target_frame_rate = target_frame_rate;
    player_context->deadline_missed_count = 0;

    shared_context->frame_bytes =
        player_context->st7789_handle->screen_width *
        player_context->st7789_handle->screen_height * sizeof(video_buffer_t);

    if (st7789_init_display(player_context->st7789_handle) != ST7789_STATUS_OK) {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_INIT_DISPLAY;
    }
    return status;
}

video_context_status_t
video_player_print_video_buffer(video_player_context_t *player_context,
                                video_shared_context_t *shared_context) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    if (st7789_print_pixels_with_range(
            player_context->st7789_handle, shared_context->buffer,
            player_context->sx, player_context->sy, player_context->ex,
            player_context->ey) == ST7789_STATUS_OK) {
        video_context_step_next_range(player_context);
    } else {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_PRINT_VIDEO_BUFFER_TO_DISPLAY;
    }
    return status;
}
