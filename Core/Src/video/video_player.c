#include "video/video_player.h"

static void video_player_step_next_range(
    video_player_context_t *player_context);
static void
video_player_update_video_meta_data(video_player_context_t *player_context,
                                    video_shared_context_t *shared_context);
static void video_player_calculate_current_frame_rate(
    video_player_context_t *player_context);
static void
video_player_calculate_next_frame_tick(video_player_context_t *player_context);
static void video_player_update_next_frame_start_byte(
    video_player_context_t *player_context,
    video_shared_context_t *shared_context);
static uint8_t video_player_is_frame_print_complete(
    video_shared_context_t *shared_context);
static uint8_t video_player_is_next_frame_deadline_missed(
    video_player_context_t *player_context);

video_context_status_t
video_player_init(video_player_context_t *player_context,
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
    player_context->frame_bytes =
        player_context->st7789_handle->screen_width *
        player_context->st7789_handle->screen_height * sizeof(video_buffer_t);

    if (st7789_init_display(player_context->st7789_handle) != ST7789_STATUS_OK) {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_INIT_DISPLAY;
    }
    return status;
}

video_context_status_t
video_player_print_video_buffer(video_player_context_t *player_context,
                                video_buffer_t *buffer) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    if (st7789_print_pixels_with_range(
            player_context->st7789_handle, buffer,
            player_context->sx, player_context->sy, player_context->ex,
            player_context->ey) == ST7789_STATUS_OK) {
        video_player_step_next_range(player_context);
    } else {
        status = VIDEO_CONTEXT_STATUS_FAILED_TO_PRINT_VIDEO_BUFFER_TO_DISPLAY;
    }
    return status;
}

/**
 * https://www.st.com/resource/en/errata_sheet/es0287-stm32f411xcxe-device-errata-stmicroelectronics.pdf
 * 위 문서의 2.2.11절에 의하면, DMA2가 동시에 AHB와 APB2간 memory to
 * peripherial 모드로 동작하면, DMA에 의해 SDIO를 제 때 읽지 못해 동기를
 * 맞추지 못하여 CRC mismatch가 발생한다. 따라서, 동기를 맞추기 위해서
 * 강제로 DMA나 SDIO가 실행중이라면 대기하는 제약사항을 걸었음
 *
 */
void video_player_wait_for_dma_and_sdio_idle(
    video_player_context_t *player_context) {
    while (!(player_context->st7789_handle->is_dma_tx_done))
        ;
}

video_context_status_t
video_player_process_frame_timing(video_player_context_t *player_context,
                                  video_shared_context_t *shared_context) {
    video_context_status_t status = VIDEO_CONTEXT_STATUS_OK;

    if (video_player_is_frame_print_complete(shared_context)) {
        while (HAL_GetTick() < player_context->next_frame_tick)
            ;
        video_player_update_video_meta_data(player_context, shared_context);
    }

    if (video_player_is_next_frame_deadline_missed(player_context)) {
        player_context->deadline_missed_count++;
        if (player_context->deadline_missed_count > 10) {
            status = VIDEO_CONTEXT_STATUS_FAILED_TO_PROCESS_TIMING;
        }
    }
    return status;
}

static void
video_player_step_next_range(video_player_context_t *player_context) {
    player_context->sy += VIDEO_CONTEXT_CHUNK_OFFSET;
    player_context->ey += VIDEO_CONTEXT_CHUNK_OFFSET;
    if (player_context->ey > player_context->st7789_handle->screen_height) {
        player_context->sy = 0;
        player_context->ey = VIDEO_CONTEXT_CHUNK_OFFSET;
    }
}

static void
video_player_update_video_meta_data(video_player_context_t *player_context,
                                    video_shared_context_t *shared_context) {
    video_player_calculate_current_frame_rate(player_context);
    video_player_calculate_next_frame_tick(player_context);
    video_player_update_next_frame_start_byte(player_context, shared_context);
}

static void video_player_calculate_current_frame_rate(
    video_player_context_t *player_context) {
    uint32_t tick = HAL_GetTick();
    uint32_t tick_diff = tick - player_context->last_tick;

    player_context->last_tick = tick;
    if (tick_diff > 0) {
        player_context->current_frame_rate = 1000000U / tick_diff;
    }
}

static void
video_player_calculate_next_frame_tick(video_player_context_t *player_context) {
    player_context->next_frame_tick =
        HAL_GetTick() + 1000U / player_context->target_frame_rate;
}

static void video_player_update_next_frame_start_byte(
    video_player_context_t *player_context,
    video_shared_context_t *shared_context) {
    shared_context->next_frame_start_byte =
        shared_context->total_bytes_read + player_context->frame_bytes;
}

static uint8_t video_player_is_frame_print_complete(
    video_shared_context_t *shared_context) {
    return shared_context->total_bytes_read >=
           shared_context->next_frame_start_byte;
}

static uint8_t video_player_is_next_frame_deadline_missed(
    video_player_context_t *player_context) {
    return HAL_GetTick() >= player_context->next_frame_tick;
}
