#ifndef _MICRO_SD_H_
#define _MICRO_SD_H_

#include "stm32f4xx_hal.h"

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *GPIO_Port_CS;
    uint16_t GPIO_Pin_CS;
} micro_sd_handle_t;

typedef enum {
    STATUS_OK = 0,
    STATUS_NO_FOLDER_EXIST,
    STATUS_NO_FILE_EXIST,
    STATUS_FAILED_TO_READ_FILE
} micro_sd_status_t;

typedef struct {

} micro_sd_file_t;

typedef struct {

} micro_sd_file_header_t;

micro_sd_status_t micro_sd_init_handle(micro_sd_handle_t *handle,
                                       SPI_HandleTypeDef *hspi,
                                       GPIO_TypeDef *GPIO_Port_CS,
                                       uint16_t GPIO_Pin_CS);

micro_sd_status_t micro_sd_init_card(micro_sd_handle_t *handle);

micro_sd_status_t micro_sd_change_directory(micro_sd_handle_t *handle,
                                            const char *path);

micro_sd_status_t micro_sd_open_file(micro_sd_handle_t *handle,
                                     const char *filename,
                                     micro_sd_file_t *file);

micro_sd_status_t micro_sd_read_file_header(micro_sd_handle_t *handle,
                                            micro_sd_file_t *file);

micro_sd_status_t micro_sd_close_file(micro_sd_handle_t *handle,
                                      micro_sd_file_t *file);
#endif
