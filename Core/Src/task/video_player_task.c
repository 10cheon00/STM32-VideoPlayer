#include "task/video_player_task.h"

#include "cmsis_os.h"
#include "main.h"

static st7789_handle_t st7789_handle;
static osMessageQId writableBufferQueueHandle;
static video_buffer_t *buffer = NULL;
static osThreadId this = 0;

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        BaseType_t hpw = pdFALSE;
        st7789_dma_tx_cplt_callback(&st7789_handle);
        vTaskNotifyGiveFromISR(this, &hpw);
        portYIELD_FROM_ISR(hpw);
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        Error_Handler();
    }
}

void video_player_task_run(void const *argument) {
    video_player_task_config_t *config = (video_player_task_config_t *)argument;
    if (config == NULL || config->player_context == NULL ||
        config->shared_context == NULL || config->hspi == NULL ||
        config->GPIO_Port_CS == NULL || config->GPIO_Port_DC == NULL ||
        config->GPIO_Port_RST == NULL || config->target_frame_rate == 0 ||
        config->writableBufferQueueHandle == NULL ||
        config->printableBufferQueueHandle == NULL ||
        config->VideoPlayerTaskHandle == NULL) {
        Error_Handler();
    }
    writableBufferQueueHandle = config->writableBufferQueueHandle;
    this = config->VideoPlayerTaskHandle;

    if (st7789_init_handle(
            &st7789_handle, config->hspi, config->GPIO_Port_CS,
            config->GPIO_Port_DC, config->GPIO_Port_RST, config->GPIO_Pin_CS,
            config->GPIO_Pin_DC, config->GPIO_Pin_RST, config->screen_width,
            config->screen_height, config->dma_status) != ST7789_STATUS_OK) {
        Error_Handler();
    }

    if (video_player_init(config->player_context, &st7789_handle,
                          config->target_frame_rate) !=
        VIDEO_CONTEXT_STATUS_OK) {
        Error_Handler();
    }

    if (st7789_print_sample_display(&st7789_handle) != ST7789_STATUS_OK) {
        Error_Handler();
    }

    for (;;) {
        xQueueReceive(config->printableBufferQueueHandle, &buffer,
                      portMAX_DELAY);

        if (video_player_print_video_buffer(config->player_context, buffer) !=
            VIDEO_CONTEXT_STATUS_OK) {
            Error_Handler();
        }
        if (video_player_process_frame_timing(config->player_context,
                                              config->shared_context) !=
            VIDEO_CONTEXT_STATUS_OK) {
            Error_Handler();
        }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (xQueueSend(writableBufferQueueHandle, &buffer, portMAX_DELAY) != pdTRUE) {
            Error_Handler();
        }
    }
}
