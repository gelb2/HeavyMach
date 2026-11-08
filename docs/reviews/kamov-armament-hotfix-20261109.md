# 카모프 기관포 제거와 미사일 2연사

2026-11-09 · `fix/kamov-missile-only-20261109` · 기준 develop `6a143380`.

카모프의 기관포 파츠·공격을 제거하고, 미사일을 2발 연속 발사하도록 수정했다. 채택한 몸체·두 로터·미사일포드와 캠페인 배치는 유지했다. 웹 치트빌드에서 실제 발사·피해·파괴를 검사했다.

![기관포가 없는 카모프의 미사일 2발](../public/media/kamov-armament-hotfix-20261109/Ka52-two-missiles.png)

한 공격에서 발사된 두 미사일이 실제 게임에 함께 보이는 장면이다. 설정한 간격은 0.5초이며, 60Hz 게임 갱신에서 관측한 간격은 0.5~0.517초다. 이 간격은 기존 2연사 미사일의 값을 참고한 구현 선택이다.

## 변경한 경로

요구 ID는 `HM-KAMOV-ARMAMENT`다. 기존 authoring JSON에서 기관포의 무기·파츠·조립 연결과 사용하지 않는 몸체 소켓을 제거했다. 미사일의 소켓 번호는 4에서 3으로 바뀌지만 실제 연결 좌표 `(5,-10)`는 같다. 미사일은 `shotCnt=2`, `shotDly=0.5`만 바꾸고 피해25·탄속160·재장전 지연5 등 나머지 수치는 유지했다.

기존 exporter로 MCH를 다시 만들고 기존 파서·공통 파츠·발사 함수가 읽는다. 카모프 전용 C++ 사격·그리기·AI 분기는 추가하지 않았다. 검증 빌드에서만 실제 탄환의 `startTime`을 읽도록 기존 관측 API에 필드 하나를 더했다. 이 값으로 발사 시각을 중복 없이 세며 탄도·피해·판정은 변경하지 않는다.

bake 목록과 Apple·Android·Web의 실행용 기관포 PNG를 제거했다. 원작 기관포 그림은 보존했다. 현재 신규 두 기체용 실행 이미지는 5종×3프로필이며, 이전 이미지를 합치면 프로필별68개다. 원본 그림·음향·RawData와 다른 기체의 MCH 레코드·순서, MAP43개·HMEX는 이전과 바이트가 같다. [보존 검사](../evidence/kamov-armament-hotfix-20261109/preservation.json)

## 실행 검사

| 검사 | 확인한 결과 |
|---|---|
| 수정 전 회귀 검사 | 기존 데이터에 기관포가 남아 있다는 assertion으로 실패 |
| 코어 | 28/28 통과. 실제 공통 AI가 미사일10발을 5묶음으로 발사. 묶음마다2발·0.5~0.517초와 재공격 대기 확인 |
| 웹 카모프 | 3회 공격에서 각2발. 발사 시각 5.2833/5.8, 13.6833/14.2, 23.0667/23.5667. 실제 피해25, 기관포 탄환·그림 없음 |
| 로터·행동 | 두 로터 각각 프레임0/1, 이동·멈춤·플레이어 사격에 의한 HP0·파괴 확인 |
| 자연 캠페인 | Stage01의 기존 MAP 생성 카모프에서 2연사·기관포 부재·피해781→756 확인 |
| 함께 검사한 기능 | 유로파이터의 기존 외관·무장·피해·파괴와 플레이어20무기의 실제 발사·소모 확인 |
| 저장·일반판 | 치트 저장 분리, 일반 웹 빌드에서 치트 API 없음. 일반판15단계와 IndexedDB 3,624바이트 저장 복원 통과 |
| 이미지 | 5종×3프로필의 재생성·디코드·업로드15회 검증. 남은 PNG204개는 이전과 바이트 동일 |

웹 검사는 Mac Chrome의 WebGL2/WebAssembly에서 실행했다. 기체 생성 검사는 별도 치트 fixture, Stage01은 실제 MAP 생성이다. 무적·탄약 보충·낙하 복귀가 전투 결과에 개입하지 않았음을 관측했다. 휴대폰 실기기나 Android·Tauri 앱을 새로 설치한 결과로 표현하지 않는다.

![원작 헬기와 기관포를 제거한 카모프](../public/media/kamov-armament-hotfix-20261109/helicopter-family-same-scale.png)

![자연 캠페인에서 생성된 카모프](../public/media/kamov-armament-hotfix-20261109/Stage01-Ka52.png)

## 실패와 수정 기록

첫 웹 실행은 빌드 폴더 URL에서 `/dist/`가 빠져 타이틀 대기 시간 초과로 끝났다. 실제 게임 주소로 고쳐 재실행했다. 최초 코어 검사는 제거된 무기·파츠 수를 기대하던 기존 총수 검사와, 20초 동안 세 번의 공격을 기다리던 관측에서 실패했다. 새 패키지의 실제 수를 반영하고 공격 관측을40초로 늘렸다. 2발·간격·피해 조건은 완화하지 않았다.

일반판 smoke의 첫 재접속은 게임 시계6초 전에 탭해 타이틀 인트로 건너뛰기로 소비됐다. 이를 직접 재현하고 벽시계 대기를 실제 게임 시계6.2초 대기로 바꾼 뒤15단계를 통과했다. 제품 입력 코드를 바꿔 통과시킨 것이 아니다. [재현](../evidence/kamov-armament-hotfix-20261109/title-timing-probe.json) · [정상판 결과](../evidence/kamov-armament-hotfix-20261109/normal-web-smoke.json)

웹 패키징은 수정 전 자원 해시가 남아 있어 중단됐다. 승인된 변경과 원본 보존을 대조한 뒤 새 파생 자원의 해시를 갱신했다. PWA의 동결 범위 기록에도 앞서 승인·병합된 카모프·유로파이터 구현과 배치가 누락돼 있었다. 기존 PR #112·#114의 소스와 이번 변경을 정확한 전후 해시로 연결했다. 검사기를 끄거나 원본 자원을 예외 처리하지 않았다.

## 배포 범위

[PR #116](https://github.com/gelb2/HeavyMach/pull/116)으로 develop `0a065776`, [PR #117](https://github.com/gelb2/HeavyMach/pull/117)로 main `a7f4ac7b`에 병합했다. 두 PR의 코어·Apple 정식/치트 빌드·문서·번역 검사가 통과했다. main의 기존 글과 병합 후보를 대조했고 후보 트리는 검증한 develop과 같았다. main 병합 커밋에 [핫픽스 태그](https://github.com/gelb2/HeavyMach/releases/tag/kamov-armament-hotfix-20261109)를 붙였다.

[정식 웹·PWA](https://gelb2.github.io/HeavyMach/)와 [카모프 웹 치트판](https://gelb2.github.io/HeavyMach/validation/kamov-armament-hotfix-20261109/?validation-unit=Ka52)을 갱신했다. 기존 카모프·유로파이터 검증 URL도 같은 수정본을 제공한다. Pages 커밋은 `2e948106`, PWA 버전은 `47fc92ce7e92a267`, 게임 버전은 `b9eb1975ed29b8f4`다. 공개 파일30개의 해시가 로컬 검증 산출물과 모두 같았다. 공개 Stage01에서 미사일 발사 시각1.11666667/1.61666679, 두 번의 실제 피해25, 두 로터0/1, 기관포 부재를 확인했다. [공개 파일](../evidence/kamov-armament-hotfix-20261109/public-http.json) · [공개 전투](../evidence/kamov-armament-hotfix-20261109/public-campaign.json)

공개 치트 URL8조건과 정식 웹15단계도 통과했다. 정식판의 이동·점프·실제 사격·자동조준·일시정지·저장3,624바이트 복원과 재접속 슬롯 진입을 확인했다. [링크8조건](../evidence/kamov-armament-hotfix-20261109/public-link.json) · [정식판15단계](../evidence/kamov-armament-hotfix-20261109/public-normal.json)

이번 갱신은 소스·웹/PWA·웹 치트판이다. 기존 아이폰·Android·Tauri 설치본은 갱신하지 않았다.

## develop·main 보호

사용자의 추가 지시는 본인 계정은 예외로 두고 다른 사람의 직접 커밋·병합을 막는 것이다. 서버 보호 설정을 적용하고 GET API로 다시 확인했다. 다른 기여자는 PR·승인1건·CODEOWNERS 승인·대화 해결·필수 검사를 거친다. `.github/CODEOWNERS`는 전체 경로의 소유자를 `@gelb2`로 지정한다. 일반 권한자의 강제 푸시·브랜치 삭제도 허용하지 않는다.

본인은 관리자 예외를 유지한다. 현재 쓰기 권한과 관리자 권한을 가진 계정은 `gelb2` 한 명이다. 향후 다른 관리자 권한을 부여하면 그 관리자도 예외가 되므로 권한 추가 시 보호 범위를 다시 확인해야 한다. 작업자는 이번에도 작업 브랜치→develop→main 경로를 사용한다.

서버 필수 검사는 두 브랜치의 `documentation`, main의 `develop-only`다. core·apple·translations는 경로 필터가 있어 모든 PR의 필수 검사로 등록하지 않았다. 이번 릴리스에서는 실제 실행된 이 검사들의 통과도 확인했다. 최신 base 동기화 강제는 껐으며, 이를 모든 통합 상태의 검사 보장으로 표현하지 않는다. [develop 설정](../evidence/kamov-armament-hotfix-20261109/protection-develop-verified.json) · [main 설정](../evidence/kamov-armament-hotfix-20261109/protection-main-verified.json)

## 근거

- [코어28개](../evidence/kamov-armament-hotfix-20261109/core-tests.txt) · [발사 시각 상세](../evidence/kamov-armament-hotfix-20261109/assembly-detail.txt)
- [웹 치트 결과](../evidence/kamov-armament-hotfix-20261109/web-validation.json) · [자연 캠페인 결과](../evidence/kamov-armament-hotfix-20261109/web-campaign.json)
- [이미지 검증](../evidence/kamov-armament-hotfix-20261109/image-verification.json) · [빌드 파일 해시](../evidence/kamov-armament-hotfix-20261109/builds.json)
