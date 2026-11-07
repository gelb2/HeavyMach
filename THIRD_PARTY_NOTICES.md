# 외부 코드와 라이선스 고지

<!-- language-nav:start -->
[한국어](THIRD_PARTY_NOTICES.md) | [English](THIRD_PARTY_NOTICES.en.md)
<!-- language-nav:end -->

## 앱에 연결되는 라이브러리

| 구성요소 | 경로·버전 | 라이선스·고지 |
|---|---|---|
| OpenAL Soft | `vendor/openal-soft`, 1.22.2 | 주요 소스는 LGPL-2.0-or-later. 각 파일의 저작권 고지 유지. [원문](vendor/openal-soft/COPYING), [고지 사본](LICENSES/OpenAL-Soft-LGPL-2.0.txt), [공식 저장소](https://github.com/kcat/openal-soft) |
| stb | `vendor/stb` | 저장소 원문은 MIT/Public Domain 선택형. 이 프로젝트에서는 MIT 조건을 사용. Copyright © 2017 Sean Barrett. [원문](vendor/stb/LICENSE), [고지 사본](LICENSES/stb.txt), [공식 저장소](https://github.com/nothings/stb) |

OpenAL Soft는 [빌드 스크립트](tools/build_openal.sh)에서 정적 라이브러리로 빌드한다. 배포할 앱과 같은 버전의 라이브러리 소스·수정분·설정을 제공하고, 수정한 라이브러리로 다시 링크할 수 있는 앱 소스 또는 오브젝트 등의 자료를 준비한다. 라이브러리 이용 부분의 수정과 디버깅을 위한 역분석을 금지하는 조건을 덧붙이지 않는다. 소스와 고지는 저장소에 있다. 재링크 명령과 실행 결과는 [소스 릴리스 검사](docs/public/github-followup-20261028.md)에 있다. 세부 조건은 LGPL 원문 제6절과 [조사](docs/public/github-licenses-and-badges-20261028.md)를 따른다.

## Android·웹·Tauri의 외부 구성요소

| 구성요소 | 사용 위치 | 라이선스·고지 |
|---|---|---|
| Gradle 8.13·Wrapper | [Android 빌드](Android/gradle/wrapper/gradle-wrapper.properties) | Apache-2.0. [라이선스와 포함 구성요소](LICENSES/Gradle-8.13-LICENSE.txt), [NOTICE](LICENSES/Gradle-8.13-NOTICE.txt), [공식 원문](https://github.com/gradle/gradle/blob/v8.13.0/LICENSE) |
| Emscripten | [Web 빌드](Web/CMakeLists.txt), WebAssembly·JavaScript 실행 코드 | MIT 또는 University of Illinois/NCSA. [고지 사본](LICENSES/Emscripten.txt), [공식 원문](https://github.com/emscripten-core/emscripten/blob/main/LICENSE). 포함된 musl·외부 구성요소는 각 소스의 고지를 따른다. |
| Tauri 2.12.5 | [Mac 실행기](Desktop/Tauri/src-tauri/Cargo.toml) | MIT 또는 Apache-2.0. [MIT 원문](LICENSES/Tauri-LICENSE-MIT.txt), [Apache-2.0 원문](LICENSES/Tauri-LICENSE-APACHE-2.0.txt), [공식 저장소](https://github.com/tauri-apps/tauri). |

Tauri의 Rust 의존성 버전은 [Cargo.lock](Desktop/Tauri/src-tauri/Cargo.lock), CLI와 웹 검사 도구 버전은 [package-lock.json](Desktop/Tauri/package-lock.json)에 기록한다. 각 의존성의 저작권·라이선스는 해당 패키지의 고지를 따른다. 위 표는 직접 사용하는 주요 도구·런타임 목록이다.

## 외부 구현을 참조하거나 보관한 자료

| 경로 | 출처 | 적용 상태 |
|---|---|---|
| `Compatibility`의 외부 실행 도구 패치 | touchHLE 계열. 정확한 저장소·커밋은 [upstream.json](Compatibility/upstream.json) | touchHLE의 MPL-2.0. [라이선스 사본](LICENSES/touchHLE-MPL-2.0.txt). fork에서 추가한 게임별 자료는 별도 출처 조사 대상. 루트 MIT 적용 제외 |
| `Sources/iOS/HMCompatibilityPixels.hpp` | 주석에 touchHLE의 이미지·CoreGraphics 경로를 명시 | MPL-2.0. 감마 변환·정수화 순서를 C++ lookup table로 구현. 루트 MIT 적용 제외 |
| `.agents/skills/engineering-{debug,review,test}/references` | Addy Osmani의 스킬 원문 | Copyright © 2025 Addy Osmani. 각 폴더의 MIT 원문 보존. [고지 사본](LICENSES/engineering-skills-MIT.txt) |
| `.agents/skills/write-project-text/references/upstream` | SaaS·Candidate 작업 지침 | [복사 출처와 해시](.agents/skills/write-project-text/references/source-manifest.json) 보존. 코드용 MIT 적용 제외 |
| `Analysis`, `Sources/OriginalInterfaces`, `Resources/Evidence` | 원작 동작·인터페이스 분석 자료 | 원작 유래 자료. 코드용 MIT 적용 제외 |

외부 라이선스는 해당 파일의 고지가 우선한다. 원작 이미지·음원은 [자원 조건](ASSET-LICENSES.md), 새 코드의 범위는 [코드 라이선스](LICENSE-SCOPE.md)에 있다.

## 참조 소스 대조 — 2026-10-28

`HMCompatibilityPixels.hpp`는 256개 입력값에 대해 `powf(powf(x, 2.2), 1/2.2)`를 계산하고 바이트로 자른다. [참조 커밋의 image.rs](https://github.com/moleworld-dev/MoleWorld-5.5.0-touchHLE-offline/blob/ebf9433e2bfd32071545bf49484b1345d15522a3/fresh-port/20-touchHLE-src/touchHLE/src/image.rs)의 `gamma_decode`·`gamma_encode`와 `cg_bitmap_context.rs`의 변환 순서를 대조했다. 현재 C++ 구현에는 MPL-2.0을 표시했다. [대조 파일 해시](docs/evidence/publication-source-audit-20261028.json)

`Compatibility/patches/rust-sdl2.patch`의 대상은 `b67f98fe6a147773b6f05d50ae775d9fe16561e7`의 `sdl2-sys/build.rs`다. 해당 프로젝트는 Copyright © 2013 Mozilla Foundation의 MIT를 사용한다. [고지 사본](LICENSES/rust-sdl2-MIT.txt), [참조 저장소](https://github.com/touchHLE/rust-sdl2/tree/b67f98fe6a147773b6f05d50ae775d9fe16561e7).
