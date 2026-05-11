#include "task/video_player_task.h"

#include "cmsis_os.h"
#include "main.h"

void video_task_run(void const *argument) {
    video_player_task_config_t *config = (video_player_task_config_t *)argument;

    if (config == NULL || config->player_context == NULL ||
        config->shared_context == NULL || config->target_frame_rate == 0 ||
        config->frameBufferQueueHandle == NULL ||
        config->ioMutexHandle == NULL || config->lcdDmaDoneSemHandle == NULL ||
        config->sdReadDoneSemHandle == NULL) {
        Error_Handler();
    }

    for (;;) {
        osDelay(1);
    }
}
