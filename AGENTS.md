# HeavyMach 작업 지침

이 저장소의 `.agents/policy.md`와 `.agents/project.md`를 따른다. 작업 전에 [정본](docs/canonical.md), [문서 위계](docs/document-governance.md), [게임 기획](docs/game-design.md), [복원 계약](docs/restoration-contract.md), [현재 상태](docs/current-status.md), [설계](docs/design.md)를 반드시 읽는다. 수정 영역에 따라 [좌표계](docs/coordinate-policy.md), [그래픽 렌더링](docs/rendering-policy.md), [데미지·수치 계산](docs/game-calculation-policy.md), [시간·저장·음향](docs/runtime-policy.md)을 읽고 실제 소비 코드와 대조한다. 이 저장소는 HeavyMach 1.13의 소스 복원 범위다. 원본 호환 실행판은 `Compatibility/`이며 Unity는 별도 저장소다.

- 최신 사용자 확인을 [현재 상태](docs/current-status.md)에 반영한다. 정적 분석, 추정, 에이전트 직접 실행, 사용자 보고를 구분한다.
- REA 함수 주소·입력 SHA·Evidence 또는 원본 출력 경로를 복원 코드와 연결한다. 디컴파일 문장을 원본 소스라고 부르지 않는다. ARM ABI 때문에 어긋난 타입·인수는 어셈블리와 호출부로 확인한다.
- 원본 IPA·추출 자원·사용자 저장을 수정하지 않는다. 변환은 별도 파생 파일로 만들고 원본 해시와 변환 방식을 기록한다. 원본 IPA·캐시·빌드·서명 비밀은 Git에 추가하지 않는다. 앱 빌드·검사에 필요한 준비된 자원은 Git/LFS로 관리하며 `docs/build-resources.json`의 SHA-256으로 확인한다.
- 화면 배치·에셋·진행·충돌·이동·사격·아이템·경험치·저장 규칙을 보존한다. 2026-10-24 최신 사용자 지시에 따라 별도 검증 앱에서 EXP·레벨·무기·부품·미션·시작 위치를 설정하는 치트 검증을 허용한다. 설정값과 실제 실행 결과를 구분한다. 판정 강제·실패 기준 완화로 통과를 만들지 않는다. 기존 사용자 저장과 일반 앱 규칙은 보존한다.
- Assault 증거는 보조 자료다. 원작과 차이가 있는 데이터·결제·터치 규칙을 원작에 섞지 않는다.
- 기존 유효한 증거는 재사용한다. 새 소스 빌드의 성공을 기존 호환 실행기의 성공으로 대신하지 않는다. 테스트·서명·설치·플레이·배포는 따로 보고한다.
- 다른 작업자의 변경을 되돌리지 않는다. 독립 저장소·원격·배포가 만들어졌다고 추정하지 않는다. 기존 Git 훅과 LFS를 유지한다.

- 정책을 읽지 않은 채 별도 좌표계·렌더 경로·수식·저장 시점을 만들지 않는다. 구현 변경은 원본 근거, 관련 정책, 회귀 검사, 현재 상태를 함께 갱신한다. 계획과 구현 완료를 구분한다. 후속 사용성 과제는 [TODO](docs/usability-todo.md), 화면 확장은 [구현 계획](docs/wide-canvas-plan.md)을 따른다.
- 치트 빌드의 무기 검증은 [전체 무기 절차](docs/weapon-validation-procedure.md)를 따른다. 실제 데이터의 모든 플레이어 무기와 필살기3종을 열거하고 무기별 검사 결과·미검사 범위를 남긴다. 기본 무기 일부 사격을 전체 무기 정상으로 보고하지 않는다.

- 사용자 지시(2026-10-25): 작업 보고는 Sites 스킬로 보고 사이트에 게시하고 링크를 제공한다. 기존 HeavyMach 보고 사이트를 갱신하며 정본 문서·TODO도 함께 유지한다. 사이트 식별자와 업데이트 경로는 [보고 사이트](docs/report-site.md)를 따른다.

- 문서·코드·앱 기능은 서로 모순 없이 설명되어야 한다. [업무 연결표](docs/document-map.json)의 관련 문서·소스·검사를 읽고 요구→정책→소비 코드→검사/앱 관측을 대조한다. `python3 tools/document_contract.py plan --task <업무>` 또는 `plan --files <경로>`는 읽기 목록을 출력한다. 실제 읽기나 검사를 대신하지 않는다.
- HeavyMach 변경을 마감할 때 `python3 tools/document_contract.py check`로 구조 연결을 검사한다. 상위 요구가 바뀌면 기획→설계/정책→코드/검사를 함께 반영하고 최신 결과를 current-status·TODO·보고에 갱신한다. 의미 일치와 실제 기능은 관련 실행 증거로 확인한다. 문서만의 변경에 게임 재빌드나 실기기 재검사를 요구하지 않는다.

- 보고서·GitHub 문서·게임 안의 텍스트·진행/최종 답변은 `.agents/skills/write-project-text/SKILL.md`를 먼저 읽고 적용한다. 보고 사이트와 다운로드는 같은 원고에서 만들며 최신 상태와 과거 계측 시점을 구분한다.

- 개발 브랜치는 develop에 병합하고 main에는 develop만 병합한다. v0.9 등 릴리스 태그는 main의 병합 커밋에 붙이며 재지정하지 않는다. [저장소 관리](docs/repository-management.md)를 따른다.

- Android 개발은 기존 Apple 소스·프로젝트·공용 코어·자원·vendor·Apple 빌드 도구를 이동하거나 수정하지 않는다. Android 전용 구현·빌드·검사는 `Android/` 아래에 추가하고 기존 코어·자원은 읽기 입력으로 참조한다. Shared·Apple 폴더 이동과 Apple 선행 공통화 제안은 철회했다. [변경 범위](docs/repository-management.md#플랫폼별-폴더와-공용-모듈--2026-10-30)를 따르고 보호 파일의 작업 전후 해시를 비교한다.

- HeavyMach Android 이식·APK 작업에는 `.agents/skills/port-heavymach-android/SKILL.md`와 `docs/android-port-plan-20261026.md`를 읽고 `document_contract.py plan --task android`로 연결을 확인한다. 조사·예정 단계와 실제 APK 완료를 구분한다.

- 개발자의 변·README 소개·GitHub 회고·SaaS Apps·Mags 작성은 `python3 tools/document_contract.py plan --task developer_story`의 자료를 읽는다. [개발 동기 원문과 작성 순서](docs/developer-background.md)를 따르고, 기술 학습 목적만으로 개인적인 동기를 대신하지 않는다. 받지 않은 다음 문단을 추측해 쓰지 않는다.

- SaaS 담당 세션에 집필 자료를 전달할 때는 [집필용 문서 안내](docs/public/publication-inventory.md)와 developer_story 경로를 사용한다. Apps·Mags 본문 작성은 SaaS 세션의 업무다. 플레이 영상·README 이미지 작업은 [영상 작업 절차](docs/media-publication.md)와 document-map의 media 업무를 읽고 원본·타임코드·파생 파일을 연결한다.

- Apple 플랫폼·키보드 작업은 `docs/apple-platforms.md`, `docs/keyboard-input-policy.md`와 `.agents/skills/game-keyboard-input/SKILL.md`를 읽는다. 앱별 소스 복사를 추가하지 않으며 Catalyst를 사용하지 않는다. iPad 터치·기울기와 키보드를 함께 유지한다.

- iPhone·iPad 무선 설치·디버깅에는 `.agents/skills/apple-wireless-device/SKILL.md`와 `docs/wireless-device-workflow.md`를 읽는다. 실제 transport와 스킴을 확인하고 일반 저장을 보존한다. 설치·실행·자체 검사 후에는 결과를 저장하고 대상 앱을 종료한 뒤 기기 홈 화면 복귀를 확인한다. 사용자가 앱을 켜 두라고 지시하면 그 지시를 따른다.

- 사용자 지시(2026-10-27): 자동조준은 develop에서 분기한 `feat/auto-aim-ipad-validation`에서 개발한다. iPad 치트 앱에서 이동·점프·사격·무기/표적 전환을 검증한다. 충분히 완성하기 전에는 develop 또는 main에 병합하지 않는다. 후속 사용자 지시에 따라 정식 앱에도 자동조준을 제공한다. 기본값은 OFF이며 A로 켜고 끈다. 정식 앱의 치트 기능은 계속 비활성화한다.

- 사용자 후속 지시(2026-10-27): C 명중 보정 기능은 삭제한다. 자동조준에서도 실제 탄속·분산·비행 경로를 덮어쓰지 않는다. 수동 사격은 자동조준 추가 전 코드와 동일 입력·난수로 대조한다.

- 사용자 후속 지시(2026-10-27): 위 자동조준 미병합 지시 이후 develop→main 병합과 버전 태그를 요청했다. `v0.11`로 원격 반영 완료. 신규 F-22·Su-57·MiG-31 작업은 `feat/lux-lunae-fighters`와 `docs/lux-lunae-plan-20261027.md`를 따른다.

- 사용자 지시(2026-10-29): main 병합 직전에 원격 main·develop을 fetch하고 main에서 사용자가 직접 수정한 문서·커밋을 확인한다. 공통 조상 이후 양쪽 변경과 병합 후보의 최종 본문을 대조한다. Git 충돌이 없어도 최신 사용자 문구가 덮이지 않았는지 확인한다. 충돌은 최신 사용자 원문을 보존해 해결하며 ours/theirs 일괄 선택으로 처리하지 않는다. 상세 절차는 `docs/repository-management.md`의 main 직접 수정 문서 보존 항목을 따른다.

- 웹 기여(PR #24, fastutlego): HeavyMach 웹(WebAssembly) 이식은 `porting/wasm`에서 진행한다. `docs/web-port-20261028.md`와 `document_contract.py plan --task web`을 읽는다. 웹 빌드는 `Sources/Core`를 그대로 쓰고 `Sources/Web`·`Web/`만 추가한다. 헤드리스 자동 검사와 실제 휴대폰 확인을 구분한다.

- 사용자 지시(2026-10-31): Tauri Mac·순수 웹 배포·Tauri Windows 개발에서는 기여자 웹 구현과 후속 수정 3건(업그레이드 정지·저장 복구·GPU 복구)을 포함한 `Web/`·`Sources/Web/` 및 기존 Apple·Android·공용 코어·자원·빌드/검사 도구를 수정하지 않는다. 새 실행기·백업은 `Desktop/Tauri/`, 새 웹 배포는 `Distribution/Web/`에서 구현한다. 기존 웹 저장을 바꾸지 않고 별도 OS 백업·시작 전 복구를 연결한다. 생성된 HTML/JS 패치·원본 함수 재정의로 이 조건을 우회하지 않는다. 보호 파일의 내용·추가/삭제·경로와 포함한 게임 산출물 해시를 검사한다. 수정이 필요하면 정확한 파일과 이유를 먼저 보고한다. [개발 계획](docs/desktop-web-plan-20261031.md)을 따른다.
