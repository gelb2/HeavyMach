<a id="구현-상태"></a>

# Implementation status

<!-- language-nav:start -->
[한국어](../../public/implementation-status.md) | [English](implementation-status.md)
<!-- language-nav:end -->

This describes main on November 2, 2026. The latest stable release tag, **v0.17**, includes Apple and Android apps. Later main changes add the public Web/PWA, Tauri Mac app, shared recovery codes and replacement game buttons. There is no v0.18 tag yet.

<a id="플랫폼별-상태"></a>

## Status by platform

| Platform | Implemented and checked | Distribution and remaining work |
|---|---|---|
| iPhone and iPad | Regular/cheat apps, combat, HUD editing, recovery codes, device restoration and restart | Xcode source builds. Extended thermal and some display-time tests remain |
| Native Mac | AppKit regular/cheat apps, keyboard combat, recovery codes and exit | Xcode source builds. Frame processing exceeded the threshold on November 2 |
| Android | Regular/cheat APKs, Galaxy USB/wireless installation, combat, restoration and restart | Gradle/Android Studio source builds. Distribution signing, tablets and extended tests remain |
| Web/PWA | Public play, installation, offline restart and recovery codes | Live on [GitHub Pages](https://gelb2.github.io/HeavyMach/). Actual long-inactivity retention tests remain |
| Tauri Mac | Local app build, combat, OS backups, self-code restoration, restart and exit | [Download Tauri Mac](https://github.com/gelb2/HeavyMach/releases/tag/tauri-mac-20261102) available. Ad-hoc signed; notarization pending |
| Tauri Windows | Development plan | Windows build, runtime testing and distribution incomplete |

[Build and installation](build-and-install.md) · [Platform tests](validation.md)

<a id="구현된-기능"></a>

## Implemented features

| Feature | Current behavior | Checks and evidence |
|---|---|---|
| Combat, progression and missions | Movement, jumping, weapons, special attacks, enemies, bosses, experience, upgrades and saves | My full-mission playthrough; core and app tests |
| Graphics, audio and timing | OpenGL/WebGL2, OpenAL reuse, platform display handling, event-based saves | Calculation, resource, save and audio tests; some display-time failures remain |
| Wider screens | Visible world follows aspect ratio; expanded-area touch; result background | iPhone, iPad and Mac display/input tests |
| Menus and doors | Connected central and side artwork; full-screen doors | Aspect-ratio, transition and sound tests |
| Landscape and lifecycle | Both landscape orientations; pause on lock, background or 120 seconds idle; manual resume | Device tests and my checks |
| Apple modules | Shared C++ core, Apple runtime and resource bundle; AppKit Mac | Builds and launch on three platforms |
| Keyboard | Full combat/menu control, Shift fine adjustment, remapping, iPad touch coexistence | API/OS keys and iPad Magic Keyboard movement, jump and trackpad firing checked; reconnection tests remain |
| Auto-aim | Toggle, target cycling, barrel adjustment, HUD indicators; original projectile rules | v0.11; 25,200 manual-firing frames matched after C removal |
| Three VTOLs | 113 additions in 14 sub-missions; existing placements preserved | v0.10; movement, weapons, collision and device checks |
| Three fighters | 72 reinforcements over 14 events; gold glass, grouped missiles, off-screen entry | v0.12; core and three-platform tests and my play |
| Two ground enemies | Shilka and Tunguska; 152 vehicles across 35 stages; independent radar; 6/4 gun rounds and 2 missiles | v0.13; [final artwork and tests](../../public/ground-aa-art-20261028.md#두-포탑-높이-맞춤) |
| HUD editing | Move buttons, gauges and decorations; preview, cancel, apply and reset | v0.14; 13 tests each on physical iPhone/iPad; v0.15 entry-change tests |
| App icons | Text-free Apple and Android app icons | v0.15; four Apple builds passed. [Artwork and material](../../public/app-icon-and-media-20261029.md) |
| Cheat app | Set mission, weapon and upgrade conditions; saves separate from regular app | iPhone, iPad, Mac and Android |
| Wireless development | iPhone/iPad installation, launch and LLDB | Reboot repetitions and full iPhone profiling remain |

[Feature details](features.md) · [Measurements and conditions](validation.md)

<a id="이번-문서-작업"></a>
<a id="공용-복원-코드와-게임-버튼"></a>

## Shared recovery codes and game buttons

Create or enter a code on the save-slot screen to move progress to another device. It includes three slots, experience, missions, weapon order, ammunition, upgrades, HUD layout, key bindings and aim settings. Apple restores all slots; Android and Web restore all slots or a selected slot. The shared module validates codes, backs up existing saves, applies restoration and reads the result back for comparison. [Module and integrations](../../../Modules/SaveTransfer/README.md).

Added HUD editing, recovery and key-setting buttons now use the original artwork, pressed states and input procedure. Code entry, clipboard access and file selection use OS and browser services. Original menus and SET GAME UI remain. Buttons, restoration and restart were checked on iPhone, iPad, Galaxy, Web/PWA and Tauri. [Test results](validation.md#게임-버튼과-복원--2026-11-02).

<a id="남은-기능과-연구"></a>

## Remaining features and research

| Task | Stage | Work |
|---|---|---|
| Added-enemy validation | Full-campaign and long-session testing remain | Track difficulty, heat and memory with new ground and air enemies |
| Build warnings | Full audit and fixes planned | Deprecated APIs are a suspected cause. Collect platform, regular/cheat and configuration logs; investigate alongside feature development |
| Build/run script | Examples researched; implementation planned | Combine resource download/checks, dependency/app builds, Mac launch and iPhone/iPad installation/signing guidance |
| Auto-aim indicators | [Display plan](../../public/auto-aim-visibility-plan-20261028.md); not implemented | Show no target, off-screen and underground targets while retaining the aiming algorithm |
| Tutorial | Research pending | Keep existing missions; add top package 0-0 through 0-4 covering tilt, weapons, switching and auto-aim using original UI, art and clear-condition conventions |
| Android | Regular and cheat apps implemented; Galaxy USB/wireless installation and gameplay checked | Full campaign completion, extended thermal tests, physical tablets and distribution signing |
| Folded screen | Concept recorded | Explore a game screen above controls and the associated input layout |
| Enemy AI and fixed coordinates | Follow-up preventive review | Check aircraft movement and activation ranges affected by screen width |

Remaining tests cover display timing including Mac, long-session heat/memory, keyboard reconnection, added-enemy difficulty and repeated wireless work. I saw no abnormal behavior in the installed iPhone and iPad apps. CI builds passed for all four Apple apps. [All remaining tasks](../../public/ground-units-research-20261028.md). [Work records and detailed requirements](../../usability-todo.md)

<a id="배포와-버전"></a>

## Distribution and versions

Web/PWA builds from current main are published on [GitHub Pages](https://gelb2.github.io/HeavyMach/). Tauri Mac can be [built from source](../../../Desktop/Tauri/README.md); [Download Tauri Mac](https://github.com/gelb2/HeavyMach/releases/tag/tauri-mac-20261102) is also available as a ZIP. Notarization remains pending.

The repository became public on October 28, 2026. The unified report site requires owner login. Git tags run from v0.9 through v0.17; v0.17 Releases offers a source-and-resource archive. Build and install iPhone, iPad and Mac apps through Xcode. A fresh clone builds using Git/LFS resources. v0.9 predates resource registration and needs additional resource preparation to build that historical code. [Installation](build-and-install.md) · [Version restoration](../../repository-management.md)

<a id="android-구현--2026-10-30"></a>

## Android implementation · 2026-10-30

The [Android development account](../../public/android-development-20261030.md) covers the regular and cheat APKs, Galaxy USB and wireless installation, the shared core, the Android host, tests and fixes. The implementation was developed on `feat/android-app` and merged through develop into main. The initial Android implementation preserved Apple sources. Later shared recovery-code and game-button integrations were updated on main. v0.17 is the Android version before those later changes. [Android installation](build-and-install.md#android-빌드와-설치), [Galaxy gameplay gallery](../../public/android-gameplay-20261030.md).
