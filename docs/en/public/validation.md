<a id="heavymach-검증-결과"></a>

# HeavyMach validation results

<!-- language-nav:start -->
[한국어](../../public/validation.md) | [English](validation.md)
<!-- language-nav:end -->

The latest release tag is v0.17 and includes the regular and cheat Android apps. [Galaxy tests and fixes](../../public/android-development-20261030.md). The tables record tests and measurements from their respective versions. Codex ran automated tests and measurements; I played all missions myself.

<a id="게임-버튼과-복원--2026-11-02"></a>

## Game buttons and recovery · 2026-11-02

Added buttons on current main use the original game renderer and input procedure. Code creation, validation, restoration and restart were checked again after public Web/PWA deployment.

| Target | Checks and results |
|---|---|
| iPhone, iPad and native Mac | Three functional checks passed on each after merging. Code entry, restoration, cancellation and original menus checked |
| Galaxy | Regular/cheat buttons and restoration checked. Nine restoration/restart checks also passed in the previously installed PWA |
| Public Web | 20 touch, full-screen and restoration checks passed. Closing a modal with two fingers did not pass input to the menu behind it |
| Installed PWA | 18 checks passed. Actual save restoration and offline restart preserved progress and settings |
| Actual Tauri Mac app | Created, copied and checked its own code; cancelled confirmation; applied restoration; quit and launched a new process. Progress (3,624 bytes), HUD (173 bytes), key and aim settings matched |
| Preserved files | Comparison found no changes in 1,453 protected core, original resource and menu/HUD files |

The tested public site version was `7c03b1eb844c409c`; its game bundle was `3665a237a9dccd69`. [Apple evidence](../../evidence/game-buttons-apple-20261102.json) · [Android evidence](../../evidence/game-buttons-android-20261102.json) · [Web/PWA and Tauri evidence](../../evidence/game-buttons-web-tauri-20261102.json).

Native Mac functional tests passed, but frame processing p95 measured **24.87–51.02ms**, exceeding the **16.67ms threshold**, on the same day. Earlier Mac measurements below describe their own builds and conditions. Current Mac performance remains an open task.

<a id="플레이와-기능-검사"></a>

## Play and functional tests

| Date or target | Test | Result |
|---|---|---|
| v0.9 personal playthrough | All missions in the installed iPhone app | No observed crashes, malfunctions or abnormal enemy movement |
| October 26 fresh remote clone | Four builds, 17 core tests, 14 iOS tests, regular and cheat launch in a new Simulator | Passed, including decoding comparisons for 695 images at that time |
| October 26–27 native Mac | Development signing, unlocked session, movement, jumping, aiming, firing, 20 weapons, menus, GL, audio and full screen | 33/33 passed, including 280 weapon-selection combinations |
| October 27 iPhone Air | iOS 27.0.1, complete shared-module cheat-app suite | 84/85 passed; one display-time comparison failed |
| October 27 iPad Air 13-inch M2 | iPadOS 27.0.1, same suite | 84/85 passed; one display-time comparison failed |
| C removal `15968772` | 20 core tests; four input tests each on Simulator, iPhone and iPad | Passed |
| Manual firing after C removal | Compared with pre-feature `8fdeb629`: 20 weapons × 3 seeds × 420 frames | 25,200 frames matched byte for byte |
| VTOLs | Preserved 1,182 placements; 113 additions; movement, attacks, attitude, pause, destruction; 60 weapon collision cases | Passed |
| Fighters `45befedc` | 21 core tests; four combat/input tests each on Simulator, iPhone, iPad and Mac, plus HUD tests | Passed, including later off-screen entry tests |
| v0.16 entry placement | 140 conditions, 23 core tests, three functional tests each on iPhone and iPad | Passed. iPad entry p95 of 21.39ms exceeded the 20ms threshold |
| Personal play of the installed apps | Regular iPhone and iPad apps | No observed frame drops or malfunctions |

The 84 passing device tests include selecting and firing 20 weapons by keyboard input, combined touch input, resources, lifecycle, audio and zero save calls during combat. Codex used Device Hub OS key input to check movement, jumping, aiming, firing and weapon switching. I later checked movement, Space to jump and trackpad firing with an iPad Magic Keyboard in the Web version. Connection and reconnection tests across Bluetooth/USB keyboard models remain.

[Fresh clone](../../public/clone-build-20261026.md) · [Apple tests](../../public/apple-platform-progress-20261026.md) · [Auto-aim](../../public/auto-aim-ipad-20261027.md) · [VTOLs](../../public/vtol-art-and-loadout-20261027.md) · [Fighters](../../public/lux-lunae-design-20261027.md)

<a id="프레임메모리-계측"></a>

## Frame and memory measurements

| Target and conditions | Observation | Result and remaining work |
|---|---|---|
| Native Mac, development signed, unlocked, no profiler attached; 30 warm-up and 180 measured frames | Six scenes at 1200×800 and 1920×1080: p95 10.86–14.29ms, mean callback interval 16.67–16.69ms | Passed the 16.67ms processing threshold for those scenes |
| Mac regular and cheat apps, development certificate | Zero leaks | Resolved 417 connection-leak allocations totaling 20,016 bytes in the earlier ad-hoc app |
| iPad Simulator, Apple Software Renderer | Working build p95 44.311–48.133ms; previous main 39.972–40.580ms | Both failed. Software pixel processing dominated the final enlarged presentation |
| Follow-up iPhone menu measurement | p95 17.309ms, repeated at 17.629ms | Above 16.67ms; cause and reproduction conditions need further checks |
| Initial iPhone combat, 127.77 seconds | Mean 59.71 callbacks/second, p95 interval 16.67ms, maximum 50.02ms, thermal Serious | Mostly firing from a fixed position; no extended moving-combat or power comparison |
| Short iPad measurement | Zero leaks, physical footprint 493.2MB | Includes XCTest and screen recreation; long-session regular-play memory test needed |

Simulator measurements used Apple Software Renderer. Device-specific iPhone/iPad measurements and personal play results appear above.

A 3840×2160 OS capture confirmed that the Mac game reached all four screen edges. White margins visible in CUA window captures were absent from the actual display capture. [Measurement JSON](../../evidence/apple-platform-performance-20261026.json), [full-screen JSON](../../evidence/mac-fullscreen-capture-20261027.json)

<a id="설치무선-디버깅"></a>

## Installation and wireless debugging

Codex installed regular and cheat apps on iPhone and iPad. Device connection, installation, launch and LLDB attachment were also checked with cables disconnected. On iPad, eight wireless tests and 21.716 seconds of startup CPU activity were recorded. Complete wireless XCTest and Instruments runs on iPhone and repeated tests after reboot remain.

[Wireless procedure](../../wireless-device-workflow.md) · [Result JSON](../../evidence/wireless-apple-20261027.json)

<a id="문서미디어-확인"></a>

## Documents and media

The November 2 resource check matched SHA-256 hashes for 1,505 build and test inputs. For the October 27 iOS media set, Codex extracted eight images and three MP4s from my play recordings. Two GIFs use short sections of those same scenes. The [scene manifest](../../public/media/gameplay-20261027/media-scenes.json) records source hashes, times and output hashes. The recording device, app type and build number for that video were not supplied. Later material is grouped into [iOS](../../public/ios-gameplay-gallery-20261030.md) and [Android](../../public/android-gameplay-20261030.md) galleries.

<a id="남은-검사"></a>

## Remaining tests

- Web/PWA: actual save retention after 7, 14, 30 and 40 days of inactivity. Date-shift regression tests do not advance the browser’s storage-eviction clock.
- Tauri: extended play, memory/audio comparisons and public-installer tests.
- Display timing: investigate the November 2 native Mac threshold failures; reproduce and isolate the iPhone menu threshold failure and the corresponding iPhone/iPad suite failures.
- Long sessions: heat, power and memory during movement, jumping, combat and mission transitions.
- Input: physical Bluetooth/USB keyboard connection, disconnection, reconnection, held keys and combined touch input.
- Difficulty: ammunition, damage and completion difficulty across all missions with added aircraft.
- Wireless: iPhone XCTest/Instruments and repeated installation/debugging after reboot on both devices.

[All-weapon test procedure](../../weapon-validation-procedure.md) · [Current tasks](implementation-status.md)

<a id="android-구현--2026-10-30"></a>

## Android implementation · 2026-10-30

The [Android development account](../../public/android-development-20261030.md) covers the regular and cheat APKs, Galaxy USB and wireless installation, the shared core, the Android host, tests and fixes. The implementation was developed on `feat/android-app`; existing Apple sources were preserved.

<a id="android-검사--2026-10-30"></a>

## Android tests · 2026-10-30

| Target and conditions | Checked | Result |
|---|---|---|
| Galaxy SM-S731N | Movement, jumping, 20 weapons and resume | 31 checks passed |
| Galaxy after USB disconnection | Wireless installation, launch, termination and home return for regular/cheat apps | 11 checks passed |
| Galaxy and emulator | Mission entry with a deferred upgrade, selection, confirmation, interruption and resume | 12 checks passed on each |
| Personal play in the regular Galaxy app | Missions 3-4 and 3-5 played and recorded on the fixed build | No malfunctions observed |
| Galaxy boss combat, 45 seconds | 2,763 display intervals | P99 in the 17ms bucket; 0 intervals above 33ms |

Full campaign completion, extended thermal and memory tests, Android tablets and physical 16KB-page devices remain. [Implementation and tests](../../android-implementation-20261030.md), [upgrade regression results](../../evidence/android-upgrade-fix-20261030.json), [regular-app gameplay](../../public/android-gameplay-20261030.md).
