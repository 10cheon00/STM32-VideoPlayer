#include "video_player.h"


video_context_status_t
video_player_print_video_buffer(video_context_t *context) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    if (st7789_print_pixels_with_range(context->st7789_handle, context->buffer,
                                       context->sx, context->sy, context->ex,
                                       context->ey) == ST7789_STATUS_OK) {
        video_context_step_next_range(context);
    } else {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_PRINT_VIDEO_BUFFER_TO_SCREEN;
    }
    return status;
}

