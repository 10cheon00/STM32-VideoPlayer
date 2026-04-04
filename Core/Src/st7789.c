#include "st7789.h"

static st7789_status_t
st7789_init_handle(st7789_handle_t *handle, SPI_HandleTypeDef *hspi,
                   GPIO_TypeDef *GPIO_Port_CS, GPIO_TypeDef *GPIO_Port_SDA,
                   GPIO_TypeDef *GPIO_Port_SCL, GPIO_TypeDef *GPIO_Port_DC,
                   GPIO_TypeDef *GPIO_Port_RST, uint16_t GPIO_Pin_CS,
                   uint16_t GPIO_Pin_SDA, uint16_t GPIO_Pin_SCL,
                   uint16_t GPIO_Pin_DC, uint16_t GPIO_Pin_RST) {
    return STATUS_OK;
}

static st7789_status_t st7789_init_display(st7789_handle_t *handle) {
    return STATUS_OK;
}

static st7789_status_t st7789_print_sample_display(st7789_handle_t *handle) {
    return STATUS_OK;
}
