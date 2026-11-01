# Moving progress with a recovery code, without a server

<!-- language-nav:start -->
[한국어](../../public/save-recovery-20261102.md) | [English](save-recovery-20261102.md)
<!-- language-nav:end -->

I wanted to use the experience and weapons I had earned on my iPhone on other devices. I did not want to start over on each platform or create an account and store progress on a server. HeavyMach is a single-player game. What needs restoring is the state of weapons, parts and missions already in the game. I thought that state could be exported as a code and reconstructed on another device.

That idea became **TRANSFER PROGRESS** on the save-slot screen. iOS, iPadOS, Android, native Mac, Web/PWA and Tauri Mac use the same C++ recovery module. Copying a code moves progress without a server or account.

## Starting with the original save slots

There was no need to design a new save model. The original game had three slots and an existing save structure. I directed Codex to build the module around that structure without changing or reinterpreting it. Creating codes during combat would require handling unsaved battle state, so I also limited entry to the save-slot selection screen.

A code contains the original 3,624-byte save file and settings added by the restoration. Experience, mission progress, weapon order, ammunition, tank parts and upgrades move together. The game reads the restored part values to draw the tank again. Images, sounds and game code do not need to be included in the string.

| State included | How it is restored |
|---|---|
| Three original slots | Existing save records and field layout are retained |
| HUD positions | Ten placement values are saved and applied to the new screen size |
| Keyboard controls | Sixteen key bindings are restored |
| Auto-aim settings | Saved settings are restored |

Live enemies, projectiles and the tank’s instantaneous position are not included. Play resumes from the slot’s last save. HUD, key and auto-aim settings apply to the whole app, rather than individual slots.

[Shared-module format](../../../Modules/SaveTransfer/README.md) · [Timing and save policy](../../runtime-policy.md)

## Data to restore, not just a hash

My initial phrase was “recovery hash code.” The implemented string contains the save data itself. A hash alone cannot recover the original progress.

The module serializes saves and settings, applies PackBits-style compression to repeated bytes, then encodes the result with Base64url. The prefix is `HM1-`. It records a version and length, and uses CRC32 to detect truncated or incorrectly pasted data. It adds no encryption or salt. The scope is moving and restoring this game's progress directly.

Creating a code reads the save. It does not generate codes or write files whenever a shot is fired or a key is pressed. Importing a code restores the save **as it was when the code was created**. Later play is not merged automatically.

## The shared module and platform adapters

I directed Codex to share the recovery procedure as we had shared ballistics and movement. Codex implemented encoding, decoding, slot selection, value validation and interrupted-restore handling in `Modules/SaveTransfer`. The original combat core's ballistics, movement, damage calculations and save records were unchanged.

Platform adapters read each app's saves and write restored data to its storage. Apple and Android connect app files and settings; Web/PWA connects browser storage. Tauri connects WebView storage and a separate OS-file backup. Tests also checked that C++ and WebAssembly produce identical codes from identical input.

| Platform | Restore choices offered in the UI |
|---|---|
| iPhone, iPad, native Mac | All three slots and settings |
| Android | All slots or selected source/destination slots; HUD and control settings can be selected |
| Web, PWA, Tauri Mac | All slots or selected source/destination slots; HUD and control settings can be selected |

On restore, the module preserves the existing save as a before-restore code, then records the target state. It writes the data, reads it back for comparison, and clears the pending record once it matches. If the app closes during this process, the next launch reapplies and checks the recorded target. A partially written set of files is not treated as a completed restore.

[Slot and interrupted-restore tests](../../evidence/save-transfer-slots-20261101.json) · [Code and adapters](architecture.md)

## Browser saves and Tauri backups

Web/PWA stores progress in IndexedDB and also uses localStorage for settings. The offline game-file cache is separate from progress storage. Tauri Mac keeps another backup in the app's data directory outside the WebView. If WebView saves are entirely empty on launch, it restores them automatically from OS files. If only some files or settings are missing, it stops game startup and offers backup recovery.

Deleting a browser's site data can delete progress. This is why the game offers PWA installation, persistent-storage requests and recovery codes together. A code kept outside the app can restore its recorded state after site data is removed or the app is reinstalled.

Saves survived tests that moved page time forward by 7, 14, 30 and 40 days and then restored it. Those tests did not advance the browser's internal eviction clock by the same periods, so observation after actual long inactivity remains open. [Date-change results](../../evidence/web-date-retention-20261101.json)

## Why the buttons were rebuilt

The first recovery entry button was implemented separately with UIKit on iOS, native Android buttons, and HTML/CSS on Web and Tauri. The game was already running and the original button implementation was available, but Codex added platform UI. In the Galaxy PWA, fullscreen covered only the game document, leaving the external recovery button out of view.

I directed Codex to use the original game's procedure for buttons inside the game, as it had for **SET GAME UI** in the pause menu. **SET GAME UI** combines the original menu assets' background and lettering, using the game renderer and existing pressed-state and touch handling. The same procedure was applied to recovery entry, recovery actions, HUD-editing tools and key-setting buttons.

Existing menu artwork and buttons were preserved. New English lettering was made as a static sheet. I also rejected the first sheet's fantasy-game-like font and had it redone against the original gold lettering and outlines. Platform features remain for text input and the clipboard.

After the replacement, fixes covered a late COPY completion message overwriting the restore summary, a finger held on an empty area of a modal leaking input into the game after another finger closed it, a remaining finger after HUD editing, and Mac copy/paste menus. [Full incident and fixes](../../reviews/game-button-incident-20261101.md) · [Troubleshooting](troubleshooting.md)

## What was checked

Tests on November 1–2, 2026 used iPhone progress to check restoration on iPad, Android, native Mac, Tauri and PWA. Later button builds were checked for creation, input, review, restore and restart. The table distinguishes the evidence collected for each platform.

| Environment | Result checked |
|---|---|
| iPhone, iPad, native Mac | Recovery UI, save preservation and boundary input tests |
| Native Galaxy | Seven follow-up checks each for regular and cheat apps; existing saves retained |
| Public web and installed Chrome PWA | Twenty web and eighteen PWA checks, restored-data comparison and offline restart |
| Galaxy Chrome and installed PWA | Nine checks each; code restore, HUD, key and auto-aim setting comparisons |
| iPad Safari | Code creation, invalid code, confirmation cancel, restore and restart; slot level, appearance and date checked |
| Tauri Mac | Actual restore, quit and restart; 3,624-byte save, 173-byte HUD and settings matched |

The iPad home-screen PWA showed the updated landing page and recovery panel. A repeat of the entire restore procedure in that installed PWA is not recorded separately from the Safari test. The latest native Mac frame test failed its threshold, with p95 around 25–51 ms. A comparison against the previous version under the same conditions remains open.

[Apple results](../../evidence/game-buttons-apple-20261102.json) · [Android results](../../evidence/game-buttons-android-20261102.json) · [Web and Tauri results](../../evidence/game-buttons-web-tauri-20261102.json) · [Post-deployment checks](../../evidence/game-buttons-web-postdeployment-20261102.json)

<a id="recovery-gallery"></a>

## Gameplay and recovery screens

The following captures show the same game build running on Web, PWA and Tauri Mac. Scenes displaying codes use a separate test save. The image inventory records platform, build and scene provenance.

### Web

Chrome on Mac, using a separate local test origin and a new level-1 slot. The game bundle is the same as the public deployment.

![Mission 1-1 in the Chrome browser](../../public/media/desktop-web-recovery-20261102/web-gameplay.png)

Mission 1-1 in the Chrome browser.

![Creating a code and reviewing the test-slot summary](../../public/media/desktop-web-recovery-20261102/web-create.png)

Creating a code and reviewing the test-slot summary.

![Slot, experience and weapon count read by CHECK CODE](../../public/media/desktop-web-recovery-20261102/web-check.png)

Slot, experience and weapon count read by CHECK CODE.

![Web restore confirmation](../../public/media/desktop-web-recovery-20261102/web-confirm.png)

Web restore confirmation.

![Web slots reopened after restoration](../../public/media/desktop-web-recovery-20261102/web-restored.png)

Web slots reopened after restoration.

### PWA

The same test origin installed as a standalone Chrome PWA on Mac.

![Combat in the installed Chrome PWA](../../public/media/desktop-web-recovery-20261102/pwa-gameplay.png)

Combat in the installed Chrome PWA.

![Creating the test-slot code in the PWA](../../public/media/desktop-web-recovery-20261102/pwa-create.png)

Creating the test-slot code in the PWA.

![PWA slot details after checking the code](../../public/media/desktop-web-recovery-20261102/pwa-check.png)

PWA slot details after checking the code.

![PWA restore confirmation](../../public/media/desktop-web-recovery-20261102/pwa-confirm.png)

PWA restore confirmation.

![Installed PWA slots reopened after restoration](../../public/media/desktop-web-recovery-20261102/pwa-restored.png)

Installed PWA slots reopened after restoration.

### Tauri Mac

Actual app captures. Confirmation and restarted-slot images come from the earlier restore test of the same product build. The gameplay image was captured during this documentation update. No personal recovery-code string is shown.

![Combat in the actual Tauri Mac app](../../public/media/desktop-web-recovery-20261102/tauri-gameplay.png)

Combat in the actual Tauri Mac app.

![Tauri confirmation: mission 7-6, level 19 and 14 weapons](../../public/media/desktop-web-recovery-20261102/tauri-confirm.png)

Tauri confirmation: mission 7-6, level 19 and 14 weapons.

![Level-19 slot after restoring and restarting Tauri](../../public/media/desktop-web-recovery-20261102/tauri-restored.png)

Level-19 slot after restoring and restarting Tauri.

[Capture provenance and original hashes](../../public/media/desktop-web-recovery-20261102/README.md)


## Deployment and remaining work

Web/PWA runs on [GitHub Pages](https://gelb2.github.io/HeavyMach/). Tauri Mac has passed local launch and recovery checks; a public installer and notarization remain. The Windows build is also pending.

The latest release tag is v0.17. Current main includes the subsequent Web/PWA, Tauri, recovery-code and button changes. The next release also needs consistent displayed app versions: iPhone currently shows 1.13 and Android shows 0.16-android-dev. Word wrapping in recovery instructions at 480 logical width and the native Mac frame comparison remain open. [Implementation status](implementation-status.md) · [Remaining work](roadmap.md)
