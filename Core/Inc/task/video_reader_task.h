#ifndef _SD_TASK_H_
#define _SD_TASK_H_

#include "cmsis_os.h"
#include "video/video_reader.h"

typedef struct {
    video_reader_context_t *reader_context;
    video_shared_context_t *shared_context;
    FATFS *sd_fatfs;
    SD_HandleTypeDef *hsd;
    const TCHAR *sd_path;
    const TCHAR *file_path;
    osMessageQId frameBufferQueueHandle;
    osMutexId ioMutexHandle;
    osSemaphoreId sdReadDoneSemHandle;
} video_reader_task_config_t;

void sd_task_run(void const *argument);

#endif
