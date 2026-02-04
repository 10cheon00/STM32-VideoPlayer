#include "drv_st7789.h"

static ST7789_HandleTypeDef *hst7789;

void setDataSelection(enum DataSelectionControlSignal dc) {
  HAL_GPIO_WritePin(hst7789->DC_GPIO_Port, hst7789->DC_Pin, (GPIO_PinState)dc);
}

void resetLCD() {
  HAL_GPIO_WritePin(hst7789->RST_GPIO_Port, hst7789->RST_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(hst7789->RST_GPIO_Port, hst7789->RST_Pin, GPIO_PIN_SET);
}

void enableChip() {
  HAL_GPIO_WritePin(hst7789->CS_GPIO_Port, hst7789->CS_Pin, GPIO_PIN_RESET);
}

void disableChip() {
  HAL_GPIO_WritePin(hst7789->CS_GPIO_Port, hst7789->CS_Pin, GPIO_PIN_SET);
}

void sendCommandArgs(const uint8_t command, const uint8_t delay, const uint8_t *arguments, const uint16_t argumentLength) {
  enableChip();

  setDataSelection(DATA_SELECTION_CONTROL_SIGNAL_REGISTER);
  HAL_SPI_Transmit(hst7789->hspi, &command, ST7789_CMD_SIZE,
                   ST7789_CMD_TIMEOUT);

  if (argumentLength > 0) {
    setDataSelection(DATA_SELECTION_CONTROL_SIGNAL_DATA);
    HAL_SPI_Transmit(hst7789->hspi, arguments, argumentLength * ST7789_CMD_SIZE, ST7789_CMD_TIMEOUT);
  }

  disableChip();

  if (delay > 0) {
    HAL_Delay(delay);
  }
}

void sendCommand(const uint8_t command, const uint8_t delay) {
  sendCommandArgs(command, delay, NULL, 0);
}

void sendData(const uint8_t *data, const uint16_t dataLength) {
  
}

void ST7789_Init(ST7789_HandleTypeDef *hst7789Param) {
  // TODO: 초기화 명령어들을 처리하는 로직 구현하기

  const uint8_t memoryDataAccessControlArg =
      ST7789_MADCTL_PAGE_ADDRESS_ORDER_TOP_TO_BOTTOM |
      ST7789_MADCTL_COLOMN_ADDRESS_ORDER_LEFT_TO_RIGHT |
      ST7789_MADCTL_PAGE_COLUMN_ORDER_NORMAL |
      ST7789_MADCTL_LINE_ORDER_TOP_TO_BOTTOM | ST7789_MADCTL_RGB_BGR_ORDER_RGB |
      ST7789_MADCTL_DISPLAY_LATCH_ORDER_LEFT_TO_RIGHT;
  const uint8_t interfacePixelFormatArgs =
      ST7789_COLMOD_RGB_INTERFACE_65K | ST7789_COLMOD_COLOR_FORMAT_12BIT;

  hst7789 = hst7789Param;
  resetLCD();

  sendCommand(ST7789_SWRESET, 150);
  sendCommand(ST7789_SLPOUT, 150);
  sendCommandArgs(ST7789_MADCTL, 0, &memoryDataAccessControlArg, 1);
  sendCommand(ST7789_NORON, 0);
  sendCommand(ST7789_INVON, 10);
  sendCommand(ST7789_DISPON, 10);
  sendCommandArgs(ST7789_COLMOD, 10, &interfacePixelFormatArgs, 1);
}

void ST7789_PrintImage(ST7789_Image *image) {
  // TODO: 원본 이미지 데이터를 압축하는 과정을 구현해야함. 지금은 4비트 압축이
  // 된 상태를 전제로 하여 동작하고 있음
  const uint8_t columnAddressSetArgs[4] = {
      (uint8_t)((image->x & 0xFF00) >> 8), (uint8_t)(image->x & 0x00FF),
      (uint8_t)(((image->x + image->width) & 0xFF00) >> 8),
      (uint8_t)((image->x + image->width) & 0x00FF)};
  const uint8_t rowAddressSetArgs[4] = {
      (uint8_t)((image->y & 0xFF00) >> 8), (uint8_t)(image->y & 0x00FF),
      (uint8_t)(((image->y + image->height) & 0xFF00) >> 8),
      (uint8_t)((image->y + image->height) & 0x00FF)};
  const uint16_t packedDataLength = (image->width * image->height * 3) >> 1;

  sendCommandArgs(ST7789_CASET, 0, columnAddressSetArgs, 4);
  sendCommandArgs(ST7789_RASET, 0, rowAddressSetArgs, 4);
  sendCommandArgs(ST7789_RAMWR, 0, image->rawData, packedDataLength);
}
