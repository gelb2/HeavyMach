# HeavyMach Tauri 실행기

웹 게임을 포함하는 Apple Silicon Mac 앱이다. 게임 C++·웹 호스트·자원을 빌드해 포함한다. 이 폴더는 창, 앱 저장소, OS 파일 백업과 복구만 담당한다.

## 파일 배치

| 경로 | 역할 |
|---|---|
| `frontend/` | 시작 화면, IDBFS 읽기·시작 전 복구, 저장 완료 관찰 |
| `src-tauri/src/` | 창·종료, 백업 검증·원자 기록·정상본 두 개 보관 |
| `scripts/` | 기존 웹 빌드 호출, 산출물 복사·해시 대조, 보호 파일 검사 |
| `tests/` | 별도 IndexedDB의 복구·중단·저장 대기 검사 |
| `protected-files.json` | 개발 시작 전 기존 파일의 경로·SHA-256 |
| `build/`, `src-tauri/target/` | Git에서 제외한 생성물 |

최초 실행기는 기존 게임 파일을 수정하지 않고 추가했다. 후속 공통 복원 코드 작업은 `Modules/SaveTransfer`와 선언된 플랫폼 연결부·빌드 구성 변경을 포함한다. 11월 2일 승인된 게임 버튼 교체는 Sources/Web·Web의 UI 변경도 포함한다. 파일별 범위는 game-button-scope.json에 기록했다. 기존 게임 코어와 원작 에셋은 유지한다. 웹·PWA 실행기는 `Distribution/Web/`에 있다. Tauri 앱은 Service Worker를 사용하지 않는다.

## 준비와 빌드

macOS 14 이상, Apple Silicon, Xcode Command Line Tools, Node.js, Rust 1.99.1, Emscripten 6.0.13, CMake가 필요하다. Rust 버전은 `rust-toolchain.toml`, npm·Rust 의존성은 잠금 파일로 고정한다. 저장소 루트에서 실행한다.

```sh
npm ci --prefix Desktop/Tauri
python3 Desktop/Tauri/scripts/prepare.py --emsdk /path/to/emsdk
cd Desktop/Tauri
npm run build
```

`prepare.py`는 기존 `Web/CMakeLists.txt`로 게임을 빌드하고 `build/frontend/game/`에 같은 바이트로 복사한다. 별도 시작 화면은 그 상위 폴더에 둔다. `npm run build`도 포함 파일 복사와 보호 검사를 다시 실행해 실행기 수정이 번들에 반영되게 한다. 기존 앱 아이콘의 형식 변환도 이 폴더의 Git 제외 생성물에만 기록한다.

Mac 앱은 `src-tauri/target/release/bundle/macos/Heavy Mach Desktop.app`에 만들어진다. 공개 배포용 서명·공증과 DMG는 후속 작업이다. 현재의 `0.17.0`은 기반 게임 버전이며 새 릴리스 태그를 발행한 것은 아니다.

## 저장

식별자는 `com.gelb2.heavymach.desktop`이며 기존 네이티브 Mac 저장과 분리한다. Mac 웹뷰 저장소 식별자는 버전과 무관하게 고정한다. 게임은 원래의 IndexedDB `/persist`와 두 localStorage 설정 키를 사용한다.

OS 파일 백업은 앱 데이터 경로의 `saves/`에 있다. Mac에서는 `~/Library/Application Support/com.gelb2.heavymach.desktop/saves/`다. 게임이 저장을 끝낸 뒤 5초 간격으로 확인하고 내용이 바뀌었을 때만 새 파일을 쓴다. 발사·피격·프레임에서 저장을 호출하지 않는다. 정상 종료 때 마지막 백업을 기다린다.

웹 저장이 비어 있으면 시작 화면에서 마지막 정상 파일을 복구한다. 기존 진행은 덮어쓰지 않는다. 복구 중 중단은 별도 복구 기록으로 재개한다. 손상 백업은 보존하고 시작을 중단한다. 시작 화면의 Saved progress에서 외부 백업을 가져오거나 마지막 정상 백업을 선택한다. 복구 전 진행과 백업은 recovery/에 보관한다. 강제 종료는 마지막으로 기록이 끝난 백업까지 복구한다.

## 검사

```sh
python3 Desktop/Tauri/scripts/protected.py check
npm test --prefix Desktop/Tauri
cargo test --locked --manifest-path Desktop/Tauri/src-tauri/Cargo.toml
```

JS 검사는 별도 메모리 IndexedDB, Rust 검사는 임시 디렉터리를 쓴다. 실제 사용자의 진행 데이터에 쓰지 않는다. 앱 안의 WKWebView 저장 복구·플레이·기존 네이티브 Mac과의 성능 비교는 별도 실행 검사다.

게임 실행 중 **Diagnostics → Record 15 Seconds of Frame Timing** (`⌘⇧P`)를 선택하면 화면 갱신 간격과 게임 시각을 15초 동안 읽는다. 결과는 앱 데이터 경로의 `diagnostics/`에 한 번 저장한다. 진단 메뉴를 선택하기 전에는 측정하지 않는다. [Mac·웹·PWA 비교](../../docs/reviews/mac-web-pwa-comparison-20261101.md).

[전체 계획](../../docs/desktop-web-plan-20261031.md) · [현재 상태](../../docs/current-status.md)

## Save files

Open **Saved progress** in the launcher to export a JSON backup, import one through the system file picker, or restore the latest valid backup. Import checks the file structure and checksum before asking you to restore. The old progress and OS backup files stay in the app data directory under `recovery/<operation>/`. An interrupted restore resumes before Play becomes available.

The launcher uses the menu artwork prepared by `Distribution/Web/launcher/`. Game files remain unchanged.

OS 오류 회귀는 `src-tauri/tests/storage_faults.rs`에서 실제 저장 모듈을 사용한다. 기본 실행은 임시 폴더의 권한 거부와 별도 저장 프로세스 강제 종료를 검사한다. 디스크 부족 검사는 별도로 마운트한 시험 디스크에 `heavymach-test-volume` 표식을 만들고, `HM_FULL_DISK`에 그 경로를 지정해 `cargo test --test storage_faults -- --ignored --exact disk_full_retains_save`로 실행한다. 앱 저장 경로를 지정하지 않는다.
