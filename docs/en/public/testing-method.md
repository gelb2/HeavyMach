# How I tested the game

<!-- language-nav:start -->
[한국어](../../public/testing-method.md) | [English](testing-method.md)
<!-- language-nav:end -->

I have no professional game-development experience. In iOS development, however, I learned that unit tests should be quick and easy to run frequently. I also thought about why so many games have cheat codes. Why spend time leveling up and collecting weapons before every check of a restored game? That was why I told Codex to make a separate cheat app that could go straight to the required conditions.

## I first established a running reference

For HeavyMach, we first made a compatibility build that ran the original package. I used that execution as the comparison reference and had Codex write new source code while examining the original data, functions, and screens. We needed a running version to compare what appeared after a button press, when saves occurred, and how weapons traveled.

The regular and cheat apps used the same game code with separate storage. The cheat app could set experience, level, parts, weapons, mission, and starting position. Testing a particular weapon in front of a particular boss no longer required replaying all the preceding progression. [Cheat validation mode](../../validation-mode.md)

## Supplying a weapon and checking what it does

Once the conditions were set, the checks used the actual game paths: weapon selection, firing, ammunition consumption, projectile movement and collision, and touch release. I instructed Codex to test every weapon rather than stop after firing a few basic ones.

On October 25, 2026, the checks combined 20 weapons, 3 screen widths, and 6 release or pause inputs into 360 cases. The same 360 cases ran on iPhone Air; 1,080 projectiles produced 0 target or trajectory violations. These checks followed the fix for air-support attacks falling behind the tank when a touch ended. [Full weapon test procedure](../../weapon-validation-procedure.md)

When auto-aim was added, we also compared manual firing. Running the earlier code and the new code across 20 weapons × 3 seeds × 420 frames produced byte-identical results for 25,200 frames. The checks covered preservation of the existing controls as well as hitting the intended target. [Validation results](validation.md)

## The saves and stutters I challenged after playing

When I asked about frame drops on my iPhone, the early implementation turned out to connect firing, damage, and experience changes to file saving. I had to start by asking, “You save the file every time a shot is fired?” After the fix, 180 frames of firing in the cheat app produced 0 save writes, and returning to the menu produced 1.

Images were repeatedly generated on the CPU through UIKit, and sound effects were restarted through AVAudioPlayer. The p95 cost of the sound-restart call was 58.549ms. After preparing and reusing OpenAL buffers and sources, the p95 playback-call cost was 0.008167ms. We separated the things a shot was waiting for and measured them as we fixed them. [Save, rendering, and audio fixes](troubleshooting.md)

## Fast checks and frame measurements

Checks that changed menus and equipment conditions ran quickly through automation. To examine how steadily the screen updated, combat ran in real time while frame intervals and processing costs were measured.

An early 127.77-second iPhone combat recording averaged 59.71 updates per second, with a p95 frame interval of 16.67ms and a maximum of 50.02ms. The device's thermal state was Serious. At 60fps, one frame interval is about 16.67ms. The p95 is the value around the 95% point when measured intervals are sorted from shortest to longest. The maximum of 50.02ms shows that longer intervals also occurred. [Measurements by device](validation.md)

In the later MachDefense restoration, an iPad combat and HUD-scrolling test recorded p99 16.67ms and 0 intervals above 25ms. The p99 is the corresponding 99% point. I kept each result with its device and scene instead of combining the two games into a single measurement. [The MachDefense test process](https://github.com/gelb2/MachDefense/blob/main/docs/public/testing-method.md)

## I still played the regular app

Teleporting in front of a boss produced enemy and random-state conditions different from natural progression. The first 48-boss run killed 8 bosses; in 40 cases, the player died first. Later checks retained failures while examining starting positions and inputs. [Cheat-test records from that stage](../../validation-mode.md)

I kept playing the regular app and pointing out artwork, sound, and control problems that automation had missed. That is how I found the Android upgrade window that would not close after the first boss, on entering 2-1. Codex reproduced it with the save from that point, fixed it, and ran 12 regression checks each on the emulator and Galaxy. [Android fix records](../../public/android-development-20261030.md)

The cheat app made the required weapons and progression states quick to prepare. The checks of what the game actually did in those states still had to grow. Failures from my play sessions became additional automated checks.

[How I directed the work](work-method.md) · [Troubleshooting](troubleshooting.md)
