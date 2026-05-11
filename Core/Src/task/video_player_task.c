#include "task/video_player_task.h"

#include "cmsis_os.h"
#include "main.h"

static st7789_handle_t *st7789_handle_ref;
static osSemaphoreId lcd_dma_done_sem_handle;

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1 && st7789_handle_ref != NULL) {
        st7789_dma_tx_cplt_callback(st7789_handle_ref);
        if (lcd_dma_done_sem_handle != NULL) {
            osSemaphoreRelease(lcd_dma_done_sem_handle);
        }
    }
}

void video_player_task_run(void const *argument) {
    video_player_task_config_t *config = (video_player_task_config_t *)argument;

    if (config == NULL || config->player_context == NULL ||
        config->shared_context == NULL || config->st7789_handle == NULL ||
        config->hspi == NULL || config->GPIO_Port_CS == NULL ||
        config->GPIO_Port_DC == NULL || config->GPIO_Port_RST == NULL ||
        config->target_frame_rate == 0 ||
        config->frameBufferQueueHandle == NULL ||
        config->ioMutexHandle == NULL || config->lcdDmaDoneSemHandle == NULL ||
        config->sdReadDoneSemHandle == NULL) {
        Error_Handler();
    }

    lcd_dma_done_sem_handle = config->lcdDmaDoneSemHandle;
    st7789_handle_ref = config->st7789_handle;
    config->player_context->st7789_handle = config->st7789_handle;

    if (st7789_init_handle(
            config->st7789_handle, config->hspi, config->GPIO_Port_CS,
            config->GPIO_Port_DC, config->GPIO_Port_RST, config->GPIO_Pin_CS,
            config->GPIO_Pin_DC, config->GPIO_Pin_RST, config->screen_width,
            config->screen_height, config->dma_status) != ST7789_STATUS_OK) {
        Error_Handler();
    }

    if (st7789_init_display(config->st7789_handle) != ST7789_STATUS_OK) {
        Error_Handler();
    }

    if (st7789_print_sample_display(config->st7789_handle) !=
        ST7789_STATUS_OK) {
        Error_Handler();
    }

    for (;;) {
        osDelay(1);
    }
}
