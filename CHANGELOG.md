# 변경 이력

<!-- language-nav:start -->
[한국어](CHANGELOG.md) | [English](CHANGELOG.en.md)
<!-- language-nav:end -->

[최신 릴리스](https://github.com/gelb2/HeavyMach/releases/latest) · [GitHub 전체 릴리스](https://github.com/gelb2/HeavyMach/releases)

## Tauri Mac 다운로드 — 2026-11-02

[다운로드](https://github.com/gelb2/HeavyMach/releases/tag/tauri-mac-20261102) · Apple Silicon·macOS 14 이상. 앱과 라이선스 고지, 소스 링크를 ZIP으로 제공한다. 앱 버전은 0.17.0이며 게임 묶음은 `3665a237a9dccd69`다. ad-hoc 서명을 적용하고 ZIP 압축해제 후 실행·슬롯·복원 입력·종료를 확인했다. Developer ID 공증과 다른 Mac의 첫 실행 검사는 남아 있다. 기존 v0.17 정식 릴리스는 유지한다.

## main 통합 — 웹·PWA·Tauri 및 복원 코드

[fastutlego](https://github.com/fastutlego)의 [PR #24](https://github.com/gelb2/HeavyMach/pull/24) 웹 이식을 기반으로 순수 웹 배포, 오프라인 PWA, Tauri Apple Silicon Mac 실행기를 추가했다. 기여자의 커밋·작성자 기록을 보존한다.

- 설치 우선 시작 화면, 다운로드 진행률·해시 확인, 오프라인 캐시와 업데이트를 제공한다.
- Tauri는 OS 파일 백업·복구와 종료 시 저장 대기를 처리한다. 공개 서명·공증과 Windows 빌드는 후속 과제다.
- 공통 C++ 복원 코드로 세 슬롯·HUD·키·조준 설정을 옮긴다. Apple·Android 연결부와 빌드 구성 변경을 포함하며 기존 전투 코어·에셋은 유지한다.
- iPhone 14개·Galaxy 40개 기능 검사를 통과했다. iPhone 버튼 원본 캐시 19,344바이트 보관을 수정했다.
- 추가했던 HUD 도구·복원·키 설정·치트 버튼을 원작 게임 버튼 절차로 교체했다. 텍스트 입력·클립보드·파일 선택은 플랫폼 기능을 사용한다. 원작 버튼과 전투 코드는 유지한다.
- Android PWA 전체 화면에서도 게임 안의 복원 버튼을 표시한다. 추가 영어 문구는 원작 OK·미션 선택 버튼을 참고한 정적 글자 시트를 사용한다.
- iPhone·iPad·Galaxy 실기기와 웹/PWA에서 편집·복원·키 입력·원작 전투를 검사했다. 네이티브 Mac 프레임 검사는 원작 타이틀·전투를 포함해 기준을 넘었다. [수정·화면·검사 결과](docs/reviews/game-button-incident-20261101.md#game-button-results).

[플랫폼 설명·검사](docs/reviews/mac-web-pwa-comparison-20261101.md) · [복원 모듈](Modules/SaveTransfer/README.md). 새 버전 태그와 설치본 릴리스는 별도 기록한다.

| 버전 | 태그 기준일 | 주요 변경 |
|---|---|---|
| [v0.17](#v017) | 2026-10-30 | Android 앱·업그레이드 창 오류 수정 |
| [v0.16](#v016) | 2026-10-29 | 럭스루나에 진입 배치 |
| [v0.15](#v015) | 2026-10-29 | 앱 아이콘·HUD 편집 진입 정리 |
| [v0.14](#v014) | 2026-10-29 | HUD 배치 편집 |
| [v0.13](#v013) | 2026-10-28 | 쉴카·퉁구스카 |
| [v0.12](#v012) | 2026-10-27 | 럭스루나에 전투기 |
| [v0.11](#v011) | 2026-10-27 | 자동조준·전체 화면 도어 |
| [v0.10](#v010) | 2026-10-27 | Apple 플랫폼·키보드·VTOL |
| [v0.9](#v09) | 2026-10-26 | iPhone 복원판 |

<a id="unreleased"></a>

<a id="v017"></a>

## v0.17 — 2026-10-30

[Release・다운로드](https://github.com/gelb2/HeavyMach/releases/tag/v0.17) · [소스 태그](https://github.com/gelb2/HeavyMach/tree/v0.17) · [이전 버전과 비교](https://github.com/gelb2/HeavyMach/compare/v0.16...v0.17)

### Android 일반·치트 앱 추가

기존 C++ 게임 코어와 이미지·음원·맵을 사용하는 Android 앱을 추가했다. Kotlin·JNI·Android C++ 호스트로 터치·틸트·키보드, 메뉴·HUD, EGL/GLES 그래픽, OpenAL 음향과 저장을 연결했다. 자동조준, 추가 항공기·지상 유닛, HUD 위치 편집을 제공한다. 앱 중단과 오디오 포커스 상실 시 정지하고 그래픽 컨텍스트가 재생성되면 자원을 복구한다.

- Galaxy SM-S731N에 일반·치트 APK를 USB와 Wi-Fi로 설치했다. 이동·점프·20종 무기 등 31개, USB 분리 후 무선 검사 11개를 통과했다.
- 첫 보스 이후 2-1 진입 시 업그레이드 창 뒤로 전투가 계속되고 버튼이 반응하지 않던 오류를 수정했다. Galaxy와 에뮬레이터에서 각각 12개 회귀 검사를 통과했다.
- 수정본으로 직접 플레이한 미션 3-4·3-5의 이미지 8장·GIF 3개·영상 3개를 공개했다. 이어서 플레이한 범위에서 오작동을 발견하지 못했다.
- Android 구현 당시 기존 Apple 소스·공용 코어·자원·빌드 구성을 유지했다. iOS Simulator와 native Mac의 일반·치트 앱 네 빌드가 통과했다.
- 현재 APK는 최적화한 debug 서명본이다. 앱 내부 버전은 `0.16-android-dev`를 유지하며 소스 릴리스 태그는 v0.17이다. 전체 캠페인 클리어, 장시간 발열·메모리, Android 태블릿·16KB 페이지 실기기와 배포 서명이 남아 있다.

[Android 설치](docs/public/build-and-install.md#android-빌드와-설치) · [개발 과정·공용 모듈·수정 내역](docs/public/android-development-20261030.md) · [Galaxy 플레이 자료](docs/public/android-gameplay-20261030.md) · [검증](docs/public/validation.md#android-검사--2026-10-30)

[소스·자원 압축본](https://github.com/gelb2/HeavyMach/releases/download/v0.17/HeavyMach-v0.17-source-with-assets.tar.gz) · [고지 ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.17/HeavyMach-v0.17-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.17/SHA256SUMS.txt)

<a id="v016"></a>

## v0.16 — 2026-10-29

[Release・다운로드](https://github.com/gelb2/HeavyMach/releases/tag/v0.16) · [소스 태그](https://github.com/gelb2/HeavyMach/tree/v0.16) · [배포 커밋 0dfb2a02](https://github.com/gelb2/HeavyMach/commit/0dfb2a02dbdfe356087b789f3ab03476bd94ce02) · [이전 버전과 비교](https://github.com/gelb2/HeavyMach/compare/v0.15...v0.16)

럭스루나에의 생성점과 진입 목적지를 가로·세로로 분산했다. 진입 중 기체가 겹치는 문제를 수정했으며 수량·무장·속도와 진입 후 AI는 유지했다.

- 14개 증원 구간·화면 5종·플레이어 높이 2종의 140조건에서 동시 진입 간격·지형·천장을 검사했다.
- 코어 23개, iPhone·iPad 기능 검사 3개씩 통과했다. 두 기기의 일반·치트 앱을 무선 업데이트하고 저장·HUD 배치를 보존했다.
- iPhone 진입·전투와 iPad 전투의 프레임 검사는 통과했다. iPad 진입 호출 간격 p95는 21.39ms로 기준 20ms를 넘었다. 수정 전 같은 조건과의 비교는 남았다.
- 전투 중에는 기존 AI에 따라 기체가 다시 모일 수 있다.
- 검사 후 앱 종료·홈 화면 확인을 무선 작업 절차에 추가했다. Mac과 iOS 시뮬레이터의 지원 범위를 Apple Silicon arm64로 정리했다.

첨부한 소스·자원 압축본은 추적 파일 2,672개를 포함한다. 새 폴더에 풀어 자원 1,504개 해시와 문서 연결을 확인했다.

[배치 수정·실기기 검사](https://github.com/gelb2/HeavyMach/blob/v0.16/docs/public/lux-formation-plan-20261029.md) · [빌드·설치](https://github.com/gelb2/HeavyMach/blob/v0.16/docs/public/build-and-install.md) · [당시 남은 과제](https://github.com/gelb2/HeavyMach/blob/v0.16/docs/usability-todo.md)

[소스·자원 압축본](https://github.com/gelb2/HeavyMach/releases/download/v0.16/HeavyMach-v0.16-source-with-assets.tar.gz) · [고지 ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.16/HeavyMach-v0.16-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.16/SHA256SUMS.txt)

<a id="v015"></a>

## v0.15 — 2026-10-29

[Release・다운로드](https://github.com/gelb2/HeavyMach/releases/tag/v0.15) · [소스 태그](https://github.com/gelb2/HeavyMach/tree/v0.15) · [배포 커밋 a442e69e](https://github.com/gelb2/HeavyMach/commit/a442e69e0f340dc76b0ca2d90208f47241194f52) · [이전 버전과 비교](https://github.com/gelb2/HeavyMach/compare/v0.14...v0.15)

앱 아이콘의 글자를 제거하고 iPhone·iPad·Mac의 일반·치트 앱에 적용했다. HUD 편집은 전투 일시정지 메뉴의 **SET GAME UI**에서 열도록 정리하고, 미션 선택 화면의 편집 버튼을 제거했다.

- GitHub·SaaS 집필 자료에 최종 쉴카·퉁구스카·HUD 화면과 캡션을 추가했다.
- README와 개발자의 변에 작성자가 제공한 문장을 반영했다.
- iPhone·iPad 시뮬레이터 각 3개, native Mac 2개 검사와 Apple 앱 4종 CI 빌드가 통과했다. 실제 두 기기에 일반·치트 앱을 무선 설치·실행했다.
- Android 아이콘 PNG를 준비했다. APK 구현과 아이콘 연결은 후속 과제였다.

첨부한 소스·자원 압축본은 추적 파일 2,652개를 포함한다. 새 폴더에 풀어 자원 1,504개 해시와 문서 연결을 확인했다.

[아이콘·화면 자료](https://github.com/gelb2/HeavyMach/blob/v0.15/docs/public/app-icon-and-media-20261029.md) · [빌드·설치](https://github.com/gelb2/HeavyMach/blob/v0.15/docs/public/build-and-install.md) · [당시 남은 과제](https://github.com/gelb2/HeavyMach/blob/v0.15/docs/usability-todo.md)

[소스·자원 압축본](https://github.com/gelb2/HeavyMach/releases/download/v0.15/HeavyMach-v0.15-source-with-assets.tar.gz) · [고지 ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.15/HeavyMach-v0.15-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.15/SHA256SUMS.txt)

<a id="v014"></a>

## v0.14 — 2026-10-29

[Release・다운로드](https://github.com/gelb2/HeavyMach/releases/tag/v0.14) · [소스 태그](https://github.com/gelb2/HeavyMach/tree/v0.14) · [배포 커밋 9025b585](https://github.com/gelb2/HeavyMach/commit/9025b585fb4777f02255b048a74a26b6f5f854e9) · [이전 버전과 비교](https://github.com/gelb2/HeavyMach/compare/v0.13...v0.14)

HUD의 버튼·계기판·상단 장식을 원하는 위치로 옮기는 편집 기능을 추가했다. 이 버전에서는 미션 선택과 전투 일시정지 메뉴의 **SET GAME UI**에서 편집한다.

- 그림과 터치 영역이 함께 이동한다. 바뀐 위치에서 메뉴·점프·무기 전환·자동조준 버튼이 동작한다.
- 멈춘 전투 또는 예시 전투를 보며 편집한다. 기본 복원·취소·적용을 제공한다.
- 오른쪽에 영문 금속 도구 패널을 배치했다. 드래그할 때 패널이 접히고 TOOLS로 다시 연다.
- 상단 장식도 보존해 별도로 옮긴다. 기존 배치 파일을 읽으며, 배치를 바꿔 APPLY를 누를 때 한 번 저장한다.
- iPhone·iPad 각각 13개, 코어 24개, native Mac 2개 검사가 통과했다. 두 기기에 일반·치트 앱을 무선 설치했다.
- Mac 프레임 처리시간 조사와 장시간 발열 검사는 후속 과제로 남았다.

첨부한 소스·자원 압축본은 추적 파일 2,626개를 포함한다. 새 폴더에 풀어 자원 1,498개 해시를 확인했다.

[HUD 화면·검사](https://github.com/gelb2/HeavyMach/blob/v0.14/docs/public/hud-improvement-plan-20261028.md) · [빌드·설치](https://github.com/gelb2/HeavyMach/blob/v0.14/docs/public/build-and-install.md) · [당시 남은 과제](https://github.com/gelb2/HeavyMach/blob/v0.14/docs/usability-todo.md)

[소스·자원 압축본](https://github.com/gelb2/HeavyMach/releases/download/v0.14/HeavyMach-v0.14-source-with-assets.tar.gz) · [고지 ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.14/HeavyMach-v0.14-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.14/SHA256SUMS.txt)

<a id="v013"></a>

## v0.13 — 2026-10-28

[Release・다운로드](https://github.com/gelb2/HeavyMach/releases/tag/v0.13) · [소스 태그](https://github.com/gelb2/HeavyMach/tree/v0.13) · [배포 커밋 bc4e80c0](https://github.com/gelb2/HeavyMach/commit/bc4e80c0aa4920adbb09e7dc9c9d9b26fe39d888) · [이전 버전과 비교](https://github.com/gelb2/HeavyMach/compare/v0.12...v0.13)

쉴카·퉁구스카 지상 적을 추가했다. 기존 적 배치를 유지하고 35개 스테이지에 총 152대를 더했다.

- 쉴카는 흰색 총탄 6발을 연사한다. 퉁구스카는 흰색 총탄 4발과 미사일 2발을 연사한다.
- 차체·포탑·포신의 파츠와 질감을 수정했다. 레이더는 고정 받침 위에서 24방향 그림으로 독립 회전한다.
- 쉴카 포탑 높이는 요청에 따라 퉁구스카와 같은 29로 맞췄다.
- 최종 제품의 코어 23개, iPad 시뮬레이터 2개, iPhone·iPad 각각 5개 검사가 통과했다. 두 기기에 일반·치트 앱을 무선 설치했다.
- 추가 적을 포함한 전체 캠페인 난이도와 장시간 발열·메모리 검사는 남았다.

첨부한 소스·자원 압축본은 추적 파일 2,555개를 포함한다. 자원 1,498개 해시를 확인했다.

[지상 유닛 화면·수정](https://github.com/gelb2/HeavyMach/blob/v0.13/docs/public/ground-aa-art-20261028.md) · [빌드·설치](https://github.com/gelb2/HeavyMach/blob/v0.13/docs/public/build-and-install.md) · [당시 남은 과제](https://github.com/gelb2/HeavyMach/blob/v0.13/docs/usability-todo.md)

[소스·자원 압축본](https://github.com/gelb2/HeavyMach/releases/download/v0.13/HeavyMach-v0.13-source-with-assets.tar.gz) · [고지 ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.13/HeavyMach-v0.13-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.13/SHA256SUMS.txt)

<a id="v012"></a>

## v0.12 — 2026-10-27

[Release・다운로드](https://github.com/gelb2/HeavyMach/releases/tag/v0.12) · [소스 태그](https://github.com/gelb2/HeavyMach/tree/v0.12) · [배포 커밋 eedf245f](https://github.com/gelb2/HeavyMach/commit/eedf245f859a5224adf8f1c4a4b010eda1b5a5bb) · [이전 버전과 비교](https://github.com/gelb2/HeavyMach/compare/v0.11...v0.12)

럭스루나에 도색의 F-22·Su-57·MiG-31을 추가했다. 기존 헬기와 VTOL 배치를 유지하고 14개 구간에 72대를 증원한다.

- 금색 조종석과 HP 765를 적용했다. 기종에 따라 미사일 3연사 또는 2발씩 3묶음을 발사한다.
- 보스전 진행에 따라 2·4·6·8대, 보스전 직전 미션의 마지막 구간에는 4대씩 등장한다.
- 기체 전체를 화면 밖에서 생성하고 진입 중에는 발사하지 않도록 수정했다.
- 미사일 연사 속도 증가는 25%에서 10%로 낮췄다. 미사일 이동 속도 25% 증가는 유지했다.
- 코어 21개, 시뮬레이터 4개, iPhone·iPad 각각 4개, native Mac 5개 검사가 통과했다. 이후 진입 수정본도 플랫폼별로 검사하고 두 기기의 일반·치트 앱을 업데이트했다.
- 수동 사격 25,200프레임 기록은 자동조준 추가 전 기준과 같았다. 추가 적의 전체 캠페인 난이도·장시간 발열·메모리 검사는 남았다.

10월 28일에 소스·자원 압축본을 새 폴더에 풀어 자원 1,482개, 코어 21개, iOS 시뮬레이터·Mac 일반/치트 앱 4종 빌드를 확인했다. OpenAL 수정 후 재링크 검사도 통과했다. 압축본은 태그의 추적 파일 2,366개를 포함한다. 고지 ZIP에는 공개 시점에 정리한 고지와 재링크 검사 도구가 들어 있다.

[전투기 구현·검사](https://github.com/gelb2/HeavyMach/blob/v0.12/docs/lux-lunae-plan-20261027.md) · [도안·게임 화면](https://github.com/gelb2/HeavyMach/blob/v0.12/docs/public/lux-lunae-design-20261027.md) · [빌드·설치](https://github.com/gelb2/HeavyMach/blob/v0.12/docs/public/build-and-install.md) · [당시 남은 과제](https://github.com/gelb2/HeavyMach/blob/v0.12/docs/usability-todo.md)

[10월 28일 소스 압축본 빌드 검사](https://github.com/gelb2/HeavyMach/blob/78d950c7a3880be7147e1717496854b1e412ea2e/docs/public/github-followup-20261028.md)

[소스·자원 압축본](https://github.com/gelb2/HeavyMach/releases/download/v0.12/HeavyMach-v0.12-source-with-assets.tar.gz) · [고지 ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.12/HeavyMach-v0.12-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.12/SHA256SUMS.txt)

<a id="v011"></a>

## v0.11 — 2026-10-27

[Release・다운로드](https://github.com/gelb2/HeavyMach/releases/tag/v0.11) · [소스 태그](https://github.com/gelb2/HeavyMach/tree/v0.11) · [배포 커밋 f0c4432f](https://github.com/gelb2/HeavyMach/commit/f0c4432f073a2b5bdea02026f279fbd91bb7b093) · [이전 버전과 비교](https://github.com/gelb2/HeavyMach/compare/v0.10...v0.11)

자동조준과 전체 화면 도어 전환을 추가했다. 일반 앱에서도 자동조준을 켜고 끌 수 있으며, 기본값은 OFF다.

- 표적 선택·전환, 포신 각도 조정과 발사 버튼을 추가했다. 버튼을 기존 HUD에 맞춰 배치하고 LOCKED·TRACK·BLOCKED 표지를 넣었다.
- C 명중 보정과 해당 버튼을 삭제했다. 자동조준에서도 탄속·분산·비행 경로를 덮어쓰지 않는다.
- 자동조준 OFF의 20무기 × 3난수 × 420프레임, 총 25,200프레임 기록이 추가 전 코드와 바이트 단위로 같았다.
- 최종 C 삭제본의 코어 20개, 시뮬레이터 입력 4개, iPhone·iPad 입력 4개씩 통과했다. 두 기기에 일반·치트 앱을 업데이트했다.
- 도어의 중앙 장식을 유지하며 외곽 철판을 화면 끝까지 확장했다. 닫힌 문 뒤 배경 노출과 경계 틈을 수정했다.
- 당시 표시시간 기준 초과와 장시간 발열 검사는 후속 과제로 남았다.

태그의 소스와 Git LFS 자원을 받아 Xcode에서 빌드한다. iPhone·iPad·native Mac의 일반·치트 앱을 제공한다.

[자동조준 검사](https://github.com/gelb2/HeavyMach/blob/v0.11/docs/public/auto-aim-ipad-20261027.md) · [도어 전환](https://github.com/gelb2/HeavyMach/blob/v0.11/docs/public/fullscreen-transitions-20261027.md) · [빌드·설치](https://github.com/gelb2/HeavyMach/blob/v0.11/docs/public/build-and-install.md) · [당시 남은 과제](https://github.com/gelb2/HeavyMach/blob/v0.11/docs/usability-todo.md)

<a id="v010"></a>

## v0.10 — 2026-10-27

[Release・다운로드](https://github.com/gelb2/HeavyMach/releases/tag/v0.10) · [소스 태그](https://github.com/gelb2/HeavyMach/tree/v0.10) · [배포 커밋 76cbd21c](https://github.com/gelb2/HeavyMach/commit/76cbd21c17d077d385be5a4695b0a598fc8e9aba) · [이전 버전과 비교](https://github.com/gelb2/HeavyMach/compare/v0.9...v0.10)

iPad·native Mac과 키보드 조작을 추가하고, 공용 코어·Apple 런타임·자원을 하나의 workspace에서 사용하도록 구성했다. 필요한 게임·검사 자원을 Git LFS에 포함했다.

- AppKit 기반 Mac 앱을 추가했다. iPad는 터치·틸트와 키보드를 함께 지원한다.
- 키보드로 이동·점프·조준·사격·무기 전환·메뉴를 조작하고 키를 다시 지정할 수 있다.
- Mac 개발 서명·화면 제출 대기·전체 화면 문제를 수정했다. Mac 33개 검사와 누수 0건을 확인했다.
- iPhone·iPad의 무선 설치·실행·LLDB 연결을 확인했다. 당시 두 기기 기능 검사에서는 각각 85개 중 84개가 통과했고, 표시시간 비교 검사 1개씩이 실패했다.
- Yak-141·Harrier GR3·F-35B를 14개 서브미션에 총 113대 추가했다. 기존 적 1,182항목을 유지했다.
- 채택한 실물형 도안의 외곽선과 크기를 수정하고 후방 불꽃만 남겼다. VTOL 60건 피격 검사와 두 기기의 새 기체 표시 검사를 통과하고 일반·치트 앱을 업데이트했다.
- 프레임 기준 초과와 추가 적의 전체 캠페인 난이도·장시간 발열 검사는 남았다.

태그의 소스와 Git LFS 자원을 받아 빌드한다. 이 버전부터 자원을 별도로 복사하지 않고 저장소에서 받을 수 있다.

[Apple 플랫폼 검사](https://github.com/gelb2/HeavyMach/blob/v0.10/docs/public/apple-platform-progress-20261026.md) · [VTOL 도안·검사](https://github.com/gelb2/HeavyMach/blob/v0.10/docs/public/vtol-art-and-loadout-20261027.md) · [빌드·설치](https://github.com/gelb2/HeavyMach/blob/v0.10/docs/public/build-and-install.md) · [당시 남은 과제](https://github.com/gelb2/HeavyMach/blob/v0.10/docs/usability-todo.md)

<a id="v09"></a>

## v0.9 — 2026-10-26

[Release・다운로드](https://github.com/gelb2/HeavyMach/releases/tag/v0.9) · [소스 태그](https://github.com/gelb2/HeavyMach/tree/v0.9) · [배포 커밋 4e17f5ef](https://github.com/gelb2/HeavyMach/commit/4e17f5efd786095dfcd74a7334229908954da930)

iPhone에서 전투·성장·미션·보스·메뉴·저장이 동작하는 복원판을 첫 정상 플레이 기준으로 고정했다.

- 렌더링·음향·저장 시점·게임 진행 시간·틸트 입력을 수정했다.
- 필살기 탄착 위치, 미션 클리어 일러스트, 넓은 화면과 메뉴 배경을 수정했다.
- 일반 앱과 별도 저장을 쓰는 치트 검증 앱을 제공했다.
- 전체 미션을 직접 플레이했을 때 크래시·오동작·적 이동 이상이 보이지 않았다는 확인을 받았다.
- 독립 저장소로 옮긴 뒤 코어 17개와 문서 연결 검사, arm64 시뮬레이터 일반·치트 빌드를 통과했다. 제품 소스는 기존 설치본과 같은 Git 트리임을 확인했다.
- 전체 화면 단일 도어, 추가 물리 회전 검사와 장시간 발열 검사는 이후 과제로 남았다.

v0.9 태그는 게임 자원을 저장소에 포함하기 전의 소스다. 당시 자원은 별도로 보관했다. 소스와 자원을 함께 받는 구성은 v0.10부터 제공한다.

[첫 릴리스 기록](https://github.com/gelb2/HeavyMach/blob/v0.9/docs/release-v0.9.md) · [설치·검사 기록](https://github.com/gelb2/HeavyMach/blob/v0.9/docs/release-review-20261025.md) · [당시 남은 과제](https://github.com/gelb2/HeavyMach/blob/v0.9/docs/usability-todo.md)

## 문서 작업 기록

### 2026-10-28 — 왜 럭스루나에인가

두 게임의 개인적인 의미, 럭스루나에의 기종·도색·전투 기획, Codex로 확인하려던 것과 제작 결과를 원고로 정리했다. 최종 도안 3장·게임 화면 4장·전투 영상을 배치하고 README·개발자의 변·문서 목록·기능·개발 과정·트러블슈팅에서 연결했다. 편대 밀집은 후속 과제로 기록했다. 게임 버전은 v0.12다.

### 2026-10-28 — 도입문·기여 안내·자동 검사·소스 릴리스

README와 개발자의 변에 같은 도입문을 추가했다. About·Topics, 버그·기능 제안·PR 양식, 문서·자원·코어·Apple 빌드 workflow를 구성했다. v0.12 압축본의 자원1,482개·코어21개·앱 빌드4개·OpenAL 재링크2개를 확인했다. 외부 고지와 공유 이미지, 자원 공개 범위 목록을 추가했다. [검사 결과와 남은 항목](docs/public/github-followup-20261028.md)

버전은 Git의 출시 태그다. 앱 번들 버전과 별도로 관리한다. 출시 태그는 develop을 main에 병합한 커밋을 가리킨다.

### 2026-10-28 — GitHub 문서 main 반영

`docs/github-publication`에서 README와 기능·조작·설치·구조·트러블슈팅·개발 과정·검증 문서를 작성했다. 플레이 이미지 8장, GIF 2개와 MP4 3개를 배치했다. 게임 소스·에셋은 v0.12와 같다. develop `0a2c0e33`을 거쳐 main `db634ce6`에 병합했다.

후속 문서 변경에 새 코드용 MIT·적용 범위·자원 조건·외부 고지와 기술 배지5개, 환경 표, GitHub 구성 점검·계획을 추가했다.
