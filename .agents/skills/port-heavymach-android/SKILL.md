---
name: port-heavymach-android
description: HeavyMach 1의 검증된 C++ 코어를 Android에 연결하고 일반·치트 APK를 빌드·검증할 때 사용한다. 후속 타이틀 복원이나 hm-defense의 Kotlin 구현에는 적용하지 않는다.
---

# HeavyMach Android 이식

저장소 루트의 AGENTS.md와 docs/canonical.md를 따른다. [개발 계획](../../../docs/android-port-plan-20261026.md), [시간·저장·음향](../../../docs/runtime-policy.md), [렌더](../../../docs/rendering-policy.md), [좌표](../../../docs/coordinate-policy.md), [전체 무기 검사](../../../docs/weapon-validation-procedure.md)를 읽는다. `python3 tools/document_contract.py plan --task android`로 현재 연결을 확인한다.

## 작업 범위

- 계획의 A0~A5 중 요청된 단계와 실제 파일·산출물 상태부터 확인한다. 문서의 예정 경로·Gradle task를 이미 구현된 것으로 취급하지 않는다.
- 기존 Apple 소스·프로젝트·공용 코어·자원·vendor·Apple 빌드 스크립트를 이동·수정하지 않는다. 새 구현·Gradle/CMake·검사·로컬 ignore는 Android/ 아래에 둔다. Sources/Core와 자원은 기존 파일을 읽어 사용한다. HMGameView/HMMenus/HMGameHUD 동작은 Android 전용 코드로 이식하고 Apple 원본은 유지한다. Kotlin은 OS 연결, C++는 코어와 가까운 실행·메뉴·HUD를 맡는다. 게임 규칙을 별도 언어로 다시 쓰지 않는다.
- 먼저 GLES 1.1/EGL와 기존 OpenAL Soft를 검사한다. 확장·driver 실패가 있으면 근거를 남기고 대안을 비교한다. 렌더 API·필터·알파·좌표를 편의상 바꾸지 않는다.
- 한 논리 프레임 한 tick, 원래 시간 증가와 50Hz 센서, 지정된 저장 시점을 보존한다. 고주사율 표시·backlog·background를 이유로 추가 tick/파일 쓰기를 넣지 않는다.
- 컨텍스트 보존 옵션을 복구 구현의 대체로 삼지 않는다. 화면 Surface만 소멸한 경우와 EGL context 소실을 구분하고 GPU 자원만 재생성한다. 세션·난수·게임 시간은 프로세스 생존 시 유지한다. AssetManager/stb 뒤의 기존 크롭·축소·알파·색 처리도 대조한다. OpenAL 링크 성공과 실제 audio focus·장치 복귀 성공을 구분한다.
- Surface/context 소실, Activity 재생성, 프로세스 종료를 분리한다. 일시정지·입력 해제·자원 복구·명시 재개를 검사한다. 센서 m/s²→g·기기 기본 방향·display rotation을 검증한다.

## 실행과 판정

1. develop에서 작업 브랜치를 만들고 버전·원본·저장 해시를 기록한다. v0.9와 hm-defense의 로컬 작업을 보존한다.
2. 계획의 단계별 산출물을 만든다. 보호 파일의 작업 전후 해시와 Git 변경 목록을 검사한다. Android 동작을 같은 입력·난수·프레임 수의 기존 Apple 동작과 대조한다. 기존 코드를 추출·공통화하는 작업을 선행하지 않는다.
3. 치트판의 같은 입력 경로로 전체 무기·필살기·이동·성장·진행·저장·넓은 화면을 대조한다. 일반판에는 치트 코드·별도 저장을 섞지 않는다.
4. APK 서명·패키지·ABI·16KB 정렬·SHA-256을 확인한다. 설치 전 기기 serial과 대상 패키지·저장 보존을 확인한다. 실기기 성능은 실제 trace로 판정한다.
5. 실패의 입력·소스·도구·로그를 보존하고 수정 범위를 좁힌다. 빌드·서명·설치·플레이·배포를 구분해 docs/current-status.md와 계획의 진행 상태를 갱신한다.

원본 해석이 필요할 때만 REA를 사용한다. 기존 iOS 성공을 Android 성공으로, 코어 정적 라이브러리를 APK로 보고하지 않는다. 조사 요청을 임의의 기기 설치·앱 배포로 확대하지 않는다.
