#include "Runtime.hpp"
#include "VTOL.hpp"
#include "Fighters.hpp"
#include "GroundAA.hpp"
#include "HMAimOverlay.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
namespace hm {
static void atomicWrite(const std::string& path,const uint8_t* data,size_t n){
 std::string temp=path+".tmp";int fd=::open(temp.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600);if(fd<0)throw std::runtime_error("Cannot open save file");size_t done=0;
 while(done<n){ssize_t count=::write(fd,data+done,n-done);if(count<=0){::close(fd);::unlink(temp.c_str());throw std::runtime_error("Cannot write save file");}done+=size_t(count);}
 int sync=::fsync(fd);int closed=::close(fd);if(sync||closed||::rename(temp.c_str(),path.c_str())){::unlink(temp.c_str());throw std::runtime_error("Cannot commit save file");}
}
static void atomicText(const std::string& path,const std::string& s){atomicWrite(path,reinterpret_cast<const uint8_t*>(s.data()),s.size());}
static std::vector<uint8_t> readLocal(const std::string& path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static bool contains(CGRect r,CGPoint p){return p.x>=r.origin.x&&p.x<CGRectGetMaxX(r)&&p.y>=r.origin.y&&p.y<CGRectGetMaxY(r);}
Runtime::Runtime(AAssetManager* a,std::string files):assets_(a),files_(std::move(files)),mediaOwner_(std::make_unique<Media>(a)),_media(mediaOwner_.get()){
 random_=uint32_t(std::chrono::steady_clock::now().time_since_epoch().count())|1u;
 loadData();auto bytes=readLocal(files_+"/HvM.dat");if(!bytes.empty())_save=heavymach::SaveData::decode(bytes);
 loadSettings();transferStore().recover();hudOwner_=std::make_unique<HUD>(_media);_hud=hudOwner_.get();menuOwner_=std::make_unique<Menus>(_media,&_save,&_data);_menus=menuOwner_.get();
 _hud->button=[this](int i,int s){gameButton(i,s);};_hud->upgradeClosed=[this]{if(_session){_session->finishUpgradeSlide();if(_resumeRequired)pauseForInterruption();consumeGameEvents();}};
 _menus->random31=[this]{return nextOriginalRandom();};_menus->changeScreen=[this](int s){requestScreen(s);};_menus->startGame=[this](int s,int m,int mi,int st){startSlot(s,m,mi,st);};_menus->saveChanged=[this]{persistSave();};
 _menus->slotSelected=[this](int s){_originalSelectedSlot=&_save.slots.at(s);};
 _menus->previewPlayerCreated=[this]{_ammoPreview=_hud->changeWeaponForSlot(_originalSelectedSlot,_originalSelectedWeapon,_originalWeaponSlot,_originalWeaponIndex,_time).value_or(_ammoPreview);};
 for(const char* name:{"Loading_Top.png","Loading_Bottom.png","Loading_Left.png","Loading_Right.png","GameUI_Btm.png","GameUI_Top.png","GameMenu.png","GameMenuSub.png","GameUI_Exp.png","NumSet01.png","GameUI_Gauge.png","GameUI_BlueLight.png","GameUI_RedLight.png","GameUI_Gear.png","Btn_Booster.png","Btn_Weapon.png","WeaponTile.png","NumSet02b.png","SelWeapon.png","ItemTile.png","Text_Mission.png","NumSet_Mission.png"})_media->imageSize(name,"shared");
 refreshScreen(1);_media->audio->active(false);
}
Runtime::~Runtime()=default;
void Runtime::loadData(){
 for(auto v:heavymach::GameData::parseWeapons(assetBytes(assets_,"RawData/WeaponInfo.mch")))_data.weapons.emplace(v.name,v);
 for(auto v:heavymach::GameData::parseParts(assetBytes(assets_,"RawData/PartsInfo.mch")))_data.parts.emplace(v.name,v);
 for(auto v:heavymach::GameData::parseMachines(assetBytes(assets_,"RawData/MachInfo.mch")))_data.machines.emplace(v.name,v);
 AAssetDir* dir=AAssetManager_openDir(assets_,"RawData");if(!dir)throw std::runtime_error("RawData directory missing");
 try{while(const char* n=AAssetDir_getNextFileName(dir)){std::string name(n);if(name.ends_with(".map"))_data.maps.emplace(name.substr(0,name.size()-4),heavymach::GameData::parseMap(assetBytes(assets_,"RawData/"+name)));}}catch(...){AAssetDir_close(dir);throw;}AAssetDir_close(dir);
 heavymach::addVTOLExpansion(_data);heavymach::addFighterExpansion(_data);heavymach::addGroundAAExpansion(_data);
}
int32_t Runtime::nextOriginalRandom(){uint32_t x=random_;x^=x<<13;x^=x>>17;x^=x<<5;random_=x;return int32_t(x&0x7fffffff);}
void Runtime::persistSave(std::shared_ptr<const heavymach::SaveSlot> snapshot){auto saved=_save;if(snapshot)saved.slots.at(_slotIndex)=*snapshot;auto bytes=saved.encode();atomicWrite(files_+"/HvM.dat",bytes.data(),bytes.size());++saveWrites_;}
void Runtime::loadSettings(){
 std::ifstream f(files_+"/InputV1.txt");int mode=1;if(f>>mode){_preferredAimMode=mode==2?heavymach::AimMode::automatic:heavymach::AimMode::manual;auto keys=_keyboardControls.bindings;std::set<unsigned> seen;bool valid=true;for(auto& k:keys)if(!(f>>k)||k==0||k>=256||!seen.insert(k).second)valid=false;if(valid)_keyboardControls.bindings=keys;}
 std::ifstream h(files_+"/HUDLayoutV1.txt");int version=0;if(!(h>>version)||version!=1)return;auto candidate=_hudLayout;
 for(auto& p:candidate.positions){int has;double x,y;if(!(h>>has>>x>>y)||!std::isfinite(x)||!std::isfinite(y)||x<0||x>1||y<0||y>1)return;if(has)p=heavymach::hud::Placement{x,y};else p.reset();}_hudLayout=candidate;
}
void Runtime::saveSettings(){std::ostringstream s;s<<int(_preferredAimMode)<<'\n';for(auto k:_keyboardControls.bindings)s<<k<<' ';s<<'\n';atomicText(files_+"/InputV1.txt",s.str());}
void Runtime::saveHUD(){std::ostringstream s;s.precision(17);s<<"1\n";for(auto p:_hudDraft.positions)s<<(p?1:0)<<' '<<(p?p->x:0)<<' '<<(p?p->y:0)<<'\n';atomicText(files_+"/HUDLayoutV1.txt",s.str());++hudWrites_;_hudLayout=_hudDraft;}
void Runtime::surface(ANativeWindow* w){if(!w){pauseForInterruption();_media->detach();return;}_media->attach(w);updateCanvasGeometry();}
void Runtime::insets(int l,int t,int r,int b){insets_={l,t,r,b};updateCanvasGeometry();}
void Runtime::updateCanvasGeometry(){
 auto c=heavymach::CanvasGeometry::forDrawable(_media->drawableWidth,_media->drawableHeight,wide_);bool changed=c.width!=_canvasWidth||c.height!=_canvasHeight;
 if(changed){cancelGameInput();if(_hudEditing){dragToken_=-1;_hudExample=true;exampleSession_.reset();_media->releaseScope("hud-example");}}
 _canvasWidth=c.width;_canvasHeight=c.height;
 double sx=_media->drawableWidth>0?_canvasWidth/_media->drawableWidth:1,sy=_media->drawableHeight>0?_canvasHeight/_media->drawableHeight:1;
 double l=insets_[0]*sx,r=_canvasWidth-insets_[2]*sx,b=insets_[3]*sy,t=_canvasHeight-insets_[1]*sy;
 _menuSafeRect=CGRectMake(l,b,std::max(1.,r-l),std::max(1.,t-b));_hudScale=std::max(.01,std::min({1.,(r-l)/480,(t-b)/320}));_hudOffset={(l+r-480*_hudScale)/2,(b+t-320*_hudScale)/2};
 _hudLayout.cheatBounds={((l+r)/2-80*_hudScale-_hudOffset.x)/_hudScale,(t-8-32*_hudScale-_hudOffset.y)/_hudScale,160,32};_hudDraft.cheatBounds=_hudLayout.cheatBounds;
 _hudLayout.legacy={{_hudOffset.x,_hudOffset.y},_hudScale};_hudLayout.safe={l,b,r-l,t-b};_hudDraft.legacy=_hudLayout.legacy;_hudDraft.safe=_hudLayout.safe;
 _menus->setCanvasWidth(_canvasWidth,CGRectMake(l,std::max(0.,b-(_canvasHeight-320)/2),r-l,std::min(320.,t-b)));
 if(_session&&!_hudEditing){_session->setCanvasWidth(c.width);_session->setCanvasHeight(c.height);}
}
void Runtime::startSlot(int slot,int mode,int mission,int stage){
 if(_session){_retainedWorldTime=_session->world().gameTime;_retainedWorldCamera=_session->world().camera;}
 _slotIndex=slot;_originalSelectedSlot=&_save.slots.at(slot);_hasKeyboardAim=false;_ammoPreview=-1;_gameMenu=false;_hpGauge=1;_expGauge=_reloadGauge=0;
 for(const auto& art:heavymach::groundAAImages())_media->imageSize(art.name,"shared");
 _session=std::make_unique<heavymach::GameSession>(_data,_save.slots.at(slot),[this](std::string_view n){auto z=_media->imageSize(std::string(n),"shared");return heavymach::Vector2{float(z.width),float(z.height)};},[this]{return nextOriginalRandom();},&_passives,[this](std::string_view n){auto z=_media->imageSize(std::string(n),"world");return heavymach::Vector2{float(z.width),float(z.height)};});
 _showMenu=false;updateCanvasGeometry();_session->restoreWorldCamera(_retainedWorldCamera);
#if HM_VALIDATION
 if(fixture_){auto f=*fixture_;fixture_.reset();_session->startValidation(f);}else _session->start(mode,mission,stage,_retainedWorldTime);
 _session->validationSetCheats(cheats_);_session->validationObserveShots([this](const auto&,float,heavymach::Vector2){++observedShots_;});
#else
 _session->start(mode,mission,stage,_retainedWorldTime);
#endif
 _session->setAimMode(_preferredAimMode);_hud->setGameStart(_time);consumeGameEvents();_session->setPaused(_session->player()&&_session->player()->upgradeStep!=0);lastInteraction_=realTime_;if(_resumeRequired){pauseForInterruption();_session->prepareInitialPausedFrame();}
}
void Runtime::preloadScreen(int screen){
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
  for(int i=1;i<=7;++i)names.push_back(fmt("SelMission_TileMission%02d",i));
  for(int i=1;i<=6;++i)names.push_back(fmt("SelMission_Stage%02d",i));
 }
 for(const auto& name:names)_media->imageSize(name+".png",_screenResourceScope);
 const char* extension=screen==1?"TitleWide":screen==2?"ModeWide":screen==3?"MenuMetal":screen==4?"MissionWide":nullptr;
 if(extension)_media->imageSize(std::string("Extensions/")+extension+".png",_screenResourceScope);
}
void Runtime::refreshScreen(int screen){
 transferScreen_=screen;
 heavymach::StagePosition stage{};bool fromGame=bool(_session);
 if(_session){_originalWeaponSlot=&_save.slots.at(_slotIndex);_originalSelectedWeapon=_originalWeaponIndex=_session->selectedWeapon();stage=_session->stage();_retainedWorldTime=_session->world().gameTime;_retainedWorldCamera=_session->world().camera;_menus->setPreviewWorldTime(_retainedWorldTime,_session->world().paused);}
 _session.reset();if(screen!=6)_resumeRequired=false;_media->releaseScope("world");auto old=_screenResourceScope;_screenResourceScope="screen"+std::to_string(++screenGeneration_);
 switch(screen){case 1:_showMenu=false;_titleShake={};_title={};_title.startTime=float(_time);break;case 2:_showMenu=true;_menus->showMode();break;case 3:_showMenu=true;_menus->showSlots();break;case 4:_showMenu=true;if(fromGame)_menus->showMissions(stage.mission,stage.stage);else _menus->showMissions();break;case 6:_menus->startSelectedGame();break;default:throw std::runtime_error("Unknown game screen "+std::to_string(screen));}
 preloadScreen(screen);_media->releaseScope(old);
}
void Runtime::consumeGameEvents(){
 if(!_session)return;using namespace heavymach;
 static const char* sounds[]={"UI_Click","UI_Btn_OK","UI_Btn_Cancel","Game_GetItem","Game_ChangeWeapon","Game_WeaponEnd","Game_LevelUp","Game_LowHP","SFX_HitBody01","SFX_HitBody02","SFX_HitBody03","SFX_HitBody04","SFX_GroundHit","SFX_ExplGround","SFX_ExplMach"};
 _originalWeaponSlot=&_save.slots.at(_slotIndex);_originalSelectedWeapon=_originalWeaponIndex=_session->selectedWeapon();
 for(const auto& e:_session->takeEvents()){
  if(e.kind==SessionEventKind::AudioIndex&&e.index>=0&&e.index<15)_media->sound(sounds[e.index],e.pan);
  else if(e.kind==SessionEventKind::AudioName)_media->sound(e.text,e.pan);
  else if(e.kind==SessionEventKind::PersistSave)persistSave(e.savedSlot);
  else if(e.kind==SessionEventKind::CombatUI)switch(CombatEventKind(e.code)){case CombatEventKind::ReloadGauge:_reloadGauge=e.value;_hud->setReloadGauge(e.value);break;case CombatEventKind::ReloadComplete:_ammoPreview=-1;_hud->setReloadComplete(_time);break;case CombatEventKind::ChargePreview:_ammoPreview=int(e.value);break;default:break;}
  else if(e.kind==SessionEventKind::PlayerUI)switch(PlayerEventKind(e.code)){case PlayerEventKind::Quake:_worldShake.setQuake(e.value,e.amount,e.delay,_time);break;case PlayerEventKind::HPGauge:_hpGauge=e.value;_hud->setHPGauge(e.value);break;case PlayerEventKind::UpgradeOpen:cancelGameInput();_gameMenu=false;_hud->setMenuVisible(false);_hud->openUpgrade(_save.slots.at(_slotIndex));break;case PlayerEventKind::UpgradeClose:_hud->closeUpgrade();break;case PlayerEventKind::ExperienceGauge:_expGauge=e.value;break;case PlayerEventKind::WeaponInventory:_ammoPreview=-1;_hud->showWeaponChange(_time);break;default:break;}
  else if(e.kind==SessionEventKind::WorldUI)switch(WorldEventKind(e.code)){case WorldEventKind::Quake:_worldShake.setQuake(10,4,.3f,_time);break;case WorldEventKind::GameStart:_hud->setGameStart(_time);break;case WorldEventKind::ClearTileResources:_media->releaseScope("world");break;case WorldEventKind::ChangeScreen:requestScreen(e.index);break;default:break;}
  else if(e.kind==SessionEventKind::Unsupported)throw std::runtime_error(e.text);
 }
}
void Runtime::cancelGameInput(){gameButtons_.cancel();dragToken_=-1;aimTouches_.clear();hudTouches_.clear();worldTouches_.clear();heldTouches_.clear();_hud->cancelInput();_keyboardControls.cancel();_keyboardFire=false;_hasKeyboardAim=false;_keyboardAimVisible=false;_tilt={};if(_session){_session->releaseFire();_session->setAutoFire(false);}else if(_showMenu)_menus->touch({-1000,-1000},3);}
bool Runtime::worldInputBlocked()const{return !_session||_gameMenu||_session->world().paused||_session->world().gameState!=0;}
void Runtime::pauseForInterruption(){cancelGameInput();if(!_session)return;_resumeRequired=true;auto*p=_session->player();const auto&w=_session->world();if((w.gameState==0&&(!p||p->upgradeStep==0))||(w.gameState!=0&&w.presentation.step>=3)){_gameMenu=true;_hud->setMenuVisible(true);}_session->setPaused(true);}
void Runtime::active(bool on){if(_active==on)return;_active=on;if(!on)pauseForInterruption();else{_tilt={};lastInteraction_=realTime_;}_media->audio->active(on);}
void Runtime::sensor(double x,double y,double z){if(!_active||_hudEditing||_keyboardSettingsOpen||worldInputBlocked())return;if(_keyboardControls.moving()){float v=_keyboardControls.sample();_tilt.sample(0,v,-std::sqrt(1-v*v));}else _tilt.sample(x,y,z);}
void Runtime::gameButton(int id,int state){
 if(!_active||!_session)return;lastInteraction_=realTime_;const auto&w=_session->world();if(w.presentation.step==2&&(id==1000||id==1001)){if(state==2){_resumeRequired=false;_session->resultButton(id);}return;}
 if(id==1000&&state==0){if(!w.paused){_session->booster();_media->sound("UI_Click");}return;}if(state!=2)return;
 if(w.paused&&(id==100||id==1100))return;if(!w.paused&&((id>=101&&id<=104)||(id>=1200&&id<=1202)||id==1300))return;
 switch(id){case 100:cancelGameInput();_gameMenu=true;_hud->setMenuVisible(true);_session->setPaused(true);_media->sound("UI_Click");break;
 case 101:if(!_gameMenu||(_session->player()&&_session->player()->upgradeStep!=0))return;_resumeRequired=false;_tilt={};_gameMenu=false;_hud->setMenuVisible(false);_session->setPaused(w.gameState!=0);_media->sound("UI_Click");break;
 case 102:_resumeRequired=false;_session->setPaused(false);_gameMenu=false;_hud->setMenuVisible(false);_media->sound("UI_Click");persistSave();requestScreen(3);break;
 case 103:if(w.paused){_media->sound("UI_Click");pendingURL="http://www.indiean.com";}break;
 case 104:if(_gameMenu&&w.gameState==0){openHUDEditor();_media->sound("UI_Click");}break;
 case 1100:_session->cycleWeapon();_media->sound("Game_ChangeWeapon");break;
 case 1200:case 1201:case 1202:if(_session->player()&&_session->player()->upgradeStep==1)_session->selectUpgrade(id-1200);break;
 case 1300:if(_session->player()&&_session->player()->upgradeStep==1){_resumeRequired=false;_session->confirmUpgrade();}break;}
 consumeGameEvents();
}
void Runtime::advanceOriginalFrame(){
 if(_hudEditing)return;_time+=0.01666666753590107;if(_session)_worldShake.tick(_time);else if(!_showMenu)_titleShake.tick(_time);if(_keyboardSettingsOpen)return;
 _loading.tick([this](heavymach::LoadingEvent e){switch(e.kind){case heavymach::LoadingEventKind::CloseSound:_media->sound("Loading_Close");break;case heavymach::LoadingEventKind::OpenSound:_media->sound("Loading_Open");break;case heavymach::LoadingEventKind::RefreshScreen:refreshScreen(e.screen);break;}});
 if(_showMenu){_menus->tick(_time);_passives.tick();_hud->tick(_time);return;}
 if(_session){updateKeyboardAim();if(_gameMenu||(_resumeRequired&&_session->world().gameState==0)){_hud->tick(_time);return;}_session->setCameraOffset(_worldShake.offset);_session->tick(_time,_tilt.y);consumeGameEvents();_hud->tick(_time);return;}
 _passives.tick();_hud->tick(_time);for(auto e:_title.tick(_time,[this]{return uint32_t(nextOriginalRandom());}))switch(e){case heavymach::TitleEvent::titleIn:_media->sound("Title_TitleIn");break;case heavymach::TitleEvent::bangTitle:_titleShake.setQuake(10,4,.3f,_time);_media->sound("Title_Bang");break;case heavymach::TitleEvent::bangEpisode:_titleShake.setQuake(6,4,.3f,_time);_media->sound("Title_Bang");break;case heavymach::TitleEvent::tapSound:_media->sound("Title_TapToStart");break;}
}
void Runtime::frame(double real){realTime_=real;if(!_active)return;if(_session&&!worldInputBlocked()&&!_loading.visible()&&!_hudEditing){if(!heldTouches_.empty()||_keyboardFire||_keyboardControls.moving()||_keyboardControls.aiming()||std::fabs(_tilt.y)>.03f)lastInteraction_=real;else if(real-lastInteraction_>=120)pauseForInterruption();}if(!_media->begin(_canvasWidth,_canvasHeight)){pauseForInterruption();return;}advanceOriginalFrame();drawLogicalFrame();drawGameButtons();drawKeyboardIndicators();if(!_media->present())pauseForInterruption();}

void Runtime::drawLoading() {
    if(!_loading.visible())return;
    _media->presentationOffset=CGPointZero;
    // Door is a canvas overlay, independent of HUD safe-area scale and clip.
    _media->resetClip();_media->setDrawingOffset(CGPointZero);_media->setDrawingScale(1);_media->setCameraOffset(CGPointZero);
    if(_canvasWidth>480||_canvasHeight>320){
        for(const auto& panel:_loading.expandedPanels(_canvasWidth,_canvasHeight))
            _media->drawExpandedPanel(std::string(panel.sprite.tile.c_str()),panel.sprite.position.x,panel.sprite.position.y,panel.left,panel.right,panel.top,panel.bottom);
    }else for(const auto& sprite:_loading.sprites())
        _media->draw(std::string(sprite.tile.c_str()),sprite.position.x,sprite.position.y,sprite.cols,sprite.rows,sprite.frame,sprite.scale.x,sprite.scale.y,sprite.rotation,sprite.alpha,sprite.blend);
}

void Runtime::drawLogicalFrame() {
    _media->presentationOffset=CGPointZero;

    _media->setCameraOffset(CGPointZero);_media->setDrawingOffset(CGPointZero);_media->setDrawingScale(1);_media->resetClip();_media->resourceScope="shared";

    if(_hudEditing){drawHUDEditor();return;}
    if (_showMenu) {_media->resourceScope=_screenResourceScope;
        if(_canvasHeight>320){CGSize size=_media->imageSize("Extensions/MenuMetal.png");CGFloat scale=std::max(_canvasWidth/size.width,_canvasHeight/size.height);_media->draw("Extensions/MenuMetal.png",_canvasWidth/2,_canvasHeight/2,1,1,0,scale,scale,0,1);}
        _media->presentationOffset=CGPointMake(0,(_canvasHeight-320)/2);_menus->draw();_media->presentationOffset=CGPointZero;_media->resourceScope="shared";this->drawLoading();return;}
    if (_session) {
        const auto& world=_session->world();
        if(world.paused&&world.hasBackground){std::string asset=std::string(world.map.background.c_str());_media->resourceScope="world";CGSize size=_media->imageSize(asset);CGFloat scale=std::max(_canvasWidth/std::max(CGFloat(1),size.width-1),_canvasHeight/std::max(CGFloat(1),size.height-1));_media->draw(asset,_canvasWidth/2,_canvasHeight/2,1,1,0,scale,scale,0,1);}
        _media->setCameraOffset(CGPointMake(_worldShake.offset.x,_worldShake.offset.y));
        for (const auto& item:_session->renderItems()) {
            if (const auto *sprite=std::get_if<heavymach::DrawSprite>(&item)){
                _media->resourceScope=sprite->worldTexture?"world":"shared";
                _media->draw(std::string(sprite->tile.c_str()),sprite->position.x,sprite->position.y,sprite->cols,sprite->rows,sprite->frame,sprite->scale.x,sprite->scale.y,sprite->rotation,sprite->alpha,sprite->blend);}
            else {_media->resourceScope="shared";_media->drawTrail(std::get<heavymach::TrailMesh>(item));}
        }
        _media->setCameraOffset(CGPointZero);_media->resourceScope="shared";
        _media->setDrawingOffset(_hudOffset);_media->setDrawingScale(_hudScale);
        _media->setClipRect(CGRectMake(_hudOffset.x,_hudOffset.y,480*_hudScale,320*_hudScale));
        HMGameHUDSnapshot snapshot;
        snapshot.canvasWidth=_canvasWidth;snapshot.canvasHeight=_canvasHeight;snapshot.hudOffset=_hudOffset;snapshot.hudScale=_hudScale;
        snapshot.slot=&_save.slots[_slotIndex];snapshot.data=&_data;snapshot.passives=&_passives;
        snapshot.hpGauge=_hpGauge;snapshot.expGauge=_expGauge;snapshot.reloadGauge=_reloadGauge;
        snapshot.selectedWeapon=_session->selectedWeapon();
        if(snapshot.selectedWeapon>=0 && snapshot.selectedWeapon<snapshot.slot->weaponCount()) {
            auto name=snapshot.slot->weaponName(snapshot.selectedWeapon);
            auto found=_data.weapons.find(name);if(found!=_data.weapons.end())snapshot.weapon=&found->second;
            snapshot.ammo=_ammoPreview>=0?_ammoPreview:snapshot.slot->weaponCharge(snapshot.selectedWeapon);
        }
        snapshot.presentation=&world.presentation;snapshot.paused=world.paused;snapshot.mode=world.gameMode;
        auto stage=_session->stage();snapshot.mission=stage.mission;snapshot.stage=stage.stage;
        snapshot.menuVisible=_gameMenu;snapshot.time=_time;
        if(auto player=_session->player()) {
            snapshot.upgradeVisible=player->upgradeStep==1;
            snapshot.upgradeSelection=player->upgradeSelection;
        }
        _hud->setLayout(currentHUDLayout(),false);
        _hud->draw(snapshot);
        this->drawAimControls();
        this->drawLoading();_media->resetClip();_media->setDrawingOffset(CGPointZero);_media->setDrawingScale(1);return;
    }
    _media->resourceScope=_screenResourceScope;
    if(_canvasHeight>320){CGSize size=_media->imageSize("Main_BG.png");CGFloat scale=std::max(_canvasWidth/(size.width-1),_canvasHeight/(size.height-1));_media->draw("Main_BG.png",_canvasWidth/2,_canvasHeight/2,1,1,0,scale,scale,0,1);}
    _media->presentationOffset=CGPointMake(0,(_canvasHeight-320)/2);_media->setDrawingOffset(CGPointZero);
    if(_canvasWidth>480) {
        CGSize size=_media->imageSize("Extensions/TitleWide.png");
        // Draw only the two OUTER quarters of the generated candidate. Its center
        // never enters the frame; the original owns that moving rectangle.
        _media->setCameraOffset(CGPointMake(_titleShake.offset.x,_titleShake.offset.y));
        CGFloat extensionScale=320/size.height;
        CGFloat distance=240+size.width/8*extensionScale;
        for(int side:{-1,1}) {
            CGFloat dx=side*distance*_title.bgScale;
            _media->draw("Extensions/TitleWide.png",_canvasWidth/2+_title.camX+std::cos(_title.bgRotate)*dx,160+_title.camY+std::sin(_title.bgRotate)*dx,4,1,side<0?0:3,extensionScale*_title.bgScale,extensionScale*_title.bgScale,_title.bgRotate,1);
        }
        _media->setDrawingOffset(CGPointMake((_canvasWidth-480)/2,0));
        _media->drawEdgeContinuation("Main_BG.png",240+_title.camX,160+_title.camY,_title.bgScale,_title.bgRotate,20);
    }
    _media->setCameraOffset(CGPointMake(_titleShake.offset.x,_titleShake.offset.y));
    if(_canvasWidth>480)_media->drawJoinedBackground("Main_BG.png",240+_title.camX,160+_title.camY,_title.bgScale,_title.bgRotate);
    else _media->draw("Main_BG.png",240+_title.camX,160+_title.camY,1,1,0,_title.bgScale,_title.bgScale,_title.bgRotate,1);
    if(_canvasWidth>480)_media->setDrawingOffset(CGPointMake(CGRectGetMidX(_menuSafeRect)-240,0));
    if (_title.showTitle) _media->draw("Main_Title.png",240,80,1,1,0,_title.titleScale,_title.titleScale,0,1);
    if (_title.showEpisode) _media->draw("Main_Episode.png",240,290,1,1,0,_title.episodeScale,_title.episodeScale,0,1);
    _media->draw("Main_Copyright.png",240,_canvasWidth>480?std::max(CGFloat(12),CGRectGetMinY(_menuSafeRect)+8):12);
    if (_title.showStart) _media->draw("Main_TapToStart.png",240,140);
    _media->setCameraOffset(CGPointZero);
    _media->setDrawingOffset(CGPointZero);
    _media->resourceScope="shared";this->drawLoading();

}

void Runtime::drawAimControls() {
    if((!_session&&!_hudEditing)||(!_hudEditing&&this->worldInputBlocked())||_loading.visible())return;
    _media->resetClip();_media->setDrawingOffset(CGPointZero);_media->setDrawingScale(1);
    auto mode=_hudEditing?heavymach::AimMode::automatic:_session->aimMode();int count=mode==heavymach::AimMode::automatic?4:2;
    for(int action=1;action<count;++action){auto r=this->aimControlRect(action);float x=CGRectGetMidX(r),y=CGRectGetMidY(r);
        bool selected=(action==1&&mode==heavymach::AimMode::automatic);
        for(const auto& held:aimTouches_)if(held.second==action)selected=true;
        std::string  sheet="Extensions/AutoAimControls.png";CGSize size=_media->imageSize(sheet);
        _media->draw(sheet,x,y,3,2,(selected?3:0)+action-1,r.size.width/std::max(CGFloat(1),size.width/3),r.size.height/std::max(CGFloat(1),size.height/2),0,1);
    }
    if(_hudEditing||mode!=heavymach::AimMode::automatic)return;auto aim=_session->aimSnapshot();if(!aim.target){_drawnAimTarget=0;return;}
    if(_drawnAimTarget!=aim.target){_drawnAimTarget=aim.target;_aimAcquiredTime=_time;}
    float gap=4+12*std::max(0.,1-(_time-_aimAcquiredTime)/.15);
    float left=aim.screenPosition.x+aim.bounds.left-gap,right=aim.screenPosition.x+aim.bounds.right+gap;
    float bottom=aim.screenPosition.y+aim.bounds.bottom-gap,top=aim.screenPosition.y+aim.bounds.top+gap;
    float red=aim.status==heavymach::AimStatus::ready?.2f:aim.status==heavymach::AimStatus::blocked?.55f:1;
    float green=aim.status==heavymach::AimStatus::ready?1:.65f,blue=aim.status==heavymach::AimStatus::blocked?.65f:.2f;
    if(right<0||left>_canvasWidth||top<0||bottom>_canvasHeight){
        float cx=(left+right)/2,cy=(bottom+top)/2;
        float x=std::clamp(cx,12.f,float(_canvasWidth-12)),y=std::clamp(cy,12.f,float(_canvasHeight-12));
        float angle=std::atan2(cy-y,cx-x),dx=std::cos(angle),dy=std::sin(angle);
        float lines[]={x-10*dx+6*dy,y-10*dy-6*dx,x,y,x,y,x-10*dx-6*dy,y-10*dy+6*dx};
        _media->lines(lines,4,red,green,blue);
    }else {
        float length=8;float lines[]={left,bottom+length,left,bottom,left,bottom,left+length,bottom,right-length,bottom,right,bottom,right,bottom,right,bottom+length,left,top-length,left,top,left,top,left+length,top,right-length,top,right,top,right,top,right,top-length};
        _media->lines(lines,16,red,green,blue);
        auto badge=heavymach::aimHUDLabel(left,right,top,float(_canvasWidth),float(_canvasHeight),aim.status,float(_time-_aimAcquiredTime));
        float leader[]={badge.anchorX,badge.anchorY,badge.elbowX,badge.elbowY,badge.elbowX,badge.elbowY,badge.endX,badge.elbowY};
        _media->lines(leader,4,red,green,blue);
        auto text=heavymach::aimHUDText(badge);
        // A small dark offset keeps the stencil readable over bright backgrounds.
        auto shadow=text;for(size_t i=0;i<shadow.size;i+=2){shadow.points[i]+=.7f;shadow.points[i+1]-=.7f;}
        _media->lines(shadow.points.data(),int(shadow.size/2),.06f,.08f,.08f);
        float flash=badge.flash?1:0;
        _media->lines(text.points.data(),int(text.size/2),std::max(red,flash),std::max(green,flash),std::max(blue,flash));
    }
}
heavymach::hud::Configuration Runtime::currentHUDLayout()const{auto c=_hudEditing?_hudDraft:_hudLayout;if(!_hudEditing&&!heavymach::hud::valid(c,HM_VALIDATION))c.positions={};return c;}
CGRect Runtime::aimControlRect(int action)const{if(action<1||action>3)return {};int slot=action==1?0:action==3?1:2;auto q=currentHUDLayout().group(heavymach::hud::aim).hitRect({334.+slot*48,10,46,46});return CGRectMake(q.x,q.y,q.width,q.height);}
void Runtime::refreshAutoFire(){if(!_session)return;bool fire=_keyboardFire;for(auto [id,action]:aimTouches_)if(action==2)fire=true;_session->setAutoFire(fire);}
void Runtime::setInputAimMode(heavymach::AimMode mode){if(!_session||_session->aimMode()==mode)return;cancelledTouches_.insert(heldTouches_.begin(),heldTouches_.end());auto keys=_keyboardControls;auto tilt=_tilt;auto fire=keys.bindings[unsigned(heavymach::KeyAction::fire)];blockedKeys_[fire]=blockedKeys_[fire]||keys.keys[fire];keys.keys[fire]=false;cancelGameInput();_keyboardControls=keys;_tilt=tilt;_session->setAimMode(mode);_preferredAimMode=mode;saveSettings();}
bool Runtime::routeAimPoint(CGPoint p,int state,int token){if(!_session)return false;auto found=aimTouches_.find(token);if(found!=aimTouches_.end()){int action=found->second;if(state==1&&action==2&&!contains(aimControlRect(2),p)){aimTouches_.erase(found);cancelledTouches_.insert(token);refreshAutoFire();return true;}if(state>=2){aimTouches_.erase(found);if(action==2)refreshAutoFire();else if(state==2&&!worldInputBlocked()&&contains(aimControlRect(action),p)){if(action==3)_session->cycleAimTarget();else setInputAimMode(_session->aimMode()==heavymach::AimMode::automatic?heavymach::AimMode::manual:heavymach::AimMode::automatic);}}return true;}if(state!=0||worldInputBlocked()||_loading.visible())return false;int count=_session->aimMode()==heavymach::AimMode::automatic?4:2;for(int action=1;action<count;++action)if(contains(aimControlRect(action),p)){aimTouches_[token]=action;if(action==2)refreshAutoFire();return true;}return false;}
void Runtime::touch(int token,int state,float nx,float ny){
 if(!_active||!error.empty())return;CGPoint p{nx*_canvasWidth,(1-ny)*_canvasHeight};bool inside=nx>=0&&nx<=1&&ny>=0&&ny<=1;
 if(routeGameButtons(p,state,token))return;
 if(_keyboardSettingsOpen||panel_!=Panel::none)return;
 if(_hudEditing){hudEditorPoint(p,state,token);return;}_keyboardAimVisible=false;
 if(cancelledTouches_.count(token)){if(state>=2)cancelledTouches_.erase(token);return;}
 if(state>=2)heldTouches_.erase(token);if(inside){lastInteraction_=realTime_;if(state==0)heldTouches_.insert(token);}if(routeAimPoint(p,state,token))return;
 if(_session){CGPoint hp{(p.x-_hudOffset.x)/_hudScale,(p.y-_hudOffset.y)/_hudScale};bool insideHUD=_hudLayout.customized()||(hp.x>=0&&hp.x<=480&&hp.y>=0&&hp.y<=320);
  if(state==0){int receiver=insideHUD?_hud->receiverAt(hp):-1;if(insideHUD&&_hud->touch(hp,0)){hudTouches_[token]=_hudLayout.customized()?receiver:-1;}else if(inside&&!worldInputBlocked()){worldTouches_.insert(token);_session->touch({float(p.x),float(p.y)},0);_keyboardAim=p;_hasKeyboardAim=true;}}
  else if(hudTouches_.count(token)){int receiver=hudTouches_.at(token);if(state>=2)hudTouches_.erase(token);_hud->touch(insideHUD?hp:CGPoint{-1000,-1000},state,receiver);}
  else if(worldTouches_.count(token)){if(state>=2){worldTouches_.erase(token);if(worldTouches_.empty()){if(state==2&&inside)_session->touch({float(p.x),float(p.y)},2);else if(_session->aimMode()==heavymach::AimMode::manual)_session->releaseFire();}}else if(inside&&!worldInputBlocked()){_session->touch({float(p.x),float(p.y)},state);_keyboardAim=p;_hasKeyboardAim=true;}}
  consumeGameEvents();return;
 }
 if(!inside){if(_showMenu&&state>=2)_menus->touch({-1000,-1000},3);return;}if(_showMenu){_menus->touch({p.x,p.y-(_canvasHeight-320)/2},state);return;}if(state==2&&_title.released(_time))requestScreen(3);
}
void Runtime::ensureKeyboardAim(){_keyboardAimVisible=true;if(_hasKeyboardAim||!_session)return;auto*p=_session->player();if(!p)return;heavymach::Vector2 q{p->position.x-_session->world().camera.x,p->position.y-_session->world().camera.y};_keyboardAim={std::min(float(_canvasWidth-8),q.x+float(_canvasWidth/2)+160),std::clamp(q.y+180,8.f,312.f)};_hasKeyboardAim=true;}
void Runtime::updateKeyboardAim(){if(worldInputBlocked()||_keyboardSettingsOpen||_session->aimMode()!=heavymach::AimMode::manual)return;if(_keyboardControls.aiming()){ensureKeyboardAim();float x=_keyboardAim.x,y=_keyboardAim.y;_keyboardControls.aim(x,y,_canvasWidth,_canvasHeight);_keyboardAim={x,y};lastInteraction_=realTime_;if(_keyboardFire)_session->touch({x,y},1);}}
bool Runtime::key(unsigned code,bool pressed){
 using A=heavymach::KeyAction;if(!_active)return false;
 if(panel_!=Panel::none){if(code==0x29&&pressed){if(panel_==Panel::confirmTransfer)panel_=Panel::transfer;else closeGamePanel();gameButtons_.cancel();return true;}if(auto event=gameButtons_.key(code,pressed))gameUIAction(event->id,event->state);return true;}
 if(code==0x3c&&transferAvailable()){if(pressed)gameUIAction(22000,2);return true;}
 if(!_keyboardSettingsOpen&&code==0x3f){if(pressed){cancelGameInput();gameToolsFocus_=!gameToolsFocus_;}return true;}
 if(gameToolsFocus_&&!_keyboardSettingsOpen){if(code==0x29&&pressed){gameToolsFocus_=false;gameButtons_.cancel();return true;}if(auto event=gameButtons_.key(code,pressed))gameUIAction(event->id,event->state);return true;}
if(code<blockedKeys_.size()&&blockedKeys_[code]){if(!pressed)blockedKeys_[code]=false;return true;}
 if(_hudEditing){if(!pressed)return true;if(code==0x29){closeHUDEditor();return true;}if(code==0x2b){do{_hudSelected=(_hudSelected+1)%heavymach::hud::groupCount;}while(!HM_VALIDATION&&_hudSelected==heavymach::hud::cheat);_hudHideGuides=false;return true;}auto c=currentHUDLayout();auto r=c.group(_hudSelected).visualBounds;double dx=code==0x4f?1:code==0x50?-1:0,dy=code==0x52?1:code==0x51?-1:0;if(dx||dy)_hudDraft.positions[_hudSelected]=heavymach::hud::placementForOrigin(c.localBounds(_hudSelected),c.legacy,c.safe,{r.x+dx,r.y+dy});return true;}
 if(_keyboardSettingsOpen){if(!_keyboardControls.set(code,pressed)||!pressed)return true;if(_capturingKey){if(code!=0x29&&_keyboardControls.remap(A(_settingsSelection),code))saveSettings();_capturingKey=false;}else if(code==0x29){command("keys");return true;}else if(code==0x52)_settingsSelection=(_settingsSelection+heavymach::KeyboardInput::count-1)%heavymach::KeyboardInput::count;else if(code==0x51||code==0x2b)_settingsSelection=(_settingsSelection+1)%heavymach::KeyboardInput::count;else if(code==0x28)_capturingKey=true;else if(code==0x2a){_keyboardControls.bindings=heavymach::KeyboardInput::defaults;saveSettings();}return true;}
 if(!_keyboardControls.handles(code))return false;if(!_keyboardControls.set(code,pressed))return true;lastInteraction_=realTime_;
 if((_keyboardControls.is(code,A::left)||_keyboardControls.is(code,A::right))&&!_keyboardControls.moving())_tilt={};
 if(pressed&&_keyboardControls.is(code,A::settings)){command("keys");return true;}if(_loading.visible())return true;
 if(!pressed&&_keyboardControls.is(code,A::fire)){_keyboardFire=false;if(_session&&_session->aimMode()==heavymach::AimMode::automatic)refreshAutoFire();else if(_session&&worldTouches_.empty())_session->releaseFire();return true;}
 if(pressed&&_keyboardControls.is(code,A::back)){if(_session){if(_gameMenu)gameButton(101,2);else if(!worldInputBlocked())gameButton(100,2);}else if(_showMenu)requestScreen(3);return true;}
 if(_showMenu||(_session&&worldInputBlocked())){if(!pressed)return true;int delta=0;if(_keyboardControls.is(code,A::focus))delta=_keyboardControls.fine()?-1:1;else if(_keyboardControls.is(code,A::aimLeft)||_keyboardControls.is(code,A::aimUp))delta=-1;else if(_keyboardControls.is(code,A::aimRight)||_keyboardControls.is(code,A::aimDown))delta=1;if(delta){if(_showMenu)_menus->moveKeyboardFocus(delta);else _hud->moveKeyboardFocus(delta);}if(_keyboardControls.is(code,A::confirm)){if(_showMenu)_menus->activateKeyboardFocus();else _hud->activateKeyboardFocus();}return true;}
 if(_session){if(pressed&&_keyboardControls.is(code,A::fire)){_keyboardFire=true;if(_session->aimMode()==heavymach::AimMode::manual){ensureKeyboardAim();_session->touch({float(_keyboardAim.x),float(_keyboardAim.y)},0);}else refreshAutoFire();}if(pressed&&_keyboardControls.is(code,A::nextTarget))_session->cycleAimTarget();if(pressed&&_keyboardControls.is(code,A::jump))gameButton(1000,0);if(pressed&&(_keyboardControls.is(code,A::nextWeapon)||_keyboardControls.is(code,A::previousWeapon))){_session->cycleWeapon(_keyboardControls.is(code,A::previousWeapon)?-1:1);_media->sound("Game_ChangeWeapon");}consumeGameEvents();}else if(pressed&&_keyboardControls.is(code,A::confirm)&&_title.released(_time))requestScreen(3);return true;
}
void Runtime::drawKeyboardIndicators(){if(_loading.visible()||_hudEditing||_keyboardSettingsOpen||panel_!=Panel::none)return;CGRect r{};if(_showMenu)r=CGRectOffset(_menus->keyboardFocusRect(),0,(_canvasHeight-320)/2);else if(_session&&worldInputBlocked()){r=_hud->keyboardFocusRect();r=CGRectMake(r.origin.x*_hudScale+_hudOffset.x,r.origin.y*_hudScale+_hudOffset.y,r.size.width*_hudScale,r.size.height*_hudScale);}else if(_hasKeyboardAim&&_keyboardAimVisible&&_session&&_session->aimMode()==heavymach::AimMode::manual)r=CGRectMake(_keyboardAim.x-4,_keyboardAim.y-4,8,8);if(CGRectIsEmpty(r))return;float x=r.origin.x,y=r.origin.y,R=CGRectGetMaxX(r),T=CGRectGetMaxY(r);float p[]={x,y,R,y,R,y,R,T,R,T,x,T,x,T,x,y};_media->resetClip();_media->lines(p,8,1,1,0);}
void Runtime::openHUDEditor(){if(_loading.state()<6||_hudEditing||_showMenu||!_session||_session->world().gameState!=0)return;cancelGameInput();_gameMenu=true;_hud->setMenuVisible(true);_session->setPaused(true);_hudDraft=_hudLayout;_hudEditing=true;_hudExample=false;_hudHideGuides=false;dragToken_=-1;}
void Runtime::closeHUDEditor(){_hudEditing=false;gameToolsFocus_=false;dragToken_=-1;exampleSession_.reset();exampleData_={};_media->releaseScope("hud-example");cancelGameInput();lastInteraction_=realTime_;updateCanvasGeometry();}
void Runtime::hudEditorPoint(CGPoint p,int state,int token){auto c=currentHUDLayout();if(state==0&&dragToken_<0){int hit=-1;for(int j=heavymach::hud::groupCount-1;j>=0;--j){int i=j==0?heavymach::hud::upperTrim:j-1;if(!HM_VALIDATION&&i==heavymach::hud::cheat)continue;auto r=c.group(i).visualBounds;if(r.contains({p.x,p.y},heavymach::hud::HitEdges::standard)){hit=i;_hudGrab={p.x-r.x,p.y-r.y};break;}}if(hit<0)return;_hudHideGuides=true;_hudSelected=hit;dragToken_=token;}if(token!=dragToken_)return;if(state<3&&p.x>=0&&p.y>=0){auto r=c.group(_hudSelected).visualBounds;double x=p.x-_hudGrab.x,y=p.y-_hudGrab.y;if(std::abs(x-r.x)>.01||std::abs(y-r.y)>.01){auto original=c.legacy.apply(c.localBounds(_hudSelected));if(std::abs(x-original.x)<.01&&std::abs(y-original.y)<.01)_hudDraft.positions[_hudSelected].reset();else _hudDraft.positions[_hudSelected]=heavymach::hud::placementForOrigin(c.localBounds(_hudSelected),c.legacy,c.safe,{x,y});}}if(state>=2)dragToken_=-1;}
void Runtime::prepareHUDExample(){if(exampleSession_)return;exampleData_=_data;exampleSlot_={};heavymach::GlobalInfo(exampleSlot_).selectSlot(1);exampleRandom_=12345;auto& map=exampleData_.maps.at("Stage01");map.enemies={{"Tank001",{map.left+240,100},false},{"Heli001",{map.left+330,190},false}};exampleSession_=std::make_unique<heavymach::GameSession>(exampleData_,exampleSlot_,[this](std::string_view n){auto z=_media->imageSize(std::string(n),"hud-example");return heavymach::Vector2{float(z.width),float(z.height)};},[this]{auto x=exampleRandom_;x^=x<<13;x^=x>>17;x^=x<<5;exampleRandom_=x;return int32_t(x&0x7fffffff);},&examplePassives_);exampleSession_->setCanvasWidth(_canvasWidth);exampleSession_->setCanvasHeight(_canvasHeight);exampleSession_->start(0,0,0);for(int i=1;i<=45;++i){exampleSession_->setPaused(false);exampleSession_->tick(i/60.,0);exampleSession_->takeEvents();}exampleSession_->prepareHUDPreview();}
void Runtime::drawHUDEditor(){if(_hudExample)prepareHUDExample();auto* scene=_hudExample?exampleSession_.get():_session.get();if(!scene)return;_media->setDrawingOffset({});_media->setDrawingScale(1);_media->resetClip();const auto& world=scene->world();if(world.hasBackground){_media->resourceScope=_hudExample?"hud-example":"world";auto size=_media->imageSize(world.map.background);double scale=std::max(_canvasWidth/std::max(1.,size.width-1),_canvasHeight/std::max(1.,size.height-1));_media->setCameraOffset({});_media->draw(world.map.background,_canvasWidth/2,_canvasHeight/2,1,1,0,scale,scale);}if(!_hudExample)_media->setCameraOffset({_worldShake.offset.x,_worldShake.offset.y});for(const auto& item:scene->renderItems()){if(auto*s=std::get_if<heavymach::DrawSprite>(&item)){_media->resourceScope=_hudExample?"hud-example":s->worldTexture?"world":"shared";_media->draw(s->tile,s->position.x,s->position.y,s->cols,s->rows,s->frame,s->scale.x,s->scale.y,s->rotation,s->alpha,s->blend);}else{_media->resourceScope=_hudExample?"hud-example":"shared";_media->drawTrail(std::get<heavymach::TrailMesh>(item));}}_media->resourceScope="shared";_media->setCameraOffset({});_media->setDrawingOffset(_hudOffset);_media->setDrawingScale(_hudScale);_media->setClipRect({_hudOffset,{480*_hudScale,320*_hudScale}});
 HMGameHUDSnapshot s;s.slot=_hudExample?&exampleSlot_:&_save.slots[_slotIndex];s.data=&_data;s.passives=_hudExample?&examplePassives_:&_passives;s.time=_time;s.ammo=99;s.expGauge=_expGauge;s.reloadGauge=_reloadGauge;s.canvasWidth=_canvasWidth;s.canvasHeight=_canvasHeight;s.hudOffset=_hudOffset;s.hudScale=_hudScale;s.selectedWeapon=scene->selectedWeapon();if(s.slot->weaponCount()>0){auto w=_data.weapons.find(s.slot->weaponName(s.selectedWeapon));if(w!=_data.weapons.end())s.weapon=&w->second;s.ammo=s.slot->weaponCharge(s.selectedWeapon);}_hud->setLayout(currentHUDLayout(),true);_hud->draw(s);drawAimControls();_media->resetClip();_media->setDrawingOffset({});_media->setDrawingScale(1);
}
static std::string escape(const std::string& s){std::string out;for(unsigned char c:s){if(c=='"'||c=='\\'){out+='\\';out+=char(c);}else if(c=='\n')out+="\\n";else if(c>=32)out+=char(c);}return out;}
std::string Runtime::ui(){
 static const char* groups[]={"MENU","LEVEL / EXP","JUMP","WEAPON / AMMO","GAUGES","AIM CONTROLS","PASSIVES","WEAPON PREVIEW","CHEAT CONTROLS","UPPER TRIM"};
 auto textRect=recoveryTextRect();
 std::ostringstream s;s<<"{\"textEntry\":"<<((panel_==Panel::transfer)?"true":"false")<<",\"textRect\":["<<textRect.x/_canvasWidth<<','<<1-(textRect.y+textRect.height)/_canvasHeight<<','<<textRect.width/_canvasWidth<<','<<textRect.height/_canvasHeight<<"],\"textRevision\":"<<textRevision_<<",\"textValue\":\""<<escape(recoveryCode_)<<"\",\"clipboardRevision\":"<<clipboardRevision_<<",\"clipboardValue\":\""<<escape(clipboardValue_)<<"\",\"transferAvailable\":"<<(transferAvailable()?"true":"false")<<",\"editor\":"<<(_hudEditing?"true":"false")<<",\"tools\":"<<(!_hudHideGuides?"true":"false")<<",\"sample\":"<<(_hudExample?"true":"false")<<",\"valid\":"<<(heavymach::hud::valid(currentHUDLayout(),HM_VALIDATION)?"true":"false")<<",\"selection\":\""<<groups[_hudSelected]<<"\",\"game\":"<<(_session?"true":"false")<<",\"keys\":"<<(_keyboardSettingsOpen?"true":"false")<<",\"capture\":"<<(_capturingKey?"true":"false")<<",\"keySelection\":"<<_settingsSelection<<",\"bindings\":[";
 for(unsigned i=0;i<heavymach::KeyboardInput::count;++i){if(i)s<<',';s<<_keyboardControls.bindings[i];}s<<"],\"url\":\""<<escape(pendingURL)<<"\",\"error\":\""<<escape(error)<<"\",\"notice\":\""<<escape(notice)<<"\",\"fallback\":"<<(!_hudEditing&&!heavymach::hud::valid(_hudLayout,HM_VALIDATION)?"true":"false")<<",\"cheatRect\":[";
 auto r=currentHUDLayout().group(heavymach::hud::cheat).visualBounds;s<<r.x/_canvasWidth<<','<<1-(r.y+r.height)/_canvasHeight<<','<<r.width/_canvasWidth<<','<<r.height/_canvasHeight<<"]}";pendingURL.clear();return s.str();
}
void Runtime::command(const std::string& name,int a,int b,int c,int d){
#if HM_VALIDATION
 if(name=="recreateContext"){pauseForInterruption();_media->recreateContext();return;}
 if(name=="dump"){atomicText(files_+"/diagnostics.json",diagnostics());return;}
#endif
 if(name=="focus"){if(!a)pauseForInterruption();_media->audio->active(a&&_active);return;}if(name=="interrupt"){pauseForInterruption();return;}
 if(name=="editor"){openHUDEditor();return;}if(name=="keys"){if(!_keyboardSettingsOpen&&_loading.state()<6)return;if(!_keyboardSettingsOpen){settingsWasPaused_=_session&&_session->world().paused;cancelGameInput();if(_session)_session->setPaused(true);}else{if(_session)_session->setPaused(settingsWasPaused_||_resumeRequired);cancelGameInput();}_keyboardSettingsOpen=!_keyboardSettingsOpen;_capturingKey=false;return;}
 if(name=="keySelect"&&_keyboardSettingsOpen){_settingsSelection=std::clamp(a,0,int(heavymach::KeyboardInput::count)-1);_capturingKey=true;return;}
 if(name=="keysReset"){_keyboardControls.bindings=heavymach::KeyboardInput::defaults;saveSettings();return;}
 if(_hudEditing){if(name=="reset"){_hudDraft.positions={};dragToken_=-1;}else if(name=="cancel")closeHUDEditor();else if(name=="apply"){if(heavymach::hud::valid(currentHUDLayout(),HM_VALIDATION)){if(!_hudDraft.samePositions(_hudLayout))saveHUD();closeHUDEditor();}}else if(name=="preview")_hudHideGuides=true;else if(name=="tools")_hudHideGuides=!_hudHideGuides;else if(name=="sample")_hudExample=!_hudExample;return;}
#if HM_VALIDATION
 if(name=="stage"){heavymach::ValidationScenario f;f.mission=a;f.stage=b;f.internalLevel=std::clamp(c,0,99);f.allWeapons=true;f.selectedWeapon=std::max(0,d);f.grades=fixtureGrades_;f.nearLevelUp=fixtureNearLevel_;if(fixtureBoss_>=0)f.bossIndex=size_t(fixtureBoss_);heavymach::SaveSlot candidate;heavymach::prepareValidationSlot(candidate,_data,f);fixture_=f;_resumeRequired=false;_loading={};_hud->setMenuVisible(false);startSlot(0,0,a,b);persistSave();}
 else if(name=="fixtureSettings"){fixtureGrades_={uint8_t(std::clamp(a,0,6)),uint8_t(std::clamp(b,0,6)),uint8_t(std::clamp(c,0,6))};fixtureNearLevel_=bool(d&1);fixtureBoss_=(d>>1)-1;}
 else if(name=="saved"){if(!_save.slots[0].occupied()){notice="No saved game.";return;}_resumeRequired=false;_loading={};_hud->setMenuVisible(false);startSlot(0,0,_save.slots[0].i32(0xc),_save.slots[0].i32(0x10));}
 else if(name=="cheats"){cheats_=a!=0;if(_session)_session->validationSetCheats(cheats_);}
 else if(name=="weapon"&&_session){_session->selectWeapon(a);consumeGameEvents();}
 else if(name=="exp"&&_session){_session->validationAddExperience(a);consumeGameEvents();}
 else if(name=="wide"){wide_=a!=0;updateCanvasGeometry();}

 else if(name=="aim"&&_session)setInputAimMode(a?heavymach::AimMode::automatic:heavymach::AimMode::manual);
 else if(name=="pause")pauseForInterruption();
 else if(name=="resume")gameButton(101,2);
 else if(name=="key")key(unsigned(a),b!=0);
 else if(name=="jump")gameButton(1000,0);
 else if(name=="fire"&&_session){_keyboardFire=a!=0;if(_session->aimMode()==heavymach::AimMode::automatic)refreshAutoFire();else if(a){ensureKeyboardAim();_session->touch({float(_keyboardAim.x),float(_keyboardAim.y)},0);}else _session->releaseFire();}
#else
 (void)a;(void)b;(void)c;(void)d;
#endif
}
std::string Runtime::diagnostics(){std::ostringstream s;s<<"{\"frames\":"<<_media->frames<<",\"drawCalls\":"<<_media->_drawCalls<<",\"uploads\":"<<_media->uploads<<",\"contexts\":"<<_media->contextCreations<<",\"editor\":"<<(_hudEditing?"true":"false")<<",\"time\":"<<_time<<",\"canvas\":["<<_canvasWidth<<','<<_canvasHeight<<"],\"saveWrites\":"<<saveWrites_<<",\"hudWrites\":"<<hudWrites_<<",\"voices\":"<<_media->audio->voices()<<",\"audioStarts\":"<<_media->audio->starts()<<",\"audioStops\":"<<_media->audio->stops()<<",\"audioActive\":"<<(_media->audio->isActive()?"true":"false")<<",\"active\":"<<(_active?"true":"false")<<",\"error\":\""<<escape(error)<<'"';if(_session){auto*p=_session->player();auto stage=_session->stage();const auto& slot=_save.slots.at(_slotIndex);s<<",\"level\":"<<slot.level()<<",\"experience\":"<<slot.experience()<<",\"upgradeStep\":"<<(p?p->upgradeStep:0)<<",\"gameMenu\":"<<(_gameMenu?"true":"false")<<",\"grades\":["<<int(slot.bytes[0x2f])<<','<<int(slot.bytes[0x2e])<<','<<int(slot.bytes[0x2d])<<']';s<<",\"worldTime\":"<<_session->world().gameTime<<",\"mission\":"<<stage.mission<<",\"stage\":"<<stage.stage<<",\"paused\":"<<(_session->world().paused?"true":"false")<<",\"x\":"<<(p?p->position.x:0)<<",\"y\":"<<(p?p->position.y:0)<<",\"hp\":"<<(p?p->hp:0)<<",\"enemies\":"<<_session->enemyCount()<<",\"bullets\":"<<_session->bulletCount()<<",\"weapon\":"<<_session->selectedWeapon()<<",\"aim\":"<<int(_session->aimMode())<<",\"aimTarget\":"<<_session->aimSnapshot().target<<",\"aimStatus\":"<<int(_session->aimSnapshot().status)<<",\"assistedShots\":"<<_session->aimSnapshot().assistedShots<<",\"targetHits\":"<<_session->aimSnapshot().targetHits;
#if HM_VALIDATION
 s<<",\"shots\":"<<observedShots_;
#endif
 }s<<",\"stageFields\":[";for(size_t i=0;i<stageFields_.size();++i){if(i)s<<',';s<<stageFields_[i];}s<<"],\"bindings\":[";for(size_t i=0;i<_keyboardControls.bindings.size();++i){if(i)s<<',';s<<_keyboardControls.bindings[i];}s<<"],\"screen\":"<<transferScreen_<<",\"showMenu\":"<<(_showMenu?"true":"false")<<",\"captureKey\":"<<(_capturingKey?"true":"false")<<",\"keySelection\":"<<_settingsSelection<<",\"recoveryCodeBytes\":"<<recoveryCode_.size()<<",\"panel\":"<<int(panel_)<<",\"textRevision\":"<<textRevision_<<",\"uiStatus\":\""<<escape(uiStatus_)<<"\",\"buttons\":[";bool first=true;for(const auto& button:gameButtons_.items()){if(!first)s<<',';first=false;const auto&r=button.bounds;s<<"{\"id\":"<<button.id<<",\"label\":\""<<escape(button.label)<<"\",\"rect\":["<<r.x<<','<<r.y<<','<<r.width<<','<<r.height<<"],\"enabled\":"<<(button.enabled?"true":"false")<<'}';}s<<"],\"originalButtons\":[";first=true;auto emit=[&](int id,CGRect r){if(!first)s<<',';first=false;s<<"{\"id\":"<<id<<",\"rect\":["<<r.origin.x<<','<<r.origin.y<<','<<r.size.width<<','<<r.size.height<<"]}";};if(_showMenu){for(auto& button:_menus->_buttons)emit(button.identifier,CGRectOffset(button.bounds,0,(_canvasHeight-320)/2));}else if(_session){for(auto&button:_hud->keyboardButtons()){auto r=button.bounds;emit(button.identifier,CGRectMake(_hudOffset.x+r.origin.x*_hudScale,_hudOffset.y+r.origin.y*_hudScale,r.size.width*_hudScale,r.size.height*_hudScale));}}s<<"]}";return s.str();}
#include "Transfer.inc"
#include "GameButtons.inc"

}
