# HeavyMach 빌드와 설치

<!-- language-nav:start -->
[한국어](build-and-install.md) | [English](../en/public/build-and-install.md)
<!-- language-nav:end -->

## 웹·PWA·Tauri에서 플레이

[HeavyMach 웹 페이지](https://gelb2.github.io/HeavyMach/)에서 **INSTALL GAME**을 선택하면 PWA 설치 안내를 볼 수 있다. iPhone·iPad Safari에서는 공유 메뉴의 홈 화면에 추가를 사용한다. Android에서는 Chrome의 앱 설치를 사용한다. 브라우저에서 바로 하려면 **PLAY IN BROWSER**를 선택한다. 첫 다운로드가 끝나면 설치한 PWA에서 오프라인으로 플레이할 수 있다.

진행을 다른 기기로 옮길 때는 게임의 세이브 슬롯 화면에서 **TRANSFER PROGRESS**를 누른다. **CREATE CODE**로 코드를 만들고 다른 기기에서 입력한 뒤 **CHECK CODE → RESTORE**를 누른다. 복원 확인창에서 슬롯·경험치·레벨을 확인한다. **CANCEL**은 현재 저장을 유지한다.

Tauri Mac 앱은 같은 웹 게임을 앱 창에서 실행하고 OS 파일에도 저장을 백업한다. Apple Silicon과 macOS 14 이상을 대상으로 한다. 현재 [Tauri 빌드 방법](../../Desktop/Tauri/README.md)으로 앱을 만들 수 있다. 공개 설치본의 서명·공증은 후속 과제다. [플랫폼별 저장과 검사](../reviews/mac-web-pwa-comparison-20261101.md).


현재 main의 Apple·Android 일반 앱과 치트 검증 앱을 빌드하는 방법이다. 두 앱은 같은 게임 코어를 쓰며 별도 앱 ID와 저장 공간을 사용한다.

[Apple 준비물](#준비물) · [Android 빌드와 설치](#android-빌드와-설치)

## 준비물

Apple Silicon Mac, iOS SDK와 Simulator 런타임을 설치한 Xcode, Python 3, CMake 3.20 이상, Git LFS가 필요하다. Xcode 프로젝트는 저장소에 포함돼 있다. 프로젝트의 `project.yml`을 변경할 때 XcodeGen으로 다시 생성한다.

2026년 10월 26일 작성 환경은 macOS 27.0.1, Xcode 27.1, CMake 4.4.4, Python 3.14.6, XcodeGen 2.45.4다. deployment target은 iOS/iPadOS 15.0, macOS 12.0이다. 현재 OpenAL 빌드 스크립트는 arm64를 대상으로 한다.

## 소스와 자원 받기

```sh
git clone https://github.com/gelb2/HeavyMach.git
cd HeavyMach
git lfs install --local
git lfs pull
```

이미지·음원·맵·앱 아이콘·이미지 비교 자료는 저장소에 포함돼 있다. 앱이 사용하는 자원은 `HeavyMachAssets.bundle`에 포함된다.

공개 저장소이므로 로그인 없이 소스를 내려받을 수 있다. `v0.9`는 자원을 Git에 넣기 전의 제품 코드 태그다. 새로 빌드할 때는 자원이 포함된 현재 `main`을 사용한다. [v0.17 Releases](https://github.com/gelb2/HeavyMach/releases/tag/v0.17)에 소스·자원 압축본, 고지 묶음과 SHA-256 목록을 등록했다. 압축본을 풀면 Git LFS 없이 자원 검사를 시작할 수 있다. 앱은 Xcode에서 본인의 개발 팀을 지정해 빌드하고 설치한다.

## Android 빌드와 설치

Android 앱은 v0.17에 포함됐다. 위 Git clone·LFS 명령이나 v0.17 소스·자원 압축본으로 받는다. Android 8.0(API 26) 이상, arm64-v8a 기기를 대상으로 하며 Galaxy SM-S731N의 Android 16에서 플레이했다.

JDK 17, Python 3, Android SDK Command-line Tools가 필요하다. Android Studio의 SDK Manager 또는 다음 명령으로 SDK 36·NDK·CMake·ADB를 설치한다.

```sh
sdkmanager "platform-tools" "platforms;android-36" "build-tools;36.0.0" "ndk;26.3.11579264" "cmake;3.22.1"
sdkmanager --licenses
```

SDK 경로를 `ANDROID_HOME`에 지정하거나 Android Studio에서 `Android/` 폴더를 열어 SDK 위치를 설정한다. 명령행에서는 SDK의 `platform-tools`와 `cmdline-tools/latest/bin`을 PATH에 추가한다. 일반 앱과 치트 앱을 빌드하고 설치한다.

```sh
cd Android
./gradlew :app:assembleNormalDebug :app:assembleCheatDebug
adb devices
adb -s SERIAL install -r app/build/outputs/apk/normal/debug/app-normal-debug.apk
adb -s SERIAL install -r app/build/outputs/apk/cheat/debug/app-cheat-debug.apk
```

휴대폰에서 USB 디버깅을 켜고 연결한 컴퓨터의 RSA 키를 허용한다. `SERIAL`은 `adb devices`에 표시되는 기기 ID로 바꾼다. 설치 후 앱 목록에서 **Heavy Mach 1** 또는 **HM Validation**을 연다. 일반·치트 앱은 서로 다른 저장 공간을 사용한다. 같은 서명의 앱을 `install -r`로 갱신하면 기존 저장을 유지한다.

| 앱 | APK 경로 — Android 폴더 기준 |
|---|---|
| 일반 | `app/build/outputs/apk/normal/debug/app-normal-debug.apk` |
| 치트 | `app/build/outputs/apk/cheat/debug/app-cheat-debug.apk` |

현재 APK는 게임 코드를 최적화한 debug 서명본이다. 배포 서명, 장시간 발열·메모리, Android 태블릿 검사는 후속 과제다. Android 11 이상 기기는 개발자 옵션의 무선 디버깅으로 페어링해 설치할 수 있다. [USB·무선 연결과 검사 도구](../../Android/README.md), [개발·오류 수정·기기 검사](android-development-20261030.md), [Galaxy 일반 앱 플레이](android-gameplay-20261030.md).

## 자원 검사

```sh
python3 tools/verify_build_resources.py
```

빌드·검사 자원 1,505개의 크기와 SHA-256을 확인한다. 누락·손상·다운로드되지 않은 LFS 파일이 있으면 종료 코드 1을 반환한다. LFS 포인터가 남았다면 `git lfs pull`을 실행하고 다시 검사한다.

## 시뮬레이터 빌드

저장소 루트에서 음향 라이브러리를 먼저 준비한다. 이어 일반 앱을 빌드한다.

```sh
bash tools/build_openal.sh iphonesimulator
xcodebuild -workspace HeavyMach.xcworkspace \
  -scheme HeavyMachRestored -configuration Debug \
  -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' \
  -derivedDataPath build/ios-simulator \
  ARCHS=arm64 ONLY_ACTIVE_ARCH=YES CODE_SIGNING_ALLOWED=NO build
```

산출물은 `build/ios-simulator/Build/Products/Debug-iphonesimulator/HeavyMachRestored.app`이다. 치트 앱은 같은 명령에서 scheme을 `HeavyMachValidation`으로 바꾼다. 산출물 이름도 `HeavyMachValidation.app`으로 바뀐다.

Simulator에서 iPhone 한 대를 부팅한 뒤 설치하고 실행한다.

```sh
xcrun simctl install booted build/ios-simulator/Build/Products/Debug-iphonesimulator/HeavyMachRestored.app
xcrun simctl launch booted com.gelb2.heavymach.restored
```

여러 Simulator가 부팅돼 있으면 `xcrun simctl list devices booted`에서 기기 ID를 확인하고 두 명령의 `booted`를 해당 ID로 바꾼다. 치트 앱은 경로의 앱 이름과 앱 ID를 각각 `HeavyMachValidation.app`, `com.gelb2.heavymach.validation`으로 바꾼다.

## iPhone·iPad 설치

Xcode에서 `HeavyMach.xcworkspace`를 연다. 사용할 target의 **Signing & Capabilities**에서 본인의 개발 팀을 선택한다. 저장소에는 기존 개발 팀의 ID가 기록돼 있다. 앱 ID 등록 충돌이 발생하면 본인 팀에서 사용할 고유한 Bundle Identifier를 지정한다.

아이폰을 연결하고 Mac을 신뢰한 뒤 개발자 모드를 켠다. 실행 대상을 연결한 iPhone 또는 iPad로 바꾸고 일반 앱은 `HeavyMachRestored`, 치트 앱은 `HeavyMachValidation` scheme으로 Run한다. Xcode의 빌드 단계가 `iphoneos`용 OpenAL을 준비한다.

기존 설치를 갱신할 때는 먼저 Xcode의 **Devices and Simulators**에서 해당 앱의 컨테이너를 내려받아 저장 데이터를 보관한다. 같은 앱 ID를 유지하면 기존 앱을 업데이트한다. 앱 ID를 바꾸면 별도 앱으로 설치되며 이전 저장이 자동으로 옮겨지지 않는다.

`tools/prepare_device_validation.py`는 기존 검증용 아이폰과 앱 ID에 고정돼 있다. 다른 아이폰의 설치에는 위 Xcode 경로를 사용한다.

## 실행 확인과 오류 해결

일반 앱은 타이틀→슬롯→모드·미션→전투 순서로 진입한다. 치트 앱은 타이틀의 검증 패널에서 시작 조건을 지정한다. 이동·점프·사격·무기 전환, 메뉴 복귀 후 진행 복구를 확인한다. 전체 무기 검사는 [검증 절차](../weapon-validation-procedure.md)를 따른다.

| 증상 | 확인할 항목 |
|---|---|
| `libopenal.a`를 찾지 못함 | 선택한 SDK에 맞게 `build_openal.sh`를 먼저 실행했는지 확인한다. |
| x86_64 링크 실패 | 현재 라이브러리는 arm64다. 위 명령의 `ARCHS=arm64`와 Apple Silicon 실행 대상을 확인한다. |
| 그림이 표시되지 않음 | `git lfs pull`과 자원 검사를 실행하고 앱 번들 안 `HeavyMachAssets.bundle/OriginalImages`를 확인한다. |
| 맵·무기 데이터를 찾지 못함 | `git lfs pull`과 자원 검사를 실행해 `Resources/RawData`를 확인한다. |
| 서명 실패 | Xcode의 팀·인증서·기기 등록 오류를 확인한다. 기존 작업에서는 CLI 서명 실패 후 Xcode GUI 빌드로 설치했다. |
| 이미지 비교 테스트에서 자료 없음 | 저장소에 포함된 `Resources/Validation/ImageDecodeParity.json`을 자원 검사로 확인한다. |

코어 검사는 자원 검사를 통과한 뒤 실행한다.

```sh
cmake -S . -B build/core -DCMAKE_BUILD_TYPE=Debug
cmake --build build/core -j 4
ctest --test-dir build/core --output-on-failure
```

[새 원격 clone 검사](clone-build-20261026.md)에서 GitHub의 소스·LFS 자원으로 빌드 4개, 코어 17개·iOS 14개 검사와 새 Simulator의 두 앱 실행을 통과했다. 현재 플랫폼별 검사 결과는 [검증 문서](validation.md)에 있다.

## Mac 빌드와 실행

Xcode에서 같은 workspace를 열고 `HeavyMachMac` 또는 `HeavyMachMacValidation` scheme과 **My Mac**을 선택한다. 각 앱 target의 Signing & Capabilities에서 본인의 개발 팀과 Apple Development 인증서를 지정한 뒤 Run한다. AppKit 앱으로 실행되며 키보드로 전투와 메뉴를 조작한다.

빌드만 확인하려면 다음 명령을 사용한다.

```sh
bash tools/build_openal.sh macosx
xcodebuild -workspace HeavyMach.xcworkspace \
  -scheme HeavyMachMac -configuration Debug \
  -destination 'platform=macOS,arch=arm64' \
  -derivedDataPath build/mac-documentation \
  CODE_SIGNING_ALLOWED=NO build
```

산출물은 `build/mac-documentation/Build/Products/Debug/HeavyMachMac.app`이다. 치트 앱은 scheme과 산출물 이름을 `HeavyMachMacValidation`으로 바꾼다. 위 명령은 서명하지 않는다. 실행·디버깅·누수 검사는 Xcode에서 개발 인증서로 서명한 앱을 사용한다. 개발 팀이 없는 임시 서명에서 시스템 등록 재시도와 연결 누수가 발생했던 기록은 [Mac 검사](apple-platform-progress-20261026.md)에 있다.

## Scheme과 앱 구분

| 대상 | 일반 | 치트 |
|---|---|---|
| iPhone·iPad·Simulator | HeavyMachRestored | HeavyMachValidation |
| native Mac | HeavyMachMac | HeavyMachMacValidation |

일반·치트 앱은 별도 ID와 저장 공간을 사용한다. iOS 치트 앱 ID는 `com.gelb2.heavymach.validation`이다. 키 배치는 [조작 안내](gameplay-guide.md), 기기 무선 설치는 [무선 개발 절차](../wireless-device-workflow.md)에 있다.

## 프로젝트 재생성

소스 파일·의존 모듈·빌드 설정을 바꿀 때 해당 YAML과 생성 프로젝트를 함께 갱신한다.

```sh
xcodegen generate --spec Modules/project.yml
xcodegen generate --spec iOS/project.yml
xcodegen generate --spec macOS/project.yml
```

일상적인 빌드에서는 저장소에 포함된 프로젝트를 그대로 사용한다. [workspace 구조](architecture.md)

[복원 코드 형식·플랫폼별 복원 범위와 화면](save-recovery-20261102.md)
