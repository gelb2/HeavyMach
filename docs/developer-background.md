# 개발 동기 자료와 작성 순서

개발자의 변, README 소개, GitHub 회고, SaaS Apps·Mags에서 HeavyMach를 설명할 때 이 문서와 아래 원문을 먼저 읽는다. 기술 기획서의 학습·AI 실험 목적만으로 개인적인 시작 이유를 대신하지 않는다.

## 읽을 원문

1. [사용자가 추가한 경험과 연도 정정](references/developer-background/user-background-20261026.md). 필리핀 근무는 2016년, iOS 개발자 전업은 2017년 7월이다. 약 35%는 본인이 평가한 HeavyMach의 영향 비중이다.
2. [기존 MachDefense 복원 문서의 시작 배경](references/developer-background/hm-defense-origin.md). 대학생 시절의 플레이와 이후 직업 전환, Mesabloo 프로젝트 발견을 기록했다. 원문의 2008년 표기는 그대로 보존하되 새 원고에는 최신 사용자 설명과 당시 출시 자료의 2009년을 사용한다. 첫 플레이의 정확한 날짜는 새로 만들지 않는다.
3. [SaaS 「항명의 대가」 공개 글](https://sass.studio/magazine/cerulean). V09, № 06. 편집디자인팀의 집단 항명과 근무 조건에 관한 작성자의 경험이다.
4. [Unity 초기 기획 발췌](references/developer-background/unity-planning-origin.md). 2026년 7월 21일 개발·학습 목적과 7월 22일 HeavyMach 1을 참고한 게임 착수 결정이다.
5. [개발자의 변 원고](RESTORATION.md). 첫 글감과 [후속 글감](references/developer-background/user-restoration-reason-20261026.md)을 반영했다. 현재 복원 동기·원작자 연락·조건부 무료 배포 의향까지 작성했다.

복사 범위·출처 커밋·파일 해시는 [출처 목록](references/developer-background/source-manifest.json)에 있다. hm-defense의 기존 RESTORATION.md는 수정하지 않는다. 다른 비공개 프로젝트의 소스와 내부 문서는 이 저장소에 복사하지 않는다.

6. [복원 시작과 꿈에 관한 글감·정정](references/developer-background/user-project-origin-20261028.md). 복원 시작에 관한 세 문단은 README와 개발자의 변에 같은 문장으로 싣는다. 최신 README는 아래 채택 원고의 순서를 따른다.

7. [위메이드 인수 인지 시점 정정](references/developer-background/user-wemade-discovery-20261028.md). Mesabloo의 복원 작업은 알고 있었지만, 위메이드 인수 사실은 2026-10-28 크레딧 조사에서 처음 알았다고 밝혔다. 두 제작자의 이름을 먼저 기록한 경위도 개발자의 변에 반영했다.

8. [채택한 README 도입 원고](references/developer-background/approved-readme-opening-20261028.md). 「HeavyMach를 다시 만든 이유」 아래에 2017년 7월 개발자 전업 → HeavyMach의 영향 → 지인들의 말과 9년 3개월 → 현재 복원 이유 → Mesabloo·꿈·구현 경위를 배치한다. 앞서 지정한 ‘요즘 나는…’ 첫 문단 배치는 이 후속 지시로 변경했다.

9. [Cydia 이후 미국 App Store 구매 경위](references/developer-background/user-cydia-purchase-20261028.md). Cydia로 처음 접한 뒤 미국 계정을 만드는 방법을 검색해 구매했다. 당시 한국 스토어에 게임 카테고리가 없었다는 경험을 README와 개발자의 변에 반영했다.

10. [왜 럭스루나에인가](public/lux-lunae-intent-20261028.md). 두 게임의 개인적인 의미, 달빛에 관한 경험, 보스전 기획과 기술 검증 의도를 담았다. [대화 원문 14개](references/developer-background/user-lux-lunae-intent-20261028.md)와 [데스 스트랜딩 사연 출처](references/developer-background/death-stranding-community-story-20261028.md)를 함께 읽는다.

## 경위

| 시점 | 기록된 내용 | 근거 |
|---|---|---|
| 대학생 시절 | iPod touch에서 HeavyMach를 플레이 | 기존 복원 문서. 첫 플레이 날짜는 확정하지 않음 |
| 2015년 | 잡지사 근무, 편집디자인팀의 항명과 처우 변화 관찰, 공장 자동화 기업 임원 인터뷰 | 사용자 추가 설명·SaaS 발췌 |
| 2016년 | 필리핀 근무 | 이번 사용자 정정 |
| 2017년 7월 | 세 번째 직업으로 iOS 개발자 선택 | 이번 사용자 정정 |
| 2026년 2월 | Mesabloo의 MachDefense 복원 프로젝트 포크 | hm-defense의 기존 RESTORATION.md에 2월 24일로 기록 |
| 2026년 7월 | HeavyMach 1의 조작·구조를 참고한 오리지널 Unity 게임 착수 | Unity 초기 기획 |
| 2026년 10월 | IPA 분석, 호환 실행판과 소스 복원판 제작·비교, iPhone 설치 | [복원 과정](public/ipa-port-and-native-app-20261026.md)·[현재 상태](current-status.md) |

일어난 작업의 순서와 사용자가 그 일을 택한 이유를 구분한다. 개인적인 이유는 사용자가 제공한 두 글감을 따른다.

## 작성·수정

- 본문에는 경험·사건·판단을 쓴다. AI의 자기평가, 원고를 설명하는 메타카피, 요청하지 않은 경고·면책, 대안 없는 의견을 붙이지 않는다. 기술과 작은 수정에 의미·교훈을 덧붙이지 않는다.
- 약 35%는 작성자의 회고다. 통계나 유일한 전업 동기로 바꾸지 않는다. 편집디자이너들의 기술과 교섭력, 자동화 기업 취재 경험을 함께 남긴다. 나머지 영향의 비율을 계산해 만들지 않는다.
- 출처 확인 메모와 아직 받지 않은 문단의 상태는 이 문서에서 관리한다. 이를 개발자의 변·Apps·Mags 본문에 복사하지 않는다.
- 사용자 보충·정정 → 보존 원문 추가 → RESTORATION 원고 → README·Apps·Mags의 해당 문단 순서로 수정한다. 새 사용자 문장이 없는 곳은 임의로 이어 쓰지 않는다.
- Apps는 기술 설명, Mags는 매거진 인터뷰 형식으로 구성한다. 두 곳 모두 현재 기능과 시정 과정을 포함한다. 개인 경험과 답변은 사용자 원문을 따르고, 새 질문은 질문안으로 둔다. 구현 과정·완료 범위는 실행 기록과 대조한다.

## 출시·성과 확인

2009년 2월 10일 [TouchArcade의 직접 플레이 기사](https://toucharcade.com/2009/02/10/first-look-at-heavy-mach-side-scrolling-tank-shooter/)에서 출시를 확인할 수 있다. [중앙일보의 개발자 인터뷰](https://www.joongang.co.kr/article/3534205)는 한국 개발자 작품 최초의 앱스토어 유료 5위권 진입과 약 10만 달러, 당시 환산 약 1억4천만 원의 매출을 보도했다.

사용자 원문에는 ‘대한민국 최초 미국 앱스토어 1위’, ‘대한민국 최초 1억원 매출’이 있다. 이번에 읽은 자료로는 두 ‘최초’를 확인하지 못했다. [2009년 8월 11일 전자신문에 실린 연합뉴스 기사](https://www.etnews.com/200908110002)는 미국 유료 전체 첫 1위를 Cartoon Wars로, HeavyMach의 앞선 기록을 전체 5위로 설명한다. 분야별 순위와 전체 순위를 섞지 않는다. 현재 원고에는 인터뷰에서 확인한 5위권·1억4천만 원만 사용했다.

「항명의 대가」는 위 공개 글에서 읽을 수 있다. 로크웰 오토메이션·B&R·요코가와 임원 인터뷰 경험은 이번 사용자 설명을 근거로 기록하며 기사 제목이나 취재 날짜를 추정하지 않는다.

## 후속 글감의 출처

‘남은 시간이 많지 않다는 느낌’, ‘수명을 9년 3개월 연장해 주었다’는 이번 사용자 글감의 표현이다. 지인들의 말은 사용자의 후속 정정에 따라 “투신자살로 생을 마감했을 거라는 이야기를 듣곤 한다”로 반영했다. 원문은 별도 파일에 보존했다.

메신저 문의는 작성자가 전한 내용이다. ‘어제’는 2026-10-26 제공 문장의 시점이다. 프로필의 본인 여부는 확인하지 않았다. 페이퍼블루 필명과 설국열차 참여에 관한 추정은 원문에 남겼고, 설국열차 경력은 본문에 추가하지 않았다.

수익 목적이 없다는 의향을 반영했다. 향후 배포는 원작자·이해관계자의 승인을 전제로 한 가능성이며, 무료 배포도 확정된 결정으로 쓰지 않았다.

## 게시용 원고

[게시 자료 목록](public/publication-inventory.md)에 GitHub·Apps·Mags별 원고와 아직 필요한 자료를 모았다. 설치와 구현 상태는 연결된 기술 문서를 사용한다. Apps·Mags 본문은 SaaS 담당 세션이 이 프로젝트의 자료를 읽고 작성한다. 개인 서사의 수정은 RESTORATION과 사용자 정정 원문을 먼저 반영한다.

## 2026-10-29 문장 변경

[추가 원문](references/developer-background/user-foreword-20261029.md)의 게시 중단·9년 3개월·수익에 관한 문장을 README 첫 소개와 개발자의 변에 같은 문단으로 반영했다.

## 원본을 받은 사이트 — 2026-10-30

사용자가 최초 복원 원본의 Internet Archive 출처 확인과 GitHub 명시를 지시했다. 최초 보관본과 재다운로드의 SHA-256이 같음을 확인했다. README·개발자의 변·개발 과정에서 [HeavyMach 시리즈 모음](https://archive.org/details/heavy-mach-archive_202406)에서 1.13을 받았다고 적고 해당 모음을 연결한다. 사용자의 후속 출처 표기 지시를 반영했다. [출처 증거](source-provenance.md).

## 첫 외부 PR에 관한 글감 — 2026-10-31

[제공 원문](references/developer-background/user-first-pr-20261031.md)과 [집필 과제](usability-todo.md#hm-first-pr-story)를 연결했다. 기여자가 들인 시간에 대한 생각, 네이티브 앱 유지에 관한 고민, 웹판에서 Windows·Mac 지원 가능성을 발견한 순서를 다룬다. 후속 지시에 따라 [첫 PR과 Tauri 원고](public/first-pr-and-tauri-20261031.md)를 작성했다. 승인 리뷰 생략 경험을 같은 글에 담았다.
