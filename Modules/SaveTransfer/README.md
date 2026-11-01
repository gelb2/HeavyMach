# SaveTransfer

세이브 슬롯 선택 화면에서 진행을 복원 코드로 옮기는 공통 C++ 모듈이다. Apple·Android는 네이티브로, 웹·PWA·Tauri는 같은 구현을 WebAssembly로 빌드한다.

- 원작의 세 슬롯 3,624바이트와 HUD 위치 10개, 키 배치 16개, 조준 설정을 담는다.
- `HM1-` 코드는 버전·길이·CRC32를 포함한다. CRC32는 손상 검사용이며 암호화가 아니다.
- 복원은 기존 값 보관 → 복원 대기 기록 → 적용 → 다시 읽어 대조 → 완료 순서로 처리한다.
- 게임 코어의 저장 형식·탄도·이동·전투 로직은 변경하지 않는다.

## 연결부

| 실행 환경 | 소비 파일 |
|---|---|
| iOS·iPadOS·네이티브 macOS | `Sources/iOS/HMTransfer.inc`, Apple 프로젝트의 정적 프레임워크 |
| Android | `Android/app/src/main/cpp/Transfer.inc`, Kotlin 입력창과 JNI |
| 순수 웹·PWA | `Distribution/Web/pwa/host.js`, `saves.js` |
| Tauri | `Desktop/Tauri/frontend/transfer-game.js`, `transfer-host.js`, Rust 파일 저장 명령 |

`SaveTransfer.cpp`가 형식·검증·복원 순서를 맡는다. `FileStore.hpp`와 `web/`는 저장소 및 화면 연결을 제공한다. Apple 입력창은 세 슬롯 전체를 복원한다. Android·웹 입력창은 전체 또는 한 슬롯을 선택할 수 있다.

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

[실행한 검사와 남은 항목](../../docs/evidence/save-transfer-slots-20261101.json) · [기기별 결과](../../docs/reviews/mac-web-pwa-comparison-20261101.md#unified-save-work-order)
