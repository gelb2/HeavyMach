# SaaS Apps·Mags 집필 자료

HeavyMach에 관해 내가 쓴 원문과 기술 설명·수정 과정·이미지를 이 저장소에 모았다. Apps는 기술 설명, Mags는 매거진 인터뷰 형식이다. 두 곳 모두 현재 기능과 시정 과정을 다룬다. SaaS 담당 세션이 이 자료로 원고를 작성하고 서비스에 반영한다.

집필은 이 저장소 **main의 최신 문서**에서 시작한다. Apple·Android 게임 릴리스는 **v0.17**이다. Android 추가 자료는 [개발 원고](android-development-20261030.md)에 있다. 이후 추가한 글과 이미지 설명은 main에 있다.

```sh
python3 tools/document_contract.py plan --task developer_story
python3 tools/document_contract.py check
```

첫 명령은 원문·기술 문서·그림·글쓰기 스킬의 읽기 목록을 출력한다. 아래 순서로 읽고, 기능의 세부 조건은 연결된 정책과 코드에서 확인한다.

## 읽는 순서

1. [개발자의 변](../RESTORATION.md)과 [개발 동기 자료](../developer-background.md): 내가 제공한 원문, 연도 정정, 잡지사·필리핀 근무와 개발자 전업, HeavyMach를 선택한 이유.
2. [README](../../README.md), [기능](features.md), [구현 상태](implementation-status.md): v0.17 게임과 추가 기능, 완료·남은 과제.
3. [개발 과정](development-history.md), [트러블슈팅](troubleshooting.md): 분석·비교 순서, 내 질문과 시정 지시, 기술 원인과 결과.
4. [구조](architecture.md), [설치](build-and-install.md), [검증](validation.md): 기술 구성, 실제 사용 절차, 날짜·기기·조건별 수치.
5. [미디어 명세](media/gameplay-20261027/media-scenes.json), [문장 스킬](../../.agents/skills/write-project-text/SKILL.md): 장면·캡션·원본 시간과 집필 규칙.

## Apps 기술 설명에 연결할 자료

| 주제 | 원고 | 세부 근거 |
|---|---|---|
| 게임 기능과 조작 | [기능](features.md), [조작](gameplay-guide.md) | 게임 기획, 입력·계산 정책 |
| 자동조준 | [기능의 자동조준 절](features.md) | AutoAim.hpp·GameSession.cpp, C 삭제와 수동 사격 25,200프레임 기록 |
| VTOL·전투기 | [기능의 항공기 절](features.md) | VTOL.cpp·Fighters.cpp, 수량·무장·배치·실기기 검사 |
| workspace·모듈 | [구조](architecture.md) | Modules/iOS/macOS 프로젝트 설정, 공용 코어·자원 bundle |
| 성능·버그 수정 | [트러블슈팅](troubleshooting.md) | 각 사건의 정책·코드·원시 검사 링크 |
| 설치·운영 | [설치](build-and-install.md), [버전 관리](../repository-management.md) | clone·LFS·서명·무선·태그 |
| 현재 범위 | [상태](implementation-status.md), [검증](validation.md) | 내 플레이, 자동 검사, 미구현·남은 검사 |

## Mags 인터뷰 자료

| 이야기 | 내가 제공한 원문·사건 | 기술 설명 |
|---|---|---|
| 왜 HeavyMach인가 | [개발자의 변](../RESTORATION.md), [개인 경험 원문 목록](../developer-background.md) | 2009년의 플레이와 2017년 7월 전업, 2026년 복원 |
| 원작과 무엇이 달랐나 | 사격마다 저장, UIKit 렌더, 시간 누적에 대한 내 질문 | [트러블슈팅 1~5절](troubleshooting.md) |
| 직접 해보고 무엇을 발견했나 | 틸트 반전, 필살기 후방 낙하, 보스 움직임, 그림 누락·검은줄 | [트러블슈팅 6~11절](troubleshooting.md) |
| 어떤 게임을 더 만들고 싶었나 | 넓은 화면, 키보드, 자동조준, 실제 기체·도색·무장 조건 | [기능](features.md) |
| 구현자가 틀렸을 때 어떻게 고쳤나 | 원본 비교 요구, C 삭제, 전투기 진입·연사 수정 | [개발 과정](development-history.md), [트러블슈팅](troubleshooting.md) |
| 어디까지 됐나 | 직접 한 전체 미션 및 iPhone·iPad 플레이 확인 | [검증](validation.md), [남은 과제](implementation-status.md) |

인터뷰 답변의 화자는 나다. 내 기획·판단·직접 플레이와 Codex가 맡은 분석·구현·검사를 구분한다. 기술 설명에서는 기능과 코드의 동작을 주어로 쓴다.

개인적인 답변의 원문은 `docs/references/developer-background/`에 있다. 필리핀 근무는 2016년, 개발자 전업은 2017년 7월이다. HeavyMach의 영향 약 35%는 내 회고다. 원문·수정 문장·출처 해시는 [자료 목록](../references/developer-background/source-manifest.json)에 보존했다.

## 사용할 이미지와 영상

| 파일 | 보여주는 내용 | 배치 예 |
|---|---|---|
| 01-auto-aim.png / gif / mp4 | 표적·탱크·HUD와 자동조준 사격 | 추가 항공기 소개 다음, 자동조준 |
| 02-jump.png | 경사로 위 점프와 사격 | 첫 전투 화면, 원작의 틸트·터치 조작 |
| 03-weapon.png | 무기 목록과 발사 궤적 | 무기 전환 |
| 04-special.png | 상공에서 내려오는 푸른 광선 | 공중 지원 공격 |
| 05-upgrade.png | 이동·방어·공격 강화 창 | 성장 |
| 06-stage-clear.png | 결과 일러스트와 다음 미션 버튼 | 미션·일러스트 복원 |
| 07-fighters.png / mp4 | 추가 항공기 편대와 전투 | 신규 유닛 |
| 08-jump-clip.png / gif / mp4 | 표적을 추적하며 사격·점프·착지·무기 목록 | 자동조준 중 이동·전투 |

모든 파일은 [playback 자료 폴더](media/gameplay-20261027/README.md)에 있다. PNG는 2736×1260, GIF는 원래 속도의 짧은 미리보기, MP4는 소리를 포함한다. 촬영 기종·앱 종류·빌드 번호는 미확인이다. 파일별 원본 시간·SHA-256·캡션·대체 텍스트를 함께 전달한다.

기존 [그림 색인](media/README.md), [VTOL 시안과 실제 게임](vtol-art-and-loadout-20261027.md), [전투기 시안·실제 게임](lux-lunae-design-20261027.md)에는 미술 비교 자료도 있다.

## 버전과 남은 근거

최신 릴리스 태그는 v0.17이며 Android 일반·치트 앱을 포함한다. 현재 구현과 남은 일은 [구현 상태](implementation-status.md)와 [내부 TODO](../usability-todo.md), 변경 순서는 [작업 이력](../current-status.md)에 있다. 작업 이력의 날짜별 항목은 당시 상태를 담고 있다. [변경 이력](../../CHANGELOG.md)과 [전체 문서 목차](../README.md)로 연결된다.

빌드 경고는 아직 전수조사하지 않았다. deprecated API 관련이라는 추정을 확인하고 추가 기능 개발과 병행해 수정할 예정이다. 빌드·실행 스크립트는 사례 조사를 마쳤고 구현이 남았다. [스크립트 조사·브랜치 정리](github-followup-20261028.md).

추가로 필요한 자료는 장시간 발열·전력 비교, 추가 적의 전체 미션 난이도 검사, 실제 Bluetooth·USB 키보드 조작, 촬영 환경 정보, 원작자 답변이다. 현재의 실행 수치와 실패는 [검증](validation.md)에 모았다.


## 공개·배포 준비 자료

[라이선스·README 배지 조사](github-licenses-and-badges-20261028.md)에 코드·자원·외부 라이브러리의 배포 조건과 필요한 문건, 기술 배지·환경 표를 정리했다. 새 코드용 MIT·적용 범위·자원 조건·외부 고지와 README 배지를 추가했다. 2026-10-28 저장소를 공개했다. [GitHub 구성 점검·후속 계획](github-repository-plan-20261028.md).

## GitHub 후속 자료 — 2026-10-28

[도입문 원문·정정](../references/developer-background/user-project-origin-20261028.md), [GitHub 구성·소스 릴리스 검사](github-followup-20261028.md), [자원 공개 범위](../rights-manifest.json)를 추가했다. README는 「HeavyMach를 다시 만든 이유」 아래에 2017년 7월 전업, HeavyMach의 영향, 지인들의 말과 9년 3개월, 현재 복원 이유, Mesabloo와 꿈의 순서로 배치했다. [채택 원고](../references/developer-background/approved-readme-opening-20261028.md)에 전문이 있다. 개발자의 변과 공유하는 문단은 같은 문장을 유지했다. Apps·Mags 집필은 SaaS 담당 세션에서 진행한다.


## 원작 기능과 추가 기능

| 구분 | 설명할 내용 | 원고·그림 |
|---|---|---|
| 원작에서 이어온 기능 | 틸트 이동·점프·터치 사격, 무기 전환·지원 공격, 강화·미션 진행 | [조작](gameplay-guide.md), 플레이 이미지 02~06 |
| 내가 요구한 추가 기능 | 화면 확장, 가로 회전, iPad·Mac·키보드, 자동조준, VTOL·전투기·쉴카·퉁구스카·HUD 편집 | [기획 의도·조건·구현](features.md), [공용 모듈](architecture.md), 플레이 이미지 07→01·08 |
| 개발 중 발견하고 고친 문제 | 발사마다 저장, 렌더링·갱신 주기, 좌표·그림·입력, 임의로 바꾼 기체 외형과 탄도 | [트러블슈팅](troubleshooting.md), [수정 순서](development-history.md) |
| 아직 남은 기능·검사 | 조준 상태 안내, 튜토리얼, Android 장시간·태블릿 검사·배포 서명, 빌드 경고·실행 스크립트 등 | [상태표](implementation-status.md), [상세 TODO](../usability-todo.md) |

자동조준은 표적 선택·추적과 포신 각도 조절을 맡는다. 이동·점프·무기 선택·발사는 플레이어가 한다. 탄속·탄도·분산을 바꾸던 C 기능은 삭제했다. 추가 유닛의 기획 조건과 실제 채택 수치는 기능 문서에서 각각 설명한다.

## 원문과 집필 범위

개인 경험 원문과 채택 원고 8개, SaaS·Candidate에서 가져온 글쓰기 자료는 저장소 안에 있다. [원문 목록](../references/developer-background/source-manifest.json)과 [스킬 출처](../../.agents/skills/write-project-text/references/source-manifest.json)에서 복사 범위와 해시를 확인할 수 있다. Unity 초기 기획은 개인적인 시작 배경을 설명하는 별도 프로젝트 자료다.

Mags의 답변은 개발자의 변과 제공 원문을 바탕으로 쓴다. 추가로 물을 내용은 질문안으로 정리한다. Apps는 현재 기능·기술 구성·시정 과정·검사 결과를 설명한다. 개발자의 판단과 직접 플레이, Codex의 구현과 검사를 구분한다. 문장 규칙은 [글쓰기 스킬](../../.agents/skills/write-project-text/SKILL.md)에 있다.

과거 분석 기록에는 당시 도구와 원본 파일을 다룬 내용이 있다. 새 원고의 범위는 복원 동기, 현재 기능, 구현·수정 과정과 설치 방법이다. 기술 사실을 확인할 때는 분석 기록을 참고하고, 집필에는 위에 연결한 원고와 최신 사용자 정정을 사용한다.

## 2026-10-28 자료 검사

최신 개발 동기 원문, 선정 미디어, 내부 TODO와 GitHub 후속 조사 문서를 업무별 읽기 목록에 추가했다. 이전 집필 브랜치 안내를 최신 main 기준으로 바꿨다. 개인 원문·스킬 사본·선정 미디어·비교 그림의 해시와 문서 연결을 검사했다. [검사 파일과 결과](../evidence/sass-handoff-audit-20261028.json).


## 원작 제작자와 위메이드

[원작 크레딧 검토](github-followup-20261028.md)에 2009년 제작·출시와 2011년 IP 인수·리뉴얼 발표 자료, README·CREDITS 보강안을 모았다. 변해준·박재철의 제작 역할과 위메이드 크리에이티브의 인수 역할을 구분한다. [CREDITS](../../CREDITS.md)·README·자원 문서에 반영했다. Apps에는 제작자·출시·인수 역할을 구분해 설명한다. Mags에는 [이번에 인수 사실을 알았다는 원문](../references/developer-background/user-wemade-discovery-20261028.md)과 개발자의 변에 추가한 경위를 사용한다.

## 후속 기획 자료 · 2026-10-28

[쉴카·퉁구스카 추가 연구와 남은 과제](ground-units-research-20261028.md)에 재사용 후보, 차체·파츠의 덜컹거림 요구, 무장 1종/2종 구성안과 검증 계획을 정리했다. [새 외형·파츠 도안 2장과 조립안](ground-aa-art-20261028.md)을 추가했다. 이후 `feat/ground-aa`에서 독립 회전 레이더·흰색 총탄 6/4발·미사일 2발, 35개 스테이지 152대 배치를 구현했다. [기술 문서](../ground-aa-expansion.md)와 [검증 기록](../evidence/ground-aa-20261028.json)에 현재 결과를 모았다.

2026-10-28 구매 경위 추가: Cydia로 처음 접한 뒤 미국 App Store 계정을 만드는 방법을 찾아 구매했다. 당시 한국 스토어에는 게임 카테고리가 없었다. [추가 원문](../references/developer-background/user-cydia-purchase-20261028.md)을 README·개발자의 변·채택 원고에 반영했다.


## 럭스루나에 기획 의도 보고

[왜 럭스루나에인가](lux-lunae-intent-20261028.md)에 개인 경험·보스전 기획·기술 검증 의도, 도안 3장, 게임 화면 4장과 플레이 영상 1개를 모았다. 대화 원문 14개를 함께 보존한다. [통합 보고 사이트](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/lux-lunae-intent.html).

기종 선택과 형태의 유사성에 대한 아쉬움, 악역의 위협과 매력, 실제 필살기 사용 증가, Codex의 원작 화풍 재현 실패, 직접 지시하지 않았지만 마음에 들어 유지한 빠른 진입을 담았다. 편대 밀집은 내부 TODO의 `HM-LUX-FORMATION`으로 남아 있다. 화면 밖 이탈·재등장은 구상이다.

사용자가 채택한 해석을 반영해 두 게임이 삶에서 했던 역할과 럭스루나에를 강한 적으로 구현한 이유를 연결했다. [데스 스트랜딩 사연 조사](../references/developer-background/death-stranding-community-story-20261028.md)를 참고했고, 마지막에는 현재도 이어지는 수정과 구상을 적었다. Apps·Mags 집필 때에는 [사용자 원문](../references/developer-background/user-lux-lunae-intent-20261028.md)과 이 원고를 함께 읽는다.

## HUD 편집 · v0.14

[HUD 편집 원고와 실제 기기 화면](hud-improvement-plan-20261028.md)에 요구·분할·입력·저장·편집 도구 수정과 iPhone/iPad 검사를 모았다. 버튼·계기판·상단 장식 이동은 구현됐고, 조준 상태 안내 개선은 별도 후속 과제다. [최신 검사 증거](../evidence/hud-editor-release-20261029.json).

## 최신 기능 이미지 · 2026-10-29

[쉴카·퉁구스카·HUD 편집 이미지](app-icon-and-media-20261029.md)와 [장면별 출처·캡션](media/updates-20261029/media-scenes.json)을 추가했다. 쉴카는 포탑 높이29 수정 후 이미지, 퉁구스카는 최종 파츠 적용 이미지, HUD는 영문 도구·상단 장식 복원 후 이미지를 사용한다. 각 장면의 기기·날짜·치트 앱 여부·원본 해시를 함께 기록했다.

Apps에는 지상 유닛의 파츠·무장과 HUD 그림/터치 위치 공유를 설명할 때 쓴다. Mags에는 내가 지정한 유닛 조건과 화면 배치 요구를 설명하는 장면으로 쓴다. 이 작업에서는 Apps·Mags 본문을 작성하거나 sass.studio를 수정하지 않았다.

새 아이콘은 [1024px PNG](media/app-icon-20261029/icon.png), 생성·파생 파일은 [아이콘 원본 폴더](../../Art/AppIcon/20261029/README.md)에 있다. 새 아이콘은 v0.15부터 포함됐으며 최신 릴리스는 v0.17이다. 이후 변경은 [현재 상태](../current-status.md)를 확인한다.

## Android판 · 2026-10-30

[Android 개발 원고](android-development-20261030.md)를 Apps의 플랫폼 확장 설명과 Mags의 개발 과정 문답에 사용한다. 개발 기간, Apple 소스를 바꾸지 말라는 지시, 공유 코어·자원, Android 전용 파일, 빌드·입력·GPU·무선 연결 수정과 직접 플레이에서 발견한 업그레이드 오류를 담았다.

세부 수치는 [구현 기록](../android-implementation-20261030.md), [시간 근거](../evidence/android-development-timeline-20261030.json), [업그레이드 오류 증거](../evidence/android-upgrade-fix-20261030.json)에 있다. [이미지와 캡션](media/android-20261030/scenes.json)은 타이틀·보스 전투·오류 전후 5장이다. Android 구현과 자료는 main에 병합됐다. v0.17에 포함됐으며 [변경 이력](../../CHANGELOG.md#v017)에 정리했다.

[개발 범위를 정한 네 가지 판단](android-development-20261030.md#개발-범위를-정한-네-가지-판단)에 부분적인 코어 공유, Apple 파일 변경 금지, 컨텍스트·생명주기 사전 확인, Android 스킬 준비를 실제 코드와 검사 결과에 연결했다. Apps에는 플랫폼 분리와 재사용 방식, Mags에는 내가 작업 범위와 검사 대상을 정한 이유를 설명하는 자료로 사용한다. 수정 빌드 완료 시각과 이후 내 플레이에서 오작동이 보이지 않았다는 확인도 추가했다.

## Android 실제 플레이 자료 · 2026-10-30

[Galaxy 미션 3-4·3-5 캡처 8장·GIF 3개·영상 3개](android-gameplay-20261030.md). [출처·타임코드·해시](media/android-gameplay-20261030/scenes.json). Android 기술 원고는 [개발 과정](android-development-20261030.md), 빌드와 설치는 [Android 안내](../../Android/README.md)를 읽는다. iOS 자료와 구분된 폴더이며, 인터뷰에는 직접 플레이한 경험과 구현·검사 기록을 함께 활용할 수 있다.

## iOS 플레이 캡처 모음 · 2026-10-30

[iOS 플레이 갤러리](ios-gameplay-gallery-20261030.md)에 이미지 8장·GIF 2개·음향 포함 영상 3개를 모았다. 실제 iPhone·iPad 치트 앱 검수 화면 4장은 별도 절에 배치했다. 각 장면의 출처·시간·기기 정보는 갤러리에서 연결한 미디어 명세에 있다.

## Android 개발 경험과 가설

[신(新) 리센코주의와 정렬세](android-ai-experience-20261030.md)에 Android 개발 뒤의 의문, 웹·정치인 지면·회계 업무에서 겪은 사례, 작업 조건, 기술적 재사용 효과와 원인 가설을 한 편으로 정리했다. [영문](../en/public/android-ai-experience-20261030.md)도 있다.

Apps의 Android 구현·검사 설명은 [개발 과정](android-development-20261030.md)을 기준으로 쓴다. Mags에서 경험과 가설을 다룰 때는 위 전문을 기준으로 질문과 답변을 구성한다. 개발자의 변와 럭스루나에 원고의 기존 문단은 각각 유지한다.
