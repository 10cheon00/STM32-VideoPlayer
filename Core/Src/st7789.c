#include "st7789.h"

static ST7789_HandleTypeDef *hst7789;

void set_DC_Pin(GPIO_PinState state) {
    HAL_GPIO_WritePin(hst7789->DC_GPIO_Port, hst7789->DC_Pin, state);
}

void set_RST_Pin(GPIO_PinState state) {
    HAL_GPIO_WritePin(hst7789->RST_GPIO_Port, hst7789->RST_Pin, state);
}

void set_CS_Pin(GPIO_PinState state) {
    HAL_GPIO_WritePin(hst7789->CS_GPIO_Port, hst7789->CS_Pin, state);
}

void ST7789_Init(ST7789_HandleTypeDef *hst7789Param) {
    hst7789 = hst7789Param;
    
    // TODO: 초기화 명령어들을 처리하는 로직 구현하기
    // HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, const uint8_t *pData, uint16_t Size, uint32_t Timeout)
}

void ST7789_PrintCharacter() {

}
