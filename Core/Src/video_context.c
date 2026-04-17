#include "video_context.h"

static video_buffer_t first_buffer[VIDEO_CONTEXT_BUFFER_SIZE];
static video_buffer_t second_buffer[VIDEO_CONTEXT_BUFFER_SIZE];

static void video_context_update_video_meta_data(video_context_t *context);
static void
video_context_calculate_current_frame_rate(video_context_t *context);
static void video_context_calculate_next_frame_tick(video_context_t *context);

video_context_status_t video_context_init(video_context_t *context,
                                          FATFS *sd_fatfs,
                                          SD_HandleTypeDef *hsd,
                                          st7789_handle_t *st7789_handle,
                                          uint32_t target_frame_rate) {
    context->sd_fatfs = sd_fatfs;
    context->hsd = hsd;
    context->use_first_buffer = 0;
    context->first_buffer = first_buffer;
    context->second_buffer = second_buffer;
    context->buffer = context->first_buffer;

    context->st7789_handle = st7789_handle;
    context->sx = 0;
    context->sy = 0;
    context->ex = context->st7789_handle->screen_width;
    context->ey = VIDEO_CONTEXT_CHUNK_OFFSET;

    context->last_tick = HAL_GetTick();
    context->current_frame_rate = 0;
    context->target_frame_rate = target_frame_rate;

    return VIDEO_CONTEXT_STATUS_OK;
}

/**
 * https://www.st.com/resource/en/errata_sheet/es0287-stm32f411xcxe-device-errata-stmicroelectronics.pdf
 * 위 문서의 2.2.11절에 의하면, DMA2가 동시에 AHB와 APB2간 memory to
 * peripherial 모드로 동작하면, DMA에 의해 SDIO를 제 때 읽지 못해 동기를
 * 맞추지 못하여 CRC mismatch가 발생한다. 따라서, 동기를 맞추기 위해서
 * 강제로 DMA나 SDIO가 실행중이라면 대기하는 제약사항을 걸었음
 *
 */
void video_context_wait_for_dma_and_sdio_idle(video_context_t *context) {

    while (!(context->st7789_handle->is_dma_tx_done))
        ;
}

void video_context_step_next_range(video_context_t *context) {
    context->sy += VIDEO_CONTEXT_CHUNK_OFFSET;
    context->ey += VIDEO_CONTEXT_CHUNK_OFFSET;
    if (context->ey > context->st7789_handle->screen_height) {
        context->sy = 0;
        context->ey = VIDEO_CONTEXT_CHUNK_OFFSET;

        video_context_update_video_meta_data(context);
    }
}

void video_context_update_video_meta_data(video_context_t *context) {
    video_context_calculate_current_frame_rate(context);
    video_context_calculate_next_frame_tick(context);
}

static void
video_context_calculate_current_frame_rate(video_context_t *context) {
    // 프레임레이트 계산
    // 1초동안 보낸 프레임 개수...는 정수 단위임
    // 프레임 시간차 : 1 = 1000 : 프레임 레이트
    // 프레임 레이트 * 1000 = 1000000 / 프레임 시간차
    uint32_t tick = HAL_GetTick();
    uint32_t tick_diff = tick - context->last_tick;

    context->last_tick = tick;
    if (tick_diff > 0) {
        context->current_frame_rate = 1000000U / tick_diff;
    }
}

static void video_context_calculate_next_frame_tick(video_context_t *context) {
    context->next_frame_tick = HAL_GetTick() + 1000U / context->target_frame_rate;
}
