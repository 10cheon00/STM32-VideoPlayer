#include "video_context.h"

static video_buffer_t buffer_a[VIDEO_CONTEXT_BUFFER_SIZE];
static video_buffer_t buffer_b[VIDEO_CONTEXT_BUFFER_SIZE];

video_context_status_t video_context_init(video_context_t *context,
                                          FATFS *sd_fatfs,
                                          SD_HandleTypeDef *hsd,
                                          st7789_handle_t *st7789_handle) {
    context->sd_fatfs = sd_fatfs;
    context->hsd = hsd;
    context->use_buffer_a = 0;
    context->buffer = buffer_b;
    context->last_tick = HAL_GetTick();
    context->frame_per_milliseconds = 0;
    context->st7789_handle = st7789_handle;
    context->sx = 0;
    context->sy = 0;
    context->ex = context->st7789_handle->screen_width;
    context->ey = VIDEO_CONTEXT_CHUNK_OFFSET;

    return VIDEO_CONTEXT_STATUS_OK;
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

/**
 * https://www.st.com/resource/en/errata_sheet/es0287-stm32f411xcxe-device-errata-stmicroelectronics.pdf
 * 위 문서의 2.2.11절에 의하면, DMA2가 동시에 AHB와 APB2간 memory to peripherial
 * 모드로 동작하면, DMA에 의해 SDIO를 제 때 읽지 못해 동기를 맞추지 못하여 CRC
 * mismatch가 발생한다.
 * 따라서, 동기를 맞추기 위해서 강제로 DMA나 SDIO가 실행중이라면 대기하는
 * 제약사항을 걸었음
 *
 */
void video_context_wait_for_dma_and_sdio_idle(video_context_t *context) {

    while (!(context->st7789_handle->is_dma_tx_done))
        ;
}
