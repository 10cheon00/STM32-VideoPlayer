#include "task/lcd_task.h"

#include "cmsis_os.h"

void lcd_task_run(void const *argument) {
    (void)argument;

    for (;;) {
        osDelay(1);
    }
}
