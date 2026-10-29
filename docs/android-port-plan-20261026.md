# HeavyMach 안드로이드 APK 개발 계획

2026-10-26 초안, 2026-10-30 사용자 후속 지시 반영. **기존 Apple 소스·프로젝트·공용 코어·에셋을 변경하지 않고 Android 앱을 추가한다.** 일반용·치트용 C++ 코어는 수정 없이 Android ARM64로 컴파일됐다. Android 전용 실행·메뉴·HUD·OS 연결 코드를 `Android/`에 작성한다. APK와 Android 플레이는 아직 미구현이다.

## 2026-10-26 최초 조사 범위

당시 기준은 HeavyMach main `7c73e06fe5405a880226e554ef8de0174b7238f0`과 정상 플레이 기준 태그 `v0.9`다. 현재 앱은 C++20·Objective-C++로 구현돼 있다. `Sources/Core`의 전투·수치·진행·저장 바이트 해석을 재사용한다. `Sources/iOS`의 Objective-C++ 9개 파일, 총 2,320줄은 플랫폼 API와 메뉴·프레임 조립이 섞인 범위다.

| 현재 코드 | 재사용 또는 이식 방법 |
|---|---|
| Sources/Core, CMakeLists.txt | 같은 C++ 파일을 NDK로 컴파일. Android용 게임 규칙 복사본을 만들지 않음 |
| HMGLRenderer.mm | OpenGL ES 1.1 그리기 순서를 유지. EAGL/CAEAGLLayer를 EGL/ANativeWindow로 교체 |
| HMGameView.mm | 기존 파일 유지. 순서·난수·정지 조건을 대조하며 Android 전용 실행 코드 작성 |
| HMMenus.mm, HMGameHUD.mm | 기존 파일 유지. 상태·레이아웃·버튼 동작을 Android 전용 C++로 이식. 규칙·좌표·호출 순서 보존 |
| HMMedia.mm, HMCompatibilityPixels.hpp, vendor/stb | 동일 디코더·POT·색·UV 처리. NSBundle/NSData/UIImage 보관 부분을 Android 자원 읽기와 C++ 메타데이터로 교체 |
| HMOpenALAudio.mm, AudioContract.hpp | OpenAL Soft 1.22.2와 source 재시작·거리 모델 유지. Android 전용 C++ 래퍼와 출력 backend 연결. 기존 Apple 래퍼 유지 |
| HMValidationPanel/Controls | 치트 APK에 조건 설정·이동·점프·발사·무기 전환 제공. 일반 APK에는 컴파일하지 않음 |
| Resources, Art | 저장소에서 관리하는 게임 자원·채택 확장 에셋의 파일명·해시 유지. APK assets에 같은 파일을 패키징 |

메뉴와 프레임 조립은 코어 밖에도 있다. Android판은 이 동작을 별도 구현하고 기존 Apple판을 비교 기준으로 사용한다. Apple판의 소스 추출·호출 변경은 하지 않는다.

## Android 구성안

HeavyMach 저장소 하나를 유지하고 Android 전용 파일은 모두 최상위 `Android/` 아래에 둔다. **앞서 제안한 Shared·Apple 폴더 이동과 Apple 코드의 선행 공통화는 철회한다.** 기존 워크스페이스와 iOS·macOS 경로를 그대로 유지한다. [저장소 관리 규칙](repository-management.md#플랫폼별-폴더와-공용-모듈--2026-10-30).

```text
HeavyMach/
  HeavyMach.xcworkspace      기존 경로 유지
  iOS/                      기존 프로젝트 유지
  macOS/                    기존 프로젝트 유지
  Modules/                  기존 Apple 모듈 구성 유지
  Sources/
    Core/                   Android도 같은 C++ 소스를 참조
    iOS/                    기존 Apple 실행·메뉴·HUD 유지
    Apple/                  기존 Apple 연결 유지
    Mac/                    기존 Mac 코드 유지
  Resources/                기존 게임 자원 유지
  Art/                      기존 추가 에셋 유지
  vendor/                   기존 의존성 소스 유지
  Android/                  새로 만들 Android 전용 프로젝트
    app/src/main/java/      Kotlin Activity·생명주기·터치·센서
    app/src/main/cpp/       JNI·실행·메뉴·HUD·EGL·미디어·음향 연결
    app/src/androidTest/    Android 앱 검사
    cmake/                  기존 Core·vendor를 참조하는 빌드 설정
    tools/                  자원 패키징·기존 파일 보존 검사
    gradle/                 Gradle wrapper
    .gitignore              Android 로컬 설정·캐시·서명·산출물 제외
```

Android Studio는 `Android/`를 연다. Android CMake가 루트의 기존 코어 타깃을 별도 출력 디렉터리에서 빌드하고 JNI 공유 라이브러리에 연결한다. XcodeGen·Apple 프로젝트·루트 CMake·Apple 음향 빌드 스크립트를 수정하지 않는다. 기존 에셋은 읽어서 Android 빌드 산출물에 패키징하며 원본을 덮어쓰지 않는다. 플랫폼별 영구 브랜치는 만들지 않는다.

Kotlin은 Activity·OS 통지·입력·센서를 맡는다. C++ 상태를 직접 참조하는 실행·메뉴·HUD는 Android 전용 C++로 옮기고 기존 코어 API를 호출한다. JNI는 상태 전환·입력 묶음 등 제한된 연결에 사용한다. 스프라이트마다 Kotlin과 C++를 왕복하거나 게임 규칙을 Kotlin으로 다시 쓰지 않는다.

앱 진입은 얇은 Kotlin Activity와 SurfaceView, 게임 실행은 하나의 native thread를 기본안으로 한다. JNI는 수명·입력·화면 정보 전달에 한정하고 sprite마다 호출하지 않는다. 입력·센서는 timestamp와 함께 큐로 전달하고 게임 상태·GL context는 지정한 스레드만 변경한다. Gradle의 externalNativeBuild로 CMake와 shared library를 묶을 수 있다. [공식 CMake 연결 안내](https://developer.android.com/studio/projects/gradle-external-native-builds).

GameActivity도 후보지만 초기 UI·센서·회전 연결에 필요한 범위부터 비교한다. 초기 단계에서 Activity 방식과 함께 검토하고 하나로 고정한다. 두 실행 루프를 동시에 유지하지 않는다. [공식 GameActivity 안내](https://developer.android.com/games/agdk/game-activity).

첫 대상은 minSdk 26, arm64-v8a 휴대폰으로 제안한다. 이는 조사용 코어 컴파일 조건이며 지원 하한을 확정한 출시 약속은 아니다. x86_64는 필요 시 에뮬레이터용으로 추가한다. 일반 `com.gelb2.heavymach.restored`, 치트 `com.gelb2.heavymach.validation` 패키지를 제안하며 두 앱의 데이터 디렉터리를 분리한다.

## 그래픽·프레임·입력

Android NDK는 GLES 1.x와 `libGLESv1_CM`, EGL을 제공한다. 따라서 첫 기준판은 현재 GLES 1.1 경로를 유지할 수 있다. 다만 `OES_framebuffer_object`, RGBA8 renderbuffer 등 실제 사용 확장과 EGLConfig를 기기에서 확인해야 한다. 지원이 부족하면 먼저 해당 GL 호출·픽셀 차이를 기록한다. ES 2/3 셰이더나 Vulkan 전환은 구체적인 호환 실패와 별도 비교 후 결정한다. [NDK 그래픽 API](https://developer.android.com/ndk/guides/stable_apis).

논리 W×320, 원본 비교 480×320, sprite blend·GL_REPLACE·RGB565 변환·LINEAR/REPEAT와 최종 LINEAR/CLAMP_TO_EDGE를 유지한다. Android Surface에는 검증된 같은 최종 quad를 표시한다. 게임 프레임을 Canvas/Bitmap/Compose로 매번 합성하지 않는다. Surface 소멸과 context 소실을 구분하고, GPU 자원은 원본에서 재생성하되 진행 상태를 초기화하지 않는다.

현재의 한 논리 프레임당 `0.01666666753590107` 증가·한 번 tick/draw·별도 50Hz 센서 정책을 유지한다. 표시 동기화는 Choreographer/EGL을 조사하고, 90/120Hz에서 게임이 1.5/2배 진행하지 않게 60Hz 논리 프레임만 선택한다. 놓친 프레임이나 백그라운드 시간을 몰아서 tick하지 않는다. Swappy는 표시 간격을 조정하는 후보로 검토한다. 실제 60/90/120Hz callback·tick·present 수를 대조한 뒤 적용한다. [Frame Pacing](https://developer.android.com/games/sdk/frame-pacing).

터치 좌표는 Surface 실제 픽셀→현재 viewport→월드/HUD 변환을 한 번만 적용한다. dp·상태바·cutout·가로 회전·멀티터치 pointer ID와 CANCEL을 다룬다. 방향 버튼은 치트판에서 실제 입력 경로를 호출한다. SensorManager의 가속도는 m/s²이므로 g 단위로 변환하고 기기 기본 방향과 현재 display rotation으로 축을 바꾼다. 센서 축은 OS 회전으로 자동 변경되지 않으며 태블릿의 기본 방향을 세로로 가정할 수 없다. [센서 좌표 안내](https://developer.android.com/develop/sensors-and-location/sensors/sensors_overview).

양쪽 가로 방향만 플레이하도록 `userLandscape`를 기본 후보로 둔다. OS 자동 회전 잠금도 존중한다. Android 16의 큰 화면에서는 방향 제한이 무시될 수 있다. portrait/multi-window/fold 상태가 강제되면 진행을 정지하고 가로 게임 영역과 안내를 유지하는 별도 처리를 검사한다. 접이식 아래 조작부는 첫 APK 이후 별도 UI 과제다. [Activity 방향·큰 화면 동작](https://developer.android.com/guide/topics/manifest/activity-element).

## 음향·저장·중단과 복귀

현재 vendor OpenAL Soft에는 Android OpenSL·Oboe backend 조건이 있다. 첫 동등성 검사에는 기존 1.22.2 소스를 NDK로 빌드하고 OpenSL backend를 우선 확인한다. 출력 지연·기기 문제로 Oboe가 필요하면 backend만 비교한다. 31개 WAV의 buffer/source를 준비해 재사용하고 source 위치→stop→play 순서를 지킨다. 무기 전환음·동시 효과음·볼륨·출력장치 전환은 실물 스피커로 확인한다.

`HvM.dat`의 3,624바이트 형식과 정상 저장 이벤트를 보존하고 앱 내부 filesDir에 원자 저장한다. 사격·피격·EXP 변화·매 프레임·background 진입에 파일 쓰기를 추가하지 않는다. 프로세스가 종료되면 마지막 정상 저장에서 이어간다. OS 복원용 임시 상태를 영속 저장 규칙과 혼동하지 않는다. iPhone 저장 가져오기는 별도 요청이 있을 때 백업·해시 대조 후 처리한다.

onPause·focus 상실·Surface 파괴·오디오 focus 상실에 대한 상태 전이를 정한다. 게임·터치·센서·음향을 멈추고 복귀하면 정지 메뉴에서 명시 재개한다. background에서 GL 호출을 하지 않고 과거 터치·소리를 재생하지 않는다. foreground 120초 무입력 정책도 유지한다. Activity 재생성·프로세스 종료·GL context 복구를 서로 다른 검사로 둔다.

## 빌드 도구와 이번 실행 결과

로컬에는 JDK 17.0.14, SDK 34/35/36/36.1, NDK 26.3.11579264, CMake 3.22.1, adb·Emulator와 API 36 AVD들이 있다. `/Applications/Android Studio.app`는 없었다. 다른 설치 위치까지 전수 조사한 것은 아니다. CLI 빌드는 가능하며 조사 시 adb에 연결된 기기는 없었다.

실제 실행한 명령은 다음과 같다. 성공 산출물은 ARM64 정적 라이브러리이며 APK가 아니다.

```sh
cmake -S . -B build/research-20261026/android-core-probe \
  -DCMAKE_TOOLCHAIN_FILE=/Users/sokol/Library/Android/sdk/ndk/26.3.11579264/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26 \
  -DCMAKE_BUILD_TYPE=Release -DHM_BUILD_VALIDATION_TESTS=OFF
cmake --build build/research-20261026/android-core-probe --target HeavyMachCore -j 4
```

20개 소스가 컴파일됐고 `GlobalInfo.cpp.o`는 AArch64 ELF였다. GameSession의 필드 초기화 경고 9개와 NDK CMake 최소 버전 폐기 예고는 남았다. 조사 때문에 소스를 변경하지 않았다. Android에서 코어 검사를 실행하거나 OpenAL·EGL을 빌드한 것은 아니다. [입력 커밋·도구·산출물 해시](https://github.com/gelb2/HeavyMach/blob/main/docs/research/android-and-titles-evidence-20261026.json).

APK용 도구는 조사용 NDK와 구분한다. 첫 구성안은 AGP 8.13.2·Gradle 8.13·JDK 17·compile/targetSdk 36, NDK r28 이상 중 설치한 한 버전 고정이다. AGP 공식 호환표가 API 36.1·Gradle 8.13·JDK 17을 지원한다. 새 도구 설치와 Gradle dependency resolution은 아직 실행하지 않았다. [AGP 호환표](https://developer.android.com/build/releases/agp-8-13-0-release-notes).

16KB 페이지 환경을 초기 APK 기준에 포함한다. NDK r28 이상과 AGP 8.5.1 이상을 쓰고 OpenAL·C++ runtime 등 모든 native library의 ELF/ZIP 정렬을 확인한다. 이번 NDK r26 정적 라이브러리 성공은 그 증거가 아니다. [16KB 빌드·검사 안내](https://developer.android.google.cn/guide/practices/page-sizes?hl=en).

## 구현 순서와 완료 조건

| 단계 | 만들 것 | 다음 단계로 넘어갈 근거 |
|---|---|---|
| A0 조사 | 코드 경계·코어 NDK 컴파일·개발 계획 | 10월 30일 최신 일반용·치트용 코어 컴파일 완료. 아래 경계 조사 참조 |
| A0.5 변경 경계 고정 | 기존 Apple·코어·자원·vendor 해시와 Android 단독 빌드 경로 기록 | 보호 파일의 수정·삭제·이동 0건 |
| A1 앱 뼈대 | Android Gradle·JNI·Surface·EGL·일반/치트 variant, assets 준비 | 두 debug APK 설치·콜드 실행, GL vendor/config·자원 해시 기록 |
| A2 Android 화면 | 기존 동작을 옮긴 Android 전용 메뉴·HUD·프레임 조립·GLES·미디어 | 타이틀→슬롯→모드→미션→전투를 기존 Apple판과 대조 |
| A3 입력·실행 | 틸트·점프·사격·전체 무기·음향·저장·pause/resume | 치트 전 무기 검사, 양방향 터치/실제 틸트, 저장 횟수·바이트 비교 |
| A4 회귀·성능 | 전체 미션·Survival, 화면비·context 소실·120Hz·16KB | 차이 목록 해결, 에뮬레이터와 실기기 결과 분리, 장시간 trace |
| A5 APK 인계 | 일반 release·치트 검증 APK, 재현 가능한 빌드·서명·해시 | 일반판 치트 코드 제외, 서명 확인, 업데이트 저장 유지, 설치·플레이 기록 |

A2는 Android 전용 코드에서 작은 단위로 진행한다. 기존 Apple판의 동일 장면·입력·난수·이벤트를 대조하고 차이가 나면 Android API 연결·좌표·tick 순서를 조사한다. Apple판을 Android 결과에 맞춰 수정하지 않는다. 전체 구현 기간은 아직 산정하지 않는다. A1의 EGL/OpenAL 실기기 결과가 있어야 A2 이후의 비용을 판단할 수 있다.

개발 브랜치는 `feat/android-bootstrap`, `feat/android-renderer`처럼 develop에서 나눈다. 작업 브랜치→develop→main을 지키고 v0.9를 이동하지 않는다. 새 버전 태그는 Android 완료 상태와 확인 범위가 정해진 뒤 부여한다.

## APK 생성·설치 절차

Android 프로젝트 구성 후 사용할 예정 명령이다. Gradle wrapper와 task 작성은 A1 단계에 포함된다.

```sh
cd Android
./gradlew :app:assembleNormalDebug :app:assembleCheatDebug
./gradlew :app:connectedCheatDebugAndroidTest
./gradlew :app:assembleNormalRelease
apksigner verify --verbose --print-certs app/build/outputs/apk/normal/release/app-normal-release.apk
zipalign -c -P 16 -v 4 app/build/outputs/apk/normal/release/app-normal-release.apk
adb -s DEVICE_SERIAL install -r app/build/outputs/apk/cheat/debug/app-cheat-debug.apk
```

출력명은 실제 variant 구성 뒤 task·산출물과 맞춘다. Debug와 release 서명 키를 구분하고 release 키를 Git에 넣지 않는다. 키는 새 앱 업데이트에도 같은 키를 쓸 수 있도록 보존한다. 패키지명·versionCode·commit·ABI·에셋 해시·APK SHA-256·서명 인증서 지문을 인계 기록에 남긴다. APK 빌드·서명·설치·정상 플레이를 각각 판정한다. [Gradle APK 생성](https://developer.android.com/build/building-cmdline), [apksigner 검사](https://developer.android.com/tools/apksigner).

## 검증 계획

치트판에서 모든 플레이어 무기와 세 필살기를 실제 데이터로 열거한다. 기본 이동·점프·사격·전환·연사·탄약·피격·아이템·EXP·업그레이드·보스·미션 전환·결과 일러스트·저장 재실행을 검사한다. 화면 양 끝·탱크 후방·미션 3-5 항공기·원본480/확장폭을 회귀 조건에 포함한다. 동일 seed와 입력·프레임 수로 iOS/Android 상태·이벤트·저장 바이트를 비교하고 부동소수 차이는 발생 지점과 영향부터 조사한다.

에뮬레이터는 자동 조건 재현·경계 터치·회전·Activity 재생성·저장·메모리 오류를 검사한다. 실기기는 센서 방향·스피커·GPU driver·60/90/120Hz·발열·입력 지연을 확인한다.

프레임 계측에는 callback, simulation tick, CPU draw, EGL swap, 실제 표시 간격을 따로 남긴다. 초기 열 상태를 맞춰 고정 장면·연사·보스·메뉴 왕복을 20~30분 반복하는 검사를 제안한다. 평균·p95·p99·최대·16.67ms/33.3ms 초과 프레임 수와 메모리·파일 쓰기·이미지 디코딩 횟수를 기록한다. 짧은 Perfetto trace 구간을 함께 확보한다. Native Surface의 GPU 비용은 Perfetto로 추적한다. [Android system tracing](https://developer.android.com/topic/performance/tracing), [Perfetto 기록](https://perfetto.dev/docs/getting-started/system-tracing).

## 사용할 스킬과 첫 작업

이 프로젝트의 `port-heavymach-android` 스킬을 추가했다. 계획·기준·완료 조건을 먼저 읽고 해당 단계만 실행하도록 한다. 오류 재현은 engineering-debug, 의미 있는 회귀 검사는 engineering-test, 원본 동작이 불명확할 때만 reverse-engineer-anything을 사용한다. 중요한 공통 코드 변경 검토에는 engineering-review, 보고에는 write-project-text와 present-game-artifacts/Sites를 적용한다.

다음 개발 작업은 A0.5의 기존 파일 해시와 변경 허용 범위 고정이다. A1에서 Android 도구 버전을 고정하고 EGL·OpenAL을 연결한 치트 debug APK부터 실행한다. 게임 동작을 이식하기 전에 논리 버퍼·최종 표시·입력·음향의 기준을 확보한다.


## 코어와 플랫폼 경계 조사 — 2026-10-30

사용자의 질문은 Android를 추가할 때 기존 iOS 코드나 게임 코어를 고쳐야 하는지였다. 조사 기준은 `develop`의 `d46e122ac49ebdf8a3eb5d89b6af4ea5ccf18cd8`이다. 해당 시점 `main`과 `develop`의 파일 내용은 같았다.

**게임 코어는 그대로 재사용할 수 있다.** Apple 실행 모듈은 OS 처리와 게임 진행 처리가 함께 있다. 이를 Apple에서 추출하는 최초 제안은 후속 사용자 지시로 철회했다. 아래 경계는 Android 전용 구현을 작성할 때 참고한다.

| 확인한 코드 | 현재 동작 | Android 작업 시 처리 |
|---|---|---|
| `Sources/Core` 56개 파일 | 표준 C++·내부 헤더만 포함. Apple·Android SDK 의존성 없음 | 같은 소스를 NDK·Xcode에서 컴파일 |
| `GameSession.hpp` | 입력·tick을 받고 그리기 항목·음향·저장 이벤트를 반환 | Android 입력·출력 연결만 추가 |
| `SaveData.hpp` | 바이트를 읽고 저장용 바이트를 생성 | Android 파일 어댑터가 기존 저장 이벤트에만 원자 저장 |
| `GameData::loadDirectory` | 표준 C++ 파일 읽기·디렉터리 탐색 포함 | Android AssetManager로 읽은 바이트를 기존 공개 `parseWeapons/parseParts/parseMachines/parseMap`에 전달 |
| `HMGameView::advanceOriginalFrame` | 시간 증가, 메뉴·세션·이벤트·HUD 실행 순서를 Objective-C++ 뷰에서 결정 | Android 전용 실행 코드에 같은 순서·정지 조건 구현. 기존 Apple 코드 유지 |
| `HMGameHUD::changeWeaponForSlot` | 기본 무기 선택 시 슬롯의 탄약 값을 99로 설정 | Android에서도 같은 값·호출 시점 보존. 기존 Apple HUD 유지 |
| `HMMenus::activate` | 슬롯 삭제·선택, 진행에 따른 미션 선택 제한과 화면 전환 | Android 메뉴 제어 코드에 동일 규칙 구현. 기존 Apple 메뉴 유지 |
| `HMMedia`, `HMGLRenderer`, `HMOpenALAudio` | 이미지·GL·OpenAL 처리와 Foundation·Apple 화면 API가 결합 | 같은 처리 규칙을 쓰는 Android 자원·표시·출력 연결 추가 |

`HMGameHUD`의 탄약 변경은 현재 복원된 규칙이다. 이 조사에서는 제거하거나 값을 바꾸지 않았다. Android 이식 시 이 변경이 일어나는 시점까지 보존한다. `GameData::loadDirectory`도 파일 접근을 코어에 포함한다는 점에서 역할이 섞여 있다. 다만 바이트 파서가 이미 공개돼 있어 Android 지원을 위해 이 함수를 수정할 필요는 없다.

Android 전용 실행 코드는 기존 `GameSession` API를 호출한다. Apple 클래스나 JNI 객체를 코어에 추가하지 않는다. Android 생명주기·Surface/EGL 소멸·음향 focus는 Android 코드에서 처리한다. 전투 정지·입력 해제·복귀 후 명시 재개·저장 시점은 기존 게임 정책을 따른다.

```text
기존 Apple 앱 → 기존 Apple 실행 코드 → 기존 C++ Core
새 Android 앱 → 새 Android 실행 코드 → 같은 C++ Core
```

메뉴·HUD 동작을 나중에 변경하면 두 구현을 각각 수정·검사해야 한다. 이 중복 관리는 기존 Apple판을 수정하지 않는 개발 방식의 비용으로 둔다. 추후 공통화는 별도 과제이며 Android 착수 조건으로 두지 않는다.

### 실제 컴파일 결과

NDK `26.3.11579264`, `arm64-v8a`, API 26, Release 조건으로 일반용 `HeavyMachCore`와 치트용 `HeavyMachValidationCore`를 모두 빌드했다. 일반용은 C++ 구현 파일 22개, 치트용은 검증 진입점 1개를 추가한다. 소스·헤더·inc 파일 56개의 해시와 산출물 해시는 [검사 기록](evidence/android-boundaries-20261030.json)에 있다.

```sh
cmake -S . -B build/android-boundary-audit-20261030 \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26 \
  -DCMAKE_BUILD_TYPE=Release -DHM_BUILD_VALIDATION_TESTS=ON
cmake --build build/android-boundary-audit-20261030 \
  --target HeavyMachCore HeavyMachValidationCore -j 4
```

`ANDROID_NDK_HOME`은 해당 NDK 설치 경로로 지정한다. 두 정적 라이브러리 생성은 성공했다. 기존 필드 초기화 경고가 타깃마다 9개 남았다. Android 앱 실행·EGL 표시·음향은 다음 APK 단계에서 검사한다.

### 기존 파일을 보존하는 구현 순서

1. Apple 소스·프로젝트·공용 코어·에셋·vendor·빌드 스크립트의 경로와 해시를 기록한다. Git 변경 목록과 대조해 수정·삭제·이동이 없는지 검사한다.
2. `Android/`에 Gradle·CMake·Kotlin·JNI를 추가한다. 기존 파일은 읽기 입력으로만 사용하고 빌드 산출물은 별도 경로에 쓴다.
3. Android 전용 실행·메뉴·HUD·미디어·렌더러·음향 코드를 작성한다. 기존 세 파일의 상태 소유·실행 순서·난수 소비·터치 우선순위를 보존한다.
4. 같은 입력·난수·프레임 수로 월드 상태·이벤트·저장 바이트를 대조한다. 기본 무기 탄약99, 슬롯 선택·삭제, HUD 편집, 중단·복귀, 전체 무기를 포함한다.
5. 보호 파일의 해시가 모두 같고 Android APK의 설치·플레이 검사가 통과해야 해당 구현 단계를 완료한다. Android에서 실패한 코드를 고치며 기존 Apple판을 변경하지 않는다.

이번 작업은 조사·컴파일·계획 수정까지다. Apple 파일 이동·수정과 Android 앱 구현은 실행하지 않았다.


## Apple 파일 무수정 재검사와 Android 착수 기준 — 2026-10-30

기존 Apple 소스·프로젝트·코어·자원·vendor 등 보호 파일 2,000개의 작업 전후 SHA-256이 일치했다. 빌드 입력을 수정하지 않고 일반용·치트용 ARM64 공유 라이브러리 두 개를 생성했다. 이 검사에는 코어 전체 오브젝트, OpenAL Soft의 OpenSL ES 백엔드, 기존 stb 디코더, JNI, EGL, GLES 1.x, Android AssetManager 연결이 포함된다. `--no-undefined`로 링크했고 ELF AArch64 형식을 확인했다. [소스·빌드 구성·해시](evidence/android-isolation-20261030.json).

| 항목 | 현재 확인한 것 | Android 앱에서 남은 구현·검사 |
|---|---|---|
| 코어 | 일반·치트 NDK 컴파일 및 전체 오브젝트 공유 라이브러리 링크 | 실제 게임 루프 연결·동일 입력 비교 |
| 자원 | 기존 빌드/검사용 1,504개 파일 SHA-256 일치, stb Android 컴파일·링크 | APK 패키징·읽기·크롭·축소·업로드·화면 비교 |
| 음향 | 기존 OpenAL Soft를 OpenSL ES 백엔드로 컴파일·링크 | 실제 31개 WAV 출력·focus·장치 변경·중단/복귀 |
| 그래픽 | EGL·GLES1 심벌 연결 | Surface·context 수명, GPU 자원 복구·실제 화면 |
| Apple 보존 | 보호 파일 2,000개 변경 0건 | 각 구현 단계에서도 동일 검사 |

이 검사는 라이브러리 생성까지다. Android APK·게임 실행·실제 출력·컨텍스트 복구는 아직 구현하지 않았다. NDK r26/API26은 이번 조사 조건이며 출시 APK의 16KB 대응 도구 구성은 별도로 진행한다.

### 공용 게임 기능

| 기능 | 기존 C++ 구현 |
|---|---|
| 탄도·발사·명중 | `Combat.cpp`, `Geometry.cpp`, `GameSession.cpp` |
| 자동조준 대상 선택·각도 계산 | `AutoAim.hpp`, `GameSession.cpp` |
| 캠페인·서바이벌 적 생성 | `World.cpp`, `GameSession.cpp` |
| 신규 유닛 배치·증원 | `VTOL.cpp`, `Fighters.cpp`, `GroundAA.cpp`, `GameSession.cpp` |
| 적 사망·화면 뒤 오브젝트 정리 | `Enemy.cpp`, `World.cpp`, `GameSession.cpp` |
| 공격력·강화·버프·방어·체력 반영 | `Combat.cpp`, `Enemy.cpp`, `Player.cpp` |

이 기능들은 같은 코어를 호출한다. Android에서 수식을 다시 작성하지 않는다. 자동조준 버튼·표적 장식·음향·실제 그림 출력은 Android 연결 코드가 담당한다. 신규 유닛 데이터 추가 호출은 기존 `addVTOLExpansion`, `addFighterExpansion`, `addGroundAAExpansion`을 같은 순서로 실행한다.

### Surface와 GL 컨텍스트 복구

복구 핸들러는 아직 없다. 현재 기본안은 Kotlin SurfaceView와 C++ EGL이다. GLSurfaceView를 선택하면 EGL·렌더 스레드를 프레임워크가 관리하고, 자체 EGL안은 Android 구현이 관리한다. A1에서 한 방식으로 고정하며 두 관리자를 동시에 쓰지 않는다.

GLSurfaceView의 `setPreserveEGLContextOnPause` 기본값은 false다. 일시정지 때 컨텍스트가 해제되고 재개 때 다시 생성될 수 있다. true 설정을 하더라도 손실된 자원을 복구하는 경로는 유지한다. 컨텍스트가 소실되면 그 컨텍스트의 텍스처·프레임버퍼 등 GL 자원은 유효하지 않다. [GLSurfaceView](https://developer.android.com/reference/android/opengl/GLSurfaceView), [Renderer 복구 안내](https://developer.android.com/reference/android/opengl/GLSurfaceView.Renderer).

- **Surface만 소멸:** 해당 화면으로의 그리기를 멈추고 렌더 스레드가 더 이상 사용하지 않음을 확인한 뒤 EGLSurface/ANativeWindow 참조를 정리한다. 컨텍스트가 살아 있으면 텍스처까지 다시 만들지 않는다.
- **컨텍스트 소멸:** 기존 GL 식별자와 상태 캐시를 무효화한다. 새 컨텍스트에서 텍스처·게임 논리 버퍼·프레임버퍼·GL 상태·viewport를 재구성한다. 이전 컨텍스트의 식별자로 새 컨텍스트에서 삭제·그리기를 하지 않는다. 자체 EGL에서는 `eglMakeCurrent`/`eglSwapBuffers` 오류를 분류하고 `EGL_CONTEXT_LOST`와 Surface 오류를 구분한다. GLSurfaceView에서는 `onSurfaceCreated`를 자원 재생성 진입점으로 쓴다.
- **게임 상태:** 프로세스가 살아 있으면 GameSession·탱크·적·탄환·난수·게임 시간은 유지한다. 렌더러 재생성 때문에 새 게임을 시작하거나 시간을 보충하지 않는다.
- **Activity 재생성:** 게임 세션과 Activity 참조 수명을 분리한다. 새 화면·입력을 연결하고 이전 Activity/Surface/JNI 참조를 해제한다.
- **프로세스 종료:** 기존 정상 저장을 읽어 재시작한다. 화면 복구와 다른 경로로 다룬다.
- **재개:** 앱 활성·유효 Surface·복구된 GPU 자원이 모두 준비돼야 표시한다. 전투는 정지 상태를 유지하고 사용자가 재개한다. 오디오 복귀도 이전 효과음을 몰아서 재생하지 않는다.

A1에서 타이틀/테스트 그림으로 강제 Surface 재생성·컨텍스트 파괴/재생성부터 검사한다. A2~A4에서는 전투·HUD 편집·로딩·결과 화면에서 홈 이동, 잠금, 양쪽 가로 회전, Activity 재생성, 연속 전환을 검사한다. 실제 OS에 의한 소실과 검증용 강제 소실을 구분해 기록한다. CPU 게임 상태 보존·GPU 자원 재생성·누락 그림·메모리 증가·멈춤 메뉴·입력 해제를 함께 확인한다.

### 에셋 파이프라인

기존 자원 목록은 원본 입력으로 유지한다. Android 빌드가 필요한 게임 자원을 목록으로 선택하고 `Android`의 생성 산출물 디렉터리에 `OriginalImages/`, `Images/`, `Audio/`, `RawData/`, `Extensions/` 구조로 준비한다. Apple asset catalog·검증 전용 파일·제작 중 시안은 제품 APK에서 제외한다. 같은 그림을 중복 패키징하지 않도록 실제 소비 파일을 확인한다. 선택한 파일의 경로·원본 해시와 APK에서 꺼낸 바이트를 대조한다.

AssetManager에서 바이트로 읽고, 맵·무기·부품 데이터는 기존 GameData 파서에 전달한다. ZIP 압축 자원의 파일 디스크립터 직접 접근을 가정하지 않는다. JNI로 전달한 AssetManager의 Java 수명도 native 사용 기간과 맞춘다. [NDK 자원 API](https://developer.android.com/ndk/reference/group/asset).

이미지는 기존 `vendor/stb` 버전과 iPhone PNG 변환·unpremultiply 설정을 유지한다. 현재 Apple 코드는 디코딩 이후에도 PNG 알파 곱셈, CoreGraphics 파츠 크롭·축소, RGB565/POT·UV 처리를 수행한다. Android에서도 이 절차를 구현해야 한다. 특히 쉴카·퉁구스카 파츠, VTOL·럭스루나에 축소, 자동조준 버튼의 크기·외곽선·투명도를 기준 화면과 비교한다. 축소 방식이 다른 결과는 실제 픽셀과 표시 크기로 확인한 뒤 조정한다. 기존 Apple 코드와 에셋은 바꾸지 않는다.

이미지 읽기·디코딩·축소는 준비 단계에서 수행하고 캐시한다. 매 프레임·매 발사마다 다시 읽지 않는다. 컨텍스트가 바뀌면 GPU 자원만 다시 올리고, CPU 캐시가 없으면 원본 자원으로 다시 만든다. 그림 준비가 끝나기 전에 전투를 재개하지 않는다.

### 오디오 백엔드

이번에 빌드·링크한 경로는 `게임 이벤트 → Android C++ OpenAL 래퍼 → 기존 OpenAL Soft → OpenSL ES`다. 기존 Apple의 OpenAL/CoreAudio 경로와 빌드 스크립트는 유지한다. 현재 vendored OpenAL에는 Oboe 연결 조건도 있지만 이번에는 Oboe를 설치·빌드하지 않았다. 첫 비교는 검증된 OpenSL ES 빌드로 진행하고 실제 지연·끊김 문제가 확인되면 Android 전용 출력 구성에서 Oboe/AAudio를 비교한다. [Oboe의 출력 API 선택](https://github.com/google/oboe).

31개 WAV의 PCM16 디코딩과 buffer/source 재사용, 위치→stop→play, 같은 효과음 재시작, 다른 효과음 동시 재생, 거리 감쇠를 기존 코드와 맞춘다. SoundPool·MediaPlayer로 게임 음향 규칙을 별도로 다시 만들지 않는다.

Kotlin AudioManager에서 게임용 audio focus를 요청하고 실제 획득 여부를 C++ 실행 코드에 전달한다. focus 상실·잠금·background에서 게임과 소리를 중단한다. 복귀 시 현재 출력 장치와 OpenAL 상태를 확인하며, 장치/컨텍스트를 다시 열어야 하면 source/buffer를 복구한다. 무조건 재생 호출만 반복해 복구로 간주하지 않는다. target API35 이상에서는 최상위 앱 등 focus 요청 조건도 확인한다. 이 게임에 background 재생용 서비스를 추가하지 않는다. [Android audio focus](https://developer.android.com/media/optimize/audio-focus).

실제 스피커에서 무기 전환·연사·동시 폭발·좌우 이동 소리를 확인한다. 알람·전화·다른 앱 음향, Bluetooth 연결/해제, 볼륨 변경, 잠금과 반복 복귀를 검사한다. 게임 상태는 같은데 소리만 복귀하지 않는 경우를 별도 실패로 기록한다.
