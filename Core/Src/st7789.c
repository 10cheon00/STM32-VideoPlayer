#include "st7789.h"

static st7789_status_t
st7789_init_handle(st7789_handle_t *handle, SPI_HandleTypeDef *hspi,
                   GPIO_TypeDef *GPIO_Port_CS, GPIO_TypeDef *GPIO_Port_SDA,
                   GPIO_TypeDef *GPIO_Port_SCL, GPIO_TypeDef *GPIO_Port_DC,
                   GPIO_TypeDef *GPIO_Port_RST, uint16_t GPIO_Pin_CS,
                   uint16_t GPIO_Pin_SDA, uint16_t GPIO_Pin_SCL,
                   uint16_t GPIO_Pin_DC, uint16_t GPIO_Pin_RST) {
    handle->hspi = hspi;
    handle->GPIO_Port_CS = GPIO_Port_CS;
    handle->GPIO_Port_DC = GPIO_Port_DC;
    handle->GPIO_Port_RST = GPIO_Port_RST;
    handle->GPIO_Port_SCL = GPIO_Port_SCL;
    handle->GPIO_Port_SDA = GPIO_Port_SDA;
    handle->GPIO_Pin_CS = GPIO_Pin_CS;
    handle->GPIO_Pin_DC = GPIO_Pin_DC;
    handle->GPIO_Pin_RST = GPIO_Pin_RST;
    handle->GPIO_Pin_SCL = GPIO_Pin_SCL;
    handle->GPIO_Pin_SDA = GPIO_Pin_SDA;
    return STATUS_OK;
}

static st7789_status_t st7789_init_display(st7789_handle_t *handle) {
    return STATUS_OK;
}

static st7789_status_t st7789_print_sample_display(st7789_handle_t *handle) {
    return STATUS_OK;
}
