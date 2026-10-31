# 저장소와 브랜치 관리

2026-10-26에 정한 규칙이다. 허브·HeavyMach·Unity 저장소에 공통 적용한다.

1. 개발은 각 저장소의 `develop`에서 분기한 `feat/*`, `fix/*`, `docs/*` 등 작업 브랜치에서 수행한다.
2. 작업 브랜치는 `develop`에 병합한다. `main`에 직접 병합하지 않는다.
3. `main`에는 `develop`만 병합한다. 릴리스에는 병합 커밋이 남는 merge 방식을 사용한다.
4. 릴리스 태그는 main의 해당 병합 커밋에 붙인다. 기존 태그를 이동하거나 덮어쓰지 않는다.
5. HeavyMach 최초 정상 플레이 기준 버전은 `v0.9`다. Unity 또는 허브의 게임 플레이 완료를 뜻하지 않는다.
6. 게임 코드와 함께 빌드 구성·의존성 버전·에셋 해시·검사 근거를 보존한다. 플레이 저장·서명 키·로컬 빌드는 Git에 올리지 않는다.

## main 직접 수정 문서 보존 — 2026-10-29

사용자는 GitHub 웹에서 main의 README·개발자의 변 등 문서를 직접 수정할 수 있다. 이후 develop→main 병합 때 해당 변경을 보존한다.

1. 병합 직전에 `git fetch origin main develop`을 실행하고 원격 두 브랜치의 커밋을 기록한다. 로컬 main만 기준으로 삼지 않는다.
2. 공통 조상 이후 main과 develop 양쪽의 커밋·변경 파일·본문 차이를 확인한다. README·개발자의 변뿐 아니라 사용자가 수정한 모든 문서를 포함한다.
3. main의 사용자 변경을 통합할 작업 브랜치에서 보존한다. 동일 문단의 변경은 최신 사용자 원문을 기준으로 조합하며, 판단이 필요한 의미 충돌은 구체적인 두 문장을 확인한다. 한쪽 파일 전체를 일괄 선택해 덮어쓰지 않는다.
4. 병합 후보의 최종 문서를 origin/main과 대조한다. 자동 병합이 성공해도 사용자 문장·문단 순서·링크가 이전 내용으로 되돌아가지 않았는지 읽는다. 원문 검사 스크립트도 최신 사용자 본문을 기준으로 유지한다.
5. 반영 직전에 원격 main이 바뀌지 않았는지 다시 fetch한다. 바뀌었으면 그 이후 변경을 추가 대조한다. 관련 문서 검사 후 기존 작업 브랜치→develop→main 경로로 병합한다.

충돌 여부와 최종 본문 보존을 각각 확인한다. 이 절차의 등록만으로 이번 변경을 main에 병합하지 않는다.

## 현재 태그에서 다시 빌드

이번 릴리스 태그는 v0.17이다. v0.10 이후 태그는 게임 자원을 Git/LFS에 포함한다. 별도 폴더에서 이전 버전을 빌드할 수 있다.

```sh
git fetch origin --tags
git worktree add --detach ../HeavyMach-v0.17 v0.17
cd ../HeavyMach-v0.17
git lfs pull
python3 tools/verify_build_resources.py
open HeavyMach.xcworkspace
```

재설치 전 현재 앱 컨테이너를 백업한다. 태그는 코드·자원을 복원하고, 앱의 진행 상태는 별도 저장 데이터에서 복원한다. [빌드·설치](public/build-and-install.md)

## v0.9 복원

`git worktree add --detach ../HeavyMach-v0.9 v0.9`로 별도 폴더에 그 버전을 꺼낸다. `v0.9`는 자원을 Git에 넣기 전 태그다. 태그는 이동하지 않는다. 해당 worktree를 빌드하려면 현재 main에서 `git lfs pull`로 받은 `Resources`를 복사하고 현재 자원 목록의 SHA-256과 대조한다. 현재 main의 새 clone은 별도 복사 없이 빌드한다. iPhone 재설치는 해당 소스를 빌드·서명하고 현재 저장을 백업한 뒤 진행한다. Git 태그만으로 폰 설치본이나 저장 데이터가 되돌아가지는 않는다.

후속 변경을 제품에서 취소할 때는 작업 브랜치에서 필요한 revert 커밋을 만들고 develop→main으로 전달한다. 공유 main을 reset하거나 force-push하지 않는다.

## 허브 연결

허브는 `.gitmodules`와 gitlink에 하위 저장소 URL·정확한 커밋을 기록한다. 하위 저장소의 커밋을 먼저 원격에 올린 뒤 허브 포인터를 갱신한다. 과거 허브 커밋을 checkout하면 당시 두 게임의 연결 버전을 확인할 수 있다.

## 검사와 한계

`.githooks/pre-push`는 main의 두 번째 부모가 develop인지 검사하고 Git LFS 업로드를 실행한다. PR 검사도 main의 source branch가 develop인지 확인한다. 로컬 훅을 사용하려면 clone마다 `git config core.hooksPath .githooks`를 설정한다.

현재 GitHub 계정의 비공개 저장소 branch protection API는 요금제 제한으로 403을 반환했다. 서버 branch protection은 설정되지 않았다. 로컬 훅과 PR 검사, 저장소 지침을 적용하며 관리자에 의한 우회까지 차단한 상태는 아니다.

## v0.10 — 2026-10-27

VTOL 크기·외곽선·불꽃 수정 `6404c999`까지 작업 브랜치→develop(`63afa4f5`)→main(`76cbd21c`)으로 병합했다. main 병합 커밋에 annotated tag `v0.10`을 붙였고 develop·main·태그의 원격 반영을 확인했다. v0.9는 이동하지 않았다. v0.10은 Git LFS 자원을 포함한다.

이후 전체 화면 도어 수정은 develop에서 분기한 `fix/fullscreen-transitions`에서 진행한다. v0.10에는 포함되지 않는다. [변경·검사](public/fullscreen-transitions-20261027.md).

## 자동조준 개발 브랜치 — 2026-10-27

`develop`의 `8fdeb629`에서 `feat/auto-aim-ipad-validation`을 분기했다. 내 지시의 dev는 이 저장소의 develop을 가리킨다. 자동조준 개발과 iPad 치트 앱 검증은 이 브랜치에서 진행한다. 완성 전 develop/main 병합과 릴리스 태그 생성은 하지 않는다.

## v0.11 — 2026-10-27

내가 자동조준 작업의 develop→main 병합과 새 버전 태그를 지시했다. `15968772`까지의 자동조준·C 삭제와 이전 전체 화면 도어 변경을 포함한다. `v0.11`은 main 병합 커밋에 붙이며 기존 v0.9/v0.10은 유지한다. C 삭제 검사는 코어20개, 시뮬레이터·iPhone·iPad 각4개, 수동 사격25,200프레임의 기준 버전 일치다. 이전 프레임 계측과 장시간 검사는 남은 과제로 보존한다. 후속 신규 전투기3종은 별도 작업 브랜치에서 진행한다.

실행 결과: develop `e9376910`, main `f0c4432f`, 주석 태그 `v0.11`을 원격에 반영했다. 태그는 main 병합 커밋을 가리킨다. 신규 기획은 develop에서 분기한 `feat/lux-lunae-fighters`에서 진행한다.

## 럭스루나에 develop 반영 — 2026-10-27

Codex가 내 지시에 따라 커밋·dev 병합·브랜치 정리를 진행했다. `feat/lux-lunae-fighters`의 `bd801119`를 develop에 merge commit으로 병합했다. 제품 구현은 `45befedc`이며 화면 밖 진입·연사10% 수정본을 두 기기의 일반/치트 앱으로 설치·실행했다. 병합 충돌은 없었다. main 릴리스와 새 태그는 이번 요청 범위에 포함하지 않는다.

브랜치 정리는 develop의 조상인 작업 브랜치만 대상으로 한다. 로컬은 `git branch -d`, origin은 해당 작업 ref 삭제를 사용한다. main·develop·기존 태그·migration-source 보존 원격은 유지한다. 병합되지 않은 브랜치나 다른 worktree에서 사용 중인 브랜치는 삭제하지 않는다.

## v0.12 — 2026-10-27

내 지시에 따라 남은 문서 작업을 develop에 병합하고, develop을 main에 병합한 커밋에 주석 태그 `v0.12`를 붙인다. 포함 범위는 럭스루나에 전투기3종·금색 조종석·묶음 연사·화면 밖 진입·연사속도10% 조정과 후속 투두/조사 문서다. 당시 아이콘·영상 제작·자동조준 시각 안내·튜토리얼 패키지는 후속 과제였다.

제품 소스·에셋은 두 기기에 설치·검사한 `45befedc`와 같다. 기존 코어21개·시뮬레이터·실기기·Mac의 유효 검사를 재사용하고 문서 연결·자원 해시·병합 결과를 확인한다. 기존 v0.9/v0.10/v0.11 태그를 이동하지 않는다. 이번 버전은 Git 릴리스 태그이며 앱 번들 버전·새 설치 작업은 포함하지 않는다.

## v0.13 — 2026-10-28

쉴카·퉁구스카 추가와 최종 파츠·레이더·포탑 수정을 `feat/ground-aa-refinement`→develop `3677b0a7`→main `bc4e80c0` 순서로 병합했다. main 병합 커밋에 주석 태그 v0.13을 붙이고 원격에 반영했다. 제품 코드는 `d21cfa1c`, 크기·비율 승인 기록은 `4917fe75`다. 원래 적 배치를 보존하고 35개 스테이지에 152대를 추가했다.

최종 제품의 코어23개·iPad 시뮬레이터2개·iPhone/iPad 각5개 검사는 통과했다. 두 기기의 일반·치트 앱은 같은 제품 코드로 무선 설치했다. 이번 릴리스에서는 소스·자원 해시와 병합 결과를 확인한다. 앱 번들 버전은 기존 값을 유지한다. [실기기 검사](evidence/shilka-height-20261028.json).

## v0.14 — 2026-10-29

HUD 편집과 도구·장식 수정을 feat/hud-layout-editor→develop→main 순서로 병합한다. 태그는 main의 병합 커밋에 붙인다. iPhone·iPad 각13개, 코어24개, Mac2개 검사와 두 기기의 정식·치트 설치를 완료했다. 앱 번들 버전은 기존 값을 유지하며 Git 릴리스 태그를 v0.14로 정한다. [검사 증거](evidence/hud-editor-release-20261029.json).

## v0.15 — 2026-10-29

글자 없는 앱 아이콘, 최신 기능 이미지·설명, 미션 선택 HUD 편집 버튼 제거를 포함한다. 작업 브랜치→develop→main 순서로 병합하고 main 병합 커밋에 태그를 붙인다. 앱 번들 버전은 유지한다. iPhone·iPad 시뮬레이터 각 3개와 native Mac 2개 검사, 실제 두 기기 정식·치트 설치·실행, Apple 4종 CI 빌드가 통과했다. [검사](evidence/icon-and-media-20261029.json).

## v0.16 — 2026-10-29

럭스루나에 X·Y 진입 배치와 실기기 검사, 무선 검사 후 앱 종료 절차를 포함한다. 작업 브랜치→develop→main 순서로 병합하고 main 병합 커밋에 주석 태그 v0.16을 붙인다. 앱 번들 버전은 유지한다. iPad 진입 프레임 검사 실패와 전투 중 재밀집은 [변경 이력](../CHANGELOG.md)에 남긴다. 버전별 릴리스 노트·GitHub 공개 절차의 후속 계획은 [투두](usability-todo.md#hm-release-notes--버전별-릴리스-노트와-github-공개-절차--2026-10-29)에 등록했다.

## 버전별 릴리스 노트

[CHANGELOG](../CHANGELOG.md)를 원본으로 쓰고 버전별 GitHub Releases에 같은 본문과 다운로드·검사 링크를 게시한다. README와 문서 목차에서 전체 버전 이력으로 연결한다. [작성·공개 절차](release-notes-plan.md). 기존 태그와 첨부 파일은 유지한다.


## 플랫폼별 폴더와 공용 모듈 — 2026-10-30

HeavyMach는 Apple·Android를 같은 저장소에서 관리한다. 최신 사용자 지시는 **Android 개발 때문에 기존 iOS 프로젝트나 파일을 변경하지 않는 것**이다. 따라서 앞서 제안한 Shared·Apple 폴더 이동과 Apple 코드 선행 공통화는 철회한다.

- 새 Android 앱·Kotlin·JNI·C++ 실행 코드·빌드 설정·검사·로컬 ignore는 `Android/` 아래에 둔다.
- 기존 `HeavyMach.xcworkspace`, `iOS/`, `macOS/`, `Modules/`, `Sources/`, 루트 `CMakeLists.txt`와 Apple 빌드 도구는 이동·수정하지 않는다.
- `Sources/Core`, `Resources`, `Art`, `vendor`는 Android 빌드에서도 기존 파일을 읽어 사용한다. 이 작업에서 수정·덮어쓰지 않는다.
- Android용 실행·메뉴·HUD·렌더·음향 연결은 별도 구현한다. 기존 Apple 코드와 같은 동작을 비교한다.
- 플랫폼별 영구 브랜치는 만들지 않는다. Android 작업 브랜치는 develop에서 분기하고 기존 병합 규칙을 따른다.
- 작업 전후 보호 파일의 경로·해시와 Git 변경 목록을 검사한다. 문서·계획·지침은 최신 요구에 맞춰 갱신한다.

[구성과 검증 순서](android-port-plan-20261026.md#android-구성안). 기존 파일을 이동하지 않으므로 현재 Apple 빌드·설치 문서의 경로도 유지한다.

## v0.17 — 2026-10-30

Android 일반·치트 앱, 업그레이드 창 오류 수정, Galaxy 플레이 자료를 포함한다. `release/v0.17`→`develop`→`main` 순서로 병합하고 main 병합 커밋에 주석 태그를 붙인다. 기존 앱 내부 버전과 게임 코드는 유지한다. [릴리스](https://github.com/gelb2/HeavyMach/releases/tag/v0.17) · [변경 이력](../CHANGELOG.md#v017).

## 외부 PR 기여 보존 — 2026-10-31

PR #24 이후 외부 기여를 통합할 때 원저자의 검토 대상 커밋 SHA·author·공동 작성자·메시지를 보존한다. 후속 수정은 별도 커밋으로 추가하고 merge commit으로 develop에 병합한다. 기존 이력을 squash·임의 rebase·작성자 교체로 대체하지 않는다. main에는 develop만 병합한다.

머지 후 원본 SHA가 대상 브랜치 조상인지 확인하고 GitHub가 원저자 계정과 연결하는지도 검사한다. README·CREDITS·릴리스 노트에 작성자·PR·기여 범위를 적는다. 기본 브랜치 main에 들어간 뒤 Contributors 그래프와 API의 실제 표시를 확인한다. 표시 지연을 기여 완료로 처리하지 않으며 이를 해결하려고 원본 작성자 정보를 바꾸지 않는다. [PR #24 검사](evidence/pr24-merge-preservation-20261031.json) · [남은 확인](usability-todo.md#hm-pr24-integration).

외부 PR의 코드를 검토하고 필요한 수정을 마치면, 검사 대상 커밋·발견한 문제·수정·검사 결과를 GitHub PR 리뷰에 기록하고 `Approve`한다. 그 뒤 merge commit으로 병합한다. 원저자의 기여 범위와 후속 수정은 분리해 남긴다. PR #24는 로컬 검토·수정·검사는 했으나 GitHub 승인 리뷰를 생략한 채 병합했다. 이후 절차에는 이 단계를 포함한다. 자기 계정이 작성한 릴리스 PR은 GitHub에서 자기 승인할 수 없으므로 외부 기여 PR과 구분한다.


## Tauri·웹 폴더 — 2026-10-31

Tauri Mac·Windows는 새 `Desktop/Tauri/`, 순수 웹 배포는 새 `Distribution/Web/`에서 관리한다. 기여자 웹 구현과 후속 버그 수정 3건을 담은 `Web/`·`Sources/Web/`는 수정 금지다. Apple·Android·공용 코어·자원·vendor·기존 빌드/검사 도구도 그대로 읽는다. 기준 main은 `cf3dca50`이며 구현 착수 때 최신 develop의 후속 변경을 확인하고 보호 파일 해시·경로 목록을 기록한다. 생성된 게임 파일 패치나 원본 함수 교체도 하지 않는다. 새 플랫폼 CI와 ignore 항목·문서만 기존 공통 경로에 추가한다. 보호 파일 변경이 필요하면 파일·이유를 먼저 보고한다. 폴더별 README에 소비자·입력·출력을 명시한다. [폴더와 개발 순서](desktop-web-plan-20261031.md).
