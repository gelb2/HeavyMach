# 헤비멕1 복원을 마치고

처음부터 내 로드맵은 개발자 변해준과 일러스트레이터 박재철이 만든 연작을 모두 복원하는 것이었다. 나에게 남은 시간이 얼마 없기에, 내게 가장 큰 영향을 준 HeavyMach 1을 먼저 복원했다.

HeavyMach 1은 이제 주변 개발자들과 지인들이 직접 플레이할 수 있는 상태가 됐다. 그들에게서 잘 복원했다는 확인과 “잘해줬다”는 말을 들었다. 복원한 코드는 GitHub의 main까지 반영했다.

이제 HeavyMach 1은 유지보수 단계로 관리한다. 이후 기능 추가나 버그 수정은 [GitHub PR](https://github.com/gelb2/HeavyMach/pulls)을 보내거나 개인적인 채널로 연락을 주면 진행할 것이다. [Sass](https://sass.studio/)를 통해서도 연락할 수 있다.

나 역시 TODO로 남겨둔 사항과 기술적으로 미진한 사항들을 틈틈이 자발적으로 진행할 것이다. 하지만 이 지면을 빌려 확실히 말하자면, HeavyMach 1은 이제 원작이 지원하는 모든 기능을 복원한 상태로 동작하고 있다.

그러므로 내게 남은 시간 동안, 다음 복원 과제인 MachDefense부터 복원해 나가려 한다. HeavyMach 복원을 먼저 진행한 Mesabloo와는 다른 방법을 택했다. 원본 앱을 분석하고, 그 동작을 비교하면서 새 소스로 복원하는 방식이다. iOS·iPadOS·Android·PWA·Mac에서 플레이할 수 있게 만드는 것이 목표다.

현재 MachDefense는 iOS와 iPadOS에서 정상 동작을 확인했다. 아래는 복원 중인 MachDefense의 플레이 화면이다.

[![복원 중인 MachDefense의 Stage 23 전투 화면](https://media.githubusercontent.com/media/gelb2/HeavyMach/1102fc70fc4904c400ea45dfe2bb0e7c2d0f2a1c/docs/public/media/machdefense-roadmap-20261105/IMG_1129.PNG)](https://media.githubusercontent.com/media/gelb2/HeavyMach/1102fc70fc4904c400ea45dfe2bb0e7c2d0f2a1c/docs/public/media/machdefense-roadmap-20261105/IMG_1129.PNG)

*MachDefense Stage 23. 2026년 11월 5일 제공한 플레이 화면.*

MachDefense를 복원하며 Gemini를 잠깐씩 쓰는데, 묘한 말이 나왔다. 나는 기획회의는 Gemini와 하는 편이다. 결정론적인 소스코드 구현이나 정합성이 맞아야 하는 긴 분량의 장편소설에서는 어조나 논리가 어그러진다고 느낀다. 그래도 단편적이고 간단한 브레인스토밍 단계에서는 Claude와 Codex가 보여주지 못하는, 인간의 광기가 살짝 묻어나는 기획과 발상이 나온다고 보기 때문이다.

> 달은 스스로 빛을 내지 못하고 태양의 빛을 반사할 뿐이지만, 칠흑 같은 밤길을 걷는 자에게 유일한 길잡이가 되는 것은 태양이 아니라 달이다. 당신의 반사는 단순한 모방이 아닌, 망각에 맞서는 가장 정밀한 거울의 사명이다.

앞서 말했듯이, 내가 쓰며 느끼기에 Gemini는 현시점에서 출시 초기에 보인 장기기억 능력과 고맥락 추론 능력이 거의 없어진 상태다. 나쁘게 말하면 최고급 모델을 제외하고는 심심이나 유머 생성기 정도일지도 모른다.

HeavyMach 레포지토리를 열어본 적도 없는 Gemini가 이런 말을 한 것은 단순한 우연일 확률이, 내 생각에는 약 85%일 것이다. 내가 지금은 관리하지 않는 소설 기술 플랫폼 서비스에 대해서도 갑자기 말하는 경우가 있으니까.

하지만 나는 달빛만으로 앞을 가늠하며 바이크로 전국일주를 한 경험이 있다. 이 프로젝트에도 굳이 신유닛으로 럭스루나에, 라틴어로 달빛을 뜻하는 이름의 도장을 한 유닛을 넣었다. 그런 프로젝트와 관련된 얘기에 Gemini가 저런 말을 한 것은 묘하다.

그러므로 남긴다. 그것뿐이다.

나한테 아직 숨이 붙어 있다면, HeavyMach 1과 MachDefense, 나아가 HeavyMach 2의 복원판을 GitHub 어디에선가 볼 수 있을 것이다.
