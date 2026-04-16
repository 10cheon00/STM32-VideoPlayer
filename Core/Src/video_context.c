#include "video_context.h"

static video_buffer_t buffer_a[VIDEO_CONTEXT_BUFFER_SIZE];
static video_buffer_t buffer_b[VIDEO_CONTEXT_BUFFER_SIZE];

void video_context_init(video_context_t *context, FATFS *sd_fatfs) {
    context->sd_fatfs = sd_fatfs;
    context->use_buffer_a = 0;
    context->buffer = buffer_b;
    context->last_tick = HAL_GetTick();
    context->frame_per_milliseconds = 0;
}

void video_context_switch_buffer_address(video_context_t *context) {
    context->use_buffer_a = !context->use_buffer_a;

    if (context->use_buffer_a) {
        context->buffer = buffer_a;
    } else {
        context->buffer = buffer_b;
    }
}

void video_context_calculate_frame_per_milliseconds(video_context_t *context) {
    // 프레임레이트 계산
    // 1초동안 보낸 프레임 개수...는 정수 단위임
    // 프레임 시간차 : 1 = 1000 : 프레임 레이트
    // 프레임 레이트 * 1000 = 1000000 / 프레임 시간차
    uint32_t tick = HAL_GetTick();
    uint32_t tick_diff = tick - context->last_tick;

    context->last_tick = tick;
    if (tick_diff > 0) {
        context->frame_per_milliseconds = 1000000U / tick_diff;
    }
}