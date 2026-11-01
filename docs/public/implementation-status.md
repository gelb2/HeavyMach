# 구현 상태

<!-- language-nav:start -->
[한국어](implementation-status.md) | [English](../en/public/implementation-status.md)
<!-- language-nav:end -->

2026년 11월 2일 main 기준이다. 최신 정식 릴리스 태그 **v0.17**은 Apple·Android 앱을 포함한다. 이후 main에는 웹/PWA 공개, Tauri Mac 앱, 공용 복원 코드와 게임 버튼 교체가 추가됐다. v0.18 태그는 아직 없다.

## 플랫폼별 상태

| 플랫폼 | 구현·확인한 동작 | 배포와 남은 일 |
|---|---|---|
| iPhone·iPad | 일반·치트 앱, 전투·HUD 편집·복원 코드, 실제 기기 복원·재시작 | Xcode 소스 빌드. 장시간 발열·일부 표시시간 검사 남음 |
| 네이티브 Mac | AppKit 일반·치트 앱, 키보드 전투·복원 코드·종료 | Xcode 소스 빌드. 11월 2일 프레임 처리시간 기준 실패 |
| Android | 일반·치트 APK, Galaxy 유선·무선 설치, 전투·복원·재시작 | Gradle/Android Studio 소스 빌드. 배포 서명·태블릿·장시간 검사 남음 |
| 웹/PWA | 공개 주소 플레이·설치·오프라인 재시작·복원 코드 | [GitHub Pages](https://gelb2.github.io/HeavyMach/) 공개. 실제 장기 미접속 보존 검사 남음 |
| Tauri Mac | 로컬 앱 빌드, 전투·OS 백업·자기 코드 복원·재시작·종료 | [Tauri Mac 다운로드](https://github.com/gelb2/HeavyMach/releases/tag/tauri-mac-20261102) 제공. ad-hoc 서명·공증 미완료 |
| Tauri Windows | 개발 계획 | Windows 빌드·실행·배포 미완료 |

[설치와 실행](build-and-install.md) · [플랫폼별 검사](validation.md)

## 구현된 기능

| 항목 | 현재 동작 | 확인 범위·근거 |
|---|---|---|
| 전투·성장·진행 | 이동·점프·무기·필살기·적·보스·경험치·강화·저장 | 직접 한 전체 미션 플레이, 코어·앱 검사 |
| 그래픽·음향·시간 | OpenGL/WebGL2, OpenAL 재사용, 플랫폼별 표시 처리, 지정 시점 저장 | 계산·자원·저장·음향 검사. 일부 표시시간 실패 잔여 |
| 넓은 화면 | 기기 비율별 월드 표시, 확장 영역 터치, 결과 배경 | iPhone·iPad·Mac 화면·입력 검사 |
| 메뉴·도어 | 중앙 에셋과 좌우 배경 연결, 전체 화면 도어 | 화면 비율·전환·효과음 검사 |
| 가로 회전·생명주기 | 두 가로 방향, 잠금·백그라운드·120초 무입력 정지, 직접 재개 | 실기기 검사와 내 확인 |
| Apple 모듈 | C++ 코어·Apple 런타임·자원 bundle 공유, AppKit Mac | 3플랫폼 빌드·실행 |
| 키보드 | 전투·메뉴 전체 조작, Shift 미세 조절, 리매핑, iPad 터치 병용 | API·OS 키 입력, iPad Magic Keyboard 이동·점프·트랙패드 사격 확인. 재연결 검사 잔여 |
| 자동조준 | ON/OFF·표적 전환·포신 조절·HUD 표시, 기존 탄환 규칙 | v0.11, C 삭제 뒤 수동 사격 25,200프레임 일치 |
| VTOL 3종 | 14개 서브미션에 113대 추가, 기존 배치 보존 | v0.10, 이동·무기·충돌·실기기 확인 |
| 전투기 3종 | 14회·72대 증원, 금색 유리, 묶음 미사일, 화면 밖 진입 | v0.12, 코어·3플랫폼 검사와 내 확인 |
| 지상 적 2종 | 쉴카·퉁구스카, 35개 스테이지 152대, 독립 회전 레이더, 총탄 6/4발·미사일 2발 | v0.13, [최종 그림·검사](ground-aa-art-20261028.md#두-포탑-높이-맞춤) |
| HUD 편집 | 버튼·계기판·장식 이동, 미리보기, 취소·적용·기본 복원 | v0.14 구현·실제 iPhone/iPad 각13개 검사, v0.15 진입 변경 검사 |
| 앱 아이콘 | 글자 없는 Apple·Android 앱 아이콘 | v0.15 Apple 4종 빌드 통과. [그림·자료](app-icon-and-media-20261029.md) |
| 치트 앱 | 미션·무기·강화 조건 지정, 일반 앱과 저장 분리 | iPhone·iPad·Mac·Android |
| 무선 개발 | iPhone·iPad 설치·실행·LLDB | 반복 재부팅·iPhone 전체 계측 잔여 |

[기능 상세](features.md) · [검사 수치·조건](validation.md)

## 공용 복원 코드와 게임 버튼

세이브 슬롯 화면에서 코드를 생성하거나 입력해 진행을 다른 기기로 옮긴다. 세 슬롯, 경험치·미션·무기 순서·탄약·강화 상태, HUD 배치와 키·조준 설정을 저장한다. Apple은 전체 슬롯, Android·웹판은 전체 또는 선택 슬롯을 복원한다. 공용 모듈이 코드 검사·기존 저장 백업·복원·재읽기 비교를 담당한다. [모듈과 연결부](../../Modules/SaveTransfer/README.md).

HUD 편집·복원·키 설정의 추가 버튼을 원작의 그림·눌림·입력 처리로 교체했다. 코드 입력과 클립보드·파일 선택은 OS와 브라우저에서 처리한다. 원작 메뉴와 SET GAME UI는 유지했다. iPhone·iPad·Galaxy·웹/PWA·Tauri에서 버튼과 복원 후 재시작을 확인했다. [검사 결과](validation.md#게임-버튼과-복원--2026-11-02).

## 남은 기능과 연구

| 과제 | 현재 단계 | 구현·조사할 내용 |
|---|---|---|
| 추가 적 검증 | 전체 캠페인·장시간 검사 남음 | 새 지상·공중 적의 난이도·발열·메모리 추적 |
| 빌드 경고 | 전수조사·수정 예정 | deprecated API 관련으로 추정. 플랫폼·일반/치트·빌드 설정별 로그를 모아 원인을 확인하고 추가 기능 개발과 병행해 수정 |
| 빌드·실행 스크립트 | 사례 조사 완료, 구현 예정 | 자원 다운로드·검사, 의존 라이브러리·앱 빌드, Mac 실행·iPhone/iPad 설치와 서명 설정 안내를 통합 |
| 자동조준 상태 안내 | [별도 표시 계획](auto-aim-visibility-plan-20261028.md), 미구현 | 표적 없음·화면 밖·지형 아래 상태를 시각적으로 전달. 조준 알고리즘은 유지 |
| 튜토리얼 | 연구 대기 | 기존 미션 유지, 최상단 패키지 0-0~0-4, 틸트·무기·전환·자동조준 안내, 원작 UI·그림·클리어 규칙에 맞춤 |
| Android | 일반·치트 앱 구현, Galaxy 유선·무선 설치·플레이 확인 | 전체 캠페인 클리어·장시간 발열·태블릿 실기기·배포 서명 |
| 접힌 화면 | 구상 기록 | 위 게임 화면·아래 조작부의 배치와 입력 연구 |
| 적 AI·고정 좌표 | 후속 예방 조사 | 기체별 이동·활성 범위가 화면 폭 변화에 영향을 받는 지점 점검 |


검증 잔여는 Mac을 포함한 표시시간·장시간 발열/메모리·키보드 재연결·추가 적 난이도·무선 반복 검사다. 나는 설치된 iPhone·iPad 앱에서 이상 동작을 보지 못했다. Apple 앱 4종 CI 빌드는 통과했다. [남은 과제 전체](ground-units-research-20261028.md). [작업 기록과 상세 요구](../usability-todo.md)

## 배포와 버전

웹/PWA는 현재 main에서 빌드해 [GitHub Pages](https://gelb2.github.io/HeavyMach/)에 게시했다. Tauri Mac은 [소스에서 빌드](../../Desktop/Tauri/README.md)할 수 있으며 [Tauri Mac 다운로드](https://github.com/gelb2/HeavyMach/releases/tag/tauri-mac-20261102) ZIP도 제공한다. 공증은 남아 있다.

저장소는 2026-10-28 공개했다. 통합 보고 사이트는 소유자 로그인으로 열람한다. Git 태그는 v0.9~v0.17이며, v0.17 Releases에서 소스·자원 압축본을 받을 수 있다. iPhone·iPad·Mac 앱은 Xcode에서 빌드해 설치한다. 새 clone은 Git/LFS 자원으로 빌드한다. v0.9는 자원 등록 이전 태그이므로 당시 코드를 빌드할 때 자원 준비가 추가로 필요하다. [설치](build-and-install.md) · [버전 복원](../repository-management.md)

## Android 구현 · 2026-10-30

일반·치트 APK 구현, Galaxy 유선·무선 설치, 공용 코어와 전용 호스트, 검사와 수정 내역은 [Android 개발 과정](android-development-20261030.md)에 정리했다. `feat/android-app`에서 구현하고 develop을 거쳐 main에 병합했다. Android 최초 구현은 기존 Apple 소스를 유지했다. 이후 공용 복원 코드·게임 버튼 연결부는 main에서 갱신했다. v0.17은 이 후속 변경 전의 Android 버전이다. [Android 설치](build-and-install.md#android-빌드와-설치), [Galaxy 플레이 캡처와 영상](android-gameplay-20261030.md).
