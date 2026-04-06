#include "micro_sd.h"

micro_sd_status_t micro_sd_init_handle(micro_sd_handle_t *handle,
                                       SPI_HandleTypeDef *hspi,
                                       GPIO_TypeDef *GPIO_Port_CS,
                                       uint16_t GPIO_Pin_CS) {

    handle->hspi = hspi;
    handle->GPIO_Port_CS = GPIO_Port_CS;
    handle->GPIO_Pin_CS = GPIO_Pin_CS;

    return STATUS_OK;
}

micro_sd_status_t micro_sd_is_valid_handle(micro_sd_handle_t *handle) {
    micro_sd_status_t status = STATUS_OK;

    if (handle == NULL) {
        status = STATUS_HANDLE_NOT_VALID;
    } else {
        if (handle->hspi == NULL || handle->GPIO_Port_CS == NULL ||
            handle->GPIO_Pin_CS == NULL) {
            status = STATUS_HANDLE_NOT_VALID;
        }
    }

    return status;
}

micro_sd_status_t micro_sd_init_card(micro_sd_handle_t *handle) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_get_status(micro_sd_handle_t *handle) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_read_block(micro_sd_handle_t *handle, BYTE *buffer,
                                      DWORD sector, UINT count) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_write_block(micro_sd_handle_t *handle, BYTE *buffer,
                                       DWORD sector, UINT count) {
    return STATUS_OK;
}

micro_sd_status_t micro_sd_ioctl(micro_sd_handle_t *handle, BYTE cmd,
                                 BYTE *buffer) {
    return STATUS_OK;
}
