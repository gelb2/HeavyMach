<a id="heavymach-조작"></a>

# HeavyMach controls

<!-- language-nav:start -->
[한국어](../../public/gameplay-guide.md) | [English](gameplay-guide.md)
<!-- language-nav:end -->

Hold the device in landscape and select a save slot and mission. The regular app unlocks weapons and parts through play. The cheat app's title-screen validation panel lets you choose a mission, weapons and upgrades. The two apps use separate saves.

<a id="터치와-틸트"></a>

## Touch and tilt

Tilt an iPhone, iPad or Android device to move the tank forward or backward. Touch a point on the battlefield to aim the barrel and fire. Use the jump button to clear obstacles and the weapon button to change weapons. Both landscape orientations are supported.

The menu button pauses combat. Sending the app to the background or locking the device also pauses it; resume manually when you return. Combat pauses after 120 seconds without input.

<a id="hud-위치-편집"></a>

## Editing the HUD layout

During play, press MENU, then **SET GAME UI** in the pause menu. Drag buttons, gauges and upper decorations to move them. The tools on the right collapse when dragging starts. Press **TOOLS** to reopen them.

**APPLY** saves the layout, **CANCEL** discards changes and **RESET LAYOUT** restores the default. **PREVIEW** hides the tools to show the game screen. If you entered from combat, **SAMPLE SCENE** shows a sample with a tank, terrain and enemies. Combat stays paused while editing. After applying, press **RESUME GAME**.

When editing with a keyboard, press F6 to switch between moving HUD elements and selecting tool buttons. Use Tab or Shift+Tab to select a tool and Enter to press it.

[Screen examples](../../public/hud-improvement-plan-20261028.md).

<a id="자동조준"></a>

## Auto-aim

Use the HUD's **A button** to turn auto-aim on or off. It selects the nearest living enemy ahead and keeps calculating the barrel angle as the tank moves or jumps and the enemy moves.

- Arrow button: select another target.
- Reticle-shaped fire button: fire along the current barrel direction. Hold it to fire at the weapon's normal rate.
- Corners, diagonal marks and `LOCKED`, `TRACK`, `BLOCKED`: show the selected enemy and aiming state.
- Target leaves the screen: keep the target selected and show its direction at the screen edge.
- Target dies or moves behind the tank: select another enemy ahead.
- No enemy ahead: return the barrel to its default angle. The fire button still works.

In auto-aim mode, battlefield touches do not select targets or fire. The existing dash input remains available. Switching modes clears held-fire input, so press again to fire. Projectiles already in flight and bursts already started follow their original rules.

Auto-aim adjusts the barrel. Projectile speed, spread, gravity and damage come from the weapon. Shots can miss enemies behind terrain or moving quickly.

<a id="키보드"></a>

## Keyboard

| Default key | In combat |
|---|---|
| A / D | Move backward / forward |
| Arrow keys | Adjust aim |
| F | Hold to fire |
| Space | Jump |
| Q / E | Previous / next weapon |
| Shift | Fine movement and aiming |
| R | Cycle auto-aim targets |
| Esc | Pause / resume |
| F2 | Open key settings |

The keyboard **A key moves the tank**. The on-screen **A button controls auto-aim**. In menus, use the arrow keys or Tab/Shift+Tab to move between items and Enter to select. Esc cancels or returns to the previous menu.

In key settings, select an action with up/down, press Enter, then press the new key. Assigning a key already in use swaps the two bindings. Backspace restores defaults; Esc cancels changes. Settings save when confirmed.

iPad supports touch and keyboard together. Releasing movement keys clears the keyboard's simulated tilt and returns control to the sensor. Losing window focus or pausing clears held keys. See the [input policy](../../keyboard-input-policy.md) and [validation results](validation.md) for implementation and physical-keyboard test status.

<a id="복원-코드"></a>

## Recovery codes

On the save slot screen, press **TRANSFER PROGRESS**, or F3 on a keyboard. Use **CREATE CODE** to generate a code from the current save and **COPY** to copy it. On another device, paste the code into the same screen and use **CHECK CODE** to inspect it. Press **RESTORE**, review what will be restored, then press **RESTORE** again to apply it. **CANCEL** cancels the restore; **CLOSE** returns to the save slots.

<a id="무기강화미션"></a>

## Weapons, upgrades and missions

The selection outline in the upper weapon list shows the current weapon. Cannons, spread weapons, missiles, beams and air support have different firing intervals and trajectories. Watch ammunition and reload status while switching weapons. Defeat enemies to gain experience, then choose MOVEMENT, DEFENSE or ATTACK in the level-up window.

Finishing a stage shows the result illustration and buttons for the next stage or mission selection. Progress saves when selecting a slot, advancing to the next stage, reaching game over or returning to the menu. Firing and taking damage update game state in memory.

[Gameplay images](../../../README.en.md) · [Features](features.md) · [Installation](build-and-install.md)
