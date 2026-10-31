# Tauri Mac·웹·Windows 개발 계획

2026-10-31. 개발 순서는 **Tauri Mac 앱 → 순수 웹판 → Tauri Windows 설치본 빌드**다. Mac에서는 기존 네이티브 HeavyMach와 비교한다. Electron은 기술 비교 자료로 남긴다.

현재 웹 게임 코어·렌더러·입력·음향·저장을 재사용해 시작할 수 있다. Tauri 실행기와 OS 파일 저장 연결은 아직 없다. 이번 검토에서는 소스 의존 관계, 도구, 배포 절차를 확인했다. [점검 기록](evidence/desktop-web-readiness-20261031.json).

## 1. 개발 순서

| 단계 | 할 일 | 완료할 결과 |
|---|---|---|
| 1. Tauri Mac | 기존 웹판을 앱에 포함하고 창·파일 저장·종료 처리를 연결한다. 기존 네이티브 Mac 앱과 비교한다 | Apple Silicon Mac에서 설치·플레이·재실행·저장 복구가 되는 앱과 비교 결과 |
| 2. 순수 웹 | 같은 웹 실행부를 브라우저 저장과 연결하고 고정 HTTPS 주소로 배포한다 | 게임 주소, 버전별 웹 묶음, 저장 유지·업데이트·브라우저 검사 결과 |
| 3. Tauri Windows | 같은 Tauri 프로젝트를 Windows용으로 빌드하고 설치본을 만든다 | Windows x64 빌드 결과, 설치 파일, 해시·빌드 로그 |

첫 단계에서도 Mac 앱에 넣을 Wasm·자원을 빌드한다. 두 번째 단계는 그 실행부의 브라우저 배포다. 순수 웹판은 Tauri나 Rust 설치 없이 브라우저에서 실행한다. Windows 설치·플레이·성능 검사는 이번 세 번째 단계 이후의 별도 과제다.

이 순서면 지금 사용 가능한 Mac에서 입력·음향·저장·렌더 차이를 먼저 확인하고, 그 결과를 웹판과 Windows 패키지에 재사용할 수 있다. 기존 iOS·Android·네이티브 Mac 앱은 계속 유지한다.

## 2. 현재 준비 상태

| 항목 | 확인한 상태 | 필요한 작업 |
|---|---|---|
| 게임 코어 | `Web/CMakeLists.txt`가 기존 `Sources/Core`를 직접 빌드 | 코어를 그대로 읽는다 |
| 웹 실행 | `Sources/Web`의 WebGL2·Web Audio·입력·메뉴·HUD 구현 존재 | Tauri에서 로딩·포커스·외부 링크·창 크기 처리 |
| 진행 저장 | `Web/shell.html`의 IDBFS, 초기 읽기 대기·실패 재시도·쓰기 완료 처리 존재 | 데스크톱 파일 저장 어댑터 |
| 설정 저장 | HUD는 `/persist/HUDLayoutV1-web.json`, 키 설정·자동조준 설정은 localStorage | 세 종류 모두 데스크톱 저장과 연결 |
| 복구 검사 | 웹 복구 19개, Wasm 코어 24개와 Apple·Android 검사 기록 존재 | 실제 Tauri 앱에 같은 시나리오 적용 |
| 도구 | Xcode 27.1, Emscripten 6.0.13의 로컬 설치 확인 | Rust·Cargo·Tauri CLI와 의존성 버전 고정 |
| Tauri 프로젝트 | `Desktop/` 추적 파일 없음 | 새 프로젝트·저장 코드·검사·빌드 도구 작성 |
| Mac 공개 배포 | 로컬 키체인에 Apple Development 인증서만 확인 | Developer ID 서명·공증을 사용할 배포 자격 확인 |
| Windows 빌드 | 저장소에 Windows 작업·self-hosted runner 없음 | GitHub Windows runner의 빌드·패키징 작업 추가 |
| 웹 배포 | 기여자의 기존 테스트 사이트 기록 존재 | 이 저장소 수정본의 공식 주소·업데이트 구성 |

기존 검사는 [PR 수정 검사](evidence/pr24-web-fixes-20261031.json)의 당시 실행 결과다. Tauri 앱의 실행 결과는 개발하면서 추가한다. 개발 도구 준비는 [Tauri 준비물](https://v2.tauri.app/start/prerequisites/)을 따른다.

## 3. 폴더와 의존 관계

아래 `Desktop/Tauri/`, `Web/host/`, `Web/deploy/`는 추가할 폴더다. 기존 Apple·Android 프로젝트와 소스는 현재 위치를 유지한다.

```text
HeavyMach/
├── HeavyMach.xcworkspace/       기존 Apple workspace
├── iOS/                        기존 iPhone·iPad Xcode 프로젝트
├── macOS/                      기존 네이티브 Mac Xcode 프로젝트
├── Modules/                    기존 Apple 모듈 프로젝트
├── Android/                    Kotlin·JNI·Gradle·Android 검사
├── Sources/
│   ├── Core/                   모든 플랫폼이 읽는 C++ 게임 규칙
│   ├── iOS/ Apple/ Mac/         기존 Apple 실행 코드
│   └── Web/                    브라우저·Tauri가 공유하는 C++ 웹 실행부
├── Resources/ Art/ vendor/      기존 공용 자원·라이브러리
├── Web/
│   ├── CMakeLists.txt          Wasm 빌드
│   ├── shell.html              브라우저와 Tauri가 사용하는 화면
│   ├── host/                  저장·설정 연결 규약, 브라우저 구현
│   ├── tools/                 웹 자원 패키징
│   ├── tests/                 웹 입력·저장·GPU 검사
│   └── deploy/                순수 웹 호스팅·캐시·버전 설정
└── Desktop/
    └── Tauri/
        ├── README.md          소비 코드·빌드 명령·출력 경로
        ├── package.json       Tauri 도구와 잠금 파일
        ├── frontend/          Tauri 전용 JS 저장·창 연결
        ├── scripts/           웹 빌드·앱 포함용 복사·패키지 검사
        ├── tests/             Mac 비교·Rust 저장·패키지 검사
        └── src-tauri/
            ├── Cargo.toml     Rust 의존성, Cargo.lock 함께 관리
            ├── src/           실행기·파일 저장
            ├── capabilities/  앱 창의 명령 접근 범위
            ├── tauri.conf.json
            ├── tauri.macos.conf.json
            └── tauri.windows.conf.json
```

Mac과 Windows는 하나의 Tauri 프로젝트를 쓴다. OS별 차이는 위 설정 파일과 작은 Rust 분기로 둔다. Tauri 모바일 프로젝트는 생성하지 않는다. 기존 `macOS/`에는 Tauri 파일을 넣지 않는다.

빌드 출력도 분리한다. Wasm은 `build/web/`, Tauri용 웹 묶음과 Rust 출력은 `Desktop/Tauri/build/` 아래에 둔다. 이 폴더는 Git에서 제외한다. 앱에 넣을 자원은 기존 자원을 빌드 때 복사하고 경로·SHA 목록으로 대조한다. `frontend/`에는 Tauri 연결 코드만 두고 게임 코드나 원본 에셋 복사본을 추적하지 않는다.

의존 방향은 `Tauri → 웹 산출물 → Sources/Web + Sources/Core + 공용 자원`이다. Apple·Android 빌드에 Tauri 도구를 요구하지 않는다. 각 폴더 README와 파일 목록에 소유 플랫폼·읽는 경로·생성 경로를 적고 검사 스크립트로 실제 의존 경로를 대조한다.

### 기존 iOS 폴더에 있는 공용 헤더 세 개

웹 빌드는 현재 `Sources/iOS`를 include 경로로 사용한다. 읽는 파일은 `HMButtonState.hpp`, `HMAimOverlay.hpp`, `HMCompatibilityPixels.hpp`다. 버튼 움직임·조준 표시·픽셀 변환을 담은 C++ 헤더이며 UIKit 실행 코드가 아니다. Tauri 추가 전에 이미 존재하는 의존 관계다.

이번 작업에서는 세 파일을 이동하거나 새 사본을 만들지 않는다. 웹 빌드의 읽기 목록에 정확히 명시하고 변경 전후 해시를 검사한다. 이 배치의 정리는 Apple 빌드 경로도 바뀌므로 별도 공통 헤더 정리 과제로 둔다. Tauri용 파일을 iOS 폴더에 추가할 필요는 없다.

## 4. 첫 단계: Tauri Mac 앱

### 로딩·저장 연결

게임은 기존 `Web/` 빌드로 만든 HTML·JS·Wasm·자원 묶음을 `frontendDist`에 포함해 실행한다. 원격 게임 주소를 열지 않으므로 앱 설치 뒤 네트워크 없이 시작할 수 있다. [정적 파일 구성](https://v2.tauri.app/develop/).

`Web/host/`에 초기 읽기·쓰기 완료·설정 읽기/쓰기를 담당하는 작은 연결 규약을 둔다. 브라우저 구현은 현재 IDBFS와 localStorage를 사용한다. Tauri 구현은 `Desktop/Tauri/frontend/`에서 Rust 명령에 연결한다. 브라우저 빌드에는 Tauri 패키지를 넣지 않는다. 기존 웹 게임 동작은 회귀 검사로 확인한다.

- 시작할 때 Rust가 `HvM.dat`와 HUD 배치·설정을 읽고, JS가 `/persist`와 설정 메모리를 준비한 뒤 게임을 시작한다. 읽기 실패와 최초 실행의 파일 없음은 구분한다.
- `HvM.dat`는 기존 3,624바이트 형식을 유지한다. 슬롯 선택·스테이지 전환·게임오버·메뉴 복귀의 저장 요청만 전달한다. HUD는 APPLY, 키 설정은 확정 시점에 저장한다.
- 저장 요청에 순번을 붙여 직렬 처리한다. 임시 파일·백업·교체를 마친 뒤 성공을 반환한다. 실패한 요청은 메모리에 유지하고 재시도한다. Mac과 Windows의 파일 교체 동작은 각각 검사한다.
- 닫기 요청 때 진행 중인 쓰기의 완료를 기다린다. 실패하면 창을 유지하고 재시도 또는 취소를 제공한다. 창을 닫는다는 이유로 전투 중간 진행을 새로 저장하지 않는다.
- 앱 데이터 경로와 앱 식별자는 기존 네이티브 Mac 앱과 분리한다. 비교할 때도 원래 사용자 저장을 직접 열어 덮어쓰지 않는다. 내보내기·가져오기는 형식 검사와 백업을 거친다.
- 키 설정·자동조준 설정은 현재 localStorage의 실패를 조용히 무시하는 코드도 함께 점검한다. 진행 저장만 성공하고 설정이 사라지는 경우를 검사한다.

Rust 명령은 저장에 필요한 파일 이름만 받는다. 임의 경로 쓰기와 원격 페이지의 명령 호출을 열지 않는다. Wasm·로컬 자원·외부 링크 처리에 필요한 CSP와 권한만 설정한다. [Tauri 명령](https://v2.tauri.app/concept/inter-process-communication/), [접근 범위](https://v2.tauri.app/security/capabilities/), [CSP](https://v2.tauri.app/security/csp/).

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

Tauri의 최신 공식 문서는 `@wdio/tauri-service`와 앱 안의 WebDriver 플러그인으로 macOS 검사를 안내한다. 이를 검증 빌드에만 넣고 실제 WKWebView와 Rust 저장 명령을 검사한다. 브라우저에서 명령을 모의한 검사와 앱 실행 검사를 구분한다. 입력·화면·음향은 실제 앱에서도 확인한다. [Tauri WebDriver 안내](https://v2.tauri.app/develop/tests/webdriver/).

### Mac 배포

첫 대상은 Apple Silicon arm64다. 기존 Intel Mac 미지원 방침을 유지한다. 로컬 검증 앱과 일반 배포 앱을 구분하고, 배포 앱에는 검사 서버·치트 패널을 포함하지 않는다. 공개 설치본은 Developer ID 서명·공증·DMG 검사를 거친다. 현재 확인된 Apple Development 인증서는 배포용 Developer ID와 다르다. [Mac 패키지](https://v2.tauri.app/distribute/macos-application-bundle/), [서명·공증](https://v2.tauri.app/distribute/sign/macos/).

## 5. 두 번째 단계: 순수 웹판

Mac에서 검사한 공용 웹 실행부를 브라우저 저장 어댑터로 빌드한다. 정적 호스팅용 결과는 `Web/deploy/`의 설정으로 만든다. 기존 기여자 사이트의 배포 상태와 이 저장소의 공식 배포 상태를 별도로 기록한다.

고정 HTTPS origin을 정하고 버전별 HTML·JS·Wasm·자원 묶음을 함께 배포한다. 이전 HTML과 새 Wasm이 섞이지 않게 버전 경로와 캐시를 구성한다. Wasm MIME, CSP, 하위 경로 로딩, 다운로드 실패·재시도와 이전 버전 복귀를 검사한다. 현재 자원 분할 도구를 그대로 사용한다.

브라우저 진행은 IndexedDB, 설정은 브라우저 저장소에 남긴다. 저장 내보내기·가져오기, 저장소 사용 불가·용량 초과·새로고침·동시 탭·업데이트 후 복구를 검사한다. 같은 origin을 사용해도 여러 탭의 동시 쓰기로 이전 진행이 덮일 수 있으므로 한 탭의 쓰기 소유권과 잠금 해제를 구현한다. 서버 계정 동기화는 추가하지 않는다.

Mac Safari·Chrome과 모바일 브라우저의 터치·틸트·가로 회전·소리·복귀를 검사한다. 사용자가 제외했던 Firefox는 이번 기본 검사 대상에서도 제외한다. 웹 manifest는 이미 있으며 Service Worker 오프라인 캐시는 별도 후속 과제로 둔다. Tauri 앱의 오프라인 실행은 앱 내부 자원으로 처리한다.

## 6. 세 번째 단계: Windows 빌드

동일한 `Desktop/Tauri/`를 Windows x64 대상으로 빌드한다. `.github/workflows/desktop-tauri.yml`에서 Windows runner·MSVC·Rust·Node·Emscripten 버전을 고정하고 WebView2 설치 방식을 지정한다. 앱 코드·자원·Wasm 해시, 서명 상태, `.exe` 설치본 또는 `.msi`와 빌드 로그를 첨부한다.

기본안은 Windows runner에서 NSIS 설치본을 만드는 것이다. Tauri 문서상 MSI는 Windows에서 만들어야 하며, Mac의 Windows 교차 빌드는 제약이 더 많다. 이번에는 Windows CI를 사용한다. 최초 WebView2 설치에 인터넷이 필요한지 패키지 설명에 적고, 완전 오프라인 설치 묶음은 후속 선택지로 남긴다. [Windows 설치본](https://v2.tauri.app/distribute/windows-installer/), [GitHub 빌드 구성](https://v2.tauri.app/distribute/pipelines/github/).

이번 단계의 완료는 **Windows 설치본 생성과 산출물 검사**다. Windows에서 실제 실행·설치·전투·저장·GPU·음향·업데이트를 확인하기 전에는 해당 항목을 미검사로 표시한다. 실행 검증 전 파일은 시험용 빌드 첨부로 제공하고, 안정판 Windows 출시 표시는 그 검사 뒤에 한다. Windows 코드 서명 자격도 패키징 단계에서 확인한다.

## 7. 기존 플랫폼·기여 보존과 릴리스

시작 기준 develop의 Apple·Android·Core·Resources·Art·vendor·기존 빌드 파일 경로와 해시를 저장한다. 허용 변경은 웹 실행부·웹 빌드·새 Desktop 폴더·전용 검사/워크플로·관련 문서다. Apple 폴더에 Tauri·Android 파일이 들어오거나 Android 폴더에 Tauri 파일이 들어오면 경로 검사에서 실패시킨다. 웹이 읽는 기존 공용 헤더 세 개도 보호 목록에 둔다.

`Web/`와 `Sources/Web/` 수정은 기여자 원본 커밋 뒤의 새 커밋으로 남긴다. 원본 19개 SHA·작성자·공동 작성자 표기를 유지한다. fastutlego의 웹판과 후속 Tauri 실행기·저장·배포 작업의 크레딧을 구분한다.

릴리스 때는 `CHANGELOG`에 Mac 비교 결과, 웹 주소, Windows 빌드 상태를 나눠 적는다. develop→main 병합 후 새 태그에 소스·자원 묶음과 플랫폼별 산출물·해시·고지를 연결한다. 기존 릴리스 도구는 소스/고지 묶음을 처리하므로 DMG·Windows 설치본·웹 묶음의 추가와 재개 검사를 구현해야 한다. [릴리스 절차](release-notes-plan.md).

다음 구현은 develop에서 별도 `feat/tauri-mac` 브랜치를 만든 뒤 시작한다. 이번 문서 작업에서는 Tauri 앱을 생성하거나 기존 앱을 변경하지 않았다.
