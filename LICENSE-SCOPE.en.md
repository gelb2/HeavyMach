<a id="코드-라이선스-적용-범위"></a>

# Code license scope

<!-- language-nav:start -->
[한국어](LICENSE-SCOPE.md) | [English](LICENSE-SCOPE.en.md)
<!-- language-nav:end -->

The MIT license in `LICENSE` applies to original software contributions made by Jun Young Jee to this restoration, only to the extent of the rights held by that contributor. Third-party code, original-game material, artwork, audio, game data, names and marks are excluded. Their existing rights and licenses are unchanged.

<a id="mit-적용-대상"></a>

## Code covered by MIT

New code written for this project uses [MIT](LICENSE). Keep the copyright and license notices when using, modifying, redistributing, or commercially using that code. Material taken from existing works follows the individual terms below.

| Path | Scope |
|---|---|
| `Sources/Core`, `Sources/Apple`, `Sources/iOS`, `Sources/Mac` | Newly written implementation. Excludes third-party code and expression derived from the original game |
| `Tests`, `tools` | Newly written test, build, and documentation tool code |
| `CMakeLists.txt`, `Modules`, `iOS`, `macOS`, `HeavyMach.xcworkspace` | Build and project settings written for this project. Included art and data follow the asset terms |
| `docs/public/media/badges/*.svg` | SVG code written for this README |

<a id="별도-조건이-적용되는-자료"></a>

## Materials with separate terms

- Original images, audio, maps, numerical data, additional artwork, and game captures and videos: [ASSET-LICENSES.md](ASSET-LICENSES.en.md).
- `vendor`, patches to external implementations, and copied skills: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.en.md). Retain each file's existing notices.
- `Analysis`, `Resources/Evidence`, `Sources/OriginalInterfaces`, `Compatibility`: analysis of the original game, external execution tools, and references. Excluded from the root MIT license.
- `Sources/iOS/HMCompatibilityPixels.hpp`: references touchHLE's gamma conversion. It uses MPL-2.0 and is excluded from the root MIT license. Follow the [third-party notices](THIRD_PARTY_NOTICES.en.md).
- The developer's statement, interview source texts, documentation prose, and quoted material are not collectively relicensed under the code license.

The rights granted by MIT cover this project's new code contributions. See [asset terms](ASSET-LICENSES.en.md) for publishing or distributing the app with the original name, graphics, and audio. The [distribution research](docs/public/github-licenses-and-badges-20261028.md) records the investigation into publishing and distributing the code and assets.
