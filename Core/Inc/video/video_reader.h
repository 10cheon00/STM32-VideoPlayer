#ifndef _VIDEO_READER_H_
#define _VIDEO_READER_H_

#include "sd/micro_sd.h"
#include "video_context.h"

typedef struct {
    FATFS *sd_fatfs;
    FIL file;
    uint16_t GPIO_Pin_CS;
    DWORD frame_bytes;
    UINT bytes_read;
    DWORD file_bytes;
    uint32_t max_frame_index;

    micro_sd_handle_t sd_handle;
    SPI_HandleTypeDef *hspi;
    micro_sd_spi_bus_clock_t spi_bus_clock_max;
    GPIO_TypeDef *GPIO_Port_CS;
} video_reader_context_t;

video_context_status_t
video_reader_init(video_reader_context_t *reader_context, DWORD frame_bytes,
                  FATFS *sd_fatfs, SPI_HandleTypeDef *hspi,
                  GPIO_TypeDef *GPIO_Port_CS, uint16_t GPIO_Pin_CS,
                  micro_sd_spi_bus_clock_t spi_bus_clock_max);

video_context_status_t
video_reader_mount(video_reader_context_t *reader_context,
                   const TCHAR *sd_path);

video_context_status_t
video_reader_open_file(video_reader_context_t *reader_context,
                       const TCHAR *file_path);

video_context_status_t
video_reader_close_file(video_reader_context_t *reader_context);

video_context_status_t
video_reader_read_file(video_reader_context_t *reader_context,
                       video_shared_context_t *shared_context,
                       video_buffer_t *buffer);

#endif
