# 시간·저장·음향 정책

## Apple 플랫폼 확장 — 2026-10-26

Mac 저장 위치는 Application Support/<bundle ID>/HvM.dat다. 일반·치트 저장은 분리하며 iPhone/iPad의 Documents 경로는 유지한다. Mac 앱/창 비활성도 같은 중단 처리를 호출한다. 키 설정 저장은 게임 저장과 별도이며 배정 확정 때만 쓴다. [입력 정책](keyboard-input-policy.md).

정본: [canonical](canonical.md). 제품 소비자는 HMGameView/GameSession/GlobalInfo/SaveData/HMOpenALAudio다.

## 시간

renderScene 0x21e0 기준 한 프레임에 double 시간 `0.01666666753590107`을 한 번 더하고 게임 tick/draw를 각 한 번 실행한다. wall-clock accumulator로0회 또는 여러 회 tick하지 않는다. 비활성 상태는 진행하지 않고 복귀 시 배경 시간을 몰아서 보충하지 않는다.

프레임 예약은 호환판 ns_timer의 기한 계산을 일회용 NSTimer로 연결한다. 음향·표시의 실제 지연과 게임의 고정 시간은 구분한다. present60Hz 제한은 표시 경로에 있다. 센서 샘플은 별도의 원본50Hz 경로다. 게임 상수·센서 주기·표시 주기를 하나로 합치지 않는다.

## 저장

정상 파일 쓰기 지점은 슬롯 선택0x3b20, 다음 스테이지0xb8f0, 게임오버0xbf74, 메뉴 복귀0x17c4c다. 탄약·피격·EXP의 메모리 변경 알림은 파일 쓰기가 아니다. 사격·draw·tick·단순 background 진입에서 저장을 추가하지 않는다.

게임 저장은 3,624바이트 `HvM.dat`다. iPhone·iPad는 Documents, 네이티브 Mac은 Application Support/<bundle ID>, Android는 앱 전용 files, 웹·PWA·Tauri 게임은 IDBFS `/persist`에 둔다. 필드 offset·LE 바이트·미확인 바이트를 보존한다. 현재 C++ 구조체를 그대로 직렬화하지 않는다. 저장 이벤트 전달 전에 해당 시점 슬롯을 복사해 나중 상태가 섞이지 않게 한다. 정상 저장은 원자 쓰기를 사용하고 실패/손상 로드 시 기존 파일을 보존한다. 임의 초기화로 실패를 숨기지 않는다.

일반/검증 bundle의 저장은 분리한다. 검증 패널 초기 조건 재설정은 검증 저장에만 적용하고 이어하기는 조건을 다시 덮어쓰지 않는다. 설치 전 저장을 백업하고 업데이트 직후 실행 전 해시를 대조한다. 일반 저장을 치트 데이터로 교체하지 않는다.

## 음향

일반 제품은 OpenAL Soft1.22.2, 원본31개 WAV, 재사용 buffer/source를 사용한다. 음향 source 위치 설정→stop→play 순서와 원본 요청 순서를 유지한다. 매 발사 AVAudioPlayer 생성이나 파일 재읽기를 추가하지 않는다. 같은 이름 source 재시작과 다른 이름의 동시 재생을 구분한다.

원본 source X는 `(soundWorldX−playerX)/100` 등 호출별 위치 값이다. 단순 UIKit stereo pan[-1,1]로 대체하지 않는다. 원본 OpenAL 기본 거리 모델·source 수명과 초기화 값을 따른다. 근거 없이 음향만 수동 pause/resume하지 않는다. 사용자가 요청한 백그라운드·잠금·장기 미사용 정책은 [사용성 TODO](usability-todo.md)의 상태 전이와 원본 비교에 따라 게임·입력·음향을 함께 처리하도록 구현한다. 이 정책은 2026-10-25에 구현했다. 현대 출력장치와 구형 원본의 정확한 지연·음색 일치는 별도 관측 대상이다.

검사 근거: [직접 비교](compat-source-comparison-20261024.md), [저장/효과 수명](session-lifecycle.md), `HMOpenALAudio.mm`, `AudioContract.hpp`. 과거 AVAudioPlayer 보고는 실험 이력이며 현행 구현 지시가 아니다.


## 비활성·잠금·장기 미사용 (2026-10-25 채택)

`sceneWillResignActive`에서 게임을 정지하고 입력·OpenAL source·프레임/센서 타이머·CoreMotion 생산을 중단한다. `sceneDidEnterBackground` 중복 통지는 추가 GL 작업을 하지 않는다. `sceneDidBecomeActive`에서 표시와 센서를 다시 준비하되 전투는 메뉴의 재개를 눌러야 진행한다. foreground 콜백만으로 재개하지 않는다. 시간 보충·추가 파일 저장·이전 소리 자동 재생은 없다.

진행 중인 결과/업그레이드 선택은 유지한다. 결과 전환 도중 중단하면 일러스트 위에 재개 메뉴를 띄운다. 재개 시 남은 전환을 계속하고 완료 뒤 전투로 돌아간다. 로딩 중 중단하면 새 월드의 최초 정지 그림을 한 번 준비하며 시뮬레이션 시간을 진행하지 않는다. 정지 메뉴에서는 패시브 지속시간도 멈춘다.

foreground 전투에서 유효 조작이 120초 없으면 동일 정지 메뉴를 연다. 단조 시계로 계산하고 유지 중 터치/사격/방향키·이동 dead zone을 넘는 틸트는 조작으로 센다. 기울어진 채 놓인 폰과 실제 틸트 조작을 센서만으로 구분하지는 않는다. OS 비활성 통지가 오면 이 조건과 무관하게 즉시 정지한다. OS 종료 후에는 기존 정상 저장만 복구한다.

무기 버튼1100 release에서 원본 `0x17c4c`의 sfxID4(`Game_ChangeWeapon`)를 한 번 요청한다. press/move/cancel·paused 입력에는 요청하지 않는다. 초기 장비 선택·자동 무기 변경에 임의 음향을 추가하지 않는다.

검사: `build/lifecycle-20261025/third.xcresult`의 iOS21/21, `core-tests.log`의 Core17/17 통과. 첫 iOS10/10 이후 결과 전환/최초 정지 그림 검사를 추가했다. 두 번째 실행은 Simulator SBMainWorkspace Busy로 실행 전 실패했고 세 번째 실행에서 해결했다. 폰 잠금·출력 음향의 물리폰 재확인은 별도다.

## HUD 배치 저장·편집 — 2026-10-29

편집은 전투 시간·난수·탄약·진행을 멈춘다. 예시 전투의 슬롯·패시브·난수는 실제 플레이와 분리한다. 드래그·기본 배치·취소는 파일을 쓰지 않는다. 변경한 배치를 `APPLY`로 확정할 때 HUDLayoutV1-iphone/ipad/mac.json을 한 번 원자적으로 저장한다. 정식·치트 앱은 각 컨테이너에 저장한다. 저장 실패는 편집 화면을 유지하고 재시도를 받는다. 적용·취소 후에는 정지 메뉴로 돌아간다. 미션 선택 화면에는 편집 버튼을 두지 않는다. 백그라운드 전환 중에도 편집 정지 상태를 유지하며 강제 종료 후에는 마지막 적용값을 읽는다.

HUDLayoutV1의 기존9개 그룹 인덱스를 보존하고 upperTrim을 마지막10번째에 추가한다. 예전9항목 파일은 장식 기본값을 붙여 읽고, 새 저장은10항목을 쓴다. TOOLS·PREVIEW·드래그·RESET LAYOUT·CANCEL은 파일을 쓰지 않는다.

## 웹 실행

브라우저의 requestAnimationFrame마다 기한을 확인한다. 60Hz 콜백에서는 iOS처럼 한 번에1틱이다. 콜백 간격의 평균이 틱 간격의 배수이면(예: iOS 저전력 모드의30Hz) 한 콜백에서 기한이 된 틱을 최대 round(간격/틱)개(최대4) 진행하고 한 번 그린다. 기한보다1/4 간격 이내로 이른 콜백은 그 틱으로 처리하고, 0.1초를 넘는 멈춤 뒤 밀린 틱은 보충하지 않는다. 50Hz 센서 샘플은 별도 타이머 기한을 따른다. 저장은 IDBFS `/persist/HvM.dat`에 임시 파일→rename으로 쓰고 IndexedDB에 동기화하며, 정상 저장 지점은 iOS와 같다. 탭 숨김·창 포커스 상실·pagehide는 비활성으로 처리해 게임·입력·음향을 멈추고, 복귀 후 메뉴에서 재개한다. 터치 기기의 세로 화면은 2026-10-29 사용자 결정에 따라 '세로 보류'로 처리한다: 게임 시간·입력·음향은 멈추지만 멈춤 메뉴를 열지 않고, 가로로 돌아오면 그대로 이어진다. 보류 중 다른 중단이 오면 일반 중단으로 바꿔 멈춤 메뉴를 연다. 음향은 Emscripten OpenAL(Web Audio)로 같은 버퍼·소스·재생 순서를 쓴다. HUD 배치는 APPLY 때만 `/persist/HUDLayoutV1-web.json`에 임시 파일→rename으로 쓰고 IndexedDB에 동기화한다. 형식과 검증은 iOS `HUDLayoutV1`과 같다. [이식 문서](web-port-20261028.md).

## 복원 코드와 플랫폼별 저장 — 2026-11-02

복원 코드는 `Modules/SaveTransfer`가 원작 세 슬롯과 HUD·키·조준 설정을 읽어 만든다. 게임 정상 저장 이벤트와 별도로, 슬롯 선택 화면에서 사용자가 복원을 확정했을 때만 이 경로가 저장을 쓴다. 생성·복사·검사·확인 취소는 진행을 저장하지 않는다. 전투 중 입력, 드로잉, 프레임 갱신에 코드 생성이나 저장을 연결하지 않는다.

### 옮기는 값과 범위

원작 `HvM.dat`의 세 슬롯 3,624바이트를 그대로 담는다. 미션·스테이지, 경험치·레벨·강화 포인트·부품 단계, 무기 순서·잔탄과 슬롯의 나머지 바이트를 보존한다. 추가 설정은 HUD 10개 그룹 위치, 키 16개, 조준 모드다. 에셋·앱 파일·실행 중인 전투 객체·치트 설정은 담지 않는다.

Apple 앱은 전체 슬롯과 설정을 적용한다. Android·웹·PWA·Tauri는 전체 또는 단일 슬롯을 선택하며 설정 제외도 가능하다. 단일 슬롯을 복원해도 설정은 앱 전체에 적용한다. 전체 복원은 빈 슬롯도 교체한다. 여러 기기의 변경을 자동 합치는 기능은 없으며, 코드를 만든 시점의 저장을 수동으로 전달한다.

### 복원 중단과 재시작

공통 `Restore`는 기존 값 보관, 복원 대기 기록, 적용, 재읽기 대조, 완료 순서로 진행한다. 손상 코드와 범위를 벗어난 값은 적용 전에 거부한다. 기존 값과 대기 기록을 쓰지 못하면 적용을 시작하지 않는다. 대기 기록을 남긴 뒤 실패하면 그 기록을 유지하고 오류를 표시한다. 다음 실행은 대기 중인 목표값을 다시 적용하고 대조한다. 기존 저장으로 자동 롤백하는 방식은 아니며 복원 전 코드를 별도로 남긴다.

| 플랫폼 | 실제 저장과 복원 기록 | 적용 후 처리 |
|---|---|---|
| Apple | `HvM.dat`, 현재 기기 종류의 `HUDLayoutV1-iphone/ipad/mac.json`, UserDefaults의 `KeyboardBindingsV1`·`AimModeV1`. 저장 폴더의 `transfer/before.hmcode`·`restore.hmcode` | 다시 읽어 대조하고 메모리를 갱신한 뒤 슬롯 화면 요청 |
| Android | `files/HvM.dat`, `HUDLayoutV1.txt`, `InputV1.txt`. `files/transfer/before.hmcode`·`restore.hmcode` | 설정·진행을 다시 읽고 슬롯 화면 요청 |
| 웹·PWA | IndexedDB `/persist`의 `HvM.dat`·`HUDLayoutV1-web.json`, localStorage 키·조준 설정. `HeavyMach.TransferBefore`·`TransferPending` | 게임 페이지를 종료한 뒤 시작 화면에서 적용하고, PLAY로 저장을 다시 읽음 |
| Tauri | 게임은 웹과 같은 IDBFS·localStorage. 복원 코드는 Rust가 앱 데이터 폴더 `transfer/`에 기록 | 게임 페이지를 나가 적용·재읽기·OS 백업 완료 후 대기 기록 삭제 |

Apple·Android 파일 기록은 임시 파일에 쓰고 `fsync`와 이름 교체를 사용한다. 여러 저장 파일과 설정을 한 번에 교체하지 않으므로, 중간 실패는 복원 대기 코드로 재개한다. 웹은 IndexedDB 트랜잭션 뒤 localStorage 설정을 적용한다. 두 저장소 사이의 중단도 대기 기록으로 재개한다. Tauri는 OS 백업 쓰기까지 마친 뒤 웹과 OS의 대기 기록을 지운다.

웹·PWA의 복원 요청은 먼저 sessionStorage에 담고 페이지를 다시 연다. 시작 화면이 `HeavyMach-save-owner` 잠금을 얻은 뒤 요청을 처리한다. 두 창이 같은 저장에 동시에 게임을 실행하는 것을 막고, 게임의 미완료 IDBFS 동기화가 복원값을 덮어쓰지 않게 한다. Tauri도 게임을 벗어난 실행기에서 복원한다.

### 오프라인 캐시와 진행 저장

Service Worker의 게임 파일 캐시와 IndexedDB의 진행은 따로 관리한다. 브라우저 플레이 시작 때 `navigator.storage.persist()`를 요청하며, 설치 PWA를 우선 안내한다. 브라우저의 사이트 데이터 삭제는 진행과 같은 영역의 복원 기록도 지울 수 있다. 별도로 보관한 `HM1-` 코드는 게임 에셋을 다시 준비한 뒤 사용할 수 있다. 데이터 보존을 위한 주기적 핑·푸시·서버는 구현하지 않았다.

Tauri는 WebView 저장 외에 앱 데이터 폴더에 OS 백업을 둔다. 저장소가 비어 있으면 백업에서 복구하고, 일부 파일만 빠졌으면 실행 전에 복구 선택을 받는다. 파일 형식·코드 규격과 세부 소비 경로는 [SaveTransfer](../Modules/SaveTransfer/README.md), 실제 기기 검사는 [현재 상태](current-status.md)와 [Galaxy 결과](evidence/game-buttons-android-20261102.json)에 연결한다.
