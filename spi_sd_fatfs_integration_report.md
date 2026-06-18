# SPI microSD FatFs Integration Report

## Summary

현재 브랜치는 `feat/spi-sd-library-refactoring`이며, `Core/Inc/micro_sd.h`와
`Core/Src/_micro_sd.c`가 추가된 상태다. 기존 SDIO 기반 FatFs 생성 파일들은
삭제된 상태로 보이며, 앞으로는 SPI microSD 드라이버를 FatFs의 disk I/O 계층에
연결하는 구조가 필요하다.

FatFs는 SDIO/SPI 같은 물리 인터페이스를 직접 알지 않는다. FatFs가 요구하는 것은
`disk_initialize`, `disk_status`, `disk_read`, 필요 시 `disk_write`,
`disk_ioctl` 같은 block device glue 함수다. 따라서 `micro_sd` 라이브러리는
파일 시스템을 직접 다루기보다 512-byte sector 단위 read/write/ioctl만 제공하는
raw block device driver로 두는 편이 맞다.

## Current Library State

`micro_sd.h`에는 FatFs 연결에 필요한 API 모양이 어느 정도 잡혀 있다.

- `micro_sd_init_card()`
- `micro_sd_get_status()`
- `micro_sd_read_block()`
- `micro_sd_write_block()`
- `micro_sd_ioctl()`
- `micro_sd_get_handle()`

하지만 `_micro_sd.c` 구현은 아직 초기화 실험 단계에 가깝다. 현재는 SPI clock을
낮추고, CS high 상태에서 dummy clock을 보내고, CMD0을 전송해 SPI mode 진입을
시도하는 수준이다. 실제 FatFs 연결에 필요한 block read/write 동작은 아직 구현되어
있지 않다.

## FatFs Connection Point

FatFs glue 계층은 대략 다음 형태가 된다.

```c
DSTATUS micro_sd_disk_initialize(BYTE pdrv) {
    micro_sd_handle_t *handle = micro_sd_get_handle();

    if (micro_sd_init_card(handle) == MICRO_SD_STATUS_OK) {
        return 0;
    }

    return STA_NOINIT;
}

DSTATUS micro_sd_disk_status(BYTE pdrv) {
    micro_sd_handle_t *handle = micro_sd_get_handle();

    return micro_sd_get_status(handle) == MICRO_SD_STATUS_OK ? 0 : STA_NOINIT;
}

DRESULT micro_sd_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count) {
    micro_sd_handle_t *handle = micro_sd_get_handle();

    return micro_sd_read_block(handle, buff, sector, count) == MICRO_SD_STATUS_OK
               ? RES_OK
               : RES_ERROR;
}
```

STM32Cube 방식의 FatFs를 다시 사용할 경우에는 `Diskio_drvTypeDef`를 만들고
`FATFS_LinkDriver()`로 붙이는 방식이 자연스럽다. 순수 Elm-Chan FatFs 방식이면
`diskio.c`의 `disk_initialize/read/write/ioctl`에서 직접 `micro_sd_*`를 호출하면
된다.

## Missing Implementation

FatFs에 붙이려면 아래 구현이 필요하다.

- SD SPI command 전송 함수 재작성
- R1, R3, R7 응답 읽기 구현
- CMD0 이후 CMD8, CMD55+ACMD41, CMD58 초기화 시퀀스 구현
- SDv1, SDv2, SDHC/SDXC 카드 타입 판별 및 handle에 저장
- SDSC byte addressing / SDHC block addressing 차이 처리
- CMD17 단일 블록 read 구현
- CMD18 다중 블록 read 구현
- data token `0xFE` 대기 및 512-byte payload 수신
- CRC 2바이트 discard 처리
- CMD12로 multi-block read 종료
- CMD9로 CSD 읽기
- CSD v1/v2 파싱을 통한 sector count 계산
- `micro_sd_ioctl()`에서 `CTRL_SYNC`, `GET_SECTOR_COUNT`,
  `GET_SECTOR_SIZE`, `GET_BLOCK_SIZE` 처리
- read-only로 시작하지 않을 경우 CMD24/CMD25 기반 write 구현

## Design Notes

`micro_sd_file_t`, `micro_sd_file_header_t`는 FatFs를 사용할 계획이라면
라이브러리 책임과 맞지 않는다. 파일 열기, 읽기, seek는 FatFs의 `FIL`,
`f_open`, `f_read`, `f_lseek`가 담당해야 한다. `micro_sd`는 sector 단위
block device 역할까지만 담당하는 편이 구조가 명확하다.

RTOS 환경에서는 SPI 접근 동기화도 필요하다. LCD와 microSD가 같은 SPI bus를
공유하거나 DMA 타이밍 제약이 있다면, 상위 task 또는 driver 내부에서 mutex로
동시 접근을 막아야 한다.

## Recommended Order

1. `_micro_sd.c`를 `micro_sd.c`로 정리하고 빌드 대상에 추가한다.
2. `micro_sd_send_command(cmd, arg, crc)`와 응답 파서를 구현한다.
3. `micro_sd_init_card()`에서 SDv1/v2/SDHC 초기화를 완료한다.
4. `micro_sd_read_block()`에 CMD17/CMD18 기반 read를 구현한다.
5. FatFs disk I/O glue 파일을 작성해 `micro_sd_*` 함수로 연결한다.
6. 우선 read-only FatFs 설정으로 동작을 검증한다.
7. 필요 시 write 및 ioctl 확장 기능을 추가한다.

