# HeavyMach browser and PWA host

[Play in a browser or install the PWA](https://gelb2.github.io/HeavyMach/).

This folder packages the existing Web game without changing its HTML, JavaScript, WebAssembly or resources. `Desktop/Tauri/` packages the same game for desktop. Apple, Android and original Web sources remain in their existing folders.

## Build and run

Build the game with `Desktop/Tauri/scripts/prepare.py --emsdk /path/to/emsdk` first. On macOS:

```sh
python3 Distribution/Web/scripts/package.py
python3 -m http.server 48726 --directory Distribution/Web/build/site
```

Open `http://localhost:48726/`. The first download is about 75 MB. **Play** becomes available after every cached file passes SHA-256 verification. Install from the browser menu or the **Install** button when offered. Installation and first download require a connection. After that, the cached game can start offline.

## Saves and updates

The original game writes progress to IndexedDB (`/persist`), HUD layout to the same database, and keyboard/aim settings to two localStorage keys. The host does not add saves during combat. **Export save** downloads these data as a JSON file. **Import save** validates the checksum and structure before replacing progress; it keeps and downloads the previous data. An interrupted import resumes before the next game starts.

A browser and its installed PWA may share storage, depending on the browser. Both use the same Web Lock to prevent simultaneous play in a shared profile. Different browsers and the native/Tauri Mac applications have separate storage. Clearing site data removes browser progress and offline files; an exported save can restore progress after the game is downloaded again.

Updates download into a separate versioned cache. Failed downloads do not replace the active version. A completed update waits until old game windows close. New game files live under `game/<content-hash>/`. The service worker never rewrites those files.

## Verification

Uses the repository's existing Puppeteer dependency (`cd Web/tests && npm ci`):

```sh
node Distribution/Web/tests/pwa.mjs http://127.0.0.1:48726/
node Distribution/Web/tests/updates.mjs
HM_GPU=1 HM_INSTALLED=1 node Distribution/Web/tests/pwa.mjs https://gelb2.github.io/HeavyMach/
python3 Desktop/Tauri/scripts/protected.py check
```

The default test uses an isolated Chrome profile and SwiftShader. `HM_GPU=1 HM_INSTALLED=1` uses a visible Chrome window and the Mac GPU, installs the PWA through Chrome’s test interface, and checks standalone execution. The browser connection uses a local debugging pipe. They check gameplay, offline restart, save round-trip, corrupt imports, concurrent windows, interrupted imports and update failure/activation. Actual Mac GPU frame measurements and installed-window checks are recorded separately in the project report.

Generated files and browser test evidence stay in ignored `build/`. Publish only `build/site/` to the dedicated GitHub Pages branch. Game source and development branches are not deployment output.
