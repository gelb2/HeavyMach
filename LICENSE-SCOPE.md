# 코드 라이선스 적용 범위

<!-- language-nav:start -->
[한국어](LICENSE-SCOPE.md) | [English](LICENSE-SCOPE.en.md)
<!-- language-nav:end -->

The MIT license in `LICENSE` applies to original software contributions made by Jun Young Jee to this restoration, only to the extent of the rights held by that contributor. Third-party code, original-game material, artwork, audio, game data, names and marks are excluded. Their existing rights and licenses are unchanged.

## MIT 적용 대상

이 프로젝트에서 새로 작성한 코드에 [MIT](LICENSE)를 적용한다. 해당 코드의 사용·수정·재배포·상업 이용에는 저작권과 라이선스 고지를 유지한다. 기존 저작물에서 가져온 부분에는 아래의 개별 조건이 적용된다.

| 경로 | 적용 범위 |
|---|---|
| `Sources/Core`, `Sources/Apple`, `Sources/iOS`, `Sources/Mac` | 새로 작성한 구현 부분. 외부 코드·원작에서 유래한 표현은 제외 |
| `Android`, `Sources/Web`, `Web`, `Distribution/Web`, `Desktop/Tauri` | 직접 작성한 플랫폼 연결·실행기·배포 코드와 설정. 외부 라이브러리·도구·기여자의 기존 고지와 원작 자원 조건은 유지 |
| `Tests`, `tools` | 새로 작성한 검사·빌드·문서 도구의 코드 |
| `CMakeLists.txt`, `Modules`, `iOS`, `macOS`, `HeavyMach.xcworkspace` | 직접 작성한 빌드·프로젝트 설정. 포함된 그림·데이터는 자원 조건 적용 |
| `docs/public/media/badges/*.svg` | 이번 README용으로 작성한 SVG 코드 |

## 별도 조건이 적용되는 자료

- 원작 이미지·음원·맵·수치 데이터, 추가 그림, 게임 캡처·영상: [ASSET-LICENSES.md](ASSET-LICENSES.md).
- `vendor`, 외부 구현 패치와 복사한 스킬: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). 각 파일의 기존 고지를 유지한다.
- `Analysis`, `Resources/Evidence`, `Sources/OriginalInterfaces`, `Compatibility`: 원작 분석 결과·외부 실행 도구·참고 자료. 루트 MIT의 적용 대상에서 제외한다.
- `Sources/iOS/HMCompatibilityPixels.hpp`: touchHLE의 감마 변환을 참조한 파일이다. MPL-2.0을 적용하고 루트 MIT에서 제외한다. [외부 고지](THIRD_PARTY_NOTICES.md)를 따른다.
- 개발자의 변, 인터뷰 원문, 문서 본문과 인용 자료: 코드 라이선스로 일괄 재허락하지 않는다.

MIT가 부여하는 권리는 이 프로젝트의 새 코드 기여분에 한정된다. 원작 이름·그림·음원과 함께 앱을 공개하거나 배포하는 범위는 [자원 이용 조건](ASSET-LICENSES.md)에서 확인한다. 코드·에셋 공개와 배포 조건 조사 기록은 [배포 조사](docs/public/github-licenses-and-badges-20261028.md)에 있다.
