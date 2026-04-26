# Frame Read and Display

- 기능 개요: 시스템은 RGB565 파일에서 프레임 데이터를 읽고 LCD에 순차적으로 출력한다.
- 기능 설명: 이 기능은 `video_reader_read_file()`와 `video_player_print_video_buffer()`를 중심으로 구성된다. 파일에서 청크 단위 픽셀 데이터를 읽은 뒤, 현재 출력 범위에 맞춰 ST7789로 전송하고 성공 시 다음 출력 범위로 이동한다.
- 문서 생성 날짜: 2026-04-27
- 마지막 수정 날짜: 2026-04-27
- 문서 버전: v1.0.0

```mermaid
flowchart TD
    A["프레임 처리 시작"]
    B["video_reader_read_file 호출"]
    C["버퍼 주소 전환"]
    D["f_read 호출"]
    E{"읽기 성공"}
    F["파일 읽기 실패 상태 반환"]
    G["누적 읽기 바이트 갱신"]
    H{"파일 끝 도달"}
    I["video_context_read_restart_from_beginning 호출"]
    J["f_lseek 호출"]
    K["누적 바이트와 다음 프레임 위치 초기화"]
    L["첫 청크 다시 읽기"]
    M{"재시작 읽기 성공"}
    N["파일 읽기 실패 상태 반환"]
    O["video_player_print_video_buffer 호출"]
    P["st7789_print_pixels_with_range 호출"]
    Q["st7789_send_image 호출"]
    R{"전송 성공"}
    S["다음 출력 범위로 이동"]
    T["화면 출력 실패 상태 반환"]
    U["프레임 처리 완료"]

    A --> B --> C --> D --> E
    E -- 아니오 --> F
    E -- 예 --> G --> H
    H -- 예 --> I --> J --> K --> L --> M
    M -- 아니오 --> N
    M -- 예 --> O
    H -- 아니오 --> O
    O --> P --> Q --> R
    R -- 예 --> S --> U
    R -- 아니오 --> T
```
