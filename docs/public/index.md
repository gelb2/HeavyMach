# HeavyMach 문서

<!-- language-nav:start -->
[한국어](index.md) | [English](../en/public/index.md)
<!-- language-nav:end -->

HeavyMach는 iPhone·iPad·네이티브 Mac·Android·웹/PWA·Tauri Mac에서 실행한다. 최신 릴리스 태그는 v0.17이며, 현재 main에는 웹 배포·복원 코드·게임 버튼 수정도 들어 있다.

## 시작하기

- [소개와 플레이 화면](../../README.md)
- [빌드와 설치](build-and-install.md): Apple·Android 준비물, 소스·자원 다운로드, Xcode·Gradle 빌드와 기기 설치.
- [조작](gameplay-guide.md): 터치·틸트·키보드, 자동조준, HUD 편집.
- [구현 상태](implementation-status.md): 제공하는 기능과 기기별 확인 범위.


## 웹·PWA·Tauri와 복원 코드

- [플레이와 설치](https://gelb2.github.io/HeavyMach/): 브라우저 실행과 오프라인 PWA.
- [복원 코드와 화면](save-recovery-20261102.md): 원작 슬롯, 공용 모듈, 서버 없는 이전, 버튼 수정과 플랫폼별 검사.
- [Tauri 빌드](../../Desktop/Tauri/README.md): 로컬 Mac 앱. 공개 설치본·공증은 남아 있다.

## Android

- [빌드와 설치](build-and-install.md#android-빌드와-설치): SDK 설정, 일반·치트 APK, USB·무선 설치.
- [개발 과정](android-development-20261030.md): 공용 C++ 코어, Android 전용 구현, 개발 시간, 업그레이드 오류 수정.
- [Galaxy 플레이 캡처와 영상](android-gameplay-20261030.md): 일반 앱의 미션 3-4·3-5, 이미지 8장·GIF 3개·영상 3개.

## 개발 배경과 기획

- [개발자의 변](../RESTORATION.md): 이 게임을 복원한 개인적인 이유.
- [추가 기능의 기획 의도](design-intent.md): 넓은 화면, 공용 모듈, 자동조준, 새 적과 HUD 편집.
- [왜 럭스루나에인가](lux-lunae-intent-20261028.md): 두 게임의 의미, 도색과 전투 기획, 승인 도안과 게임 화면.
- [기능과 세부 구현](features.md): 표적·탄도 계산, 항공기 무장과 배치 수치.

- [Android 개발 뒤에 남은 위화감](android-ai-experience-20261030.md): 다른 프로젝트에서 겪은 업무 거부·허위보고와 그 차이에 관한 가설.

## 구현과 검사

- [코드 구조](architecture.md): workspace, 공용 코어·런타임·자원, 한 프레임의 처리.
- [개발 과정](development-history.md): 분석·구현·비교 순서와 사용한 도구.
- [트러블슈팅](troubleshooting.md): 저장·렌더링·음향·시간·좌표·기기·그림에서 고친 문제.
- [검증 결과](validation.md): 자동 검사, 직접 플레이, 프레임·메모리 측정과 남은 검사.
- [변경 이력](../../CHANGELOG.md): v0.17 Android 출시와 v0.9~v0.17의 태그·다운로드·검사.
- [남은 과제](roadmap.md): 버그 수정, 검사, 편의 기능과 추가 플랫폼.

## 참여와 제작자

- [기여 안내](../../CONTRIBUTING.md)
- [크레딧](../../CREDITS.md)
- [코드 라이선스 범위](../../LICENSE-SCOPE.md)
- [자원 이용 조건](../../ASSET-LICENSES.md)
- [외부 라이브러리 고지](../../THIRD_PARTY_NOTICES.md)

각 문서 상단의 한국어·English 링크로 같은 문서의 언어를 바꿀 수 있다. 이미지와 영상은 두 언어에서 같은 파일을 사용한다.
