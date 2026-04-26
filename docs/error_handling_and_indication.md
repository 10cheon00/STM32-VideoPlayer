# Error Handling and Indication

- 기능 개요: 시스템은 오류 발생 시 재생을 중단하고 오류 상태로 전환하며, LED 점등 패턴과 오류 화면으로 상태를 표시한다.
- 기능 설명: 이 기능은 `error_handler_get_error_code()`와 `error_handler_handle_error()`로 구현된다. 비디오 계층 상태값을 오류 코드로 변환한 뒤, 무한 루프에서 오류 코드별 LED 패턴을 출력하고 microSD 관련 오류는 LCD에 단색 화면으로도 표시한다.
- 문서 생성 날짜: 2026-04-27
- 마지막 수정 날짜: 2026-04-27
- 문서 버전: v1.0.0

```mermaid
flowchart TD
    A["비디오 상태 오류 발생"]
    B["error_handler_get_error_code 호출"]
    C{"오류 상태 분류"}
    D["디스플레이 초기화 오류 코드 선택"]
    E["디스플레이 출력 오류 코드 선택"]
    F["microSD 마운트 오류 코드 선택"]
    G["microSD 파일 탐색 오류 코드 선택"]
    H["microSD 파일 읽기 오류 코드 선택"]
    I["프레임 타이밍 오류 코드 선택"]
    J["error_handler_handle_error 호출"]
    K["LED 초기 상태 비활성화"]
    L["무한 오류 처리 루프"]
    M["오류 코드별 LED 패턴 선택"]
    N{"microSD 관련 오류"}
    O["오류 색상으로 픽셀 배열 채우기"]
    P["라인 단위 오류 화면 출력"]
    Q["오류 상태 유지"]

    A --> B --> C
    C --> D --> J
    C --> E --> J
    C --> F --> J
    C --> G --> J
    C --> H --> J
    C --> I --> J
    J --> K --> L --> M --> N
    N -- 예 --> O --> P --> Q --> L
    N -- 아니오 --> Q --> L
```
