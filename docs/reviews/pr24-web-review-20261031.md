# PR #24 웹판 검토

2026-10-31 · [PR #24](https://github.com/gelb2/HeavyMach/pull/24) · 기여자 [fastutlego](https://github.com/fastutlego)

**PR #24를 develop에 병합했다.** 원저자 fastutlego의 19개 커밋을 보존하고 웹 전용 수정 2개를 추가했다. 병합은 `757523e1`, 검사한 최종 코드는 `fdb16df0`다. 두 파일 트리는 같다. 이어 PR #68로 main `91e4c7a2`에 반영했다. 새 버전 태그는 만들지 않았다.

아래 1~10절은 수정 전 `9d2505f7`을 조사한 기록이다. 당시 실패 화면과 재현 결과를 남겼다. **수정·검사·기여 기록은 11절, Tauri/Electron 비교는 12절, 남은 과제와 글감은 13~14절**에 있다.

Android·iOS·native Mac 앱은 유지한다. 웹판은 브라우저에서 같은 코어를 실행하는 추가 플랫폼이다. Tauri·Electron 데스크톱 앱은 아직 만들지 않았다.

## 1. 리베이스 전후와 develop 차이

| 비교 항목 | 확인 결과 |
| --- | --- |
| 현재 develop | `2f82c85214d84c470a3936c452a2d46a524eeb32` |
| 현재 main | `d3faf80a57c6c6e482d2bf5f9b059170c636720e` · develop과 파일 내용 동일 |
| 리베이스 전 PR | `568331251cd275de02474a78912f953b424006b7` |
| 현재 PR | `9d2505f74ad95dc0c24692f2360623401fbe15b2` |
| 현재 대상 | develop. PR에만 19개 커밋, develop에만 있는 커밋 0개 |
| 변경 규모 | 38개 파일, 4,895줄 추가·2줄 삭제 |
| 과거 코드 직접 병합 계산 | 충돌 7개. 게임 소스 충돌은 없음 |
| 현재 코드 직접 병합 계산 | 충돌 0개. main과의 병합 계산도 충돌 0개 |
| 현재 코드 로컬 리베이스 | `HEAD is up to date`. 커밋 SHA 그대로 |
| 기여자의 웹 구현 보존 | 리베이스 전후 `Sources/Web/`·`Web/` 모든 추적 파일 동일 |
| 기존 제품 보존 | develop 대비 `Sources/Core`, `Sources/iOS`, `Sources/Apple`, `Sources/Mac`, `Android`, `Resources`, `Art`, `vendor`, 루트 `CMakeLists.txt` 차이 0개 |

GitHub에는 오늘 **16:30:25 한국 시각에 강제 푸시한 기록 한 번**이 있다. 리베이스 후 마지막 커밋 작성 시각은 **13:42:27**이며 제목은 `docs(web): record the rebase onto upstream v0.17`이다. 커밋에 표시되는 상대 시간과 PR에 실제 푸시한 시각은 다르다.

충돌했던 파일은 `.gitignore`, `AGENTS.md`, `README.md`, `docs/current-status.md`, `docs/design.md`, `docs/document-map.json`, `docs/usability-todo.md`다. 이전 PR과 현재 develop으로 `git merge-tree`를 실행해 재현했다. 현재 PR에서도 같은 검사를 실행하고, 분리한 검토 작업 폴더에서 `git rebase origin/develop`이 아무 변경 없이 끝나는 것을 확인했다.

19개 커밋은 원저자의 이력을 유지하고 있다. `git range-diff`와 파일 대조에서 웹 코드의 누락은 발견하지 못했다. 기존 제품 코드·자원도 보존됐다. 다만 문서 충돌 해결 과정에서 소제목 하나가 잘못 바뀌었다. 아래 5번에서 다룬다.

## 2. 머지 전에 고칠 문제

### P1 · 업그레이드 창을 연 채 전투가 진행되고 버튼이 무시됨

레벨 3·경험치 2,415·2-1 진입 조건의 **검사용 저장**을 만들었다. 업그레이드 창이 표시됐지만 `paused=0`, `inputBlocked=0`이었다. 창 뒤에서 적이 사격하고 플레이어의 체력이 줄었다. 업그레이드 선택과 OK를 눌러도 창이 닫히지 않았다.

`GameSession`은 업그레이드 이벤트에서 정지한다. 그런데 웹 호스트의 `startSlot()`이 이벤트를 받은 다음 `setPaused(false)`를 호출한다. `gameButton()`은 반대로 정지하지 않은 상태에서 업그레이드 버튼을 거절한다. 이 두 조건 때문에 창과 전투 상태가 어긋난다.

- 위치: [WebGameView.cpp 156~159줄](https://github.com/gelb2/HeavyMach/blob/9d2505f74ad95dc0c24692f2360623401fbe15b2/Sources/Web/WebGameView.cpp#L156), [버튼 조건 263~266줄](https://github.com/gelb2/HeavyMach/blob/9d2505f74ad95dc0c24692f2360623401fbe15b2/Sources/Web/WebGameView.cpp#L263).
- Android의 `Runtime.cpp`에는 이미 `player()->upgradeStep!=0`일 때 정지를 유지하는 수정이 있다. 웹 호스트에도 이 정책을 적용하고, 닫힘·백그라운드 복귀·키보드 재개를 함께 검사한다.
- 웹판이 옮겨 온 Apple 코드에도 같은 강제 해제문이 남아 있다. 이번 PR은 Apple 파일을 변경하지 않았다. Apple의 같은 조건 재현은 별도 후속 검사로 남긴다.

### P1 · 저장 읽기 실패 후 새 저장이 기존 진행을 덮어쓸 수 있음

시작할 때 `FS.syncfs(true)`로 IndexedDB의 파일을 가져온다. 이 호출이 실패해도 오류를 콘솔에만 쓰고 게임을 시작한다. 게임은 저장이 없는 상태로 실행되므로 슬롯을 선택하면 새 데이터를 저장한다.

**일시적인 IndexedDB 읽기 실패 한 번을 주입해 재현했다.** 기존 경험치 2,415인 검사용 저장이 있는 상태에서 읽기만 한 번 실패시켰다. 이후 저장 장치는 정상으로 되돌렸다. 게임은 오류 안내 없이 빈 슬롯을 열었고, 슬롯 선택 뒤 새 페이지에서 다시 읽은 저장은 경험치 0·레벨 0이었다. 원래 진행이 새 저장으로 덮였다.

- 위치: [Web/shell.html 308~311줄](https://github.com/gelb2/HeavyMach/blob/9d2505f74ad95dc0c24692f2360623401fbe15b2/Web/shell.html#L308).
- 수정: 초기 읽기 성공 전에는 새 슬롯 생성과 저장을 허용하지 않는다. 읽기 실패는 재시도 화면으로 전달한다. 기존 저장의 읽기 실패와 정말로 저장 파일이 없는 경우를 구분한다.
- 쓰기 실패도 콘솔에만 표시된다. [214~216줄](https://github.com/gelb2/HeavyMach/blob/9d2505f74ad95dc0c24692f2360623401fbe15b2/Web/shell.html#L214). 실패 상태를 유지하고 재시도하며, HUD의 APPLY도 IndexedDB 반영 성공까지 확인하도록 바꾼다.

### P1 · WebGL 컨텍스트를 잃으면 화면 복구 없이 게임이 계속됨

`WEBGL_lose_context`로 그래픽 컨텍스트 손실을 발생시켰다. 화면은 사라졌지만 게임 시간은 계속 증가했고 정지하지 않았다. 복구를 요청해도 `context restoration not allowed`가 발생했으며 `isContextLost()`가 계속 true였다.

`WebMain.cpp`는 시작할 때만 컨텍스트를 만들고, `Web/shell.html`에는 `webglcontextlost`·`webglcontextrestored` 처리가 없다. 렌더러와 이미지 캐시는 사라진 GPU 객체를 계속 유효한 것으로 취급한다.

- 위치: [WebMain.cpp 44~55줄](https://github.com/gelb2/HeavyMach/blob/9d2505f74ad95dc0c24692f2360623401fbe15b2/Sources/Web/WebMain.cpp#L44), [WebMedia.cpp 텍스처 캐시](https://github.com/gelb2/HeavyMach/blob/9d2505f74ad95dc0c24692f2360623401fbe15b2/Sources/Web/WebMedia.cpp#L129).
- 수정: 손실 이벤트에서 복구를 허용하고 전투·입력·음향을 정지한다. 복구 때 셰이더·버퍼·텍스처·캐시를 재생성한다. 현재 게임 세션과 진행 상태를 유지하고 메뉴에서 재개한다.

이 세 문제는 검토용 브라우저 프로필과 합성한 저장으로 검사했다. 아이폰·아이패드·갤럭시의 사용자 저장은 사용하거나 변경하지 않았다.

### 직접 실행한 화면

![정상 1-1 진입 화면](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/pr24-assets/normal-game.png)

![2-1 진입 직후 업그레이드 창 뒤에서 전투가 진행되는 화면](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/pr24-assets/upgrade-stuck.png)

![업그레이드 선택과 OK를 눌러도 창이 남아 있는 화면](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/pr24-assets/upgrade-after-buttons.png)

![WebGL 컨텍스트 손실 후 복구되지 않은 화면](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/pr24-assets/context-lost.png)

### 공용 코어와 클라이언트별 원인

| 항목 | 공용 C++ 코어 | iOS·기존 native Mac | Android | 이번 웹 PR |
| --- | --- | --- | --- | --- |
| 업그레이드 창의 전투 정지 | `GameSession.cpp:170`이 `UpgradeOpen`에서 `world.paused=true`로 설정. 이 경로는 의도대로 정지함 | 공용으로 사용하는 `HMGameView.mm:399`가 이후 `setPaused(false)` 호출. 같은 위험 코드가 존재하나 이번 Apple 앱 재현은 미실시 | 최초 오류는 `Runtime`의 정지 해제·입력 조건에서 발생. 현재는 업그레이드 상태를 유지하도록 수정됨 | `WebGameView.cpp:159`가 정지 해제. 2-1에서 전투 지속·버튼 무반응을 직접 재현 |
| 업그레이드 버튼 | 선택·확정·슬라이드 종료 함수를 제공 | `HMGameView`에서 버튼을 전달 | `Runtime`에서 업그레이드 상태를 확인하고 전달. Resume 우회도 차단 | `gameButton()`이 정지하지 않은 상태의 업그레이드 입력을 거절. 위 강제 해제와 맞물림 |
| IndexedDB 읽기 실패 후 저장 덮어쓰기 | `SaveData`의 형식 처리와 별개. 읽어야 할 파일을 클라이언트가 가져오지 못한 상태 | IndexedDB 사용 안 함 | IndexedDB 사용 안 함 | `shell.html`의 오류 처리 결함. 읽기 실패를 빈 저장으로 취급하고 새 저장을 허용 |
| 저장 쓰기 실패 안내 | 저장할 데이터와 저장 이벤트를 제공 | Apple 파일 저장 경로 | Android 파일 저장 경로 | 비동기 `syncfs` 실패를 콘솔에만 기록. HUD APPLY에도 결과가 전달되지 않음 |
| GPU 컨텍스트 손실·복구 | 그래픽 컨텍스트를 소유하지 않음 | Apple 렌더 호스트 | Android `Media`의 EGL 수명 관리 | WebGL 이벤트·객체 재생성 경로가 빠짐. 손실 뒤 전투 지속·복구 실패 재현 |

**Android와 웹에서 발생한 업그레이드 증상은 각 클라이언트의 상태 연결 문제다. 이번 재현에서 공용 코어를 수정해야 할 근거는 나오지 않았다.** 코어가 정지시킨 뒤 호스트가 이를 덮는 순서를 먼저 바로잡아야 한다. Android 수정이 웹에 자동 반영되지 않은 이유도 `Runtime`과 `WebGameView`가 각각 그 순서를 구현하기 때문이다.

Apple에서 동일한 저장 조건을 따로 검사하고, 실패한다면 Apple 호스트의 같은 위치를 수정하는 과제를 남긴다. 코드에 같은 문장이 있다는 사실과 각 앱에서 실제 증상을 재현했다는 결과를 구분한다.

## 3. 검사 결과와 범위

| 검사 | 결과 |
| --- | --- |
| Emscripten 6.0.13 Release 빌드 | 성공. Wasm 709,803바이트 |
| 자원 | 799개를 4묶음으로 패키징. 총 72,918,871바이트, 각 묶음 20MiB 미만 |
| 기존 빌드 자원 SHA-256 | 1,504개 일치 |
| `browser-smoke.mjs` | 14단계 통과. 메뉴·전투·이동·점프·사격·자동조준·일시정지·저장 후 새 페이지 |
| `hud-editor.mjs` | 11단계 통과. 편집 진입·이동·적용·다시 읽기·변경된 버튼·초기화 |
| `orientation-hold.mjs` | 5단계 통과. 세로 보류·가로 복귀·보류 중 포커스 이탈 |
| 공용 코어 Wasm 검사 | Debug/assert 활성 상태로 빌드, Node에서 24개 모두 통과 |
| 추가 업그레이드·저장 장애·GPU 손실 검사 | 위 3개 결함 재현 |
| 문서 연결 검사 | 통과: 15개 업무 경로·141개 참조 파일 |
| 공개 문서 검사 | 통과: 47개 연결 문서·177개 편집 문서·923개 로컬 링크 |
| 번역 검사 | README 관련 4개 실패 |

직접 실행한 환경은 Mac의 헤드리스 Chrome과 별도 임시 프로필이다. 기여자가 첨부한 iPad·Android·Safari 확인 기록은 기여자의 검사 기록으로 보존한다. 이번 검사에서 실제 휴대폰의 틸트·멀티터치·스피커, Windows WebView2, Tauri 설치본, 모든 미션의 장시간 플레이는 실행하지 않았다.

기본 저장 검사는 파일 크기 3,624바이트를 확인한다. 경험치·부품·미션 값까지 왕복 비교하는 검사를 더해야 한다. 자동조준 검사도 모드 전환을 확인하지만 모든 무기의 명중과 표적 전환을 판정하지는 않는다. 기본 검사가 통과해도 위 업그레이드·저장 장애는 잡히지 않았다.

## 4. 게임 코어와 원작 감각에 미치는 영향

탄도·피격·적 이동·스폰·경험치·저장 형식은 기존 `Sources/Core`를 그대로 컴파일한다. 원작 자원과 신규 유닛 자원도 파일 내용이 같다. 기존 Apple·Android 빌드 입력을 변경하지 않아, 이 PR의 병합만으로 현재 네이티브 앱 코드가 바뀌는 경로는 발견하지 못했다.

웹 호스트는 별도로 구현됐다. 다음 차이는 브라우저 플레이에 영향을 줄 수 있다.

| 영역 | 웹판의 차이와 검토 |
| --- | --- |
| 화면 출력 | GLES 1.1 상태를 WebGL2 셰이더로 구현. CPU로 매 프레임 에셋을 다시 그리는 구조가 아님 |
| 이미지 축소 | 신규 유닛의 CoreGraphics 보간을 면적 평균 필터로 대체. 코드·자원이 같아도 가장자리 픽셀이 완전히 같다는 뜻은 아님 |
| 시간 진행 | 60Hz는 통상 1틱, 낮은 콜백 주파수에서는 최대 4틱 후 한 번 그림. 게임 규칙은 공유하지만 호스트의 실행 간격은 별도 구현 |
| 입력 | Pointer Events·HID 키 변환·DeviceMotion 사용. 모바일 브라우저 엔진과 권한에 따른 검사가 필요 |
| 세로 화면 | 터치 기기의 세로 전환은 보류 후 가로 복귀 시 자동으로 이어짐. 탭 이탈은 메뉴 정지. 제품 정책에 이 구분을 명시해야 함 |
| 저장·생명주기 | 웹의 IndexedDB·비동기 저장·GPU 복구를 따로 처리해야 함. 현재 결함은 이 경계와 메뉴 상태에서 발생 |

코어 공용화가 되어 있어도 호스트의 메뉴·HUD·상태 전환 수정은 플랫폼별로 확인해야 한다. Android에서 고친 업그레이드 오류가 웹판에 남은 것이 실제 사례다. 해결을 위해 Apple 소스를 옮기거나 공용 모듈을 다시 설계할 필요는 없다. 우선 웹 호스트를 수정하고 플랫폼별 같은 조건 검사로 연결한다.

리베이스 위험을 수치 확률로 계산할 근거는 없다. 이번 결과로 구분하면 **기여자의 웹 코드 손실은 발견되지 않았고, 기존 제품 파일 변경도 없으며, 문서 충돌 해결 오류 1건과 웹 런타임 결함 3건이 확인된 상태**다.

## 5. 문서와 빌드 관리에서 보완할 부분

- `docs/current-status.md`의 ‘리센코 인용·해설·현대 연구 구분’ 소제목이 ‘웹판 v0.15 반영’으로 바뀌었다. 기존 원고 작업 본문은 그대로여서 제목을 복구하면 된다. 사용자 README·개인 원고 본문의 삭제는 발견하지 못했다.
- `docs/design.md`와 웹 이식 문서에는 `Sources/Platform` 추출이 앞으로 할 일로 적혀 있다. 현재 정본은 Apple 선행 공통화·폴더 이동 계획을 철회했다. 이 계획을 다시 적용하지 않도록 수정해야 한다.
- 국문 README에는 웹 행과 빌드 명령이 추가됐지만 영문과 번역 기록은 그대로다. `check_translations.py`가 원문 해시·코드·표·링크 차이 4건을 검출했다.
- 웹 문서 앞부분의 3개 묶음·46MB와 뒤의 최신 리베이스 결과가 섞여 있다. 현재 빌드 결과는 4개 묶음·약 72.9MB다. 최신 결과와 과거 측정을 나눠 적는다.
- 외부 기여자의 환경에서 나온 ‘사용자 지시’·배포 경로를 프로젝트 소유자의 지시처럼 읽지 않도록 출처를 분명히 쓴다. 특히 `AGENTS.md`의 작업 브랜치 지시는 받아들일 범위를 정리한다.
- 웹 패키지의 잠금 파일을 `.gitignore`로 제외하고 있다. 재현 가능한 테스트 설치를 위해 잠금 파일을 추적하고 `npm ci`를 사용하도록 제안한다.
- 웹 빌드·기본 검사·추가 회귀를 CI에 연결한다. 현재 GitHub가 표시하는 `MERGEABLE`은 충돌이 없다는 결과이며 기능 검증 결과가 아니다. 조회 시 `mergeStateStatus`는 `UNSTABLE`이었다.

## 6. 브라우저 저장·불러오기

| 데이터 | 저장 위치와 시점 |
| --- | --- |
| 3개 게임 슬롯 | `/persist/HvM.dat` · 기존 바이너리 형식 3,624바이트 |
| HUD 배치 | `/persist/HUDLayoutV1-web.json` · APPLY 때 저장 |
| 키 설정 | localStorage의 `HeavyMach.KeyboardBindingsV1` |
| 자동조준 모드 | localStorage의 `HeavyMach.AimModeV1` |

시작할 때 `/persist`에 IDBFS를 연결하고 IndexedDB → 메모리 파일시스템으로 읽는다. 게임은 그 안의 파일을 읽어 슬롯을 만든다. 저장 때는 메모리 파일에 임시 파일을 쓰고 rename한 다음, `syncfs(false)`로 브라우저의 IndexedDB에 반영한다. 저장 중 새 요청은 다음 동기화 한 번으로 모은다. [Emscripten IDBFS 설명](https://emscripten.org/docs/api_reference/Filesystem-API.html#filesystem-api-idbfs).

게임 진행 저장은 슬롯 선택·진행 갱신·게임오버·메뉴 복귀의 기존 경로를 따른다. 탄환마다 DB를 쓰는 경로는 없다. HUD 편집·키 설정·자동조준 모드 저장은 해당 설정을 바꿀 때 실행한다.

서버 계정이나 클라우드 저장은 없다. **같은 브라우저 프로필·같은 출처에서 다시 열면 저장을 읽는다.** 다른 기기·브라우저·도메인·포트로 자동 전달되지 않는다. 동일 출처에서 경로만 다른 웹판을 둘 열면 `/persist`와 설정 키를 공유할 수 있다. 버전별 별도 저장이 필요하면 이름을 분리해야 한다.

브라우저 사이트 데이터 삭제, 비공개 창 종료, 저장소 정리 정책으로 진행이 사라질 수 있다. 현재 코드는 지속 저장 요청·내보내기·가져오기·여러 탭 동시 쓰기 방지를 제공하지 않는다. 두 탭에서 플레이하면 나중에 저장한 내용이 앞선 진행을 덮을 수 있다. [브라우저 저장 수명](https://developer.mozilla.org/en-US/docs/Web/API/Storage_API/Storage_quotas_and_eviction_criteria).

게임용 장기 저장을 위해서는 먼저 읽기 실패 보호와 쓰기 완료 처리를 고친다. 그다음 저장 내보내기·가져오기, 가능할 때 `navigator.storage.persist()`, 같은 출처의 동시 실행 제한을 추가하는 순서가 적합하다. manifest는 있지만 Service Worker가 없어 오프라인 재실행을 보장하는 구현은 아직 없다.

## 7. 현재 Android는 네이티브인가

**그렇다.** `GameActivity`가 입력·화면·수명주기를 받고, `GameEngine`이 전용 스레드에서 JNI로 C++에 전달한다. `Native.kt`가 `System.loadLibrary("heavymach")`로 공유 라이브러리를 로드한다. Android CMake는 `HeavyMachCore`와 Android 전용 `Bridge`·`Runtime`·`Media`·`Audio`를 묶고 EGL·GLESv1_CM·OpenAL을 연결한다.

화면은 Android Surface에 EGL/GLES로 그린다. WebView나 WebAssembly로 Android 앱을 실행하는 구조가 아니다. 이번 PR의 웹판은 Android Chrome에서 별도로 열 수 있지만, 현재 설치된 APK와는 실행 경로와 저장 위치가 다르다.

## 8. Tauri Mac·Windows와 순수 웹판

| 대상 | 가능 여부 | 추가할 작업 |
| --- | --- | --- |
| 순수 웹브라우저 | 이미 PR에 구현돼 있고 이번 소스 빌드도 실행됨 | 위 3개 결함·저장 보호·배포/캐시·브라우저별 검증 |
| Tauri Mac | 이 웹판을 재사용할 수 있음 | Rust/Tauri 실행기·WKWebView·앱 저장·창 수명·서명·설치 검사 |
| Tauri Windows | 이 웹판을 재사용할 수 있음 | WebView2·앱 저장·설치 패키지·Windows 입력/전체 화면/음향 검사 |
| 기존 native Mac | 현재 별도 구현이 있음 | 유지. Tauri판으로 교체하려면 별도 제품 결정 필요 |

Tauri는 Mac에서 WKWebView, Windows에서 WebView2를 사용한다. 이 경우 게임 C++은 Wasm으로 실행되고 화면은 WebGL2가 그린다. Rust는 앱 창과 운영체제 기능을 담당한다. Catalyst를 사용하지 않는다. [Tauri WebView 구성](https://v2.tauri.app/reference/webview-versions/).

제안하는 구현 순서는 다음과 같다.

1. 웹 결함과 문서를 수정하고 기본·장애 검사를 통과시킨다. PR의 기여자 커밋은 유지하고 수정은 별도 커밋으로 남긴다.
2. `Desktop/Tauri/`에 Tauri 설정·Rust 실행기·플랫폼별 패키지 설정을 둔다. `Sources/Web`·`Web`의 빌드 결과를 그대로 사용하고 게임 코어를 복사하지 않는다. `frontendDist`로 `index.html`, JS, Wasm, 자원 묶음을 앱에 포함한다. [정적 자원 포함 설정](https://v2.tauri.app/reference/config/#frontenddist).
3. 브라우저는 IDBFS를 유지하고 Tauri는 앱 데이터 디렉터리로 저장한다. JS 저장 어댑터가 파일을 읽어 `/persist`에 넣고, 기존 저장 이벤트의 결과만 Rust로 전달한다. 게임 프레임·탄환마다 IPC나 디스크 쓰기를 하지 않는다. 임시 파일→교체·실패 재시도·창 종료 전 저장 완료를 구현한다. [Tauri 파일 접근](https://v2.tauri.app/plugin/file-system/).
4. 웹 자원과 앱 내 Wasm을 허용하는 CSP, 자원 주소, 키보드·전체 화면·창 포커스·오디오 재개를 확인한다. 원격 웹사이트를 그대로 앱에 띄우는 방식보다 자원 포함 빌드를 사용한다.
5. Mac arm64와 Windows에서 각각 빌드·설치·재실행·업데이트 후 저장 유지·업그레이드·GPU 복구·종료를 검사한다. Windows 지원 CPU와 최소 OS는 이 단계에서 정한다. 이번 검토에서는 Tauri 앱을 빌드하지 않았다.

현재 Apple·Android 폴더와 공용 코어를 이동할 필요는 없다. 웹 호스트는 두 Tauri 대상과 순수 브라우저가 공유할 수 있다. 이번 PR은 이 확장에 필요한 게임 실행 부분을 제공한다.

## 9. 기여자 표시와 저자 기록

기여자의 커밋 작성자가 GitHub의 `fastutlego` 계정에 연결돼 있음을 확인했다. 저장소 기본 브랜치는 main이다. **커밋을 보존해 develop에 머지한 뒤 main까지 반영하면 GitHub의 기여자 집계 대상이 된다.** develop에만 머지한 시점에는 저장소 Contributors에 바로 보이지 않을 수 있다. 집계 갱신에는 시간이 걸릴 수 있다. [GitHub Contributors 집계 조건](https://docs.github.com/en/repositories/viewing-activity-and-data-for-your-repository/viewing-a-projects-contributors).

이번 PR에는 일반 merge commit 방식을 권한다. 19개 원저자 커밋과 PR 연결을 그대로 남길 수 있다. Rebase and merge도 작성자 정보는 유지할 수 있지만 커밋 SHA를 바꾼다. 지금은 추가 리베이스가 필요 없다. Squash는 전체 변경을 한 커밋으로 합치므로 이번 기여의 상세 이력을 보존하는 목적에는 이점이 적다. [GitHub 머지 방식](https://docs.github.com/en/pull-requests/reference/pull-request-merges).

자동 목록 외에 README의 기여 항목, 웹 설치 문서, 해당 버전 릴리스 노트에 다음처럼 적으면 기여 범위를 직접 확인할 수 있다.

> 웹(WebAssembly) 버전은 [fastutlego](https://github.com/fastutlego)가 구현했다. 기존 C++ 게임 코어를 브라우저용으로 빌드하고 WebGL2 렌더링, 입력, 음향, 저장과 HUD 편집을 연결했다. [PR #24](https://github.com/gelb2/HeavyMach/pull/24).

나중에 Tauri판을 추가하면 ‘fastutlego의 웹판을 기반으로 Tauri 실행기와 데스크톱 저장·배포를 추가했다’고 역할을 나눠 적는다. 기존 커밋의 공동 작성자 표기도 유지한다. 기여자 목록에 표시되는 것과 저장소 쓰기 권한을 주는 것은 별개다.

## 10. 병합 제안

**수정 후 develop에 원저자 커밋을 보존해 머지하고, 검증된 상태를 develop→main으로 배포하는 순서**를 권한다.

먼저 웹판의 업그레이드 정지·입력, 저장 읽기 실패 보호·쓰기 완료, WebGL 손실·복구를 수정한다. 이어서 문서 소제목·폐기된 공통화 계획·영문 README를 정리한다. 이번 장애 재현을 회귀 검사에 포함하고, 병합 직전에 원격 main·develop과 PR의 최신 SHA를 다시 받아 사용자 웹 수정이 추가됐는지 확인한다.

현재 PR은 그대로 열려 있다. 원격 브랜치·기여자 커밋·릴리스 태그를 변경하지 않았다. Tauri는 이 검토 뒤 진행할 수 있는 별도 개발 과제다.

<a id="fixes-and-merge"></a>

## 11. 수정·검사·병합 결과

2026-10-31 21:15:17 KST, PR #24를 develop에 merge commit `757523e17f5ba54449c34f20f9f0c0690161d40d`로 병합했다. 원저자 19개 커밋 뒤에 `23aa4177`(웹 수정·검사), `fdb16df0`(기여 표기·검사 기록)를 추가했다. squash·강제 푸시·원본 커밋 재작성은 하지 않았다.

### 수정한 동작

| 문제 | 수정 후 |
| --- | --- |
| 업그레이드 창 뒤에서 전투 진행 | 업그레이드 상태의 정지를 유지한다. 선택과 확인이 끝나야 전투를 재개한다. 키보드 Resume도 창을 건너뛰지 못한다. |
| IndexedDB 초기 읽기 실패 | 기존 저장을 읽기 전에는 게임을 시작하지 않는다. Retry 뒤 원래 저장을 다시 읽는다. |
| 쓰기 실패·HUD APPLY의 성급한 종료 | 저장 변경분과 HUD 초안을 유지한다. IndexedDB 쓰기가 성공한 뒤 편집을 닫는다. 실패하면 재시도한다. |
| WebGL 손실 | 전투·입력·음향을 정지하고 GPU 객체를 다시 만든다. 현재 세션을 유지하며 Resume으로 재개한다. 손실된 객체와 JS 핸들도 해제한다. |

수정은 `Sources/Web`·`Web`에 한정했다. 공용 코어는 그대로다. Android에 이미 있던 업그레이드 정지 처리는 수정하지 않았다. Apple에 남은 유사한 강제 해제문은 별도 재현 과제로 등록했다.

### 실행한 검사

| 대상 | 결과 |
| --- | --- |
| 웹 장애 복구 | 19/19. 읽기·쓰기 실패, HUD 저장 완료, 업그레이드, 연속 2회 그래픽 복구 |
| 기존 브라우저 검사 | 기본 플레이 14단계·HUD 11단계·화면 방향 5단계 통과 |
| Wasm 코어 | Debug assertion 검사 24/24 |
| iPad 시뮬레이터 | 입력·저장 검사 30/30 |
| iPhone 시뮬레이터 | 이동·조준·점프·사격·전체 무기·자동조준·이동한 HUD 검사 4/4 |
| Android 에뮬레이터 | 입력·20종 무기 31/31, HUD·EGL 복구 9/9, 캠페인 39/39, 업그레이드 12/12 |
| 빌드 | Apple 일반·치트 시뮬레이터 앱, Android arm64 일반·치트 Debug APK, WebAssembly Release 성공 |
| 자원·기존 플랫폼 | 자원 1,504개 SHA 검사 통과. 보호 파일 2,143개 실제 해시 일치. 머지 후에도 보호 경로 변경 0개 |

이번 iPhone·Galaxy 실기기 검사는 하지 않았다. Android 검사용 저장은 검사 후 원래 바이트로 복원했다. [검사 결과 JSON](https://github.com/gelb2/HeavyMach/blob/develop/docs/evidence/pr24-web-fixes-20261031.json).

![업그레이드 상태에서 정지한 웹판](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/pr24-assets/upgrade-fixed.png)

![두 번째 WebGL 복구 뒤 같은 게임 세션과 메뉴](https://heavymach-dev-report-20261025.hiphopxy.chatgpt.site/pr24-assets/restored-1.png)

### main 반영

PR #68의 코어·Apple·문서·번역·브랜치·비밀 검사 통과 뒤 2026-10-31 21:41:56 KST에 main으로 병합했다. 커밋은 `91e4c7a29bc7391e341aa3344cdcf5f1cd279750`이다. main 전용 변경은 기존 merge commit 3개였고 공통 조상 대비 파일 차이는 없었다. main 트리도 검사한 최종 트리와 같다.

### 기여자와 원본 커밋

PR #24의 GitHub 상태는 `MERGED`다. 변경을 버리고 닫은 PR이 아니다. `Approve`는 리뷰 승인이고 `Merge`는 코드 병합이다. 병합된 PR은 열린 목록에서 빠지지만 커밋은 대상 브랜치에 남는다.

원본 19개 SHA가 모두 develop과 main의 조상으로 남아 있다. GitHub API도 19개 모두를 fastutlego 계정에 연결한다. 동일 SHA는 작성자·시각·메시지·공동 작성자 정보가 바뀌지 않았다는 뜻이다. README·CREDITS·CHANGELOG 국영문에 웹 구현 기여와 PR 링크를 추가했다. 후속 수정은 별도 커밋으로 기록했다.

**GitHub Contributors 통계에서 fastutlego의 19개 기여를 확인했다.** 기본 브랜치 main에 원본 19개 커밋이 포함됐고, 통계 API는 fastutlego 19개·공동 작성자 claude 19개를 반환했다. README·CREDITS·CHANGELOG 국영문 6개도 main에서 확인했다. 저장소 요약 Contributors API는 아직 이전 캐시를 반환했다. 후속 Safari 검사에서 Contributors 화면의 fastutlego — 19 commits와 main 작성자별 목록의 원본 19개를 직접 확인했다.

GitHub는 기본 브랜치 포함 여부와 계정에 연결된 커밋 이메일을 Contributors 조건으로 설명한다. [GitHub 공식 설명](https://docs.github.com/en/repositories/viewing-activity-and-data-for-your-repository/viewing-a-projects-contributors).

[머지·계정 연결 검사](https://github.com/gelb2/HeavyMach/blob/docs/pr24-followup/docs/evidence/pr24-merge-preservation-20261031.json).

<a id="tauri-electron"></a>

## 12. Tauri와 Electron 비교

**HeavyMach에는 Tauri 2로 작은 데스크톱 후보를 먼저 만드는 안을 제안한다.** 웹판의 HTML·Wasm을 그대로 활용하고 창과 파일 저장을 추가하면 되기 때문이다. 기존 iOS·Android·native Mac 앱은 유지한다. Tauri는 Vue 전용이 아니며 HTML·CSS·JavaScript를 내보내는 프런트엔드를 받을 수 있다. [Tauri 프런트엔드 설정](https://v2.tauri.app/start/frontend/).

사용자는 Electron을 잠깐 사용하며 소스 구조·웹뷰 로딩·자원 사용에 불안정을 느꼈다고 했다. 그 프로젝트를 계측한 결과는 없다. 이번 비교에서는 HeavyMach가 어떤 구조와 검사를 필요로 하는지 정리한다.

| 항목 | Tauri 2 | Electron | HeavyMach에서 확인할 것 |
| --- | --- | --- | --- |
| 게임 코드 | 기존 Web 산출물 + Rust 창·저장 계층 | 기존 Web 산출물 + main/preload 계층 | 코어와 게임 로직을 다시 작성할 필요가 없다. |
| 웹 엔진 | Mac WKWebView, Windows WebView2 | 앱에 포함한 Chromium | Tauri는 OS별 엔진 차이, Electron은 선택한 Chromium 버전에서 검사한다. |
| 설치 크기 | 시스템 웹뷰를 활용 | Chromium·Node 런타임 포함 | 동일 게임 자원을 넣은 실제 설치본 크기를 비교한다. |
| 메모리 | Rust·웹뷰·GPU 프로세스와 게임 자원 | main·renderer·GPU 프로세스와 게임 자원 | 부모 프로세스 하나만 비교하지 않는다. |
| 진행 저장 | IndexedDB 또는 Rust를 통한 앱 데이터 파일 | IndexedDB 또는 main의 파일 저장 | 초기 읽기·쓰기 성공 응답·종료·오류 복구를 구현한다. |
| 로딩 | 앱에 포함한 정적 파일·Wasm·자원 | 앱에 포함한 정적 파일·Wasm·자원 | 파일 경로·MIME·초기화 순서·오프라인 실행을 검사한다. |
| 유지보수 | Rust와 OS별 웹뷰 검사 필요 | Node·preload·Chromium 업데이트 필요 | 게임 호스트의 API를 저장·창 기능으로 좁힌다. |

Tauri는 시스템 웹뷰를 사용한다. Electron은 Chromium의 다중 프로세스 구조를 쓰며 main에서 Node API를 제공한다. Electron의 별도 프로세스와 preload는 역할이 있는 구조다. 두 방식 모두 프로세스 간 호출과 초기화 순서를 제대로 정해야 한다. [Tauri 웹뷰](https://v2.tauri.app/reference/webview-versions/), [Electron 프로세스](https://www.electronjs.org/docs/latest/tutorial/process-model), [Electron context isolation](https://www.electronjs.org/docs/latest/tutorial/context-isolation).

### 성능과 로딩 안정성

Tauri의 엔진 미포함은 기본 배포 크기를 줄일 수 있다. 게임 자원 비용까지 없애지는 않는다. 현재 웹 자원 묶음은 72,918,871바이트이며 Wasm 설정의 초기 메모리는 128MiB다. 이 값은 실제 프로세스 메모리 사용량이 아니다. 두 후보 모두 이미지 디코딩·텍스처·음향·게임 메모리를 사용한다.

Tauri로 바꾸면 로딩 오류가 자동으로 사라진다고 볼 수 없다. Wasm과 자원 경로, 초기 저장 읽기, 그래픽 준비, 음향 시작 순서를 맞춰야 한다. Electron은 같은 Chromium 버전을 배포하므로 OS별 엔진 차이를 줄일 수 있다. Tauri는 WKWebView와 WebView2에서 각각 검사해야 한다. 실제 프레임·메모리·시작 시간 비교는 아직 하지 않았다. [Electron 성능 측정 안내](https://www.electronjs.org/docs/latest/tutorial/performance).

Windows의 WebView2 설치 방식도 배포안에 포함한다. 고정 런타임을 함께 넣으면 설치 크기가 달라진다. 인터넷이 없는 새 환경에서도 설치·실행할지에 따라 번들 옵션을 고른다. [Tauri Windows 설치](https://v2.tauri.app/distribute/windows-installer/).

### 저장은 어느 쪽에서도 구현할 수 있다

브라우저 웹판은 이미 IDBFS/IndexedDB에 저장한다. 단순 웹뷰라는 이유로 저장이 불가능한 것은 아니다. Electron도 영속 세션을 제공하며 사용자 지정 partition은 `persist:` 접두어 여부에 따라 디스크 저장과 메모리 세션이 나뉜다. [Electron session](https://www.electronjs.org/docs/latest/api/session).

데스크톱 후보에는 OS 앱 데이터 폴더의 파일 저장을 제안한다. Tauri에서는 Rust, Electron에서는 main이 이를 담당할 수 있다. 저장 포맷과 기존 저장 시점은 유지한다. 초기 읽기를 마치기 전에 게임을 시작하지 않고, 임시 파일 쓰기·교체·백업과 성공 응답을 처리한다. 교체의 원자성·디스크 오류·강제 종료는 OS별로 검사한다. HUD도 파일 저장 성공 뒤 APPLY를 끝낸다. 발사나 프레임마다 파일을 쓰거나 호스트를 호출하지 않는다. [Tauri 파일 시스템](https://v2.tauri.app/plugin/file-system/).

앱 식별자·저장 경로를 고정하고 업데이트 후에도 진행을 읽는다. 웹 저장을 유지한다면 origin·scheme 변경도 검사해야 한다. Tauri 설정 문서는 scheme 변경으로 IndexedDB 등 저장 위치가 달라질 수 있음을 설명한다. [Tauri 설정](https://v2.tauri.app/reference/config/). 다른 기기로 진행을 옮길 때는 저장 내보내기·가져오기를 별도로 제공하는 안이다.

### 순수 웹페이지 버전

PR의 브라우저 실행 경로는 이미 있다. `Web/`에서 Emscripten으로 만든 HTML·JavaScript·Wasm·자원을 정적 호스팅하면 브라우저가 실행한다. 데스크톱 포장은 이 웹판을 재사용하는 후속 작업이다. 기여자의 fastutle.com 배포 기록과 이번에 수정한 빌드는 구분한다. 이번 수정본을 그 사이트에 재배포하지는 않았다.

공식 웹 배포를 준비할 때는 다음 항목을 정한다.

- 같은 버전의 HTML·JavaScript·Wasm·자원만 함께 배포한다. Wasm의 `application/wasm` MIME, 상대 자원 경로, 압축·캐시 헤더를 확인하고 이전 HTML이 새 Wasm을 잘못 읽지 않게 한다.
- HTTPS의 고정 도메인·경로를 사용한다. 저장은 IndexedDB/IDBFS에 두며 초기 `syncfs(true)`가 끝나야 실행한다. 쓰기 완료와 실패 처리는 이번 수정본에 들어 있다. [Emscripten 파일 시스템](https://emscripten.org/docs/api_reference/Filesystem-API.html).
- 브라우저 데이터 삭제·비공개 모드·저장 공간 정리와 origin 변경에서 진행이 어떻게 되는지 검사한다. 저장 내보내기·가져오기와 영속 저장 요청을 검토한다. 브라우저 저장은 계정 동기화가 아니며 서버 없이 다른 기기에 자동으로 이어지지는 않는다. [브라우저 저장 정책](https://developer.mozilla.org/en-US/docs/Web/API/Storage_API/Storage_quotas_and_eviction_criteria).
- 오프라인 재실행이 필요하면 Service Worker 캐시와 업데이트 정책을 따로 설계한다. PWA는 설치·오프라인 선택지이며 웹판 실행의 필수 조건은 아니다.
- Safari·Chromium·실제 모바일에서 멀티터치·틸트 권한·음향 시작·가로 전환·전체 화면·백그라운드 복귀·GPU 복구를 검사한다.

### 후보 구현과 비교 순서

1. `develop`에서 별도 브랜치를 만들고 `Desktop/Tauri/`에 창·설정·Rust 저장 코드를 둔다. 기존 `Web/` 산출물을 앱에 포함해 원격 사이트 없이 실행한다. Apple·Android·코어는 수정하지 않는다.
2. 같은 Web 산출물로 Electron 비교 앱을 만들 경우 `Desktop/Electron/`에 분리한다. main·preload는 창·저장만 담당한다. Node API를 게임 페이지 전체에 노출하지 않는다.
3. 동일 Wasm·자원 SHA, 화면 크기·배율, 무기·적 조건을 고정한다. 시작 10회, 전체 프로세스 메모리, 프레임 간격의 중앙값·p95·p99, 30분 전투를 비교한다.
4. 그래픽 손실·복구, 키보드·포커스·음향·절전·전체 화면, 쓰기 실패·강제 종료·업데이트 후 저장을 검사한다. Mac과 실제 Windows 환경의 결과를 각각 남긴다.
5. 측정 결과로 배포 후보를 정한다. Windows 설치본과 Mac 앱의 서명·업데이트는 해당 후보의 배포 작업으로 진행한다.

위 순서는 후속 계획이다. 아직 Tauri·Electron 앱이나 Windows 설치본은 만들지 않았다.

<a id="followup-todo"></a>

## 13. 남은 개발 과제와 기여 보존

- main 병합과 원본 19개 커밋 보존은 완료했다. 이후에도 외부 PR의 검토 결과·Approve 리뷰를 남기고 merge commit으로 병합한다. 이번 PR의 승인 리뷰는 생략했다.
- main의 GitHub 계정 연결과 README·CREDITS·CHANGELOG 표기는 확인했다. Contributors 통계도 fastutlego 19개를 확인했다. 요약 목록의 캐시 갱신과 다음 버전 릴리스 노트는 후속 확인한다.
- 실제 모바일 브라우저의 틸트·멀티터치·음향·저장·복귀를 검사한다.
- Apple의 미사용 경험치 미션 진입은 별도 재현한다. 이번 웹 수정으로 Apple 코드가 바뀌지는 않았다.
- Tauri 후보와 Electron 비교는 12절의 저장·로딩·성능·배포 조건으로 수행한다.

[개발 TODO](https://github.com/gelb2/HeavyMach/blob/docs/pr24-followup/docs/usability-todo.md#hm-pr24-integration)에 완료와 남은 항목을 나눴다.

<a id="first-pr-story"></a>

## 14. 첫 외부 PR에 관한 글감

사용자가 이번 경험을 별도 글감으로 지정했다. 처음 받은 외부 PR에 다른 사람이 투자한 시간과 기술적 성취, 기존 Android·iOS 네이티브 앱을 유지할지 고민한 과정, 웹판에서 Windows·Mac 지원 가능성을 발견한 순서를 담는다. 웹 게임으로 복원할 생각은 이 PR을 보기 전에는 해본 적이 없었다는 원문도 보존했다.

후속 지시에 따라 [첫 PR을 받다. 그 다음 이어진 스케일업, 그리고 타우리](../public/first-pr-and-tauri-20261031.md)를 작성했다. 당시 고민을 개발자 1인칭으로 쓰고 승인 리뷰 생략으로 화가 난 경험도 같은 글에 담았다. 원저자의 웹 구현·Codex의 후속 수정·실제 병합 결과를 구분했다. SaaS 담당 세션의 Apps·Mags 집필 자료에도 연결했다.

[사용자 원문](https://github.com/gelb2/HeavyMach/blob/docs/pr24-followup/docs/references/developer-background/user-first-pr-20261031.md) · [글감 TODO](https://github.com/gelb2/HeavyMach/blob/docs/pr24-followup/docs/usability-todo.md#hm-first-pr-story) · [SaaS 집필 안내](https://github.com/gelb2/HeavyMach/blob/docs/pr24-followup/docs/public/publication-inventory.md).
