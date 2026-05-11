#include "task/video_reader_task.h"

#include "cmsis_os.h"
#include "main.h"

void video_reader_task_run(void const *argument) {
    video_reader_task_config_t *config = (video_reader_task_config_t *)argument;

    if (config == NULL || config->reader_context == NULL ||
        config->shared_context == NULL || config->sd_fatfs == NULL ||
        config->hsd == NULL || config->sd_path == NULL ||
        config->file_path == NULL || config->frameBufferQueueHandle == NULL ||
        config->ioMutexHandle == NULL || config->sdReadDoneSemHandle == NULL) {
        Error_Handler();
    }

    for (;;) {
        osDelay(1);
    }
}
