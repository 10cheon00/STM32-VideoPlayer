# LCD Module Initialization

- 기능 개요: 시스템은 ST7789 LCD 모듈을 초기화하여 화면 출력이 가능한 상태로 만든다.
- 기능 설명: 이 기능은 `st7789_init_handle()`과 `st7789_init_display()`로 구성된다. 먼저 SPI, GPIO, 해상도, DMA 설정을 핸들에 저장하고, 이후 리셋 신호와 ST7789 명령 시퀀스를 전송하여 디스플레이 동작 모드를 설정한다.
- 문서 생성 날짜: 2026-04-27
- 마지막 수정 날짜: 2026-04-27
- 문서 버전: v1.0.0

```mermaid
flowchart TD
    A["초기화 시작"]
    B["st7789_init_handle 호출"]
    C["핸들에 SPI GPIO 해상도 DMA 설정 저장"]
    D{"DMA 사용 가능"}
    E["DMA 완료 플래그 활성화"]
    F["DMA 완료 플래그 비활성화"]
    G["st7789_init_display 호출"]
    H["RST 핀 토글"]
    I["SWRESET 명령 전송"]
    J["대기"]
    K["SLPOUT 명령 전송"]
    L["대기"]
    M["NORON 명령 전송"]
    N["INVON 명령 전송"]
    O["DISPON 명령 전송"]
    P["MADCTL 명령 전송"]
    Q["COLMOD 명령 전송"]
    R["LCD 초기화 완료"]

    A --> B --> C --> D
    D -- 예 --> E --> G
    D -- 아니오 --> F --> G
    G --> H --> I --> J --> K --> L --> M --> N --> O --> P --> Q --> R
```
