<a id="변경-이력"></a>

# Changelog

<!-- language-nav:start -->
[한국어](CHANGELOG.md) | [English](CHANGELOG.en.md)
<!-- language-nav:end -->

[Latest release](https://github.com/gelb2/HeavyMach/releases/latest) · [All GitHub releases](https://github.com/gelb2/HeavyMach/releases)

<a id="다음-릴리스-웹판"></a>

## Tauri Mac download — 2026-11-02

[Download](https://github.com/gelb2/HeavyMach/releases/tag/tauri-mac-20261102) · Apple Silicon, macOS 14 or later. The ZIP includes the app, license notices and source link. App version is 0.17.0; game bundle is `3665a237a9dccd69`. The ad-hoc signed app passed launch, slot, recovery-input and quit checks after extracting the ZIP. Developer ID notarization and first launch on another Mac remain untested. The stable v0.17 release is unchanged.

## Main integration — Web, PWA, Tauri and recovery codes

Built on [fastutlego](https://github.com/fastutlego)'s [PR #24](https://github.com/gelb2/HeavyMach/pull/24), this adds browser deployment, an offline PWA and a Tauri Apple Silicon Mac host. The contributor's commits and authorship remain intact.

- Install-first launcher, verified download progress, offline cache and updates.
- Tauri OS-file backups, recovery and save completion before quitting. Public signing/notarization and Windows builds remain pending.
- Shared C++ recovery codes for three slots, HUD, keys and aim settings. Apple/Android adapters and build files change; the existing combat core and assets do not.
- 14 iPhone and 40 Galaxy functional checks passed. A retained 19,344-byte iPhone button source-image cache was released.
- Added HUD tools, recovery controls, key settings and cheat controls now use the original game-button procedure. Text entry, clipboard and file selection use platform services. Original buttons and combat code remain unchanged.
- In-game recovery controls stay visible in Android PWA fullscreen. New English labels use a static sprite sheet based on the original OK and mission-selection buttons.
- Physical iPhone/iPad/Galaxy and browser/PWA checks cover editing, recovery, keyboard input and original gameplay. Native Mac frame-time checks exceeded the budget, including original title and combat scenes. [Changes, screenshots and test results (Korean)](docs/reviews/game-button-incident-20261101.md#game-button-results).

[Platform checks (Korean)](docs/reviews/mac-web-pwa-comparison-20261101.md) · [Recovery module (Korean)](Modules/SaveTransfer/README.md). Version tags and installer releases are recorded separately.

| Version | Tag date | Main changes |
|---|---|---|
| [v0.17](#v017) | 2026-10-30 | Android apps and upgrade-window fix |
| [v0.16](#v016) | 2026-10-29 | Lux Lunae entry placement |
| [v0.15](#v015) | 2026-10-29 | App icons and HUD editor entry |
| [v0.14](#v014) | 2026-10-29 | HUD layout editing |
| [v0.13](#v013) | 2026-10-28 | Shilka and Tunguska |
| [v0.12](#v012) | 2026-10-27 | Lux Lunae fighters |
| [v0.11](#v011) | 2026-10-27 | Auto-aim and full-screen doors |
| [v0.10](#v010) | 2026-10-27 | Apple platforms, keyboard and VTOLs |
| [v0.9](#v09) | 2026-10-26 | iPhone restoration |

<a id="unreleased"></a>

<a id="v017"></a>

## v0.17 — 2026-10-30

[Release and downloads](https://github.com/gelb2/HeavyMach/releases/tag/v0.17) · [Source tag](https://github.com/gelb2/HeavyMach/tree/v0.17) · [Compare with previous version](https://github.com/gelb2/HeavyMach/compare/v0.16...v0.17)

<a id="android-일반치트-앱-추가"></a>

### Regular and cheat Android apps

Added Android apps using the existing C++ game core, images, audio and maps. Kotlin, JNI and an Android C++ host connect touch, tilt, keyboard, menus, HUD, EGL/GLES graphics, OpenAL audio and saves. Auto-aim, added aircraft and ground units, and HUD editing are available. App interruption and audio-focus loss pause play; graphics resources are restored when the context is recreated.

- Installed regular and cheat APKs on Galaxy SM-S731N over USB and Wi-Fi. Passed 31 checks covering movement, jumping and 20 weapons, plus 11 wireless checks after disconnecting USB.
- Fixed combat continuing behind an unresponsive upgrade window when entering 2-1 after the first boss. Passed 12 regression checks on each of Galaxy and emulator.
- Published 8 images, 3 GIFs and 3 videos from my play of missions 3-4 and 3-5 on the fixed build. I found no malfunctions during my subsequent play.
- During Android implementation, existing Apple sources, shared core, resources and build configuration were preserved. All four regular/cheat iOS Simulator and native Mac builds passed.
- Current APKs use optimized code and debug signing. Their internal app version remains `0.16-android-dev`; the source release tag is v0.17. Full campaign completion, extended thermal and memory tests, Android tablets, physical 16KB-page devices and distribution signing remain.

[Android installation](docs/en/public/build-and-install.md#android-빌드와-설치) · [Development, shared modules and fixes](docs/public/android-development-20261030.md) · [Galaxy gameplay](docs/public/android-gameplay-20261030.md) · [Validation](docs/en/public/validation.md#android-검사--2026-10-30)

[Source and resource archive](https://github.com/gelb2/HeavyMach/releases/download/v0.17/HeavyMach-v0.17-source-with-assets.tar.gz) · [Notices ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.17/HeavyMach-v0.17-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.17/SHA256SUMS.txt)

<a id="v016"></a>

## v0.16 — 2026-10-29

[Release and downloads](https://github.com/gelb2/HeavyMach/releases/tag/v0.16) · [Source tag](https://github.com/gelb2/HeavyMach/tree/v0.16) · [Release commit 0dfb2a02](https://github.com/gelb2/HeavyMach/commit/0dfb2a02dbdfe356087b789f3ab03476bd94ce02) · [Compare with previous version](https://github.com/gelb2/HeavyMach/compare/v0.15...v0.16)

Lux Lunae spawn points and entry destinations are spread horizontally and vertically. This fixes overlapping aircraft during entry while preserving numbers, weapons, speed and post-entry AI.

- Tested simultaneous-entry spacing, terrain and ceiling across 140 conditions: 14 reinforcement sections, five screen sizes and two player heights.
- Passed 23 core tests and three functional tests each on iPhone and iPad. Regular and cheat apps were updated wirelessly on both devices, preserving saves and HUD layouts.
- Frame tests passed for iPhone entry/combat and iPad combat. iPad entry callback-interval p95 was 21.39ms, above the 20ms threshold. Comparison with the same pre-fix conditions remains.
- Aircraft can regroup during combat under existing AI.
- Added app termination and Home-screen checks after testing to the wireless workflow. Mac and iOS Simulator support is Apple Silicon arm64.

The attached source-and-resource archive contains 2,672 tracked files. Extraction into a fresh folder passed 1,504 resource hashes and document-link checks.

[Placement fix and device tests](https://github.com/gelb2/HeavyMach/blob/v0.16/docs/public/lux-formation-plan-20261029.md) · [Build and install](https://github.com/gelb2/HeavyMach/blob/v0.16/docs/public/build-and-install.md) · [Tasks remaining at release](https://github.com/gelb2/HeavyMach/blob/v0.16/docs/usability-todo.md)

[Source and resource archive](https://github.com/gelb2/HeavyMach/releases/download/v0.16/HeavyMach-v0.16-source-with-assets.tar.gz) · [Notices ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.16/HeavyMach-v0.16-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.16/SHA256SUMS.txt)

<a id="v015"></a>

## v0.15 — 2026-10-29

[Release and downloads](https://github.com/gelb2/HeavyMach/releases/tag/v0.15) · [Source tag](https://github.com/gelb2/HeavyMach/tree/v0.15) · [Release commit a442e69e](https://github.com/gelb2/HeavyMach/commit/a442e69e0f340dc76b0ca2d90208f47241194f52) · [Compare with previous version](https://github.com/gelb2/HeavyMach/compare/v0.14...v0.15)

Removed text from the app icon and applied it to regular and cheat apps on iPhone, iPad and Mac. HUD editing now opens from **SET GAME UI** in the combat pause menu; its mission-selection button was removed.

- Added final Shilka, Tunguska and HUD images and captions to GitHub/SaaS writing material.
- Incorporated author-supplied passages in the README and developer's statement.
- Passed three tests each on iPhone/iPad Simulators, two native Mac tests and CI builds for four Apple apps. Regular and cheat apps were installed and launched wirelessly on both physical devices.
- Prepared Android icon PNGs. APK implementation and icon integration remained future work.

The attached source-and-resource archive contains 2,652 tracked files. Extraction into a fresh folder passed 1,504 resource hashes and document-link checks.

[Icons and screenshots](https://github.com/gelb2/HeavyMach/blob/v0.15/docs/public/app-icon-and-media-20261029.md) · [Build and install](https://github.com/gelb2/HeavyMach/blob/v0.15/docs/public/build-and-install.md) · [Tasks remaining at release](https://github.com/gelb2/HeavyMach/blob/v0.15/docs/usability-todo.md)

[Source and resource archive](https://github.com/gelb2/HeavyMach/releases/download/v0.15/HeavyMach-v0.15-source-with-assets.tar.gz) · [Notices ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.15/HeavyMach-v0.15-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.15/SHA256SUMS.txt)

<a id="v014"></a>

## v0.14 — 2026-10-29

[Release and downloads](https://github.com/gelb2/HeavyMach/releases/tag/v0.14) · [Source tag](https://github.com/gelb2/HeavyMach/tree/v0.14) · [Release commit 9025b585](https://github.com/gelb2/HeavyMach/commit/9025b585fb4777f02255b048a74a26b6f5f854e9) · [Compare with previous version](https://github.com/gelb2/HeavyMach/compare/v0.13...v0.14)

Added an editor for moving HUD buttons, gauges and upper decorations. In this version, **SET GAME UI** opens from mission selection or the combat pause menu.

- Artwork and hit areas move together. Menu, jump, weapon-switch and auto-aim buttons work in their new positions.
- Edit over paused combat or a sample battle. Reset, cancel and apply are available.
- English metal-styled tools sit on the right. Dragging collapses the panel; TOOLS reopens it.
- Upper decorations are preserved and movable separately. Existing layout files load; changing the layout and pressing APPLY saves once.
- Passed 13 tests each on iPhone/iPad, 24 core tests and two native Mac tests. Regular and cheat apps were installed wirelessly on both devices.
- Mac frame-processing investigation and long-session heat testing remained.

The attached source-and-resource archive contains 2,626 tracked files. Extraction into a fresh folder passed 1,498 resource hashes.

[HUD screens and tests](https://github.com/gelb2/HeavyMach/blob/v0.14/docs/public/hud-improvement-plan-20261028.md) · [Build and install](https://github.com/gelb2/HeavyMach/blob/v0.14/docs/public/build-and-install.md) · [Tasks remaining at release](https://github.com/gelb2/HeavyMach/blob/v0.14/docs/usability-todo.md)

[Source and resource archive](https://github.com/gelb2/HeavyMach/releases/download/v0.14/HeavyMach-v0.14-source-with-assets.tar.gz) · [Notices ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.14/HeavyMach-v0.14-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.14/SHA256SUMS.txt)

<a id="v013"></a>

## v0.13 — 2026-10-28

[Release and downloads](https://github.com/gelb2/HeavyMach/releases/tag/v0.13) · [Source tag](https://github.com/gelb2/HeavyMach/tree/v0.13) · [Release commit bc4e80c0](https://github.com/gelb2/HeavyMach/commit/bc4e80c0aa4920adbb09e7dc9c9d9b26fe39d888) · [Compare with previous version](https://github.com/gelb2/HeavyMach/compare/v0.12...v0.13)

Added Shilka and Tunguska ground enemies: 152 vehicles across 35 stages, preserving existing placements.

- Shilka fires six white rounds in succession. Tunguska fires four white rounds and two missiles.
- Revised hull, turret and gun parts and textures. Radar rotates independently through 24 directional images over a fixed mount.
- At the developer's request, Shilka's turret height matches Tunguska's at 29.
- The final build passed 23 core tests, two iPad Simulator tests and five tests each on iPhone/iPad. Regular and cheat apps were installed wirelessly on both devices.
- Full-campaign difficulty with added enemies and long-session heat/memory testing remained.

The attached source-and-resource archive contains 2,555 tracked files. It passed 1,498 resource hashes.

[Ground-unit images and revisions](https://github.com/gelb2/HeavyMach/blob/v0.13/docs/public/ground-aa-art-20261028.md) · [Build and install](https://github.com/gelb2/HeavyMach/blob/v0.13/docs/public/build-and-install.md) · [Tasks remaining at release](https://github.com/gelb2/HeavyMach/blob/v0.13/docs/usability-todo.md)

[Source and resource archive](https://github.com/gelb2/HeavyMach/releases/download/v0.13/HeavyMach-v0.13-source-with-assets.tar.gz) · [Notices ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.13/HeavyMach-v0.13-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.13/SHA256SUMS.txt)

<a id="v012"></a>

## v0.12 — 2026-10-27

[Release and downloads](https://github.com/gelb2/HeavyMach/releases/tag/v0.12) · [Source tag](https://github.com/gelb2/HeavyMach/tree/v0.12) · [Release commit eedf245f](https://github.com/gelb2/HeavyMach/commit/eedf245f859a5224adf8f1c4a4b010eda1b5a5bb) · [Compare with previous version](https://github.com/gelb2/HeavyMach/compare/v0.11...v0.12)

Added F-22, Su-57 and MiG-31 fighters in Lux Lunae colors. They add 72 reinforcements across 14 sections, preserving existing helicopter and VTOL placements.

- Gold cockpits and HP 765. Aircraft fire either three single missiles or three pairs.
- Boss fights receive 2, 4, 6, then 8 aircraft as missions progress. The preceding mission's final section receives four.
- Changed entry to spawn the entire aircraft off-screen with no firing during entry.
- Reduced the missile firing-rate increase from 25% to 10%. The 25% missile-speed increase remained.
- Passed 21 core tests, four Simulator tests, four each on iPhone/iPad and five native Mac tests. Later entry fixes were also checked per platform, and both devices' regular/cheat apps were updated.
- The 25,200-frame manual-firing record matched the pre-auto-aim baseline. Full-campaign added-enemy difficulty and long-session heat/memory tests remained.

On October 28, the source-and-resource archive was extracted into a fresh folder and passed 1,482 resource checks, 21 core tests and four regular/cheat app builds for iOS Simulator and Mac. Relinking after modifying OpenAL also passed. The archive contains the tag's 2,366 tracked files. The notices ZIP contains notices prepared for publication and relinking test tools.

[Fighter implementation and tests](https://github.com/gelb2/HeavyMach/blob/v0.12/docs/lux-lunae-plan-20261027.md) · [Designs and gameplay](https://github.com/gelb2/HeavyMach/blob/v0.12/docs/public/lux-lunae-design-20261027.md) · [Build and install](https://github.com/gelb2/HeavyMach/blob/v0.12/docs/public/build-and-install.md) · [Tasks remaining at release](https://github.com/gelb2/HeavyMach/blob/v0.12/docs/usability-todo.md)

[October 28 source-archive build checks](https://github.com/gelb2/HeavyMach/blob/78d950c7a3880be7147e1717496854b1e412ea2e/docs/public/github-followup-20261028.md)

[Source and resource archive](https://github.com/gelb2/HeavyMach/releases/download/v0.12/HeavyMach-v0.12-source-with-assets.tar.gz) · [Notices ZIP](https://github.com/gelb2/HeavyMach/releases/download/v0.12/HeavyMach-v0.12-notices.zip) · [SHA256SUMS](https://github.com/gelb2/HeavyMach/releases/download/v0.12/SHA256SUMS.txt)

<a id="v011"></a>

## v0.11 — 2026-10-27

[Release and downloads](https://github.com/gelb2/HeavyMach/releases/tag/v0.11) · [Source tag](https://github.com/gelb2/HeavyMach/tree/v0.11) · [Release commit f0c4432f](https://github.com/gelb2/HeavyMach/commit/f0c4432f073a2b5bdea02026f279fbd91bb7b093) · [Compare with previous version](https://github.com/gelb2/HeavyMach/compare/v0.10...v0.11)

Added auto-aim and full-screen door transitions. Auto-aim can be toggled in the regular app and defaults to OFF.

- Added target selection/cycling, barrel adjustment and a fire button. Buttons match the existing HUD, with LOCKED, TRACK and BLOCKED indicators.
- Removed C hit correction and its button. Auto-aim does not override projectile speed, spread or flight paths.
- With auto-aim OFF, 20 weapons × 3 random seeds × 420 frames—25,200 frames total—matched the pre-feature code byte for byte.
- The final C-removal build passed 20 core tests, four Simulator input tests and four input tests each on iPhone/iPad. Regular and cheat apps were updated on both devices.
- Preserved the central door ornament while extending outer metal panels to the screen edges. Fixed exposed background behind closed doors and seam gaps.
- Display-timing threshold failures and long-session heat tests remained.

Download the tag's source and Git LFS resources, then build in Xcode. Regular and cheat apps are available for iPhone, iPad and native Mac.

[Auto-aim tests](https://github.com/gelb2/HeavyMach/blob/v0.11/docs/public/auto-aim-ipad-20261027.md) · [Door transitions](https://github.com/gelb2/HeavyMach/blob/v0.11/docs/public/fullscreen-transitions-20261027.md) · [Build and install](https://github.com/gelb2/HeavyMach/blob/v0.11/docs/public/build-and-install.md) · [Tasks remaining at release](https://github.com/gelb2/HeavyMach/blob/v0.11/docs/usability-todo.md)

<a id="v010"></a>

## v0.10 — 2026-10-27

[Release and downloads](https://github.com/gelb2/HeavyMach/releases/tag/v0.10) · [Source tag](https://github.com/gelb2/HeavyMach/tree/v0.10) · [Release commit 76cbd21c](https://github.com/gelb2/HeavyMach/commit/76cbd21c17d077d385be5a4695b0a598fc8e9aba) · [Compare with previous version](https://github.com/gelb2/HeavyMach/compare/v0.9...v0.10)

Added iPad, native Mac and keyboard controls. Shared core, Apple runtime and resources are managed in one workspace. Required game and test resources were added to Git LFS.

- Added an AppKit Mac app. iPad supports touch, tilt and keyboard together.
- Keyboard controls cover movement, jumping, aiming, firing, weapon switching and menus, with remapping.
- Fixed Mac development signing, presentation waits and full-screen issues. Passed 33 Mac tests with zero leaks.
- Verified wireless installation, launch and LLDB on iPhone/iPad. Each device passed 84 of 85 functional tests; one display-time comparison failed on each.
- Added 113 Yak-141, Harrier GR3 and F-35B aircraft across 14 sub-missions. Preserved 1,182 original enemy entries.
- Revised outlines and sizes of the approved realistic designs and retained only rear flames. Passed 60 VTOL hit tests and new-aircraft display tests on both devices; updated regular/cheat apps.
- Frame threshold failures, full-campaign added-enemy difficulty and long-session heat tests remained.

Build using the tag's source and Git LFS resources. From this version, resources can be downloaded from the repository without copying them separately.

[Apple platform tests](https://github.com/gelb2/HeavyMach/blob/v0.10/docs/public/apple-platform-progress-20261026.md) · [VTOL designs and tests](https://github.com/gelb2/HeavyMach/blob/v0.10/docs/public/vtol-art-and-loadout-20261027.md) · [Build and install](https://github.com/gelb2/HeavyMach/blob/v0.10/docs/public/build-and-install.md) · [Tasks remaining at release](https://github.com/gelb2/HeavyMach/blob/v0.10/docs/usability-todo.md)

<a id="v09"></a>

## v0.9 — 2026-10-26

[Release and downloads](https://github.com/gelb2/HeavyMach/releases/tag/v0.9) · [Source tag](https://github.com/gelb2/HeavyMach/tree/v0.9) · [Release commit 4e17f5ef](https://github.com/gelb2/HeavyMach/commit/4e17f5efd786095dfcd74a7334229908954da930)

Established the first confirmed playable iPhone restoration, with combat, progression, missions, bosses, menus and saves.

- Fixed rendering, audio, save timing, game timing and tilt input.
- Fixed special-attack impact positions, mission-clear illustrations, wider screens and menu backgrounds.
- Supplied a cheat validation app with saves separate from the regular app.
- The developer reported no crashes, malfunctions or abnormal enemy movement after playing all missions.
- After migration to the independent repository, 17 core tests, document-link checks and arm64 Simulator regular/cheat builds passed. The product source had the same Git tree as the installed build.
- A single full-screen door transition, further physical rotation tests and long-session heat tests remained.

The v0.9 tag predates adding game resources to the repository. Resources were stored separately at that time. Source and resources together are provided from v0.10.

[First release record](https://github.com/gelb2/HeavyMach/blob/v0.9/docs/release-v0.9.md) · [Installation and test record](https://github.com/gelb2/HeavyMach/blob/v0.9/docs/release-review-20261025.md) · [Tasks remaining at release](https://github.com/gelb2/HeavyMach/blob/v0.9/docs/usability-todo.md)

<a id="문서-작업-기록"></a>

## Documentation history

<a id="2026-10-28--왜-럭스루나에인가"></a>

### 2026-10-28 — Why Lux Lunae?

Recorded the two games' personal meaning, Lux Lunae aircraft, livery and combat intent, what I wanted to test with Codex, and production results. Added three final designs, four gameplay images and a combat video. Linked the article from the README, developer's statement, document index, features, development history and troubleshooting. Recorded formation crowding as follow-up work. The game version was v0.12.

<a id="2026-10-28--도입문기여-안내자동-검사소스-릴리스"></a>

### 2026-10-28 — Introduction, contribution guidance, automated checks and source release

Added the same introduction to the README and developer's statement. Configured About, Topics, bug/feature/PR templates and document, resource, core and Apple-build workflows. Checked 1,482 resources, 21 core tests, four app builds and two OpenAL relinking tests from the v0.12 archive. Added external notices, a sharing image and the resource-publication inventory. [Results and remaining items](docs/public/github-followup-20261028.md)

Versions are Git release tags, managed separately from app bundle versions. Release tags point to commits merging develop into main.

<a id="2026-10-28--github-문서-main-반영"></a>

### 2026-10-28 — GitHub documents merged into main

Wrote the README, features, controls, installation, architecture, troubleshooting, development history and validation on `docs/github-publication`. Added eight gameplay images, two GIFs and three MP4s. Game source and assets match v0.12. Merged through develop `0a2c0e33` into main `db634ce6`.

Later documentation changes added MIT for new code, license scope, asset terms, external notices, five technical badges, an environment table and a GitHub configuration review and plan.
