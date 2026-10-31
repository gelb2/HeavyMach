# HeavyMach 정본과 작업 기준

## Android 추가 시 기존 Apple 파일 보존 — 2026-10-30

Android 개발 때문에 기존 iOS·macOS 프로젝트·소스·공용 코어·에셋·vendor·Apple 빌드 도구를 이동하거나 수정하지 않는다. Android 전용 구현과 빌드·검사는 `Android/`에 추가한다. 기존 코어와 자원은 읽어 사용한다. Apple 선행 공통화와 Shared·Apple 폴더 이동 제안은 철회했다. [Android 계획](android-port-plan-20261026.md), [경로·검사 규칙](repository-management.md#플랫폼별-폴더와-공용-모듈--2026-10-30).


## Apple 플랫폼 확장 — 2026-10-26

2026-10-29 지원 범위: Mac 앱과 iOS 시뮬레이터는 Apple Silicon arm64만 지원한다. Intel Mac은 지원하지 않는다.

Apple 플랫폼 공용 모듈·native Mac·키보드 요구는 2026-10-26 사용자 지시를 따른다. [구조](apple-platforms.md)와 [키보드 정책](keyboard-input-policy.md)을 적용한다. Catalyst는 사용하지 않는다. iPad는 터치·기울기·키보드를 함께 지원한다.

현행 기준: 2026-10-25. 적용 범위는 이 독립 저장소다. Unity는 별도 저장소이며 IPA 호환 실행기는 `Compatibility/`에 보존한다.

## 읽는 순서와 우선순위

1. 최신 사용자 지시와 사용자 제공 원본을 먼저 확인한다.
2. 이 문서, [문서 위계](document-governance.md), [게임 기획](game-design.md), [복원 계약](restoration-contract.md), [현재 상태](current-status.md), [설계](design.md)를 읽는다.
3. [업무 연결표](document-map.json)로 업무/파일에 맞는 필수 문서·소스·검사를 고른다. 수정할 경로의 [좌표·입력](coordinate-policy.md), [그래픽](rendering-policy.md), [게임 수치](game-calculation-policy.md), [시간·저장·음향](runtime-policy.md)을 읽고 실제 소비 코드와 대조한다.
4. 사용자 후속 요구는 [사용성 TODO](usability-todo.md)와 [전체 캔버스 계획](wide-canvas-plan.md)에서 확인한다. 현행 정책과 변경 예정 정책을 구분한다.
5. 과거 인계·실험 보고는 증거다. 과거의 UIKit/AVAudioPlayer/CADisplayLink 구현 설명을 현행 정책으로 사용하지 않는다.

제품 요구는 사용자 지시와 원본 동작이 소유한다. 이 정책들은 확인한 요구를 코드에 연결한다. 코드가 정책과 다르면 어느 쪽이 원본과 일치하는지 조사한다. 정책을 현재 버그에 맞춰 바꾸거나 코드에 적혀 있다는 이유만으로 정상 판정하지 않는다.

## 요구와 구현의 일치

문서·소스·앱 기능은 서로를 설명해야 한다. 기획은 플레이어 경험과 승인 범위, 설계·정책은 구현 계약, 코드는 소비 경로, 검사·앱 관측은 실행 근거를 소유한다. [위계와 갱신 절차](document-governance.md)를 적용한다. 단순 링크 존재나 검사 개수로 의미 일치를 대신하지 않는다.

현행 문서 목록은 document-map의 current_documents다. 나머지 조사·과거 구현·회고는 증거로 읽는다. 과거 기록에 “최신”이라고 쓰여 있어도 current-status의 후속 소스·기기·날짜 확인을 대신하지 않는다.

## 비교 대상

- 원작: HeavyMach 1.13 ARMv6 IPA. 식별 SHA와 원본 위치는 [출처](source-provenance.md).
- 정상 비교 실행본: `Compatibility/compat-source/fresh-port/20-touchHLE-src/touchHLE` 및 보존된 앱. 사용자가 같은 아이폰에서 프레임 드랍 없이 동작했다고 확인했다.
- 복원 제품: `Sources/Core/` C++20, `Sources/iOS/` Objective-C++. 원본 개발 소스를 확보한 것으로 표현하지 않는다.
- 일반 앱: `HeavyMachRestored`, `com.gelb2.heavymach.restored`, 표시 이름 Heavy Mach 1, `HM_VALIDATION=0`.
- 치트 검증 앱: `HeavyMachValidation`, `com.gelb2.heavymach.validation`, `HM_VALIDATION=1`. 저장·조건 설정·방향 버튼을 일반 앱과 분리한다.

## 변경 규칙

원본 근거 없이 렌더 API·좌표계·타이머·수식·저장 시점을 별도 설계로 바꾸지 않는다. 현대 iOS에서 필요한 변환은 플랫폼 경계에 한정하며, 원본이 고정한 조건과 새 앱이 허용하는 조건의 차이를 적는다. 예: 원본의 고정 가로 방향과 새 앱의 양방향 가로 회전은 같은 조건이 아니다.

변경 기록에는 요구, 원본 함수/파일/주소, 현재 소비자, 재현 입력, 변경, 검사 결과와 남은 범위를 남긴다. 틸트는 버튼 대체 입력만으로 통과시키지 않고 양쪽 화면 방향의 실제 센서 축 입력을 검사한다. 저장은 실제 임시 파일의 쓰기 횟수와 기존 저장 보존을 검사한다. 렌더는 논리 버퍼·최종 표시·실기기 계측을 구별한다.

새 실패를 숨기거나 기준을 낮춰 통과시키지 않는다. 유효한 기존 검사 결과는 재사용한다. 정책 변경 시 관련 구현·검사·상태 문서를 같은 변경에 포함한다. 일반 앱과 검증 앱의 공통 경로 수정은 두 산출물을 모두 빌드하며 실제 설치 여부를 각각 기록한다.

## 저장소와 증거 보존

코드·테스트·도구·문서·Xcode 구성·vendor와 라이선스, 앱 빌드·검사에 필요한 자원을 Git/LFS로 관리한다. `Resources/OriginalImages`, `Images`, `Audio`, `RawData`, `Assets.xcassets`, `Validation`과 `Art/Extensions`는 `docs/build-resources.json`에 파일별 크기·SHA-256을 기록한다. 새 clone은 `git lfs pull` 뒤 `python3 tools/verify_build_resources.py`로 확인한다. IPA나 별도 자원 복사는 필요하지 않다. 원본 IPA, 로컬 REA 출력 `Analysis/`, 자원 분석 기록 `Resources/Data`·`Evidence`, `build/`, 사용자 저장, 인증서·프로파일·키는 커밋하지 않고 로컬에 보존한다.

최신 결과는 current-status가 소유한다. 시뮬레이터 통과를 실제 폰 통과로, 치트 클리어를 정상 난이도 통과로, 일부 장면 일치를 전체 원본 동등성으로 바꾸어 보고하지 않는다.

- [REA 분석·복원 방법과 전체 증거 색인](public/rea-restoration-method-20261025.md): 사용한 스킬·도구, 25개 함수군, 실패 수정, 구현·검증 기록.
- [IPA 포팅과 네이티브 앱 복원](public/ipa-port-and-native-app-20261026.md): 두 판본의 제작·비교·수정과 최종 설치를 정리한 기술 블로그 원고. 현행 정책은 위 문서들을 따른다.

- [저장소 관리·v0.9 복원](repository-management.md): develop→main 병합, 태그 고정, 별도 worktree와 저장 보존.

## Android 후속 계획

2026-10-26 요청으로 [Android APK 개발 계획](android-port-plan-20261026.md)과 [후속 타이틀 조사](related-titles-20261026.md)를 추가했다. 2026-10-30 `feat/android-app`에서 일반·치트 APK를 구현하고 Galaxy USB·무선 설치와 실행을 검사했다. [현재 구현·검사 범위](android-implementation-20261030.md). 관련 업무는 document-map의 android, port-heavymach-android와 debug-heavymach-android 스킬을 읽는다. Android 구현과 공개 개발·플레이 문서는 2026-10-30 `feat/android-app`→`develop`→`main`으로 병합했다(main `9a63578f`). v0.17은 Android 일반·치트 앱과 개발·플레이 자료를 포함한다. [변경 이력](../CHANGELOG.md#v017).

## 개발자의 변과 프로젝트 소개

[개발 동기 자료](developer-background.md)에 개인 경험 원문·MachDefense 복원 배경·Unity 착수 기록·SaaS 글을 연결했다. 개발자의 변·README·Apps·Mags 작성 시 이 자료와 [RESTORATION](RESTORATION.md)을 읽는다. 개인 동기를 기술 학습·AI 실험 목적만으로 요약하지 않는다.

## 쉴카·퉁구스카 — 2026-10-28

신규 지상 적 작업은 [조립·무장·배치](ground-aa-expansion.md)를 읽는다. 쉴카는 흰색 총탄 6발, 퉁구스카는 흰색 총탄 4발과 미사일 2발을 연사한다. 레이더는 포신과 독립적으로 회전한다. 원작 배치를 유지하고 35개 스테이지에 두 종류 합계 152대를 추가했다. 최종 파츠와 포탑 수정까지 v0.13에 포함한다.

## 웹(WebAssembly) 빌드 — 2026-10-28

fastutlego가 `porting/wasm`에서 웹 빌드를 작성해 PR #24로 기여했다. `Sources/Core`를 그대로 소비하고 iOS 플랫폼 계층의 번역인 `Sources/Web`, 빌드 구성 `Web/`을 사용한다. 그래픽은 WebGL2에서 GLES 1.1 상태를 재현하고 음향은 Emscripten OpenAL, 저장은 IndexedDB의 HvM.dat다. 일반 앱만 대상이며 치트 웹 빌드는 없다. 관련 업무는 document-map의 web을 읽는다. [이식 문서](web-port-20261028.md).


## Tauri·웹 확장 계획 — 2026-10-31

Tauri·순수 웹 개발은 기여자 웹 구현과 후속 수정 3건까지 고정한다. `Web/`·`Sources/Web/`·기존 Apple·Android·공용 코어·자원·도구는 수정하지 않는다. 새 코드는 `Desktop/Tauri/`·`Distribution/Web/`에 둔다. 저장 데이터 보존과 Mac·Windows 지원을 목적으로, 기존 웹 저장을 유지하고 별도 실행기에 OS 파일 백업·복구를 추가한다. Mac 비교 → 순수 웹·PWA 배포 → Windows 설치본 빌드 순서로 진행한다. [개발 계획](desktop-web-plan-20261031.md). `feat/tauri-mac`의 첫 Mac 실행기·OS 백업·시작 전 복구를 구현했다. 실제 슬롯 복구와 JS/Rust 20개 검사를 통과했다. 네이티브 Mac 비교·배포와 PWA·Windows는 남아 있다. [실행 기록](evidence/tauri-mac-start-20261031.json).
