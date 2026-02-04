#include "sdcard.h"

static bool SD_PowerOn();
static void SD_Select();
static void SD_Deselect();

static SD_Response SD_Send_Command(SD_Command_Type cmd, DWORD arg);
static void        SD_SPI_ReceiveInformation(SD_Information info);
static void        SD_SPI_Send(BYTE data);
static void        SD_SPI_SendReceive(BYTE request, BYTE *response);

static bool SD_BusyWait();

static DSTATUS           status;
static SD_Version_Type   sd_version;
static volatile uint32_t Timer1, Timer2;

static SDCard_HandleTypeDef* hsdcard;

/**
 * SPI를 사용한 초기화 과정
 *
 * https://www.dejazzer.com/ee379/lecture_notes/lec12_sd_card.pdf
 * https://elm-chan.org/docs/mmc/mmc_e.html#spiinit
 * https://onlinedocs.microchip.com/oxy/GUID-F9FE1ABC-D4DD-4988-87CE-2AFD74DEA334-en-US-3/GUID-48879CB2-9C60-4279-8B98-E17C499B12AF.html
 */

void SD_Init(SDCard_HandleTypeDef* __hsdcard) {
    hsdcard = __hsdcard;
}

DSTATUS SD_Initialize(BYTE pdrv) {
    SD_Response    res;
    SD_Information info;

    HAL_Delay(300);
    /**
     * 100khz ~ 400khz로 클럭 낮추기
     * -----------------------------------------------------------------------
     * To ensure the proper operation of the SD card, the SD CLK signal should
     * have a frequency in the range of 100 to 400 kHz.
     */
    hsdcard->hspi->Init.BaudRatePrescaler =
        SPI_BAUDRATEPRESCALER_256; /* 예: 24 MHz /256 ≈ 94 kHz */

    // TODO: 이 코드가 필요한 이유는?
    HAL_SPI_Init(hsdcard->hspi);

    if (!SD_PowerOn()) {
        return status = STA_NOINIT;
    }

    /**
     * 이 이후로는 SD카드의 버전 탐색 및 초기화 로직 수행
     * 여기서부터는 다이어그램을 참고하여 진행했다.
     */
    sd_version = SD_TYPE_UNKNOWN;
    status     = 0;

    SD_Select();
    // CMD8 실행
    res = SD_Send_Command(SD_CMD8, 0x1AA);

    if (res == 1) {
        // Check Voltage
        SD_SPI_ReceiveInformation(info);
        if (info[3] & 0x1AA) {
            Timer1 = 1000;
            do {
                // CMD55 for Leading ACMD
                res = SD_Send_Command(SD_CMD55, 0);
                if (res != SD_RESPONSE_IN_IDLE_STATE) {
                    return status = STA_NOINIT;
                }
                // APP Init
                res = SD_Send_Command(SD_ACMD41, 1 << 30);
            } while (Timer1 && res != 0);
            if (!Timer1) {
                return status = STA_NOINIT;
            }

            // Read OCR
            res = SD_Send_Command(SD_CMD58, 0);
            if (res == 0) {
                SD_SPI_ReceiveInformation(info);
                // Check High capacity
                if (info[0] & 0x40) {
                    sd_version = SD_TYPE_V2_BLOCK_ADDRESS;
                } else {
                    sd_version = SD_TYPE_V2_BYTE_ADDRESS;
                }
            } else {
                sd_version = SD_TYPE_V2_BYTE_ADDRESS;
            }
        } else {
            sd_version = SD_TYPE_UNKNOWN;
        }
    } else {

    }

    /**
     * 초기화 단계를 마치면 SD카드의 상태를 Idle 상태에서 벗어나게 해야 읽기
     * 쓰기가 가능하다. CMD1을 전송하다보면 Idle 상태를 나타내는 비트가
     * 지워진다.
     * -------------------------------------------------------------------------
     * To detect end of the initialization process, the host controller needs to
     * send CMD1 and check the response until end of the initialization. When
     * the card is initialized successfuly, In Idle State bit in the R1 response
     * is cleared (R1 resp changes 0x01 to 0x00).
     */
    do {
        res = SD_Send_Command(SD_CMD1, 0);
    } while (res & SD_RESPONSE_IN_IDLE_STATE);

    if (sd_version == SD_TYPE_V2_BYTE_ADDRESS || sd_version == SD_TYPE_V1 ||
        sd_version == SD_TYPE_MMC_V3) {
        res = SD_Send_Command(SD_CMD16, 512);
        if (res == 0) {
            // increate SPI Clock Speed...?
            // hspi1.Init.BaudRatePrescaler =
            //     SPI_BAUDRATEPRESCALER_4; /* 예: 24 MHz /256 ≈ 94 kHz */
            // HAL_SPI_Init(&hspi1);
            
            hsdcard->hspi->Init.BaudRatePrescaler =
            SPI_BAUDRATEPRESCALER_4; /* 예: 24 MHz /256 ≈ 94 kHz */
            // TODO: 이 코드가 필요한 이유는?
            HAL_SPI_Init(hsdcard->hspi);
        } else {
            sd_version = SD_TYPE_UNKNOWN;
        }
    }

    status &= ~STA_NOINIT;

    SD_Deselect();

    return status;
}

static bool SD_PowerOn() {
    uint8_t res, dummy = 0xFF, n = 0xFF;
    /**
     * SD 카드를 SPI모드로 전환하기 위해 CS핀을 High로, MOSI라인도 High로
     * 설정하고 74번 전송하기 정확히 74번 보내기 어려우므로 단순하게 80번
     * 전송(=8비트를 10번 쓰기).
     * -------------------------------------------------------------------------
     * To communicate with the SD card, your program has to place the SD card
     * into the SPI mode. To do this, set the MOSI and CS lines to logic value 1
     * and toggle SD CLK for at least 74 cycles. After the 74 cycles (or more)
     * have occurred, your program should set the CS line to 0 and send the
     * command CMD0: 01 000000 00000000 00000000 00000000 00000000 1001010 1
     */

    SD_Deselect();
    for (int i = 0; i < 10; i++) {
        SD_SPI_Send(0xFF);
    }
    /**
     * SPI 모드로 전환한 후에는 리셋을 수행해야함(GO_IDLE_STATE 명령어 전송).
     * 일반적인 SPI 통신처럼 CS를 Low로 만들고 명령어 전송.
     * 8비트 응답에 에러가 포함되어 있다면 초기화 로직 수행 불가.
     * ------------------------------------------------------------------------
     * After the 74 cycles (or more) have occurred, your program should set the
     * CS line to 0 and send the command CMD0: 01 000000 00000000 00000000
     * 00000000 00000000 1001010 1 This is the reset command, which puts the SD
     * card into the SPI mode if executed when the CS line is low. The SD card
     * will respond to the reset command by sending a basic 8-bit response on
     * the MISO line.
     */
    SD_Select();
    SD_SPI_Send(0x40);
    SD_SPI_Send(0);
    SD_SPI_Send(0);
    SD_SPI_Send(0);
    SD_SPI_Send(0);
    SD_SPI_Send(0x95);

    do {
        // HAL_SPI_TransmitReceive(&hspi1, &dummy, &res, 1, SD_SPI_TIMEOUT_MS);
        HAL_SPI_TransmitReceive(hsdcard->hspi, &dummy, &res, 1, SD_SPI_TIMEOUT_MS);
    } while ((res != 0x01) && --n);

    SD_Deselect();
    if (!n) {
        return SD_ERROR;
    }

    return SD_OK;
}

static void SD_Select() {
    HAL_GPIO_WritePin(hsdcard->CS_GPIO_Port, hsdcard->CS_Pin, GPIO_PIN_RESET);
}

static void SD_Deselect() {
    HAL_GPIO_WritePin(hsdcard->CS_GPIO_Port, hsdcard->CS_Pin, GPIO_PIN_SET);
}

SD_Response SD_Send_Command(SD_Command_Type cmd, DWORD arg) {
    uint8_t     crc   = 0x01;
    uint8_t     dummy = 0xFF;
    uint32_t    n     = 0xFF;
    SD_Response res;

    SD_BusyWait();

    /**
     * CMD0, CMD8, CMD58의 경우 고정된 CRC값을 포함해야함. 나머지 명령어의 경우
     * 신경쓰지 않는다.
     */
    if (cmd == SD_CMD0) {
        crc = SD_CMD0_CRC;
    } else if (cmd == SD_CMD8) {
        crc = SD_CMD8_CRC;
    } else if (cmd == SD_CMD58) {
        crc = SD_CMD58_CRC;
    }
    /**
     * start bit(2) + cmd(6) + arg(32) + CRC(7) + stop bit(1)
     *  = 48 bits -> send 6 times
     */
    SD_SPI_Send(cmd);
    SD_SPI_Send((BYTE)(arg >> 24));
    SD_SPI_Send((BYTE)(arg >> 16));
    SD_SPI_Send((BYTE)(arg >> 8));
    SD_SPI_Send((BYTE)(arg));
    SD_SPI_Send(crc);

    /**
     * 일반적인 SPI 수신 절차에 따라 8번 클럭을 토글하기 위해 MOSI를 high로 둔
     * 더미데이터 전송.
     * SD카드의 경우 이전에 보냈던 명령에 따라 8비트 응답만 오거나, 32비트 추가
     * 응답이 온다. 추가 정보는 SD_SPI_ReceiveInformation 함수에서 얻도록
     * 처리한다.
     * ---------------------------------------------------------------------
     * Once the SD card receives a command it will begin processing it. To
     * respond to a command, the SD card requires the SD CLK signal to
     * toggle for at least 8 cycles. Your program will have to toggle the SD
     * CLK signal and maintain the MOSI line high while waiting for a
     * response. The length of a response message varies depending on the
     * command. Most of the commands get a response mostly in the form of
     * 8-bit messages, with two exceptions where the response consists of 40
     * bits.
     */

    do {
        /**
         * 16클럭 내로 응답이 오지 않을 경우 리셋 명령어를 다시 전송해야함.
         * 16클럭이면 8비트*2이므로 2번만 읽어도 되지만, 안전성을 위해 10번
         * 읽어보도록 설정함.
         * --------------------------------------------------------------------
         * Note that the response to each command is sent by the card a few SD
         * CLK cycles later. If the expected response is not received within 16
         * clock cycles after sending the reset command, the reset command has
         * to be sent again.
         */
        SD_SPI_SendReceive(dummy, &res);
        n--;
    } while ((res & 0x80) && n > 0);

    return res;
}

static void SD_SPI_Send(BYTE data) {
    // while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY)
    //     ;
    // HAL_SPI_Transmit(&hspi1, &data, 1, SD_SPI_TIMEOUT_MS);
    while (HAL_SPI_GetState(hsdcard->hspi) != HAL_SPI_STATE_READY)
        ;
    HAL_SPI_Transmit(hsdcard->hspi, &data, 1, SD_SPI_TIMEOUT_MS);
}

static void SD_SPI_SendReceive(BYTE request, BYTE *response) {
    // while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY)
    //     ;
    // HAL_SPI_TransmitReceive(&hspi1, &request, response, 1, SD_SPI_TIMEOUT_MS);
    while (HAL_SPI_GetState(hsdcard->hspi) != HAL_SPI_STATE_READY)
        ;
    HAL_SPI_TransmitReceive(hsdcard->hspi, &request, response, 1, SD_SPI_TIMEOUT_MS);
}

static void SD_SPI_ReceiveInformation(SD_Information info) {
    /**
     * CMD8과 CMD55의 경우 58비트 응답이 오므로, R1 응답을 제외한 32비트 응답을
     * 받도록 처리하는 함수
     */
    uint8_t dummy = 0xFF;

    for (int i = 0; i < 4; i++) {
        SD_SPI_SendReceive(dummy, &info[i]);
    }
}

DSTATUS SD_Status(BYTE pdrv) { return status; }

SD_Version_Type SD_GetVersion() { return sd_version; }

DSTATUS SD_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    SD_Response r;
    uint8_t     csd[32];
    uint8_t     dummy = 0xFF;
    DSTATUS     res   = RES_OK;

    if (cmd == CTRL_SYNC) {
        /**
         * 지연 쓰기 방식을 사용한다면 일단 메모리에 변경된 내용을 갖고 있다가
         * 파일을 닫을 때 한꺼번에 저장한다. jpa에서 영속성 컨텍스트와 같이
         * 곧바로 디스크에 쓰기작업을 한다면 당연히 응답속도가 늦기 때문이다.
         * 여기서는 파일을 닫을 때 쓰기 작업이 완료될 때까지 기다리는 용도로
         * 쓰인다.
         * ---------------------------------------------------------------------
         * Makes sure that the device has finished pending write process. If the
         * disk I/O layer or storage device has a write-back cache, the dirty
         * cache data must be committed to the medium immediately. Nothing to do
         * for this command if each write operation to the medium is completed
         * in the disk_write function.
         * ---------------------------------------------------------------------
         * Make sure that no pending write process in the physical drive
         * if (disk_ioctl(fs->drv, CTRL_SYNC, 0) != RES_OK)
         *	   res = FR_DISK_ERR;
         */
        /**
         * 0xFF가 수신된다면 busy flag가 끝난 것
         * ---------------------------------------------------------------------
         * It is an R1 response followed by busy flag (DO is driven to low as
         * long as internal process is in progress). The host controller should
         * wait for end of the process until DO goes high (a 0xFF is received).
         */
        // Timer2 = 1000;

        if (!SD_BusyWait()) {
            res = RES_ERROR;
        }
    } else if (cmd == GET_SECTOR_SIZE) {
        /**
         * ---------------------------------------------------------------------
         * Retrieves sector size (minimum data unit for generic read/write) into
         * the WORD variable that pointed by buff. Valid sector sizes are 512,
         * 1024, 2048 and 4096. This command is required only if FF_MAX_SS >
         * FF_MIN_SS. When FF_MAX_SS == FF_MIN_SS, this command will never be
         * used and the disk_read and disk_write function must work in FF_MAX_SS
         * bytes/sector.
         */
        *(WORD *)buff = 512; // 왜 512?
    } else if (cmd == GET_BLOCK_SIZE) {
        *(DWORD *)buff = 8;
    } else {
        res = RES_PARERR;
    }
    return res;
}

DSTATUS SD_Read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count) {
    /**
     * 명령 요청 후에는 항상 CmdResponse가 먼저 응답된다. 그 이후 DataPacket이
     * 전달된다. DataPacket은 Token + Block + CRC를 의미한다. CMD12(Stop
     * Transmission)은 Token만 전달되고 Block과 CRC는 전달되지 않는다.
     * ------------------------------------------------------------------------
     * The data block is transferred as a data packet that consist of Token,
     * Data Block and CRC. The format of the data packet is showin in right
     * image and there are three data tokens. Stop Tran token is to terminate a
     * multiple block write transaction, it is used as single byte packet
     * without data block and CRC.
     */
    SD_DataToken token;
    SD_Response  res;
    uint8_t      dummy = 0xFF, crc = 0x01;
    DWORD        addr = (sd_version == SD_TYPE_V2_BLOCK_ADDRESS)
                            ? sector        /* SDHC/SDXC: block address */
                            : sector * 512; /* SDSC: byte address */

    if (count == 1) {
        /**
         * read single block
         * request and receive token
         * todo: SD_Send_Command 함수는 초기화를 위해 작성되어서, 읽기쓰기작업이
         * 완료된 후 CS핀이 high로 바뀌어야 함을 간과했다. 일단 내부 구현을
         * 그대로 옮겨와 작성하여 문제를 회피했다.
         */
        SD_Select();

        res = SD_Send_Command(SD_CMD17, addr);

        if (res == 0) {
            /**
             * Read DataToken
             */
            Timer1 = 200; // 타임아웃 200ms
            do {
                SD_SPI_SendReceive(dummy, &token);
            } while (Timer1 && token == 0xFF);
            /**
             * 에러 토큰 검사
             */
            if (token != 0xFE) {
                SD_Deselect();
                SD_SPI_Send(0xFF);
                return RES_ERROR;
            }

            /**
             * Read DataBlock
             */
            for (UINT i = 0; i < 512; i++) {
                SD_SPI_SendReceive(dummy, &buff[i]);
            }

            /**
             * Read CRC, but skip
             */
            SD_SPI_Send(crc);
            SD_SPI_Send(crc);

            SD_Deselect();
        } else {
            return RES_ERROR;
        }

    } else {
    }
    return RES_OK;
}

DSTATUS SD_Write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count) {
    SD_DataResponse data_res;
    SD_Response     res;
    uint8_t         dummy = 0xFF;

    SD_Select();
    if (count == 1) {
        data_res = (SD_DataResponse)SD_Send_Command(SD_CMD24, sector);

        if (data_res == 0) {
            /**
             * 0. 더미 1바이트 전송
             * 1. 데이터 토큰 전송
             * 2. 데이터 블록 전송
             * 3. CRC 전송
             * 4. busy flag 처리
             */
            uint8_t token = SD_DATA_TOKEN_CMD17_18_24;
            uint8_t crc   = 0xFF;

            SD_SPI_Send(dummy);
            SD_SPI_Send(token);

            for (int i = 0; i < 512; i++) {
                SD_SPI_Send(*(buff++));
            }
            SD_SPI_Send(crc);
            SD_SPI_Send(crc);

            SD_SPI_SendReceive(dummy, &data_res);
            if (SD_IS_DATA_ACCEPTED(data_res)) {
                do {
                    SD_SPI_SendReceive(dummy, &res);
                } while (res != 0xFF);
            }
        }
    } else {
    }
    SD_Deselect();
    return RES_OK;
}

bool SD_BusyWait() {
    Timer2 = 500;
    uint8_t res, dummy = 0xFF;
    do {
        SD_SPI_SendReceive(dummy, &res);
    } while ((res != 0xFF) && Timer2);

    if (!Timer2) {
        return SD_ERROR;
    }
    return SD_OK;
}