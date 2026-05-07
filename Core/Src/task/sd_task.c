#include "task/sd_task.h"

#include "cmsis_os.h"

void sd_task_run(void const *argument) {
    (void)argument;

    for (;;) {
        osDelay(1);
    }
}
