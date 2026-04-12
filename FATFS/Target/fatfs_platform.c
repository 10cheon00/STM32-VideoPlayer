/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : fatfs_platform.c
 * @brief          : fatfs_platform source file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
#include "fatfs_platform.h"

uint8_t	BSP_PlatformIsDetected(void) {
    uint8_t status = SD_PRESENT;
    /* Check SD card detect pin */
    if(HAL_GPIO_ReadPin(SD_DETECT_GPIO_PORT, SD_DETECT_PIN) != GPIO_PIN_RESET)
    {
        status = SD_NOT_PRESENT;
    }
    /* USER CODE BEGIN 1 */
    /* user code can be inserted here */

    // Adafruit SDIO sd카드 모듈은 신호를 다르게 출력한다.
    // sd카드가 모듈에 있으면 3V 출력을, 모듈에 없으면 0V 출력을 낸다.
    // CubeMX가 생성한 코드는 DET GPIO핀에 HIGH신호가 들어온다면 카드가 없다고
    // 판별하도록 작성되어 있어서, 이를 무시하고 아래에 새로 코드를 작성했다.
    status = SD_PRESENT;
    /* Check SD card detect pin */
    if (HAL_GPIO_ReadPin(SD_DETECT_GPIO_PORT, SD_DETECT_PIN) !=
        GPIO_PIN_SET) {
        status = SD_NOT_PRESENT;
    }
    /* USER CODE END 1 */
    return status;
}
