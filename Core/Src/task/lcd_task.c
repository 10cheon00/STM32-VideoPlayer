#include "task/lcd_task.h"

#include "cmsis_os.h"
#include "main.h"

static st7789_handle_t handle;

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        st7789_dma_tx_cplt_callback(&handle);
    }
}

void lcd_task_run(void const *argument) {
    lcd_task_config_t *config = (lcd_task_config_t *)argument;

    if (config == NULL || config->hspi == NULL) {
        Error_Handler();
    }

    if (st7789_init_handle(
            &handle, config->hspi, config->GPIO_Port_CS, config->GPIO_Port_DC,
            config->GPIO_Port_RST, config->GPIO_Pin_CS, config->GPIO_Pin_DC,
            config->GPIO_Pin_RST, config->screen_width, config->screen_height,
            config->dma_status) != ST7789_STATUS_OK) {
        Error_Handler();
    }

    if (st7789_init_display(&handle) != ST7789_STATUS_OK) {
        Error_Handler();
    }


    if(st7789_print_sample_display(&handle) != ST7789_STATUS_OK) {
        Error_Handler();
    }

    for (;;) {
        osDelay(1);
    }
}
