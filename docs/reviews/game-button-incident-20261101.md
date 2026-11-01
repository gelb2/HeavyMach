# 게임 버튼 별도 구현 조사와 경위서

작성: Codex · 최초 조사 2026-11-01, 수정·검사 2026-11-02. 조사 기준은 develop `aa2af710`이다. 1~10절은 교체 전 구현과 조사한 수정 범위, [11절](#game-button-results)은 실제 수정과 검사 결과다.

## 1. 조사 결과

게임이 실행된 뒤 쓰는 HUD 편집 도구와 복원 진입 버튼을 UIKit·AppKit·Android UI·HTML로 각각 구현했다. Android 키 설정과 치트 조작부에도 별도 버튼이 있다. 게임에서 이미 사용하던 에셋·버튼 상태·터치 판정을 이 버튼들에 일관되게 적용하지 않았다.

멈춤 메뉴의 `SET GAME UI`는 다른 방식이다. 원작 `GameMenuSub.png`의 글자와 바탕을 조합하고, 게임 렌더러·메뉴 이동·눌림 상태·입력 목록에 연결했다. 이후 복원 진입 버튼을 추가하면서 이 구현을 재사용하지 않고 HUD 편집 도구의 네이티브 버튼 생성 함수를 호출했다. 웹 복원 버튼은 새 HTML UI로 만들었다.

Android 웹에서는 게임 iframe 내부 문서가 전체 화면이 되지만 복원 버튼은 바깥 문서에 남았다. 버튼의 표시 조건이 참이어도 화면에 보이지 않았다. 출시 전 복원 UI 검사는 데스크톱 Chrome의 마우스 입력을 사용했으며 모바일의 터치 전체 화면 진입을 검사하지 않았다.

11월 2일에는 각 버튼을 게임 안으로 옮길 때 바꿔야 할 파일과 호출 경로를 추가 조사했다. [교체 범위와 방법](#game-button-replacement)에 정리했다.

## 2. SET GAME UI를 만든 방법

### 원작 그림에서 글자를 가져왔다

원작 멈춤 메뉴 이미지 `GameMenuSub.png`는 256×240 크기다. 기존 메뉴의 일반·눌림 상태가 40픽셀 높이의 행으로 들어 있다. 신규 문구를 시스템 글꼴로 그리지 않고 기존 문구에서 S, E, T, G, A, M, U, I 글자 영역을 가져와 `SET GAME UI`를 조합했다.

`HMGameHUD.mm`의 `drawHUDLayoutButtonAtX:y:`에는 글자별 원본 좌표와 폭이 있다. 예를 들어 S는 `(25, 80, 16)`, E는 `(66, 0, 13)`, T는 `(68, 161, 13)`을 사용한다. `VISIT` 행에서 가져온 글자는 다른 행과 기준선이 1픽셀 달라 그 좌표를 반영했다. 글자 사이 간격은 1픽셀로 처리한다. 원작 파일은 덮어쓰지 않는다.

### 바탕과 눌림 상태도 원작 그림을 사용했다

- 양 끝은 원작 버튼의 좌우 20픽셀 영역을 사용한다.
- 글자 사이와 남는 길이는 원작의 빈 바탕 1픽셀 영역으로 채운다.
- 글자 이미지 아래에 바탕을 중복해서 그리지 않는다. 반투명 픽셀이 겹쳐 글자 주위만 어두워지는 문제를 피한다.
- 일반 상태와 눌림 상태는 원본 세로 좌표를 40픽셀 이동해 선택한다. 글자 색·그림자·배경색 변화도 해당 원작 픽셀에서 가져온다.
- 결과 크기는 기존 메뉴와 같은 256×40이다. UI 프레임워크의 버튼이나 새 래스터 이미지를 만들지 않고 `HMMedia.drawRegion`으로 필요한 영역을 게임 화면에 그린다.

![SET GAME UI 일반 상태 — 2026-10-29 보존 캡처](../public/media/hud-editor/pause-menu-normal.png)

![SET GAME UI 눌림 상태 — 2026-10-29 보존 캡처](../public/media/hud-editor/pause-menu-pressed.png)

### 그림과 입력을 같은 게임 버튼에 연결했다

버튼 식별자는 `104`다. 기존 `_buttonStates`의 눌림 상태와 `yOffset`을 사용하고, `_buttons`에 그림과 같은 256×40 터치 영역을 등록한다. `originalButtonContains`와 기존 입력 수신 순서로 판정한다. 키보드 포커스도 같은 버튼 목록을 읽는다.

멈춤 메뉴 배치는 `RESUME GAME → SET GAME UI → SAVE & EXIT GAME → VISIT INDIEAN.COM` 순서다. 세로 위치는 235·190·145·100으로 45씩 떨어져 있다. 기존 `_menuX` 이동 계산으로 메뉴가 들어오고 나간다. 멈춤 상태의 정상 전투에서 `104`를 누르면 `openHUDEditor`를 호출하고 `UI_Click`을 재생한다. 적용·취소 뒤에는 멈춤 메뉴로 돌아간다.

근거: [게임 버튼 그림·판정](../../Sources/iOS/HMGameHUD.mm), [104번 버튼 동작](../../Sources/iOS/HMGameView.mm), [당시 검사와 캡처](../evidence/hud-editor-20261029.json), [렌더링 정책](../rendering-policy.md).

`SET GAME UI`는 복원판에서 추가한 기능이다. 원작 에셋과 복원한 버튼 처리에 연결했다.

## 3. 원작 방식을 따르지 않은 버튼 전체 목록

Sources/iOS·Sources/Mac·Sources/Web·Android/app/src·Modules/SaveTransfer/web·Web·Distribution/Web/pwa·Desktop/Tauri/frontend의 생성 코드와 소비 호출부를 조사했다. 버튼 객체, 대화상자 액션, 클릭 가능한 목록, 숨긴 보조 버튼을 포함했다. 생성 코드가 있는 것과 실제 표시되는 조건을 나누어 적었다.

### 일반 앱과 웹 게임 안에서 사용하는 버튼

| 묶음 | 버튼 이름 | 구현·소비 파일 | 표시 조건·입력 |
|---|---|---|---|
| Apple HUD 도구 6개 | TOOLS, PREVIEW, SAMPLE SCENE / CURRENT SCENE, RESET LAYOUT, CANCEL, APPLY | `Sources/iOS/HMHUDEditor.inc`의 `editorButton`; iOS·iPad는 UIButton, Mac은 NSButton | HUD 편집 중. target/action으로 `hudEditorAction` 호출 |
| Android HUD 도구 6개 | 위와 동일 | `GameActivity.kt`의 `metalButton`, Android Button | 편집 중 tools 상태에 따라 표시. `GameEngine.command`로 C++에 전달 |
| 웹 HUD 도구 6개 | 위와 동일 | `Web/shell.html`의 `hudButton`, HTML button | `Sources/Web/WebHUDEditor.inc`의 상태·좌표를 DOM에 반영. `_hm_hud_control` 호출. PWA·Tauri도 이 웹 게임을 사용 |
| Apple 복원 진입 1개 | TRANSFER PROGRESS | `HMGameView.mm`에서 `editorButton` 호출 | 세이브 슬롯 화면, 로딩·키 설정·전투가 없는 상태. `openTransfer` 호출 |
| Android 복원 진입 1개 | TRANSFER PROGRESS | `GameActivity.kt`의 `metalButton` | `transferAvailable`이면 표시. `showTransfer` 호출 |
| 웹·PWA·Tauri 복원 진입 1개 | TRANSFER PROGRESS | `Modules/SaveTransfer/web/ui.js`의 HTML button | `slot-entry.js`가 슬롯·저장 상태를 검사. 웹에서는 바깥 실행기 문서에 배치해 전체 화면에서 가려졌음 |
| Android 키 설정 18개 | 아래 16개 동작 선택 + RESET KEYS + CLOSE | `GameActivity.kt`의 `showKeys`·`metalButton` | 키 설정이 열렸을 때. `keySelect`, `keysReset`, `keys` 명령 |
| 웹 저장 오류 1개 | Retry | `Web/shell.html`의 `hm-storage-retry` | 저장 오류 안내에 표시. `retryStorage` 실행 |

Android의 키 설정 동작 16개는 `Move left`, `Move right`, `Aim left`, `Aim right`, `Aim up`, `Aim down`, `Fire`, `Jump`, `Previous weapon`, `Next weapon`, `Fine movement / aim`, `Confirm`, `Pause / back`, `Next button`, `Key settings`, `Next target`이다.

Apple 키 설정은 UILabel/NSTextField로 안내문을 표시하고 키보드로 선택한다. Android의 18개 버튼과 같은 버튼 UI가 아니다.

### 같은 이름으로 남아 있는 보조 SET GAME UI

| 플랫폼 | 생성 코드 | 현재 표시 조건 |
|---|---|---|
| iOS·iPad·Mac | `HMHUDEditor.inc`의 `_hudEntry`, 네이티브 버튼 | `updateHUDEntry`가 항상 숨김. 생성 코드는 남아 있음 |
| Android | `GameActivity.kt`의 `fallback`, Android Button | `showState`가 항상 GONE. 생성 코드는 남아 있음 |
| 웹·PWA·Tauri | `Web/shell.html`의 tag -1, HTML button | HUD 배치가 유효하지 않고, 로딩·편집·키 설정·메뉴가 아니며 전투 세션이 있을 때 표시 가능 |

이 보조 버튼들은 2절의 멈춤 메뉴 버튼과 구현이 다르다. 웹에서는 전투 화면에도 보조 버튼이 나올 수 있다. Apple·Android의 후속 숨김 처리와도 차이가 난다.

### 복원 입력창 안의 버튼

| 플랫폼 | 전체 버튼·조작 | 구현 |
|---|---|---|
| iOS·iPad | CREATE CODE, RESTORE, CANCEL; 복원 확인의 RESTORE·CANCEL; 결과 안내의 OK | `Sources/iOS/HMTransfer.inc`, UIAlertController/UIAlertAction. 코드 생성 시 클립보드에 복사 |
| 네이티브 Mac | 위와 동일 | 같은 파일의 NSAlert·NSTextField·NSPasteboard 경로 |
| Android | CREATE CODE, COPY, RESTORE, CLOSE; 복원 확인의 RESTORE·CANCEL; 원본 슬롯·대상 슬롯 선택 | `GameActivity.kt`, Button·AlertDialog·Spinner·EditText |
| 웹·PWA·Tauri | CREATE CODE, COPY, SAVE CODE FILE, OPEN CODE FILE, CHECK CODE, RESTORE, CLOSE; 원본·대상 슬롯, Include HUD and controls | `Modules/SaveTransfer/web/ui.js`, HTML dialog·button·label·select·checkbox. 최종 확인은 브라우저 confirm |

키보드 입력, 붙여넣기, 클립보드, 파일 선택의 플랫폼 연결과 버튼의 그림·눌림·배치는 서로 다른 책임이다. 입력창이 OS 기능을 쓴다는 이유만으로 게임 화면의 진입 버튼까지 네이티브·HTML로 만들 필요는 없다.

### 치트 앱의 버튼과 선택 조작부

| 플랫폼 | 버튼·조작 목록 | 코드·조건 |
|---|---|---|
| iOS·iPad 화면 조작 | 검증, ←, 정지, →, 기울기 | `HMGameView.mm`, `HMValidationControls.mm`; HM_VALIDATION 빌드. 방향은 UIKit 버튼 4개 |
| Mac 화면 조작 | 검증, ←, 정지, →, 키보드 | `HMGameView.mm`, `Sources/Mac/HMValidationControlsMac.mm`; 검증 버튼과 NSSegmentedControl |
| iOS·iPad 검증 패널 | 무기 선택, 초기 조건 재설정 후 시작, 저장 이어하기, 현재 게임에 EXP 1 획득, 현재 화면으로 돌아가기; 무기 목록 각 항목·취소 | `HMValidationPanel.mm`; UIButton·UIAlertAction. 미션·스테이지·레벨·보스 Stepper, 3종 등급 SegmentedControl, 시야·EXP·전체 무기 Switch도 별도 네이티브 조작부 |
| Mac 검증 패널 | 초기 조건 재설정 후 시작, 저장 이어하기, 현재 게임에 EXP 1 획득, 현재 화면으로 돌아가기; 전체 무기·EXP·화면 전체 사용 체크박스; 무기 팝업 선택 | `Sources/Mac/HMValidationPanelMac.mm`; NSButton·NSPopUpButton. 미션·스테이지·레벨·등급·보스는 NSTextField |
| Android 화면 조작 | TEST, ◀, ▶, JUMP, FIRE | `GameActivity.kt`; CHEAT 빌드. TEST는 편집 중 숨김. 이동·점프·사격은 전투 세션 또는 HUD 편집 중 표시 |
| Android 검증 목록 | START STAGE, INVINCIBLE / UNLIMITED AMMO ON, CHEATS OFF, ADD EXPERIENCE, RESUME, KEY SETTINGS, SAVE DIAGNOSTICS, RESUME SAVED GAME | 같은 파일의 AlertDialog 목록 8개. 미션 입력창의 START·CANCEL 포함 |
| 검사 오류 확인 | Apple 저장 없음의 확인/OK, Mac 시작 조건 오류의 기본 확인 | `HMGameView.mm`, `HMValidationPanelMac.mm`의 시스템 경고창 |

치트 조작 기능은 사용자가 요청했다. 이 목록은 그 기능의 승인 여부와 별개로 실제 버튼을 무엇으로 만들었는지를 기록한다. 웹·PWA·Tauri 게임에는 별도 치트 패널이 없다.

### 게임 시작 전 실행기와 OS 메뉴

게임 안 버튼과 섞어 빠뜨리지 않도록 별도 기록한다.

- 웹 실행기: INSTALL GAME, PLAY IN BROWSER / PLAY, RETRY DOWNLOAD, 설치 안내 CLOSE. `Distribution/Web/pwa/index.html`·`host.js`의 HTML이다.
- Tauri 실행기: PLAY, RETRY, EXPORT SAVE, IMPORT SAVE, RESTORE LAST BACKUP, 복원 확인의 CANCEL·RESTORE. `Desktop/Tauri/frontend/index.html`·`boot.js`의 HTML이다. 웹 실행기의 파일 백업 버튼은 제거했지만 Tauri 실행기에는 남아 있다.
- 네이티브 Mac의 전체 화면·종료는 `Sources/Mac/Application.mm`의 OS 메뉴다. 게임 버튼 목록과 구분한다.

## 4. 외형을 맞춘 것과 구현을 통일한 것은 달랐다

Apple HUD 도구는 `Common_Btn_OK.png`에서 양 끝 8픽셀과 중앙 2픽셀을 가져와 168×44 버튼판을 만들고 CoreText의 DINCondensed-Bold로 글자를 합성했다. 결과를 UIButton/NSButton에 붙인다. 생성 시 합성하며, SAMPLE SCENE/CURRENT SCENE 문구가 바뀔 때 해당 이미지를 다시 만든다.

Android는 같은 금속판을 Bitmap·Canvas로 합성해 캐시하고 Android Button이 시스템 글꼴로 글자를 그린다. 웹 HUD 도구는 Canvas 2D로 금속판과 글자를 합성해 캐시한 뒤 HTML 버튼 배경으로 사용한다. 웹 복원 버튼은 별도 skin/button.png와 CSS border-image, Arial 글꼴을 사용한다.

![HUD 편집 도구 APPLY — 원작 금속판에 새 글꼴을 합성한 2026-10-29 산출물](../public/media/hud-editor/release-tool-normal.png)

이 버튼들의 그림 생성·글꼴·포커스·터치 수신·레이아웃은 각 플랫폼의 UI가 관리한다.

`docs/rendering-policy.md`에도 편집 도구 모음은 UIKit/AppKit을 쓴다고 적혀 있었다. 11월 2일 버튼 교체와 함께 게임 버튼 절차로 수정했다.

Android 웹에서는 전체 화면에 들어가면 복원 버튼이 가려진다. Android 키 설정은 UI 상태가 바뀔 때 목록을 지우고 다시 만든다.

## 5. 구현과 배포의 경위

| 날짜·커밋 | 수행한 작업 | 이번 조사에서 확인한 문제 |
|---|---|---|
| 10월 29일 `8aa85ebd` | HUD 편집 기능 추가 | 편집 도구를 별도 네이티브 UI로 만듦 |
| 10월 29일 `0d435579` | 멈춤 메뉴 SET GAME UI 추가 | 원작 글자·바탕·눌림·입력 방식을 사용하는 구현을 확보 |
| 10월 29일 `24dc8d7f` | HUD 도구 디자인·상단 장식 수정 | 도구 외형은 개선했지만 네이티브 버튼 구현을 유지 |
| 웹 기여 `87bf7d2ac` | 기존 Apple HUD 도구를 웹으로 이식 | fastutlego가 당시 Apple 도구 구조를 HTML로 옮김 |
| Android `9d536ce0` | Android 실행기 구현 | 기존 편집 도구와 검증 조작을 Android Button으로 구현. 키 설정도 별도 버튼으로 추가 |
| 11월 1일 `9aed4591` | 공통 복원 코드와 플랫폼 UI 연결 | Apple·Android는 편집 도구의 버튼 함수를 재사용하고 웹 복원 UI는 새 HTML로 추가 |
| main `5025b23f` 배포 뒤 | 사용자가 Galaxy에서 복원 버튼 누락을 제보 | 모바일 전체 화면에서 바깥 문서 버튼이 보이지 않는 조건을 놓침 |

나는 복원 진입 버튼에 기존 editorButton·metalButton을 사용했고 웹에는 HTML 버튼을 추가했다. 이미 구현해 둔 SET GAME UI의 그림·입력 처리를 사용하지 않았다.

이후 사용자에게 버튼 위치를 설명할 때도 코드의 슬롯 표시 조건을 근거로 답했다. Galaxy에서 실제로 보이고 눌리는지 확인한 결과가 아니었다. 네이티브 기기의 데이터 이전 검사와 데스크톱 웹 검사를 모바일 PWA 버튼 검증까지 끝난 것처럼 설명한 것이 잘못이다.

복원 진입 버튼에는 OS 입력창 기능이 필요하지 않았다. 게임 버튼으로 만들 수 있었다. Android 모바일 전체 화면 검사도 배포 전에 빠뜨렸다.

## 6. Android 전체 화면 결함과 검사 누락

Galaxy에서 확인한 상태는 바깥 문서의 fullscreenElement가 IFRAME, 게임 문서의 fullscreenElement가 HTML이었다. 복원 버튼은 바깥 문서에 존재했고 hidden=false, display=flex였다. 게임의 복원 가능 값도 1이었다. 저장이 로드되지 않아 숨겨진 문제가 아니었다.

`Web/shell.html`은 터치 기기에서 게임 문서를 전체 화면으로 전환한다. `Distribution/Web/pwa/host.js`는 복원 UI를 바깥 document.body에 붙였다. 전체 화면에 포함되는 문서와 버튼의 소속 문서가 달랐다.

배포 당시 `browser-slots.mjs`는 1024×768 데스크톱 Chrome에서 page.mouse로 조작했다. 설치형 PWA 검사도 데스크톱 창이었다. `hasTouch`, 모바일 전체 화면 진입, fullscreenElement 확인, 전체 화면에서의 실제 버튼 클릭을 포함하지 않았다. 슬롯 조건·코드 생성·복원 성공만으로 모바일 표시까지 확인할 수 없었다.

수정 중인 로컬 코드에서는 버튼과 입력창을 게임 문서로 옮겼다. 터치 전체 화면 검사에서 버튼 표시와 위치 판정까지 확인했으나, Puppeteer 클릭 호출이 `Node is either not clickable or not an Element`로 실패했다. 클릭부터 복원까지 통과하지 않았으므로 수정 완료가 아니다. 실패를 제품 클릭 실패와 검사 도구 좌표 문제 중 어느 쪽으로 확정할지도 남아 있다.

iPhone Safari에서는 설치 권장 문구와 INSTALL GAME을 직접 보았다. 사용자가 앞서 안내를 보지 못한 원인은 확인하지 못했다. Android 버튼 결함과 같은 원인으로 단정하지 않는다.

## 7. 설명 중 바로잡을 내용

- “원작 버튼 방식으로 만들었다”는 설명은 멈춤 메뉴의 SET GAME UI에 해당한다. 편집 도구·복원 버튼까지 포함하지 않는다.
- Android 버튼은 Kotlin으로 만든 Android View다. XML 레이아웃 버튼이라고 했던 표현은 정확하지 않다.
- 웹 HUD 편집 UI는 PR 기여자가 기존 Apple 구조를 이식한 코드다. Codex가 추가한 복원 UI와 작성 경위를 구분한다.
- 과거 게임 전체를 CPU/UIKit 이미지 합성으로 그렸던 성능 문제와 이번 버튼 문제는 구현 범위가 다르다. 이번에 같은 프레임 드랍을 재현한 것은 아니다.
- 앞서 구현 편의를 이유로 설명했다. Git에서 확인되는 사실은 기존 editorButton/metalButton 재사용과 새 HTML 추가다. 이를 반드시 써야 했다는 기술적 근거는 확인하지 못했다.

## 8. 남은 수정과 확인

| 대상 | 수정할 내용 | 확인할 동작 |
|---|---|---|
| 복원 진입 버튼 | 원작 에셋·게임 버튼 렌더·터치 목록으로 옮기는 변경 범위를 정함 | 슬롯에서만 표시, 웹/PWA 전체 화면, 터치·키보드, 저장 중 진입 차단 |
| HUD 도구 | 원작 스타일의 일반·눌림 에셋과 게임 입력 경로를 사용하고 플랫폼별 중복 버튼 제거 | 도구 펼침·접힘, 장면 전환, 적용·취소·초기화, HUD 드래그와 입력 충돌 |
| 보조 SET GAME UI | 멈춤 메뉴 진입과 중복된 숨김 코드 및 웹 전투 중 표시 조건 정리 | 이동·전투 중 노출 없음, 멈춤 메뉴에서 편집 가능 |
| Android 키 설정 | 게임 UI 사용과 입력 수집을 분리하고 반복 생성 정리 | 16개 동작 재할당, 초기화, 취소, 게임 입력에 키 누출 없음 |
| 복원 입력창 | 텍스트·클립보드·파일 연결을 별도 책임으로 유지하며 버튼 외형·동작 대조 | 붙여넣기, 잘못된 코드, 복원 확인·취소, 기존 저장 백업 |
| 치트 조작·오류 안내 | 위 목록 전체를 교체 대상 검토에 포함 | 치트 전용 조건, 정상 앱 미노출, 오류 재시도 기능 보존 |
| 문서·검사 | 렌더링 정책·현재 상태·플랫폼 안내를 실제 구현과 함께 갱신 | 기능명만 확인하지 않고 각 플랫폼의 표시·입력·저장까지 대조 |

교체할 파일과 순서는 아래 10절에 정리했다.

## 9. 근거 파일

- [Apple 편집 도구 생성·표시](../../Sources/iOS/HMHUDEditor.inc)
- [Apple 복원 입력창](../../Sources/iOS/HMTransfer.inc)
- [Android 버튼·키 설정·복원·검증 패널](../../Android/app/src/main/java/com/gelb2/heavymach/GameActivity.kt)
- [웹 HUD 도구·전체 화면·저장 Retry](../../Web/shell.html)
- [웹 HUD 보조 진입 조건](../../Sources/Web/WebHUDEditor.inc)
- [공통 웹 복원 UI](../../Modules/SaveTransfer/web/ui.js)
- [웹 복원 연결부](../../Distribution/Web/pwa/host.js)
- [Apple 검증 패널](../../Sources/iOS/HMValidationPanel.mm) · [Mac 검증 패널](../../Sources/Mac/HMValidationPanelMac.mm)
- [기존 복원 검사 기록](../evidence/save-transfer-slots-20261101.json)
- [조사 범위·후속 기준](../ui-implementation-audit.md)

<a id="game-button-replacement"></a>

## 10. 게임 버튼으로 바꿀 때 수정할 범위

2026-11-02 소스 조사 결과다. **버튼 이미지 교체만으로 끝나지는 않는다. 그림·입력 판정·기능 호출을 게임 UI에 연결하고 기존 네이티브·HTML 버튼을 제거해야 한다. 게임과 앱 전체를 다시 작성할 필요는 없다.** 이후 사용자 지시로 교체 구현과 검사를 시작했다.

### 그대로 쓸 수 있는 코드

`SET GAME UI`는 이미 원작 메뉴 그림을 잘라 그리며 일반·눌림 상태와 터치 영역을 게임에 등록한다. Apple의 `HMGameHUD.mm`, Android의 `HUD.hpp`, 웹의 `WebGameHUD.cpp`에 이 경로가 있다. 세 렌더러 모두 `drawRegion`으로 이미지 일부를 그릴 수 있다.

눌림 상태는 `HMButtonState.hpp`에 복원한 QobButton 처리다. 이 파일은 Sources/iOS 아래에 있지만 내용은 C++이며 Android와 웹도 읽는다. 새 버튼에도 이 상태 처리를 사용할 수 있다. 버튼 모양과 글자는 원작 에셋을 먼저 사용하고, 필요한 글자가 없으면 정적 에셋을 보충한다. `GameMenuSub.png`만으로 모든 새 문구를 만들 수 있다고 가정해서는 안 된다. TRANSFER의 F처럼 기존 메뉴 문구에서 얻을 수 없는 글자도 있다.

HUD 적용·취소·초기화, 무기 선택, 치트 조건 설정, 복원 코드 생성·검사·적용 함수는 이미 있다. 새 게임 버튼이 이 함수를 호출하도록 바꾼다.

### 버튼별 작업량

| 대상 | 바꿀 부분 | 재사용할 부분 | 상대적인 작업량 |
|---|---|---|---|
| 멈춤 메뉴 SET GAME UI | 새로 교체할 필요 없음. 비교 기준으로 사용 | 원작 그림 조합·눌림·메뉴 이동·입력 | 없음 |
| 숨긴 보조 SET GAME UI | Apple·Android의 생성 코드를 제거. 웹의 전투 중 표시 경로도 제거 | 멈춤 메뉴의 104번 버튼 | 작음 |
| HUD 편집 도구 6개 | 게임 렌더·터치 영역·키보드 포커스를 추가. native/DOM 도구 생성·배치 제거 | 현재 장면·예시 장면, 드래그, 초기화·취소·적용과 저장 | 중간 |
| 슬롯의 TRANSFER PROGRESS | 슬롯 화면에 게임 버튼 추가. 게임 안의 복원 패널을 여는 동작 연결 | 슬롯 조건, 코드 변환·검사·백업·복원 | 중간 |
| Android 키 설정 | 16개 동작·초기화·닫기를 게임 화면에 표시. 선택·키 입력 대기·목록 이동 연결 | 키 할당 값과 저장, 실제 키 이벤트 수신 | 중간 |
| 치트 이동·점프·사격 버튼 | 스프라이트와 누름·뗌 처리를 게임 입력에 연결 | 현재 검증용 이동·사격 명령 | 중간 |
| 치트 설정 패널 | 목록·선택·스크롤·확인·닫기 화면을 게임 UI로 옮김 | 미션·무기·경험치·강화 조건과 검증 실행 함수 | 큼 |
| 복원 입력창 내부 버튼·슬롯 선택 | 생성·복사·검사·복원·취소와 선택 표시를 게임 UI로 옮기고 입력창 구성도 변경 | 코드 입력·붙여넣기·클립보드·파일 선택, 복원 모듈 | 큼 |

마지막 두 항목은 여러 버튼과 선택 목록이 있는 화면이다. 버튼 배경만 바꾸면 기존 플랫폼 UI가 계속 남는다. 숫자·복원 코드 입력에 필요한 텍스트 필드와 OS 키보드는 연결부로 유지할 수 있다. 화면의 버튼·바탕·선택 표시까지 OS UI로 만들 필요는 없다.

### 플랫폼별 수정 파일과 연결 방법

| 플랫폼 | 수정할 주요 파일 | 연결 방법 |
|---|---|---|
| iPhone·iPad·네이티브 Mac | `Sources/iOS/HMGameView.mm`, `HMHUDEditor.inc`, `HMTransfer.inc`와 관련 선언 | 게임 그리기·입력 경로에 버튼을 추가. `hudEditorAction`의 sender.tag 의존을 동작 번호로 분리. transferEntry·editorButton 생성과 배치를 제거하고 게임 복원 패널에서 기존 복원 함수 호출 |
| Apple 치트 | `Sources/iOS/HMValidationControls.mm`, `HMValidationPanel.mm`, `Sources/Mac/HMValidationControlsMac.mm`, `HMValidationPanelMac.mm` | 화면 버튼·패널을 게임 UI에 연결. 조건 설정·검증 실행 코드는 유지 |
| Android | `Android/app/src/main/cpp/Runtime.cpp`, `Runtime.hpp`, `Android/app/src/main/java/com/gelb2/heavymach/GameActivity.kt`, `GameEngine.kt` | Runtime에서 버튼을 그리고 입력을 받음. 기존 command 동작을 재사용. GameActivity의 도구·키 목록·치트 버튼 제거. 텍스트 입력·클립보드 요청만 게임 스레드에서 메인 스레드로 전달 |
| 웹·PWA·Tauri | `Sources/Web/WebGameView.cpp`와 헤더, `WebHUDEditor.inc`, `WebMain.cpp`, `Web/shell.html` | 게임 캔버스 안에 버튼을 그림. HTML HUD 도구와 보조 진입 제거. 버튼 동작을 기존 편집·복원 함수에 연결 |
| 웹 복원 연결부 | `Modules/SaveTransfer/web/ui.js`, `slot-entry.js`, `Distribution/Web/pwa/host.js`, `Desktop/Tauri/frontend/transfer-game.js` | HTML 버튼·dialog를 제거. 복원 패널은 게임에서 그리고 텍스트 필드·파일 선택만 연결. 저장 로드·동기화 중 진입 차단 유지 |

Android에서 게임은 HandlerThread에서 돌고 입력창은 메인 스레드에서 열린다. 현재 GameEngine이 상태를 메인 스레드에 전달하는 경로를 활용할 수 있다. 열기 요청은 한 번만 소비해야 한다. 매 프레임 같은 창을 다시 여는 값으로 만들면 안 된다.

웹의 버튼을 게임 캔버스에 그리면 게임과 함께 전체 화면에 들어간다. **현재 Android PWA에서 버튼이 전체 화면 밖에 남는 구조를 없앨 수 있다.** 코드 입력창도 전체 화면 문서 안에 열어야 한다. 입력창이 열린 동안 키보드·터치가 뒤의 슬롯 메뉴나 게임으로 전달되지 않게 한다. 코드 복사·파일 선택은 각 기능 버튼의 직접 입력에서 실행한다.

Tauri는 이 웹 게임을 포함한다. Tauri 전용 게임 버튼을 다시 만들 필요는 없다. Rust의 파일 백업·복구·종료 코드는 버튼 교체와 별개로 유지할 수 있다.

### 화면과 입력에서 함께 고쳐야 할 부분

현재 세 플랫폼의 `drawLogicalFrame`은 HUD 편집 화면과 메뉴 화면을 그린 뒤 곧바로 반환한다. HUD 클래스에 버튼만 추가하면 슬롯 화면에서는 그 코드가 실행되지 않는다. 편집 화면과 슬롯 화면 각각에 그리는 호출을 넣어야 한다.

현재 편집 중 입력은 곧바로 `hudEditorPoint`로 들어간다. 도구 버튼을 먼저 판정하고, 버튼에서 시작한 터치를 뗄 때까지 그 버튼이 받아야 한다. 그렇지 않으면 APPLY를 누르는 동작이 뒤에 있는 HUD 부품을 드래그할 수 있다. 치트 사격·이동 버튼은 누른 상태와 뗀 상태를 모두 전달해야 한다.

그림과 터치 영역은 같은 게임 좌표를 사용한다. 메뉴의 세로 중앙 보정, HUD 사용자 배치, 안전영역, 회전·창 크기 변경을 적용한 뒤 같은 사각형으로 그리기와 입력을 판정한다. 도구를 그릴 때 월드 흔들림·카메라 이동·HUD 클립을 물려받지 않도록 한다.

키보드도 수정 대상이다. 현재 편집 중 Tab은 HUD 묶음을 고르고 방향키는 그 묶음을 이동한다. 도구 버튼 포커스를 추가할 때 이 동작과 충돌하지 않게 연결해야 한다. 편집 중 전투 시간은 멈추므로 버튼 눌림 애니메이션을 위해 전투를 다시 진행시키면 안 된다.

### 원작 버튼 코드 재사용과 빌드

이미 복원한 `HMButtonState`와 원작 버튼 영역 판정·스프라이트 그리기를 재사용한다. 추가 버튼의 등록·그리기·입력 호출을 `HMGameButtons.hpp`에 모아 각 플랫폼에서 사용한다. 기존 원작 버튼 함수는 그대로 둔다. 모든 버튼 문구와 그림은 같은 정적 에셋을 사용한다.

공통 코드는 기존 포함 경로에서 읽는 헤더로 구성한다. 웹 호스트 연결 함수는 `Web/CMakeLists.txt`의 내보내기 목록에 등록한다. 원작 자원은 그대로 두고 추가 그림이 필요하면 `Art/Extensions`에 둔다. Apple 자원 번들·Android 준비 도구·웹 패키지는 이미 이 폴더를 사용한다. 자원 목록과 해시도 갱신한다.

웹 게임 코드가 바뀌므로 WebAssembly를 다시 빌드하고 PWA·Tauri에 새 산출물을 넣어야 한다. 실행기 HTML만 배포하면 새 게임 버튼은 생기지 않는다. PWA에서는 이전 캐시에서 새 게임 버전으로 바뀌는 것까지 검사한다.

이 변경은 기존 Tauri 실행기 폴더만으로 끝낼 수 없다. 위 표의 웹 게임 UI 파일과 Apple·Android UI 파일이 수정 대상이다. 구현할 때 보호 검사에 이 파일들의 변경 목적과 범위를 기록한다. 기존 기준 해시를 새 파일로 덮어 검사 차이를 없애지 않는다. 기여자의 커밋은 그대로 두고 UI 교체를 후속 커밋으로 남긴다.

### 유지할 코드와 OS 기능

- `Sources/Core`의 전투·탄도·이동·적 생성·데미지·경험치 계산과 원작 세이브 슬롯 구조.
- HUD 배치 값·키 할당 값의 저장 형식, `Modules/SaveTransfer`의 코드 변환·검사와 파일 백업.
- 기존 OpenGL·WebGL 렌더러와 음향·앱 생명주기 처리.
- 복원 코드와 숫자를 입력하는 OS 키보드, 붙여넣기·클립보드·파일 선택.
- 게임 자원을 받기 전의 설치·다운로드 화면과 렌더러 초기화 실패 때의 재시도 안내. 게임을 그릴 수 없는 상태에서도 접근할 수 있어야 한다.
- Mac 메뉴 막대의 종료·전체 화면 항목.

### 구현 순서와 검사

1. 원작 버튼 에셋·글자 목록을 대조하고 공통 버튼 그림·상태·입력 코드를 만든다. 일반·눌림 상태를 SET GAME UI와 비교한다.
2. HUD 도구와 슬롯 복원 진입을 옮긴다. 실제 기능은 기존 함수를 호출한다. 숨긴 보조 SET GAME UI를 제거한다.
3. 키 설정·치트 조작과 패널을 옮긴다. 코드 입력창 내부의 생성·복사·확인·취소도 모두 원작 버튼으로 옮긴다. 입력 필드·클립보드·파일 선택 연결을 유지한다.
4. 버튼마다 누름·뗌·바깥으로 드래그·취소·동시 터치를 검사한다. HUD 편집·슬롯·전투 사이에 입력이 새지 않는지 확인한다.
5. Apple·Android·웹에서 회전·크기 변경·백그라운드 복귀·그래픽 컨텍스트 복구 뒤 그림과 터치 영역을 대조한다. Android PWA 전체 화면과 iOS 홈 화면 앱에서도 복원 진입부터 실제 코드 생성·복원까지 수행한다.
6. 적용·복원 전후 저장값과 쓰기 횟수를 비교한다. 단순 누름·화면 그리기·HUD 드래그는 저장을 발생시키지 않아야 한다. 취소는 기존 데이터를 보존하고, 저장 오류는 편집 상태와 원래 슬롯을 보존해야 한다.
7. 기존 전투·자동조준·무기 전환 검사를 실행하고 일반·치트 앱을 각각 확인한다. 프레임별 이미지 합성·디코딩·파일 쓰기를 새로 만들지 않았는지 계측한다.


<a id="game-button-results"></a>

## 11. 교체한 버튼과 검사 — 2026-11-02

HUD 편집 도구, 복원 진입·슬롯 선택·생성·복사·검사·복원 확인, 키 설정과 치트 조작부를 게임 버튼으로 옮겼다. 웹 저장 실패의 RETRY도 게임에서 그린다. 원작 버튼의 그림·눌림·입력 판정과 SET GAME UI는 유지했다.

공통 `HMGameButtons.hpp`가 기존 `HMButtonState`를 사용한다. 원작 `Common_Btn_OK.png`의 일반·눌림 그림을 기존 OpenGL/WebGL 렌더러에 제출한다. 그림과 같은 사각형으로 터치와 키보드 포커스를 처리한다. 추가 버튼 문구는 모든 플랫폼에서 영어다. 복원 코드의 실제 텍스트 입력, 클립보드와 파일 선택만 OS·브라우저에 연결했다.

### 글자 수정

첫 글자 시트는 글자 끝의 장식과 굵기가 원작과 달랐다. 개발자가 판타지 게임 글꼴처럼 보인다고 지적했다. 원작 OK·SELECT MISSION·NEXT STAGE 버튼을 직접 참조해 산세리프 금색 테두리 글자를 다시 만들었다. 작은 버튼의 글자 여백과 긴 문구의 마지막 글자 잘림도 수정했다.

글자 시트는 정적 PNG다. 실행 중 폰트를 그리거나 버튼 이미지를 합성하지 않는다. [생성 원본·참조·프롬프트](../../Art/UI/20261102-original-button-lettering/provenance.json), [시트와 해시](../evidence/game-menu-glyphs-20261102.json), [게임에서 사용하는 PNG](../../Art/Extensions/GameMenuGlyphs.png).

![Galaxy HUD 편집 도구 — 원작 금속판과 새 영문 글자](../public/media/game-buttons/galaxy-hud-tools.png)

![Galaxy 일반 앱 — 복원 전 슬롯·경험치·레벨·무기 수와 확인 버튼](../public/media/game-buttons/galaxy-restore-confirm.png)

![Galaxy 치트 앱 — 이동·점프·사격 버튼과 원작 HUD](../public/media/game-buttons/galaxy-cheat-controls.png)

### 원작 파일과 기능 보존

원작 이미지, 게임 코어, 기존 HUD·메뉴·버튼 상태 코드 1,453개를 작업 전 해시와 대조했다. 변경은 없었다. 추가 글자 시트까지 포함한 빌드 자원 1,505개도 해시 검사를 통과했다. [보존 검사](../evidence/game-buttons-preservation-20261102.json).

전투·탄도·이동·적 생성·데미지·경험치 계산과 복원 코드 형식은 그대로다. 버튼의 그리기·입력 경로와 플랫폼 연결부를 수정했다. 기존 코어를 WebAssembly로 실행한 25개 검사와 네이티브 SaveTransfer 검사가 통과했다.

### 실행 검사

| 대상 | 확인한 내용 | 결과 |
|---|---|---|
| iPhone·iPad 실기기 | HUD 편집·키 설정·복원·치트 조작, 일반 앱 원작 메뉴 | 각 33개 기능 검사와 새 글자 관련 각 5개 검사 통과 |
| Galaxy 실기기 | 이동·점프·사격·HUD·키·치트·복원·잘못된 코드·그래픽 재생성 | 58개 검사 통과. 일반 앱 복원 7개, 최종 작은 버튼 6개 통과 |
| 웹 마우스·터치 전체 화면·설치형 PWA | 코드 생성·복사·파일 저장/열기·슬롯 선택·취소·실제 복원·오프라인 재실행 | 마우스 16개, 터치와 설치형 PWA 각 17개 통과 |
| 웹 HUD·키 설정 | 6개 도구, 초기화·취소·적용, 16개 키 선택·초기화 | 통과 |
| 웹 저장 오류·그래픽 복구 | 실패 후 RETRY·재로딩·컨텍스트 복구 | 통과 |
| 공통 버튼 | 영역 경계·드래그·다중 터치·비활성·키 반복·모달 종료 후 입력 | AddressSanitizer·UndefinedBehaviorSanitizer 검사 통과 |

Galaxy 검사 후 기존 일반·치트 저장 파일을 백업과 대조했고 일치했다. 일반 앱에서 생성한 코드를 확인·복원한 뒤 슬롯 내용도 대조했다. Android 검사기의 `adb exec-in`은 원격 압축 해제 종료 전에 반환하는 문제가 있었다. `adb shell -T`로 원격 종료를 기다리도록 바꾸고 파일 해시를 재확인했다.

교차 코드 검토에서는 COPY의 늦은 완료 메시지가 복원 확인 내용을 덮는 경우, 모달을 닫은 뒤 다른 손가락의 입력이 뒤 메뉴로 전달되는 경우, Android의 도구 포커스 전환 중 키 입력 잔류와 로딩 중 키 설정 진입을 추가로 찾았다. 각 조건의 재현과 수정 후 검사를 별도로 남긴다.

### 렌더 비용

Apple M4 Pro의 Chromium/ANGLE Metal에서 각 장면을 5초씩 측정했다. 전투 60.14fps, 멈춤 메뉴 60.13fps, HUD 도구 59.95fps, 미리보기 59.94fps였다. 편집 도구·미리보기 구간의 새 이미지 디코딩·업로드는 0회였다.

네이티브 Mac의 별도 프레임 검사는 기준을 넘었다. 원작 타이틀·슬롯·전투와 추가 HUD·복원창 모두에서 지연을 측정했고, WindowServer 갱신 대기 시간 초과도 기록됐다. 기존 빌드와 같은 조건의 비교가 필요하다. 이 실패는 [TODO](../usability-todo.md)에 남긴다.

### 배포

현재 기능 브랜치에서 최종 입력 회귀와 Tauri 실제 창 검사를 진행한다. develop·main 병합, GitHub Pages 배포와 공개 주소 재검사 결과는 완료 후 이 절에 추가한다.
