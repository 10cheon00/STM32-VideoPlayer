#include "micro_sd.h"

micro_sd_status_t micro_sd_init_handle(micro_sd_handle_t *handle,
                                       SPI_HandleTypeDef *hspi,
                                       GPIO_TypeDef *GPIO_Port_CS,
                                       uint16_t GPIO_Pin_CS) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_init_card(micro_sd_handle_t *handle) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_change_directory(micro_sd_handle_t *handle,
                                            const char *path) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_open_file(micro_sd_handle_t *handle,
                                     const char *filename,
                                     micro_sd_file_t *file) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_read_file_header(micro_sd_handle_t *handle,
                                            micro_sd_file_t *file) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_close_file(micro_sd_handle_t *handle,
                                      micro_sd_file_t *file) {
    return STATUS_OK;
}
