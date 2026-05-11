#ifndef _VIDEO_TASK_H_
#define _VIDEO_TASK_H_

#include "cmsis_os.h"
#include "video/video_player.h"

typedef struct {
    video_player_context_t *player_context;
    video_shared_context_t *shared_context;
    uint32_t target_frame_rate;
    osMessageQId frameBufferQueueHandle;
    osMutexId ioMutexHandle;
    osSemaphoreId lcdDmaDoneSemHandle;
    osSemaphoreId sdReadDoneSemHandle;
} video_player_task_config_t;

void video_task_run(void const *argument);

#endif
