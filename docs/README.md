# HeavyMach 문서

최신 릴리스 태그는 v0.17이며 Android 일반·치트 앱을 포함한다. 현재 main에는 이후의 웹/PWA·Tauri Mac·복원 코드·게임 버튼 수정도 들어 있다. [프로젝트 소개와 플레이 화면](../README.md)에서 시작하거나 아래 주제로 바로 이동할 수 있다.

## 게임과 개발 과정

| 문서 | 내용 |
|---|---|
| [개발자의 변](RESTORATION.md) | HeavyMach가 직업 전환에 준 영향과 다시 만든 이유 |
| [왜 럭스루나에인가](public/lux-lunae-intent-20261028.md) | 두 게임의 개인적인 의미, 기종·도색·전투 기획, 도안과 플레이 화면 |
| [기능](public/features.md) | 기존 게임, 자동조준·항공기·화면·플랫폼 추가의 기획과 구현 |
| [조작](public/gameplay-guide.md) | 터치·틸트·키보드·자동조준·무기·강화 |
| [빌드·설치](public/build-and-install.md) | clone·LFS·workspace·scheme·서명·실행 |
| [구조](public/architecture.md) | 코어·Apple 런타임·자원·플랫폼 경계와 모듈화 이유 |
| [트러블슈팅](public/troubleshooting.md) | 문제의 증상·내 지적·원인·수정·결과 |
| [복원 코드·웹·PWA·Tauri](public/save-recovery-20261102.md) | 서버 없는 진행 이전, 공용 모듈, 버튼 수정, 실제 플레이·복원 화면 |
| [Android판 개발 과정](public/android-development-20261030.md) | 개발 기간·공유 코어·전용 구현·Galaxy 검사·업그레이드 오류 수정 |
| [Android 설치](public/build-and-install.md#android-빌드와-설치) | SDK·NDK 설정, 일반·치트 APK, USB·무선 설치 |
| [Android 플레이 캡처와 영상](public/android-gameplay-20261030.md) | Galaxy 일반 앱의 이미지 8장·GIF 3개·영상 3개 |
| [개발 과정](public/development-history.md) | 분석·구현·비교·검증 순서와 도구·스킬 |
| [검증](public/validation.md) | 기기·조건·빌드별 결과와 남은 검사 |
| [쉴카·퉁구스카 도안](public/ground-aa-art-20261028.md) | 새 외형·파츠 분해도와 기존 유닛 조립 구조 비교 |
| [지상 유닛 추가 연구](public/ground-units-research-20261028.md) | 쉴카·퉁구스카, 원작 파츠·무기·움직임 재사용과 남은 TODO |
| [HUD 개선 계획](public/hud-improvement-plan-20261028.md) | 그림 분할·현재 장면/예시 전투·일시정지/시작 전 편집·입력 |
| [자동조준 상태 안내](public/auto-aim-visibility-plan-20261028.md) | HUD 편집과 별도인 대상 없음·화면 밖·가림 표시 |
| [구현 상태](public/implementation-status.md) | 구현·문서 작업·후속 과제 |
| [변경 이력](../CHANGELOG.md) | v0.9~v0.17의 변경·검사·다운로드와 이전 버전 비교 |
| [제작자·자원](../CREDITS.md) | 원작, 기획·구현 역할, 그림·라이브러리 |
| [저장소 관리](repository-management.md) | develop→main, 태그와 이전 버전 재빌드 |

## 코드 변경 시 참고할 문서

[정본](canonical.md)과 [게임 기획](game-design.md)이 요구를 정의한다. [문서 위계](document-governance.md)와 [업무별 연결표](document-map.json)에서 변경할 분야의 정책·소스·검사·증거를 찾는다.

| 분야 | 정책·절차 |
|---|---|
| 월드·입력 | [좌표](coordinate-policy.md), [키보드](keyboard-input-policy.md), [미션 전환](stage-transition-contract.md) |
| 그래픽·아트 | [렌더링](rendering-policy.md), [VTOL](vtol-expansion.md), [전투기](lux-lunae-plan-20261027.md) |
| 전투·성장 | [계산](game-calculation-policy.md), [자동조준](auto-aim-plan-20261026.md), [전체 무기 검사](weapon-validation-procedure.md) |
| 실행·저장 | [시간·음향·생명주기](runtime-policy.md), [Apple 플랫폼](apple-platforms.md), [무선 기기](wireless-device-workflow.md) |
| Android 개발 | [구현과 검사](android-implementation-20261030.md), [이식 계획과 보존 조건](android-port-plan-20261026.md), [빌드·기기 도구](../Android/README.md) |
| Mac·웹·PWA | [개발 계획](desktop-web-plan-20261031.md), [실행 비교](reviews/mac-web-pwa-comparison-20261101.md), [웹 실행·설치](../Distribution/Web/README.md) |
| 후속 개발 | [투두](usability-todo.md) |

```sh
python3 tools/document_contract.py plan --task documentation
python3 tools/document_contract.py check
```

**공개와 라이선스**

- [코드 라이선스 범위](../LICENSE-SCOPE.md), [MIT 원문](../LICENSE), [자원 조건](../ASSET-LICENSES.md), [외부 고지](../THIRD_PARTY_NOTICES.md).
- [GitHub 구성 점검·보완 계획](public/github-repository-plan-20261028.md): 현재 기본 화면, 다른 게임 사례, 배지·릴리스·기여·CI 순서.

## 글·이미지·기록

- [공개 라이선스·README 배지 조사](public/github-licenses-and-badges-20261028.md): 코드·자원 배포 조건, 필요한 문건, 기술 배지와 OS 표.

- [SaaS 집필 자료](public/publication-inventory.md): Apps 기술 설명과 Mags 인터뷰에 쓸 개인 경험 원문·기능·수정 과정·검사 기록.
- [PWA·Tauri·첫 PR·버튼 수정 집필 안내](editorial/web-tauri-sass-handoff-20261102.md): 주제별 근거, Apps 구성, Mags 질문·답변 자료와 이미지.
- [플레이 이미지·GIF·영상](public/media/gameplay-20261027/README.md): 채택한 8장·2개·3개와 원본 시간.
- [기존 그림 색인](public/media/README.md): 메뉴 수정 전후와 추가 아트.
- [현재 작업 기록](current-status.md): 변경·설치·계측의 누적 기록.
- [날짜별 보고](public): 당시 상태와 후속 수정 기록.
- [검사 증거](evidence): 수치·파일·빌드·조건 JSON과 자료.
- [소스 대응](ios-source-map.md), [분리 후 전수검사](migration/completeness-audit-20261026.md): 파일과 문서 보존 내역.

과거 보고의 최초 결과 뒤에는 후속 수정이 이어진다. 현재 기능은 [구현 상태](public/implementation-status.md), 수치별 조건은 [검증](public/validation.md)에서 찾을 수 있다.

- [GitHub 구성과 v0.12 소스 릴리스](public/github-followup-20261028.md): 도입문, 기여 안내, 자동 검사, 압축본 빌드·OpenAL 재링크.

- [앱 아이콘·최신 기능 이미지](public/app-icon-and-media-20261029.md): 아이콘 변경, 최종 지상 유닛·HUD 캡처, GitHub/SaaS 자료.
