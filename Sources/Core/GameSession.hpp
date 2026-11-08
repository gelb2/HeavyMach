#pragma once
#include "World.hpp"
#include "AutoAim.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "ItemBox.hpp"
#include "Trail.hpp"
#if defined(HM_VALIDATION) && HM_VALIDATION
#include "ValidationScenario.hpp"
#endif
#include <variant>
#include <memory>
namespace heavymach {
enum class SessionEventKind { AudioIndex,AudioName,CombatUI,PlayerUI,EnemyUI,WorldUI,SaveChanged,Unsupported,PersistSave };
// code preserves the originating PlayerEventKind/EnemyEventKind/WorldEventKind integer.
struct SessionEvent {
 SessionEventKind kind; int code=0,index=0,amount=0;float value=0,pan=0,delay=0;
 Vector2 position{};std::string text{};bool active=false;
 std::shared_ptr<const SaveSlot> savedSlot; // Snapshot at the original synchronous saveDataFile call.
};
using SessionRenderItem=std::variant<DrawSprite,TrailMesh>;
class GameSession {
public:
 using TileSize=std::function<Vector2(std::string_view)>;
 using Random31=std::function<int32_t()>;
 // Default objects share original random() ordering. Explicit MachineBehavior
 // contracts may select the preserved per-machine random stream.
 // Native host supplies its GuiGame-lifetime passive slots. Default ownership
 // lasts for this session, including every player/stage replacement.
 // An external sharedPassives owner must outlive the session.
 GameSession(const GameData&,SaveSlot&,TileSize,Random31,PlayerPassives* sharedPassives=nullptr,TileSize worldTileSize={});
 ~GameSession();GameSession(GameSession&&) noexcept;GameSession& operator=(GameSession&&) noexcept;
 GameSession(const GameSession&)=delete;GameSession& operator=(const GameSession&)=delete;
 // Original World clock persists across screen/map changes. Fresh sessions begin at0.
 void start(int mode,int mission,int stage);
 // Native host carries the clock when replacing the session object at a menu boundary.
 void start(int mode,int mission,int stage,double retainedWorldTime);
 // QobCamera survives clearAllObject0xd27c/menu transitions with its position.
 void restoreWorldCamera(Vector2 position);
#if defined(HM_VALIDATION) && HM_VALIDATION
 struct ValidationEnemySnapshot { uintptr_t id;std::string name;Vector2 position;int32_t hp,hpMax;bool boss,removed;Vector2 spawnPosition;ObjectBounds bounds; };
 std::vector<ValidationEnemySnapshot> validationEnemies()const;
 // Read-only view of live projectiles for the separate validation host.
 struct ValidationBulletSnapshot {std::string weapon;bool toPlayer,removed;Vector2 position,velocity;double startTime;};
 std::vector<ValidationBulletSnapshot> validationBullets()const;
 void validationSpawnEnemy(const EnemyDefinition&);
 Vector2 validationAimOrigin()const;
 WeaponState validationWeaponState()const;
 // Optional read-only observer. No shot history or per-frame allocation in the app.
 using ValidationShotObserver=std::function<void(const WeaponInfo&,float,Vector2)>;
 void validationObserveShots(ValidationShotObserver);
 using ValidationDamageObserver=std::function<void(uintptr_t,std::string_view,int32_t,int32_t,int32_t)>;
 void validationObserveDamage(ValidationDamageObserver);
 void startValidation(const ValidationScenario&);
 void validationAddExperience(int32_t amount);
 struct ValidationCheatStats {uint64_t blockedHits=0,refilledRounds=0,bossKills=0,fallRescues=0;};
 void validationSetCheats(bool enabled);
 ValidationCheatStats validationCheatStats()const;
#endif
 // Exactly one original 60 Hz tick. tilt is already-filtered original accelY.
 void tick(double globalTime,float tilt);
 // Build a newly loaded paused scene once, without advancing simulation/time.
 void prepareInitialPausedFrame();
 // Arrange only a separately owned HUD example session; never call on live play.
 void prepareHUDPreview();
 // Supply current camera quake (+Y up) before tick, without recollecting draws.
 // Fixed HP images subtract this offset before rounding/culling, then add it
 // back to render positions because the host translates all world draws by -offset.
 void setCameraOffset(Vector2 offset);
 void setCanvasWidth(float width);
 float canvasWidth()const;
 void setCanvasHeight(float height);float canvasHeight()const;
 // Canvas-width x 320 logical coordinates with origin bottom-left; state 0/1/2.
 void touch(Vector2 logical,int state);
 // Cancel/keyboard/outside release has no new aim coordinate. Keep a live burst's target.
 void releaseFire();
 // Optional assisted input. Manual remains the default and original path.
 void setAimMode(AimMode);AimMode aimMode()const;
 void setAutoFire(bool);
 void cycleAimTarget();AimSnapshot aimSnapshot()const;
 void booster();void selectWeapon(int index);void cycleWeapon(int delta=1);
 bool selectUpgrade(int part);void confirmUpgrade();void finishUpgradeSlide();
 void resultButton(int id);void setPaused(bool);
 const WorldState& world()const;const GobHvM_Player* player()const;
 StagePosition stage()const;int selectedWeapon()const;
 size_t enemyCount()const;size_t bulletCount()const;size_t itemCount()const;
 // Draws are already in screen coordinates (+Y up). Host flips Y once.
 std::span<const DrawSprite> sprites()const;
 // Preferred renderer API: preserves sprite/mesh layer ordering.
 std::span<const SessionRenderItem> renderItems()const;
 std::vector<SessionEvent> takeEvents();
private:
 struct Impl;std::unique_ptr<Impl> impl_;
};
}
