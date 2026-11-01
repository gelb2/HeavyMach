#include "WebGameView.hpp"
#include "Fighters.hpp"
#include "GroundAA.hpp"
#include "VTOL.hpp"
#include "HMAimOverlay.hpp"
#include "CanvasGeometry.hpp"
#include "GlobalInfo.hpp"
#include <emscripten/emscripten.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>

// Browser seams. NSUserDefaults -> localStorage (keys prefixed per app),
// UILabel overlays -> DOM elements, IDBFS flush after the atomic save.
EM_JS(int,hm_pref_get_int,(const char* key,int fallback),{
    try{const v=localStorage.getItem('HeavyMach.'+UTF8ToString(key));return v===null?fallback:(parseInt(v,10)|0);}catch(e){return fallback;}
});
EM_JS(void,hm_pref_set_int,(const char* key,int value),{
    try{localStorage.setItem('HeavyMach.'+UTF8ToString(key),String(value));}catch(e){}
});
EM_JS(char*,hm_pref_get_string,(const char* key),{
    let v=null;try{v=localStorage.getItem('HeavyMach.'+UTF8ToString(key));}catch(e){}
    if(v===null)return 0;return stringToNewUTF8(v);
});
EM_JS(void,hm_pref_set_string,(const char* key,const char* value),{
    try{localStorage.setItem('HeavyMach.'+UTF8ToString(key),UTF8ToString(value));}catch(e){}
});
EM_JS(void,hm_set_overlay,(const char* id,const char* text,int visible),{
    const el=document.getElementById(UTF8ToString(id));if(!el)return;
    el.textContent=UTF8ToString(text);el.style.display=visible?'block':'none';
});
EM_JS(void,hm_open_url,(const char* url),{try{window.open(UTF8ToString(url),'_blank','noopener');}catch(e){}});
EM_JS(void,hm_sync_save,(),{if(Module.hmSyncSave)Module.hmSyncSave();});
EM_JS(void,hm_sync_hud_save,(),{Module.hmSyncSave(ok=>Module._hm_hud_save_result(ok?1:0));});
EM_JS(int,hm_trace_enabled,(),{try{return new URLSearchParams(location.search).get('trace')==='1'?1:0;}catch(e){return 0;}});

namespace hmweb {
using namespace heavymach;
namespace {
constexpr double originalInterval=0.01666666753590107; // Original constant 0x3f91111120000000.
std::string format(const char* fmt,int value){char text[64];std::snprintf(text,sizeof(text),fmt,value);return text;}
}

GameView::GameView(std::string assetRoot,std::string saveDirectory):_assetRoot(std::move(assetRoot)){
    _savePath=saveDirectory+"/HvM.dat";
    // KeyboardBindingsV1: comma-separated HID usages (NSUserDefaults array on Apple).
    if(char* stored=hm_pref_get_string("KeyboardBindingsV1")){
        std::vector<unsigned> mapping;std::stringstream in(stored);std::string item;
        while(std::getline(in,item,','))mapping.push_back(unsigned(std::strtoul(item.c_str(),nullptr,10)));
        std::free(stored);
        if(mapping.size()==KeyboardInput::count||mapping.size()+1==KeyboardInput::count){
            auto candidate=_keyboardControls.bindings;bool valid=true;std::array<bool,256> seen{};
            for(unsigned i=0;i<mapping.size();++i){unsigned code=mapping[i];if(!code||code>=256||code==0xe3||code==0xe7||seen[code]){valid=false;break;}seen[code]=true;candidate[i]=code;}
            if(valid&&mapping.size()<candidate.size()&&seen[candidate.back()]){
                for(unsigned code=4;code<256;++code)if(!seen[code]&&code!=0xe3&&code!=0xe7){candidate.back()=code;break;}
            }
            if(valid)_keyboardControls.bindings=candidate;
        }
    }
    _lastInteraction=interactionTime();
    _preferredAimMode=hm_pref_get_int("AimModeV1",0)==int(AimMode::automatic)?AimMode::automatic:AimMode::manual;
    _canvasWidth=480;_canvasHeight=320;_hudScale=1;
    _wideCanvasEnabled=true;_appliedWideCanvas=true;
    _audio=std::make_unique<Audio>(_assetRoot+"/Audio");
    _media=std::make_unique<Media>(*_audio,_assetRoot);_active=true;_ammoPreview=-1;
    _randomState=uint32_t(uint64_t(emscripten_get_now()*1e6));
    _trace=hm_trace_enabled();
    try{
        _data=GameData::loadDirectory(_assetRoot+"/RawData");
        addVTOLExpansion(_data);
        addFighterExpansion(_data);
        addGroundAAExpansion(_data);
        if(std::filesystem::exists(_savePath)){
            std::ifstream file(_savePath,std::ios::binary);
            if(!file)throw std::runtime_error("Cannot read save file");
            std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
            _save=SaveData::decode({bytes.data(),bytes.size()});
        }
    }catch(const std::exception& error){
        _loadError=error.what();
        std::fprintf(stderr,"HeavyMach load failed; existing save preserved: %s\n",_loadError.c_str());
    }
}

bool GameView::initRenderer(){
    _glRenderer=std::make_unique<GLRenderer>();
    if(!_glRenderer->init()){_loadError="WebGL2 renderer unavailable: "+_glRenderer->lastError();_glRenderer.reset();hm_set_overlay("hm-error",_loadError.c_str(),1);return false;}
    _media->textureRenderer=_glRenderer.get();
    _menus=std::make_unique<Menus>(*_media,&_save,&_data);
    _menus->random31=[this]{return nextOriginalRandom();};
    _menus->slotSelected=[this](int slot){_originalSelectedSlot=&_save.slots.at(slot);};
    _menus->previewPlayerCreated=[this]{
        auto charge=_hud->changeWeaponForSlot(_originalSelectedSlot,_originalSelectedWeapon,_originalWeaponSlot,_originalWeaponIndex,_time);
        if(charge)_ammoPreview=*charge;
    };
    _menus->saveChanged=[this]{persistSave();};
    _menus->changeScreen=[this](int screen){requestScreen(screen);};
    _menus->startGame=[this](int slot,int mode,int mission,int stage){startSlot(slot,mode,mission,stage);};
    for(const char* name:{"Loading_Top.png","Loading_Bottom.png","Loading_Left.png","Loading_Right.png","GameUI_Btm.png","GameUI_Top.png","GameMenu.png","GameMenuSub.png","GameUI_Exp.png","NumSet01.png","GameUI_Gauge.png","GameUI_BlueLight.png","GameUI_RedLight.png","GameUI_Gear.png","Btn_Booster.png","Btn_Weapon.png","WeaponTile.png","NumSet02b.png","SelWeapon.png","ItemTile.png","Text_Mission.png","NumSet_Mission.png"})_media->imageSize(name,"shared");
    _hud=std::make_unique<GameHUD>(*_media);
    _hud->button=[this](int identifier,int state){gameButton(identifier,state);};
    _hud->upgradeClosed=[this]{finishUpgrade();};
    loadHUDLayout();prepareHUDEditor();
    prepareScreenResources(1);
    const double now=interactionTime();
    // The host samples the latest motion at the original 50 Hz delegate interval.
    _nextMotionDue=now+TiltState::interval;
    // Original guest NSTimer @0x20b8 runs through touchHLE's deadline rules.
    _nextTimerDue=now+originalInterval;
    _rendererReady=true;
    if(!_loadError.empty())hm_set_overlay("hm-error",_loadError.c_str(),1);
    return true;
}

void GameView::loseRenderer(){
    // The GL objects are already invalid; stop callbacks before any new draw.
    _rendererReady=false;
    setApplicationActive(false);
    _media->setGlRenderer(nullptr);_media->textureRenderer=nullptr;
    _glRenderer.reset();
}
bool GameView::restoreRenderer(){
    _glRenderer=std::make_unique<GLRenderer>();
    if(!_glRenderer->init())return false;
    _media->textureRenderer=_glRenderer.get();
    // Menus, HUD, session, save, RNG and modal state survive the GPU reset.
    _rendererReady=true;updateCanvasGeometry();
    _lastFrameCallback=0;_frameCallbackInterval=0;
    return true;
}
void GameView::hudSaveResult(bool success){
    if(!_hudSaving)return;
    _hudSaving=false;
    if(!success){refreshHUDEditorLabel("Could not save. Try APPLY again.");publishHUDControls();return;}
    ++_hudLayoutWrites;_hudLayout=_hudDraft;closeHUDEditor();
}

void GameView::persistSave(){
    if(!_loadError.empty())return;
    try{
        auto saved=_save;
        if(_pendingSaveSnapshot)saved.slots.at(_slotIndex)=*_pendingSaveSnapshot;
        const auto bytes=saved.encode();
        // NSDataWritingAtomic: write a sibling file, then rename over HvM.dat.
        const std::string temporary=_savePath+".tmp";
        {
            std::ofstream file(temporary,std::ios::binary|std::ios::trunc);
            file.write(reinterpret_cast<const char*>(bytes.data()),std::streamsize(bytes.size()));
            if(!file){std::fprintf(stderr,"HeavyMach save failed: write\n");return;}
        }
        std::error_code error;std::filesystem::rename(temporary,_savePath,error);
        if(error){std::fprintf(stderr,"HeavyMach save failed: %s\n",error.message().c_str());return;}
        hm_sync_save();
    }catch(const std::exception& error){std::fprintf(stderr,"HeavyMach save rejected: %s\n",error.what());}
}
void GameView::startSlot(int slot,int mode,int mission,int stage){
    try{
        if(_session){_retainedWorldTime=_session->world().gameTime;_retainedWorldCamera=_session->world().camera;}
        _slotIndex=slot;_originalSelectedSlot=&_save.slots.at(slot);_hasKeyboardAim=false;_ammoPreview=-1;_gameMenu=false;_hpGauge=1;_expGauge=0;_reloadGauge=0;
        // Original GameMain keeps GuiGame across screen and slot changes.
        // Its HP needle, gear and closing animations continue while hidden.
        Media* media=_media.get();
        // Prepare every radar/body frame before combat; animation must not decode PNGs.
        for(const auto& art:groundAAImages())media->imageSize(art.name,"shared");
        _session=std::make_unique<GameSession>(_data,_save.slots.at(slot),
            [media](std::string_view name){Size size=media->imageSize(std::string(name),"shared");return Vector2{float(size.width),float(size.height)};},
            [this]{return nextOriginalRandom();},&_passives,
            [media](std::string_view name){Size size=media->imageSize(std::string(name),"world");return Vector2{float(size.width),float(size.height)};});
        // openMap must know the target extent before preparing a new world.
        _showMenu=false;updateCanvasGeometry();
        _session->restoreWorldCamera(_retainedWorldCamera);
        _session->start(mode,mission,stage,_retainedWorldTime);
        _session->setAimMode(_preferredAimMode);
        _showMenu=false;
        _hud->setGameStart(_time);consumeGameEvents();
        // openMap can open an upgrade from pending EXP. Keep its modal pause.
        _session->setPaused(_session->player()&&_session->player()->upgradeStep!=0);updateCanvasGeometry();
        noteInteraction();if(_resumeRequired){pauseForInterruption();_session->prepareInitialPausedFrame();}
    }catch(const std::exception& error){_loadError=error.what();}
}
void GameView::requestScreen(int screen){
    // Original GameMain accepts repeated requests even while shutters move.
    _loading.changeScreen(screen);
}
void GameView::prepareScreenResources(int screen){
    _screenResourceScope="screen"+std::to_string(++_screenResourceGeneration);
    std::vector<std::string> names;
    if(screen==1)names={"Main_BG","Main_Episode","Main_Title","Main_Copyright","Main_TapToStart"};
    if(screen==2)names={"SelMode_BG","Common_Btn_OK","SelMode_Btn_Scenario","SelMode_Btn_Survival"};
    if(screen==3){
        names={"SelSlot_BG","Common_Btn_OK","Common_Btn_DEL","SelSlot_Sel","SelSlot_DelSlot","Common_Btn_YES","Common_Btn_NO","NumSet01","SelSlot_Btn"};
        bool occupied=false,empty=false;for(const auto& slot:_save.slots){occupied|=slot.occupied();empty|=!slot.occupied();}
        if(occupied)names.push_back("SelSlot_ExpGauge");if(empty)names.push_back("SelSlot_Empty");
    }
    if(screen==4){
        names={"SelMission_BG","Common_Btn_OK","SelMission_Btn_Up","SelMission_Btn_Dn","SelMission_Slot","SelMission_Text","SelMission_Btn_Left","SelMission_Btn_Right"};
        for(int i=1;i<=7;++i)names.push_back(format("SelMission_TileMission%02d",i));
        for(int i=1;i<=6;++i)names.push_back(format("SelMission_Stage%02d",i));
    }
    for(const auto& name:names)_media->imageSize(name+".png",_screenResourceScope);
    if(screen==1)_media->imageSize("Extensions/TitleWide.png",_screenResourceScope);
    if(screen==2)_media->imageSize("Extensions/ModeWide.png",_screenResourceScope);
    if(screen==3)_media->imageSize("Extensions/MenuMetal.png",_screenResourceScope);
    if(screen==4)_media->imageSize("Extensions/MissionWide.png",_screenResourceScope);
}
void GameView::refreshScreen(int screen){
    _transferScreen=screen;
    // Called synchronously at loading state3, before this frame's child ticks.
    StagePosition stage{};bool fromGame=bool(_session);
    if(_session){_originalWeaponSlot=&_save.slots.at(_slotIndex);_originalSelectedWeapon=_session->selectedWeapon();_originalWeaponIndex=_originalSelectedWeapon;stage=_session->stage();_retainedWorldTime=_session->world().gameTime;_retainedWorldCamera=_session->world().camera;_menus->setPreviewWorldTime(_retainedWorldTime,_session->world().paused);}
    _session.reset();if(screen!=6)_resumeRequired=false;_media->releaseScope("world");
    std::string oldScope=_screenResourceScope;prepareScreenResources(screen);
    switch(screen){
        case 1:_showMenu=false;_titleShake={};_title=TitleState{};_title.startTime=float(_time);break;
        case 2:_showMenu=true;_menus->showMode();break;
        case 3:_showMenu=true;_menus->showSlots();break;
        case 4:_showMenu=true;
            if(fromGame)_menus->showMissions(stage.mission,stage.stage);else _menus->showMissions();
            break;
        case 6:_menus->startSelectedGame();break;
        default:std::fprintf(stderr,"Unrestored screen ID %d\n",screen);break;
    }
    if(!oldScope.empty())_media->releaseScope(oldScope);
}
void GameView::drawLoading(){
    if(!_loading.visible())return;
    _media->presentationOffset={};
    // Door is a canvas overlay, independent of HUD safe-area scale and clip.
    _media->resetClip();_media->setDrawingOffset({});_media->setDrawingScale(1);_media->setCameraOffset({});
    if(_canvasWidth>480||_canvasHeight>320){
        for(const auto& panel:_loading.expandedPanels(_canvasWidth,_canvasHeight))
            _media->drawExpandedPanel(panel.sprite.tile,panel.sprite.position.x,panel.sprite.position.y,panel.left,panel.right,panel.top,panel.bottom);
    }else for(const auto& sprite:_loading.sprites())
        _media->draw(sprite.tile,sprite.position.x,sprite.position.y,sprite.cols,sprite.rows,sprite.frame,sprite.scale.x,sprite.scale.y,sprite.rotation,sprite.alpha,sprite.blend);
}
void GameView::finishUpgrade(){
    if(!_session)return;
    _session->finishUpgradeSlide();
    if(_resumeRequired)pauseForInterruption();
}
double GameView::interactionTime() const{return emscripten_get_now()/1000.0;}
void GameView::noteInteraction(){_lastInteraction=interactionTime();}
void GameView::cancelGameInput(){
    _aimTouches.clear();
    _leftPressed=_rightPressed=_upPressed=_downPressed=_keyboardFire=false;_hasKeyboardAim=false;_keyboardAimVisible=false;
    _heldTouches.clear();_worldTouches.clear();_hudTouches.clear();_hudCapturedReceivers.clear();if(_hud)_hud->cancelInput();
    if(_session)_session->releaseFire();
    else if(_showMenu&&_menus)_menus->touch({-1000,-1000},3);
    _tilt={};_keyboardControls.cancel();
}
void GameView::pauseForInterruption(){
    cancelGameInput();
    if(!_session)return;
    _resumeRequired=true;
    const auto* player=_session->player();
    // Keep result/upgrade panels; never cover them with an ordinary pause menu.
    const auto& world=_session->world();
    if((world.gameState==0&&(!player||player->upgradeStep==0))||
       (world.gameState!=0&&world.presentation.step>=3)){
        _gameMenu=true;_hud->setMenuVisible(true);
    }
    _session->setPaused(true);
}
void GameView::checkInactivity(){
    if(!_active||!_session||_session->world().paused||_session->world().gameState!=0||_loading.visible())return;
    // Foreground inactivity is elapsed real time, not a catch-up game tick.
    if(!_heldTouches.empty()||_keyboardFire||_leftPressed||_rightPressed||_upPressed||_downPressed||std::fabs(_tilt.y)>.03f){noteInteraction();return;}
    if(interactionTime()-_lastInteraction>=120.0)pauseForInterruption();
}
void GameView::gameButton(int identifier,int state){
    if(!_active||!_session)return;
    noteInteraction();
    const auto& world=_session->world();
    if(world.presentation.step==2&&(identifier==1000||identifier==1001)){
        if(state==2){_resumeRequired=false;_session->resultButton(identifier);}return;
    }
    if(identifier==1000&&state==0){
        if(!world.paused){_session->booster();_media->sound("UI_Click");}
        return;
    }
    if(state!=2)return;
    // GuiGame.handleButton 0x17c4c keeps buttons active and gates actions here.
    if(world.paused&&(identifier==100||identifier==1100))return;
    if(!world.paused&&((identifier>=101&&identifier<=104)||
                       (identifier>=1200&&identifier<=1202)||identifier==1300))return;
    switch(identifier){
        case 100:
            cancelGameInput();_gameMenu=true;_hud->setMenuVisible(true);_session->setPaused(true);_media->sound("UI_Click");break;
        case 101:if(_session->player()&&_session->player()->upgradeStep!=0)return;_resumeRequired=false;_tilt={};noteInteraction();_gameMenu=false;_hud->setMenuVisible(false);_session->setPaused(world.gameState!=0);_media->sound("UI_Click");break;
        case 102:_resumeRequired=false;_session->setPaused(false);_gameMenu=false;_hud->setMenuVisible(false);_media->sound("UI_Click");persistSave();requestScreen(3);break;
        case 103:
            if(world.paused){
                // 0x17c4c: original CFString bytes at file offset0x24964.
                _media->sound("UI_Click");
                hm_open_url("http://www.indiean.com");
            }break;
        case 104:if(_gameMenu&&world.gameState==0){openHUDEditor();_media->sound("UI_Click");}break;
        case 1100:_session->cycleWeapon();_media->sound("Game_ChangeWeapon");break; // 0x17c4c: release, then sfxID4.
        case 1200:case 1201:case 1202:if(!_session->player()||_session->player()->upgradeStep!=1)return;_session->selectUpgrade(identifier-1200);break;
        case 1300:if(!_session->player()||_session->player()->upgradeStep!=1)return;_resumeRequired=false;_session->confirmUpgrade();break;
    }
    consumeGameEvents();
}
void GameView::consumeGameEvents(){
    if(!_session)return;
    static const char* const sounds[]={"UI_Click","UI_Btn_OK","UI_Btn_Cancel","Game_GetItem","Game_ChangeWeapon","Game_WeaponEnd","Game_LevelUp","Game_LowHP","SFX_HitBody01","SFX_HitBody02","SFX_HitBody03","SFX_HitBody04","SFX_GroundHit","SFX_ExplGround","SFX_ExplMach"};
    _originalWeaponSlot=&_save.slots.at(_slotIndex);_originalSelectedWeapon=_session->selectedWeapon();_originalWeaponIndex=_originalSelectedWeapon;
    const auto events=_session->takeEvents();
    for(const auto& e:events){
        if(e.kind==SessionEventKind::AudioIndex&&e.index>=0&&e.index<15)_media->sound(sounds[e.index],e.pan);
        else if(e.kind==SessionEventKind::AudioName)_media->sound(e.text,e.pan);
        else if(e.kind==SessionEventKind::PersistSave){
            _pendingSaveSnapshot=e.savedSlot;persistSave();_pendingSaveSnapshot.reset();
        }
        else if(e.kind==SessionEventKind::CombatUI){
            switch(CombatEventKind(e.code)){
                case CombatEventKind::ReloadGauge:_reloadGauge=e.value;_hud->setReloadGauge(e.value);break;
                case CombatEventKind::ReloadComplete:_ammoPreview=-1;_hud->setReloadComplete(_time);break;
                case CombatEventKind::ChargePreview:_ammoPreview=int(e.value);break;
                default:break;
            }
        }else if(e.kind==SessionEventKind::PlayerUI){
            switch(PlayerEventKind(e.code)){
                case PlayerEventKind::Quake:_worldShake.setQuake(e.value,e.amount,e.delay,_time);break;
                case PlayerEventKind::HPGauge:_hpGauge=e.value;_hud->setHPGauge(e.value);break;
                case PlayerEventKind::UpgradeOpen:_hud->openUpgrade(_save.slots.at(_slotIndex));break;
                case PlayerEventKind::UpgradeClose:_hud->closeUpgrade();break;
                case PlayerEventKind::ExperienceGauge:_expGauge=e.value;break;
                case PlayerEventKind::WeaponInventory:_ammoPreview=-1;_hud->showWeaponChange(_time);break;
                default:break;
            }
        }else if(e.kind==SessionEventKind::WorldUI){
            switch(WorldEventKind(e.code)){
                case WorldEventKind::Quake:_worldShake.setQuake(10,4,.3f,_time);break;
                case WorldEventKind::GameStart:_hud->setGameStart(_time);break;
                case WorldEventKind::ClearTileResources:_media->releaseScope("world");break;
                case WorldEventKind::ChangeScreen:
                    if(_session)requestScreen(e.index);break;
                default:break;
            }
        }else if(e.kind==SessionEventKind::Unsupported)std::fprintf(stderr,"Unrestored behavior: %s\n",e.text.c_str());
    }
}
std::string GameView::keyName(unsigned code) const{
    if(code>=4&&code<=29)return std::string(1,char('A'+code-4));
    if(code>=30&&code<=38)return std::to_string(code-29);
    switch(code){case 39:return "0";case 0x28:return "Enter";case 0x29:return "Esc";case 0x2b:return "Tab";case 0x2c:return "Space";case 0x4f:return "→";case 0x50:return "←";case 0x51:return "↓";case 0x52:return "↑";case 0xe1:return "Shift";case 0xe5:return "Right Shift";case 0x3b:return "F2";default:{char text[16];std::snprintf(text,sizeof(text),"HID %02X",code);return text;}}
}
void GameView::refreshKeyboardHelp(){
    static const char* names[]={"왼쪽 이동","오른쪽 이동","왼쪽 조준","오른쪽 조준","위로 조준","아래로 조준","사격","점프","이전 무기","다음 무기","천천히 이동·조준","선택","일시정지·뒤로","다음 버튼","키 설정","다음 표적"};
    std::string text="키 설정\n↑↓ 선택 · Enter 키 변경 · Backspace 기본값 · Esc 닫기\n같은 키를 지정하면 두 동작의 키가 서로 바뀝니다.\n\n";
    for(unsigned i=0;i<std::size(names);++i)text+=std::string(i==_settingsSelection?"▶":"  ")+" "+names[i]+"  —  "+keyName(_keyboardControls.bindings[i])+"\n";
    if(_capturingKey)text+="\n사용할 키를 누르세요. Esc: 취소";
    hm_set_overlay("hm-keyboard",text.c_str(),_keyboardSettingsOpen);
}
void GameView::saveKeyboardBindings(){
    std::string values;for(auto key:_keyboardControls.bindings){if(!values.empty())values+=",";values+=std::to_string(key);}
    hm_pref_set_string("KeyboardBindingsV1",values.c_str());
}
void GameView::toggleKeyboardSettings(){
    if(!_keyboardSettingsOpen){_settingsWasPaused=_session&&_session->world().paused;cancelGameInput();if(_session)_session->setPaused(true);}
    else{if(_session)_session->setPaused(_settingsWasPaused||_resumeRequired);cancelGameInput();}
    _keyboardSettingsOpen=!_keyboardSettingsOpen;_capturingKey=false;refreshKeyboardHelp();
}
void GameView::ensureKeyboardAim(){
    _keyboardAimVisible=true;
    if(_hasKeyboardAim||!_session)return;
    auto player=_session->player();if(!player)return;auto point=Vector2{player->position.x-_session->world().camera.x,player->position.y-_session->world().camera.y};point.x+=float(_canvasWidth/2);point.y+=160;_keyboardAim={std::min(float(_canvasWidth-8),point.x+100),std::clamp(point.y+20,8.f,312.f)};_hasKeyboardAim=true;
}
bool GameView::key(unsigned code,bool pressed){
    using A=KeyAction;
    if(!_active||!_rendererReady)return false;
    if(_hudEditing)return hudEditorKey(code,pressed);
    if(code<_aimBlockedKeys.size()&&_aimBlockedKeys[code]){if(!pressed)_aimBlockedKeys[code]=false;return true;}
    if(_keyboardSettingsOpen){
        if(!_keyboardControls.set(code,pressed)||!pressed)return true;
        if(_capturingKey){if(code!=0x29&&_keyboardControls.remap(A(_settingsSelection),code))saveKeyboardBindings();_capturingKey=false;}
        else if(code==0x29){toggleKeyboardSettings();return true;}
        else if(code==0x52)_settingsSelection=(_settingsSelection+KeyboardInput::count-1)%KeyboardInput::count;
        else if(code==0x51||code==0x2b)_settingsSelection=(_settingsSelection+1)%KeyboardInput::count;
        else if(code==0x28)_capturingKey=true;
        else if(code==0x2a){_keyboardControls.bindings=KeyboardInput::defaults;saveKeyboardBindings();}
        refreshKeyboardHelp();return true;
    }
    if(!_keyboardControls.handles(code))return false;
    bool changed=_keyboardControls.set(code,pressed);if(!changed)return true;
    noteInteraction();
    _leftPressed=_keyboardControls.down(A::left);_rightPressed=_keyboardControls.down(A::right);
    if(_keyboardControls.is(code,A::left)||_keyboardControls.is(code,A::right)){
        if(!_keyboardControls.moving()){_keyboardControls.virtualTilt=0;_tilt={};}
    }
    if(pressed&&_keyboardControls.is(code,A::settings)){toggleKeyboardSettings();return true;}
    if(_loading.visible())return true;
    if(!pressed&&_keyboardControls.is(code,A::fire)){
        _keyboardFire=false;
        if(_session&&_session->aimMode()==AimMode::automatic){refreshAutoFire();return true;}
        if(_session&&_worldTouches.empty())_session->releaseFire();return true;
    }
    if(pressed&&_keyboardControls.is(code,A::back)){
        if(_session){if(_gameMenu)gameButton(101,2);else if(!worldInputBlocked())gameButton(100,2);}
        else if(_showMenu)requestScreen(3);return true;
    }
    if(_showMenu||(_session&&worldInputBlocked())){
        if(!pressed)return true;
        int delta=0;
        if(_keyboardControls.is(code,A::focus))delta=_keyboardControls.fine()?-1:1;
        else if(_keyboardControls.is(code,A::aimLeft)||_keyboardControls.is(code,A::aimUp))delta=-1;
        else if(_keyboardControls.is(code,A::aimRight)||_keyboardControls.is(code,A::aimDown))delta=1;
        if(delta){if(_showMenu)_menus->moveKeyboardFocus(delta);else _hud->moveKeyboardFocus(delta);}
        if(_keyboardControls.is(code,A::confirm)){if(_showMenu)_menus->activateKeyboardFocus();else _hud->activateKeyboardFocus();}
        return true;
    }
    if(_session){
        if(pressed&&_keyboardControls.is(code,A::fire)){
            _keyboardFire=true;
            if(_session->aimMode()==AimMode::manual){ensureKeyboardAim();_session->touch({float(_keyboardAim.x),float(_keyboardAim.y)},0);}
            else refreshAutoFire();
        }
        if(pressed&&_keyboardControls.is(code,A::nextTarget))_session->cycleAimTarget();
        if(pressed&&_keyboardControls.is(code,A::jump))gameButton(1000,0);
        if(pressed&&(_keyboardControls.is(code,A::nextWeapon)||_keyboardControls.is(code,A::previousWeapon))){
            _session->cycleWeapon(_keyboardControls.is(code,A::previousWeapon)?-1:1);_media->sound("Game_ChangeWeapon");
        }
        consumeGameEvents();
    }else if(pressed&&_keyboardControls.is(code,A::confirm)&&_title.released(_time))requestScreen(3);
    return true;
}
void GameView::updateKeyboardAim(){
    if(!_session||worldInputBlocked()||_keyboardSettingsOpen||_session->aimMode()!=AimMode::manual)return;
    if(_keyboardControls.aiming()){
        ensureKeyboardAim();float x=float(_keyboardAim.x),y=float(_keyboardAim.y);_keyboardControls.aim(x,y,float(_canvasWidth),float(_canvasHeight));_keyboardAim={x,y};noteInteraction();
        if(_keyboardFire)_session->touch({x,y},1);
    }
}
void GameView::drawKeyboardIndicators(){
    if(_loading.visible())return;
    Rect rect{};
    if(_showMenu)rect=offset(_menus->keyboardFocusRect(),0,(_canvasHeight-320)/2);
    else if(_session&&worldInputBlocked()){rect=_hud->keyboardFocusRect();rect=makeRect(rect.origin.x*_hudScale+_hudOffset.x,rect.origin.y*_hudScale+_hudOffset.y,rect.size.width*_hudScale,rect.size.height*_hudScale);}
    // Manual aim coordinates remain active; no extra marker over the battlefield.
    _glRenderer->resetClip();_glRenderer->drawInputOutline(rect);
}
int32_t GameView::nextOriginalRandom(){
    // touchHLE libc/stdlib.rs random(): shared xorshift32, not Darwin random().
    uint32_t state=std::max(_randomState,uint32_t(1));
    state^=state<<13;state^=state>>17;state^=state<<5;
    _randomState=state;return int32_t(state&0x7fffffff);
}
void GameView::motionTimer(double now){
    if(!_active||!_rendererReady)return;
    constexpr double interval=TiltState::interval;
    if(now<_nextMotionDue)return;
    _nextMotionDue=_nextMotionDue+interval*std::ceil(std::max((now-_nextMotionDue)/interval,1.0));
    sampleOriginalAcceleration();
}
void GameView::sampleOriginalAcceleration(){
    if(!_active)return;
    if(_keyboardControls.moving()){
        float y=_keyboardControls.sample();
        // Digital release has no physical sensor settling; stop immediately.
        if(!_keyboardControls.moving())_tilt={};else _tilt.sample(0,y,-std::sqrt(1-y*y));
        return;
    }
    float x=0,y=0,z=-1;
    if(_acceleration.available){
        // The browser host converts DeviceMotion m/s^2 to CoreMotion-style g.
        // The same SDL float round trip as iOS is applied before filtering.
        constexpr float gravity=9.80665f;
        x=-float(-_acceleration.x*gravity)/gravity;
        y=-float(-_acceleration.y*gravity)/gravity;
        z=-float(-_acceleration.z*gravity)/gravity;
        // Device axes stay attached to the portrait body; landscape-right flips X/Y.
        if(_surface.orientationAngle==90){x=-x;y=-y;}
    }
    _tilt.sample(x,y,z);
}
void GameView::setApplicationActive(bool active,bool orientationOnly){
    // Web-only (2026-10-29 user decision): a phone or tablet passing through
    // portrait while it is turned only holds the game. Time, input and sound
    // stop as for any inactive state, but no pause menu opens and returning to
    // landscape continues. Any other interruption (hidden tab, lost focus,
    // pagehide) is the iOS resign-active path, also when it arrives during a hold.
    if(!active&&orientationOnly){
        if(!_active)return;
        _orientationHold=true;cancelGameInput();
        _audio->setApplicationActive(false);
        if(_glRenderer)_glRenderer->finish();_active=false;return;
    }
    if(!active&&_orientationHold){
        _orientationHold=false;
        if(_loading.visible())_resumeRequired=true;
        pauseForInterruption();return;
    }
    // Resign-active happens before background. The duplicate background call
    // must not issue GL commands after the app has already become inactive.
    if(_active==active)return;
    if(!active){
        if(_loading.visible())_resumeRequired=true;
        pauseForInterruption();
        _audio->setApplicationActive(false);
        if(_glRenderer)_glRenderer->finish();_active=false;
    }else{
        _orientationHold=false;
        _active=true;_tilt={};noteInteraction();_audio->setApplicationActive(true);
        const double now=interactionTime();
        _nextTimerDue=now+originalInterval;_nextMotionDue=now+TiltState::interval;
        // Pause/menu state is intentionally retained until an explicit action.
    }
}
void GameView::setSurface(const HostSurface& surface){
    _surface=surface;
    if(_rendererReady)updateCanvasGeometry();
    if(_keyboardSettingsOpen)refreshKeyboardHelp();
    // Original EAGLView::layoutSubviews (0x27dc) only resizes its surface.
}
void GameView::updateCanvasGeometry(){
    const double scale=_surface.devicePixelRatio;
    const int dw=int(std::lround(_surface.cssWidth*scale));
    const int dh=int(std::lround(_surface.cssHeight*scale));
    if(!_session||!_session->world().paused)_appliedWideCanvas=_wideCanvasEnabled;
    // Apple devices are landscape and at most 4:3 (iPad, 480x360). A browser
    // window can be taller than that; keep the logical canvas at 4:3 there and
    // let Viewport::drawable letterbox the remaining height.
    const double canvasDrawableHeight=std::min(double(dh),double(dw)*3/4);
    auto canvas=CanvasGeometry::forDrawable(dw,canvasDrawableHeight,_appliedWideCanvas);
    // A paused scene keeps its cached draw commands; apply mode changes before
    // the next simulation tick, never by collecting stateful visuals twice.
    _canvasWidth=canvas.width;_canvasHeight=canvas.height;
    _viewport=Viewport::drawable(dw,dh,scale,_canvasWidth,_canvasHeight);
    _hudScale=1;_hudOffset={(_canvasWidth-480)/2,(_canvasHeight-320)/2};
    _menuSafeRect=makeRect(0,0,_canvasWidth,_canvasHeight);
    if((_canvasWidth>480||_canvasHeight>320)&&_viewport.scale>0){
        Rect safe=safeContentRect();
        const double left=std::max(0.0,(minX(safe)-_viewport.x)/_viewport.scale);
        const double right=std::min(_canvasWidth,(maxX(safe)-_viewport.x)/_viewport.scale);
        const double bottom=std::max(0.0,_canvasHeight-(maxY(safe)-_viewport.y)/_viewport.verticalScale());
        const double top=std::min(_canvasHeight,_canvasHeight-(minY(safe)-_viewport.y)/_viewport.verticalScale());
        _menuSafeRect=makeRect(left,bottom,right-left,top-bottom);
        _hudScale=std::max(.01,std::min({1.0,(right-left)/480,(top-bottom)/320}));
        _hudOffset={(left+right-480*_hudScale)/2,(bottom+top-320*_hudScale)/2};
    }
    updateHUDEditorGeometry();
    _menus->setCanvasWidth(_canvasWidth,offset(_menuSafeRect,0,-(_canvasHeight-320)/2));
    if(!_hudEditing&&_session&&_session->canvasHeight()!=float(_canvasHeight))_session->setCanvasHeight(float(_canvasHeight));
    if(!_hudEditing&&_session&&_session->canvasWidth()!=float(_canvasWidth))_session->setCanvasWidth(float(_canvasWidth));
}
bool GameView::worldInputBlocked() const{
    if(!_session)return true;
    return _loading.visible()||_gameMenu||_session->world().paused||_session->world().gameState!=0;
}
Rect GameView::safeContentRect() const{
    return makeRect(_surface.safeLeft,_surface.safeTop,_surface.cssWidth-_surface.safeLeft-_surface.safeRight,_surface.cssHeight-_surface.safeTop-_surface.safeBottom);
}
// Original IPA renderScene @0x21e0 increments this exact double once, then
// ticks and draws once. A wall-time accumulator can alternate zero/two ticks
// on adjacent display callbacks and visibly stutter even at a steady 60 Hz.
void GameView::advanceOriginalFrame(){
    if(_hudEditing)return;
    _time+=originalInterval;
    if(_session)_worldShake.tick(_time);
    else if(!_showMenu)_titleShake.tick(_time);
    if(!_loadError.empty()||_keyboardSettingsOpen)return;
    _loading.tick([this](LoadingEvent event){
        switch(event.kind){
            case LoadingEventKind::CloseSound:_media->sound("Loading_Close");break;
            case LoadingEventKind::OpenSound:_media->sound("Loading_Open");break;
            case LoadingEventKind::RefreshScreen:refreshScreen(event.screen);break;
        }
    });
    if(!_loadError.empty())return;
    if(_showMenu){_menus->tick(_time);_passives.tick();_hud->tick(_time);return;}
    if(_session){
        updateKeyboardAim();
        // Pause menus and interrupted upgrades must also freeze passive timers.
        if(_gameMenu||(_resumeRequired&&_session->world().gameState==0)){_hud->tick(_time);return;}
        try{
            float input=_tilt.y;
            // Samples have already been mapped to the original landscape axes.
            _session->setCameraOffset(_worldShake.offset);
            _session->tick(_time,input);consumeGameEvents();_hud->tick(_time);
            if(_trace&&_session&&int(_time)!=_traceSecond){
                _traceSecond=int(_time);const auto* p=_session->player();auto stage=_session->stage();
                std::printf("HM_TRACE t=%d mission=%d stage=%d x=%.2f y=%.2f hp=%d exp=%d level=%d enemies=%zu bullets=%zu gameState=%d paused=%d fireKey=%d\n",_traceSecond,stage.mission,stage.stage,p?p->position.x:0,p?p->position.y:0,p?p->hp:0,_save.slots[_slotIndex].experience(),_save.slots[_slotIndex].level(),_session->enemyCount(),_session->bulletCount(),_session->world().gameState,_session->world().paused,_keyboardFire);
            }
        }catch(const std::exception& error){_loadError=error.what();}
        return;
    }
    _passives.tick();_hud->tick(_time);
    for(auto event:_title.tick(_time,[this]{return uint32_t(nextOriginalRandom());})){
        switch(event){
            case TitleEvent::titleIn:_media->sound("Title_TitleIn");break;
            case TitleEvent::bangTitle:_titleShake.setQuake(10,4,.3f,_time);_media->sound("Title_Bang");break;
            case TitleEvent::bangEpisode:_titleShake.setQuake(6,4,.3f,_time);_media->sound("Title_Bang");break;
            case TitleEvent::tapSound:_media->sound("Title_TapToStart");break;
        }
    }
}
void GameView::animationFrame(double now){
    if(!_active||!_rendererReady)return;
    // ns_timer.rs handle_timer advances BEFORE the callback, with ceil. The
    // browser delivers display callbacks instead of a free-running NSTimer, so a
    // callback up to a quarter interval early is accepted as the due tick; on
    // 90/120/144 Hz displays the remaining callbacks are skipped.
    // At 60 Hz this is one tick per callback, as on iOS. Browsers may deliver
    // callbacks at 30 Hz (iOS Low Power Mode, throttled tabs); one tick per
    // callback would then run the game at half speed. The steady callback
    // interval allows up to round(interval/tick) due ticks per callback (max 4),
    // rendered once. A stall (gap over 0.1 s) is never caught up.
    if(_lastFrameCallback>0){
        const double gap=now-_lastFrameCallback;
        if(gap>0&&gap<0.1)_frameCallbackInterval=_frameCallbackInterval>0?_frameCallbackInterval*0.9+gap*0.1:gap;
    }
    _lastFrameCallback=now;
    _ticksPerCallback=std::clamp(int(std::lround(_frameCallbackInterval/originalInterval)),1,4);
    int ticks=0;
    while(ticks<_ticksPerCallback&&now>=_nextTimerDue-originalInterval/4){
        ++ticks;
        if(ticks==_ticksPerCallback)_nextTimerDue=_nextTimerDue+originalInterval*std::ceil(std::max((now-_nextTimerDue)/originalInterval,1.0));
        else _nextTimerDue+=originalInterval;
        if(ticks==1){if(!_active)return;updateCanvasGeometry();updateHUDEntry();if(!_hudEditing)checkInactivity();}
        advanceOriginalFrame();
    }
    if(ticks)renderOpenGLFrame();
}
void GameView::frame(){
    if(!_active)return;
    updateCanvasGeometry();updateHUDEntry();if(!_hudEditing)checkInactivity();
    advanceOriginalFrame();
    renderOpenGLFrame();
}
// Production submits directly to the WebGL drawing buffer. Readback is never used to present.
void GameView::renderOpenGLFrame(){
    if(!_active)return;
    hm_set_overlay("hm-error",_loadError.c_str(),!_loadError.empty());
    if(!_glRenderer||!_loadError.empty())return;
    if(!_glRenderer->setLogicalSize(_canvasWidth,_canvasHeight)){hm_set_overlay("hm-error","Game canvas allocation failed",1);return;}
    if(!_glRenderer->beginFrame()){hm_set_overlay("hm-error","WebGL context unavailable",1);return;}
    _media->setGlRenderer(_glRenderer.get());
    drawLogicalFrame();drawKeyboardIndicators();
    const int dw=int(std::lround(_surface.cssWidth*_surface.devicePixelRatio)),dh=int(std::lround(_surface.cssHeight*_surface.devicePixelRatio));
    if(!_glRenderer->present(_surface.cssWidth,_surface.cssHeight,dw,dh,makeRect(_viewport.x,_viewport.y,_canvasWidth*_viewport.scale,_canvasHeight*_viewport.verticalScale())))
        hm_set_overlay("hm-error","WebGL presentation failed",1);
    _media->setGlRenderer(nullptr);_glRenderer->endFrame();
}
void GameView::drawLogicalFrame(){
    _media->presentationOffset={};
    _media->setCameraOffset({});_media->setDrawingOffset({});_media->setDrawingScale(1);_media->resetClip();_media->resourceScope="shared";
    if(!_loadError.empty())return;
    if(_hudEditing){drawHUDEditor();return;}
    if(_showMenu){_media->resourceScope=_screenResourceScope;
        if(_canvasHeight>320){Size size=_media->imageSize("Extensions/MenuMetal.png");double scale=std::max(_canvasWidth/size.width,_canvasHeight/size.height);_media->draw("Extensions/MenuMetal.png",_canvasWidth/2,_canvasHeight/2,1,1,0,scale,scale,0,1);}
        _media->presentationOffset={0,(_canvasHeight-320)/2};_menus->draw();_media->presentationOffset={};_media->resourceScope="shared";drawLoading();return;}
    if(_session){
        const auto& world=_session->world();
        if(world.paused&&world.hasBackground){std::string asset=world.map.background;_media->resourceScope="world";Size size=_media->imageSize(asset);double scale=std::max(_canvasWidth/std::max(1.0,size.width-1),_canvasHeight/std::max(1.0,size.height-1));_media->draw(asset,_canvasWidth/2,_canvasHeight/2,1,1,0,scale,scale,0,1);}
        _media->setCameraOffset({_worldShake.offset.x,_worldShake.offset.y});
        for(const auto& item:_session->renderItems()){
            if(const auto* sprite=std::get_if<DrawSprite>(&item)){
                _media->resourceScope=sprite->worldTexture?"world":"shared";
                _media->draw(sprite->tile,sprite->position.x,sprite->position.y,sprite->cols,sprite->rows,sprite->frame,sprite->scale.x,sprite->scale.y,sprite->rotation,sprite->alpha,sprite->blend);}
            else{_media->resourceScope="shared";_media->drawTrail(std::get<TrailMesh>(item));}
        }
        _media->setCameraOffset({});_media->resourceScope="shared";
        _media->setDrawingOffset(_hudOffset);_media->setDrawingScale(_hudScale);
        _media->setClipRect(makeRect(_hudOffset.x,_hudOffset.y,480*_hudScale,320*_hudScale));
        GameHUDSnapshot snapshot;
        snapshot.canvasWidth=_canvasWidth;snapshot.canvasHeight=_canvasHeight;snapshot.hudOffset=_hudOffset;snapshot.hudScale=_hudScale;
        snapshot.slot=&_save.slots[_slotIndex];snapshot.data=&_data;snapshot.passives=&_passives;
        snapshot.hpGauge=_hpGauge;snapshot.expGauge=_expGauge;snapshot.reloadGauge=_reloadGauge;
        snapshot.selectedWeapon=_session->selectedWeapon();
        if(snapshot.selectedWeapon>=0&&snapshot.selectedWeapon<snapshot.slot->weaponCount()){
            auto name=snapshot.slot->weaponName(snapshot.selectedWeapon);
            auto found=_data.weapons.find(name);if(found!=_data.weapons.end())snapshot.weapon=&found->second;
            snapshot.ammo=_ammoPreview>=0?_ammoPreview:snapshot.slot->weaponCharge(snapshot.selectedWeapon);
        }
        snapshot.presentation=&world.presentation;snapshot.paused=world.paused;snapshot.mode=world.gameMode;
        auto stage=_session->stage();snapshot.mission=stage.mission;snapshot.stage=stage.stage;
        snapshot.menuVisible=_gameMenu;snapshot.time=_time;
        if(auto player=_session->player()){
            snapshot.upgradeVisible=player->upgradeStep==1;
            snapshot.upgradeSelection=player->upgradeSelection;
        }
        _hud->setLayout(currentHUDLayout(),false);
        _hud->draw(snapshot);
        drawAimControls();
        drawLoading();_media->resetClip();_media->setDrawingOffset({});_media->setDrawingScale(1);return;
    }
    _media->resourceScope=_screenResourceScope;
    if(_canvasHeight>320){Size size=_media->imageSize("Main_BG.png");double scale=std::max(_canvasWidth/(size.width-1),_canvasHeight/(size.height-1));_media->draw("Main_BG.png",_canvasWidth/2,_canvasHeight/2,1,1,0,scale,scale,0,1);}
    _media->presentationOffset={0,(_canvasHeight-320)/2};_media->setDrawingOffset({});
    if(_canvasWidth>480){
        Size size=_media->imageSize("Extensions/TitleWide.png");
        // Draw only the two OUTER quarters of the generated candidate. Its center
        // never enters the frame; the original owns that moving rectangle.
        _media->setCameraOffset({_titleShake.offset.x,_titleShake.offset.y});
        double extensionScale=320/size.height;
        double distance=240+size.width/8*extensionScale;
        for(int side:{-1,1}){
            double dx=side*distance*_title.bgScale;
            _media->draw("Extensions/TitleWide.png",_canvasWidth/2+_title.camX+std::cos(_title.bgRotate)*dx,160+_title.camY+std::sin(_title.bgRotate)*dx,4,1,side<0?0:3,extensionScale*_title.bgScale,extensionScale*_title.bgScale,_title.bgRotate,1);
        }
        _media->setDrawingOffset({(_canvasWidth-480)/2,0});
        _media->drawEdgeContinuation("Main_BG.png",240+_title.camX,160+_title.camY,_title.bgScale,_title.bgRotate,20);
    }
    _media->setCameraOffset({_titleShake.offset.x,_titleShake.offset.y});
    if(_canvasWidth>480)_media->drawJoinedBackground("Main_BG.png",240+_title.camX,160+_title.camY,_title.bgScale,_title.bgRotate);
    else _media->draw("Main_BG.png",240+_title.camX,160+_title.camY,1,1,0,_title.bgScale,_title.bgScale,_title.bgRotate,1);
    if(_canvasWidth>480)_media->setDrawingOffset({midX(_menuSafeRect)-240,0});
    if(_title.showTitle)_media->draw("Main_Title.png",240,80,1,1,0,_title.titleScale,_title.titleScale,0,1);
    if(_title.showEpisode)_media->draw("Main_Episode.png",240,290,1,1,0,_title.episodeScale,_title.episodeScale,0,1);
    _media->draw("Main_Copyright.png",240,_canvasWidth>480?std::max(12.0,minY(_menuSafeRect)+8):12);
    if(_title.showStart)_media->draw("Main_TapToStart.png",240,140);
    _media->setCameraOffset({});
    _media->setDrawingOffset({});
    _media->resourceScope="shared";drawLoading();
}
// One rectangle is consumed by both rendering and hit testing. Coordinates are
// absolute logical canvas points; none of these controls enter world touch().
Rect GameView::aimControlRect(int control) const{
    if(control<1||control>3)return nullRect(); // Removed manual/correction buttons have no hit area.
    // Mount to the same 480-unit HUD rail as booster/weapon, including safe-area scale.
    int slot=control==1?0:control==3?1:2; // A, next target, fire.
    Rect r=makeRect(334+slot*48,10,46,46);
    auto c=currentHUDLayout();auto q=c.group(heavymach::hud::aim).hitRect({r.origin.x,r.origin.y,r.size.width,r.size.height});
    return makeRect(q.x,q.y,q.width,q.height);
}
void GameView::saveAimMode(){hm_pref_set_int("AimModeV1",int(_preferredAimMode));}
void GameView::setInputAimMode(AimMode mode){
    if(!_session||_session->aimMode()==mode)return;
    cancelAimFireInput();_session->setAimMode(mode);_preferredAimMode=mode;saveAimMode();
}
void GameView::cancelAimFireInput(){
    for(long token:_heldTouches)_aimCancelledTouches.insert(token);
    auto keys=_keyboardControls;auto tilt=_tilt;
    unsigned fire=keys.bindings[unsigned(KeyAction::fire)];
    _aimBlockedKeys[fire]=_aimBlockedKeys[fire]||keys.keys[fire];keys.keys[fire]=false;
    cancelGameInput();_keyboardControls=keys;_tilt=tilt;
    _leftPressed=keys.down(KeyAction::left);_rightPressed=keys.down(KeyAction::right);
}
void GameView::refreshAutoFire(){
    if(!_session)return;bool fire=_keyboardFire;
    for(const auto& entry:_aimTouches)if(entry.second==2){fire=true;break;}
    _session->setAutoFire(fire);
}
bool GameView::routeAimPoint(Point p,int state,long token){
    if(!_session)return false;
    auto captured=_aimTouches.find(token);
    if(captured!=_aimTouches.end()){
        if(state==1&&captured->second==2&&!contains(aimControlRect(2),p)){_aimTouches.erase(captured);_aimCancelledTouches.insert(token);refreshAutoFire();return true;}
        if(state>=2){int action=captured->second;_aimTouches.erase(captured);
            if(action==2)refreshAutoFire();
            else if(state==2&&!worldInputBlocked()&&contains(aimControlRect(action),p)){
                if(action==3)_session->cycleAimTarget();
                else if(action==1)setInputAimMode(_session->aimMode()==AimMode::automatic?AimMode::manual:AimMode::automatic);
            }
        }
        return true;
    }
    if(state!=0||worldInputBlocked()||_loading.visible())return false;
    int count=_session->aimMode()==AimMode::automatic?4:2;
    for(int action=1;action<count;++action)if(contains(aimControlRect(action),p)){
        _aimTouches[token]=action;if(action==2)refreshAutoFire();return true;
    }
    return false;
}
void GameView::drawAimControls(){
    if((!_session&&!_hudEditing)||(!_hudEditing&&worldInputBlocked())||_loading.visible())return;
    _media->resetClip();_media->setDrawingOffset({});_media->setDrawingScale(1);
    auto mode=_hudEditing?AimMode::automatic:_session->aimMode();int count=mode==AimMode::automatic?4:2;
    for(int action=1;action<count;++action){auto r=aimControlRect(action);float x=float(midX(r)),y=float(midY(r));
        bool selected=(action==1&&mode==AimMode::automatic);
        for(const auto& held:_aimTouches)if(held.second==action)selected=true;
        const std::string sheet="Extensions/AutoAimControls.png";Size size=_media->imageSize(sheet);
        _media->draw(sheet,x,y,3,2,(selected?3:0)+action-1,r.size.width/std::max(1.0,size.width/3),r.size.height/std::max(1.0,size.height/2),0,1);
    }
    if(_hudEditing||mode!=AimMode::automatic)return;auto aim=_session->aimSnapshot();if(!aim.target){_drawnAimTarget=0;return;}
    if(_drawnAimTarget!=aim.target){_drawnAimTarget=aim.target;_aimAcquiredTime=_time;}
    float gap=float(4+12*std::max(0.,1-(_time-_aimAcquiredTime)/.15));
    float left=aim.screenPosition.x+aim.bounds.left-gap,right=aim.screenPosition.x+aim.bounds.right+gap;
    float bottom=aim.screenPosition.y+aim.bounds.bottom-gap,top=aim.screenPosition.y+aim.bounds.top+gap;
    float red=aim.status==AimStatus::ready?.2f:aim.status==AimStatus::blocked?.55f:1;
    float green=aim.status==AimStatus::ready?1:.65f,blue=aim.status==AimStatus::blocked?.65f:.2f;
    if(right<0||left>_canvasWidth||top<0||bottom>_canvasHeight){
        float cx=(left+right)/2,cy=(bottom+top)/2;
        float x=std::clamp(cx,12.f,float(_canvasWidth-12)),y=std::clamp(cy,12.f,float(_canvasHeight-12));
        float angle=std::atan2(cy-y,cx-x),dx=std::cos(angle),dy=std::sin(angle);
        float lines[]={x-10*dx+6*dy,y-10*dy-6*dx,x,y,x,y,x-10*dx-6*dy,y-10*dy+6*dx};
        _glRenderer->drawAimLines(lines,4,red,green,blue);
    }else{
        float length=8;float lines[]={left,bottom+length,left,bottom,left,bottom,left+length,bottom,right-length,bottom,right,bottom,right,bottom,right,bottom+length,left,top-length,left,top,left,top,left+length,top,right-length,top,right,top,right,top,right,top-length};
        _glRenderer->drawAimLines(lines,16,red,green,blue);
        auto badge=aimHUDLabel(left,right,top,float(_canvasWidth),float(_canvasHeight),aim.status,float(_time-_aimAcquiredTime));
        float leader[]={badge.anchorX,badge.anchorY,badge.elbowX,badge.elbowY,badge.elbowX,badge.elbowY,badge.endX,badge.elbowY};
        _glRenderer->drawAimLines(leader,4,red,green,blue);
        auto text=aimHUDText(badge);
        // A small dark offset keeps the stencil readable over bright backgrounds.
        auto shadow=text;for(size_t i=0;i<shadow.size;i+=2){shadow.points[i]+=.7f;shadow.points[i+1]-=.7f;}
        _glRenderer->drawAimLines(shadow.points.data(),int(shadow.size/2),.06f,.08f,.08f);
        float flash=badge.flash?1:0;
        _glRenderer->drawAimLines(text.points.data(),int(text.size/2),std::max(red,flash),std::max(green,flash),std::max(blue,flash));
    }
}
void GameView::pointer(long token,double x,double y,int state){
    if(!_rendererReady)return;
    routePoint({x,y},state,token);
}
void GameView::routePoint(Point point,int state,long touch){
    if(!_active||!_loadError.empty()||_keyboardSettingsOpen)return;
    if(_hudEditing){auto q=_viewport.logical(point.x,point.y);hudEditorPoint(q?Point{q->first,q->second}:Point{-1000,-1000},state,touch);return;}
    _keyboardAimVisible=false;
    if(_aimCancelledTouches.contains(touch)){if(state>=2)_aimCancelledTouches.erase(touch);return;}
    auto logical=_viewport.logical(point.x,point.y);
    if(state>=2)_heldTouches.erase(touch);
    if(logical){noteInteraction();if(state==0)_heldTouches.insert(touch);}
    if(routeAimPoint(logical?Point{logical->first,logical->second}:Point{-1000,-1000},state,touch))return;
    if(_session&&(_canvasWidth>480||_canvasHeight>320||_hudLayout.customized())){
        const Point p=logical?Point{logical->first,logical->second}:Point{-1000,-1000};
        const Point hp{(p.x-_hudOffset.x)/_hudScale,(p.y-_hudOffset.y)/_hudScale};
        const bool insideHUD=_hudLayout.customized()||(hp.x>=0&&hp.x<=480&&hp.y>=0&&hp.y<=320);
        if(state==0){
            if(insideHUD&&_hud->touch(hp,0)){_hudTouches.insert(touch);if(_hudLayout.customized())_hudCapturedReceivers[touch]=_hud->receiverAt(hp);}
            else if(logical&&!worldInputBlocked()){_worldTouches.insert(touch);_session->touch({float(p.x),float(p.y)},0);_keyboardAim=p;_hasKeyboardAim=true;}
        }else if(_hudTouches.contains(touch)){
            auto receiver=_hudCapturedReceivers.find(touch);
            _hud->touch(insideHUD?hp:Point{-1000,-1000},state,receiver!=_hudCapturedReceivers.end()?receiver->second:-1);
            if(state>=2){_hudTouches.erase(touch);_hudCapturedReceivers.erase(touch);}
        }else if(_worldTouches.contains(touch)){
            if(state>=2){
                _worldTouches.erase(touch);
                if(_worldTouches.empty()){
                    if(state==2&&logical)_session->touch({float(p.x),float(p.y)},2);
                    else if(_session->aimMode()==AimMode::manual)_session->releaseFire();
                }
            }
            else if(logical&&!worldInputBlocked()){_session->touch({float(p.x),float(p.y)},state);_keyboardAim=p;_hasKeyboardAim=true;}
        }
        consumeGameEvents();return;
    }
    if(_session){
        const Point p=logical?Point{logical->first,logical->second}:Point{-1000,-1000};
        // QobManager @0xfc18 dispatches each event in reverse receiver order.
        // There is no per-touch ownership; GobWorld @0xd92c receives only
        // events that no visible HUD button consumed.
        if(!_hud->touch(p,state)){
            if(logical){_keyboardAim=p;_hasKeyboardAim=true;}
            if(state>=2&&(state==3||!logical)){if(_session->aimMode()==AimMode::manual)_session->releaseFire();}
            else _session->touch({float(p.x),float(p.y)},state>=2?2:state);
        }
        consumeGameEvents();return;
    }
    if(!logical){if(_showMenu&&state>=2)_menus->touch({-1000,-1000},3);return;}
    if(_showMenu){_menus->touch({logical->first,logical->second-(_canvasHeight-320)/2},state);return;}
    if(state==2&&_title.released(_time))requestScreen(3);
}
std::string GameView::debugState(){
    char text[768];
    const char* screen=!_rendererReady?"boot":_session?"game":_showMenu?"menu":"title";
    const auto* p=_session?_session->player():nullptr;auto stage=_session?_session->stage():StagePosition{};
    std::snprintf(text,sizeof(text),"{\"screen\":\"%s\",\"time\":%.4f,\"canvas\":[%g,%g],\"loading\":%d,\"mission\":%d,\"stage\":%d,\"playerX\":%.2f,\"playerY\":%.2f,\"hp\":%d,\"enemies\":%zu,\"bullets\":%zu,\"paused\":%d,\"gameState\":%d,\"decodes\":%u,\"loadError\":%d,\"aimMode\":%d,\"aimTarget\":%d,\"inputBlocked\":%d,\"keyboardFire\":%d,\"gameMenu\":%d,\"resumeRequired\":%d,\"active\":%d,\"keyboardAimX\":%.2f,\"callbackInterval\":%.4f,\"ticksPerCallback\":%d,\"motion\":%d,\"tilt\":[%.3f,%.3f,%.3f],\"hudEditing\":%d,\"hudCustomized\":%d,\"hudFallback\":%d,\"hudSelected\":%d,\"hudDraftCustomized\":%d,\"orientationHold\":%d}",
        screen,_time,_canvasWidth,_canvasHeight,int(_loading.visible()),stage.mission,stage.stage,p?p->position.x:0.f,p?p->position.y:0.f,p?p->hp:0,
        _session?_session->enemyCount():size_t(0),_session?_session->bulletCount():size_t(0),_session?int(_session->world().paused):0,_session?_session->world().gameState:0,_media->decodeCount(),int(!_loadError.empty()),
        _session?int(_session->aimMode()):int(_preferredAimMode),_session&&_session->aimSnapshot().target?1:0,int(worldInputBlocked()),int(_keyboardFire),int(_gameMenu),int(_resumeRequired),int(_active),_keyboardAim.x,_frameCallbackInterval,_ticksPerCallback,int(_acceleration.available),_acceleration.x,_acceleration.y,_acceleration.z,int(_hudEditing),int(_hudLayout.customized()),int(_hudLayoutFallback),_hudSelected,int(_hudDraft.customized()),int(_orientationHold));
    return text;
}
}
#include "WebHUDEditor.inc"
