<a id="외부-코드와-라이선스-고지"></a>

# Third-party code and license notices

<!-- language-nav:start -->
[한국어](THIRD_PARTY_NOTICES.md) | [English](THIRD_PARTY_NOTICES.en.md)
<!-- language-nav:end -->

<a id="앱에-연결되는-라이브러리"></a>

## Libraries linked into the app

| Component | Path and version | License and notices |
|---|---|---|
| OpenAL Soft | `vendor/openal-soft`, 1.22.2 | Main sources: LGPL-2.0-or-later. Retain each file's copyright notices. [Original](vendor/openal-soft/COPYING), [notice copy](LICENSES/OpenAL-Soft-LGPL-2.0.txt), [official repository](https://github.com/kcat/openal-soft) |
| stb | `vendor/stb` | Choice of MIT/Public Domain in the upstream text. This project uses the MIT terms. Copyright © 2017 Sean Barrett. [Original](vendor/stb/LICENSE), [notice copy](LICENSES/stb.txt), [official repository](https://github.com/nothings/stb) |

The [build script](tools/build_openal.sh) builds OpenAL Soft as a static library. Provide the library source, modifications, and configuration matching the distributed app, together with app source or object files that allow relinking with a modified library. Do not add terms prohibiting modification of the library-using portions or reverse engineering to debug those modifications. The source and notices are in the repository. Relinking commands and results are in the [source release checks](docs/public/github-followup-20261028.md). See section 6 of the LGPL text and the [research](docs/public/github-licenses-and-badges-20261028.md) for details.

<a id="android웹tauri의-외부-구성요소"></a>

## External components for Android, web, and Tauri

| Component | Use | License and notices |
|---|---|---|
| Gradle 8.13 and Wrapper | [Android build](Android/gradle/wrapper/gradle-wrapper.properties) | Apache-2.0. [License and bundled components](LICENSES/Gradle-8.13-LICENSE.txt), [NOTICE](LICENSES/Gradle-8.13-NOTICE.txt), [upstream text](https://github.com/gradle/gradle/blob/v8.13.0/LICENSE) |
| Emscripten | [Web build](Web/CMakeLists.txt), WebAssembly and JavaScript runtime code | MIT or University of Illinois/NCSA. [Notice copy](LICENSES/Emscripten.txt), [upstream text](https://github.com/emscripten-core/emscripten/blob/main/LICENSE). Bundled musl and other components retain the notices in their sources. |
| Tauri 2.12.5 | [Mac launcher](Desktop/Tauri/src-tauri/Cargo.toml) | MIT or Apache-2.0. [MIT text](LICENSES/Tauri-LICENSE-MIT.txt), [Apache-2.0 text](LICENSES/Tauri-LICENSE-APACHE-2.0.txt), [upstream repository](https://github.com/tauri-apps/tauri). |

Tauri's Rust dependency versions are recorded in [Cargo.lock](Desktop/Tauri/src-tauri/Cargo.lock); CLI and web test tool versions are recorded in [package-lock.json](Desktop/Tauri/package-lock.json). Each dependency retains its own copyright and license notices. The table lists the main tools and runtimes used directly by this project.

<a id="외부-구현을-참조하거나-보관한-자료"></a>

## Referenced or retained external implementations

| Path | Source | Applicable terms |
|---|---|---|
| External execution-tool patches in `Compatibility` | touchHLE family; exact repository and commit in [upstream.json](Compatibility/upstream.json) | touchHLE's MPL-2.0. [License copy](LICENSES/touchHLE-MPL-2.0.txt). Game-specific material added in the fork requires separate source investigation. Excluded from root MIT |
| `Sources/iOS/HMCompatibilityPixels.hpp` | Comments identify touchHLE image and CoreGraphics paths | MPL-2.0. Gamma conversion and integer truncation implemented as a C++ lookup table. Excluded from root MIT |
| `.agents/skills/engineering-{debug,review,test}/references` | Original skills by Addy Osmani | Copyright © 2025 Addy Osmani. MIT originals retained in each folder. [Notice copy](LICENSES/engineering-skills-MIT.txt) |
| `.agents/skills/write-project-text/references/upstream` | SaaS and Candidate working instructions | [Copy sources and hashes](.agents/skills/write-project-text/references/source-manifest.json) retained. Excluded from code MIT |
| `Analysis`, `Sources/OriginalInterfaces`, `Resources/Evidence` | Analysis of original behavior and interfaces | Material derived from the original game. Excluded from code MIT |

For external licenses, the notice in the relevant file takes precedence. See [asset terms](ASSET-LICENSES.en.md) for original art and audio, and [code license scope](LICENSE-SCOPE.en.md) for new code.

<a id="참조-소스-대조--2026-10-28"></a>

## Reference source comparison — 2026-10-28

`HMCompatibilityPixels.hpp` evaluates `powf(powf(x, 2.2), 1/2.2)` for 256 input values and truncates the results to bytes. The conversion order was compared with `gamma_decode` and `gamma_encode` in [image.rs at the reference commit](https://github.com/moleworld-dev/MoleWorld-5.5.0-touchHLE-offline/blob/ebf9433e2bfd32071545bf49484b1345d15522a3/fresh-port/20-touchHLE-src/touchHLE/src/image.rs) and with `cg_bitmap_context.rs`. The current C++ implementation is marked MPL-2.0. [Compared file hashes](docs/evidence/publication-source-audit-20261028.json)

`Compatibility/patches/rust-sdl2.patch` targets `sdl2-sys/build.rs` at `b67f98fe6a147773b6f05d50ae775d9fe16561e7`. That project uses MIT, Copyright © 2013 Mozilla Foundation. [Notice copy](LICENSES/rust-sdl2-MIT.txt), [reference repository](https://github.com/touchHLE/rust-sdl2/tree/b67f98fe6a147773b6f05d50ae775d9fe16561e7).
