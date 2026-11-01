# HeavyMach browser and PWA host

[Play in a browser or install the PWA](https://gelb2.github.io/HeavyMach/).

This folder packages the existing Web game without changing its HTML, JavaScript, WebAssembly or resources. `Desktop/Tauri/` packages the same game for desktop. Apple, Android and original Web sources remain in their existing folders.

## Build and run

Build the game with `Desktop/Tauri/scripts/prepare.py --emsdk /path/to/emsdk` first. On macOS:

```sh
python3 Distribution/Web/scripts/package.py
python3 -m http.server 48726 --directory Distribution/Web/build/site
```

Open `http://localhost:48726/`. **Install game** is the main action. The launcher recommends installation for offline play and save retention, and explains the Home Screen exemption from Safari’s seven-day inactivity cleanup on iPhone and iPad. The initial page downloads only the launcher. Open the installed app to download the game (about 75 MB), or choose **Play in browser** to download and play there. The progress bar shows verified bytes; interrupted downloads reuse verified files. Play starts after every file passes SHA-256 verification. Safari shows home-screen installation instructions; browsers with an installation prompt open that prompt. Once downloaded, the game can start offline. Updates or cleared caches require another download.

`launcher/` contains launcher artwork preparation and CSS shared with `Desktop/Tauri/`. Both build scripts read the original menu assets and create derivatives only in their own generated build folders.

## Saves and updates

The original game writes progress to IndexedDB (`/persist`), HUD layout to the same database, and keyboard/aim settings to two localStorage keys. The host does not add saves during combat. Use **Transfer progress** on the save slot screen to create or restore an HM1 recovery code. Restoration validates the code and retains the previous state. An interrupted restore resumes before the next game starts.

A browser and its installed PWA may share storage, depending on the browser. Both use the same Web Lock to prevent simultaneous play in a shared profile. Different browsers and the native/Tauri Mac applications have separate storage. Clearing site data removes browser progress and offline files; a recovery code can restore progress after the game is downloaded again.

Updates download into a separate versioned cache. Failed downloads do not replace the active version. A completed update waits until old game windows close. New game files live under `game/<content-hash>/`. The service worker never rewrites those files.

## Verification

Uses the repository's existing Puppeteer dependency (`cd Web/tests && npm ci`):

```sh
node Distribution/Web/tests/pwa.mjs http://127.0.0.1:48726/
node Distribution/Web/tests/updates.mjs
node Distribution/Web/tests/launcher.mjs
node Distribution/Web/tests/mobile.mjs http://127.0.0.1:48726/
HM_GPU=1 HM_INSTALLED=1 node Distribution/Web/tests/pwa.mjs https://gelb2.github.io/HeavyMach/
python3 Desktop/Tauri/scripts/protected.py check
```

The default test uses an isolated Chrome profile and SwiftShader. `HM_GPU=1 HM_INSTALLED=1` uses a visible Chrome window and the Mac GPU, installs the PWA through Chrome’s test interface, and checks standalone execution. The browser connection uses a local debugging pipe. They check gameplay, offline restart, save round-trip, corrupt imports, concurrent windows, interrupted imports and update failure/activation. Actual Mac GPU frame measurements and installed-window checks are recorded separately in the project report.

Generated files and browser test evidence stay in ignored `build/`. Publish only `build/site/` to the dedicated GitHub Pages branch. Game source and development branches are not deployment output.

### Date-shift regression

Run `node Distribution/Web/tests/date-retention.mjs` after packaging. It uses disposable persistent Chrome profiles and an installed standalone PWA. It shifts page JavaScript dates by 7, 14, 30 and 40 days, restarts the browser offline, compares progress/settings, saves again, and returns to the original date. The OS clock and browser eviction clock stay unchanged. This verifies date-change handling; actual inactivity eviction is a separate test. Results are written to `build/date-retention/result.json`.

The launcher no longer offers JSON Export save / Import save buttons. Back up and transfer progress through the recovery code options on the game’s save slot screen. Publish this launcher together with the recovery-code build; the older public build still needs its existing backup controls until then.
