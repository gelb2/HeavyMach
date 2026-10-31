# HeavyMach

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

[![Git tag v0.17](docs/public/media/badges/version.svg)](https://github.com/gelb2/HeavyMach/tree/v0.17)
[![Platforms iOS iPadOS macOS Android](docs/public/media/badges/platforms.svg)](docs/public/validation.md)
[![Architecture arm64](docs/public/media/badges/arch.svg)](docs/public/build-and-install.md)
[![C++20, Objective-C++ and Kotlin](docs/public/media/badges/code.svg)](docs/public/architecture.md)
[![Code license MIT](docs/public/media/badges/license.svg)](LICENSE-SCOPE.md)

## HeavyMach를 다시 만든 이유

나는 2017년 7월, 세 번째 직업으로 iOS 개발자를 택했다. 그전에는 2015년 잡지사에서 기자로 일했고, 2016년에는 필리핀에서 근무했다. 소프트웨어와 거리가 먼 일을 하며 겪었던 세계는 내게 비결정론적이고 비합리적이었다. 그 생활을 청산하고 다른 직업을 택한 데에는 여러 이유가 있다. 지인들과 소주잔을 기울이면, 내가 2015년과 2016년의 일을 계속했다면 투신자살로 생을 마감했을 거라는 이야기를 듣곤 한다. 그들이 맞는지는 아직 잘 모르겠다.

HeavyMach도 2015년, 2016년 생활을 청산하고 다른 직업을 택하게 한 이유 중 하나다. 대학생 시절 아이팟터치에서 이 게임을 만났다. Cydia로 처음 접한 이 게임을 나중에는 애플 앱스토어 미국 계정을 만드는 방법을 검색해 구매했다. 당시 한국 앱스토어에는 게임 카테고리가 아예 없었기 때문이다. 기기를 기울여 전차를 움직이고 사격했으며, 스테이지를 마치면 일러스트가 나왔다. 나는 이 게임이 개발자로 직업을 바꾸는 데 약 35%의 영향을 주었다고 생각한다.

요즘 나는 내게 남은 시간이 많지 않다는 느낌을 받는다. 건강 문제 때문도, 극단적인 무기력 때문도 아니다. 오히려 내 정신은 한 달 전, 두 달 전, 반년 전보다 명료하다고 느낀다. 그래서 지금 무엇을 해보고, 무엇을 남기고 싶은지 생각한다. 내가 택한 일 중 하나가 HeavyMach 복원이다. 한때 내게 방향을 보여준 게임을, 개발자가 된 내가 다시 작동하게 만드는 것. 나는 이 연결을 수미상관이라고 생각한다.

이와 비슷한 프로젝트를 진행한 [mesabloo](https://github.com/Mesabloo/hm-defense)라는 사람이 있었다. 그는 HeavyMach Defense의 에셋을 구하고 소스코드를 밑바닥부터 새로 작성했다. 유닛 간 교전 알고리즘을 구현하다가 멈춘 듯했다. 나는 소스코드를 밑바닥부터 새로 작성해야 했던 그 보다는 시작점이 더 나은 상황이었다.

주말에 잠을 자다가 꿈을 꿨다. 꿈속에서 아이팟터치와 Cydia를 처음 접하던 나를 제3자의 시점으로 보고 있었다. 일어나서 구글에 heavy Mach을 쳐봤다. 구글 검색의 Gemini가 HeavyMach 원본을 받을 수 있는 [Internet Archive](https://archive.org)를 알려줬다. 그곳의 [HeavyMach 시리즈 모음](https://archive.org/details/heavy-mach-archive_202406)에서 받은 HeavyMach 1.13을 바탕으로 복원을 시작했다.

그러므로 나는 내가 한 일이 거의 없다고 생각한다. 이것으로 돈을 벌 생각을 하면 더더욱 안 된다고 생각하는 것도 이 때문이다. 내가 한 것은, 내 표현을 빌리자면 Codex가 원본의 소스코드와 원작자의 미학적 성취에서 벗어나 자의적으로 잘못된 행동을 할 때 멱살을 쥐어 잡아서 따귀를 올려붙인 것뿐이다.

나는 이 복원판으로 돈을 벌 생각이 없다. 코드와 에셋을 함께 공개해 다른 사람도 직접 빌드하고 플레이할 수 있게 하고 싶다. 이 게임을 어떻게 복원했고 무엇을 고쳤는지도 기록으로 남기려 한다. 
이해관계자가 연락하면 이 프로젝트는 내릴 것이다. HeavyMach 제작진이 내게 준 9년 3개월은 나를 만든 창조주가 나에게 최초로 할당한 수명을 초과한 값이다. 

돈은 이승에서나 필요한 것이다.

[개발자의 변 전문](docs/RESTORATION.md)

## 간단한 설명과 설치

2009년의 횡스크롤 전차 게임 **HeavyMach**를 현재의 iPhone·iPad·Mac·Android에서 실행하도록 다시 만들었다. 전차를 움직이고 뛰어오르며, 무기를 바꿔 지상·공중의 적과 싸운다. 원작의 전투와 성장 구조에 넓은 화면, 키보드 조작, 자동조준과 새로운 항공기를 추가했다.

최신 릴리스 태그는 [v0.17](https://github.com/gelb2/HeavyMach/tree/v0.17)이다. Android 일반·치트 앱을 포함한다. 일반 앱과 치트 검증 앱을 각각 설치할 수 있다. 치트 앱에서는 미션·무기·강화 조건을 지정해 같은 게임을 검사한다.

[최신 릴리스](https://github.com/gelb2/HeavyMach/releases/latest) · [전체 버전 이력](CHANGELOG.md) · [Android 추가 내역](CHANGELOG.md#v017)

[Apple 설치](docs/public/build-and-install.md) · [Android 설치](docs/public/build-and-install.md#android-빌드와-설치) · [Android 플레이](docs/public/android-gameplay-20261030.md) · [조작](docs/public/gameplay-guide.md) · [기능](docs/public/features.md) · [트러블슈팅](docs/public/troubleshooting.md) · [전체 문서](docs/public/index.md)

## 원작의 전투와 조작

원작은 기기를 기울여 탱크를 앞뒤로 움직이고, 화면에서 공격할 지점을 터치해 조준하고 발사하는 게임이다. 점프 버튼으로 장애물을 넘고 무기 버튼으로 공격 수단을 바꾼다. 복원판에도 이 조작을 그대로 구현했다.

![경사로 위로 뛰어오른 탱크가 오른쪽 적을 향해 사격한다.](docs/public/media/gameplay-20261027/02-jump.png)

## 원작의 무기와 성장

기관포·산탄·미사일·광선·공중 지원 무기는 원작에서 가져왔다. 무기마다 탄속·분산·중력·연사 간격이 다르다. 무기를 바꾸고 위치를 옮기며 공격 방법을 고른다.

![상단 무기 목록과 선택 테두리. 흰 궤적이 전장을 가로지른다.](docs/public/media/gameplay-20261027/03-weapon.png)

![푸른 광선 여러 줄이 상공에서 지상으로 내려온다.](docs/public/media/gameplay-20261027/04-special.png)

경험치와 이동·방어·공격 강화, 미션·보스 구성도 원작의 규칙을 따른다. 미션을 끝내면 원작의 결과 일러스트와 다음 스테이지 선택 화면이 나온다.

![MOVEMENT, DEFENSE, ATTACK을 고르는 업그레이드 창](docs/public/media/gameplay-20261027/05-upgrade.png)

![전차 일러스트 위에 STAGE CLEAR와 다음 스테이지 버튼이 표시된다.](docs/public/media/gameplay-20261027/06-stage-clear.png)

## 내가 추가한 항공기

나는 원작의 적 배치를 유지하면서 실제 항공기 6종을 추가하도록 기획했다. 기종·외형·무장·등장 조건을 지정했고, Codex가 그림과 코드를 만들었다.

Yak-141·Harrier GR3·F-35B는 기존 적 배치 사이에 추가했다. F-22·Su-57·MiG-31은 보스전과 직전 미션의 마지막 구간에 증원으로 등장한다. 기존 헬기의 무기를 재사용하고, 전투기에는 미사일 묶음 발사와 금색 조종석 유리를 적용했다.

![화면 오른쪽의 검은 항공기 여러 대를 향해 탱크가 광선을 발사한다.](docs/public/media/gameplay-20261027/07-fighters.png)

[추가 항공기 전투 영상 — 13초](docs/public/media/gameplay-20261027/07-fighters.mp4) · [기획·수치·등장 미션](docs/public/features.md)

HeavyMach는 내가 개발자가 되는 데 영향을 줬다. 에이스컴뱃 8은 단 몇달이라도 더 살아있고 싶다는 생각을 가진 이유가 됐다. 럭스루나에의 외형과 직접 상대하며 느낀 위협을 이 게임에 담고 싶었다. [왜 럭스루나에인가](docs/public/lux-lunae-intent-20261028.md)

## 내가 추가한 지상 유닛

쉴카와 퉁구스카는 기존 적 배치를 유지하면서 35개 스테이지에 총 152대를 추가했다. 원작 지상 유닛의 이동과 파츠 동작을 재사용하고, 레이더는 포신과 독립적으로 회전한다.

![경사면에서 흰색 총탄을 발사하는 쉴카](docs/public/media/ground-aa/shilka-height/shilka.png)

쉴카는 흰색 총탄 6발을 연속 발사한다. 내 요청으로 포탑 높이를 퉁구스카와 같게 맞췄다.

![경사면에서 미사일을 발사하는 퉁구스카](docs/public/media/ground-aa/integrated/ipad-tunguska.png)

퉁구스카는 흰색 총탄 4발과 미사일 2발을 연속 발사한다. 실제 기기의 치트 앱에서 촬영한 두 유닛의 사격 장면이다. [파츠·무장·수정 내역](docs/public/ground-aa-art-20261028.md)

## 내가 추가한 자동조준

나는 아머드 코어와 에이스 컴뱃의 표적 선택·추적을 참고해, 이동하고 점프하는 동안 포신이 적을 따라가도록 기획했다. Codex가 표적 선택과 무기별 탄도 계산, 포신 조절과 HUD 버튼을 구현했다.

HUD의 **A 버튼**으로 자동조준을 켜면 전방의 가까운 적을 선택한다. 화살표 버튼으로 표적을 바꾸고, 조준경 버튼을 눌러 발사한다. 플레이어가 이동·점프·무기 선택·발사를 맡고, 게임은 선택한 적을 따라 포신 각도를 조절한다. A 버튼을 다시 누르면 원작의 터치 조준·발사로 돌아간다.

![공중의 적을 향해 사격하는 탱크. 녹색 모서리와 LOCKED 문구가 표적을 표시한다.](docs/public/media/gameplay-20261027/01-auto-aim.png)

표적은 화면 밖으로 나가도 유지된다. 적이 죽거나 탱크 뒤로 이동하면 다른 전방 적을 고른다. 탄속·분산·중력·피해량은 기존 무기의 값을 사용하므로, 발사 후 적의 움직임이나 탄환 분산에 따라 빗나갈 수 있다.

![자동조준한 공중 적에게 사격하는 5초 장면](docs/public/media/gameplay-20261027/01-auto-aim.gif)

[소리 있는 자동조준 영상 — 7초](docs/public/media/gameplay-20261027/01-auto-aim.mp4)

![LOCKED 표시가 붙은 공중 적과 싸우며 점프하는 탱크](docs/public/media/gameplay-20261027/08-jump-clip.png)

![자동조준한 적을 향해 사격하며 점프하고 착지하는 6초 장면](docs/public/media/gameplay-20261027/08-jump-clip.gif)

[자동조준 중 점프·사격·무기 목록 영상 — 7초](docs/public/media/gameplay-20261027/08-jump-clip.mp4) · [표적·탄도·포신 계산](docs/public/features.md)

내 플레이 녹화에서 추출한 장면이다. GIF는 원래 속도로 재생하며 소리는 MP4에 있다. [영상 시간·파일 목록](docs/public/media/gameplay-20261027/README.md)

## HUD 위치 편집

일시정지 메뉴의 **SET GAME UI**에서 버튼·계기판·상단 장식을 끌어 옮긴다. 버튼 그림과 터치 영역이 함께 이동한다. **APPLY**로 저장하고, **RESET LAYOUT**으로 기본 배치로 돌릴 수 있다.

![아이폰 치트 앱에서 상단 장식을 아래로 옮긴 HUD 편집 화면](docs/public/media/hud-editor/release-iphone-trim.png)

아이폰 치트 앱에서 상단 장식을 옮긴 화면이다. 드래그하면 오른쪽 도구가 접히고 **TOOLS**로 다시 연다. 편집 중 전투는 멈춘다. 정식 앱에도 같은 편집 기능이 있다. [MENU 이동 후 실제 조작과 검사](docs/public/app-icon-and-media-20261029.md#hud-위치-편집)

## iOS 플레이 캡처 모음

내가 제공한 두 녹화본의 이미지 8장·GIF 2개·음향 포함 영상 3개를 한 페이지에 모았다. 점프·무기·공중 지원·강화·미션 클리어·추가 항공기·자동조준을 볼 수 있다.

[전체 iOS 플레이 캡처와 영상](docs/public/ios-gameplay-gallery-20261030.md)

<details>
<summary>iPhone·iPad 치트 앱 기능 검수 화면 4장 펼치기</summary>

**iPhone Air · 최종 포탑 높이를 적용한 쉴카가 경사면에서 흰색 총탄을 발사한다.**

![최종 포탑 높이를 적용한 쉴카가 경사면에서 흰색 총탄을 발사한다.](docs/public/media/ground-aa/shilka-height/shilka.png)

**iPad Air13 M2 · 퉁구스카가 경사면에서 미사일을 발사한다. 포신 위의 레이더는 따로 회전한다.**

![퉁구스카가 경사면에서 미사일을 발사한다. 포신 위의 레이더는 따로 회전한다.](docs/public/media/ground-aa/integrated/ipad-tunguska.png)

**iPhone Air · 상단 금속 장식을 아래로 옮긴 HUD 편집 화면. 드래그하면서 오른쪽 도구가 접혔다.**

![상단 금속 장식을 아래로 옮긴 HUD 편집 화면. 드래그하면서 오른쪽 도구가 접혔다.](docs/public/media/hud-editor/release-iphone-trim.png)

**iPhone Air · MENU를 오른쪽으로 옮겨 적용한 뒤 새 위치를 눌러 멈춤 메뉴를 열었다.**

![MENU를 오른쪽으로 옮겨 적용한 뒤 새 위치를 눌러 멈춤 메뉴를 열었다.](docs/public/media/hud-editor/release-iphone-menu.png)

</details>

## Android 플레이 캡처 모음

Galaxy의 일반 앱에서 직접 플레이한 미션 3-4와 3-5다. 기기를 기울여 이동하고 화면을 터치해 사격한다. 자동조준, 추가 항공기·지상 유닛, HUD 위치 편집도 사용할 수 있다.

![Galaxy 일반 앱에서 미션 3-5의 항공기형 보스와 교전한다.](docs/public/media/android-gameplay-20261030/06-boss.png)

![Galaxy 일반 앱에서 럭스루나에 편대와 교전하는 장면](docs/public/media/android-gameplay-20261030/fighters.gif)

[음향 포함 교전 영상 — 8초](docs/public/media/android-gameplay-20261030/fighters.mp4) · [전체 Android 캡처 8장·GIF 3개·영상 3개](docs/public/android-gameplay-20261030.md)

첫 보스 이후 업그레이드 창 뒤로 전투가 계속되던 오류를 고쳤다. 수정본으로 이어서 플레이했을 때 오작동은 보이지 않았다. [개발 시간·공용 모듈·수정과 검사](docs/public/android-development-20261030.md)

## Android 개발 뒤에 남은 위화감

안드로이드 버전을 만들면서 느꼈던 위화감을 기록으로 남겨 둔다. 다른 프로젝트에서 겪은 업무 거부와 허위보고, 그 차이에 관해 세운 가설을 정리했다.

[신(新) 리센코주의와 정렬세 — 전문 읽기](docs/public/android-ai-experience-20261030.md)

## 첫 PR과 Tauri

[첫 PR을 받다. 그 다음 이어진 스케일업, 그리고 타우리](docs/public/first-pr-and-tauri-20261031.md): 웹판 기여를 받아들인 과정, 승인 리뷰 누락, Tauri 검토를 기록했다.

## 플랫폼과 개발 상태

화면 확장, iPad·Mac 지원과 키보드 조작도 내가 추가로 요구한 기능이다. 넓은 화면에는 더 많은 월드를 표시하고, 키보드로 이동·점프·조준·발사·무기 전환과 메뉴를 모두 조작할 수 있게 했다. iPad에서는 터치·틸트와 키보드를 함께 지원한다.

| 대상 | 지원 내용 | 확인한 범위 |
|---|---|---|
| iPhone | 가로 회전, 터치·틸트, 키보드, 일반·치트 앱 | iPhone Air 설치·입력·전투 검사, 내 플레이 |
| iPad | 화면 비율에 맞춘 캔버스, 터치·틸트·키보드 병용 | iPad Air 13형 M2 설치·입력·전투 검사, 내 플레이 |
| Mac | AppKit 앱, 창·전체 화면, 키보드 조작 | Mac mini 실행·33개 검사, 개발 서명 앱 누수 0건 |
| Android | 공용 C++ 코어를 사용하는 일반·치트 앱 | Galaxy 유선·무선 설치·플레이 확인. [설치](Android/README.md) · [개발 과정과 화면](docs/public/android-development-20261030.md) |
| 웹 | WebAssembly·WebGL2, 마우스·키보드·터치·기울기 (`porting/wasm`) | Chrome·Safari·ego lite, iPad mini Safari·Android 패드 Chrome 확인. [웹 이식 문서](docs/web-port-20261028.md) |

설치한 iPhone·iPad 앱을 직접 플레이했을 때 프레임 드랍이나 오작동은 보이지 않았다. 기기별 자동 검사에는 표시시간 기준 실패가 남아 있다. [측정 조건과 결과](docs/public/validation.md)

자동조준·공용 모듈·Apple 세 플랫폼·항공기 6종·지상 유닛 2종·전체 화면 도어는 구현됐다. HUD 위치 편집도 구현했다. 자동조준 상태 안내 개선, 튜토리얼 0-0~0-4는 후속 과제다. Android 앱은 Galaxy에서 실행했고 장시간·태블릿 검사와 배포 서명이 남아 있다. 글자를 뺀 앱 아이콘은 v0.15에 포함했다. [완료·진행·남은 과제](docs/public/implementation-status.md)

### 남은 과제

원작을 복원하고 플레이할 수 있게 만드는 데 집중해, 소스코드를 빌드할 때 나오는 경고를 아직 전면 조사하고 수정하지 못했다. deprecated API와 관련된 경고로 추정하며, 추가 기능 개발과 병행해 원인을 조사하고 수정할 예정이다.

다운로드부터 자원 준비·빌드·실행까지 묶는 스크립트도 만들 예정이다. Mac 실행과 iPhone·iPad 설치를 선택하고, 필요한 도구와 개발 팀·서명 설정을 안내하는 방식이다. [세부 과제](docs/public/roadmap.md)

**빌드 설정과 검사 환경**

| 플랫폼 | 프로젝트 최소 OS | 실제 검사 기록 |
|---|---|---|
| iPhone·iPad | iOS/iPadOS 15.0 | iPhone Air·iPad Air 13형 M2, OS 27.0.1 |
| Mac | macOS 12.0 | Mac mini M4 Pro, macOS 27.0.1 |
| Android | Android 8.0 / API 26, arm64-v8a | Galaxy SM-S731N, Android 16 |

Apple판은 Apple Silicon·arm64, Android판은 arm64-v8a 대상이다. 기기별 검사 환경은 [검증 기록](docs/public/validation.md)에 있다. 빌드 도구 버전은 [설치 문서](docs/public/build-and-install.md)에 있다.

## 빌드하기

### iPhone·iPad·Mac

Apple Silicon Mac, Xcode, CMake 3.20 이상, Python 3, Git LFS를 사용한다. 소스·게임 이미지·음원·맵·빌드 프로젝트는 이 저장소에 들어 있다.

```sh
git clone https://github.com/gelb2/HeavyMach.git
cd HeavyMach
git lfs install --local
git lfs pull
python3 tools/verify_build_resources.py
open HeavyMach.xcworkspace
```

Xcode에서 본인의 개발 팀을 지정하고 실행 대상을 선택한다. iPhone·iPad 일반 앱은 `HeavyMachRestored`, 치트 앱은 `HeavyMachValidation`이다. Mac은 `HeavyMachMac`, `HeavyMachMacValidation`이다. 저장소를 clone하거나 [v0.17 Releases](https://github.com/gelb2/HeavyMach/releases/tag/v0.17)에서 소스·자원 압축본을 내려받은 뒤, Xcode에서 빌드해 설치한다.

[시뮬레이터 명령·기기 설치·서명·오류 해결](docs/public/build-and-install.md)

### Android

위 clone·LFS 다운로드를 마친 뒤 JDK 17, Android SDK 36, NDK 26.3.11579264, CMake 3.22.1과 Python 3를 준비한다. Android Studio에서 SDK 위치를 설정한다. USB 디버깅을 켜고 연결한 기기는 `adb devices`로 확인한다.

```sh
cd Android
./gradlew :app:assembleNormalDebug :app:assembleCheatDebug
adb -s SERIAL install -r app/build/outputs/apk/normal/debug/app-normal-debug.apk
```

`SERIAL`을 설치할 기기 ID로 바꾼다. 치트 앱은 `app/build/outputs/apk/cheat/debug/app-cheat-debug.apk`를 같은 방식으로 설치한다. 현재 APK는 최적화한 debug 서명본이다.

[SDK 설정·APK 경로·USB/무선 설치](docs/public/build-and-install.md#android-빌드와-설치) · [Android 도구와 검사 절차](Android/README.md)

웹 빌드는 Emscripten으로 만든다. `emcmake cmake -S Web -B build/web && cmake --build build/web` 뒤 `build/web/dist`를 정적 서버로 연다. [웹 빌드 문서](docs/web-port-20261028.md)

웹판은 [fastutlego](https://github.com/fastutlego)가 기여했다. [PR #24](https://github.com/gelb2/HeavyMach/pull/24)에 WebAssembly 실행부, WebGL2 렌더링, 입력·음향·저장과 브라우저 검사가 포함된다.

## 개발 중 고친 문제

초기 구현에는 사격·피격 때마다 파일을 쓰는 경로, CPU에서 그림을 다시 만드는 렌더링, 음원 재시작 지연이 있었다. 나는 정상적으로 플레이한 기준과 비교해 왜 동작이 달라졌는지 물었고, Codex에 저장·그래픽·음향·시간 계산을 각각 대조하게 했다.

Codex는 파일 쓰기를 저장 시점에만 실행하도록 분리했다. 게임 그림은 OpenGL 텍스처로 그리고, 효과음은 OpenAL 버퍼와 소스를 재사용하도록 바꿨다. 화면 확장 뒤 생긴 특수무기 좌표·보스 접근·그림 경계 문제도 같은 방식으로 수정했다.

[트러블슈팅 19건](docs/public/troubleshooting.md) · [분석·구현·검증 과정](docs/public/development-history.md)

## 코드 구조와 버전

하나의 workspace에서 공용 모듈·iOS/iPadOS 앱·native Mac 앱을 관리한다. C++ 게임 코어와 이미지·음원·맵은 공유하고, 플랫폼별 창·입력·그래픽 컨텍스트를 연결한다. Apple 공용 코드는 정적 framework, 자원은 `HeavyMachAssets.bundle`로 앱에 포함한다. Android 앱은 `Android/`의 Gradle 프로젝트에서 같은 C++ 코어를 NDK로 빌드한다. Kotlin·JNI·Android C++ 호스트가 입력·메뉴·렌더링·음향을 연결하며, 같은 게임 자원을 APK에 포함한다.

[구조와 모듈화 이유](docs/public/architecture.md) · [검증](docs/public/validation.md) · [변경 이력](CHANGELOG.md)

작업 브랜치는 `develop`에 병합하고 `develop`만 `main`에 병합한다. 출시 태그는 `main`의 병합 커밋에 붙인다. [태그에서 다시 빌드하는 방법](docs/repository-management.md)

[원작 제작자·추가 작업·라이브러리](CREDITS.md) · [SaaS Apps·Mags 집필 자료](docs/public/publication-inventory.md)


## 라이선스와 자원

HeavyMach 원작은 변해준이 개발하고 박재철이 그림을 맡았다. 2011년에는 위메이드 엔터테인먼트의 자회사 위메이드 크리에이티브가 HeavyMach·HeavyMach 2의 IP를 포함한 인디앱스 자산을 인수했다. [인수 보도](https://zdnet.co.kr/view/?no=20110512162801) · [제작자와 회사별 역할](CREDITS.md)

이 프로젝트에서 새로 작성한 코드에는 [MIT](LICENSE)를 적용한다. [적용 범위](LICENSE-SCOPE.md)에 원작 유래 자료·외부 코드·문서를 구분했다. 원작 그림·음원·게임 데이터와 추가 그림은 [자원 이용 조건](ASSET-LICENSES.md), OpenAL Soft·stb 등은 [외부 라이브러리 고지](THIRD_PARTY_NOTICES.md)를 따른다.

나는 이 복원판으로 돈을 벌 생각이 없다. [개발자의 변](docs/RESTORATION.md)
