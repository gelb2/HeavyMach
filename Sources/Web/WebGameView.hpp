#pragma once
// C++ translation of Sources/iOS/HMGameView for the browser host.
// Scope: the normal application (HM_VALIDATION=0). The validation/cheat panel,
// the CPU comparison drawRect path and Mac/AppKit branches are not ported.
#include "WebTypes.hpp"
#include "HMGameButtons.hpp"
#include "WebAudio.hpp"
#include "WebGLRenderer.hpp"
#include "WebGameHUD.hpp"
#include "WebMedia.hpp"
#include "WebMenus.hpp"
#include "CameraShake.hpp"
#include "GameData.hpp"
#include "GameSession.hpp"
#include "KeyboardInput.hpp"
#include "LoadingTransition.hpp"
#include "SaveData.hpp"
#include "TiltState.hpp"
#include "TitleState.hpp"
#include "Viewport.hpp"
#include "AutoAim.hpp"
#include "HUDConfiguration.hpp"
#include <array>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>

namespace hmweb {
struct HostSurface {
    double cssWidth=480,cssHeight=320,devicePixelRatio=1;
    double safeLeft=0,safeTop=0,safeRight=0,safeBottom=0;
    // Screen orientation angle (screen.orientation.angle): 90 corresponds to
    // UIInterfaceOrientationLandscapeRight, 270/-90 to LandscapeLeft.
    int orientationAngle=0;
};
struct Acceleration {bool available=false;double x=0,y=0,z=-1;}; // CoreMotion g units

class GameView {
public:
    GameView(std::string assetRoot,std::string saveDirectory);
    bool initRenderer();
    void loseRenderer();
    bool restoreRenderer();
    void hudSaveResult(bool success);
    void setSurface(const HostSurface& surface);
    void setAcceleration(const Acceleration& sample){_acceleration=sample;}
    // orientationOnly: inactive only because a touch device is in portrait.
    void setApplicationActive(bool active,bool orientationOnly=false);
    // Browser animation callback; runs at most one original frame per call.
    void animationFrame(double now);
    // 50 Hz accelerometer delegate deadline (separate from the frame timer).
    void motionTimer(double now);
    // CSS pixels relative to the canvas, top-left origin. state 0..3 as UITouch phases.
    void pointer(long token,double x,double y,int state);
    bool key(unsigned hidCode,bool pressed);
    void blurKeyboard(){cancelGameInput();}
    std::string debugState();
    void storageUIState(const char* status);
    void transferUIState(bool ready,int mode,bool busy,int source,int target,bool settings,const char* status);
    std::string gameUIDebug() const;
    bool transferAvailable()const{return _transferScreen==3&&_showMenu&&!_session&&!_hudEditing&&!_keyboardSettingsOpen&&_loading.state()==6&&_loadError.empty();}
    // Existing HUD-editor action bridge: -1 opens the editor (SET GAME UI),
    // 0 RESET LAYOUT, 1 CANCEL, 2 APPLY, 3 PREVIEW, 4 SAMPLE/CURRENT SCENE, 5 TOOLS.
    void hudControl(int tag);
    std::string loadError() const {return _loadError;}
private:
    heavymach::gameui::Buttons _gameButtons;
    bool _transferReady=false,_transferBusy=false,_transferSettings=true,_hudToolFocus=false;
    int _transferMode=0,_transferSource=-1,_transferTarget=-1;
    std::string _transferStatus,_storageMessage;
    void drawGameButtons();
    void gameUIAction(int id,int state);
    void persistSave();
    void startSlot(int slot,int mode,int mission,int stage);
    void requestScreen(int screen);
    void prepareScreenResources(int screen);
    void refreshScreen(int screen);
    void drawLoading();
    void finishUpgrade();
    double interactionTime() const;
    void noteInteraction();
    void cancelGameInput();
    void pauseForInterruption();
    void checkInactivity();
    void gameButton(int identifier,int state);
    void consumeGameEvents();
    std::string keyName(unsigned code) const;
    void refreshKeyboardHelp();
    void saveKeyboardBindings();
    void toggleKeyboardSettings();
    void ensureKeyboardAim();
    void updateKeyboardAim();
    void drawKeyboardIndicators();
    int32_t nextOriginalRandom();
    void sampleOriginalAcceleration();
    void updateCanvasGeometry();
    bool worldInputBlocked() const;
    Rect safeContentRect() const;
    void advanceOriginalFrame();
    void frame();
    void renderOpenGLFrame();
    void drawLogicalFrame();
    Rect aimControlRect(int control) const;
    void saveAimMode();
    void setInputAimMode(heavymach::AimMode mode);
    void cancelAimFireInput();
    void refreshAutoFire();
    bool routeAimPoint(Point p,int state,long token);
    void drawAimControls();
    void routePoint(Point point,int state,long token);
    // HMHUDEditor.inc translation (WebHUDEditor.inc).
    heavymach::hud::Configuration currentHUDLayout() const;
    std::string hudLayoutPath() const;
    void loadHUDLayout();
    void prepareHUDEditor();
    void updateHUDEntry();
    void updateHUDEditorGeometry();
    void layoutHUDControls();
    void refreshHUDEditorLabel(const char* error);
    void openHUDEditor();
    void closeHUDEditor();
    void hudEditorAction(int tag);
    bool hudEditorKey(unsigned code,bool pressed);
    void hudEditorPoint(Point p,int state,long token);
    void prepareHUDExample();
    void drawHUDEditor();
    void publishHUDControls();

    HostSurface _surface;
    Acceleration _acceleration;
    std::string _assetRoot,_savePath;
    std::unique_ptr<Audio> _audio;
    std::unique_ptr<Media> _media;
    std::unique_ptr<GLRenderer> _glRenderer;
    heavymach::KeyboardInput _keyboardControls;
    bool _keyboardSettingsOpen=false,_capturingKey=false,_settingsWasPaused=false;
    unsigned _settingsSelection=0;
    std::unique_ptr<Menus> _menus;
    std::unique_ptr<GameHUD> _hud;
    heavymach::TiltState _tilt;
    heavymach::CameraShake _titleShake,_worldShake;
    Point _keyboardAim;
    bool _hasKeyboardAim=false,_keyboardAimVisible=false;
    heavymach::PlayerPassives _passives;
    std::unique_ptr<heavymach::GameSession> _session;
    int _slotIndex=0,_ammoPreview=-1;
    heavymach::SaveSlot *_originalSelectedSlot=nullptr,*_originalWeaponSlot=nullptr;
    int _originalSelectedWeapon=0,_originalWeaponIndex=0;
    bool _gameMenu=false,_leftPressed=false,_rightPressed=false,_upPressed=false,_downPressed=false,_keyboardFire=false;
    float _hpGauge=0,_expGauge=0,_reloadGauge=0;
    heavymach::GameData _data;
    heavymach::SaveData _save;
    std::shared_ptr<const heavymach::SaveSlot> _pendingSaveSnapshot;
    int _transferScreen=0;
    bool _showMenu=false;
    std::string _screenResourceScope;
    unsigned long _screenResourceGeneration=0;
    std::string _loadError;
    double _nextTimerDue=0,_nextMotionDue=0;
    double _lastFrameCallback=0,_frameCallbackInterval=0;int _ticksPerCallback=1;
    heavymach::TitleState _title;
    heavymach::LoadingTransition _loading;
    heavymach::Viewport _viewport{0,0,0};
    double _canvasWidth=480,_canvasHeight=320,_hudScale=1;
    Point _hudOffset;
    bool _wideCanvasEnabled=true,_appliedWideCanvas=true;
    Rect _menuSafeRect;
    std::set<long> _worldTouches,_hudTouches,_heldTouches,_aimCancelledTouches;
    std::map<long,int> _aimTouches;
    std::array<bool,256> _aimBlockedKeys{};
    heavymach::AimMode _preferredAimMode=heavymach::AimMode::manual;
    uintptr_t _drawnAimTarget=0;
    double _aimAcquiredTime=0;
    double _time=0,_retainedWorldTime=0;
    uint32_t _randomState=0;
    heavymach::hud::Configuration _hudLayout,_hudDraft;
    bool _hudLayoutFallback=false,_hudEditing=false,_hudFromMenu=false,_hudExample=false,_hudHideGuides=false;
    int _hudSelected=heavymach::hud::jump;
    std::optional<long> _hudDragToken;
    Point _hudGrab;
    unsigned _hudLayoutWrites=0;
    std::map<long,int> _hudCapturedReceivers;
    std::unique_ptr<heavymach::GameSession> _hudExampleSession;
    heavymach::GameData _hudExampleData;
    heavymach::SaveSlot _hudExampleSlot;
    heavymach::PlayerPassives _hudExamplePassives;
    uint32_t _hudExampleRandom=0;
    bool _hudEntryHidden=true,_orientationHold=false;
    std::string _hudEditorText,_hudPublished;
    heavymach::Vector2 _retainedWorldCamera{};
    bool _active=true,_trace=false,_resumeRequired=false;
    double _lastInteraction=0;
    int _traceSecond=0;
    bool _rendererReady=false,_hudSaving=false;
};
}
