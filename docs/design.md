# 소스 복원 설계와 원작 근거

## 카모프·럭스루나에 유로파이터 — 2026-11-09

`tools/prepare_unit_data_authoring.json`에 `Ka52`, `Lux_Eurofighter`와 무장·파츠를 직접 작성하고 기존 내보내기로 MCH에 추가했다. 과거 C++ factory를 실행해 새 정의를 캡처하지 않았다. 기존 레코드 바이트와 순서는 `part_record_order`로 유지한다. `GameData`의 같은 파서, `enemyParts`의 같은 조립, `drawParts/drawWeapon`과 전투 함수가 소비한다.

카모프는 원작 Heli006의 HP·EXP·공중 이동 형식과 원작 로터 type3·2프레임을 사용한다. 몸체 아래 실제 기관포·미사일포드가 별도 무장 파츠로 붙는다. 유로파이터는 기존 Lux 수치와 자세·분사 속성을 사용하며 보이는 미사일포드의 소켓에서 발사한다. `aircraft_weapon_mounts=part_sockets`는 기존 고정 총구 속성을 끄는 authoring 선택이다. 카모프 HP 바 -18은 기존 HMEX customGauge 속성이다. 어느 항목도 원작 MCH에 원래 있던 추가 필드로 설명하지 않는다.

이미지 7개는 오프라인 변환한 실제 PNG이며 플랫폼별 런타임 crop·alias·기종 draw를 추가하지 않았다. 기체별 C++ factory나 별도 표시·전투 분기를 추가하지 않았다. `HM_VALIDATION`에서만 탄환·피해를 읽는 관측 API를 추가했다. 캠페인 MAP·편대·일반 저장은 유지한다. [구현·검증·남은 범위](reviews/kamov-eurofighter-implementation-20261109.md).

## 신규 유닛 구조 정리 요구 — 2026-11-09

작업 브랜치에서 신규8종의 C++ 런타임 등록과 공중 동체 별도 draw를 제거했다. `tools/prepare_unit_data.py`가 보존된 원작 레코드와 확정된 추가 정의·좌표를 `Resources/RuntimeData`에 합성한다. `GameData::loadDirectory`와 원작 MCH/MAP 파서는 이 실행용 패키지를 읽는다. 원작 보관 파일은 그대로이며, 새 레코드가 들어간 배포 MCH/MAP의 전체 해시는 원작과 다르다.

원작 레코드로 표현되지 않는 표시·pitch·총구 기준·레이더·Lux 연사/증원·난수 정책은 버전이 있는 `UnitExtensions.hmx`로 읽는다. `PartPresentation`, `MachineBehavior`, `StageFormation`은 복원 엔진의 명시적 확장이다. 원작 형식에 새 필드가 원래 있었던 것처럼 설명하지 않는다. 모든 동체·분사·지상 부품은 공통 파츠 순회와 `drawParts/drawWeapon`·표시 속성 처리를 거친다. [자료 형식과 재생성](../tools/prepare_unit_data.md), [핫픽스 계획](reviews/new-unit-hotfix-plan-20261109.md), [경위서](reviews/new-unit-implementation-incident-20261109.md).

Core26검사가 통과했고, 별도 기준판/후보판 비교는 44조건·39,644tick에서 상태·난수·탄환·draw 순서/사각형·이벤트·저장이 같았다. 이는 실제 플랫폼 화면·설치 완료를 대신하지 않는다. 플랫폼별 검증·배포 상태는 [현재 상태](current-status.md)를 따른다.

## Apple 플랫폼 확장 — 2026-10-26

Apple 구성은 [공용 모듈](apple-platforms.md)을 따른다. 세 프로젝트를 workspace에서 연결하고 Core·AppleRuntime·Assets를 공유한다. [키보드 입력](keyboard-input-policy.md)은 GameSession 앞의 입력 계층이며 게임 수식은 유지한다.

2026-10-25에 대조한 원작 경로와 이후 플랫폼·기능 확장의 소스를 연결한다. 각 모듈의 전체 원본 동등성은 current-status와 source-coverage의 확인 범위로 제한한다. 주소는 HeavyMach 1.13 실행 파일 기준이다.

## 문서 역할

이 문서는 기술 설계다. 플레이어 경험·화면/아트 의도·기능 요구 ID는 [게임 기획](game-design.md), 문서 우선순위와 변경 전파는 [문서 위계](document-governance.md)를 따른다. 업무별 실제 코드·검사·증거 연결은 [연결표](document-map.json)가 소유한다.

## 역할과 경계

| 역할 | 원작 단서 | 복원할 책임 |
| --- | --- | --- |
| 앱·루프 | HeavyMachAppDelegate, EAGLView | 최신 iOS 생명주기·창·입력, 원작 논리 루프 |
| 화면 진행 | GameMain, GuiTitle, GuiSelectMission, GuiGame | 타이틀·슬롯·미션·전환·HUD |
| 객체·표시 | QobBase, QobManager, QobCamera, QobImage | 계층·위치·레이어·타일·카메라·알파 |
| 세계·충돌 | GobWorld, ColVector | 맵·적 생성·좌우 경계·충돌·진행 |
| 기체·무기 | GobHvMach, GobHvM_Player, GobHvM_Enemy, GobMachWeapon, GobBullet | 이동·점프·AI·탄환·피격 |
| 성장·영속화 | GlobalInfo, GameMain | 슬롯·EXP·부품·무기·저장 |
| 자원·음향 | ResMgr_Tile, SoundManager, MAP/MCH/WAV | 원본 데이터 해석·표시·음향 재생 |

현재 게임 그래픽은 HMGLRenderer의 OpenGL ES 1.1, 음향은 HMOpenALAudio의 OpenAL Soft 1.22.2를 사용한다. UIKit은 창·생명주기·터치 전달·코드 텍스트 입력을 맡는다. 검증 패널과 추가 버튼은 게임 렌더러로 그린다. SpriteRaster/TrailRaster는 비교용 CPU 구현이며 현재 제품의 프레임 합성 경로가 아니다. 게임 규칙과 플랫폼 출력 연결을 분리하여 동일 입력·상태를 비교한다.

[정본](canonical.md)을 시작점으로 [좌표](coordinate-policy.md), [렌더](rendering-policy.md), [수치](game-calculation-policy.md), [시간·저장·음향](runtime-policy.md)을 적용한다. 최신 사용자 요청의 사용성 개선은 [TODO](usability-todo.md), 시야 확장 구현과 검사는 [캔버스 기록](wide-canvas-plan.md)에 둔다. 아직 구현하지 않은 계획을 현행 동작으로 읽지 않는다.

호출 흐름은 Application → HMGameView → GameSession/World → RenderCommand → HMGLRenderer다. GameData는 원본 파생 데이터를 읽고 SaveData는 3624바이트 저장을 해석한다. 게임 이벤트의 음향은 HMOpenALAudio, 저장 이벤트는 HMGameView의 제한된 파일 쓰기로 연결한다. 일반·검증 앱은 공통 코어를 쓰며 HM_VALIDATION 전처리 조건으로 치트 패널과 방향 버튼을 분리한다.

## 확인한 함수와 조건

`C=/Users/sokol/Documents/Codex/2026-10-24/rea-ready/comparison/core`, `T=/Users/sokol/Documents/Codex/2026-10-24/heavymach-recovery/title-analysis`를 경로 약칭으로 쓴다.

| 함수·주소 | 확인 내용 | 근거 |
| --- | --- | --- |
| renderScene `0x21e0` | double g_time에 프레임마다 약 1/60 추가. wall clock 읽기 없음 | T/AppDelegate_renderScene.json |
| GuiTitle processTitle `0xf464` | start+6초 이후 TapToStart 점멸. 오디오 완료 대기 없음 | T/GuiTitle_processTitle.json |
| GuiTitle onTap `0xf934` | state2에서 intro skip 또는 changeScreen:3 | T/GuiTitle_onTap.json |
| processMyMach `0xc830` | abs(accelY)>0.03이면 clamp(accelY×15,-1,1), 아니면 0 | T/game-0xc830.json |
| setTap `0x1d300` | 전방 원거리 조준·사격, state2 해제 | C/original-function-19.json |
| GuiGame init `0x151a0`, handleButton `0x17c4c` | 부스터 ID1000, GL 좌표(34,34), PushButton에서 점프 | T/game-0x151a0.json, game-0x17c4c.json |
| setBooster `0x1cc70` | HP>0, 지상에서 점프. 공중 반복은 부품 등급·사용 횟수 조건 | T/game-booster.json |
| GobHvMach tick `0x479c` | player, speed>0, x>right−30, gameMode!=1, state!=3, bossCount<1이면 다음 스테이지 | T/game-0x479c.json |
| setNextStage `0xb8f0` | stage/mission 증가, updateLastStage, saveDataFile | C/original-function-13.json |
| addExp `0x106e0` | 누적EXP>=임계치, level<99이면 level+1·upgrade point+1. 이 함수에 저장 호출 없음 | T/game-0x106e0.json |
| calcLevelUpExp `0x105f8` | 300×pow 항 누적, 초기 배수1.5, >1.25 조건에서0.01 감소 | C/original-function-17.json, original-level-assembly.json |
| saveDataFile `0x3f44` | Documents/HvM.dat, 3624바이트 원자 저장 | C/original-function-00.json |

Ghidra가 CGPoint 인수 일부를 state의 float 비트로 표기한 사례가 있다. 예를 들어 2.8026e-45를 좌표 조건으로 옮기지 않는다. 타입·인수 순서는 호출 ABI를 확인한다.

## 데이터 계약

MAP/MCH 관찰 구조는 LE u32 `declared_size,type,data_size`, payload, LE u32 child_count, children이다. 두 판본의 92파일은 재귀 파싱 후 잔여0바이트였다. 일부 declared_size는 실제 하위 길이와 다르므로 임의의 건너뛰기 길이로 쓰지 않는다. 원시 offset·payload는 `comparison/assets/parsed-game-data.json`에 있다.

원작 WeaponInfo160, PartsInfo294, MachInfo720 레코드를 확인했다. type10/11의 적 목록·레코드 의미는 Assault reader 증거와 동일 구조로 대응했고 원작 switch는 불완전하다. 원작 reader의 type10/11 분기는 추가 확인이 필요하다. CgBI 이미지는 디코딩 파생본을 만들 수 있지만 색 순서·premultiplied alpha·합성 결과를 확인한다.

저장 3624바이트 길이만으로 필드·정렬·판본 호환성을 확정하지 않는다. ABI 구조체를 현대 컴파일러의 메모리 배치 그대로 파일에 쓰지 않고, 필드 offset·바이트 순서와 미확인 바이트 보존을 검증한다.

## 구현된 렌더 경로

원본 스프라이트를 OpenGL ES 1.1의 RGBA8 버퍼에 합성하고 최종 화면에 표시한다. 게임플레이는480×320을 최소 크기로 두고 화면 비율에 따라 폭 또는 높이를 늘린다. 타이틀/선택은 중앙 원본 배치와 주변 배경을 함께 표시하며 검증 원본 모드는480×320이다. 스프라이트는 GL_REPLACE, POT texture의 LINEAR/REPEAT, 최종 화면은 LINEAR/CLAMP_TO_EDGE를 쓴다. Tile/Trail의 blend3 차이와 상태 상속을 보존한다. 텍스처 업로드 뒤 CPU 디코딩 픽셀을 해제한다. 정확한 버퍼·좌표·캐시 정책과 검사는 [그래픽 정책](rendering-policy.md)을 따른다.

## 구현된 음향과 수명

31개 WAV의 OpenAL 버퍼와 source를 준비하고 재사용한다. 원본 위치·거리 모델·stop/play 순서를 따른다. 과거 AVAudioPlayer의 근사 pan은 현행 경로가 아니다. 원작 4개 저장 호출 지점, 50Hz 가속도 필터, 한 프레임 한 tick, 메모리/파일 저장 경계는 [실행 정책](runtime-policy.md)을 따른다. 백그라운드·잠금·120초 무조작은 실행 정책에 따라 명시 재개를 요구한다. 결과/업그레이드 상태와 정상 저장 시점은 보존한다.

## 승인된 공중 적 확장 — 2026-10-27

최초 구현은 `VTOL.cpp`가 메모리 정의를 추가하고 동체를 별도로 그렸다. 이 파일과 이름 기반 실행 분기는 2026-11-09 핫픽스에서 제거했다. 현재 세 기체는 MCH의 실제 Body/Weapon/Exhaust 파츠로 생성한다. 원작 무기 참조와 공통 비행·발사·피격 처리는 유지하고, 자세·총구 기준·피격 범위·분사 속성은 명시적 확장으로 적용한다. 2026-11-07에 확정한 35개 스테이지152개 배치는 MAP로 옮겼다. 모든 미션에 세 기종이 있고, 스테이지별 총수는 쉴카·퉁구스카 합계와 같다. [기존 요구·검사 기록](vtol-expansion.md).

## 전체 화면 도어

원본 전환 상태는 `LoadingTransition`, 캔버스별 패널 배치는 `expandedPanels`, 실제 제출은 `HMMedia.drawExpandedPanel → HMGLRenderer.drawExpandedPanel`이 담당한다. 원본 네 PNG의 중앙 장식을 유지하고 외곽 부분만 늘린다. 각 패널54정점을 한 draw로 제출하며 추가 이미지나 프레임별 파일 쓰기는 없다. 화면 교체·효과음·게임 상태는 기존 전환 이벤트를 사용한다. [결과](public/fullscreen-transitions-20261027.md).

## 럭스루나에 전투기

최초 `Fighters.cpp`와 `aircraftSpec` 등록/선택 코드는 제거했다. 기체3종과 수정 무기는 실행용 MCH로 읽고, 14개 증원표는 `StageFormation`으로 읽는다. 원작 MAP 정적 배치로 표현할 수 없는 보스 활성/마지막 구간 조건은 명시적 확장이다. 공통 증원 코드가 화면 밖 편대를 생성하고 `formationEntering` 동안 기존 위치 갱신을 사용한다. 목표점 도착 시 무기 AI 대기 시각을 시작한다. 후방 분사는 동체 트리의 공통 효과 파츠가 그린다. [기존 수치·배치·검사 기록](lux-lunae-plan-20261027.md).

## 추가 지상 적

미션별 차량14개와 무기·파츠는 실행용 MCH, 고정152개 좌표는 MAP에서 읽는다. `GroundAA.cpp`의 등록·배치 생성과 `GroundAAArt.inc` 기반 런타임 crop은 제거했다. 기존 소켓 조립·지상 AI·충돌·무기 갱신을 사용하며, 실제57개 부품 PNG와 레이더 프레임별 소수점 축·2초24프레임 시계는 공통 `PartPresentation`이 소비한다. [현재 조립·무장·배치와 과거 이력](ground-aa-expansion.md).

## Android 호스트 — 2026-10-30

`Android/app/src/main/java`의 Activity가 Surface·터치·키·센서·오디오 포커스를 받는다. `GameEngine`이 전용 HandlerThread에 JNI 호출을 순서대로 전달한다. Android C++ `Runtime`이 기존 코어와 메뉴·HUD·저장을 연결하고 `Media`와 `Audio`가 EGL/GLES·OpenAL을 담당한다. Activity가 다시 만들어져도 프로세스 내 게임 세션은 유지한다.

기존 Apple 파일과 `Sources/Core`를 수정하지 않고 이 호스트를 추가했다. 새 게임 규칙 복사본은 만들지 않았다. [폴더·빌드](../Android/README.md), [검사 범위](android-implementation-20261030.md).

## 웹 빌드

`Sources/Web`은 HMGameView·HMMenus·HMGameHUD·HMMedia·HMOpenALAudio를 C++로 옮긴 브라우저 호스트다. 호출 흐름은 WebGameView → GameSession/World → RenderCommand → Media → GLRenderer로 iOS와 같다. `WebGLRenderer`는 WebGL2 셰이더로 GL_REPLACE/MODULATE, 원본 블렌드 표, 가위 영역, float 행렬, RGBA8 장면 버퍼→복사 텍스처→최종 사각형을 재현한다. 메뉴·HUD·프레임 조립은 플랫폼별 호스트에 있다. 웹 호스트의 업그레이드 정지·저장 복구·GPU 복구 수정까지 반영했다. Tauri 착수 때 이 웹 호스트를 고정했고, 이후 승인된 복원 코드·게임 버튼 연결을 추가했다. [이식 문서](web-port-20261028.md).


## Tauri·웹 확장 계획 — 2026-10-31

Tauri Mac의 실행기·시작 전 복구·OS 파일 백업은 `Desktop/Tauri/`, 웹·PWA 시작 화면·manifest·아이콘·Service Worker는 `Distribution/Web/`가 담당한다. 게임의 IDBFS·localStorage를 유지하며, Tauri는 저장이 끝난 내용을 별도 OS 파일에도 백업한다. PWA의 게임 파일 캐시와 사용자 진행 저장은 분리한다.

Tauri Mac·웹·PWA와 복원 코드는 main에 병합했고 웹·PWA는 GitHub Pages에 배포했다. 네이티브 Mac·Tauri·웹·PWA의 비교 기록과 남은 Windows 작업은 [개발 계획](desktop-web-plan-20261031.md), [현재 상태](current-status.md)를 따른다. 착수 당시의 파일 동결 이후 사용자 승인으로 추가한 복원·버튼 연결은 [범위 기록](../Desktop/Tauri/save-transfer-scope.json)과 [버튼 교체 기록](reviews/game-button-incident-20261101.md)에 있다.

## 복원 코드 공통 모듈과 UI 후속 조사 — 2026-11-01

`Modules/SaveTransfer`는 게임 코어 밖에서 `SaveData`의 세 슬롯을 읽고 쓴다. 플랫폼별 어댑터가 저장 파일·HUD·키·조준 설정을 `Snapshot`으로 바꾸고, 공통 C++가 검증·압축·코드 해석·슬롯 선택·복원 순서를 맡는다. Apple·Android는 이 C++를 직접 링크하고 웹·PWA·Tauri는 별도 `codec.wasm`을 호출한다. 탄도·이동·전투 계산은 이 모듈에 의존하지 않는다.

`HM1-`에는 원작 저장 3,624바이트와 설정 값이 들어간다. PackBits형 압축·Base64url·CRC32를 사용하며 암호화와 salt는 없다. 코드만으로 데이터를 읽을 수 있어 서버 조회가 필요 없다. 현재 실행 중인 전투를 직렬화하지 않고 마지막 저장을 옮긴다. 같은 코드를 여러 기기에서 복원할 수 있지만 이후의 변경은 새 코드를 만들어 다시 전달한다.

Apple 복원 UI는 세 슬롯과 설정을 모두 교체한다. Android·웹·PWA·Tauri는 전체 또는 단일 슬롯을 선택하고 설정 포함 여부를 고른다. 단일 슬롯 복원은 다른 두 슬롯을 유지한다. 설정은 앱 전체에 적용한다. [형식·포함 값·플랫폼 차이](../Modules/SaveTransfer/README.md).

복원은 기존 값 보관 → 목표값 대기 기록 → 적용 → 다시 읽어 대조 → 대기 기록 삭제 순서다. 도중에 종료되면 다음 시작에서 목표값 적용을 재개한다. 웹 계열은 슬롯 화면에서 요청한 뒤 게임 페이지를 종료하고 적용해, 실행 중인 메모리가 복원 파일을 덮어쓰지 않게 한다. 플랫폼별 파일과 중단 처리는 [저장 정책](runtime-policy.md#복원-코드와-플랫폼별-저장--2026-11-02)을 따른다.


## 추가 게임 버튼 — 2026-11-02

`Sources/iOS/HMGameButtons.hpp`는 Apple·Android·웹에서 함께 읽는 추가 버튼 코드다. `HMButtonState.hpp`의 원작 눌림 상태, 원작 금속판과 정적 영문 글자 시트를 사용한다. 각 플랫폼의 `HMGameButtons.inc`, `GameButtons.inc`, `WebGameButtons.inc`가 화면별 배치와 기존 동작을 연결한다. 원작 메뉴·HUD 버튼 구현은 보존한다.

버튼 그리기와 입력은 같은 게임 좌표를 쓴다. 모달 전환 때 진행 중인 터치를 취소하고 손가락을 뗄 때까지 소비한다. 복원 텍스트 필드와 클립보드·파일 선택은 플랫폼 서비스로 연결한다. 저장은 기존 편집 APPLY·복원 확인 동작에서 실행한다. [구현 범위와 검사](reviews/game-button-incident-20261101.md#game-button-results).
