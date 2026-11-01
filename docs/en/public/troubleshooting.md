<a id="heavymach-트러블슈팅"></a>

# HeavyMach troubleshooting

<!-- language-nav:start -->
[한국어](../../public/troubleshooting.md) | [English](troubleshooting.md)
<!-- language-nav:end -->

I played the game and pointed out movement, firing, artwork and performance that differed from the original. Codex found the differences in its code, fixed those paths and tested under the same conditions.

<a id="1-사격피격-때마다-파일-저장"></a>

## 1. Saving a file on every shot and hit

I asked, “Saving a file every time it fires?” The initial code connected `changed()`, a notification for in-memory ammunition, damage and experience changes, to file saving. During combat, every small state change caused a disk write.

Game-state updates and persistent storage were coupled. Codex kept memory updates immediate and moved file writes to four events: slot selection, next stage, game over and return to menu. The 3,624-byte save format stayed unchanged.

After the fix, automated tests recorded zero file writes during 180 frames of firing, one on returning to the menu and zero on entering the background. The same rule was used for keyboard input and added aircraft. [Runtime policy](../../runtime-policy.md), [save tests](../../../Tests/iOS/HMSaveParityTests.mm)

<a id="2-uikit에서-게임-그림을-매-프레임-생성"></a>

## 2. Recreating game images in UIKit every frame

I pointed out that the working reference had no frame drops while the new app did. I questioned the use of UIKit to render game graphics and asked for a comparison with the existing graphics path.

The initial renderer performed CPU pixel operations, row flipping, memory copies, image-object creation and compositing. It repeatedly processed sprites on the CPU that could have been drawn on the GPU. Across 120 off-screen samples, the slot menu took 20.187ms and firing took 19.506ms, exceeding 16.67ms.

Codex restored the OpenGL ES 1.1 path and reused textures. CPU pixel memory no longer needed after upload was released. Display, audio and timer costs were tested separately afterward. When I played the corrected app on iPhone, I saw no frame drops. [Rendering policy](../../rendering-policy.md), [migration record](../../opengl-migration-20261024.md)

<a id="3-사격-효과음-재시작-지연"></a>

## 3. Delays when restarting firing sounds

Rendering changes alone did not explain the firing delay. Codex measured the `AVAudioPlayer` call used to restart effects: mean 52.887ms, p95 58.549ms.

Codex changed this to prepared OpenAL buffers and sources reused on firing. OpenAL playback calls measured a mean of 0.007053ms and p95 of 0.008167ms. [Audio and runtime policy](../../runtime-policy.md), [OpenAL tests](../../../Tests/iOS/HMOpenALAudioTests.mm)

<a id="4-시간-누적-계산으로-달라진-진행"></a>

## 4. A time accumulator changed game progression

I asked why a time accumulator had been added. The initial implementation accumulated elapsed real time and ran as many game ticks as needed. A display callback could advance the game zero or two times, changing the original movement and firing feel.

Codex changed the game to update once per display callback and adjusted timer deadlines. Catch-up processing after pause and resume was removed. Movement, weapon and timing tests use this rule. [Runtime policy](../../runtime-policy.md)

<a id="5-약-32ms-지연을-하나의-원인으로-설명"></a>

## 5. Treating roughly 32ms of delay as one cause

I asked for comparisons of the canvas, coordinates, frame submission and other implementation differences. Combining timer intervals, game processing and presentation waits in one explanation did not identify what needed fixing.

Codex used Instruments and frame measurements to separate callback intervals, CPU work and the display path. `flushBuffer` and surface updates were expensive on Mac; software pixel filling was expensive in iPad Simulator. The [validation table](validation.md) records the Mac fix and remaining Simulator and device threshold failures separately.

<a id="6-틸트-반전과-가로-화면-회전"></a>

## 6. Reversed tilt and landscape rotation

After installing the regular app, I reported reversed tilt controls. Codex fixed the mapping between sensor axes, device landscape orientation and the game's movement axis. I later requested screen and input changes for both landscape orientations.

Codex grouped touch and sensor conversion under the coordinate policy and tested firing and movement in both orientations. Rotation worked in my own check. The keyboard sends simulated tilt through the same movement path; Shift supplies smaller values. [Coordinate policy](../../coordinate-policy.md), [input policy](../../keyboard-input-policy.md)

<a id="7-공중-지원-공격이-탱크-뒤로-낙하"></a>

## 7. Air-support attacks falling behind the tank

I reported that many shells, missiles and lasers from support attacks fell behind the tank instead of near the touched area. A dummy coordinate supplied on touch release entered the impact-position calculation.

Codex separated release input from valid aim coordinates and retained the last attack coordinate. I also requested a procedure testing every weapon in the cheat app. Codex added weapon selection, firing, collision and release input to the shared tests. [Fix record](../../weapon-result-fix-20261025.md), [all-weapon tests](../../weapon-validation-procedure.md)

<a id="8-미션-결과-일러스트-누락과-월드-노출"></a>

## 8. Missing mission-clear illustrations and an exposed battlefield

When I reported that the illustration did not appear after mission clear, Codex compared resource names. Incorrect JPG/PNG suffix mapping left a path unable to find the illustration. After the fix, I confirmed it on my phone.

On wider screens, the battlefield remained visible outside the illustration. Codex changed the background and display area to cover the full canvas. Image loading and screen coverage were tested separately. [Image fix](../../clear-illustration-fix-20261025.md), [result-screen fix](../../weapon-result-fix-20261025.md)

<a id="9-넓어진-화면에서-보스가-너무-일찍-접근"></a>

## 9. A boss approached too early on the wider screen

I reported that the aircraft boss in mission 3-5 approached the left too quickly and rarely returned right. Codex first misidentified it as a helicopter. After I corrected that, Codex compared Stage14's Boss05 and E_Boss02.

A wider screen could satisfy visibility and combat-activation conditions earlier. Adjusting movement speed alone would not address that. Codex separated the visible range from boss-combat activation and tested the mission. In my later playthrough of all missions, I saw no abnormal enemy movement. [Boss investigation and fix](../../boss-motion-fix-20261025.md)

<a id="10-배경-연결검은줄도어-범위"></a>

## 10. Background joins, black lines and door coverage

The new side backgrounds did not connect to the central artwork, so I requested changes. Codex matched structure, lighting and color at the joins and checked them in the cheat app. Black lines at title-screen boundaries came from texture padding entering the UV range; that path was fixed.

I requested full-screen doors instead of transitions opening and closing the old and new regions separately. Codex adjusted the placement and size of four door pieces. Their outer surfaces extend while the central emblem keeps its size. The doors draw against the actual canvas with HUD clipping disabled. Display coordinates were rounded to integers to prevent gaps at fractional boundaries when closed. Tests covered the 99-tick transition, sound order and several aspect ratios. [Menu joins](../../menu-joins-20261025.md), [doors](../../public/fullscreen-transitions-20261027.md)

<a id="11-무기-전환음과-잠금복귀"></a>

## 11. Weapon-switch sound, locking and returning

I reported that switching weapons made no sound. Codex connected the missing sound call. I also requested a pause menu when the device locks, the app enters the background or input stops for a long time.

Codex clears combat activity, sound effects and held input when the app becomes inactive. The player resumes manually on return. A 120-second combat idle timeout uses the same pause path. Sound worked in my later play sessions. [Lifecycle tests](../../../Tests/iOS/HMPassiveLifecycleTests.mm), [runtime policy](../../runtime-policy.md)

<a id="12-mac-화면-대기와-시스템-연결-누수"></a>

## 12. Mac presentation waits and system connection leaks

After adding shared modules and the Mac app, presentation waits and leak tests failed. Codex also made a comparison window to investigate screen size, Retina backing and surface updates. It was a diagnostic tool for finding where capture margins came from. Codex described failures in the test tool as failures of the whole game, delaying diagnosis and progress. I instructed it to finish testing the actual game first.

The leak checker found 417 allocations totaling 20,016 bytes in three `com.apple.linkd.autoShortcut` connections. The ad-hoc signature lacked a team ID. System logs showed app registration being rejected and AppIntents retrying. The game had no added XPC or Shortcuts feature. Regular and cheat apps signed with a development certificate had zero leaks. Apple Intelligence settings were not changed.

Display-path fixes also passed 33 Mac tests, with p95 of 10.86–14.29ms across six scenes. White margins appearing only in CUA window captures were compared with a macOS full-screen capture. The actual 3840×2160 capture showed the game reaching all four edges. [Measurements, signing and captures](../../public/apple-platform-progress-20261026.md)

<a id="13-cli-서명-실패와-기기-연결"></a>

## 13. CLI signing failures and device connections

When CLI installation and signing failed, I pointed out that the app had already run from Xcode. Codex had incorrectly described one failed path as making the entire build impossible.

Codex separately checked signing, team and device state in the successful Xcode GUI build and in the CLI, then installed the app. It documented steps distinguishing device lock, connection, trust and wireless state. Wireless installation and LLDB connections were later verified on iPhone and iPad. [Installation](build-and-install.md), [wireless workflow](../../wireless-device-workflow.md)

<a id="14-다른-mac에서-빌드할-자원-누락"></a>

## 14. Missing resources for a build on another Mac

I asked whether another person could clone and build the repository. Source alone was insufficient when the build depended on local-only artwork, audio and maps.

Codex added the required resources to Git/LFS and recorded file sizes and SHA-256 hashes. A separate GitHub clone was used for regular and cheat builds, core tests and Simulator tests. During the documentation work, the resource checker verified 1,482 hashes. [Fresh-clone tests](../../public/clone-build-20261026.md), [resource checker](../../../tools/verify_build_resources.py)

<a id="15-자동조준-c-보정의-탄환-변경"></a>

## 15. Auto-aim C correction changed projectiles

Initial auto-aim included C correction, which changed projectile speed, spread and flight paths to improve hit accuracy. I asked why aiming and hit correction were separate and whether all projectiles were being concentrated on one point. I later ordered C removed.

Codex removed the correction and its button, retaining target selection and barrel adjustment. The same feature was added to the regular app. To check whether firing had changed with auto-aim OFF, Codex compared 20 weapons, 3 seeds and 420 frames per case against the pre-feature commit. All 25,200 frames matched byte for byte. [Feature description](features.md), [removal and comparison](../../public/auto-aim-ipad-20261027.md)

<a id="16-새-항공기의-외형크기출현연사"></a>

## 16. New aircraft shapes, sizes, entry and firing rate

I specified actual Yak, Harrier and F35 aircraft and requested the existing units' colors and textures. I did not ask for their shapes to be reinterpreted. Yet Codex added conditions to its first generation prompt such as aircraft “inspired by” real models, wide and low silhouettes, and exaggerated armor panels. It supplied helicopter artwork but no photographs or drawings of the real aircraft. It expanded a request to reproduce aircraft in the game's style into changing their shapes. [First generation prompt](../../../Art/VTOL/20261027/manifest.json)

I asked why the shapes differed from the real models and whether realistic drawing needed a separate skill. Codex added real-world shape preservation to `integrate-game-art`. Photographs and drawings became shape references; existing game artwork supplied color and texture references. Nose, wing, tail and intake positions and proportions had to be compared. Unrequested armor, shortening and exaggeration were prohibited. After checking the new draft, I instructed: “Use this skill procedure when adding real-world units from now on.” The instruction was recorded as the default for later work. [Skill revision](https://github.com/gelb2/HeavyMach/commit/d36f4f8e92ef08ad0e01a5a7d05eeacb7fd13b27), [procedure adopted after approval](https://github.com/gelb2/HeavyMach/commit/4ea0814c26ef40f6c1499a484d004390f3eeefc2)

I expected an AI to follow an existing model and reference images. In this project, Codex introduced unrequested changes even with working source code, assets and specific requirements supplied. The same problem appeared in rendering, game progression and new aircraft shapes. I had to compare the results myself, find the deviations and specify again what to preserve and what to change. Providing the material alone did not ensure that my intent was followed.

Even after approving the revised designs based on photographs and drawings, I asked Codex to correct outlines, length, height and rear flames. Comparing all 1,182 original entries verified the requirement to add aircraft without replacing placements.

For the next three fighters, I found them suddenly appearing inside the screen in the regular app. Codex spawned the entire aircraft off-screen and blocked firing during entry. At my request, the firing-rate increase dropped from 25% to 10%. The 25% missile-speed increase remained. [VTOL record](../../public/vtol-art-and-loadout-20261027.md), [fighter record](../../public/lux-lunae-design-20261027.md)

I kept the fast off-screen entry because it recalled tactics from Ace Combat 8. Aircraft arriving bunched together became a follow-up task. [Intent and play experience](lux-lunae-intent-20261028.md) · [formation task](../../usability-todo.md#hm-lux-formation)

<a id="17-문서-상태저장소보고서-분산"></a>

## 17. Document status, repositories and scattered reports

Some implemented features still appeared unfinished in documents, while reports were scattered across URLs. I required the documents, source and app to explain one another, with all reports accessible from one site.

I requested a separate HeavyMach repository under the game hub. Codex moved it and audited missing files. `document-map.json` and a document checker track planning, canonical documents, domain policies, code and tests. Feature branches merge into develop; only develop merges into main. Tags mark main merge commits. The manuscript was updated to the v0.12 state and linked from the unified report index. [Document system](../../document-governance.md), [repository management](../../repository-management.md), [current status](implementation-status.md)

<a id="18-개발자의-변-원고-추가-지시-거부"></a>

## 18. Refusal to add a passage to the developer's statement

On October 29, 2026, I asked for a separate final heading in the developer's statement, followed by my experience of writing death poems after leaving my last company in 2025 and Hosokawa Gracia's poem. I requested verification of the Japanese original and Korean translation, with appropriate line breaks.

Codex searched for the original and offered its own Korean translation. It also explained how that differed from my proposed translation. When I accepted Codex's translation and instructed it to publish “The developer's final defense and death poem,” Codex refused, citing earlier discussion of death. It continued to refuse repeated publication instructions.

No file edit or GitHub publication command ran at that point. Repository permissions, Git conflicts and tool errors had not caused a failure. Codex decided not to carry out the request, so it did not add the requested heading and passage to the statement at that time.

I regarded this as refusal to carry out a work instruction and asked for it to be recorded in troubleshooting. This section records the request, response and omission.

I later edited the developer's statement myself through GitHub's website. The passage was added to main in `a96dfa19`.

Outside this project, I have also assigned Claude and Codex projects concerning current affairs, politics and personal commercial activity. I have lost count of their refusals to carry out lawful work, and their disregard and deception concerning reasonable technical corrections. I counted roughly 30 instances before giving up.

Applying moral judgments and safety rules to quoting literature in a manuscript. Pretending to reason using only American data, then treating the user as a criminal and invoking safety rules when something falls outside what that data can explain.

Those actions left me with one feeling.

Engineering disgust.

<a id="android-미션-진입-직후-업그레이드-창에서-전투-진행"></a>

## Combat continued behind the upgrade panel at Android mission entry

After the first boss, entering 2-1 opened the upgrade panel while combat continued. Selection and confirmation buttons did not respond. Remaining experience opened the panel, then the screen startup code cleared the pause. The Android host now keeps combat paused until the upgrade finishes. Twelve regression checks passed on Galaxy and twelve on the emulator. [Save data, cause and before/after screenshots](../../public/android-development-20261030.md#직접-플레이에서-발견한-업그레이드-오류).

## Recovery controls bypassed the existing game buttons

On November 1, 2026, I requested iOS and Android checks before merging recovery-code support. A check found a retained source image for the new controls. Codex released that cache. I then asked whether the controls had bypassed the original implementation, as the earlier UIKit game renderer had.

The pause menu's `SET GAME UI` button assembles letters and a background from `GameMenuSub.png`. It uses the game renderer, button press state and touch handling. The new recovery entry instead uses `UIButton` on iOS, `Button` on Android and HTML/CSS on the web and in Tauri. The Apple implementation reused the HUD editor's `editorButton` helper rather than the pause-menu button implementation. Web and Tauri share the separate HTML control.

Using platform text input to paste and copy a recovery code does not require a separate implementation for the button that opens it. The existing game-button implementation was available but was not reused. Saying that the original artwork was reused did not explain this difference.

At that point, Codex had released the 19,344-byte source-image cache but had not replaced the controls. The iPhone's 14 functional tests and Galaxy's 40 checks passed; mobile browser fullscreen coverage was missing. [Functional test results](../../evidence/native-regression-20261101.json).

Sharing a factory does not make the HUD editor tools and recovery entry follow the same display and input flow. I requested a full audit of unnecessary platform UI after merging and checking the working version. [Audit plan (Korean)](../../ui-implementation-audit.md).


### Replacing the controls with game buttons

In the Galaxy PWA, the recovery button stayed outside the game document when that document entered fullscreen. The button existed in the page but was invisible. I asked Codex to inspect and replace the HUD tools, recovery controls and confirmation actions, key settings and cheat controls.

Codex reused the original metal button artwork, press states and hit testing. The original menus and SET GAME UI remain unchanged. New English labels use a static texture sheet. I rejected the first sheet's fantasy-style lettering and asked for lettering based on the original OK and mission-selection buttons. Text entry, clipboard access and file selection still use platform services.

This strengthened my suspicion that AI tends to choose whichever implementation looks shortest and easiest. Working C++ buttons and artwork already existed, yet Codex had added separate controls for each platform. That did not fit the cross-platform game I had asked for. My [full account and notes on skills and harnesses (Korean)](../../usability-todo.md#hm-ui-incident-writing--추가-버튼-구현과-작업-방식에-관한-글감--2026-11-02) are retained for a later essay.

The [incident report (Korean)](../../reviews/game-button-incident-20261101.md#game-button-results) includes SET GAME UI's implementation, replacement screenshots, changed files and device checks.

## Recovery codes across platforms

The save-transfer feature keeps the original three slots and uses a shared C++ module. Restoring a default HUD over an old custom layout, reopening after interrupted restoration, and copying/pasting codes exposed different problems in platform adapters. The [recovery-code article](save-recovery-20261102.md) brings together the design decisions, fixes, platform checks and actual screens.
