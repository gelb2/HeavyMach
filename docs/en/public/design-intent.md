<a id="추가-기능의-기획-의도"></a>

# Intent behind the additions

<!-- language-nav:start -->
[한국어](../../public/design-intent.md) | [English](design-intent.md)
<!-- language-nav:end -->

After restoring the original combat, progression and missions, I requested controls suited to current devices and new enemies. The aim was to retain the original movement, projectile, damage and save rules while extending what was needed. Codex implemented the code and artwork; I played installed apps and specified revisions.

<a id="넓은-화면과-apple-플랫폼"></a>

## Wider screens and Apple platforms

Stretching original artwork to fill a screen changes its shape. I requested a canvas matching the device aspect ratio and showing more world. The added area also had to support touch aiming and firing. Menus retain their center artwork with connected side backgrounds. Doors and mission results cover the whole screen.

iPhone, iPad and Mac were to use the same rules and resources. Shared modules and platform projects under one workspace carry weapon, damage and movement changes into each app. Mac uses AppKit. The keyboard controls all combat and menus; iPad supports touch, tilt and keyboard together. [Code structure](architecture.md)

<a id="자동조준"></a>

## Auto-aim

I referred to target selection and tracking in Armored Core and Ace Combat. The barrel tracks an enemy ahead during movement and jumping, retains it off-screen and switches when it dies or moves behind the tank. Separate buttons cycle targets and fire. ON/OFF switches between this and original touch aiming/firing.

Initially, I requested guaranteed hits. After seeing that Codex's C correction changed projectile speed and spread, I ordered its removal. Current auto-aim selects targets and calculates barrel angle. Projectiles follow their original weapon rules and can miss. [Calculations, buttons and comparisons](features.md#자동조준)

<a id="추가-항공기"></a>

## Added aircraft

Yak, Harrier and F35 aircraft were to retain their real shapes with the original units' colors and textures. They reuse helicopter movement and weapons and fit between existing enemies without replacing them. Quantities refer to mission-level helicopter averages. The first science-fiction reinterpretations were rejected; the production procedure changed to use photographs/drawings for shape and game artwork for style.

F-22, Su-57 and MiG-31 fighters use Lux Lunae colors and gold cockpits. Strong enemies alongside bosses were intended to make evasion and special attacks more consequential. They fire grouped existing missiles. Boss reinforcements increase through 2, 4, 6 and 8 aircraft; preceding missions receive four. [Why Lux Lunae?](lux-lunae-intent-20261028.md)

v0.16 spread spawn points and entry destinations horizontally and vertically while retaining fast off-screen entry. Existing AI takes over afterward, so aircraft can regroup during combat. [Changelog](../../../CHANGELOG.en.md#v016)

<a id="쉴카와-퉁구스카"></a>

## Shilka and Tunguska

I requested separate hull, turret, gun and radar artwork assembled through the original ground units' part and movement behavior. Radar rotates independently of the guns. I checked the bouncing motion and chose to retain its current strength.

Shilka fires six white rounds. Tunguska fires four white rounds and two missiles. They add 152 vehicles across 35 stages while preserving original placements. Both are sized as medium ground units. Matching Shilka's turret height to Tunguska's is an intentional exception I requested after judging part proportions on the small game screen. [Artwork, parts and tests](../../public/ground-aa-art-20261028.md)

<a id="hud-편집"></a>

## HUD editing

I requested movable buttons, gauges and decorations to suit the device and the player's hands. Hit areas had to move with their artwork. Health and firing-cooldown gauge needles and upper decorations were preserved.

The editor opens from the combat pause menu. Arrange the HUD over paused combat or a sample with a tank, terrain and enemies. Right-hand tools collapse while dragging. Apply, cancel and reset are available. [Controls](gameplay-guide.md#hud-위치-편집)

[Current features](features.md) · [Validation](validation.md) · [Roadmap](roadmap.md)

## Moving progress without a server

I wanted to move the progress and weapons earned on my iPhone to other platforms. I chose a code containing the original save slots and settings, without accounts or server infrastructure. I directed Codex to share the codec in C++ and offer it from the slot-selection screen. [Design, implementation and recovery screens](save-recovery-20261102.md)
