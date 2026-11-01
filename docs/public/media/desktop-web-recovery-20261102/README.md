# Mac 앱·웹·설치 PWA의 플레이와 복원

2026년 11월 2일 실제 앱 창을 촬영했다. 파일별 크기·해시·빌드·촬영 경로는 [provenance.json](provenance.json)에 있다.

| 환경 | 플레이 | 코드 생성 | 코드 확인 | 복원 확인 | 복원 후 슬롯 |
| --- | --- | --- | --- | --- | --- |
| Chrome 웹 탭 | [플레이](web-gameplay.png) | [생성](web-create.png) | [확인](web-check.png) | [복원 확인](web-confirm.png) | [슬롯](web-restored.png) |
| Chrome에 설치한 PWA | [플레이](pwa-gameplay.png) | [생성](pwa-create.png) | [확인](pwa-check.png) | [복원 확인](pwa-confirm.png) | [슬롯](pwa-restored.png) |
| Tauri Mac 앱 | [플레이](tauri-gameplay.png) | — | — | [복원 확인](tauri-confirm.png) | [슬롯](tauri-restored.png) |

웹과 PWA는 별도 localhost 주소에 새 슬롯을 만들어 촬영했다. 화면에 보이는 복원 코드는 레벨 1 검사 저장에서 생성했다. PWA는 Chrome의 설치 절차를 거친 독립 앱 창이다.

Tauri의 복원 확인·슬롯 화면은 같은 제품 빌드에서 저장을 실제로 복원한 검사 원본이다. 게임·HUD·설정의 복원 전후 값이 같음을 검사했다. 플레이 화면은 같은 앱에서 다시 촬영했다. 촬영 후 사용자 저장 폴더를 촬영 전 상태로 돌리고 파일 해시가 같은지 확인했다. 사용자 복원 코드가 보이는 화면은 수록하지 않았다.

이미지는 잘라내거나 다시 그리지 않은 원본 PNG다.
