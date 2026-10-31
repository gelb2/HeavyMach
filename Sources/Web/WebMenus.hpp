#pragma once
// C++ translation of Sources/iOS/HMMenus (slot / mode / mission screens).
#include "WebTypes.hpp"
#include "SaveData.hpp"
#include "GameData.hpp"
#include <cstdint>
#include <functional>
#include <memory>

namespace hmweb {
class Media;
class Menus {
public:
    Menus(Media& media,heavymach::SaveData* save,const heavymach::GameData* data);
    ~Menus();
    // Original hidden World survives screen changes; previews read its clock/pause.
    void setPreviewWorldTime(double time,bool paused);
    void showSlots();
    void showMode();
    void showMissions();
    bool isMissionSelection() const;
    // Called only at LoadingTransition RefreshScreen for screen6.
    void startSelectedGame();
    void showMissions(int mission,int stage);
    // Expanded presentation only; 480 preserves the original layout and hit order.
    void setCanvasWidth(double width,Rect safeRect);
    void draw();
    void tick(double time);
    // Logical canvas coordinates, y-up. state: 0 began, 1 moved, 2 ended, 3 cancelled.
    void touch(Point logical,int state);
    void moveKeyboardFocus(int delta);
    void activateKeyboardFocus();
    Rect keyboardFocusRect();

    std::function<int32_t()> random31;
    std::function<void(int)> changeScreen;
    std::function<void(int,int,int,int)> startGame;
    std::function<void()> saveChanged;
    // GobHvM_Player.init 0x1bb84 notifies the retained game HUD before adding parts.
    std::function<void()> previewPlayerCreated;
    std::function<void(int)> slotSelected;
private:
    struct State;
    std::unique_ptr<State> s;
};
}
