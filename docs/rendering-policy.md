# 그래픽 렌더링 정책

## Apple 플랫폼 확장 — 2026-10-26

Apple 모듈화에서 iOS/iPadOS의 ES1.1 경로는 유지한다. native Mac은 AppKit NSOpenGLContext와 desktop compatibility GL을 사용하고 스프라이트·블렌드 제출 코드를 공유한다. Catalyst는 사용하지 않는다. [플랫폼 구조](apple-platforms.md).

정본: [canonical](canonical.md). 제품 경로는 `HMGameView → HMMedia → HMGLRenderer`다. 이전 CPU raster/UIImage 합성은 비교·검사 경로로 남아 있으며 일반 게임의 정상 렌더 경로로 복귀시키지 않는다.

2026-10-25 전체 캔버스 확장을 구현했다. 원본480 비교 모드와 확장 게임플레이 모드를 같은 GL 경로로 처리한다. 현재 구현과 검사는 [캔버스 기록](wide-canvas-plan.md)을 따른다.

## Mac 화면 표시

Mac은 기존60Hz 게임 타이머를 유지한다. `NSOpenGLContext`의 swap interval은0으로 두어 별도 수직동기화 대기를 더하지 않는다. 프레임 제한기의 대기 시간에서는 렌더링 경과 시간을 차감한다. iOS/iPadOS의 원본 호환 대기 계산은 변경하지 않는다.

Mac 게임 뷰는 고해상도 OpenGL 표면을 요청하고 `convertRectToBacking`의 픽셀 치수를 사용한다. GL 컨텍스트의 뷰·창·화면 위치·픽셀 크기가 달라질 때 표면을 갱신한다. 매 프레임 `setView`와 `update`를 반복하지 않는다. 창 크기 변경·전체 화면 왕복과 실제 화면이 켜진 상태의 성능을 검사한다. [측정 결과](public/apple-platform-progress-20261026.md).

## 고정할 경로

- iPhone/iPad는 OpenGL ES1.1, Mac은 desktop OpenGL compatibility로 게임을 그린다. UIKit/AppKit은 창·생명주기와 텍스트 입력·클립보드 같은 OS 연결을 담당한다. 게임 실행 후 버튼은 아래 원작 버튼 절차를 따른다.
- 논리W×H, 할당ceil(W)×ceil(H) RGBA8 renderbuffer에 원본 순서로 그린다. 원본 비교 모드는480×320이다. 확장 모드의 W/H는 좌표 정책을 따른다. 크기가 바뀔 때만 재할당하고 투영·복사·readback이 같은 치수를 사용한다. 장면을 재사용 화면 텍스처로 복사하고 drawable에 최종 확대한다.
- 원본 자원의 제품 소비자는 `OriginalImages`의 원본 파일명과 확장자를 사용한다. 미션/스테이지 클리어의 `GameLoading_XX[E].jpg`, 게임오버의 `StageClear.jpg`에 파생본 접미사 `.png`를 붙이지 않는다. `Images/*.jpg.png`는 변환·비교용 자원이며 현재 제품 로더의 이름 계약이 아니다.
- 원본 이미지695개를 보존된 stb 코드로 해석한다. POT 패딩, premultiplied 픽셀, RGB565 변환, UV와 타일 순서를 임의 보정하지 않는다.
- sprite 텍스처는 LINEAR/REPEAT, 최종 화면 텍스처는 LINEAR/CLAMP_TO_EDGE다. 두 텍스처의 규칙을 섞지 않는다.
- iPhone/iPad drawable은 nativeScale과 RGB565 요청을 사용한다. Mac drawable은 고해상도 RGBA 표면을 사용한다. 실제 드라이버 보고 형식은 별도 기록한다. 요청이 실제 RGB565 할당을 증명하지 않는다.
- 마지막 확대는 전체 화면 비율 맞춤 NDC quad, DITHER 비활성·blend 비활성 후 원래 상태 복원이다. 원본480 모드에서는 검정 여백을 유지하고 확장 게임 화면은 전체 drawable을 채운다. viewport와 터치는 [좌표 정책](coordinate-policy.md)을 함께 따른다.

## 합성과 순서

| Sprite blend 값 | GL source | GL destination |
|---|---|---|
| 0,3 | SRC_ALPHA | ONE_MINUS_SRC_ALPHA |
| 1 | SRC_COLOR | ONE_MINUS_SRC_ALPHA |
| 2 | SRC_ALPHA | ONE |
| 그 외 | 기존 상태 유지 | 기존 상태 유지 |

텍스처 환경은 원본 GL_REPLACE를 보존한다. Trail의 blend3는 sprite와 같다고 가정하지 않는다. `Trail.cpp`가 만든 setBlend/sourceFactor/destinationFactor를 renderer가 소비한다. 원본의 계층·등록·z순서·숨김 상태를 유지하고 최적화를 이유로 투명 sprite 순서를 임의 재배치하지 않는다.

## 자원 수명과 프레임 비용

공유·화면·월드 scope가 소유자다. GPU 업로드 후 CPU 이미지 픽셀을 해제하고 크기/형식 정보를 보관한다. 새 화면 준비 뒤 이전 전용 scope를 해제하며 맵 ClearTileResources에서 월드 텍스처를 해제한다. 동일 이름도 원본 소유자가 다르면 분리한다.

프레임마다 파일 읽기·이미지 재해석·UIImage 생성·부품 벡터 정렬을 추가하지 않는다. layoutSubviews/drawRect에서 게임 tick/draw를 중복 호출하지 않는다. 수명이 끝난 탄흔은 그 프레임에 그리지 않는다. 새 자원의 최초 준비 비용과 반복 프레임 비용을 따로 측정한다.

## 검사와 변경 판정

GL 오류, 논리 화면 픽셀, 최종 표시 배치, 자원 해제, 실제 표시 간격을 각각 확인한다. 완전 픽셀 일치가 실패하면 차이를 보존한다. 평균 FPS만으로 긴 프레임을 지우지 않는다. 장면·기기·OS·빌드·계측 구간·열 상태를 기록한다.

시뮬레이터 Apple Software Renderer의 수치를 아이폰 GPU 성능으로 환산하지 않는다. 같은 환경 포팅판과의 비교는 회귀 분리에 사용한다. 16.67ms 기존 검사 실패 기준을 낮추지 않는다. API 변경·저해상도화·확대 필터 변경을 성능 해결의 기본 처방으로 삼지 않는다.

원본/호환판 주소와 최종 비교 근거: [직접 비교](compat-source-comparison-20261024.md), [OpenGL 전환](opengl-migration-20261024.md), `HMGLRenderer.mm`, `HMCompatibilityPixels.hpp`, `SpriteRaster.cpp`, `Trail.cpp`.


## 확장 장면·중앙 HUD

월드 그림은 전체 폭, 기본 배치 HUD/업그레이드/결과 글자·버튼은 원본480 영역에 같은 transform과 clip을 적용한다. 사용자 HUD 배치는 아래 편집 규칙을 따른다. **결과 일러스트는 별도로 전체 캔버스를 덮는다.** 원본 이미지 비율을 유지해 중앙 확대하고 높이 방향의 초과분을 자른다. 확장 모드의 scale은 `max(W/(imageWidth−1),320/(imageHeight−1))`이며 LINEAR/REPEAT가 원본 이미지 가장자리 밖의 검정 POT 여백을 섞지 않게 반 texel 여유를 확보한다. 원본480 모드의 일러스트 배치·크기는 유지한다. 그림을 그린 뒤 HUD 변환·clip을 복원하므로 결과 버튼의 위치와 입력은 유지된다. 퇴장 중인 패널이 추가 월드 영역에 다시 보이지 않게 한다. 전환 중에도 월드를 그린다. 문은 HUD 변환·clip을 해제하고 전체 W×H 위에 네 패널로 한 벌만 그린다. 중앙 그림의 좌표·UV는 유지하고 바깥16px 철판 띠만 확장한다. 확장 패널의 바깥 UV는 반투명 테두리 안쪽1.5texel에서 시작한다. 닫힌 상태3·4에서는 상하 패널 표시 위치만223/101에 맞춰 소수점 틈을 없앤다. 원본 이동 상태와 음향·화면 교체 이벤트는 유지한다. 원본480 비교는 기존 sprite 경로를 사용한다. [검사·캡처](public/fullscreen-transitions-20261027.md). 중단된 결과 전환의 재개 메뉴는 일러스트보다 위에 그린다.

일부 원본 배경은 새 폭보다 작다. 확장 모드에서 배경만 `max(1,W/imageWidth,320/imageHeight)`로 균등 확대하고 끝부분 위치를 제한한다. 비율 왜곡·새 에셋·근거 없는 반복 타일은 없다. 탱크·지형·효과 크기는 바뀌지 않는다. 원본480은 배경 크기·parallax 수치도 유지한다.

CPU raster는 원본480 비교 전용이다. 확장 offscreen 진단은 실제GL 버퍼 snapshot을 사용한다. 제품 onscreen 경로는 계속 GL 직접 present이며 readback/UIImage를 매 프레임 만들지 않는다. `HMGLReferenceTests`의 원본8개와 확장/resize/clip4개를 함께 유지한다.

## 생성된 화면 확장 자원

모드·미션은 각 원본별 `ModeWide`/`MissionWide` 생성 배경의 바깥 사분면을 연결한다. 공통 MenuMetal은 슬롯에만 쓴다. 확장 타이틀·모드·미션의 원본 배경은 반 texel 테두리 UV를 유효 색에 고정하여 검정 POT 여백 혼입을 막는다. 중앙 UV와 원본480/sprite 경로는 유지한다. 화면별 자원 수명과 원본 정책을 우회하지 않는다. [검은줄 재현·검사](menu-joins-20261025.md).

사용자 승인으로 `Art/Extensions`의 별도 생성 이미지를 번들 `Extensions`에서 읽는다. 타이틀은 생성 이미지의 바깥 두 사분면만 원본과 같은 이동·회전·확대 변환으로 그린다. 원본 사각형 바깥20단위에만 가장자리 반사/알파 전환을 적용한다. 이 보조 draw에서만 GL_MODULATE를 쓰고 즉시 GL_REPLACE로 복원한다. 원본 sprite와 전투 렌더 규칙은 유지한다. 메뉴 원본 부분은 텍스처 clip으로 재배치하며 CPU bitmap을 만들지 않는다. 생성 자원도 화면 scope의 업로드·해제 규칙을 따른다. [적용·포맷 비용·증거](menu-art-extension-20261025.md).


## Mac 전체 화면 캡처 검사

CUA 창 캡처에 여백이 보이면 창·content view 크기, 원점, GL backing 크기를 확인한다. macOS 미리보기의 ‘스크린샷 찍기 → 전체 화면’으로 같은 앱의 전체 화면을 캡처해 대조한다. 전체 화면 전환 애니메이션이 끝난 뒤 캡처한다.

캡처 원본과 SHA-256·크기를 보존한다. 화면 가장자리 픽셀과 게임 표시를 함께 확인한다. 캡처 경로에서 생긴 축소·여백에 맞춰 게임 좌표나 출력 크기를 역보정하지 않는다. 시스템 알림 등이 포함된 원본은 로컬 증거로 보존하고 게시용 이미지와 구분한다. [2026-10-27 대조 결과](evidence/mac-fullscreen-capture-20261027.json).

## 실물 참조 유닛과 VTOL 자원

현실의 유닛은 `integrate-game-art`의 실물 사진·도면 참조 절차로 그린다. 채택된 V2는 보존하며, 2026-10-27 외곽선 수정본 `Art/VTOL/20261027-v3/`를 `Art/Extensions/VTOL_*.png`에 포함한다. HMMedia는 이3종만 로드 시 논리 폭112/108/116으로 고품질 축소하고 기존 업로드·캐시 경로를 사용한다. 기존 원본 에셋 디코딩과 GL 필터·블렌드는 유지한다. 높이는 기체 비율을 유지하며 발사점·노즐·피격 범위도 함께 줄인다. 불꽃은 후방 노즐 하나에 기존 BulletEffect2 첫 프레임을 합성한다. 기체와 같은 회전으로 노즐·발사점을 이동하고 불꽃은 충돌 범위에서 제외한다. [세부](vtol-expansion.md).

## 자동조준 표시·버튼 (정식·치트 공용)

M·C 버튼은 표시와 터치 판정에서 제거한다. A 하나로 자동/수동을 전환한다. 자동 모드에서만 발사·다음 표적 버튼을 표시한다.

세계 좌표의 적 경계 상자를 기존 카메라 변환으로 표시한다. 화면 밖 대상은 화면 가장자리 방향 표시로 대체한다. OpenGL의 선 그리기를 사용하며 새 UIKit 게임 렌더 경로를 만들지 않는다. 원본 점프·무기·메뉴·하단 HUD를 참조한 금속 버튼 시트 `Extensions/AutoAimControls.png`를 사용한다. 시트는 최초 로드 때138×92로 축소해 캐시한다. A·다음 표적·사격 순서로 HUD 원점 기준 x334/382/430, y10에46×46 크기로 배치하고 원래 하단 레일과 겹쳐 붙인다. HUD offset/scale을 적용한 동일한 `aimControlRect`를 그림·입력에 사용한다. 일시정지·메뉴·전환 문에서는 자동조준 조작부를 숨긴다.

사용자 지시로 버튼 순서를 A → 표적 전환 화살표 → 사격 조준경으로 바꿨다. 표적 네 모서리에 사선과 상태 표지를 연결했다. LOCKED는 조준 가능, TRACK은 추적 중, BLOCKED는 지형·사거리 등으로 발사할 수 없는 상태다. 표적 획득 때 사선을 0.15초 동안 그리고 표지를 한 번 점등한다. 화면 가장자리에서는 글자를 안쪽에 배치한다. 기존 OpenGL 선 그리기를 사용하며 프레임마다 파일을 쓰거나 이미지를 생성하지 않는다. 표시 계산은 `Sources/iOS/HMAimOverlay.hpp`, 실제 출력은 `HMGameView.drawAimControls`가 담당한다.

## 쉴카·퉁구스카 파츠

`Art/Extensions/GAA_*`의 원본 PNG를 보존한다. `HMMedia`가 `GroundAAArt.inc`의 영역을 잘라 논리 크기로 한 번 축소하고 캐시한다. 레이더는 24방향 이미지를 게임 시간 1/12초마다 바꾼다. 고정 받침은 포탑 그림에 있고, 회전부의 X·Y 축은 프레임별 앵커로 맞춘다. 차체 2프레임·포신 회전·반동·차체 흔들림은 기존 파츠 렌더링을 따른다. [구현과 게임 화면](ground-aa-expansion.md).

사용자가 확인한 소형 지상 유닛의 전체 크기와 부품 비율 기준은 [게임 기획](game-design.md)의 해당 절을 따른다. 쉴카 포탑51×29는 폭51을 유지한 세로 확대이며 원형 비율 보존의 승인 예외다. 퉁구스카의 차체85×22·포탑50×29를 후속 유사 크기 유닛의 우선 비교 대상으로 사용한다. 실제 합성에서는 비슷한 길이의 원작 적도 함께 표시한다.

## HUD 편집 렌더링 — 2026-10-29

기본 배치는 기존 전체 HUD 그림을 사용한다. 사용자 배치는 같은 원본 텍스처의 영역을 그리며, 원본 금속판 끝부분으로 레일을 마감한다. 계기판 바늘·기어·빛·숫자는 해당 묶음과 함께 이동한다. 편집 중 현재 전투의 renderItems를 다시 사용하고 collect·tick을 호출하지 않는다. 편집 도구의 SAMPLE SCENE에서는 별도 세션으로 만든 탱크·지형·지상 적·헬기 장면을 그린다. 편집 도구 버튼도 기존 게임 렌더러와 원작 버튼 절차를 사용한다. 추가 도구는 HMGameButtons의 정적 글자 시트와 원작 금속판을 사용한다.

### HUD 편집 진입 버튼 · 2026-10-29

멈춤 메뉴는 `RESUME GAME → SET GAME UI → SAVE & EXIT GAME → VISIT INDIEAN.COM` 순서다. 새 버튼은 원작 `GameMenuSub.png`의 글자와 바탕을 GL 영역 그리기로 조합한다. 원작 파일은 변경하지 않는다. 반투명 바탕은 글자 아래에 중복으로 그리지 않는다. 네 버튼은 같은256×40 영역, 세로45 간격이며 기존 슬라이드와 눌림 상태를 사용한다. 편집 적용·취소는 정지 메뉴로 돌아가고 재개 버튼만 전투를 재개한다.

### 편집 도구와 상단 장식 · 2026-10-29

사용자 배치에서도 GameUI_Top의 남은 금속판·레일을 upperTrim으로 그린다. 실제 옮긴 묶음에만 끝마감을 추가하며 기본 위치 재조립은 원본 픽셀과 일치한다. 편집의 선택선·터치 경계선은 표시하지 않는다. 오른쪽 영문 도구는 원작 Common_Btn_OK의 일반/눌림 금속판과 정적 글자 에셋을 게임 렌더러로 그린다. 런타임 CoreText·Canvas·시스템 글꼴 합성을 제거한다.

## 앱 아이콘 · 2026-10-29

앱 아이콘은 원작 일러스트에서 제목·부제를 제거한 공용 그림을 쓴다. Art/AppIcon/20261029에 편집 전·생성 원본·지시문을 보존하고 prepare_app_icons.py로 Apple asset catalog와 Android PNG를 파생한다. 게임 안의 타이틀·로고와 Resources/OriginalImages는 변경하지 않는다.

## 웹(WebGL2)

웹은 고정 파이프라인이 없어 `Sources/Web/WebGLRenderer.cpp`의 셰이더로 GLES 1.1 상태를 재현한다. 텍스처 환경 REPLACE/MODULATE를 RGBA·RGB565별로 구분하고, sprite blend 표·Trail의 setBlend 값·가위 영역·DITHER·POT LINEAR/REPEAT·touchHLE 감마 왕복·RGB565 패킹을 iOS와 같게 쓴다. 장면은 ceil(W)×ceil(H) RGBA8 renderbuffer에 그리고 glCopyTexSubImage2D로 LINEAR/CLAMP 텍스처에 복사해 최종 사각형으로 표시한다. WebGL은 선 폭을 1로 제한하므로 2픽셀 선은 사각형으로 그린다. 신규 기체·자동조준 버튼 시트의 1회 축소는 CoreGraphics 대신 면적 평균을 쓴다. [이식 문서](web-port-20261028.md).

HUD 편집 배치는 iOS `drawRegion`과 같은 영역 그리기(`GLRenderer::drawRegion`, GL_REPLACE, SRC_ALPHA/ONE_MINUS_SRC_ALPHA)로 원본 HUD 텍스처의 부분을 그린다. 편집기 도구 버튼도 원작 버튼 상태·게임 좌표·WebGL 영역 그리기로 처리한다. 게임 실행 후 DOM 버튼을 만들지 않는다.

## 게임 실행 후 버튼 절차 — 2026-11-02 사용자 지시

게임 실행 후 필요한 버튼은 모두 원작에서 복원한 QobButton 절차를 사용한다. 원작 스프라이트의 일반·눌림 상태를 기존 렌더러에 제출하고, 같은 좌표의 입력 영역과 기존 버튼 상태에 기능을 연결한다. HUD 도구·복원 진입과 확인창·키 설정·치트 패널·실행 중 오류 재시도도 이 규칙을 따른다.

원작 버튼 이미지·배치·기능과 멈춤 메뉴 SET GAME UI는 보존한다. 이번 교체 대상은 추가했던 네이티브·웹 버튼이다. 원작 에셋을 수정하지 않고, 새 문구에 필요한 정적 글자만 Art/Extensions에 추가한다.

텍스트 입력·IME·클립보드·파일 선택처럼 원작에 없는 기능에는 플랫폼 연결을 둔다. 이 기능을 호출하는 게임 버튼까지 UIKit·AppKit·Android View·DOM으로 만들지 않는다. 게임을 그릴 수 없는 초기화 실패와 게임 로드 전 설치·다운로드 화면은 실행기가 표시한다.

교체 구현과 검사 진행은 [현재 상태](current-status.md), 원작 보존과 파일별 변경 범위는 [버튼 조사](reviews/game-button-incident-20261101.md#game-button-replacement)를 따른다.

추가 버튼의 문구는 모든 플랫폼에서 영어로 표시한다. OS 언어에 따라 게임 버튼을 번역하지 않는다. 글자는 원작 Common_Btn_OK와 StageClearBtn의 금색 테두리·산세리프 형태를 참고한 정적 시트이며, 실행 중 시스템 글꼴로 합성하지 않는다.
