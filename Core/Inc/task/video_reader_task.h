#ifndef _SD_TASK_H_
#define _SD_TASK_H_

#include "cmsis_os.h"
#include "video/video_reader.h"

/**
 * video reader task 동작 절차
 *
 * 1. 시작 시 전달받은 config를 사용해 reader/shared context를 초기화하고,
 *    SD 카드 마운트 및 재생 파일 열기를 수행한다.
 * 2. 루프에 진입하면 먼저 sdReadDoneSemHandle을 wait 하여 다음 읽기가
 *    허가되었는지 확인한다.
 * 3. 읽기가 허가되면 ioMutexHandle을 획득하여 SDIO와 LCD DMA가 동시에
 *    실행되지 않도록 전역 I/O 접근을 독점한다.
 * 4. video_reader_read_file()에 큐에서 받은 버퍼 주소를 넘겨 한 청크를
 *    직접 읽어온다.
 * 5. 읽기가 끝나면 방금 채운 동일 버퍼 주소를 printable queue로 넘겨
 *    video player task에 전달한다.
 * 6. 이후 다시 writable queue를 wait 하며, player가 출력 후 반환한 버퍼가
 *    올 때까지 대기한다.
 */
typedef struct {
    video_reader_context_t *reader_context;
    video_shared_context_t *shared_context;
    FATFS *sd_fatfs;
    SD_HandleTypeDef *hsd;
    DWORD frame_bytes;
    const TCHAR *sd_path;
    const TCHAR *file_path;
    osMessageQId printableBufferQueueHandle;
    osMessageQId writableBufferQueueHandle;
} video_reader_task_config_t;

void video_reader_task_run(void const *argument);

#endif
