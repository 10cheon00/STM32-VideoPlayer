#include "video_context.h"

void video_context_init(video_context_t *context, FATFS *sd_fatfs) {
    context->sd_fatfs = sd_fatfs;
    context->use_front_buffer = 1;
    context->buffer = context->double_buffer;
    context->last_tick = HAL_GetTick();
    context->frame_per_milliseconds = 0;
}
