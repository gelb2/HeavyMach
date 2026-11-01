# HeavyMach 코드 구조

<!-- language-nav:start -->
[한국어](architecture.md) | [English](../en/public/architecture.md)
<!-- language-nav:end -->

게임 규칙은 C++20 코어에, Apple 플랫폼 연결은 Objective-C++에 구현했다. 하나의 workspace가 공용 모듈과 iOS/iPadOS·Mac 프로젝트의 의존 관계를 관리한다. 앱은 같은 코어와 자원을 사용한다. Android는 별도 Gradle 프로젝트인 `Android/`에서 같은 C++ 코어를 NDK로 빌드한다. Kotlin·JNI·Android C++ 호스트가 화면·입력·음향·저장을 연결한다. [공유 코드와 Android 전용 구현](android-development-20261030.md).

웹판은 같은 C++ 코어를 Emscripten으로 WebAssembly로 빌드한다. 브라우저와 PWA는 `Distribution/Web/`, Tauri Mac 앱은 `Desktop/Tauri/`가 실행·저장을 연결한다. Tauri는 웹판을 앱 안에 포함하며 기존 AppKit Mac 앱과 별도로 실행한다. [웹/PWA](../../Distribution/Web/README.md) · [Tauri Mac](../../Desktop/Tauri/README.md)

## Workspace와 프로젝트

```text
HeavyMach.xcworkspace
├── Modules/HeavyMachModules.xcodeproj
│   ├── HeavyMachCore / HeavyMachValidationCore
│   ├── HeavyMachAppleRuntime / HeavyMachValidationRuntime
│   └── HeavyMachAssets.bundle
├── iOS/HeavyMachRestored.xcodeproj
│   ├── HeavyMachRestored       (iPhone + iPad)
│   └── HeavyMachValidation     (iPhone + iPad 치트)
└── macOS/HeavyMachMac.xcodeproj
    ├── HeavyMachMac
    └── HeavyMachMacValidation
```

위 이름은 모듈 역할을 나타낸다. 실제 target에는 `_iOS`·`_macOS` SDK 구분이 붙는다. 일반·치트 코어는 `HM_VALIDATION` 값을 앱과 일치시켜 따로 빌드한다. 치트 코드와 저장은 일반 앱에서 분리된다.

[공용 프로젝트 설정](../../Modules/project.yml) · [iOS 설정](../../iOS/project.yml) · [Mac 설정](../../macOS/project.yml)

## 모듈화를 지시한 이유

나는 iPhone·iPad·Mac용 게임을 각각 복사해 만드는 대신 공용 코드와 에셋을 모듈로 두고 앱에서 가져오도록 요구했다. 이동·탄도·데미지·적 배치를 플랫폼마다 복사하면 한쪽의 수정이 다른 쪽에 빠질 수 있다. 같은 코어를 연결하면 무기 수치와 계산 변경이 세 플랫폼의 빌드에 함께 들어간다.

Workspace는 여러 프로젝트와 빌드 의존 관계를 묶는다. 코드 공유는 정적 framework를 링크하는 방식으로, 자원 공유는 bundle을 앱에 포함하는 방식으로 구현했다. SPM 없이도 이 구성이 가능하다. Apple의 [복수 프로젝트 관리](https://developer.apple.com/documentation/xcode/managing-multiple-projects-and-their-dependencies)도 프로젝트 간 의존 관계와 공용 산출물을 다룬다.

iPhone·iPad는 같은 iOS target의 기기군 1·2를 사용하며, Mac은 AppKit 앱으로 실행한다. Catalyst는 사용하지 않는다. 공용 코드는 플랫폼별 SDK로 각각 컴파일한다.

## 소스와 자원

| 위치 | 책임 |
|---|---|
| [Android](../../Android) | Kotlin Activity·JNI, Android C++ 호스트·메뉴·HUD·EGL·OpenAL, Gradle·NDK 빌드·기기 검사 |
| [Sources/Core](../../Sources/Core) | 이동·월드·적·탄환·피해·성장·저장 형식·자동조준·추가 적 |
| [Sources/iOS](../../Sources/iOS) | 게임 뷰·HUD·메뉴·OpenGL·OpenAL. Apple 공용 런타임에 포함되는 코드도 이 경로에 있음 |
| [Sources/Apple](../../Sources/Apple) | UIKit/AppKit 및 그래픽 컨텍스트 차이, 공용 입력 연결 |
| [Sources/Mac](../../Sources/Mac) | AppKit 창·앱 수명주기·Mac 진입점 |
| [Sources/Web](../../Sources/Web) · [Web](../../Web) | 웹 게임 호스트·메뉴·HUD·입력·WebGL2, Emscripten 빌드 |
| [Distribution/Web](../../Distribution/Web) | 브라우저/PWA 시작 화면·설치·오프라인 자원·업데이트 |
| [Desktop/Tauri](../../Desktop/Tauri) | 데스크톱 창·종료·OS 파일 백업과 복구, 웹 게임 패키징 |
| [Modules/SaveTransfer](../../Modules/SaveTransfer) | 플랫폼 공용 복원 코드 생성·검증·복원과 중단 복구 |
| [Resources](../../Resources) | 게임 이미지·음원·맵·무기·부품 데이터·검사 자료 |
| [Art/Extensions](../../Art/Extensions) | 메뉴 확장 배경·추가 항공기 등 새 그림 |
| [Tests](../../Tests) | C++ 코어 검사, Apple 입력·화면·음향·저장 검사 |

`HeavyMachAssets.bundle`은 OriginalImages·Images·Audio·RawData와 확장 아트를 묶는다. 코드와 자원의 파일 목록·해시는 [빌드 자원 목록](../build-resources.json)에 있다. Git LFS가 큰 자원을 내려받는다.

## 한 프레임의 처리

1. 터치·기울기·키보드를 게임 입력 상태로 바꾼다. 화면 좌표는 viewport와 카메라를 통해 월드 좌표로 변환한다.
2. `GameSession`이 플레이어·적·무기·탄환·아이템·월드를 갱신한다. 자동조준은 선택한 적의 위치와 무기 값을 이용해 포신을 조정한다.
3. 게임 이벤트를 HUD에 전달하고 OpenGL에서 텍스처·부품·탄환·효과·메뉴를 그린다.
4. 효과음 이벤트는 미리 준비한 OpenAL 버퍼·소스를 사용한다.
5. 저장 이벤트가 발생하면 진행 상태를 파일에 쓴다. 탄환 갱신·키 입력마다 파일을 쓰는 경로는 없다.

Apple 네이티브는 표시 호출에 맞춰 한 번 갱신한다. 지연 뒤 여러 tick을 한꺼번에 따라잡던 누적 시간 계산은 제거했다. 웹판은 브라우저가 30Hz로 표시하는 경우 한 호출에 두 tick을 실행하는 별도 처리가 있다. 일시정지와 백그라운드에서는 게임 시간·발사 유지 입력·효과음을 멈춘다.

## 그래픽과 좌표

게임 그림은 OpenGL로 그린다. iOS는 OpenGL ES 1.1 경로, Mac은 desktop OpenGL 호환 경로를 사용한다. Android는 EGL로 OpenGL ES 1 컨텍스트를 만들고 별도 렌더러에서 게임 그림을 그린다. 웹판은 GLES 호환 처리를 WebGL2로 그린다. 텍스처는 로드 때 준비하고 매 프레임 CPU에서 이미지를 재생성하지 않는다.

캔버스는 기기 비율에 따라 더 넓은 월드를 표시한다. 월드 좌표, 카메라 좌표, UI 좌표, 실제 픽셀을 구분한다. 버튼은 그림과 입력 영역을 같은 배치 계산으로 옮긴다. 메뉴의 중앙 에셋은 유지하고 좌우 배경을 덧붙였으며, 도어는 전체 캔버스를 덮는다.

[좌표 정책](../coordinate-policy.md) · [렌더링 정책](../rendering-policy.md) · [시간·저장·음향](../runtime-policy.md) · [데미지·성장 계산](../game-calculation-policy.md)

## 게임 버튼과 플랫폼 입력

게임이 실행된 뒤의 추가 버튼은 원작 버튼 그림, `HMButtonState`의 눌림 상태, 게임 렌더러와 터치 판정을 사용한다. HUD 편집 도구·키 설정·복원 코드의 생성·검사·확인·취소도 이 경로에 들어간다. 기존 메뉴와 SET GAME UI 버튼은 유지했다. 코드 문자열 입력, 클립보드와 파일 선택은 각 OS와 브라우저의 기능으로 연결한다.

## 저장과 기기 간 복원

[SaveTransfer](../../Modules/SaveTransfer/README.md)는 원래 세 슬롯의 3,624바이트 저장 구조, HUD 배치와 키·조준 설정을 묶어 `HM1-` 코드로 만든다. 버전·길이·CRC32를 검사하고, 기존 저장 백업 → 복원 기록 → 적용 → 다시 읽어 비교하는 순서로 복원한다. 서버나 계정은 사용하지 않는다. Apple 앱은 전체 슬롯을 복원하며 Android·웹판은 전체 또는 선택한 슬롯을 복원할 수 있다.

브라우저/PWA는 진행과 HUD를 IndexedDB에, 키·조준 설정을 localStorage에 저장한다. Tauri는 이 저장에 OS 파일 백업을 추가한다. 웹 저장이 비면 마지막 정상 파일을 복구한다. 네이티브 앱·브라우저·Tauri의 저장소는 분리되어 있으며 복원 코드로 진행을 옮긴다. 게임 코어의 저장 형식·이동·탄도·피해 계산은 이 작업에서 변경하지 않았다.

## Android 구현

`feat/android-app`에서 같은 C++ 코어와 자원을 NDK로 빌드했다. `Android/`의 Kotlin·JNI·C++ 호스트가 화면·입력·EGL/GLES·OpenAL·저장을 연결한다. 2026년 10월의 Android 최초 구현에서는 기존 Apple 소스와 프로젝트를 변경하지 않았다. 이후 복원 코드와 게임 버튼 작업은 Apple·Android의 연결부를 함께 수정했다. Galaxy에 일반·치트 APK를 설치했다. [공유 범위·전용 파일·개발 기록](android-development-20261030.md), [빌드와 설치](../../Android/README.md).

[복원 코드 처리 과정과 플랫폼별 화면](save-recovery-20261102.md)
