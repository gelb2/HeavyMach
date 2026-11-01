<a id="남은-과제"></a>

# Roadmap

<!-- language-nav:start -->
[한국어](../../public/roadmap.md) | [English](roadmap.md)
<!-- language-nav:end -->

This describes main on November 2, 2026. v0.17 includes Apple and Android apps; later main changes add Web/PWA, Tauri Mac, recovery codes and replacement game buttons. I saw no abnormal behavior while playing the installed iPhone and iPad apps. The following fixes and tests remain.

<a id="버그와-성능"></a>

## Bugs and performance

| Task | Status and next work |
|---|---|
| No sound after an external alarm | Sound reportedly disappears after an alarm app sends HeavyMach to the background and it returns. Record interruption/resume, audio-session and OpenAL output state together, then test menu and weapon sound recovery. The cause is not established. |
| iPad fighter entry timing | v0.16 entry callback-interval p95 was 21.39ms, above 20ms. Compare against the same pre-fix conditions. iPhone entry/combat and iPad combat passed. |
| Native Mac performance | On November 2, p95 was 24.87–51.02ms, above the 16.67ms threshold. Investigate processing time and callback intervals separately for menus, combat and HUD editing. [Measurements](../../evidence/game-buttons-apple-20261102.json) |
| Earlier display-time failures | Reproduce and isolate outstanding iPhone menu and shared-module timing failures by condition. |
| Long sessions | Measure heat, power and memory during movement, jumping, firing and mission changes. Investigate Mac HUD-editing processing time too. |
| Build warnings | Full audit pending. Deprecated APIs are a suspected cause. Collect warnings by platform, regular/cheat and Debug/Release configuration, then diagnose and fix. |

<a id="조작과-게임-기능"></a>

## Controls and game features

| Task | Plan |
|---|---|
| Auto-aim status | Distinguish no target, off-screen targets and terrain-hidden targets visually. Retain current target selection, barrel and projectile calculations. |
| Tutorial | Research adding five missions, 0-0 through 0-4, above existing packages while preserving missions and saves. Teach tilt, movement, jumping, weapons, reloads, special attacks and auto-aim through actual input. |
| Added-enemy difficulty | Check ammunition, damage, special-attack use and completion difficulty across the full campaign with new ground and air enemies. |
| Formation combat behavior | Spawn and entry spacing shipped in v0.16. Post-entry formation keeping, evasion and leaving/re-entering the screen are not implemented. |
| Physical keyboards | I checked Web movement, jumping and trackpad firing with an iPad Magic Keyboard. Test Bluetooth/USB connection, reconnection, held keys and combined touch input. |
| Enemy AI and wider screens | Review fixed movement/activation ranges affected by screen width. |

<a id="빌드와-플랫폼"></a>

## Builds and platforms

- Tauri Mac: [Download Tauri Mac](https://github.com/gelb2/HeavyMach/releases/tag/tauri-mac-20261102) ZIP and first-launch instructions are available. Developer ID signing, notarization, first launch on another Mac, update and removal checks remain.
- Tauri Windows: build and verify WebView2 execution, input, audio, saves, exit and installation.
- Tauri/native Mac comparison: short combat and save tests are complete. Extended performance, memory and audio tests, and investigation of long menu-transition display intervals, remain.
- Web/PWA retention: check saves after actual inactivity of 7, 14, 30 and 40 days. Date-shift regressions and offline restarts passed. Changing the date does not advance the browser's storage-eviction clock.
- Further Web/PWA checks: repeat updates, motion permissions and save restoration in browser and installed modes on iPhone, iPad and Galaxy. Code restoration and offline restart passed after public deployment.

- Investigate recommending Xcode and Android Studio GUI builds at the top of installation guides. Separate project-specific choices and settings after Git clone/LFS from general IDE instructions covered by external references. [Research task](../../usability-todo.md#hm-gui-install-guide).

- A script connecting download, resource checks, OpenAL preparation, build and launch. Choose Mac launch or iPhone/iPad installation, regular or cheat app, with development-team/signing guidance.
- Full wireless XCTest/Instruments runs on iPhone and repeated installation/debugging after reboot on both devices.
- Android regular and cheat APKs run on Galaxy. Full campaign completion, extended thermal tests, physical tablet tests and distribution signing remain. [Development account](../../public/android-development-20261030.md).
- Explore folded-screen layouts with the game above and controls below.
- Automate recovery when the release list omits a newly created draft. v0.17 was published after verifying and resuming the same draft. [Test record](../../evidence/release-v0.17-20261030.json).

[Implementation status](implementation-status.md) · [Validation](validation.md) · [Changelog](../../../CHANGELOG.en.md) · [Detailed requirements and work records](../../usability-todo.md)

- Next release: align displayed versions across apps (iPhone 1.13, Android 0.16-android-dev), rebuild, and check installed apps, release artifacts, tag and changelog together. The latest tag remains v0.17.
