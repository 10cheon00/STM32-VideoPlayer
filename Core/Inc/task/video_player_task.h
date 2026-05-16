#ifndef _VIDEO_TASK_H_
#define _VIDEO_TASK_H_

#include "cmsis_os.h"
#include "lcd/st7789.h"
#include "video/video_player.h"

/**
 * video player task 동작 절차
 *
 * 1. 시작 시 전달받은 config를 사용해 ST7789 handle을 초기화하고,
 *    st7789_init_display()로 LCD를 동작 가능한 상태로 만든다.
 * 2. 초기 동작 확인이 필요하면 st7789_print_sample_display()를 호출해
 *    샘플 화면을 출력한다.
 * 3. 루프에 진입하면 frameBufferQueueHandle에서 다음 출력 대상 버퍼
 *    인덱스를 get 하여 reader가 준비한 버퍼를 선택한다.
 * 4. 출력 직전에 ioMutexHandle을 획득하여 SDIO 읽기와 LCD DMA가 동시에
 *    실행되지 않도록 전역 I/O 접근을 독점한다.
 * 5. video_player_print_video_buffer() 또는
 *    st7789_print_pixels_with_range()를 통해 LCD DMA 전송을 시작한다.
 * 6. DMA 전송이 시작되면 mutex를 즉시 반납하지 않고,
 *    lcdDmaDoneSemHandle을 wait 하여 DMA 완료 콜백이 올 때까지 유지한다.
 * 7. HAL_SPI_TxCpltCallback()에서 DMA 완료 세마포어가 release 되면
 *    task가 깨어나고, 그 시점에 ioMutexHandle을 반납한다.
 * 8. 현재 버퍼 출력이 끝났음을 의미하므로 sdReadDoneSemHandle을 release 하여
 *    video reader task가 다음 청크를 읽을 수 있도록 허가한다.
 */
typedef struct {
    video_player_context_t *player_context;
    video_shared_context_t *shared_context;
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *GPIO_Port_CS;
    GPIO_TypeDef *GPIO_Port_DC;
    GPIO_TypeDef *GPIO_Port_RST;
    uint16_t GPIO_Pin_CS;
    uint16_t GPIO_Pin_DC;
    uint16_t GPIO_Pin_RST;
    uint16_t screen_width;
    uint16_t screen_height;
    st7789_dma_status_t dma_status;
    uint32_t target_frame_rate;
    osMessageQId writableBufferQueueHandle;
    osMessageQId printableBufferQueueHandle;
} video_player_task_config_t;

void video_player_task_run(void const *argument);

#endif
