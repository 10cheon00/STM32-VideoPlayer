#ifndef _LCD_TASK_H_
#define _LCD_TASK_H_

#include "lcd/st7789.h"

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *GPIO_Port_CS;
    GPIO_TypeDef *GPIO_Port_DC;
    GPIO_TypeDef *GPIO_Port_RST;
    uint16_t GPIO_Pin_CS;
    uint16_t GPIO_Pin_DC;
    uint16_t GPIO_Pin_RST;
    uint16_t screen_width;
    uint16_t screen_height;
    st7789_dma_status_t dma_status;
} lcd_task_config_t;

void lcd_task_run(void const *argument);

#endif
