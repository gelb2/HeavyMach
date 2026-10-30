# HeavyMach 개발 보고

최신 릴리스 태그는 **v0.17**이다. Android 일반·치트 앱을 포함한다. 완성분을 develop→main 순서로 병합하고 main에 태그를 붙였다. 자동조준·추가 항공기·쉴카·퉁구스카와 HUD 위치 편집을 포함한다. [GitHub 본문](github.html)에 플레이 이미지8장·GIF2개·MP4 3개를 배치했다.

Codex가 일반 앱과 치트 앱을 아이폰·아이패드에 설치했다. 내가 최신 설치본을 직접 플레이했을 때 프레임 드랍이나 오작동은 보이지 않았다. 과거 계측 수치와 당시 실패는 각 보고서에 남아 있다.

## 전체 보고서 {#reports}

[신(新) 리센코주의와 정렬세 — Android 개발 경험과 가설 전문](ai-project-experience.html)

- [GitHub 국문·영문 문서](document-localization.html) — 문서20쌍 작성, 언어 선택과 원문·명령·그림·링크 검사.

- [럭스루나에 편대 밀집 조사·수정 계획](lux-formation.html) — 생성점·목적지·높이 분산과 코어 비교.

- [HUD 배치 편집 설계](hud-improvement.html) — HUD16개 자원·버튼별 할당·분할선·실제 GPU 재조립과 편집 계획.
- [자동조준 상태 안내 계획](auto-aim-visibility.html) — HUD 편집과 분리한 대상 없음·화면 밖·가림 안내.

- [쉴카·퉁구스카 도안과 게임 화면](ground-aa-art.html) — 파츠 조립, 독립 회전 레이더, 6/4/2발 무장, 실기기 검사.
- [쉴카·퉁구스카 구현·전체 배치표](ground-aa-implementation.html) — 35개 스테이지 152대, 원작 이동·무기 재사용과 검사.
- [왜 럭스루나에인가](lux-lunae-intent.html) — 개인 경험·기획 의도, 승인 도안 3종과 실제 전투 화면·영상.

- [남은 TODO와 쉴카·퉁구스카 개발 연구](ground-units.html) — 원작 파츠·무기 재사용, 이동 중 덜컹거림, 단독 구현·검증 범위.

- [GitHub 구성과 v0.12 소스 릴리스](github-followup.html): 도입문·기여 안내·자동 검사·자원 포함 소스 빌드.

### GitHub 원고

- [README와 플레이 화면](github.html) — 소개·기능·설치·상태와 이미지8장·GIF2개.
- [전체 문서 목차](docs-index.html) — 독자용 문서·개발 정책·증거.
- [기능과 추가 기획](features.html) — 자동조준·VTOL·전투기의 의도·구현·수정·검사.
- [조작 안내](gameplay-guide.html) — 터치·틸트·키보드·자동조준.
- [코드 구조](architecture.html) — workspace·공용 모듈·자원·플랫폼.
- [트러블슈팅17개](troubleshooting.html) — 내 지적, 원인·변경·결과.
- [개발 과정](development-history.html) — 비교 순서와 사용 도구·스킬.
- [검증 결과](validation.html) — 날짜·기기·조건별 성공과 남은 검사.
- [변경 이력](changelog.html) — v0.9~v0.15.
- [제작자·자원](credits.html) — 원작·추가 작업·라이브러리.


### 플레이 화면·이미지

- [글자 없는 앱 아이콘과 최신 기능 이미지](icon-and-media.html) — 아이콘 변경, 쉴카·퉁구스카 사격과 HUD 편집 화면, SaaS 집필 자료.

- [플레이 영상에서 고른 이미지8장·클립3개](gameplay-media.html) — 이미지 8장 전체 배치, GIF2개·MP4 3개, GitHub 운영 절차와 스킬 조사.
- [전체 화면·도어 전환 수정](fullscreen.html) — 게임 진입 전 화면 전환.
- [야크·해리어·F35B](vtol-art.html) — 시안, 무장·미션 배치, 크기·외곽선·불꽃 수정과 설치.
- [럭스루나에 전투기3종](lux-lunae.html) — F-22·Su-57·MiG-31, 묶음 연사, 화면 밖 진입과 설치.

### 조작·플랫폼·검사

- [자동조준 구현·검사](auto-aim-progress.html) — 버튼, 표적 표시, 정식 앱 적용과 C 보정 삭제.
- [자동조준 요구·계획](auto-aim.html) — 내 요구와 설계 이력.
- [Apple 플랫폼·키보드 검사](apple-platforms.html) — iPhone·iPad·Mac, 실기기·Simulator, 무선 설치.
- [iOS 코드·문서 전수검사](ios-audit.html) — 누락·보존·문서 연결 검사.

### 설치·구현 범위

- [빌드와 설치](installation.html) — 소스·자원 받기와 실행 순서.
- [구현과 검증 범위](implementation.html) — 제공 기능과 실제 확인한 범위.
- [새 clone 빌드](clone-build.html) — 원격에서 받은 소스와 자원만으로 검사.
- [IPA 없는 빌드](independence.html) — 원본 IPA 접근을 차단한 빌드·실행 검사.

### 개발 배경·복원 과정

- [개발자의 변](restoration.html) — HeavyMach를 선택하고 복원한 이유.
- [IPA 포팅과 네이티브 복원](port-and-native.html) — 두 판본의 구현·비교 순서.
- [기술 회고](retrospective.html) — 문제, 수정과 검사 결과.
- [REA 활용 내역](rea.html) — 분석 도구·함수·근거.
- [REA 분석과 구현 대조](rea-implementation.html) — 분석 기록과 실제 코드의 연결.

### 문서·Sass·후속 계획

- [GitHub 구성 점검·보완 계획](github-repository-plan.html)
- [코드 라이선스 적용 범위](license-scope.html)
- [자원 이용 조건](asset-licenses.html)
- [외부 코드 고지](third-party-notices.html)

- [GitHub 공개 라이선스·README 배지](github-licenses-and-badges.html) — 코드·자원·라이브러리의 배포 조건, 필요한 문건, 기술 배지와 배치안.

- [GitHub 문서 작성 계획](github-documentation.html) — 문서별 내용, 트러블슈팅·추가 기능 목록, 완료·진행·남은 과제, 작성 순서.

- [Sass 집필 자료](publication.html) — Apps·Mags 담당 세션의 읽는 순서와 자료 위치.
- [영상·README 작업 절차](media-preparation.html) — 원본 보관·추출·캡션 규칙.
- [문서 체계 검사](documentation.html) — 정본·기획·정책·코드 연결.
- [같은 제작사의 다른 타이틀](related-titles.html) — Heavy Mach 2·Mach Defense 조사.
- [Android 개발 계획](android-plan.html) — 공용 코드와 APK 제작 절차.

## Apple 플랫폼·키보드·새 투두 {#apple-platforms}

야크·해리어·F35B는 기존 적을 유지하면서14개 서브미션에 총113대를 추가했다. 크기·외곽선·후방 불꽃 수정 후 두 기기에 설치했다. [시안부터 설치까지](vtol-art.html).

Codex는 2026-10-27 iPhone 일반 앱을 설치하고 두 기기의 무선 설치·실행·LLDB 중단점 검사를 마쳤다. iPad 무선 자동 검사8개가 통과했고 Instruments CPU 기록도 저장했다. 반복 작업용 스킬과 연결·서명·테스트 묶음 복구 절차를 추가했다. [무선 검증 결과](apple-platforms.html#section-14).

공용 코어·자원 모듈과 iPhone/iPad/native Mac 프로젝트를 구성했다. Catalyst는 사용하지 않는다. 키보드 이동·점프·조준·사격·무기 전환과 iPad 터치 병용 검사를 통과했다. Mac 전체 화면은 macOS 자체 캡처로 화면 끝까지 표시됨을 확인했다. CUA 창 캡처에만 흰 여백이 나타났다. 2026-10-27 iPhone·iPad에서 각각85개 중84개 통과·별도 성능 비교1개 실패를 기록했다. 실제 게임 타이머는 평균 약60회/초였다. 후속 iPhone 메뉴 처리 p95는17.309/17.629ms로16.67ms를 넘었다. 두 기기에서 회전 뒤 터치 사격·홈 복귀 정지를 확인했고 짧은 Instruments 기록의 누수는0건이다. [기기별 결과](apple-platform-progress-20261026.md#section-13).

Mac 정식·치트 앱을 개발 인증서로 빌드한 뒤 메모리 누수0건을 확인했다. 기존 약20KB 누수는 팀 ID가 없는 임시 서명 때문에 시스템 등록이 거절된 뒤 재시도하는 경로에서 발생했다. Apple Intelligence 설정은 바꾸지 않았다.

Mac의 표시 대기·표면 갱신·Retina 설정을 수정했다. 잠금을 푼 상태에서 전체 화면을 포함한33개 검사가 모두 통과했다. 여섯 장면의 처리 p95는10.86~14.29ms, 평균 호출 간격은16.67~16.69ms였다. Codex가 Mac 키 입력 조작과 Instruments Allocations+Leaks 기록도 완료했다. 2026-10-27 macOS 자체 캡처와 대조해 흰 여백을 CUA 캡처 경로의 현상으로 분류했다.

iPad Simulator는44.311~48.133ms, 변경 전 main도39.972~40.580ms로16.67ms 기준을 넘었다. Instruments에서 Simulator의 최종 화면 확대·표시가 주된 비용임을 확인했다. iPad 누수는0건이었다.

자동조준은 정식·치트 앱에 적용했고 main의 v0.12에 포함돼 있다. A로 켜고 끄며 표적 전환·사격 버튼은 하단 HUD에 배치했다. C 보정은 삭제해 원래 탄속·분산을 유지한다. [구현·검사·캡처](auto-aim-progress.html), [요구·계획](auto-aim.html). HUD 배치 편집은 v0.14에 포함됐다. v0.15에서는 미션 선택 버튼을 제거하고 전투 일시정지의 SET GAME UI에서 편집하도록 바꿨다. [HUD 편집](hud-improvement.html), [아이콘·최신 화면](icon-and-media.html).

[구현 범위·검사 결과·추가 요구](apple-platforms.html)


## 새 clone 빌드 {#clone-build}

Git에서 빠졌던 앱 자원1,471개를 추가했다. GitHub에서 새로 받은 소스·LFS 자원만으로 일반·치트 앱의 Simulator·iPhone용 빌드4개가 성공했다. 새 Simulator의 두 앱 실행, 코어17·iOS14 검사도 통과했다. IPA나 기존 Mac의 자원 복사는 필요하지 않다.

[수정과 검사 결과](clone-build.html) · [빌드·설치](installation.html)

## IPA 없는 빌드·실행 검증 {#independence}

원본 IPA와 기존 프로젝트 폴더의 읽기를 차단한 새 복사본에서 일반·치트 앱의 Simulator·iPhone용 빌드 4개가 성공했다. 이미지·음원·맵·게임 데이터의 앱 내 해시가 모두 일치했다. 새 Simulator 실행과 코어17·iOS14 검사도 통과했다. REA 분석 기록과 실제 게임 코드의 연결을 재검사했다.

[검증 결과와 실행 화면](independence.html) · [REA 구현 대조](rea-implementation.html)

## GitHub 문서·SaaS 집필 자료 {#publication}

SaaS 담당 세션이 개발 동기·내가 제공한 원문·복원 과정·검증 결과를 읽고 Apps·Mags를 작성하도록 문서 안내를 정리했다. GitHub용 설치 가이드와 구현·검증 범위도 추가했다. 기존 보고 이미지 13개를 출처와 함께 저장소에 보존했다.

[SaaS 집필용 문서 안내](publication.html) · [빌드·설치](installation.html) · [구현 범위](implementation.html) · [플레이 영상·README 준비](media-preparation.html) · [플레이 이미지·클립 후보](gameplay-media.html)

## 개발자의 변 {#developer-story}

대학생 시절 아이팟터치에서 만난 HeavyMach는 내가 개발자로 직업을 바꾸는 데 영향을 준 게임이다. 2015년 잡지사, 2016년 필리핀 근무를 거쳐 2017년 7월 iOS 개발자로 전업했다. 나는 HeavyMach의 영향을 약 35%로 평가한다. 잡지사 편집디자이너들의 집단 항명과 근무 조건 변화, 공장 자동화 기업 임원들을 취재하며 접한 기술의 편익도 함께 기록했다.

2026년에는 그 게임을 최신 아이폰에서 다시 실행하도록 복원했다. 두 제작자에게 느끼는 고마움, 원작자에게 남긴 문의, 향후 배포에 관한 생각을 이어 적었다.

[개발자의 변 읽기](restoration.html) · [Markdown 원고](RESTORATION.md)

## iOS 코드·문서 전수검사 {#ios-audit}

현재 소스 68개와 앱별 컴파일 입력 29개, iOS 테스트 12개, 헤더 참조 152개에서 누락을 찾지 못했다. 분리 전 파일 595개와 별도 보존 입력 10,292항목도 다시 대조했다. 검사 시작 시 HeavyMach 저장소의 미커밋·미추적 파일은 없었다.

호환판 내부 수정 23개와 rust-sdl2 수정 1개는 커밋된 두 패치로 재구성된다. 소스별 문서 안내를 추가했고 과거 구현 설명과 현재 상태를 구분했다. 당시 미완료였던 단일 문 전환은 [2026-10-27 수정 기록](fullscreen.html)에서 확인할 수 있다. 회전·발열 등의 후속 과제는 유지한다.

[전수검사 전문](ios-audit.html) · [GitHub용 Markdown](ios-completeness-audit-20261026.md) · [현행 소스·문서 연결](https://github.com/gelb2/HeavyMach/blob/main/docs/ios-source-map.md)

## 후속 타이틀·Android APK 계획 {#android-plan}

원래 Archive 모음에 Heavy Mach 2, Heavy Mach 2 HD, Mach Defense, Heavy MACH: Defense가 있다. IPA 19개 목록과 네 대표 파일의 다운로드 응답을 확인했다. 중단한 Defense 포크는 로컬에 Android 개발 브랜치와 기존 debug APK가 남아 있으며 이번 조사에서 보존했다.

Android 일반·치트 APK를 구현하고 Galaxy에서 유선·무선 설치와 실행을 확인했다. 같은 C++ 게임 코어와 자원을 사용하고 화면·입력·음향·저장 연결을 Android 전용으로 구현했다. 첫 보스 이후 업그레이드 창 뒤로 전투가 계속되던 오류도 수정했다. 코드·개발 문서·일반 앱 플레이 자료는 main에 병합됐다. [Android 개발 과정](https://github.com/gelb2/HeavyMach/blob/main/docs/public/android-development-20261030.md) · [설치](https://github.com/gelb2/HeavyMach/blob/main/Android/README.md) · [Galaxy 플레이](https://github.com/gelb2/HeavyMach/blob/main/docs/public/android-gameplay-20261030.md).

[타이틀·Defense 조사 전문](related-titles.html) · [Android 개발 계획 전문](android-plan.html)

## 분리 후 전수검사·REA 실행 확인 {#migration-audit}

분리 전 Git 대상 595개를 전수 대조했고 누락은 없었다. OpenAL 소스 4개가 제외 규칙에 걸린 것은 복구했다. 새 폴더에 없던 REA 분석 자료 3,571개와 호환판 소스·링크 5,119개, 상위 iOS 지침도 복원했다. 끊긴 문서 경로와 Unity용 스킬 배정을 수정했다. 로컬 입력 10,292항목을 원본과 대조했고, REA 증거 부록 6개가 동일하게 재생성됐다.

현재 Codex에서 REA 분석도 실제 실행됐다. 원본 헤비멕의 경험치 함수 `0x106e0`을 Ghidra 12.1.4로 디컴파일하고 어셈블리를 조회했다. 정책 거부나 승인 차단은 발생하지 않았다. 분석 전후 원본 파일의 SHA-256은 같았다.

[전수검사 상세](https://github.com/gelb2/HeavyMach/blob/main/docs/migration/completeness-audit-20261026.md) · [REA 실행 근거](https://github.com/gelb2/HeavyMach/blob/main/docs/rea-availability-20261026.md)

## 독립 저장소와 버전 {#repositories}

HeavyMach를 [GameDev 허브](https://github.com/gelb2/ProjectGameDev) 아래의 [독립 저장소](https://github.com/gelb2/HeavyMach)로 분리했다. Unity는 [별도 저장소](https://github.com/gelb2/ProjectGameDevUnity)에서 관리한다. 허브는 submodule로 두 게임의 커밋을 기록한다.

HeavyMach는 작업 브랜치→develop→main 순서로 병합했다. 정상 플레이가 확인된 소스와 같은 main 병합 커밋 `4e17f5ef`에 [v0.9 태그](https://github.com/gelb2/HeavyMach/tree/v0.9)를 붙였다. 새 경로에서 코어 검사 17개와 arm64 Simulator 일반·치트 빌드가 통과했다. 기존 아이폰 설치본과 저장 데이터는 변경하지 않았다.

2026-10-27에는 VTOL 크기·외곽선·불꽃 수정까지 develop→main으로 병합하고 main의 `76cbd21c`에 [v0.10 태그](https://github.com/gelb2/HeavyMach/tree/v0.10)를 붙였다. 브랜치와 태그의 원격 반영을 확인했다. 이후 도어 수정은 v0.11에 포함됐다.

태그는 고정하고 이후 작업은 develop에서 분기한다. 문제가 생기면 v0.9를 별도 worktree로 꺼내 빌드·서명하고, 현재 저장을 백업한 뒤 재설치한다. GitHub Actions는 계정 사용 한도/결제 사유로 실행되지 못해 로컬 검사와 PR 출발·대상 브랜치를 직접 대조했다.

[구조·보존·검사 기록](https://github.com/gelb2/ProjectGameDev/blob/main/docs/repository-split-20261026.md) · [브랜치와 복원 절차](https://github.com/gelb2/HeavyMach/blob/main/docs/repository-management.md)

## IPA 포팅·네이티브 앱 복원 과정 {#port-and-native}

원본 IPA를 호환 실행기로 구동한 뒤 C++·Objective-C++로 수정 가능한 앱을 만들었다. 새 앱에만 나타난 프레임 저하를 정상 호환판과 비교하면서 렌더링·음향·시간 계산·저장을 고쳤다. 치트·일반 앱을 나눈 이유와 화면 확장 후의 수정, 아이폰 설치와 내 플레이 확인까지 작업 순서대로 설명한다.

[기술 블로그 원고](port-and-native.html) · [GitHub용 Markdown](ipa-port-and-native-app-20261026.md)

## 수정 내역 {#retrospective}

정상 IPA 호환 실행판과 비교해 렌더링·음향·시간 진행·저장·좌표를 고쳤다. 이후 넓은 게임 화면, 자동 일시정지, 검증용 조작과 메뉴 배경 확장을 추가했다. 회고에는 내 지시별 판단 근거, 잘못된 접근, 수정 결과를 정리했다.

두 앱의 설치와 타이틀 실행을 확인했다. 일반 앱에는 치트 버튼이 없고, 설치 직후 저장 파일은 설치 전 백업과 같았다. 당시 미구현이던 단일 문 전환은 [2026-10-27에 수정·Simulator 검사](fullscreen.html)했다. 새 도어는 후속 v0.11 일반·치트 앱에 포함해 iPhone·iPad에 설치했다.

[기술 회고 전문](retrospective.html) · [GitHub용 원고](engineering-retrospective.md)

## REA 분석·복원 기록 {#rea}

REA의 `reverse-engineer-anything` 스킬로 IPA와 실행파일을 조사했다. 함수의 의사 코드·어셈블리·Objective-C 정보를 대조하고, 원본 자원 형식과 호출 순서를 복원 코드에 연결했다. 디컴파일만으로 부족한 부분은 원본 실행 관측과 비교 검사로 보완했다.

원작 함수 목록 1,133개, Assault 함수 목록 1,802개, 보존된 Evidence 1,872개와 JSON 산출물 2,067개를 색인했다. 본문은 25개 함수군의 구현과 실패 수정 과정을 설명한다. 부록에서 주소·해시·분석 결과의 연결을 찾을 수 있다.

[REA 활용 기록과 전체 부록](rea.html) · [GitHub용 원고](rea-restoration-method-20261025.md)

## 문서 관리 {#documentation}

게임 기획, 문서 위계, 업무 8종의 문서·코드·검사 연결표를 추가했다. 초기 UIKit 메뉴 설명과 과거 성능 실패는 당시 기록으로 표시하고 현재 구현으로 연결했다.

기획을 바꾸면 관련 설계·정책·코드·검사도 갱신한다. 구현 오류는 요구를 유지하며 고치고, 문서 오류는 실제 코드와 실행 근거로 바로잡는다. 연결 도구는 경로와 링크를 검사하며, 기능의 의미는 관련 코드와 앱 관측으로 대조한다.

[발견한 문제와 보완 결과](documentation.html) · [게임 기획](game-design.md) · [문서 위계](document-governance.md) · [업무 연결표](document-map.json)

## 메뉴 배경·타이틀 검은줄 수정 {#menu-joins}

모드와 미션 선택에 원본별 좌우 배경을 만들었다. 중앙 그림·글자·버튼은 원본을 쓰고, 철판과 레일의 높이·밝기·질감을 맞췄다.

타이틀의 검은줄은 확대·회전할 때 필터가 텍스처의 검정 여백을 읽어서 생겼다. 배경 경계의 반 texel이 유효한 가장자리 색을 읽도록 고쳤다. 흰 그림의 최소 경계 밝기는 수정 전 153~204에서 수정 후 255가 됐다. 중앙 UV와 원본 480 모드는 유지했다.

{{menu-joins-gallery}}

Simulator와 실제 iPhone Air에서 각각 26개 검사가 통과했다. 타이틀→슬롯→모드→미션→게임 진입, 원본 메뉴, 자원 해제를 검사하고 폰에서 메뉴를 직접 조작했다. 기존 GL 기준 12개도 통과했다. Air·Pro Max 크기 검사는 같은 기기 안의 View 크기를 바꾼 검사다.

수정 후 치트·일반 앱을 모두 설치하고 실행했다. 장시간 성능·발열과 물리 Pro Max 검사는 남아 있다.

[수정 원인·검사·빌드 기록](menu-joins.md)

## 타이틀·슬롯 메뉴 확장 {#menu-result}

첫 확장에서는 imagegen으로 타이틀 후보 2개와 금속 배경을 만들었다. 원본의 왼쪽 광원과 오른쪽 폐허에 가까운 후보 A의 좌우를 채택했다. 중앙 전차·로고·슬롯 카드·버튼·문구는 원본을 사용했다. 원본 이미지 695개는 IPA와 바이트가 같다.

슬롯 카드는 안전영역 안에서 벌리고 삭제·확인 버튼을 양쪽 하단에 뒀다. 표시와 터치 영역에 같은 이동량을 적용했다. 모드·미션의 원본 묶음은 중앙에 두었고, 이후 위 절의 화면별 배경으로 연결을 고쳤다. 치트 방향 버튼은 전투 중에만 표시한다.

{{menu-result-gallery}}

첫 적용 당시 관련 검사 27개와 후속 선택 경로 검사를 통과했다. 20종 무기의 360조건, 결과 그림 45조합, 세 슬롯 선택→삭제 취소→모드·미션→게임 시작을 검사했다. 크기 3종과 양쪽 안전영역을 사용했다. 당시 폰 잠금으로 멈췄던 실행 검사는 잠금 해제 후 진행했으며 최신 설치 결과는 상단에 있다.

당시 Simulator의 타이틀 평균은 27.04ms, 슬롯은 29.53ms, 원본 폭 사격은 20.64ms였다. Apple Software Renderer의 최종 화면 제출 비용이 컸다. 이 결과는 해당 시뮬레이터 기록이며 60Hz 기준을 넘었다.

반복 렌더 중 추가 이미지 디코딩은 0회, GPU 업로드 후 남은 CPU 디코딩 픽셀은 0바이트였다. 생성 텍스처는 포맷상 화면별 8MiB이며 화면을 나가면 해제한다. 프로세스 전체 메모리 실측은 별도다.

[첫 적용의 생성·선택·검사 기록](menu-extension.md)

## 아이폰 조작·무기 검사 {#phone}

iPhone Air(iOS 27.0.1)의 치트 앱에서 전진·후진·점프·사격·무기 전환과 오른쪽 확장 영역의 폭격을 직접 관측했다. 기능 검사 8개가 통과했다.

| 검사 | 결과 | 검사 방법 |
|---|---|---|
| 전체 플레이어 무기 | 20종·360조건·1,080발, 목표·궤적 기준 위반 0 | 폰에서 View 입력과 게임 로직 실행 |
| 결과 그림 | 15개 × 3화면 | 가림·배치·버튼 검사 |
| 입력·상태 | 방향 이동, 가장자리 입력, 전환음 호출, 비활성·복귀·무입력 정지 | 자동 검사와 일부 직접 조작 |
| 미션 3-5 항공기 | 원작 접근 조건 전 선행 이동·공격 제거 | 활성 조건 비교 |

전체 무기 검사는 연사·해제·목표·탄약을 다뤘다. 20종 모두를 GUI로 순환하는 시각·청취 검사와 모든 충돌·피해 조합은 남았다. 실제 홈 이동→복귀→정지 유지→재개는 관측했으며, 잠금 버튼 경로와 물리 회전은 후속 항목이다.

[무기별 결과와 실기기 검사 범위](physical-test.md)

## 화면 확장 후 적 이동 점검 {#ai}

미션 3-5의 항공기 보스는 넓어진 시야에서 일찍 생성되면서 이동·공격도 먼저 시작했다. 원작 접근 조건에서 전투를 시작하도록 표시 준비와 AI 활성을 분리했다. 이후 내 전체 미션 플레이에서는 적 이동 이상이 관측되지 않았다.

전체 적의 고정값 조사는 예방 검토로 남겼다. 원작 480과 확장 화면에서 같은 시드·입력을 사용해 이동과 전투가 달라지는지 비교한다. 차이가 재현되면 그 값을 쓰는 목적부터 확인한다.

| 조사 대상 | 현재 기준 | 비교할 영향 |
|---|---|---|
| 지상·비행 이동 | 거리 50 / 400 / 120 | 접근·후퇴 비율과 이동 공간 |
| 전장·보스 경계 | 배치+150, 전선−30 | 표시 여유와 실제 이동 범위 |
| 무기 AI | 부호 있는 dx+dy가 0..520 | 보이는 적과 공격 가능한 적의 차이 |
| 생성·카메라·정리 | 500/10, W/2, 후방 240/320 | 출현·활성·화면 밖 정리 시점 |

이동 속도·사거리 같은 게임 거리는 보존한다. 화면 가장자리를 뜻하는 값만 확인된 의도에 따라 화면 경계로 옮긴다. 모든 상수에 폭 비율을 곱하면 이동 시간과 난이도도 바뀐다.

미션 3-5에서 시작해 지상·비행·고정형·다중 보스·좁은 맵·Survival을 비교할 계획이다. 최초 가시·활성·공격 시각, 최소 거리, 좌우 이동량·체류 시간, 경계 정체 시간과 동시 공격 수를 기록한다.

참고한 설계 사례: [Godot 다중 해상도](https://docs.godotengine.org/en/stable/tutorials/rendering/multiple_resolutions.html), [화면 기반 활성화](https://docs.godotengine.org/en/stable/classes/class_visibleonscreenenabler2d.html), [Camera2D](https://docs.godotengine.org/en/stable/classes/class_camera2d.html), [Epic AI Perception](https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-perception-in-unreal-engine). 조사 당시 자료와 HeavyMach 적용 후보는 [조사 원문](research.md)에 있다.

## 가로 화면 회전 지원 {#rotation}

일반·치트 앱에는 양쪽 가로 방향이 등록돼 있고 방향별 센서 보정과 레이아웃 갱신 코드가 있다. 2026-10-27 iPhone·iPad에서 회전 뒤 터치 사격을 확인했고, 내 플레이에서도 화면 회전은 정상 동작했다. 틸트·사격을 유지하면서 반복 회전하는 검사는 남아 있다. 세로 플레이는 지원하지 않는다.

180도 회전에서는 폭·높이가 그대로일 수 있다. 크기 변화와 함께 scene의 실제 방향 변화를 받아 화면·안전영역·터치·센서 필터를 갱신하는 경로를 확인한다. 전환 중 사격 입력이나 이전 센서 값이 남는지도 검사한다.

계획한 검사는 양쪽 방향 콜드 실행, 전투 중 180도 전환 20회, 사격·틸트 유지 중 회전, 메뉴·결과·치트 패널과 백그라운드 복귀다. 화면 오른쪽을 내릴 때 오른쪽으로 움직이고 양 끝 터치가 맞는 월드 좌표를 가리켜야 한다. 회전 중 진행·저장은 유지한다.

API 조사: [effectiveGeometry](https://developer.apple.com/documentation/uikit/uiwindowscene/effectivegeometry), [지원 방향](https://developer.apple.com/documentation/BundleResources/Information-Property-List/UISupportedInterfaceOrientations), [크기 변화 통지](https://developer.apple.com/documentation/uikit/uicontentcontainer/viewwilltransition(to:with:)). 조사안은 iOS 16+ 조회, iOS 26+ 변경 통지와 최소 지원 iOS 15의 경로를 구분했다.

## 나머지 메뉴의 전체 화면 지원 {#screens}

타이틀·슬롯·모드·미션의 배경 확장을 적용했다. 2026-10-27 첫 실행 타이틀·슬롯·모드·미션의 네 화면 비율과 결과 일러스트를 검사했다. 전환의 문은 화면 전체를 덮는 한 벌로 수정했다. 게임 메뉴·업그레이드는 월드 위에 겹치는 패널로 유지한다. [수정 내용과 Simulator 캡처](fullscreen.html).

버튼·문구·로고는 원본 비율을 유지해 안전영역 안에 배치한다. 큰 배경은 원본 옆에 별도 그림을 붙인다. 늘이거나 잘라서 원본 구성을 바꾸는 대신 화면별로 필요한 주변 공간을 만든다.

### 배경 제작 방법 {#asset-plan}

원본을 참조해 imagegen으로 좌우 확장 후보를 만든다. 원근·수평선·광원·색·질감·정보 밀도를 맞추고 글자와 버튼은 생성하지 않는다. 게임은 중앙 원본과 채택한 좌우를 별도 자원으로 그린다. 배경 이동·확대·회전에는 같은 변환을 사용한다.

타이틀·슬롯부터 시작해 모드·미션으로 확장한 방식이다. 다음 화면도 치트 앱에서 Air·Pro Max 비율, 양쪽 안전영역, 이음새·글자·버튼·터치·전환·자원 해제를 확인한 뒤 일반 앱에 반영한다. 화면별 캡처와 구성·시안·선택 이유를 남긴다.

[전체 화면 조사·구현 후보](research.md)

## 남은 작업 {#evidence}

| 과제 | 현재 상태 | 다음 검사·구현 |
|---|---|---|
| 전체 화면 단일 문 | 구현·Simulator 검사 완료, 닫힌 문10조건 배경 노출0픽셀. v0.11 이후 iPhone·iPad 설치 | 기기별 화면 비율 회귀 검사 |
| 적 AI 고정값 | 보스 조기 활성 수정, 직접 한 전체 미션에서 이상 미관측 | 적 계열별 예방 비교 |
| 양방향 가로 | 두 기기에서 회전 뒤 사격 확인, 직접 플레이에서 정상 동작 확인 | 틸트·사격 유지 중 반복 회전 검사 |
| 발열 | 짧은 실기기 기록의 열 상태 Serious | 같은 정상 초기 열 상태에서 비교판·복원판 반복 3회 장시간 계측 |
| 소리·잠금 | 호출·상태 검사와 홈 복귀 관측 | 스피커 청취·실제 잠금/해제 |
| 나머지 전체 화면 UI | 타이틀·선택4화면×4비율·결과 그림 검사 완료 | 실제 기기 전체 화면 수동 검수 |
| 무기 전체 체험 | 20종 자동 연사·목표·탄약 검사 | 모든 무기의 수동 전환·효과·충돌·피해·청취 |

보스 활성 수정 `acfae6f8`은 코어 17/17·iOS 2/2 검사를 통과했다. 조사·TODO는 `53b2ef80`, 메뉴 첫 적용은 `435fa968`, 배경 연결·검은줄 수정은 `72e8c68e`에 있다. 각 기록의 검사 시점과 범위는 해당 문서에서 확인할 수 있다.

[전체 TODO](todo.md) · [보스 활성 측정](boss-fix.md) · [조사 근거](research.md)
