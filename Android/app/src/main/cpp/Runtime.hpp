#pragma once
#include "Media.hpp"
#include "HUD.hpp"
#include "Menus.hpp"
#include "TitleState.hpp"
#include "LoadingTransition.hpp"
#include "TiltState.hpp"
#include "CameraShake.hpp"
#include "KeyboardInput.hpp"
#include "CanvasGeometry.hpp"
#include <set>
#include "SaveTransfer.hpp"
#include "FileStore.hpp"
namespace hm {
class Runtime {
 AAssetManager* assets_;std::string files_;
 std::unique_ptr<Media> mediaOwner_;Media* _media;
 heavymach::GameData _data;heavymach::SaveData _save;
 std::unique_ptr<HUD> hudOwner_;HUD* _hud;
 std::unique_ptr<Menus> menuOwner_;Menus* _menus;
 heavymach::PlayerPassives _passives;
 std::unique_ptr<heavymach::GameSession> _session;
 heavymach::TitleState _title;heavymach::LoadingTransition _loading;
 heavymach::TiltState _tilt;heavymach::CameraShake _worldShake,_titleShake;
 heavymach::KeyboardInput _keyboardControls;
 heavymach::hud::Configuration _hudLayout,_hudDraft;
 heavymach::GameData exampleData_;heavymach::SaveSlot exampleSlot_;heavymach::PlayerPassives examplePassives_;
 std::unique_ptr<heavymach::GameSession> exampleSession_;uint32_t exampleRandom_=12345;
 int _slotIndex=0,_ammoPreview=-1;float _hpGauge=1,_expGauge=0,_reloadGauge=0;
 int transferScreen_=0;
 bool _active=false,_showMenu=false,_gameMenu=false,_resumeRequired=false,_keyboardFire=false;
 bool _hudEditing=false,_hudExample=false,_hudHideGuides=false,_hasKeyboardAim=false,_keyboardAimVisible=false;
 bool _keyboardSettingsOpen=false,_capturingKey=false,settingsWasPaused_=false;unsigned _settingsSelection=0;
 bool wide_=true;int _hudSelected=heavymach::hud::jump,dragToken_=-1;CGPoint _hudGrab{},_keyboardAim{};
 std::map<int,int> aimTouches_,hudTouches_;std::set<int> worldTouches_,cancelledTouches_,heldTouches_;std::array<bool,256> blockedKeys_{};
 double _time=0,_retainedWorldTime=0,lastInteraction_=0,realTime_=0,_aimAcquiredTime=0;
 uint32_t random_=0x12345678;uintptr_t _drawnAimTarget=0;
 heavymach::Vector2 _retainedWorldCamera{};heavymach::AimMode _preferredAimMode=heavymach::AimMode::manual;
 heavymach::SaveSlot* _originalSelectedSlot=nullptr;heavymach::SaveSlot* _originalWeaponSlot=nullptr;int _originalSelectedWeapon=0,_originalWeaponIndex=0;
 std::string _screenResourceScope="screen0";unsigned screenGeneration_=0;
 double _canvasWidth=480,_canvasHeight=320,_hudScale=1;CGPoint _hudOffset{};CGRect _menuSafeRect{{0,0},{480,320}};
 std::array<int,4> insets_{};
 uint64_t saveWrites_=0,hudWrites_=0;
#if HM_VALIDATION
 std::optional<heavymach::ValidationScenario> fixture_;
 bool cheats_=false;uint64_t observedShots_=0;std::array<uint8_t,3> fixtureGrades_{};bool fixtureNearLevel_=false;int fixtureBoss_=-1;
#endif
 void preloadScreen(int);void loadData();void persistSave(std::shared_ptr<const heavymach::SaveSlot> snapshot={});
 void loadSettings();void saveSettings();void saveHUD();
 void startSlot(int,int,int,int);void refreshScreen(int);void requestScreen(int s){_loading.changeScreen(s);}
 void consumeGameEvents();void advanceOriginalFrame();void drawLogicalFrame();void drawLoading();void drawAimControls();void drawKeyboardIndicators();
 void gameButton(int,int);void cancelGameInput();void pauseForInterruption();
 void ensureKeyboardAim();void updateKeyboardAim();void refreshAutoFire();void setInputAimMode(heavymach::AimMode);
 bool worldInputBlocked()const;
 void updateCanvasGeometry();heavymach::hud::Configuration currentHUDLayout()const;
 CGRect aimControlRect(int)const;bool routeAimPoint(CGPoint,int,int);
 void openHUDEditor();void closeHUDEditor();void hudEditorPoint(CGPoint,int,int);void prepareHUDExample();void drawHUDEditor();
 int32_t nextOriginalRandom();
bool transferAvailable()const{return transferScreen_==3&&_showMenu&&!_session&&!_hudEditing&&!_keyboardSettingsOpen&&!_loading.visible()&&error.empty();}
 heavymach::transfer::Snapshot transferSnapshot();
 heavymach::transfer::FileStore transferStore();
public:
 std::string transfer(const std::string&,const std::string&,int=-1,int=-1);
 Runtime(AAssetManager*,std::string);~Runtime();
 void surface(ANativeWindow*);void insets(int,int,int,int);void active(bool);
 void frame(double);void sensor(double,double,double);void touch(int,int,float,float);bool key(unsigned,bool);
 // Native UI messages consumed on the Activity thread, never callbacks into a destroyed Activity.
 std::string ui();std::string pendingURL;std::string error;std::string notice;
 void command(const std::string&,int=0,int=0,int=0,int=0);
 std::string diagnostics();Media& media(){return *_media;}
};
}
