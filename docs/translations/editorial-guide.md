# 공개 문서 번역

대상은 `docs/public/document-translation-scope.json`의 국문·영문 20쌍이다. 새 문서를 번역할 때는 사용자가 정한 방문자용 범위와 맞는지 확인한 뒤 목록을 수정한다. 정본·내부 기획·정책·사용자 명령·개발 스킬은 이 목록에 넣지 않는다.

## 원문과 문장

최신 main의 사용자 직접 수정부터 대조한다. 개인 원고는 인칭·비유·감정·비판을 보존한다. 저자의 말을 순화하거나 새로운 결론을 추가하지 않는다. `나`는 개발자다. Codex가 한 구현·검사·그림 제작은 Codex를 주어로 쓴다.

기술문서는 조건·행위·결과를 짧게 쓴다. 완료·계획·실패·삭제 상태, 수치의 대상과 단위, 원인과 관측을 대조한다. “It is important to note”, “This does not constitute”, “should not be construed as” 같은 작성자 설명을 붙이지 않는다. 실제 라이선스 조건과 저자 원문은 내용에 맞게 번역한다.

원문을 읽고 번역한 뒤 두 문서를 문단별로 대조한다. 그다음 영어판만 읽으며 지시 대상·주어·시제와 자연스러운 표현을 확인한다. 숫자 검사 결과에서 숫자를 영어 단어로 쓴 경우와 날짜·단위 변환을 확인한다. 링크·표·명령 자동 검사는 의미 대조 뒤 실행한다.

## 용어

| 국문 | 영문 | 구분 |
|---|---|---|
| 정식/일반 앱 | regular app | App Store 출시를 뜻하지 않음 |
| 치트 검증 앱 | cheat validation app | 별도 앱 ID와 저장 |
| 자동조준 | auto-aim | 자동 발사가 아님 |
| 표적 전환 | target cycling | 선택한 적을 바꿈 |
| 포신 조정 | barrel adjustment | 탄환 수치 변경과 구분 |
| 연사 속도 | firing rate | 미사일 이동 속도와 구분 |
| 탄속 | projectile speed | 해당 무기가 미사일이면 missile speed |
| 필살기 | special attack | 원작 공중 지원 계열의 맥락 보존 |
| 피해량 | damage | 체력은 HP, 경험치는 EXP |
| 생성 / 진입 | spawn / entry | 진입 후 전투 AI와 구분 |
| 개발자의 변 | developer's statement | 원고의 화자는 개발자 |
| 공용 모듈 | shared module | 플랫폼별 복사본과 구분 |
| 표시 호출 | display callback | 처리시간과 호출 간격을 구분 |

## 링크와 보존

루트 문서는 `.en.md`, 본문은 `docs/en/` 아래에 둔다. 각 문서의 언어 링크는 같은 문서의 두 언어로 연결한다. 영어판은 대상 목록 안의 영어판을 우선 연결한다. 그림·영상·소스·내부 정책·과거 검사 기록은 원본 파일을 함께 참조한다. 원문 절 링크는 영어판에 둔 명시적 앵커로 유지한다.

`language-nav` 주석 사이에는 두 언어 링크만 둔다. 저자 원고 해시를 계산할 때 언어 링크를 뺀다. 국문 개발자의 변은 `series-statement-nav`에 고정한 다른 작품 문서 링크도 뺀 뒤 원문 해시와 비교한다.

## 검사와 갱신

```sh
python3 -m unittest discover -s tools/tests -p 'test_translations.py'
python3 tools/check_translations.py
python3 tools/check_publication.py
python3 tools/document_contract.py check
```

원문이나 번역이 바뀌면 `manifest.json`의 해시가 달라져 검사가 실패한다. 의미·수치·링크를 대조한 뒤 해당 문서의 해시·숫자 차이·검토 메모를 갱신한다. 해시만 일괄 덮어쓰는 자동 승인 명령은 제공하지 않는다.

번역 CI는 명시한 공개 원고와 번역 도구 변경에만 실행된다. 게임 코드·정본·내부 정책만 바뀐 작업에 번역을 요구하지 않는다. 게임 버전 태그와 과거 릴리스 URL은 문서 번역 때문에 바꾸지 않는다.
