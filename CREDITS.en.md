<a id="제작자와-사용-자원"></a>

# Creators and resources

<!-- language-nav:start -->
[한국어](CREDITS.md) | [English](CREDITS.en.md)
<!-- language-nav:end -->

<a id="heavymach-원작"></a>

## The original HeavyMach

- Game development: **Byun Hae-jun (변해준)**
- Illustrations: **Park Jae-cheol (박재철)**
- Release credit in 2009: **IndieAn**
- IP acquisition in 2011: **WeMade Creative** — then a subsidiary of WeMade Entertainment
- Series relaunch announcement in 2011: **WeMade Entertainment**

The [2009 creator interview](https://www.koreajoongangdaily.com/lifestyle/korean-game-developers-shoot-for-success/10531072) and [release article](https://toucharcade.com/2009/02/10/first-look-at-heavy-mach-side-scrolling-tank-shooter/) identify the creators and releasing company. In 2011, WeMade Creative acquired IndieApps' assets, including the HeavyMach and HeavyMach 2 IP. [Acquisition report](https://zdnet.co.kr/view/?no=20110512162801). That year, WeMade Entertainment announced relaunches of Assault, Battle, and Defense. [Press release](https://www.newswire.co.kr/newsRead.php?no=575013)

This project reimplements HeavyMach's combat, progression, missions, and graphics for current Apple devices. My personal reasons for starting it and what I want to say to the original creators are in [the developer's statement](docs/en/RESTORATION.md).

<a id="재구현과-추가-개발"></a>

## Reimplementation and additions

I defined the comparisons with the original, the wider screen, platform and module structure, keyboard controls, auto-aim, and new units. Playing on real iPhones and iPads, I found problems and directed corrections to saving, graphics, ballistics, input, and art.

Codex performed behavior analysis, C++ and Objective-C++ implementation, image creation and integration, builds, tests, installation, and documentation. The incorrect implementations and their corrections are recorded in [troubleshooting](docs/en/public/troubleshooting.md).

Web development: [fastutlego](https://github.com/fastutlego), [PR #24](https://github.com/gelb2/HeavyMach/pull/24). The contribution implements a WebAssembly host using the existing C++ core, a WebGL2 renderer, browser input, audio, IndexedDB storage, HUD editing and automated tests. Original commits, authors and co-author records are retained; fixes found during review are recorded in separate Codex commits.

<a id="그림과-플레이-자료"></a>

## Art and gameplay media

The original images, audio, and maps are tracked in the [resource manifest](docs/build-resources.json). Codex used imagegen to create side extensions for menu backgrounds and artwork for additional aircraft. I reviewed and selected the drafts. The [VTOL](docs/public/vtol-art-and-loadout-20261027.md) and [fighter](docs/public/lux-lunae-design-20261027.md) records describe comparisons with aircraft photographs, drawings, and the game's palette, along with prompts and file hashes.

Codex extracted the README's 8 gameplay images, 2 GIFs, and 3 videos from my screen recordings. [Scenes, timestamps, and hashes](docs/public/media/gameplay-20261027/media-scenes.json)

<a id="라이브러리"></a>

## Libraries

| Library | Use | License text in this repository |
|---|---|---|
| OpenAL Soft | Sound-effect buffers, sources, and playback | [COPYING](vendor/openal-soft/COPYING), LGPL distribution terms |
| stb | Image processing | [LICENSE](vendor/stb/LICENSE), choice of MIT or Public Domain |

Newly written code uses [MIT](LICENSE). See the [code scope](LICENSE-SCOPE.en.md), [asset terms](ASSET-LICENSES.en.md), and [third-party notices](THIRD_PARTY_NOTICES.en.md) for the respective sources and terms.
