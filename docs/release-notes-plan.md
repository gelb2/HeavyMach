# 버전별 릴리스 노트와 GitHub 공개 계획

2026-10-29. [등록 과제](usability-todo.md#hm-release-notes--버전별-릴리스-노트와-github-공개-절차--2026-10-29).

## 현재 구성

GitHub Releases에는 v0.9~v0.17의 릴리스 노트가 있다. v0.12~v0.17에는 소스·자원 압축본, 고지 묶음과 SHA256SUMS가 있다. README는 최신 태그와 Releases를 연결하고, CHANGELOG는 버전별 변경을 기록한다. 이 구성을 이어간다.

## 원고 위치와 내용

`CHANGELOG.md`의 버전별 절을 릴리스 노트의 원본으로 쓴다. GitHub Releases에는 그 절의 본문을 옮기고 설치 링크·첨부 파일 설명·이전 버전 비교 링크를 붙인다. 같은 내용을 여러 원고 파일에서 따로 고치지 않는다.

버전별로 다음 내용을 실제 변경에 맞춰 적는다.

- 플레이어가 보게 되는 기능 추가와 수정. 발생 조건과 수정 후 동작을 짧게 설명한다.
- 지원 플랫폼·설치 방법·저장 호환성이 바뀌었으면 그 영향.
- 실행한 기기·검사와 결과. 코어·시뮬레이터·실기기·CI 빌드를 구분한다.
- 아직 실패하는 검사와 남은 문제. 해당 TODO·근거 문서로 연결한다.
- 필요한 게임 화면 또는 비교 이미지. 화면 변화가 없는 수정에는 이미지를 강제하지 않는다.

README에는 최신 버전, Releases 링크, CHANGELOG 링크를 둔다. 긴 릴리스 본문은 README에 반복하지 않는다. 상세 검사와 회고는 기존 문서로 연결한다. 작성자의 소개·개발자의 변은 버전 갱신 과정에서 재작성하지 않는다.

## 작성과 게시 순서

1. 작업 브랜치에서 이전 태그 이후 코드·문서·PR과 실행 근거를 대조해 CHANGELOG를 작성한다. main의 사용자 직접 수정도 fetch해 확인한다.
2. develop으로 병합한 뒤 main에 병합한다. main 병합 커밋에 새 주석 태그를 붙이고 원격 태그 대상을 확인한다. 기존 태그는 이동하지 않는다.
3. 해당 태그의 추적 파일과 실제 LFS 자원으로 압축본을 만든다. 새 폴더에 풀어 자원 해시·문서 링크와 LFS 포인터 잔존 여부를 확인한다. 고지 묶음과 SHA256SUMS도 만든다.
4. 태그를 지정해 GitHub Release 초안을 만든다. CHANGELOG 본문의 상대 링크는 그 태그에 고정된 GitHub 링크로 바꾼다. 첨부 파일·태그 대상·원고·검사 결과가 맞으면 게시한다.
5. 공개된 Release 본문과 첨부 파일 해시를 대조한다. README 최신 버전 링크, 이전 버전 비교 링크와 다운로드 접근도 확인한다. 배포 커밋·태그·Release 주소는 현재 상태 문서에 기록한다.

태그 버전은 현재 Git 배포 버전이다. 앱 번들 버전은 별도로 관리해 왔으므로 릴리스 노트에서 두 값을 혼동하지 않는다. 설치한 앱과 태그의 제품 소스·자원이 같다는 근거도 남긴다.

## 릴리스 도구

[tools/release.py](../tools/release.py)는 Python 3.12 이상, Git, Git LFS, GitHub CLI를 사용한다. `gh auth login`으로 게시할 저장소에 로그인한다. 태그의 파일과 LFS 자원으로 압축본을 만들므로 작업 폴더의 미커밋 수정과 미추적 파일은 포함되지 않는다.

다음 예시는 새 버전을 v0.17로 정했을 때의 명령이다. develop→main 병합과 주석 태그 push를 먼저 마친다.

```sh
git fetch origin main develop --tags
git lfs fetch origin v0.17
python3 tools/release.py prepare v0.17
python3 tools/release.py publish v0.17 --check
python3 tools/release.py publish v0.17
```

`prepare`는 주석 태그·main 포함 여부·병합 커밋을 확인하고 **태그에 저장된 CHANGELOG**에서 해당 버전 본문을 읽는다. 상대 문서 링크를 그 태그의 GitHub 주소로 바꾸고 파일 존재를 검사한다. 설치 안내·이전 태그 비교 링크도 붙인다. 따라서 CHANGELOG는 새 태그를 붙이기 전에 작성한다.

출력은 `build/releases/v0.17/`에 생긴다. 같은 폴더가 있으면 덮어쓰지 않는다. 다시 준비할 때는 `--output build/releases/v0.17-recheck`처럼 새 경로를 지정하고, 게시 명령에도 같은 경로를 전달한다.

| 파일 | 내용 |
|---|---|
| `release-notes.md` | GitHub Release 본문 |
| `HeavyMach-v0.17-source-with-assets.tar.gz` | 태그의 추적 파일과 실제 LFS 자원 |
| `HeavyMach-v0.17-notices.zip` | LICENSE·적용 범위·자원 고지·크레딧·외부 라이선스 |
| `SHA256SUMS.txt` | 두 압축본의 SHA-256 |
| `manifest.json` | 태그 커밋·출력 해시·파일 수·자원 검사 결과 |
| `publish-result.json` | 게시 단계·Release ID·공개 결과·실패 이유 |

압축 후 새 폴더에 풀어 추적 파일 전체의 내용·권한·심볼릭 링크를 대조한다. 빌드 자원은 `docs/build-resources.json`의 수량과 해시로 검사한다. LFS 파일이 없거나 내용이 다르면 압축본을 완료 처리하지 않는다.

`publish --check`는 로컬 해시·원격 태그·기존 Release를 읽는다. `publish`는 초안을 만들고 압축본 2개와 SHA256SUMS를 업로드한 뒤 원고·파일명·크기·서버 SHA-256·태그를 대조한다. 모두 맞으면 공개하고 최신 릴리스로 지정한다. 공개 후 한 번 더 대조해 URL을 기록한다. GitHub의 [첨부 파일 API](https://docs.github.com/en/rest/releases/assets)와 [릴리스 편집 명령](https://cli.github.com/manual/gh_release_edit)을 사용한다.

## 실패 후 재개

업로드가 중단되면 초안과 `publish-result.json`이 남는다. 같은 폴더로 재개하면 이미 올라간 정상 파일은 유지하고 빠진 파일만 올린다.

```sh
python3 tools/release.py publish v0.17 --resume
```

태그·원고·기존 첨부가 바뀌었거나 다른 작업이 만든 초안이면 재개를 중단한다. GitHub의 해당 초안과 `publish-result.json`의 실패 단계를 확인한다. 업로드 중 불완전한 파일이 생겼으면 해당 파일만 초안에서 제거한 뒤 같은 명령으로 재개한다. 공개된 Release는 이 도구로 수정하지 않는다.

게시 요청 뒤 연결이 끊기면 `published`는 `null`로 남는다. 해당 버전의 GitHub Release 공개 상태를 먼저 확인한다. 이후 `publish`를 다시 실행해도 이미 공개된 버전은 변경되지 않는다.

## 검사 결과 · 2026-10-29

v0.16 태그로 실제 압축본을 만들고 다시 풀어 **추적 파일 2,672개·빌드 자원 1,504개**를 대조했다. 원고 링크와 고지 묶음도 통과했다. v0.16의 태그 당시 CHANGELOG를 추출했으며, 이후 보강한 공개 릴리스 본문은 그대로 보존했다.

임시 Git 저장소와 메모리 내 GitHub 응답으로 17개 검사를 실행했다. 태그 밖 파일 제외, LFS 누락·변조, 원고·첨부 변조, 원격 태그 변경, 기존 공개본 보호, 업로드 실패·재개, 게시 응답 유실을 확인했다. 같은 검사를 문서 CI에 추가했다.

```sh
python3 -m unittest discover -s tools/tests -p 'test_release.py'
```

실제 GitHub에는 v0.16의 게시 사전 검사를 실행해 기존 공개본 수정 차단을 확인했다. 새 Release의 업로드·공개 실검사는 다음 버전 배포 때 수행한다. [도구 검사 기록](evidence/release-tool-20261029.json), [기존 8개 릴리스 정리 기록](evidence/release-history-20261029.json).

## v0.17 실제 게시 — 2026-10-30

`prepare`로 추적 파일 2,786개·자원 1,504개를 압축하고 재추출 대조했다. `publish` 초안 생성 직후 목록 조회가 비어 한 차례 멈췄다. 생성된 초안의 태그·제목·본문·빈 첨부 목록을 대조하고 해당 ID를 실행 기록에 복구한 뒤 `--resume`으로 완료했다. 첨부 3개와 공개 본문·태그·서버 SHA-256을 대조했다. [v0.17 Release](https://github.com/gelb2/HeavyMach/releases/tag/v0.17) · [검사 기록](evidence/release-v0.17-20261030.json).

## 웹·PWA·Tauri Mac 릴리스 문서 — 2026-11-01

README 국영문에서 언어 선택 바로 아래에 `Quick Play — PWA · Mac`을 둔다. 웹 실행·PWA 설치 링크, Tauri Mac 안내, 네이티브 빌드 절 연결과 fastutlego의 PR #24 기여·감사를 함께 표시한다. 아래에는 웹/PWA와 Tauri Mac의 별도 기능 설명을 둔다. 기존 개발자의 변 원문은 유지한다.

릴리스 전 작업:

1. Tauri Mac 설치본을 실제 Release에 첨부하고 해시·접근을 확인한 뒤, 상단의 준비 중 문장을 PWA와 Tauri Mac 권장 문장으로 바꾸고 직접 다운로드 링크를 연결한다. 첨부하지 않은 설치 파일의 주소를 만들지 않는다.
2. 국영문 설치법에 브라우저 실행, iPhone·Android PWA 설치, 최초 다운로드·오프라인·업데이트, Tauri Mac 설치와 기존 네이티브 앱의 차이를 적는다.
3. 저장·복원 안내에 세이브 슬롯의 코드 생성·입력, 플랫폼 간 이동, 웹 저장과 Tauri OS 백업의 차이를 적는다.
4. CHANGELOG 국영문을 원본으로 Releases를 작성한다. PR #24 기여와 후속 실행기·복원 모듈 개발을 구분하고, 실제 검사·남은 과제·다운로드를 연결한다.
5. 문서 목차·플랫폼 표·SaaS 집필 안내·기존 보고 사이트를 같은 출시 상태로 갱신한다. 개인정보나 비공개 프로젝트 조사 자료는 옮기지 않는다.
6. 공개 Pages의 최신 버전을 확인한 뒤 새 PWA 설치와 기존 설치 업데이트, 코드 복원·저장·종료·오프라인 재실행을 검사한다. main 병합과 Pages 배포 결과를 따로 확인한다.

문서 검사 `tools/check_publication.py`는 새 상단 배치와 실행·기여자 링크를 확인하도록 수정했다. 기존 승인 원고 보존 검사는 유지한다. 이 문서 검사 파일만 보호 목록의 예외 사유에 추가했으며 게임 소스·빌드 도구는 이번 문서 작업에서 수정하지 않는다.
