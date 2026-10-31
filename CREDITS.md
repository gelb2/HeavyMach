# 제작자와 사용 자원

<!-- language-nav:start -->
[한국어](CREDITS.md) | [English](CREDITS.en.md)
<!-- language-nav:end -->

## HeavyMach 원작

- 게임 개발: **변해준**
- 일러스트: **박재철**
- 2009년 출시 표기: **IndieAn**
- 2011년 IP 인수: **위메이드 크리에이티브** — 당시 위메이드 엔터테인먼트 자회사
- 2011년 시리즈 리뉴얼 발표: **위메이드 엔터테인먼트**

[2009년 제작자 인터뷰](https://www.koreajoongangdaily.com/lifestyle/korean-game-developers-shoot-for-success/10531072)와 [출시 기사](https://toucharcade.com/2009/02/10/first-look-at-heavy-mach-side-scrolling-tank-shooter/)에 제작자와 출시 주체가 기록돼 있다. 2011년에는 위메이드 크리에이티브가 HeavyMach·HeavyMach 2의 IP를 포함한 인디앱스 자산을 인수했다. [인수 보도](https://zdnet.co.kr/view/?no=20110512162801). 같은 해 위메이드 엔터테인먼트는 Assault·Battle·Defense 리뉴얼을 발표했다. [당시 보도자료](https://www.newswire.co.kr/newsRead.php?no=575013)

이 프로젝트는 HeavyMach의 전투·성장·미션과 그림을 현재 Apple 기기에서 실행하도록 재구현했다. 개발을 시작한 개인적인 이유와 원작자에게 전하고 싶은 이야기는 [개발자의 변](docs/RESTORATION.md)에 있다.

## 재구현과 추가 개발

나는 원작과 비교할 기준, 화면 확장, 플랫폼·모듈 구조, 키보드, 자동조준과 신규 유닛을 기획했다. 실제 iPhone·iPad 플레이로 증상을 발견하고 저장·그래픽·탄도·입력·미술 변경을 수정하도록 지시했다.

Codex는 동작 분석, C++·Objective-C++ 구현, 이미지 제작·연결, 빌드·검사·설치와 문서 작성을 수행했다. 잘못 구현한 부분과 수정 과정은 [트러블슈팅](docs/public/troubleshooting.md)에 기록했다.

웹판 개발: [fastutlego](https://github.com/fastutlego), [PR #24](https://github.com/gelb2/HeavyMach/pull/24). 기존 C++ 코어를 사용하는 WebAssembly 호스트, WebGL2 렌더러, 브라우저 입력·음향·IndexedDB 저장, HUD 편집과 자동 검사를 구현했다. 원래 커밋·작성자·공동 작성자 기록을 유지하고, 검토 중 발견한 오류의 후속 수정은 Codex 작업으로 별도 커밋한다.

## 그림과 플레이 자료

기존 게임 이미지·음원·맵은 [자원 목록](docs/build-resources.json)으로 관리한다. Codex가 imagegen으로 메뉴의 좌우 확장 그림과 추가 항공기 그림을 만들었고, 나는 시안을 검토하고 선택했다. 실제 기체 사진·도면과 기존 게임의 색조를 대조한 과정, 프롬프트와 파일 해시는 [VTOL](docs/public/vtol-art-and-loadout-20261027.md)·[전투기](docs/public/lux-lunae-design-20261027.md) 문서에 있다.

README의 플레이 이미지 8장·GIF 2개·영상 3개는 Codex가 내 화면 녹화에서 추출했다. [장면·시간·해시](docs/public/media/gameplay-20261027/media-scenes.json)

## 라이브러리

| 라이브러리 | 용도 | 저장소의 라이선스 원문 |
|---|---|---|
| OpenAL Soft | 효과음 버퍼·소스와 재생 | [COPYING](vendor/openal-soft/COPYING), LGPL 계열 배포 조건 |
| stb | 이미지 처리 | [LICENSE](vendor/stb/LICENSE), MIT 또는 Public Domain 선택 |

새로 작성한 코드에는 [MIT](LICENSE)를 적용한다. [코드 적용 범위](LICENSE-SCOPE.md), [자원별 조건](ASSET-LICENSES.md), [외부 라이브러리 고지](THIRD_PARTY_NOTICES.md)에서 각각의 출처와 조건을 확인할 수 있다.
