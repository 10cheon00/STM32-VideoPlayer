#include "task/video_task.h"

#include "cmsis_os.h"

void video_task_run(void const *argument) {
    (void)argument;

    for (;;) {
        osDelay(1);
    }
}
