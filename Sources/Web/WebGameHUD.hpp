#pragma once
// C++ translation of Sources/iOS/HMGameHUD (in-game HUD, pause menu, upgrade
// panel, result presentation buttons).
#include "WebTypes.hpp"
#include "Player.hpp"
#include "HUDConfiguration.hpp"
#include "World.hpp"
#include <functional>
#include <memory>
#include <optional>

namespace hmweb {
class Media;
struct GameHUDSnapshot {
    const heavymach::SaveSlot *slot=nullptr;
    const heavymach::GameData *data=nullptr;
    const heavymach::WeaponInfo *weapon=nullptr;
    const heavymach::PlayerPassives *passives=nullptr;
    const heavymach::StagePresentation *presentation=nullptr;
    float hpGauge=1,expGauge=0,reloadGauge=0;
    int ammo=99,mission=0,stage=0,mode=0,selectedWeapon=0;
    bool paused=false,upgradeVisible=false,menuVisible=false;
    int upgradeSelection=0;
    double time=0;
    double canvasHeight=320,canvasWidth=480,hudScale=1;
    Point hudOffset;
};
class GameHUD {
public:
    explicit GameHUD(Media& media);
    ~GameHUD();
    void draw(const GameHUDSnapshot& snapshot);
    void setLayout(const heavymach::hud::Configuration& layout,bool editing);
    void tick(double time);
    void setGameStart(double time);
    void setReloadComplete(double time);
    void showWeaponChange(double time);
    // Retained GlobalInfo pointers may be null or reference a different slot than
    // the slot currently being previewed. Returns the original charge-label update
    // (HMGameHUD returned nil/NSNumber).
    std::optional<int> changeWeaponForSlot(heavymach::SaveSlot* slot,int selectedWeapon,
                                           heavymach::SaveSlot* itemSlot,int itemIndex,double time);
    void setHPGauge(float value);
    void setReloadGauge(float value);
    void openUpgrade(const heavymach::SaveSlot& slot);
    void closeUpgrade();
    void setMenuVisible(bool visible);
    void cancelInput();
    // Logical y-up. state0 began,1 moved,2 ended,3 cancelled. true consumes touch.
    bool touch(Point logical,int state);
    int receiverAt(Point logical);
    bool touch(Point logical,int state,int receiver);
    void moveKeyboardFocus(int delta);
    void activateKeyboardFocus();
    Rect keyboardFocusRect();

    std::function<void(int originalID,int state)> button;
    // Original GuiGame unpauses after the upgrade panel moves past x659.
    std::function<void()> upgradeClosed;
private:
    struct State;
    std::unique_ptr<State> s;
};
}
