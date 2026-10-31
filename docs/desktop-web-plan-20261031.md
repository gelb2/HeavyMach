# Tauri Mac·웹·Windows 개발 계획

2026-10-31. 개발 순서는 **Tauri Mac 앱 → 순수 웹·PWA → Tauri Windows 설치본 빌드**다. Mac에서는 기존 네이티브 HeavyMach와 비교한다. Electron은 기술 비교 자료로 남긴다.

**기여자의 웹 구현과 후속 버그 수정 3건을 현재 상태로 고정한다.** 업그레이드 정지·저장 복구·GPU 복구까지 포함한 `Web/`·`Sources/Web/` 파일은 이번 Tauri·순수 웹 개발에서 수정하지 않는다. Apple·Android·공용 코어·자원·기존 빌드 도구도 그대로 읽는다.

Tauri를 검토한 목적은 **저장 데이터 보존과 Mac·Windows 지원**이다. 웹판의 저장 경로를 유지하면서, 별도 실행기가 OS 파일 백업과 복구를 담당하는 안을 세웠다. 실행기와 백업 기능은 아직 미구현이다. 이번 검토에서는 소스 의존 관계, 도구, 배포 절차를 확인했다. [점검 기록](evidence/desktop-web-readiness-20261031.json).

## 1. 개발 순서

| 단계 | 할 일 | 완료할 결과 |
|---|---|---|
| 1. Tauri Mac | 기존 웹판을 앱에 포함하고 창·별도 파일 백업·시작 전 복구·종료 처리를 추가한다. 기존 네이티브 Mac 앱과 비교한다 | Apple Silicon Mac에서 설치·플레이·재실행·저장 복구가 되는 앱과 비교 결과 |
| 2. 순수 웹·PWA | 같은 웹 산출물을 GitHub Pages에 배포하고 설치·오프라인 실행을 추가한다 | 브라우저 주소, 홈 화면 설치, 오프라인 플레이·저장·재실행, 업데이트 검사 결과 |
| 3. Tauri Windows | 같은 Tauri 프로젝트를 Windows용으로 빌드하고 설치본을 만든다 | Windows x64 빌드 결과, 설치 파일, 해시·빌드 로그 |

첫 단계에서도 Mac 앱에 넣을 Wasm·자원을 빌드한다. 두 번째 단계는 그 실행부의 브라우저 배포다. 순수 웹판은 Tauri나 Rust 설치 없이 브라우저에서 실행한다. Windows 설치·플레이·성능 검사는 이번 세 번째 단계 이후의 별도 과제다.

이 순서면 지금 사용 가능한 Mac에서 입력·음향·저장·렌더 차이를 먼저 확인하고, 그 결과를 웹판과 Windows 패키지에 재사용할 수 있다. 기존 iOS·Android·네이티브 Mac 앱은 계속 유지한다.

## 2. 현재 준비 상태

| 항목 | 확인한 상태 | 필요한 작업 |
|---|---|---|
| 게임 코어 | `Web/CMakeLists.txt`가 기존 `Sources/Core`를 직접 빌드 | 코어를 그대로 읽는다 |
| 웹 실행 | `Sources/Web`의 WebGL2·Web Audio·입력·메뉴·HUD 구현 존재 | Tauri에서 로딩·포커스·외부 링크·창 크기 처리 |
| 진행 저장 | `Web/shell.html`의 IDBFS, 초기 읽기 대기·실패 재시도·쓰기 완료 처리 존재 | 기존 IDBFS 유지, 별도 OS 파일 백업·복구 실행기 |
| 설정 저장 | HUD는 `/persist/HUDLayoutV1-web.json`, 키 설정·자동조준 설정은 localStorage | 진행·HUD·두 설정 키를 한 백업 묶음으로 보존 |
| 복구 검사 | 웹 복구 19개, Wasm 코어 24개와 Apple·Android 검사 기록 존재 | 실제 Tauri 앱에 같은 시나리오 적용 |
| 도구 | Xcode 27.1, Emscripten 6.0.13의 로컬 설치 확인 | Rust·Cargo·Tauri CLI와 의존성 버전 고정 |
| Tauri 프로젝트 | `Desktop/` 추적 파일 없음 | 새 프로젝트·백업 코드·외부 검사·빌드 도구 작성 |
| Mac 공개 배포 | 로컬 키체인에 Apple Development 인증서만 확인 | Developer ID 서명·공증을 사용할 배포 자격 확인 |
| Windows 빌드 | 저장소에 Windows 작업·self-hosted runner 없음 | GitHub Windows runner의 빌드·패키징 작업 추가 |
| 웹 배포 | 기여자의 기존 테스트 사이트 기록 존재. 이 저장소의 GitHub Pages는 비활성 | GitHub Actions 배포와 고정 주소 구성 |
| PWA | 기존 manifest·홈 화면 메타데이터 존재. 설치 아이콘·Service Worker는 없음 | 별도 설치 화면·manifest·아이콘·오프라인 캐시·업데이트 구현 |

기존 검사는 [PR 수정 검사](evidence/pr24-web-fixes-20261031.json)의 당시 실행 결과다. Tauri 앱의 실행 결과는 개발하면서 추가한다. 개발 도구 준비는 [Tauri 준비물](https://v2.tauri.app/start/prerequisites/)을 따른다.

## 3. 폴더와 의존 관계

새 파일은 `Desktop/Tauri/`와 `Distribution/Web/` 아래에 둔다. 기존 `Web/`·`Sources/Web/`에는 파일을 추가하거나 고치지 않는다.

```text
HeavyMach/
├── HeavyMach.xcworkspace/       기존 Apple workspace · 수정 금지
├── iOS/ macOS/ Modules/        기존 Apple 프로젝트 · 수정 금지
├── Android/                    기존 Android 앱 · 수정 금지
├── Sources/                    기존 모든 플랫폼·공용 소스 · 수정 금지
├── Resources/ Art/ vendor/      기존 자원·라이브러리 · 수정 금지
├── Web/                        기여자 웹 빌드·화면·검사 + 후속 수정 · 수정 금지
├── Desktop/
│   └── Tauri/
│       ├── README.md           소비 코드·빌드 명령·입출력 경로
│       ├── package.json        Tauri 도구와 잠금 파일
│       ├── frontend/           별도 시작 화면·백업 관찰·복구 화면
│       ├── scripts/            기존 웹 빌드 호출·산출물 복사·보호 해시 검사
│       ├── tests/              Mac 비교·백업/복구·원본 불변 검사
│       └── src-tauri/
│           ├── Cargo.toml      Rust 의존성, Cargo.lock 포함
│           ├── src/            창·종료·백업 파일·내보내기/가져오기
│           ├── capabilities/   로컬 앱의 백업 명령 접근 범위
│           ├── tauri.conf.json
│           ├── tauri.macos.conf.json
│           └── tauri.windows.conf.json
└── Distribution/
    └── Web/
        ├── README.md           공식 웹 배포·갱신·롤백 절차
        ├── scripts/            기존 웹 빌드 호출·산출물 복사·해시 검사
        ├── hosting/            MIME·캐시·고정 주소·배포 설정
        ├── pwa/                시작 화면·manifest·아이콘·Service Worker
        └── tests/              브라우저·설치·오프라인·저장·업데이트 검사
```

Mac과 Windows는 하나의 Tauri 프로젝트를 쓴다. OS별 차이는 전용 설정 파일과 Rust 분기로 둔다. Tauri 모바일 프로젝트는 생성하지 않는다. 기존 `macOS/`나 `Android/`에는 새 파일을 넣지 않는다.

빌드는 기존 `Web/CMakeLists.txt`를 읽고 Git 제외 출력 폴더에서 실행한다. 웹 묶음은 `build/web/`, Tauri 패키지·Rust 출력은 `Desktop/Tauri/build/`와 그 전용 target 경로, 배포 출력은 `Distribution/Web/build/`에 둔다. `frontend/`에는 실행기 코드만 추적한다. 게임 소스와 원본 자원의 별도 사본은 추적하지 않는다.

하나의 웹 빌드에서 얻은 HTML·JS·Wasm·자원 묶음을 데스크톱과 웹 배포에 그대로 복사한다. 원본 산출물과 포함된 게임 파일의 SHA-256을 대조한다. 실행기용 파일은 별도 경로로 추가한다. 생성된 HTML·JS를 패치하거나 게임 함수·`FS`·`Module.hmSyncSave`를 바꿔 끼우는 방법도 사용하지 않는다. Tauri의 런타임 주입은 별도 관찰·백업 코드로 한정하고 원본 함수를 재정의하지 않는다.

의존 방향은 `새 실행기/배포 도구 → 기존 웹 산출물 → Sources/Web + Sources/Core + 공용 자원`이다. Apple·Android 빌드는 Tauri 도구를 요구하지 않는다. 각 폴더 README에 소비자·입력·출력 경로를 적는다.

### 기존 iOS 폴더에 있는 공용 헤더 세 개

웹 빌드는 현재 `Sources/iOS`를 include 경로로 사용한다. 읽는 파일은 `HMButtonState.hpp`, `HMAimOverlay.hpp`, `HMCompatibilityPixels.hpp`다. 버튼 움직임·조준 표시·픽셀 변환을 담은 C++ 헤더이며 UIKit 실행 코드가 아니다. Tauri 추가 전에 이미 존재하는 의존 관계다.

이번 작업에서는 세 파일을 이동하거나 새 사본을 만들지 않는다. 웹 빌드의 읽기 목록에 정확히 명시하고 변경 전후 해시를 검사한다. 이 배치의 정리는 Apple 빌드 경로도 바뀌므로 별도 공통 헤더 정리 과제로 둔다. Tauri용 파일을 iOS 폴더에 추가할 필요는 없다.

## 4. 첫 단계: Tauri Mac 앱

### 웹 게임 로딩

기존 `Web/` 빌드로 만든 HTML·JS·Wasm·자원을 앱에 포함한다. 원격 게임 주소를 열지 않으므로 설치 뒤 인터넷 없이 실행할 수 있다. 앱의 첫 페이지는 `Desktop/Tauri/frontend/`의 별도 시작 화면이다. 저장 점검·필요한 복구가 끝나면 같은 origin의 원본 게임 페이지로 이동한다. 원본의 `syncfs(true)`와 게임 시작 대기는 그대로 실행된다. [정적 파일 구성](https://v2.tauri.app/develop/).

### 데이터 영속성: 웹 저장 유지 + OS 파일 백업

브라우저의 일반 HTTP 캐시와 IndexedDB·localStorage는 다른 저장소다. 사이트 데이터를 지우면 게임 진행도 지워질 수 있다. `navigator.storage.persist()`도 사용자가 직접 지우는 것을 막지는 않는다. Tauri에서는 일반 Safari·Chrome과 분리된 앱 저장소를 쓰고, 그 밖에 백업 파일을 둔다. [브라우저 저장 정책](https://developer.mozilla.org/en-US/docs/Web/API/Storage_API/Storage_quotas_and_eviction_criteria).

| 계층 | 저장 내용 | 담당 코드 |
|---|---|---|
| 게임이 사용하는 저장 | IDBFS의 `/persist/HvM.dat`, `/persist/HUDLayoutV1-web.json`; localStorage의 `HeavyMach.KeyboardBindingsV1`·`HeavyMach.AimModeV1` | 기존 웹 코드 그대로 |
| 앱 전용 웹뷰 프로필 | 위 IndexedDB·localStorage를 재실행·업데이트 뒤에도 유지 | 새 Tauri 설정 |
| OS 파일 백업 | 진행·HUD·두 설정 키, 형식 버전·순번·각 파일 해시 | 새 JS 관찰 코드와 Rust 파일 처리 |
| 사용자 내보내기 | 다른 위치에 보관할 백업 파일, 가져오기 전 검증·현재 진행 백업 | 새 Tauri 메뉴와 Rust 코드 |

Windows는 WebView2 데이터 디렉터리를 고정한다. Mac은 영속 WKWebView 저장소를 사용하며, macOS 14 이상에서는 고정 `dataStoreIdentifier`를 쓸 수 있다. 하위 OS 지원 여부는 이 API 가용성과 함께 정한다. Mac의 `dataDirectory`는 지원되지 않으므로 Windows 설정을 그대로 복사하지 않는다. 앱 ID·origin·scheme·프로필을 버전마다 바꾸지 않고 기존 네이티브 Mac 앱과 분리한다. 비공개·임시 웹뷰를 사용하지 않는다. [Tauri 저장소 설정](https://v2.tauri.app/reference/config/#datadirectory).

백업은 Tauri가 구한 앱 데이터 경로의 `saves/` 아래에 둔다. Mac은 Application Support, Windows는 앱 데이터 디렉터리를 사용하며 설치 폴더·HTTP 캐시·웹뷰 프로필 안에는 두지 않는다. 네이티브 앱의 기존 저장도 건드리지 않는다. 경로는 OS API로 구하고 절대 경로를 코드에 박지 않는다. [Tauri 파일 접근](https://v2.tauri.app/plugin/file-system/).

### 기존 코드를 고치지 않고 백업을 연결하는 방법

아래는 구현할 설계다. 현재 소스의 `hmStorageState()`와 Emscripten 6.0.13 IDBFS 구조를 확인했다. 실제 WKWebView·WebView2에서의 연결 검사는 첫 구현 단계에서 한다.

1. **시작 전:** 별도 시작 페이지가 같은 origin의 저장소와 Rust 백업을 검사한다. 유효한 웹 저장이 있으면 그것을 우선한다. 웹 저장이 없고 유효한 파일 백업이 있으면 게임을 열기 전에 IndexedDB·설정 키를 복구한다. 손상·권한 오류와 최초 실행의 빈 저장을 구별한다. 손상 시 원본을 보존하고 복구할 백업을 선택하게 한다.
2. **게임 중:** Tauri의 별도 초기화 스크립트는 `hmStorageState()`를 읽고 IndexedDB에 이미 반영된 데이터만 읽기 전용 트랜잭션으로 수집한다. `FS.syncfs`나 게임 저장 함수를 교체·호출하지 않는다. 초기안은 5초 간격이며 저장 중·오류·로딩 중에는 건너뛴다. 같은 검사 안에서 설정 값과 저장 상태가 달라졌으면 다음 번에 다시 읽는다. 포커스 복귀와 정상 종료 요청에도 확인한다.
3. **OS 파일 기록:** 직전 백업과 내용 해시가 같으면 쓰지 않는다. 달라진 데이터만 Rust에 전달하고 한 번에 한 묶음씩 기록한다. 임시 파일 쓰기·flush·검증·파일 교체 후 성공을 돌려준다. 최신본과 직전 정상본을 보관한다. 번호와 해시는 묶음 전체에 붙이고 Windows·Mac의 교체/강제 종료 결과를 각각 검사한다.
4. **정상 종료:** 기존 웹 저장이 끝났는지 확인하고 마지막 파일 백업까지 마친 뒤 창을 닫는다. 웹 저장 오류는 기존 재시도 UI로 처리한다. 백업 실패는 새 앱 UI에서 재시도·종료 취소·이전 백업을 남기고 종료를 선택하게 한다. 닫기를 눌렀다고 전투 중간 진행을 새로 저장하지 않는다.
5. **가져오기:** 게임을 닫고 별도 복구 페이지에서 검증한 백업을 반영한 뒤 게임을 다시 연다. 실행 중인 게임의 메모리나 저장 파일을 덮어쓰지 않는다. 앱 프로필마다 한 게임 인스턴스만 열어 동시 쓰기를 막는다.

초기화 스크립트는 Tauri 실행기에만 들어간다. 해당 로컬 origin과 게임 경로에서만 동작하며 원본 함수·C++·DOM을 바꾸지 않는다. 복구 때의 IndexedDB 쓰기는 게임 로딩 전에만 허용한다. 네이티브 명령은 백업 묶음만 받고 임의 파일 경로나 원격 페이지 호출을 허용하지 않는다. [Tauri 초기화 스크립트](https://docs.rs/tauri/latest/tauri/webview/struct.WebviewWindowBuilder.html#method.initialization_script), [명령 접근 범위](https://v2.tauri.app/security/capabilities/).

현재 IDBFS는 `/persist` 데이터베이스, 버전 21, `FILE_DATA` 저장소를 사용한다. 이를 읽고 복구하는 코드는 `Desktop/Tauri/`에만 둔다. Emscripten 버전과 레코드 형식을 고정하고 알 수 없는 형식에서는 쓰기를 중단한다. 최초 빈 DB 생성과 복구에는 동일한 레코드·timestamp 인덱스를 만들어 원본 IDBFS로 읽히는지 검사한다. 구현 근거는 로컬 6.0.13의 `src/lib/libidbfs.js`와 [IDBFS 설명](https://emscripten.org/docs/api_reference/Filesystem-API.html#filesystem-api-idbfs)이다. 이 내부 형식 의존도 별도 어댑터에 가두어 게임 코드 변경 없이 검사한다.

`HvM.dat`는 기존 3,624바이트와 슬롯 형식을 유지한다. 백업 묶음의 구조·길이·해시·HUD/설정 값 범위를 검사한다. 파일이 없다는 상태와 빈 값도 구별해 최초 실행 설정을 임의 생성하지 않는다. 기존 저장 시점은 슬롯 선택·스테이지 전환·게임오버·메뉴 복귀이고, HUD APPLY와 설정 변경도 그대로다. **발사·피격·프레임마다 파일을 쓰지 않는다.**

정상 종료는 마지막 백업의 완료를 기다린다. 강제 종료·전원 차단 때는 마지막으로 완료된 백업까지 복구한다. 5초는 초기 관찰 간격이며 저장 오류·OS 일시정지까지 포함한 무손실 보장 시간이 아니다. 디스크 기록이 끝나기 전 진행은 남지 않을 수 있다. 웹뷰 프로필 삭제 뒤에도 OS 백업은 남지만, 앱 데이터 전체를 직접 지우면 그 안의 백업도 사라진다. 이 경우를 위해 별도 위치 내보내기를 제공한다. 게임의 HUD APPLY 성공 표시와 앱 파일 백업 완료는 별도 상태로 다룬다.

### 저장 검사와 첫 구현 순서

먼저 새 Tauri 실행기에 **원본 웹 묶음과 백업 실험만** 연결한다. `Web/`·`Sources/Web/` 및 포함한 게임 파일의 해시가 같은지 확인하고 다음 검사를 한다. 이 연결이 원본 수정 없이는 성립하지 않으면 수정이 필요한 정확한 파일·이유를 보고한다. 보호 파일을 몰래 바꾸거나 생성된 JS를 패치하지 않는다.

| 검사 | 확인할 결과 |
|---|---|
| 재실행·업데이트 | 3개 슬롯·HUD·키·조준 설정 유지, 앱 ID·origin 유지 |
| 일반 브라우저 데이터 삭제 | Safari/Chrome 데이터 삭제가 Tauri 저장에 영향 없음 |
| 웹뷰 저장소 삭제 | 앱 종료 후 전용 프로필을 지우고 OS 백업에서 같은 바이트로 복구 |
| 강제 종료 | IDB 저장/백업 쓰기/파일 교체 중 종료 각각에서 마지막 정상본 복구 |
| 디스크 부족·쓰기 권한 오류 | 기존 정상본 유지, 실패 표시·재시도, 성공 허위 표시 없음 |
| 손상·형식 불일치 | 자동 초기화 없이 손상본 보존, 정상 이전 백업 선택 |
| 동시 실행·가져오기 | 한 게임 인스턴스 유지, 게임 종료 후 복구, 유효한 기존 진행 자동 덮어쓰기 없음 |
| 내보내기·새 설치 | 외부 백업을 새 앱 프로필에 가져와 동일 슬롯·설정 복구 |
| 성능·기록 횟수 | 30분 전투에서 해시가 같으면 OS 쓰기 0회, 프레임·발사와 백업 기록 분리 |

Mac에서 이 검사를 통과한 뒤 전투·성능 비교로 간다. Windows 설치본 빌드에는 같은 백업 구현과 단위 검사를 포함하되 실제 WebView2 저장 검사는 Windows 실행 단계에서 확인한다.

### 기존 네이티브 Mac과 비교

같은 Mac, 같은 소스·자원 버전, 같은 화면 픽셀 크기, 같은 미션·무기·저장 복사본을 사용한다. 앱은 하나씩 실행한다. 비교용 입력 순서와 난수 조건을 기록하고 OS 실행부 차이를 측정한다.

| 검사 | 비교할 내용 |
|---|---|
| 조작·전투 | 좌우 이동·점프·수동/자동 조준·사격·전체 무기·필살기·무기/표적 전환 |
| 진행 | 업그레이드 선택/확정, 보스 클리어·다음 미션, 게임오버·재시작 |
| 화면 | 스프라이트·블렌드·도어·HUD 위치/입력, 창 크기·배율·전체 화면 |
| 저장 | 정상 저장 횟수, 재실행, HUD APPLY/취소, 키 설정, 쓰기 실패·손상 파일·종료 경쟁 |
| 생명주기 | 포커스 상실·최소화·잠금·절전·복귀, 눌린 키 해제, 음향 중단/재개 |
| 성능 | 콜드 시작 10회, 전체 관련 프로세스의 메모리, 실제 화면 갱신 간격 p50/p95/p99·긴 프레임, 게임 틱, 30분 전투 |
| GPU | WebGL 손실·복구 뒤 같은 세션과 정지 메뉴 유지, 텍스처·버퍼 재생성 |

틱 수와 화면 표시 간격을 따로 기록한다. Instruments로 CPU·메모리·GPU 동작을 계측하고 WKWebView의 관련 프로세스도 포함한다. 네이티브 앱과 Tauri의 렌더러 차이 때문에 생기는 픽셀 차이는 장면 캡처로 비교한다.

현재 TODO의 ‘키보드 첫 사격 조준점’과 Apple의 ‘미사용 경험치가 남은 미션 진입’도 같은 조건으로 재현한다. 기존 앱에 이미 있던 문제를 Tauri의 정상 기준으로 삼지 않는다. 이 과정에서 Apple 수정이 필요하면 별도 버그 작업으로 분리한다.

Tauri의 최신 공식 문서는 `@wdio/tauri-service`와 앱 안의 WebDriver 플러그인으로 macOS 검사를 안내한다. 이를 검증 빌드에만 넣고 실제 WKWebView와 별도 Rust 백업 명령을 검사한다. 브라우저에서 명령을 모의한 검사와 앱 실행 검사를 구분한다. 입력·화면·음향은 실제 앱에서도 확인한다. [Tauri WebDriver 안내](https://v2.tauri.app/develop/tests/webdriver/).

### Mac 배포

첫 대상은 Apple Silicon arm64다. 기존 Intel Mac 미지원 방침을 유지한다. 로컬 검증 앱과 일반 배포 앱을 구분하고, 배포 앱에는 검사 서버·치트 패널을 포함하지 않는다. 공개 설치본은 Developer ID 서명·공증·DMG 검사를 거친다. 현재 확인된 Apple Development 인증서는 배포용 Developer ID와 다르다. [Mac 패키지](https://v2.tauri.app/distribute/macos-application-bundle/), [서명·공증](https://v2.tauri.app/distribute/sign/macos/).

<a id="web-pwa"></a>

## 5. 두 번째 단계: 순수 웹·PWA

**브라우저 실행과 PWA 설치·오프라인 플레이를 같은 단계에서 만든다.** GitHub Pages의 `https://gelb2.github.io/HeavyMach/`를 배포 주소로 계획한다. 현재 이 저장소의 Pages는 비활성이다. 새 GitHub Actions 작업에서 기존 웹 빌드를 호출하고 `Distribution/Web/`의 파일과 합쳐 배포한다. Tauri 설치본은 GitHub Releases에 둔다. [Pages 사용자 지정 배포](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages).

### 기존 게임과 PWA 파일 분리

`Distribution/Web/pwa/`에 별도 시작 화면·manifest·192/512px 설치 아이콘·Service Worker를 둔다. 배포 후 시작 주소와 manifest의 `id`·`start_url`·`scope`는 `/HeavyMach/`로 고정한다. 브라우저별 설치 방법을 안내하고, 설치하지 않은 브라우저에서도 같은 게임을 연다. iPhone·iPad에서는 홈 화면 추가 절차를 안내한다. [PWA 설치 조건](https://developer.mozilla.org/en-US/docs/Web/Progressive_web_apps/Guides/Making_PWAs_installable).

초기안은 시작 화면 안의 같은 출처 iframe에 수정하지 않은 게임 페이지를 전체 크기로 표시하는 방식이다. 게임 묶음은 `game/<version>/` 아래에 복사하고 원본 상대 경로를 유지한다. iframe의 키보드 포커스·터치·틸트 권한·음향 시작·가로 회전·전체 화면을 실제 브라우저와 설치된 PWA에서 검사한다. 이 연결에 필요한 코드는 새 시작 화면에만 둔다. 생성된 게임 HTML·JS나 게임 함수를 패치하지 않는다.

기존 `Web/manifest.webmanifest`에는 이름·시작 주소·가로 방향 등이 있지만 아이콘과 Service Worker가 없다. 기존 파일은 그대로 두고 외부 시작 화면이 새 manifest를 사용한다. Tauri 앱은 앱에 포함한 자원으로 실행하며 PWA Service Worker를 등록하지 않는다.

### 오프라인 다운로드와 업데이트

Service Worker는 배포 경로의 `/HeavyMach/sw.js`에서 등록하고 `/HeavyMach/` 범위만 담당한다. 배포 도구가 HTML·JS·Wasm·모든 자원과 시작 화면·아이콘의 목록·크기·해시를 만든다. 첫 온라인 접속에서 한 버전 전체를 내려받고 검사한 뒤 **오프라인 준비 완료**를 표시한다. 설치 아이콘을 추가했어도 다운로드가 끝나지 않았으면 준비 완료로 표시하지 않는다. [Service Worker 범위](https://developer.mozilla.org/en-US/docs/Web/API/ServiceWorkerContainer/register), [오프라인 동작](https://developer.mozilla.org/en-US/docs/Web/Progressive_web_apps/Guides/Offline_and_background_operation).

캐시는 버전별로 나눈다. 새 버전 전체 다운로드가 끝나야 전환 후보로 올리고, 실행 중인 전투를 새로고침하지 않는다. 새 버전은 다음 시작 때 적용한다. 이전 HTML과 새 Wasm·자원이 섞이지 않게 한다. 이전 버전을 쓰는 창이 모두 닫힐 때까지 해당 캐시를 유지하고, 실패한 다운로드는 기존 정상 버전에 영향을 주지 않게 처리한다. Pages에서 제공하는 MIME·하위 경로·캐시 응답을 검사하며 버전별 경로와 Service Worker로 갱신을 관리한다.

### 오프라인 저장

게임 진행은 기존 IDBFS/IndexedDB, 설정은 기존 localStorage에 기록한다. 인터넷이 없어도 이 저장을 사용한다. 게임 파일 캐시를 갱신하거나 지울 때 진행 저장소는 건드리지 않는다. 별도 시작 화면에서 영속 저장 요청과 백업 내보내기·가져오기를 연결하고, 가져오기는 게임을 닫은 상태에서만 수행한다. 저장 형식과 게임 저장 함수는 유지한다.

같은 출처의 저장소는 경로만 바꾼다고 분리되지 않는다. 기존 DB 이름 `/persist`와 설정 키를 유지하므로 `gelb2.github.io`의 다른 게임 배포와 충돌하는지 확인한다. 시작 화면에서 동시 게임 실행을 막고 버전별 게임 경로도 같은 진행을 읽게 한다. 브라우저와 설치된 PWA가 저장을 공유하는지는 OS별로 검사한다. 사이트 데이터를 직접 지우거나 OS가 저장소를 정리하면 진행이 사라질 수 있으므로 별도로 내보낸 파일로 복구하는 경로를 제공한다. [저장소 정책](https://developer.mozilla.org/en-US/docs/Web/API/Storage_API/Storage_quotas_and_eviction_criteria), [WebKit 저장 정책](https://webkit.org/blog/14403/updates-to-storage-policy/).

### 완료 검사

| 검사 | 확인할 결과 |
|---|---|
| 첫 접속·설치 | 전체 자원 다운로드와 해시 검사 후 준비 완료 표시, 올바른 아이콘·시작 주소로 설치 |
| 인터넷 차단 | 준비 완료 → 비행기 모드 → PWA 시작 → 전투·미션 클리어·저장 → 완전 종료 → 재실행에서 진행 복원 |
| 입력·화면 | 브라우저와 PWA에서 이동·점프·전체 무기·수동/자동 조준·HUD 편집, 틸트·회전·음향·복귀 확인 |
| 다운로드 실패·용량 부족 | 준비 완료를 표시하지 않고 재시도, 기존 정상 캐시·진행 유지 |
| 업데이트 | 게임 중 강제 갱신 없음, 다음 시작에 한 버전 전체 적용, 진행·HUD·키·조준 설정 유지 |
| 저장·복구 | 오프라인 저장, 동시 실행 방지, 내보내기/가져오기, 사이트 데이터 삭제 후 외부 백업 복구 |
| 브라우저·PWA 관계 | 각 OS의 설치 전후 진행 유지 여부, 별도 저장이면 백업 이동 절차 확인 |
| 게임 보존 | 원본 웹 산출물과 포함한 게임 파일의 SHA-256 일치, 보호 소스 차이 없음 |

Mac Safari·Chrome, iPhone·iPad Safari 및 홈 화면 앱, Android Chrome 및 설치된 PWA를 검사한다. 사용자가 제외한 Firefox는 기본 검사 대상에서 제외한다. 현재는 개발 계획이며 PWA 코드 작성·오프라인 실행 검사는 아직 하지 않았다.

## 6. 세 번째 단계: Windows 빌드

동일한 `Desktop/Tauri/`를 Windows x64 대상으로 빌드한다. `.github/workflows/desktop-tauri.yml`에서 Windows runner·MSVC·Rust·Node·Emscripten 버전을 고정하고 WebView2 설치 방식을 지정한다. 앱 코드·자원·Wasm 해시, 서명 상태, `.exe` 설치본 또는 `.msi`와 빌드 로그를 첨부한다.

기본안은 Windows runner에서 NSIS 설치본을 만드는 것이다. Tauri 문서상 MSI는 Windows에서 만들어야 하며, Mac의 Windows 교차 빌드는 제약이 더 많다. 이번에는 Windows CI를 사용한다. 최초 WebView2 설치에 인터넷이 필요한지 패키지 설명에 적고, 완전 오프라인 설치 묶음은 후속 선택지로 남긴다. [Windows 설치본](https://v2.tauri.app/distribute/windows-installer/), [GitHub 빌드 구성](https://v2.tauri.app/distribute/pipelines/github/).

이번 단계의 완료는 **Windows 설치본 생성과 산출물 검사**다. Windows에서 실제 실행·설치·전투·저장·GPU·음향·업데이트를 확인하기 전에는 해당 항목을 미검사로 표시한다. 실행 검증 전 파일은 시험용 빌드 첨부로 제공하고, 안정판 Windows 출시 표시는 그 검사 뒤에 한다. Windows 코드 서명 자격도 패키징 단계에서 확인한다.

## 7. 기존 플랫폼·기여 보존과 릴리스

보호 기준은 계획 수립 당시 main `cf3dca50099d2b7e840de5e3f79c6c1d017369de`의 게임·빌드 파일이다. 기여자의 원본 19개 커밋과 후속 수정 2개 커밋에 담긴 업그레이드 정지·저장 복구·GPU 복구 3건을 함께 보존한다. `Web/`·`Sources/Web/`뿐 아니라 Apple·Android·Core·Resources·Art·vendor·기존 빌드/검사 도구를 보호한다.

구현 착수 때 최신 develop을 fetch하고 기준 이후 변경을 먼저 확인한다. 사용자·기여자의 새 변경을 덮지 않고, 실제 사용할 기준 SHA와 보호 파일 SHA-256 목록을 기록한다. 수정 허용 경로는 새 `Desktop/Tauri/`·`Distribution/Web/`, 새 전용 CI 파일, 필요한 ignore 항목과 관련 문서다. `.github/workflows/desktop-tauri.yml`과 `.github/workflows/web-pages.yml`은 새로 추가하며 기존 Apple·Android·웹 workflow를 수정하지 않는다. 파일 추가·삭제·이동도 보호 검사에 포함한다.

패키지에 넣은 게임 HTML·JS·Wasm·자원은 원본 웹 빌드 산출물과 바이트로 대조한다. 원본 함수 교체·저장 요청 가로채기·생성물 패치는 금지한다. 소스 보존뿐 아니라 기존 웹 검사와 저장 전후 관측으로 실행 동작도 비교한다. 연결이 막히면 새 실행기 쪽에서 원인을 좁히고, 보호 파일 수정이 필요할 때는 파일과 이유를 먼저 보고한다.

기여자 원본 19개 SHA·작성자·공동 작성자·커밋 메시지를 유지한다. 원본을 리베이스하거나 squash하지 않는다. fastutlego의 웹 구현과 이후 Tauri 실행기·백업·배포 작업을 구분해 기록한다.

릴리스 때 `CHANGELOG`에 Mac 비교·파일 복구 결과, 웹 주소·PWA 설치·오프라인 검사 결과, Windows 빌드 상태를 나눠 적는다. develop→main 병합 후 새 태그에 소스·자원 묶음과 플랫폼별 산출물·해시·고지를 연결한다. 기존 릴리스 도구를 고치는 대신 새 플랫폼의 패키징·첨부 도구를 전용 폴더에 추가한다. [릴리스 절차](release-notes-plan.md).

다음 구현은 develop에서 별도 `feat/tauri-mac` 브랜치를 만든 뒤 시작한다. 이번 문서 작업에서는 Tauri 앱·보호 검사 도구·백업 기능을 구현하지 않았다.
