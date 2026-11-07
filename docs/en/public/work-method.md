# How I directed the work

<!-- language-nav:start -->
[한국어](../../public/work-method.md) | [English](work-method.md)
<!-- language-nav:end -->

I have never worked as a developer in the game industry. In restoring HeavyMach, implementation was the work I assigned to Codex. I decided which game to bring back, what to preserve, and what to add. I also played it myself and pointed out changes in movement, sound, and artwork.

## Plans and reports

I made Codex prepare a development plan and write a report. I read them for missing requirements and implementation choices I could not understand. When a task looked too large, I asked which files it would change and whether it would touch the shared code that contained the game's rules.

These questions came from actual failures. Even with the original game and a working compatibility build available, Codex invented other approaches. To put it in my own terms, I beat Codex into line to stop its bizarre development choices, its attacks on the architecture, and its habit of fixating on one thing, ignoring my instructions, and burning tokens on whatever it wanted to do.

## File writes on every shot

On my iPhone, the rebuilt game stuttered where the compatibility build had not. During the investigation, I asked, “You save the file every time a shot is fired?” The early implementation connected notifications of changes to ammunition, damage, and experience directly to file saving. Small changes during combat became repeated disk writes.

I challenged the decision to couple changes in game state to writes of the save file. Codex separated saving into slot selection, the next stage, game over, and returning to the menu. After the fix, 180 frames of firing produced 0 save writes; returning to the menu produced 1. The existing 3,624-byte save format remained intact. [Problem and fix](troubleshooting.md)

Rendering had a similar problem. In a path through UIKit, Codex repeatedly processed pixels on the CPU and created and composited images. I asked why it was drawing game graphics that way and made it compare the original and compatibility paths. Firing still lagged after the change to OpenGL ES and texture reuse, so audio calls had to be measured separately.

## Scope of the Android port

When I assigned the Android port, I prohibited changes to the files used by the working iPhone, iPad, and Mac versions. The new host belonged in the Android folder and had to build against the existing C++ game code and resources. Connecting display, input, audio, and lifecycle events should not disturb the combat rules of the existing platforms.

The hashes of 2,166 Apple source, project, core, and resource files matched before and after that work. An upgrade-window bug on Android was fixed in its pause and input connections. I checked what had actually changed rather than accepting the phrase “shared code.” [Android implementation and checks](../../public/android-development-20261030.md)

The web version began with [fastutlego's PR #24](https://github.com/gelb2/HeavyMach/pull/24). I required preservation of the contributor's commits and directed Codex to add the Tauri host and web distribution without changing the working web implementation and its 3 follow-up fixes. [The first PR and Tauri](../../public/first-pr-and-tauri-20261031.md)

## Reimplementing the game buttons

Codex made buttons used after the game had started with separate Apple, Android, and web UI controls. The original button assets and input procedure already existed, yet it added platform-specific implementations to maintain. I again directed it to use the original method for in-game buttons and reserve platform integration for functions such as entering a recovery code.

After moving the added buttons into the game renderer and input system, Codex checked that 1,453 original button and resource files were preserved. It pressed the buttons on each platform, generated and entered recovery codes, and checked saves after restarting. [Recovery codes and button fixes](save-recovery-20261102.md)

I required separate task branches and commits so that we could identify which change to undo if a fix went wrong. Before merging into main, Codex also had to compare the manuscripts I had edited directly on GitHub. A completed label in a report was not enough. I kept checking the buttons I would use and the results of actually running the game.

[How I tested the game](testing-method.md) · [Validation results](validation.md)
