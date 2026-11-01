# HeavyMach Android

기존 C++ 코어와 자원을 읽어 Android 앱을 빌드한다. Kotlin은 Activity·입력·센서·오디오 포커스와 복원 코드 텍스트 입력·클립보드를 담당한다. Android C++ 호스트가 메뉴·HUD·EGL/GLES·OpenAL을 연결한다.

`feat/android-app`에서 구현하고 develop을 거쳐 main에 병합했다. 처음 빌드할 때는 [소스·자원·SDK 준비와 설치 안내](../docs/public/build-and-install.md#android-빌드와-설치)를 따른다. Galaxy SM-S731N에 일반·치트 앱을 USB와 Wi-Fi로 설치했다. [구현·검사 결과](../docs/android-implementation-20261030.md).

## 폴더

- `app/src/main/java/`: Android 화면·생명주기·입력과 JNI 호출.
- `app/src/main/cpp/`: Android 호스트·메뉴·HUD·렌더·음향.
- `cmake/`: 기존 Core와 OpenAL을 읽는 NDK 빌드.
- `tools/`: 자원 생성·Apple 파일 보존 검사·기기 실행.
- `app/build/`, `build/`: 무시되는 APK·캐시·검사 증거.

## 빌드

JDK 17, Android SDK 36, CMake 3.22.1, NDK 26.3.11579264를 사용한다.

```sh
cd Android
./gradlew :app:assembleNormalDebug :app:assembleCheatDebug
```

일반 앱: `app/build/outputs/apk/normal/debug/app-normal-debug.apk`
치트 앱: `app/build/outputs/apk/cheat/debug/app-cheat-debug.apk`

`tools/prepare_assets.py`가 기존 자원의 해시와 Git LFS 실파일을 확인하고 빌드 폴더로 복사한다. 기존 자원을 다시 그리거나 변경하지 않는다. APK는 `AssetManager`로 자원을 읽으며 외부 패키지를 요구하지 않는다.

## 설치·실행

```sh
python3 tools/device.py devices
python3 tools/device.py --serial SERIAL diagnose
python3 tools/device.py --serial SERIAL install app/build/outputs/apk/cheat/debug/app-cheat-debug.apk
python3 tools/device.py --serial SERIAL launch --edition cheat
python3 tools/device.py --serial SERIAL capture --edition cheat
python3 tools/device.py --serial SERIAL logs --edition cheat
python3 tools/device.py --serial SERIAL finish --edition cheat
```

휴대폰에서 USB 디버깅을 켜고 Mac의 RSA 키를 허용한다. Android 11 이상은 개발자 옵션의 무선 디버깅 화면에서 페어링 포트·코드를 확인한다.

```sh
python3 tools/device.py pair IP:PAIRING_PORT
python3 tools/device.py connect IP:DEBUG_PORT
```

USB로 승인된 기기를 같은 Wi-Fi에서 연결하려면 `--serial USB_SERIAL wireless-usb`를 실행한다. 이후 출력된 무선 serial을 사용한다. 케이블을 뺀 상태에서 설치·실행을 다시 확인한다. [Android 공식 ADB 절차](https://developer.android.com/tools/adb#wireless).

## 조작과 검사

- 원래 조작: 기울여 이동, 화면을 눌러 조준·사격, HUD에서 점프·무기 전환.
- 자동조준: 조준경으로 켜고 끄기, 화살표로 표적 변경, 사격 버튼을 누르는 동안 발사.
- 키보드: A/D 이동, 방향키 조준, F 발사, Space 점프, Q/E 무기, R 표적, Shift 미세 조작, Esc 멈춤, F2 키 설정.
- 게임 실행 후 HUD 편집 도구·복원·키 설정·치트 패널의 버튼은 원작 QobButton 상태와 게임 스프라이트 렌더러를 사용한다. 텍스트 입력 필드와 클립보드는 Android에 연결한다.
- HUD 편집: 멈춤 메뉴의 SET GAME UI. 게임 화면 위에서 HUD를 이동하고 APPLY로 저장한다. RESET LAYOUT, CANCEL, PREVIEW, SAMPLE SCENE을 제공한다.
- 슬롯 화면의 F3은 복원 코드 화면을 연다. F6은 추가 게임 버튼의 키보드 포커스를 켜고 끈다. 도구 포커스에서는 Tab으로 선택하고 Enter/Space로 누른다. Esc로 포커스를 빠져나온다. HUD 편집의 기본 Tab·방향키 조작은 유지한다.
- 치트 앱: TEST에서 미션·스테이지·레벨·무기를 선택한다. 이동·점프·사격 버튼도 제공한다.

확인할 항목: 두 APK 빌드·서명·16KB 정렬, 전체 무기, 저장 시점, 이동·점프·터치 좌표, 자동조준, HUD 적용·취소·초기화, 가로 양방향, Surface 재생성, 백그라운드·잠금·알람 후 재개와 음향, 프레임·메모리·발열, USB·무선 설치.

## 재현 검사

```sh
python3 tools/validate_runtime.py --serial SERIAL --weapons
python3 tools/validate_platform.py --serial SERIAL --hud-only
python3 tools/validate_campaign.py --serial SERIAL
python3 tools/measure_combat.py --serial SERIAL
python3 tools/verify_apks.py
python3 tools/validate_game_buttons.py --serial SERIAL
python3 tools/validate_modal_keys.py --serial SERIAL --out build/evidence/modal-keys
python3 tools/validate_modal_touches.py --serial SERIAL --out build/evidence/modal-touches
python3 tools/validate_normal_transfer_ui.py --serial SERIAL --out build/evidence/normal-transfer
```

`validate_game_buttons.py`는 치트 앱에서 실제 터치·키 입력으로 추가 버튼을 검사한다. `validate_normal_transfer_ui.py`는 저장된 슬롯이 있는 일반 앱에서 복원 코드를 만들고 같은 데이터를 복원한다. 두 검사는 실행 전 앱 파일을 백업하고 검사 후 원래 파일을 복구한다.

플랫폼 검사에서 `--hud-only`를 빼면 전용 에뮬레이터의 화면 크기와 회전을 바꿨다가 복원한다. HUD 좌표 검사는 2340×1080 기준이다. 센서 방향 검사는 전용 에뮬레이터에서 `validate_tilt.py --serial emulator-5554`로 실행한다. Clock 알람 검사는 전용 에뮬레이터에서 `validate_audio_timer.py`로 실행한다. 세부 절차는 [Android 기기 실행 스킬](../.agents/skills/debug-heavymach-android/SKILL.md)에 있다.

현재 APK는 게임 코드를 최적화한 debug 서명 빌드다. Galaxy에서 20종 무기와 HUD·캠페인 진입·자동조준을 검사했다. 전체 캠페인 클리어, 장시간 발열·메모리, Android 태블릿 실기기와 배포 서명은 후속 검사에 남아 있다.
