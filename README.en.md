# HeavyMach

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

[![Git tag v0.17](docs/public/media/badges/version.svg)](https://github.com/gelb2/HeavyMach/tree/v0.17)
[![Platforms iOS iPadOS macOS Android](docs/public/media/badges/platforms.svg)](docs/en/public/validation.md)
[![Architecture arm64](docs/public/media/badges/arch.svg)](docs/en/public/build-and-install.md)
[![C++20, Objective-C++ and Kotlin](docs/public/media/badges/code.svg)](docs/en/public/architecture.md)
[![Code license MIT](docs/public/media/badges/license.svg)](LICENSE-SCOPE.en.md)

<a id="heavymach를-다시-만든-이유"></a>

## Why I rebuilt HeavyMach

In July 2017, I chose iOS development as my third profession. Before that, I worked as a magazine journalist in 2015 and in the Philippines in 2016. The world I experienced in work far removed from software felt nondeterministic and irrational. Several things led me to leave that life and choose another profession. Over soju with friends, I sometimes hear that if I had continued doing what I did in 2015 and 2016, I would have ended my life by jumping to my death. I still do not know whether they are right.

HeavyMach was one reason I left the life I had in 2015 and 2016 and chose another profession. I encountered it on an iPod touch in college. I first found it through Cydia, then searched for how to create a US App Store account and bought it. The Korean App Store had no games category at the time. I tilted the device to move the tank and fired its weapons; an illustration appeared after each stage. I think this game contributed about 35% to my decision to become a developer.

Lately, I have felt that I do not have much time left. It is neither a health problem nor extreme lethargy. If anything, my mind feels clearer than it did a month, two months or half a year ago. So I think about what I want to try and what I want to leave behind. Restoring HeavyMach is one of those choices. A game that once showed me a direction, made to work again by me, now a developer. To me, this brings the end back to the beginning.

Someone named [mesabloo](https://github.com/Mesabloo/hm-defense) worked on a similar project. He obtained HeavyMach Defense assets and wrote its source code from scratch. He appears to have stopped while implementing combat between units. My starting point was better than his, since he had to write the source code from scratch.

One weekend, I had a dream. I saw myself from a third-person perspective, encountering the iPod touch and Cydia for the first time. After waking up, I searched Google for heavy Mach. Gemini in Google Search pointed me to [Internet Archive](https://archive.org), where I downloaded HeavyMach 1.13 from the [HeavyMach series archive](https://archive.org/details/heavy-mach-archive_202406) and began the restoration.

So I think I did very little. This is also why I feel I have even less reason to think of making money from it. In my own words, all I did was grab Codex by the collar and slap it whenever it arbitrarily departed from the original source code and its creators' artistic achievement and did something wrong.

I have no intention of making money from this restoration. I want to publish the code and assets together so other people can build and play it themselves. I also want to record how it was restored and what was fixed.
If a stakeholder contacts me, I will take this project down. The nine years and three months HeavyMach's creators gave me exceed the lifespan my creator originally allotted to me.

Money is only needed in this life.

[Full developer's statement](docs/en/RESTORATION.md)

<a id="간단한-설명과-설치"></a>

## Overview and installation

**HeavyMach**, the 2009 side-scrolling tank game, has been rebuilt to run on current iPhone, iPad, Mac and Android devices. Move and jump the tank, switch weapons and fight ground and air enemies. Wider screens, keyboard controls, auto-aim and new aircraft extend the original combat and progression.

The latest release tag is [v0.17](https://github.com/gelb2/HeavyMach/tree/v0.17). It includes the regular and cheat Android apps. Regular and cheat validation apps can be installed separately. The cheat app tests the same game with specified missions, weapons and upgrades.

[Latest release](https://github.com/gelb2/HeavyMach/releases/latest) · [Version history](CHANGELOG.en.md) · [Android changes](CHANGELOG.en.md#v017)

[Apple installation](docs/en/public/build-and-install.md) · [Android installation](docs/en/public/build-and-install.md#android-빌드와-설치) · [Android gameplay](docs/public/android-gameplay-20261030.md) · [Controls](docs/en/public/gameplay-guide.md) · [Features](docs/en/public/features.md) · [Troubleshooting](docs/en/public/troubleshooting.md) · [Documentation](docs/en/public/index.md)

<a id="원작의-전투와-조작"></a>

## Original combat and controls

In the original game, tilting the device moves the tank forward or backward. Touching a point on the screen aims and fires. The jump button clears obstacles; the weapon button changes attacks. These controls are reproduced in the restoration.

![The tank jumps above a slope while firing at enemies to the right.](docs/public/media/gameplay-20261027/02-jump.png)

<a id="원작의-무기와-성장"></a>

## Original weapons and progression

Cannons, spread weapons, missiles, beams and air support come from the original game. Projectile speed, spread, gravity and firing intervals vary by weapon. Switch weapons and reposition to choose an attack.

![The upper weapon list and selection outline. White trails cross the battlefield.](docs/public/media/gameplay-20261027/03-weapon.png)

![Several blue beams descend from the sky to the ground.](docs/public/media/gameplay-20261027/04-special.png)

Experience, movement/defense/attack upgrades, missions and bosses also follow the original rules. Completing a mission shows the original result illustration and next-stage selection.

![Upgrade window with MOVEMENT, DEFENSE and ATTACK choices](docs/public/media/gameplay-20261027/05-upgrade.png)

![STAGE CLEAR and the next-stage button over a tank illustration.](docs/public/media/gameplay-20261027/06-stage-clear.png)

<a id="내가-추가한-항공기"></a>

## Aircraft I added

I planned six real-world aircraft additions while preserving existing enemy placements. I specified aircraft, appearance, weapons and appearance conditions; Codex made the artwork and code.

Yak-141, Harrier GR3 and F-35B aircraft were added between existing placements. F-22, Su-57 and MiG-31 fighters arrive as reinforcements during boss fights and the final sections of preceding missions. They reuse helicopter weapons; the fighters add grouped missile bursts and gold cockpit glass.

![The tank fires beams at several black aircraft on the right.](docs/public/media/gameplay-20261027/07-fighters.png)

[Added-aircraft combat video — 13 seconds](docs/public/media/gameplay-20261027/07-fighters.mp4) · [Intent, values and missions](docs/en/public/features.md)

HeavyMach influenced my decision to become a developer. Ace Combat 8 gave me a reason to want to stay alive, even for just a few more months. I wanted to bring Lux Lunae's appearance and the threat I felt fighting them into this game. [Why Lux Lunae?](docs/en/public/lux-lunae-intent-20261028.md)

<a id="내가-추가한-지상-유닛"></a>

## Ground units I added

Shilka and Tunguska add 152 vehicles across 35 stages while preserving existing placements. They reuse original ground-unit movement and part behavior. Their radar rotates independently of the guns.

![Shilka firing white rounds on a slope](docs/public/media/ground-aa/shilka-height/shilka.png)

Shilka fires six white rounds in succession. At my request, its turret height matches Tunguska's.

![Tunguska firing a missile on a slope](docs/public/media/ground-aa/integrated/ipad-tunguska.png)

Tunguska fires four white rounds and two missiles in succession. These firing scenes were captured in the cheat app on a physical device. [Parts, weapons and revisions](docs/public/ground-aa-art-20261028.md)

<a id="내가-추가한-자동조준"></a>

## Auto-aim I added

Inspired by target selection and tracking in Armored Core and Ace Combat, I planned a barrel that tracks enemies during movement and jumping. Codex implemented target selection, per-weapon trajectory calculations, barrel adjustment and HUD buttons.

Turn auto-aim on with the HUD's **A button** to select a nearby enemy ahead. Use the arrow button to change targets and the reticle button to fire. The player controls movement, jumping, weapon choice and firing; the game adjusts the barrel toward the selected enemy. Press A again to return to the original touch aiming and firing.

![The tank fires at an airborne enemy marked with green corners and LOCKED.](docs/public/media/gameplay-20261027/01-auto-aim.png)

Targets remain selected off-screen. If an enemy dies or moves behind the tank, another enemy ahead is selected. Projectiles use the original weapon's speed, spread, gravity and damage, so movement after firing or spread can cause misses.

![Five seconds of firing at an auto-aimed airborne enemy](docs/public/media/gameplay-20261027/01-auto-aim.gif)

[Auto-aim video with sound — 7 seconds](docs/public/media/gameplay-20261027/01-auto-aim.mp4)

![The tank jumps while fighting an airborne enemy marked LOCKED](docs/public/media/gameplay-20261027/08-jump-clip.png)

![Six seconds of firing at an auto-aimed enemy, jumping and landing](docs/public/media/gameplay-20261027/08-jump-clip.gif)

[Jumping, firing and weapon list during auto-aim — 7 seconds](docs/public/media/gameplay-20261027/08-jump-clip.mp4) · [Target, trajectory and barrel calculations](docs/en/public/features.md)

These scenes come from my play recordings. GIFs run at original speed; MP4s include sound. [Video times and file list](docs/public/media/gameplay-20261027/README.md)

<a id="hud-위치-편집"></a>

## HUD layout editing

Open **SET GAME UI** in the pause menu to drag buttons, gauges and upper decorations. Button images and hit areas move together. **APPLY** saves the layout; **RESET LAYOUT** restores the default.

![HUD editor in the iPhone cheat app, with the upper decoration moved down](docs/public/media/hud-editor/release-iphone-trim.png)

The upper decoration has been moved in the iPhone cheat app. Dragging collapses the right-hand tools; **TOOLS** reopens them. Combat pauses during editing. The regular app has the same editor. [Moving MENU and testing its input](docs/public/app-icon-and-media-20261029.md#hud-위치-편집)

<a id="ios-플레이-캡처-모음"></a>

## iOS gameplay gallery

The two recordings I provided contain eight selected images, two GIFs and three videos with sound. The gallery covers jumping, weapons, air support, upgrades, stage completion, added aircraft and auto-aim.

[All iOS gameplay images and videos](docs/public/ios-gameplay-gallery-20261030.md)

<details>
<summary>Show four iPhone and iPad cheat-app validation captures</summary>

**Shilka firing on a slope — iPhone Air**

![Shilka firing on a slope — iPhone Air](docs/public/media/ground-aa/shilka-height/shilka.png)

**Tunguska firing missiles on a slope — iPad Air 13-inch M2**

![Tunguska firing missiles on a slope — iPad Air 13-inch M2](docs/public/media/ground-aa/integrated/ipad-tunguska.png)

**Upper HUD trim moved downward — iPhone Air**

![Upper HUD trim moved downward — iPhone Air](docs/public/media/hud-editor/release-iphone-trim.png)

**Pause menu opened after moving MENU — iPhone Air**

![Pause menu opened after moving MENU — iPhone Air](docs/public/media/hud-editor/release-iphone-menu.png)

</details>

<a id="android-플레이-캡처-모음"></a>

## Android gameplay gallery

Missions 3-4 and 3-5, recorded during my play in the regular Galaxy app. Tilt the device to move and touch the screen to fire. Auto-aim, added aircraft and ground units, and HUD layout editing are also available.

![Fighting the aircraft boss in mission 3-5 in the regular Galaxy app.](docs/public/media/android-gameplay-20261030/06-boss.png)

![Fighting the Lux Lunae squadron in the regular Galaxy app](docs/public/media/android-gameplay-20261030/fighters.gif)

[Combat video with audio — 8 seconds](docs/public/media/android-gameplay-20261030/fighters.mp4) · [All Android captures: 8 images, 3 GIFs and 3 videos](docs/public/android-gameplay-20261030.md)

Combat initially continued behind the upgrade window after the first boss. That bug was fixed, and I saw no malfunctions during my subsequent play. [Development time, shared modules, fixes and tests](docs/public/android-development-20261030.md)

<a id="android-개발-뒤에-남은-위화감"></a>

## The unease after Android development

I recorded the unease I felt while building the Android version: refusals and false reports in other projects, and my hypothesis about the difference.

[Neo-Lysenkoism and the Alignment Tax — full essay](docs/en/public/android-ai-experience-20261030.md)

<a id="플랫폼과-개발-상태"></a>

[The first PR and Tauri (Korean)](docs/public/first-pr-and-tauri-20261031.md): accepting the web contribution, the missing approval review, and the Tauri evaluation.

## Platforms and development status

Wider screens, iPad and Mac support, and keyboard controls were also my requests. Wider displays show more world. The keyboard covers movement, jumping, aiming, firing, weapon switching and menus. iPad supports touch, tilt and keyboard together.

| Target | Support | Checked |
|---|---|---|
| iPhone | Landscape rotation, touch, tilt, keyboard, regular and cheat apps | iPhone Air installation, input and combat tests; my play |
| iPad | Aspect-ratio canvas, combined touch, tilt and keyboard | iPad Air 13-inch M2 installation, input and combat tests; my play |
| Mac | AppKit app, window/full-screen modes, keyboard | Mac mini execution, 33 tests, zero leaks in development-signed apps |
| Android | Regular and cheat apps using the shared C++ core | Galaxy USB/wireless installation and gameplay checked. [Installation](Android/README.md) · [Development and screenshots](docs/public/android-development-20261030.md) |
| Web | WebAssembly·WebGL2, mouse, keyboard, touch and tilt (`porting/wasm`) | Contributor-tested Chrome·Safari·ego lite, iPad mini Safari and Android tablet Chrome. [Web port](docs/web-port-20261028.md) |

I saw no frame drops or malfunctions while playing the installed iPhone and iPad apps. Device automation still has display-timing threshold failures. [Measurement conditions and results](docs/en/public/validation.md)

Auto-aim, shared modules, three Apple platforms, six aircraft, two ground units and full-screen doors are implemented. HUD editing is also implemented. Clearer auto-aim indicators, tutorials 0-0 through 0-4 remain. Android runs on Galaxy; extended sessions, tablet tests and distribution signing remain. Text-free app icons shipped in v0.15. [Completed, ongoing and remaining work](docs/en/public/implementation-status.md)

<a id="남은-과제"></a>

### Remaining work

The focus has been restoring the original and making it playable. Build warnings have not yet received a full audit and fix pass. Deprecated APIs are a suspected cause; investigation and fixes will continue alongside new features.

A script combining download, resource preparation, building and launch is also planned. It will offer Mac launch or iPhone/iPad installation and explain required tools, development-team selection and signing. [Task details](docs/en/public/roadmap.md)

**Build settings and test environments**

| Platform | Project minimum OS | Recorded test environment |
|---|---|---|
| iPhone and iPad | iOS/iPadOS 15.0 | iPhone Air and iPad Air 13-inch M2, OS 27.0.1 |
| Mac | macOS 12.0 | Mac mini M4 Pro, macOS 27.0.1 |
| Android | Android 8.0 / API 26, arm64-v8a | Galaxy SM-S731N, Android 16 |

Apple builds target Apple Silicon arm64; Android builds target arm64-v8a. See [validation](docs/en/public/validation.md) for device environments and [installation](docs/en/public/build-and-install.md) for build-tool versions.

<a id="빌드하기"></a>

## Build

<a id="iphoneipadmac"></a>

### iPhone, iPad and Mac

Use an Apple Silicon Mac, Xcode, CMake 3.20 or later, Python 3 and Git LFS. The repository includes source, game images, audio, maps and build projects.

```sh
git clone https://github.com/gelb2/HeavyMach.git
cd HeavyMach
git lfs install --local
git lfs pull
python3 tools/verify_build_resources.py
open HeavyMach.xcworkspace
```

Select your development team and destination in Xcode. iPhone/iPad schemes are `HeavyMachRestored` for the regular app and `HeavyMachValidation` for the cheat app. Mac schemes are `HeavyMachMac` and `HeavyMachMacValidation`. Clone the repository or download the source-and-resource archive from [v0.17 Releases](https://github.com/gelb2/HeavyMach/releases/tag/v0.17), then build and install through Xcode.

[Simulator commands, device installation, signing and troubleshooting](docs/en/public/build-and-install.md)

### Android

After cloning and downloading LFS resources above, prepare JDK 17, Android SDK 36, NDK 26.3.11579264, CMake 3.22.1 and Python 3. Set the SDK location in Android Studio. Enable USB debugging and find the connected device with `adb devices`.

```sh
cd Android
./gradlew :app:assembleNormalDebug :app:assembleCheatDebug
adb -s SERIAL install -r app/build/outputs/apk/normal/debug/app-normal-debug.apk
```

Replace `SERIAL` with the target device ID. Install the cheat app from `app/build/outputs/apk/cheat/debug/app-cheat-debug.apk` the same way. Current APKs are optimized builds with debug signing.

[SDK setup, APK paths and USB/wireless installation](docs/en/public/build-and-install.md#android-빌드와-설치) · [Android tools and test procedures](Android/README.md)

Build the web version with Emscripten. Run `emcmake cmake -S Web -B build/web && cmake --build build/web`, then serve `build/web/dist` with a static server. [Web build guide](docs/web-port-20261028.md)

The web version was contributed by [fastutlego](https://github.com/fastutlego). [PR #24](https://github.com/gelb2/HeavyMach/pull/24) adds the WebAssembly host, WebGL2 renderer, input, audio, persistence and browser tests.

<a id="개발-중-고친-문제"></a>

## Problems fixed during development

The initial implementation wrote files on every shot and hit, recreated images on the CPU and incurred delays restarting sounds. I asked why behavior differed from the working reference and required Codex to compare saves, graphics, audio and timing separately.

Codex limited file writes to save events. Game graphics changed to OpenGL textures, and sound effects to reused OpenAL buffers and sources. Special-weapon coordinates, boss approach and artwork joins affected by screen expansion were fixed through the same comparisons.

[19 troubleshooting cases](docs/en/public/troubleshooting.md) · [Analysis, implementation and validation](docs/en/public/development-history.md)

<a id="코드-구조와-버전"></a>

## Code structure and versions

One workspace manages shared modules, the iOS/iPadOS app and native Mac app. The C++ core, images, audio and maps are shared, with platform-specific window, input and graphics-context connections. On Apple platforms, static frameworks provide shared code and `HeavyMachAssets.bundle` supplies resources. Android uses the Gradle project in `Android/` to build the same C++ core with the NDK. Kotlin, JNI and an Android C++ host connect input, menus, rendering and audio; the APK includes the same game resources.

[Structure and reasons for modularization](docs/en/public/architecture.md) · [Validation](docs/en/public/validation.md) · [Changelog](CHANGELOG.en.md)

Work branches merge into `develop`; only `develop` merges into `main`. Release tags point to merge commits on `main`. [Rebuilding from a tag](docs/repository-management.md)

[Original creators, additional work and libraries](CREDITS.en.md) · [SaaS Apps/Mags writing material](docs/public/publication-inventory.md)

<a id="라이선스와-자원"></a>

## Licenses and resources

The original HeavyMach was developed by Byun Hae-jun, with artwork by Park Jae-cheol. In 2011, WeMade Creative, a subsidiary of WeMade Entertainment, acquired IndiApps assets including the HeavyMach and HeavyMach 2 IP. [Acquisition report](https://zdnet.co.kr/view/?no=20110512162801) · [Creator and company roles](CREDITS.en.md)

New code written for this project uses the [MIT license](LICENSE). [License scope](LICENSE-SCOPE.en.md) distinguishes original-derived material, external code and documents. Original images, audio, game data and additional artwork follow the [asset terms](ASSET-LICENSES.en.md); OpenAL Soft, stb and other libraries are covered by [third-party notices](THIRD_PARTY_NOTICES.en.md).

I have no intention of making money from this restoration. [Developer's statement](docs/en/RESTORATION.md)
