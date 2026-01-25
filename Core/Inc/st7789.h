#ifndef ST7789_H
#define ST7789_H

#include "stm32f4xx_hal.h"

#define ST7789_CMD_SIZE (sizeof(uint8_t))
#define ST7789_CMD_TIMEOUT ((uint8_t) 255)

#define ST7789_NOP 0x00
#define ST7789_SWRESET 0x01 // 소프트웨어 리셋
#define ST7789_SLPOUT 0x11  // 절전모드 탈출
#define ST7789_COLMOD 0x3A  // 픽셀 형식 선택
#define ST7789_MADCTL 0x36  // 메모리 접근법 선택
#define ST7789_CASET 0x2A   // 열 주소값 설정
#define ST7789_RASET 0x2B   // 행 주소값 설정
#define ST7789_INVON 0x21   // 디스플레이 인버전 모드 설정
#define ST7789_NORON 0x13   // 일반 디스플레이 모드 설정
#define ST7789_DISPON 0x29  // 디스플레이 켜기? TODO: 반대되는 DISPOFF 명령어로 꺼지는지 확인해볼까?
#define ST7789_RAMWR 0x2C   // 이미지 픽셀 데이터 전송

#define ST7789_MADCTL_PAGE_ADDRESS_ORDER_TOP_TO_BOTTOM 0x00
#define ST7789_MADCTL_PAGE_ADDRESS_ORDER_BOTTOM_TO_TOP 0x80
#define ST7789_MADCTL_COLOMN_ADDRESS_ORDER_LEFT_TO_RIGHT 0x00
#define ST7789_MADCTL_COLOMN_ADDRESS_ORDER_RIGHT_TO_LEFT 0x40
#define ST7789_MADCTL_PAGE_COLUMN_ORDER_NORMAL 0x00
#define ST7789_MADCTL_PAGE_COLUMN_ORDER_REVERSE 0x20
#define ST7789_MADCTL_LINE_ORDER_TOP_TO_BOTTOM 0x00
#define ST7789_MADCTL_LINE_ORDER_BOTTOM_TO_TOP 0x10
#define ST7789_MADCTL_RGB_BGR_ORDER_RGB 0x00
#define ST7789_MADCTL_RGB_BGR_ORDER_BGR 0x08
#define ST7789_MADCTL_DISPLAY_LATCH_ORDER_LEFT_TO_RIGHT 0x00
#define ST7789_MADCTL_DISPLAY_LATCH_ORDER_RIGHT_TO_LEFT 0x04

#define ST7789_COLMOD_RGB_INTERFACE_65K 0x50
#define ST7789_COLMOD_RGB_INTERFACE_262K 0x60
#define ST7789_COLMOD_COLOR_FORMAT_12BIT 0x03
#define ST7789_COLMOD_COLOR_FORMAT_16BIT 0x05
#define ST7789_COLMOD_COLOR_FORMAT_18BIT 0x06
#define ST7789_COLMOD_COLOR_FORMAT_16M_TRUNCATED 0x07

enum DataSelectionControlSignal{
  DATA_SELECTION_CONTROL_SIGNAL_REGISTER = GPIO_PIN_RESET,
  DATA_SELECTION_CONTROL_SIGNAL_DATA = GPIO_PIN_SET
};

/*
static const uint8_t PROGMEM
  generic_st7789[] =  {             // Init commands for 7789 screens
    9,                              //  9 commands in list:
    ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, no args, w/delay
      150,                          //     ~150 ms delay
    ST77XX_SLPOUT ,   ST_CMD_DELAY, //  2: Out of sleep mode, no args, w/delay
      10,                           //      10 ms delay
    ST77XX_COLMOD , 1+ST_CMD_DELAY, //  3: Set color mode, 1 arg + delay:
      0x55,                         //     16-bit color
      10,                           //     10 ms delay
    ST77XX_MADCTL , 1,              //  4: Mem access ctrl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    ST77XX_CASET  , 4,              //  5: Column addr set, 4 args, no delay:
      0x00,                         //
      0,                            //     XSTART = 0
      0,                            //
      240,                          //     XEND = 240
    ST77XX_RASET  , 4,              //  6: Row addr set, 4 args, no delay:
      0x00,                         //
      0,                            //     YSTART = 0
      320>>8,                       //
      320&0xFF,                     //     YEND = 320
    ST77XX_INVON  ,   ST_CMD_DELAY, //  7: hack
      10,                           //
    ST77XX_NORON  ,   ST_CMD_DELAY, //  8: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON ,   ST_CMD_DELAY, //  9: Main screen turn on, no args, delay
      10 };                         //    10 ms delay
*/

typedef struct {
    SPI_HandleTypeDef* hspi;
    GPIO_TypeDef *SCL_GPIO_Port;
    GPIO_TypeDef *SDA_GPIO_Port;
    GPIO_TypeDef *RST_GPIO_Port;
    GPIO_TypeDef *DC_GPIO_Port;
    GPIO_TypeDef *CS_GPIO_Port;
    uint16_t SCL_Pin;
    uint16_t SDA_Pin;
    uint16_t RST_Pin;
    uint16_t DC_Pin;
    uint16_t CS_Pin;
} ST7789_HandleTypeDef;

/* 
 */
typedef struct ST7789_Image {
    uint16_t width;
    uint16_t height;
    uint16_t x;
    uint16_t y;
    uint8_t *rawData; // TODO: 영상에서 얻게되는 이미지 타입에 따라 이 변수의 타입을 수정해야함
} ST7789_Image;

void ST7789_Init(ST7789_HandleTypeDef* hst7789);
void ST7789_PrintImage(ST7789_Image *image);

#endif /* ST7789_H */
