#include "task/video_reader_task.h"

#include "cmsis_os.h"
#include "main.h"

void video_reader_task_run(void const *argument) {
    video_reader_task_config_t *config = (video_reader_task_config_t *)argument;
    video_buffer_t *buffer;

    if (config == NULL || config->reader_context == NULL ||
        config->shared_context == NULL || config->sd_fatfs == NULL ||
        config->hsd == NULL || config->sd_path == NULL ||
        config->file_path == NULL || config->frameBufferQueueHandle == NULL ||
        config->ioMutexHandle == NULL || config->sdReadDoneSemHandle == NULL ||
        config->frame_bytes == 0) {
        Error_Handler();
    }

    osDelay(100);
    video_context_init(config->shared_context);
    video_reader_init(config->reader_context, config->frame_bytes,
                      config->sd_fatfs, config->hsd);

    if (video_reader_mount(config->reader_context, config->sd_path) !=
        VIDEO_CONTEXT_STATUS_OK) {
        Error_Handler();
    }

    if (video_reader_open_file(config->reader_context, config->file_path) !=
        VIDEO_CONTEXT_STATUS_OK) {
        Error_Handler();
    }

    osStatus os_status = osOK;

    for (;;) {
        if (osSemaphoreWait(config->sdReadDoneSemHandle, osWaitForever) !=
            osOK) {
            Error_Handler();
        }
        if (video_reader_read_file(config->reader_context,
                                   config->shared_context) !=
            VIDEO_CONTEXT_STATUS_OK) {
            Error_Handler();
        }
        buffer = config->reader_context->use_first_buffer
                     ? config->reader_context->first_buffer
                     : config->reader_context->second_buffer;
        xQueueSend(config->frameBufferQueueHandle, &buffer, osWaitForever);
        if ((os_status = osSemaphoreRelease(config->lcdDmaDoneSemHandle)) !=
            osOK) {
            Error_Handler();
        }
    }
}
