<a id="heavymach-기능과-추가-기획"></a>

# HeavyMach features and additions

<!-- language-nav:start -->
[한국어](../../public/features.md) | [English](features.md)
<!-- language-nav:end -->

Current main includes tank movement, jumping, firing, weapons, parts, upgrades, missions, bosses, result screens and saves. Wider screens, Apple/Android support, auto-aim, six aircraft, two ground enemies, HUD editing and cross-device recovery codes are additions I planned and asked Codex to implement. The web port came from [fastutlego’s PR #24](https://github.com/gelb2/HeavyMach/pull/24); I then requested PWA and Tauri distribution based on that work.

<a id="넓은-화면과-메뉴"></a>

## Wider screens and menus

I asked for a wider canvas showing more of the world instead of enlarged artwork. Codex expanded the visible range according to aspect ratio and converted touches in the added side areas to world coordinates. Enemy visibility and combat activation were handled separately.

Title, mode and mission menus retain the original central assets with matching backgrounds on both sides. I found disconnected edges and black lines; Codex corrected the artwork joins and texture UVs. Menu transitions now use four door pieces covering the screen. The battlefield showing behind the result illustration was also fixed.

[Screen expansion and door tests](../../public/fullscreen-transitions-20261027.md) · [Coordinate policy](../../coordinate-policy.md)

<a id="플랫폼입력치트-앱"></a>

## Platforms, input and cheat app

I asked for one workspace managing iPhone, iPad and Mac, with shared game code and resources. iPad supports touch, tilt and keyboard together. Codex implemented Mac as an AppKit app. The keyboard covers movement, jumping, aiming, firing, weapon switching and menus. Shift provides fine tilt and barrel-angle control.

The cheat app has its own app ID and saves. Starting conditions include mission, all weapons, parts and upgrades, allowing weapon and enemy tests without replaying progression. Keyboard and touch use the game's shared input paths.

[Controls](gameplay-guide.md) · [Architecture](architecture.md) · [Cheat validation](../../validation-mode.md)

The regular and cheat Android apps use the same C++ game rules and resources. They support touch, tilt, keyboard, auto-aim, added enemies and HUD editing. Android screen, input, audio and save connections were implemented separately while preserving existing Apple files. [Android development](../../public/android-development-20261030.md), [installation](build-and-install.md#android-빌드와-설치), [Galaxy gameplay](../../public/android-gameplay-20261030.md).

<a id="웹pwatauri-mac"></a>

## Web, PWA and Tauri Mac

The [Web version](https://gelb2.github.io/HeavyMach/) runs in a browser or as an installed Home Screen PWA. The launcher recommends installation and also offers browser play. Once resources finish downloading and pass validation, the game can start offline. It compiles the same C++ game rules to WebAssembly and supports touch, tilt, keyboards and trackpads.

Tauri Mac includes this Web game and adds OS file backups. If Web storage is empty, it restores the last valid file. Local app builds, play, restoration and exit have been checked. [Download Tauri Mac](https://github.com/gelb2/HeavyMach/releases/tag/tauri-mac-20261102) is available as a ZIP. Notarization and the Windows version remain. [Web/PWA installation](../../../Distribution/Web/README.md) · [Tauri builds and saves](../../../Desktop/Tauri/README.md).

<a id="복원-코드"></a>

## Recovery codes

I asked for progress transfer between platforms without a server or account. TRANSFER PROGRESS on the slot-selection screen creates an `HM1-` code to enter on another device. It carries missions, experience, weapon order, ammunition, part upgrades, HUD layout, key bindings and aim settings. The restored part upgrades reconstruct the tank's appearance.

Apple restores all slots. Android, Web/PWA and Tauri can restore all slots or a selected slot, with options to include HUD and control settings. After code validation, a confirmation screen applies restoration. Existing data is backed up first, and the applied result is read back for comparison. Code creation and entry are available on the slot-selection screen.

`Modules/SaveTransfer` provides the shared format, validation and restoration. Core movement, ballistics, enemies, damage calculations and the original slot structure remain unchanged. [Module](../../../Modules/SaveTransfer/README.md) · [Device and public Web tests](validation.md#게임-버튼과-복원--2026-11-02).

<a id="hud-편집과-게임-버튼"></a>

## HUD editing and game buttons

Pause combat and select SET GAME UI to drag buttons, gauges and decorations over a preview containing the tank, terrain and enemies. Hit areas move with the artwork. Reset, cancel and apply are available. [Screens and tests](../../public/hud-improvement-plan-20261028.md).

Added HUD editing, recovery and key-setting buttons use the original button artwork, pressed states and input procedure. Existing menus and SET GAME UI remain. OS and browser services handle code text entry, clipboard access and file selection.

<a id="지상-적-2종"></a>

## Two ground enemies

Shilka fires six white gun rounds in a burst; Tunguska fires four white rounds and two missiles. They reuse existing ground-unit part assembly and movement, with radar rotating independently of the turret. There are 152 additions across 35 stages. Shilka's turret height follows the in-game proportions I requested. [Final artwork, placements and tests](../../public/ground-aa-art-20261028.md#두-포탑-높이-맞춤).

<a id="자동조준"></a>

## Auto-aim

<a id="기획-의도와-요구"></a>

### Intent and requirements

I requested auto-aim with target selection and tracking inspired by Armored Core 6 and Ace Combat 7 and 8. The barrel needed to track the target while the tank moved or jumped, with a separate button to change targets. An ON/OFF switch would retain the existing touch aiming and firing.

Initially, I requested guaranteed hits by accounting for weapon arcs and enemy movement. Codex added C correction, which changed projectile speed, spread and flight paths. After checking its effects, I instructed Codex to remove it. The final feature selects targets and adjusts the barrel. Fired projectiles follow their original weapon rules.

<a id="표적과-포신-계산"></a>

### Target selection and barrel calculations

`GameSession` selects living enemies whose world X is ahead of the tank. The initial target has the smallest squared distance. It remains selected when off-screen. A target that dies, is removed or moves behind the tank is released, and another enemy ahead is selected. Cycling uses candidates ordered by distance. With no enemy, the barrel returns to its default angle of 0.1rad.

Enemies have integer IDs that increase within a stage. The enemy list and selected ID reset together at stage start. The code also handles immediate reselection of an enemy just released.

`AutoAim.hpp` searches for intercept candidates within 600 ticks using weapon speed, gravity, lifetime and the enemy's previous displacement. It applies gravity before updating position, like the actual projectile. Changing barrel angle also changes muzzle position, so muzzle and trajectory are recalculated three times. If terrain blocks a low arc, the calculation checks a higher arc. Air-support weapons use their launch point above the battlefield.

This calculation does not change speed or spread in `spawnBullet`. Movement after firing and projectile spread can still cause misses.

<a id="버튼과-표시"></a>

### Buttons and indicators

The HUD's A button toggles auto-aim, the arrow button cycles targets and the reticle button fires. Metal texture, gray and yellow colors, and pressed states match the existing jump, menu and weapon buttons. Codex also swapped the arrow and reticle positions at my request.

Corners, diagonal decorations and `LOCKED`, `TRACK`, `BLOCKED` mark the selected target and aiming state. An edge indicator points toward an off-screen target. Switching modes clears held-fire input. Turning auto-aim off restores touch aiming and firing.

![Auto-aim target and lower controls](../../public/media/gameplay-20261027/01-auto-aim.png)

After removing C, Codex passed 20 core tests and Apple input tests. Manual firing records matched the pre-auto-aim commit byte for byte: 20 weapons × 3 seeds × 420 frames, totaling 25,200 frames. Clearer indicators for off-screen, underground and absent targets remain planned.

[Code](../../../Sources/Core/AutoAim.hpp) · [Game integration](../../../Sources/Core/GameSession.cpp) · [Tests and fixes](../../public/auto-aim-ipad-20261027.md)

<a id="수직이착륙기-3종"></a>

## Three VTOL aircraft

<a id="기획과-외형"></a>

### Intent and appearance

I requested Yak, Harrier and F35 aircraft as airborne enemies moving like helicopters. They were to be added between existing placements, without replacing enemies, in numbers based on each mission's average helicopter count. Codex's first draft introduced an unrequested reinterpretation of their shapes; I asked for a redraw. Codex added rules for preserving real aircraft shapes to the art skill, then checked nose, intake, wing and tail proportions against photographs and drawings. Existing helicopters supplied color and texture references. After approving the new draft, I required this procedure for future real-world units. [First draft and corrections](../../public/vtol-art-and-loadout-20261027.md)

I later requested cleaner outlines and removal of the forward flames. Codex changed the artwork and effects. All three aircraft are longer than Heli001's body length of 99 and shorter than Heli006's 126, with proportional heights. Images are resized only at load time and reused on the GPU.

| Aircraft | HP / EXP | Reused weapon | Display length |
|---|---|---|---:|
| Yak-141 | 210 / 10, based on Heli004 | E_Wpn_ASM, single missile, damage 25 | 112 |
| Harrier GR3 | 400 / 25, based on Heli008 | E_Boss_Wpn_01, two cannon rounds, damage 50 | 108 |
| F-35B | 900 / 60, based on E0604 | E_0604_Wpn, two missiles, damage 100 | 116 |

I chose the three aircraft and required weapon reuse. Codex proposed the reference units, weapon IDs and values above.

<a id="이동과-배치"></a>

### Movement and placement

The aircraft reuse the helicopters' `machType=1` movement. Only the new aircraft add pitch of up to 8 degrees from horizontal speed and 3 degrees from vertical speed. Attitude changes gradually toward the target angle. The muzzle, rear nozzle and hit area rotate with the aircraft. One rear nozzle uses the existing exhaust image.

`VTOL.cpp` appends definitions and placements after data loading. The 1,182 original enemy entries retain their names, coordinates, order and boss flags. The number of ordinary helicopters per mission is divided by its total sub-mission count and rounded; selected stages receive that many aircraft.

| Aircraft | Missions and quantities | Total |
|---|---|---:|
| Yak-141 | 2-2: 5, 2-4: 5, 3-2: 7, 4-2: 4, 5-2: 9 | 30 |
| Harrier GR3 | 3-3: 7, 4-3: 4, 5-3: 9, 6-3: 10, 7-2: 11 | 41 |
| F-35B | 6-2: 10, 6-4: 10, 7-3: 11, 7-4: 11 | 42 |

There are 113 additions across 14 sub-missions. Missions 1 and 8 and Survival placements remain unchanged. Aircraft avoid the outer 300 units at each map end and 600 around bosses, and are spread at 140 above terrain. New units use separate random state, preserving the original enemies' random-call sequence.

Codex's automated tests covered original placement preservation, duplicate prevention, movement, firing, attitude, pausing, destruction and collisions for 20 weapons × 3 aircraft. Added difficulty across all missions and long-session heat have not yet been tested.

[Artwork, placement and tests](../../public/vtol-art-and-loadout-20261027.md) · [VTOL.cpp](../../../Sources/Core/VTOL.cpp)

<a id="럭스루나에-도색-전투기-3종"></a>

## Three fighters in Lux Lunae colors

[Why Lux Lunae?](lux-lunae-intent-20261028.md) explains the aircraft and livery choices, the intended tension in boss fights and my play experience.

<a id="기획과-무장"></a>

### Intent and weapons

I requested F-22, Su-57 and MiG-31 fighters in Lux Lunae squadron colors with gold cockpit glass. Durability would be 1.7 times that of the strongest ordinary helicopter, using existing missiles. I specified three-shot bursts for single missiles and three pairs for twin missiles.

| Aircraft | HP | Weapon | Burst | Display length |
|---|---:|---|---|---:|
| F-22 | 765 | Derived from E_Wpn_ASM, damage 25 | Three single shots | 112 |
| Su-57 | 765 | Derived from E_Wpn_ASM, damage 25 | Three single shots | 116 |
| MiG-31 | 765 | Derived from E_0604_Wpn, damage 100 | Three pairs, six missiles total | 120 |

HP is the ordinary Heli definition's maximum of 450 × 1.7. Missile speed increased 25%, from 160 to 200. At my request, the firing-rate increase was reduced from 25% to **10%**. Intervals are divided by 1.1: 0.5 seconds becomes about 0.455, 1 second between bursts becomes about 0.909, and a 5-second reload becomes about 4.545. AI waiting times use the same ratio. Original weapon damage, gravity, spread and lifetime remain unchanged.

<a id="등장-조건"></a>

### Appearance conditions

| Boss fight | F-22 / Su-57 / MiG-31 | Total | Preceding reinforcement mission |
|---|---|---:|---|
| 1-4 | 1 / 1 / 0 | 2 | 1-3 |
| 2-5 | 1 / 1 / 2 | 4 | 2-4 |
| 3-5 | 2 / 2 / 2 | 6 | 3-4 |
| 4-5 | 3 / 3 / 2 | 8 | 4-4 |
| 5-5 | 3 / 2 / 3 | 8 | 5-4 |
| 6-5 | 2 / 3 / 3 | 8 | 6-4 |
| 7-6 | 3 / 3 / 2 | 8 | 7-5 |

Each preceding mission gets four aircraft in its final world section. Boss missions use the actual boss-combat activation signal; preceding missions use a threshold 800 units from the world's right edge. Reinforcements occur once per stage, totaling 72 aircraft over 14 events. Existing boss-clear conditions remain intact.

While playing the regular app, I saw fighters appear suddenly in midair and asked Codex to fix it. Codex moved their spawn points beyond the right edge so the entire aircraft starts off-screen. They do not fire during entry, and pausing also pauses entry. Existing helicopter and VTOL spawn rules remain unchanged.

![Combat with the added fighters](../../public/media/gameplay-20261027/07-fighters.png)

Codex ran 21 core tests and combat, input and off-screen entry tests on iPhone, iPad and Mac. After Codex updated regular and cheat apps on iPhone and iPad, I played the regular app. Full-campaign difficulty tuning remains for later testing.

[Designs, gameplay and tests](../../public/lux-lunae-design-20261027.md) · [Fighters.cpp](../../../Sources/Core/Fighters.cpp)

<a id="다음-기능"></a>

## Further features

Further work includes clearer auto-aim status, tutorial missions 0-0 through 0-4 and separate controls on a folded screen. Extended Android sessions and tablet tests, Mac performance improvements, Tauri notarization and Windows builds also remain. [Detailed status](implementation-status.md)

[Recovery-code design, button fixes and screenshots](save-recovery-20261102.md)
