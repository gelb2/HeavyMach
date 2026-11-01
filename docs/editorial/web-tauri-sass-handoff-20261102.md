# PWA·Tauri·첫 PR·버튼 수정 — Sass 집필 안내

2026-11-02. 확인 기준은 HeavyMach main `a119ea6f`다. Apps의 기술 설명과 Mags의 인터뷰에 쓸 원문·구현·검사·이미지를 연결한다. 전체 자료 목차는 [집필 자료](../public/publication-inventory.md)다.

## 먼저 읽을 세 문서

1. [첫 PR을 받다. 그 다음 이어진 스케일업, 그리고 타우리](../public/first-pr-and-tauri-20261031.md): 웹판 기여를 받은 경험, 기여 보존 요구, 승인 리뷰 누락, 기존 앱을 유지하면서 플랫폼을 늘리려던 판단.
2. [서버 없이 진행도를 옮기는 복원 코드](../public/save-recovery-20261102.md): 서버 없는 이전을 제안한 이유, 원작 슬롯·공용 모듈, 저장·복원, 실제 화면 13장.
3. [게임 버튼 조사와 경위서](../reviews/game-button-incident-20261101.md): 1~10절은 수정 전 조사, 11절은 교체 후 구현·검사·배포다. SET GAME UI의 그림·눌림·입력 방식은 2절에 있다.

개인적인 동기와 어조는 최신 [개발자의 변](../RESTORATION.md)과 [제공 원문 안내](../developer-background.md)를 따른다. 기존 Sass 원고의 발문·종결절은 이번 기술 자료로 다시 쓰지 않는다.

## 현재 이용할 수 있는 버전

| 대상 | 현재 상태 | 설치·근거 |
|---|---|---|
| iOS·iPadOS | 네이티브 일반·치트 앱. 게임 버튼과 복원 코드 반영 | [설치](../public/build-and-install.md), [Apple 검사](../evidence/game-buttons-apple-20261102.json) |
| Android | C++ 코어를 쓰는 네이티브 일반·치트 앱. 웹뷰 앱으로 대체하지 않음 | [Android 개발](../public/android-development-20261030.md), [Galaxy 검사](../evidence/game-buttons-android-20261102.json) |
| 네이티브 Mac | AppKit 앱 유지. 버튼·복원 기능 검사 완료. 프레임 비교 후속 조사 있음 | [플랫폼 검증](../public/validation.md) |
| 웹·PWA | GitHub Pages 공개. 브라우저 플레이, 홈 화면 설치, 다운로드 후 오프라인 플레이·저장 | [실행](https://gelb2.github.io/HeavyMach/), [배포 후 검사](../evidence/game-buttons-web-postdeployment-20261102.json) |
| Tauri Mac | Apple Silicon·macOS 14 이상용 ZIP 공개. 웹뷰 저장 외 OS 파일 백업 | [다운로드](https://github.com/gelb2/HeavyMach/releases/tag/tauri-mac-20261102), [배포 검사](../evidence/tauri-mac-download-20261102.json) |
| Tauri Windows | 후속 개발 과제 | [개발 계획](../desktop-web-plan-20261031.md) |

전체 플랫폼을 묶은 다음 정식 릴리스는 준비 중이다. 기존 정식 태그는 v0.17, Tauri 다운로드 태그는 `tauri-mac-20261102`다. Tauri 앱 내부 표시는 0.17.0, iPhone은 1.13, Android는 0.16-android-dev다. [릴리스 이력](../../CHANGELOG.md)과 [현재 구현 상태](../public/implementation-status.md)를 함께 본다.

## 첫 PR에서 플랫폼 확장까지

**출발점.** fastutlego가 [PR #24](https://github.com/gelb2/HeavyMach/pull/24)로 C++ 코어의 WebAssembly 실행, WebGL2, 웹 입력·음향·저장과 HUD 편집을 기여했다. 개발자는 웹으로 게임을 돌리는 발상을 이 PR에서 처음 접했다고 말했다. 이미 완성한 iOS·Android를 유지하면서 웹·Mac·Windows로 확장할 방법을 찾았다.

**개발자의 요구.** 곧바로 병합하지 않고 develop 차이와 리베이스 위험, 기존 플랫폼 영향, 저장·복구, 커밋과 Contributors 기록을 조사하게 했다. 기여자의 시간을 존중하고 작업 내역을 온전히 남기려는 요구였다. [사용자 원문](../references/developer-background/user-first-pr-20261031.md).

**수정과 병합.** 웹 업그레이드 중 전투 정지, 저장 읽기·쓰기 실패, WebGL 컨텍스트 복구를 후속 수정했다. 원저자의 19개 커밋은 그대로 두고 두 개의 수정 커밋을 추가했다. PR #24를 develop에 병합하고 PR #68로 main에 반영했다. Codex는 별도 Approve 리뷰를 남기지 않고 병합했다. 개발자가 이 누락을 지적했다. 병합과 승인 리뷰는 이 사건에서 따로 기록할 사항이다.

**기여 기록.** 당시 main의 19개 원본 커밋·작성자 연결과 GitHub Contributors 통계를 확인했다. README·CREDITS·CHANGELOG에도 이름과 PR을 연결했다. [기여 보존 증거](../evidence/pr24-merge-preservation-20261031.json), [후속 수정·검사](../evidence/pr24-web-fixes-20261031.json), [크레딧](../../CREDITS.md).

**확장 판단.** 개발자는 Electron을 다뤘을 때의 구조·자원 사용 경험 때문에 Tauri와 비교해 달라고 요청했다. Tauri는 기존 HTML·JavaScript·Wasm을 쓰고 OS 웹뷰와 Rust 파일 기능을 연결한다. 순수 웹·PWA는 같은 게임을 브라우저에서 실행한다. Tauri가 모든 조건에서 더 빠르다는 비교 결과는 아직 없다. [Tauri/Electron 비교](../reviews/pr24-web-review-20261031.md#tauri-electron).

초기 Tauri 과제에서는 기여자 웹 코드와 기존 Apple·Android·게임 코어를 고정했다. 새 파일은 `Desktop/Tauri/`와 `Distribution/Web/`에 뒀다. 이후 개발자가 게임 안 버튼 전면 교체를 지시하면서 Apple·Android·웹 UI 연결부도 수정했다. 원작 게임 코어와 기존 버튼·에셋은 보존했고, 기여자의 커밋을 다시 쓰지 않고 후속 변경으로 남겼다.

## PWA의 설치·오프라인·저장

개발자는 브라우저 플레이도 유지하되 PWA 설치를 먼저 안내하고, 게임 자원을 받은 뒤 오프라인에서도 플레이할 수 있게 하라고 지시했다. 시작 화면의 버튼·다운로드 진행 표시도 원작 메뉴의 느낌에 맞추도록 했다.

GitHub Pages는 정적 파일을 배포한다. 서비스워커가 게임 자원을 받아 오프라인 실행과 업데이트를 처리한다. 게임 진행은 IDBFS를 거쳐 IndexedDB에, 키·자동조준 같은 설정은 localStorage에도 저장한다. 게임 자원 캐시와 진행 저장은 서로 다르다. PWA는 별도 게임 코어를 가진 플랫폼이 아니며 같은 웹 게임을 설치 형태로 실행한다.

개발자는 iPhone PWA에서 미션 1-4까지 클리어했고, iPad에서는 키보드 점프·트랙패드 사격·기울기·자동조준과 비행기 모드 플레이를 확인했다고 보고했다. Galaxy 웹에서도 기울기와 자동·수동 조준이 동작한다고 보고했다. 이 직접 플레이 경험과 자동 검사 결과를 각각 쓴다. [플랫폼 비교와 사용자 확인](../reviews/mac-web-pwa-comparison-20261101.md).

공개 후 Galaxy와 iPad에서 이전 캐시가 보였고, 종료·재실행 뒤 새 시작 화면과 복원 버튼을 확인했다. 최신 코드가 main에 있는 것, GitHub Pages에 배포된 것, 설치 PWA에서 새 버전이 실행된 것은 서로 다른 확인 단계였다. [실제 배포 후 결과](../evidence/game-buttons-android-20261102.json), [iPad Safari·설치 PWA](../evidence/game-buttons-apple-20261102.json).

PWA 설치와 영구 저장 요청은 제공한다. 사용자가 사이트 데이터를 삭제해도 저장이 무조건 남는 구조는 아니다. 7·14·30·40일 페이지 시각 변경 검사는 통과했지만 실제 장기 미접속·브라우저 정리 검사는 남아 있다. [날짜 변경 검사](../evidence/web-date-retention-20261101.json). 앱 밖에 복원 코드를 남기는 절차가 이 손실에 대비하는 방법이다.

## Tauri Mac의 저장과 다운로드

개발자가 Tauri를 검토한 목적은 Mac·Windows 지원과 웹 저장소 밖의 데이터 보존이었다. Tauri Mac은 웹뷰 저장을 유지하면서 별도의 OS 앱 데이터 폴더에도 백업한다. 웹뷰 저장이 전부 비면 자동 복구하고, 일부만 누락되면 시작을 막고 백업 복구를 선택하게 한다.

자동조준 값 2를 백업 검사가 거부해 종료가 막힌 문제가 있었다. 허용 값을 고친 뒤 Command-Q와 종료 메뉴에서 프로세스가 끝나는지 확인했다. [종료 오류](../evidence/tauri-quit-fix-20261101.json). 이 문제의 위치는 Tauri 저장·종료 연결부였다.

11월 2일 공개 ZIP에는 앱·게임 자원·라이선스 고지·소스 링크를 함께 넣었다. 압축을 푼 앱의 실행·세이브 슬롯·복원 입력·종료와 서명을 검사했다. ZIP을 GitHub에서 다시 받아 SHA-256도 대조했다. 개발 도구나 로컬 개발 서버 없이 실행하는 구성이다. 현재 ad-hoc 서명이며 Apple 공증과 다른 Mac의 다운로드 후 첫 실행 검사는 남아 있다. [Tauri 실행기](../../Desktop/Tauri/README.md), [배포 증거](../evidence/tauri-mac-download-20261102.json).

## 복원 코드를 제안한 이유와 구현

개발자는 가장 많이 진행한 iPhone의 경험치·무기·강화·미션을 다른 기기에서도 쓰고 싶어 했다. 싱글플레이 게임의 상태는 이미 게임 안에 있는 데이터로 다시 조립할 수 있으므로 서버·계정 없이 코드로 옮기자는 제안이었다. 원작의 세 슬롯을 보존하고, 게임 중 예외를 늘리지 않도록 코드 생성 위치를 슬롯 선택 화면으로 정했다.

공통 C++ 모듈 [SaveTransfer](../../Modules/SaveTransfer/README.md)가 원작 슬롯 3개·3,624바이트와 HUD·키·자동조준 설정을 직렬화한다. 압축과 Base64url 표현, 버전·길이·CRC32 검사를 적용해 `HM1-` 코드로 만든다. 초기 표현은 ‘해시코드’였지만 실제 코드는 복원할 데이터를 포함한다. 암호화와 salt는 후속 지시에 따라 제외했다.

플랫폼 연결부가 실제 저장소를 읽고 쓴다. 복원 전 백업과 목표 상태를 기록하고, 적용 후 다시 읽어 비교한다. 중단되면 남은 목표 상태를 다음 실행에 재적용한다. 코드로 옮기는 것은 생성 시점의 저장이며 자동 동기화·진행 병합은 하지 않는다. Apple 화면은 전체 슬롯·설정 복원, Android·웹·PWA·Tauri 화면은 전체 또는 선택 슬롯 복원을 제공한다.

Apps의 구현 설명과 Mags의 기획 설명은 [복원 코드 원고](../public/save-recovery-20261102.md)를 사용한다. 코드 경로는 `Modules/SaveTransfer/`, Apple `Sources/iOS/HMTransfer.inc`, Android `GameActivity.kt`, 웹 `Modules/SaveTransfer/web/`다. 전투·탄도·이동 코어와 복원 모듈을 구분한다.

## 버튼 재조사와 교체

**이미 있던 구현.** SET GAME UI는 원작 `GameMenuSub.png`의 글자·바탕을 조합하고 일반·눌림 그림, 기존 버튼 상태, 같은 좌표의 입력 판정으로 동작했다.

**다르게 만든 부분.** 이후 HUD 도구·복원·키 설정·치트 조작에 UIKit/AppKit, Kotlin에서 생성한 Android Button, HTML 버튼을 따로 사용했다. Android의 이 버튼은 XML 레이아웃으로 만든 것이 아니다. Galaxy PWA에서는 게임 문서만 전체 화면이 되면서 바깥 문서의 복원 버튼이 보이지 않았다. 데스크톱 검사만으로 모바일 전체 화면의 버튼까지 확인했다고 보고한 것도 잘못이었다.

**지시와 교체.** 개발자는 게임 실행 후 필요한 버튼은 원작 버튼 절차로 만들고 실제 텍스트 입력처럼 필요한 부분만 플랫폼 기능을 쓰라고 지시했다. `HMGameButtons.hpp`에 기존 `HMButtonState`와 원작 금속판을 연결하고, 그림과 입력이 같은 사각형을 사용하게 했다. HUD 도구·복원·키 설정·치트·저장 재시도 버튼을 옮겼다. 원작 버튼과 SET GAME UI는 유지했다.

**글자와 입력 수정.** 첫 글자 시트가 판타지 게임처럼 보인다는 지적 후 원작 버튼을 기준으로 영문 정적 PNG를 다시 만들었다. 매 프레임 글자를 그리는 방식은 쓰지 않는다. COPY 완료 메시지가 복원 확인 요약을 덮는 문제, 창을 닫을 때 남은 손가락 입력이 뒤 게임으로 전달되는 문제, Mac 편집 단축키도 수정했다.

**확인 결과.** 원작 이미지·게임 코어·기존 HUD·메뉴·버튼 상태 파일 1,453개는 작업 전 해시와 같았다. 빌드 자원 1,505개 검사도 통과했다. iPhone·iPad·Galaxy·웹/PWA·Tauri에서 버튼과 복원을 검사하고, main·Pages 배포 뒤 다시 검사했다. 네이티브 Mac 프레임 기준 초과와 이전 빌드 비교는 남아 있다. [보존 검사](../evidence/game-buttons-preservation-20261102.json), [버튼 경위서 11절](../reviews/game-button-incident-20261101.md#game-button-results).

작업 방식에 관한 개발자의 강한 비판과 가설은 [제공 원문 전체](../usability-todo.md#hm-ui-incident-writing--추가-버튼-구현과-작업-방식에-관한-글감--2026-11-02)에 보존했다. “AI가 가장 짧고 쉬워 보이는 경로를 택한다”는 저자의 해석과, 실제로 별도 UI를 만든 코드·결함을 구분해 쓴다. Skill Set 원 개발팀과 Google 경력 출처 확인은 같은 TODO에 남아 있다.

## Apps에 추가할 순서

| 순서 | 절의 내용 | 사용할 자료·그림 |
|---|---|---|
| 1 | 현재 플랫폼과 바로 실행하는 방법 | 웹/PWA 주소, Tauri ZIP, 네이티브 설치 링크 |
| 2 | 외부 웹 기여와 폴더·공유 구조 | fastutlego·PR #24, `Sources/Web`, `Web`, `Distribution/Web`, `Desktop/Tauri` |
| 3 | 설치·다운로드·오프라인과 저장 | PWA 플레이, Tauri 플레이, 저장소 차이 |
| 4 | 서버 없이 코드로 진행 옮기기 | CREATE → CHECK → RESTORE → 복원 후 슬롯 |
| 5 | 버튼 구현 오류와 수정 | SET GAME UI 원작 영역 조합, Galaxy 전체 화면 결함, 수정한 HUD·복원 화면 |
| 6 | 현재 검사와 남은 작업 | 실기기·공개 배포 검사, Mac 계측·공증·Windows·장기 보존 |

개발 기간·속도 수치는 Android 개발 기록에 있는 범위만 사용한다. 이번 PWA·Tauri 작업에 Android의 개발 시간을 옮겨 쓰지 않는다.

## Mags에 보강할 질문과 답변 자료

| 질문 | 이미 받은 답변의 요지 | 원문·근거 |
|---|---|---|
| 첫 외부 PR을 받고 무엇을 고민했나? | 기여자의 시간과 기록을 보존하고, 기존 앱 유지와 새 플랫폼 확장을 함께 고민했다. | [첫 PR 원문](../references/developer-background/user-first-pr-20261031.md) |
| 웹판을 보고 기존 앱을 버리지 않은 이유는? | 정상 동작하는 네이티브를 유지하면서 웹·Mac·Windows에 기여를 활용하려 했다. | [첫 PR 원고](../public/first-pr-and-tauri-20261031.md) |
| 왜 Tauri와 PWA를 함께 요구했나? | 설치·플레이 경로를 넓히고, OS 백업과 오프라인 실행을 마련하려 했다. | [개발 계획](../desktop-web-plan-20261031.md), 이 안내의 저장·설치 절 |
| 왜 서버 대신 복원 코드였나? | 싱글플레이의 상태를 코드로 옮기면 계정·서버 없이 기기를 바꿀 수 있다고 판단했다. | [복원 코드 원고](../public/save-recovery-20261102.md) |
| 버튼 문제에서 무엇이 잘못됐다고 봤나? | 원작의 구현이 있는데 플랫폼별 버튼을 새로 만들어 관리 지점을 늘리고 전체 화면에서 누락시켰다. | [사용자 글감](../usability-todo.md#hm-ui-incident-writing--추가-버튼-구현과-작업-방식에-관한-글감--2026-11-02), [경위서](../reviews/game-button-incident-20261101.md) |
| 승인 리뷰 누락을 왜 별도로 기록했나? | 기술적 병합뿐 아니라 기여자에 대한 예우와 검토 기록을 요구했다. | 첫 PR 원고의 두 번째 절 |

답변의 ‘나’는 기획·판단·직접 플레이를 한 개발자다. 웹판은 fastutlego의 기여, 분석·후속 구현·검사는 Codex가 맡은 일로 쓴다. 새로 받지 않은 감정이나 답변을 만들지 않는다. [문장 스킬](../../.agents/skills/write-project-text/SKILL.md)을 적용하고 원문 인용·기술 설명·현재 상태를 대조한다.

## 이미지와 배치

| 설명할 장면 | 파일·목록 | 용도 |
|---|---|---|
| 웹·PWA·Tauri 전투 비교 | [13장 갤러리](../public/media/desktop-web-recovery-20261102/README.md)의 `web-gameplay`, `pwa-gameplay`, `tauri-gameplay` | 세 실행 형태 소개 |
| 코드 생성과 확인 | 같은 폴더의 `web-create`, `web-check`, `pwa-create`, `pwa-check` | 복원 코드 사용 순서 |
| 복원과 재실행 | `web-confirm`, `web-restored`, `pwa-confirm`, `pwa-restored`, `tauri-confirm`, `tauri-restored` | 확인창·복원 후 상태 |
| 원작 방식의 SET GAME UI | [일반](../public/media/hud-editor/pause-menu-normal.png), [눌림](../public/media/hud-editor/pause-menu-pressed.png) | 기존 구현 설명 |
| 교체한 HUD·복원·치트 | [버튼 경위서의 실제 화면](../reviews/game-button-incident-20261101.md#game-button-results) | 플랫폼 버튼 교체 결과 |
| iOS·Android 정식 플레이 | [iOS 갤러리](../public/ios-gameplay-gallery-20261030.md), [Galaxy 갤러리](../public/android-gameplay-20261030.md) | 기존 플랫폼과 함께 배치 |

웹·PWA 10장은 Mac Chrome의 별도 검사 주소와 레벨 1 저장에서 찍었다. Tauri 3장은 실제 앱이며 복원 확인·재시작 화면에는 Lv19·미션 7-6·무기 14개가 보인다. PWA 이미지를 iPhone 실기기 화면으로 표기하지 않는다. [촬영 환경·해시](../public/media/desktop-web-recovery-20261102/provenance.json)를 각 캡션과 연결한다. 사용자의 실제 복원 코드와 비공개 저장 파일은 공개 미디어에 넣지 않는다.

[통합 보고서의 복원·플레이 화면](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/save-recovery.html#recovery-gallery), [첫 PR 원고](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/first-pr-and-tauri.html), [버튼 경위서](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/game-button-incident.html#game-button-results)는 같은 보고 사이트에 있다. 보고 사이트는 기존 소유자 로그인으로 열고, GitHub 원고와 이미지는 공개 경로로 읽는다.

## 집필 전 마지막 확인

- 최신 main의 README·개발자의 변과 Sass의 최신 원고를 읽고 사용자가 직접 바꾼 문장을 유지한다.
- Apps의 코드·저장·플랫폼 설명과 Mags의 경험·판단을 같은 사건 자료에서 작성한다. 원문과 기술 결과의 연결을 남긴다.
- Windows, Tauri 공증·다른 Mac 첫 실행, 네이티브 Mac 프레임 비교, 실제 장기 미접속, 앱 버전 표기·작은 화면 줄바꿈은 [후속 과제](../public/roadmap.md)에서 현재 상태를 다시 확인한다.
- 그림별 기기·앱 종류·촬영 조건과 기여자 이름·PR 링크를 확인한다. 성능 수치는 측정한 기기·장면·시간을 함께 적는다.
