# SaveTransfer

세이브 슬롯 선택 화면에서 진행을 코드로 만들어 다른 기기에 옮기는 공통 C++ 모듈이다. Apple·Android는 네이티브 라이브러리로, 웹·PWA·Tauri는 같은 구현을 WebAssembly로 빌드한다. 계정이나 서버 없이 사용자가 코드를 전달한다. 코드를 만든 시점의 저장값을 복사하며, 두 기기의 이후 플레이를 자동으로 합치지는 않는다.

## 코드에 담는 데이터

| 데이터 | 저장 방식 |
|---|---|
| 게임 진행 | 원작 `HvM.dat`의 세 슬롯, 총 3,624바이트. 각 슬롯은 1,208바이트이며 빈 슬롯과 미확인 바이트도 보존한다 |
| 슬롯 안의 값 | 미션·스테이지 진행, 경험치·레벨·강화 포인트·부품 단계, 무기 목록의 순서와 잔탄, 슬롯 생성·접속 기록 등 원작 저장 전체 |
| HUD | 10개 그룹의 정규화된 위치. 기본 위치는 미지정 값으로 담는다 |
| 조작 | 키 배치 16개와 조준 모드 |

부품 단계에 맞는 탱크 그림은 받는 앱의 기존 에셋과 게임 코드로 다시 구성한다. 이미지·음향·앱 파일은 코드에 넣지 않는다. 현재 전투의 적·탄환 위치, 탱크의 순간 위치·점프, 실행 시간과 치트 패널 조건도 대상이 아니다. 마지막 정상 저장값을 옮긴 뒤 슬롯에서 다시 시작한다. HUD·키·조준 설정은 슬롯별 설정이 아니라 앱 전체 설정 한 벌이다.

## 형식과 검사

`SaveTransfer.cpp`가 저장값 검증, 코드 생성·해석, 슬롯 선택과 복원 순서를 맡는다. `Sources/Core/SaveData`의 원작 형식을 그대로 읽으며 전투·이동·정상 저장 시점은 바꾸지 않는다.

코드는 `HM1-` 접두사와 패딩 없는 Base64url 문자열이다. 내부에는 형식·압축 버전, 압축 전 길이, CRC32와 PackBits 방식의 압축 데이터가 들어간다. 같은 저장값은 네이티브와 WebAssembly에서 같은 코드가 된다. 해시만 전달하는 방식이 아니며, 복원에 필요한 저장값 자체가 들어 있다. 암호화·암호·salt는 사용하지 않는다.

해석할 때 길이·문자·버전·CRC32를 확인하고 원작 저장 크기, 미션·스테이지 범위, 무기 이름·잔탄, 부품 단계, 중복 키와 HUD 좌표를 검사한다. 입력은 최대 24,000자, 압축 해제 데이터는 최대 8,192바이트다. 공백·탭·줄바꿈은 허용한다. CRC32는 전송 중 손상 검사이며 코드 발급자나 정당한 플레이 여부를 판별하지 않는다.

## 플랫폼별 복원 범위

코드 생성은 항상 세 슬롯과 설정을 담는다. 복원 범위는 받는 앱에서 선택한다.

| 실행 환경 | 게임 슬롯 | HUD·키·조준 설정 | 연결부 |
|---|---|---|---|
| iPhone·iPad·네이티브 Mac | 세 슬롯 전체 교체 | 항상 함께 적용 | `Sources/iOS/HMTransfer.inc` |
| Android | 전체 또는 원본 슬롯 하나를 대상 슬롯 하나로 복사 | 포함 여부 선택 | `Android/app/src/main/cpp/Transfer.inc`, `GameButtons.inc` |
| 웹·PWA | 전체 또는 단일 슬롯 | 포함 여부 선택 | `web/ui.js`, `web/slot-entry.js`, `Distribution/Web/pwa/host.js` |
| Tauri Mac | 전체 또는 단일 슬롯 | 포함 여부 선택 | 공통 웹 UI, `Desktop/Tauri/frontend/transfer-game.js`, `transfer-host.js` |

전체 복원은 코드의 빈 슬롯도 적용한다. 단일 복원은 나머지 두 슬롯을 유지하며 빈 원본 슬롯은 거부한다. `FROM ALL / TO ALL` 또는 양쪽 모두 특정 슬롯을 고른 조합만 허용한다. 설정을 포함하면 단일 슬롯 복원에서도 앱 전체 HUD·조작 설정이 바뀐다. Apple 화면에는 슬롯 선택과 설정 제외 기능이 없다.

## 입력과 복원 순서

진입은 전환이 끝난 슬롯 선택 화면으로 제한한다. 웹은 저장 읽기·동기화가 끝났는지도 확인한다. 생성·복사·검사·확인 취소는 게임 저장을 쓰지 않는다. 버튼은 공통 게임 버튼 코드로 그리며, 텍스트 입력·클립보드·파일 선택만 플랫폼 기능에 연결한다.

복원을 확정하면 다음 순서로 처리한다.

1. 기존 저장을 읽어 복원 전 코드로 보관한다.
2. 적용할 값을 복원 대기 기록으로 남긴다.
3. 게임 저장과 선택한 설정을 쓴다.
4. 다시 읽어 목표값과 대조한다.
5. 대조에 성공하면 복원 대기 기록을 지운다.

Apple·Android의 `FileStore.hpp`는 `transfer/before.hmcode`와 `transfer/restore.hmcode`를 쓴다. 임시 파일 쓰기·`fsync`·이름 교체 후 디렉터리를 동기화한다. 게임·설정 파일 전체를 하나의 원자적 쓰기로 묶지는 않는다. 도중에 종료되면 다음 실행에서 남은 `restore.hmcode`의 목표를 재적용하고 대조한다. 실패한 복원을 임의 초기화하거나 자동으로 이전 값으로 되돌리지 않는다. 기존 값은 `before.hmcode`에 남는다.

웹·PWA는 복원 요청을 `sessionStorage`에 보관하고 게임 페이지를 나간 뒤 처리한다. 시작 화면은 `HeavyMach-save-owner` 잠금을 얻고, `HeavyMach.TransferBefore`와 `HeavyMach.TransferPending`을 `localStorage`에 기록한다. IndexedDB와 설정을 적용하고 검증한 뒤 대기 기록을 지운다. 새 게임을 열기 전 중단된 복원을 먼저 재개한다. 코드 복원 기록도 같은 브라우저 저장소에 있으므로 외부 백업으로 쓸 코드는 따로 보관한다.

Tauri도 게임 페이지를 나간 뒤 같은 WebAssembly 복원 절차를 실행한다. 대기·이전 코드는 Rust를 통해 앱 데이터 폴더의 `transfer/`에 저장한다. 적용 후 OS 백업까지 기록하고 웹·OS 대기 기록을 지운다. 이 파일 백업과 웹의 IndexedDB 저장은 서로 다른 저장 경로다.

## 검사

저장소 루트에서 실행한다.

```sh
cmake -S Modules/SaveTransfer -B build/save-transfer/native -DHM_TRANSFER_TESTS=ON
cmake --build build/save-transfer/native
ctest --test-dir build/save-transfer/native --output-on-failure
build/save-transfer/native/transfer-tests > build/save-transfer/native-result.txt
python3 Modules/SaveTransfer/build_web.py --emxx /path/to/em++ --out build/save-transfer/web
node Modules/SaveTransfer/tests/wasm.mjs "$(pwd)/build/save-transfer"
node Modules/SaveTransfer/tests/slot-entry.mjs "$(pwd)/build/save-transfer"
```

웹 화면 검사는 Web 테스트 의존성, Chrome, `Distribution/Web/build/site` 산출물을 준비한 뒤 `node Modules/SaveTransfer/tests/browser-slots.mjs`로 실행한다. `HM_INSTALLED=1`은 설치형 PWA를 검사한다. 별도 Chrome 프로필을 사용한다. 사용자 복원 코드는 Git에 추가하지 않는다.

초기 교차 플랫폼 검사는 [2026-11-01 기록](../../docs/evidence/save-transfer-slots-20261101.json)에 있다. 버튼 교체 후 [Apple 검사](../../docs/evidence/game-buttons-apple-20261102.json), [Galaxy 네이티브·Chrome·PWA 검사](../../docs/evidence/game-buttons-android-20261102.json), [웹·Tauri 검사](../../docs/evidence/game-buttons-web-tauri-20261102.json), [공개 웹 배포 후 검사](../../docs/evidence/game-buttons-web-postdeployment-20261102.json)를 추가했다. 최신 남은 과제는 [TODO](../../docs/usability-todo.md)를 따른다.
