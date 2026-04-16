#include "video_player.h"

static void video_player_step_next_range(video_context_t *context);

video_context_status_t
video_player_print_video_buffer(video_context_t *context) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    if (st7789_print_pixels_with_range(context->st7789_handle, context->buffer,
                                       context->sx, context->sy, context->ex,
                                       context->ey) == STATUS_OK) {
        video_player_step_next_range(context);
    } else {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_PRINT_VIDEO_BUFFER_TO_SCREEN;
    }
    return status;
}

static void video_player_step_next_range(video_context_t *context) {
    context->sy += VIDEO_CONTEXT_CHUNK_OFFSET;
    context->ey += VIDEO_CONTEXT_CHUNK_OFFSET;
    if (context->ey > context->st7789_handle->screen_height) {
        context->sy = 0;
        context->ey = VIDEO_CONTEXT_CHUNK_OFFSET;
    }
}
