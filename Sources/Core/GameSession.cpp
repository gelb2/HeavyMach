#include "GameSession.hpp"
#include "BulletVisual.hpp"
#include "CameraShake.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <list>
#include <numbers>
#include <set>
#include <variant>
#include <type_traits>
namespace heavymach {
struct GameSession::Impl final: WorldHost,CombatHost,PlayerHost,EnemyHost,ItemBoxHost,TrailHost,BulletVisualHost {
 struct Part {PartsState visual;WeaponState weapon;bool armed=false;int order=0;ObjectBounds bounds;};
 struct Machine {
  uintptr_t id=0;size_t mapEnemyIndex=SIZE_MAX;bool bossActivationPending=false;std::unique_ptr<GobHvM_Enemy> enemy;
  std::vector<std::unique_ptr<Part>> parts;std::vector<Part*> drawOrder;bool drawOrderDirty=true;ObjectBounds bounds{};bool removed=false,inScreen=true;
  const MachineBehavior* behavior=nullptr;float attitude=0;uint32_t privateRandom=1;
  bool formationEntering=false;float formationEndX=0;float entrySpeed=-2.5f,releaseSpeed=-.5f;
  Vector2 aimStep{};
  float gauge=1;std::array<bool,2> gaugeQueued{};const MachDefinition* definition=nullptr;
#if defined(HM_VALIDATION) && HM_VALIDATION
  Vector2 validationSpawnPosition{};
#endif
 };
 struct Dynamic {uintptr_t aimTarget=0;std::variant<BulletState,GobItemBox,std::shared_ptr<GobTrailEffect>> value;std::shared_ptr<GobTrailEffect> trailForBullet;explicit Dynamic(BulletState b):value(b){}explicit Dynamic(GobItemBox b):value(b){}explicit Dynamic(std::shared_ptr<GobTrailEffect> t):value(std::move(t)){}};
 const GameData& data;SaveSlot& slot;TileSize sizes,worldSizes;Random31 random;
 PlayerPassives ownedPassives;PlayerPassives& passives;
 WorldState world;StagePosition stage{};std::unique_ptr<GobHvM_Player> player;
 Vector2 cameraOffset{};
 Machine playerGraph;std::list<Machine> enemies;std::list<Dynamic> dynamic;
 std::vector<DrawSprite> mapSprites,draws;std::vector<SessionRenderItem> render;std::shared_ptr<GobTrailEffect> pendingTrail;std::vector<SessionEvent> events;
 std::set<std::string> unsupported;std::array<bool,64> particleQueued{};
 AimMode aimMode=AimMode::manual;uintptr_t aimTarget=0,excludedTarget=0;bool autoFire=false;
 AimSnapshot aimState{};Dynamic* assistedBullet=nullptr;
 Machine* current=nullptr;const MachDefinition* building=nullptr;ParticleState* acquired=nullptr;
 const StageFormation* reinforcement=nullptr;bool reinforcementSpawned=false;
 int selected=0,charge=0;bool replaceWeapon=false,active=false;double globalTime=0;
#if defined(HM_VALIDATION) && HM_VALIDATION
 bool validationCheats=false;
 GameSession::ValidationCheatStats cheatStats{};
 GameSession::ValidationShotObserver shotObserver;
 GameSession::ValidationDamageObserver damageObserver;
#endif
 float safeBackLine=-32000,removeBgLine=-32000;uintptr_t nextID=2;
 Impl(const GameData& d,SaveSlot& s,TileSize t,Random31 r,PlayerPassives* shared,TileSize worldT):data(d),slot(s),sizes(std::move(t)),worldSizes(std::move(worldT)),random(std::move(r)),passives(shared?*shared:ownedPassives){if(!sizes||!random)throw std::invalid_argument("Session requires tile dimensions and original RNG");playerGraph.id=1;}
 int32_t random31()override{if(current&&current->behavior&&current->behavior->independentRandom){current->privateRandom=current->privateRandom*1664525u+1013904223u;return int32_t(current->privateRandom>>1);}
  int32_t n=random();if(n<0)throw std::invalid_argument("Negative random31");return n;}
 float rnd(){return float(random31())*0x1p-31f;}
 Vector2 tileSize(std::string_view key)override{return sizes(key);}
 Vector2 mapTileSize(std::string_view key)override{return worldSizes?worldSizes(key):sizes(key);}
 std::optional<Vector2> cameraTarget(std::optional<Vector2>)const override{return player?std::optional<Vector2>(player->position):std::nullopt;}
 Vector2 screen(Vector2 p)const{return {p.x-world.camera.x+world.canvasWidth*.5f,p.y-world.camera.y+160};}
 Vector2 cameraPoint(Vector2 p,bool fixed=false)const{return cameraScreenPosition(p,world.camera,{world.canvasWidth*.5f,160},&cameraOffset,fixed);}
 bool visibleOnCanvas(Vector2 pos,ObjectBounds b)const{auto p=cameraPoint(pos);return !(p.x+b.right<0||p.x+b.left>world.canvasWidth+60||p.y+b.top<0||p.y+b.bottom>world.canvasHeight);}
 bool inView(Vector2 pos,ObjectBounds b)const{auto p=cameraPoint(pos);return !(p.x+b.right<0||p.x+b.left>world.canvasWidth+60||p.y+b.top<0||p.y+b.bottom>320);}
 void unsupportedOnce(std::string text){if(unsupported.insert(text).second){SessionEvent e{SessionEventKind::Unsupported};e.text=std::move(text);events.push_back(std::move(e));}}
 void audio(int n,float pan=0){SessionEvent e{SessionEventKind::AudioIndex};e.index=n;e.pan=pan;events.push_back(e);}
 void audio(std::string_view name,float pan=0){SessionEvent e{SessionEventKind::AudioName};e.text=name;e.pan=pan;events.push_back(std::move(e));}
 void changed(){events.push_back({SessionEventKind::SaveChanged});}
 void persist(){SessionEvent e{SessionEventKind::PersistSave};e.savedSlot=std::make_shared<const SaveSlot>(slot);events.push_back(std::move(e));}
 static Vector2 socket(const PartsInfo* p,size_t index){return p&&index<p->sockets.size()?p->sockets[index]:Vector2{};}
 void atlas(DrawSprite& d)const{
  if(d.tile=="Bullet.png"){d.cols=4;d.rows=8;}
  else if(d.tile.starts_with("EnemyBullet0")){d.cols=4;d.rows=4;}
  else if(d.tile.starts_with("EnemyMissile")){d.cols=2;d.rows=4;}
  else if(d.tile=="Explode.png"||d.tile=="Efx_Explode03.png"||d.tile=="Efx_ExplodeNPC.png"){d.cols=4;d.rows=4;}
  else if(d.tile=="Efx_Explode01.png"){d.cols=4;d.rows=5;}
  else if(d.tile=="Efx_Explode02.png"){d.cols=6;d.rows=4;}
  else if(d.tile=="Efx_Damage.png"){d.cols=6;d.rows=1;}
  else if(d.tile=="ItemTile.png"){d.cols=7;d.rows=4;}
  else if(d.tile=="WeaponTile.png"){d.cols=7;d.rows=4;}
 }
 Part* addPart(Machine& graph,const PartsInfo& info,Part* base,uint8_t baseSocket,int order,uint8_t machType){
  auto owned=std::make_unique<Part>();Part* part=owned.get();part->visual.info=&info;part->order=order;
  // 1.13 tileCnt0 resolves absent PartsDummy.png to nil; keep sockets and zero tile dimensions.
  auto size=info.tileCnt?tileSize(info.presentation?info.presentation->frames.front().tile:info.tileName()):Vector2{};float columns=info.tileCnt?info.tileCnt:1;part->bounds=partsBounds(size.x/columns,size.y,socket(&info,0));
  if(info.presentation&&info.presentation->logicalSize.x>0){auto logical=info.presentation->logicalSize;part->bounds=partsBounds(logical.x,logical.y,socket(&info,0));}
  part->armed=info.partsType==2;
  if(part->armed){initializeWeapon(part->weapon,world.gameTime,*this);auto w=data.weapons.find(info.name);if(w==data.weapons.end())w=data.weapons.find("E_Default_Wpn");
   if(w==data.weapons.end())throw std::runtime_error("Missing weapon definition: "+info.name);part->weapon.weaponInfo=&w->second;if(w->second.aiTimeScale!=1)part->weapon.delayTime*=w->second.aiTimeScale;
   if(machType!=3){setWeaponAim(part->weapon,float(std::numbers::pi));part->weapon.aimDir=part->weapon.aimDirDest-.01f;}}
  graph.parts.push_back(std::move(owned));graph.drawOrderDirty=true;
  if(base){attachParts(part->visual,&base->visual,baseSocket);base->visual.offset=partsSocketPosition(base->visual,0);base->visual.rotatedPosition=base->visual.offset;}
  if(&graph!=&playerGraph)accumulateMachBounds(graph.bounds,base?base->visual.rotatedPosition:Vector2{},base?base->bounds:ObjectBounds{});
  return part;
 }
 void enemyParts(Machine& graph,const std::vector<MachPartDefinition>& definitions,Part* base,uint8_t type){
  for(const auto& def:definitions){auto found=data.parts.find(def.name);Part* part=nullptr;
   if(found!=data.parts.end())part=addPart(graph,found->second,base,def.baseSocket,def.zOrder,type);
   else unsupportedOnce("Missing original enemy part: "+def.name);
   enemyParts(graph,def.children,part,type);}
 }
 void playerPart(int index,const std::string& name){
  if(!player)return;auto it=data.parts.find(name);if(it==data.parts.end())throw std::runtime_error("Missing player part: "+name);
  Part* added=addPart(playerGraph,it->second,nullptr,0,index,3);
  auto replacement=std::move(playerGraph.parts.back());playerGraph.parts.pop_back();
  if(playerGraph.parts.size()<3)playerGraph.parts.resize(3);
  playerGraph.parts[size_t(index)]=std::move(replacement);
  auto body=playerGraph.parts[0].get(),top=playerGraph.parts[1].get(),weapon=playerGraph.parts[2].get();
  if(body&&top){attachParts(top->visual,&body->visual,1);body->visual.offset=partsSocketPosition(body->visual,0);body->visual.rotatedPosition=body->visual.offset;}
  if(top&&weapon){attachParts(weapon->visual,&top->visual,1);top->visual.offset=partsSocketPosition(top->visual,0);top->visual.rotatedPosition=top->visual.offset;}
  playerGraph.bounds=playerBounds();player->rotation-=.01f;
  if(weapon)setWeaponAim(weapon->weapon,.1f);player->hasWeapon=weapon!=nullptr;(void)added;
 }
 std::string gradeName(int part)const{char name[40];const char* patterns[]={"P_Body_%02d","P_Top_%02d","P_Wpn_Gun%02d"};const size_t offsets[]={0x2f,0x2e,0x2d};std::snprintf(name,sizeof(name),patterns[part],int(slot.bytes[offsets[part]])+1);return name;}
 void refreshInventoryCharge(){
  // GuiGame::changeWeapon0x178f0 updates the live selected inventory record.
  if(selected>=0&&selected<slot.weaponCount()){
   if(selected==0)slot.setInt(0x4c,99);
   charge=slot.weaponCharge(size_t(selected));
  }
 }
 void select(int index){GlobalInfo info(slot);auto result=info.selectWeapon(index);if(!result)return;selected=int(*result);playerPart(2,selected==0?gradeName(2):slot.weaponName(size_t(selected)));refreshInventoryCharge();SessionEvent e{SessionEventKind::PlayerUI};e.code=int(PlayerEventKind::WeaponInventory);e.index=selected;events.push_back(e);}
 void spawnPlayer(Vector2 p)override{
  player=std::make_unique<GobHvM_Player>(slot,&passives);playerGraph.parts.clear();playerGraph.removed=false;playerGraph.inScreen=true;playerGraph.gaugeQueued.fill(false);
  playerPart(0,gradeName(0));playerPart(1,gradeName(1));playerPart(2,gradeName(2));
  player->position=p;player->refreshMoveSpeed();player->refreshMaxHp(*this);player->machState=2;player->useEngineBounce=true;
  safeBackLine=-32000;removeBgLine=-32000;
 }
 void spawnMapObject(const MapObjectDefinition& object,int layer)override{DrawSprite d;d.worldTexture=true;d.tile=object.name+".png";d.position=object.position;d.z=layer;mapSprites.push_back(std::move(d));}
 void spawnEnemy(const EnemyDefinition& definition)override{
  auto found=data.machines.find(definition.name);if(found==data.machines.end())throw std::runtime_error("Missing machine: "+definition.name);
  enemies.emplace_back();auto& m=enemies.back();m.id=nextID++;m.enemy=std::make_unique<GobHvM_Enemy>();m.definition=&found->second;
  m.behavior=found->second.behavior?&*found->second.behavior:nullptr;if(m.behavior&&m.behavior->independentRandom){m.privateRandom=2166136261u;for(unsigned char c:definition.name)m.privateRandom=(m.privateRandom^c)*16777619u;m.privateRandom^=uint32_t(int32_t(definition.position.x*16));}
  auto prev=current;current=&m;building=&found->second;m.enemy->makeMachFromDefinition(found->second,*this);building=nullptr;current=prev;
#if defined(HM_VALIDATION) && HM_VALIDATION
  m.validationSpawnPosition=definition.position;
#endif
  m.enemy->position=definition.position;m.enemy->machState=2;m.enemy->isBoss=definition.isBoss;
  m.enemy->hasWeapon=std::any_of(m.parts.begin(),m.parts.end(),[](const auto& p){return p&&p->armed;});
  if(m.behavior){const auto& b=*m.behavior;if(b.engineBounce)m.enemy->useEngineBounce=true;if(b.customGauge)m.enemy->gaugeY=b.gaugeY;if(b.fixedBounds)m.bounds={b.bounds[0],b.bounds[1],b.bounds[2],b.bounds[3]};}
 }
 void spawnMapEnemy(const EnemyDefinition& definition,size_t index)override{
  spawnEnemy(definition);enemies.back().mapEnemyIndex=index;enemies.back().bossActivationPending=definition.isBoss;
 }
 bool mapEnemyAlive(size_t index)const override{
  for(const auto& m:enemies)if(m.mapEnemyIndex==index)return !m.removed&&m.enemy->hp>0;
  return false;
 }
 void spawnItemBox(Vector2 p)override{GobItemBox box(*this);box.position=p;dynamic.emplace_back(box);}
 void event(ItemBoxEventKind e)override{if(e==ItemBoxEventKind::Pickup&&player){player->pickupItem(data,*this);changed();}}
 void event(const WorldEvent& e)override{
  SessionEvent out{SessionEventKind::WorldUI};out.code=int(e.kind);out.index=e.value;events.push_back(out);
  switch(e.kind){
  case WorldEventKind::ClearObjects:aimTarget=excludedTarget=0;autoFire=false;aimState={};aimState.mode=aimMode;particleQueued.fill(false);enemies.clear();dynamic.clear();mapSprites.clear();player.reset();playerGraph.parts.clear();break;
  case WorldEventKind::OpenStage:open(e.stage);break;
  case WorldEventKind::ExperienceZero:if(player)player->addExperience(0,*this);break;
  case WorldEventKind::SelectDefaultWeapon:selected=0;charge=0;refreshInventoryCharge();break;
  case WorldEventKind::SaveProgress:{GlobalInfo info(slot);info.updateLastStage(e.stage);changed();persist();break;}
  case WorldEventKind::TitleIn:audio("Title_TitleIn");break;
  case WorldEventKind::TitleBang:audio("Title_Bang");break;
  case WorldEventKind::ButtonSound:audio(0);break;
  default:break;
  }
 }
 void event(const PlayerEvent& e)override{
  SessionEvent out{SessionEventKind::PlayerUI};out.code=int(e.kind);out.index=e.index;out.amount=e.amount;out.value=e.value;out.delay=e.delay;out.position=e.position;out.text=e.name;out.active=e.active;events.push_back(std::move(out));
  switch(e.kind){
  case PlayerEventKind::Sound:audio(e.index);break;
  case PlayerEventKind::HPGauge:playerGraph.gauge=e.value;break;
  case PlayerEventKind::WeaponInventory:
   // Original selWeaponItem points into SaveSlot. Our aligned charge cache must
   // observe pickup additions before the next burst writes back its decrement.
   refreshInventoryCharge();
   break;
  case PlayerEventKind::Parts:playerPart(e.index,gradeName(e.index));changed();break;
  case PlayerEventKind::UpgradeOpen:world.paused=true;break;
  case PlayerEventKind::GameOver:world.paused=true;world.gameState=3;world.presentation=StagePresentation{};world.presentation.step=1;world.presentation.missionEnd=true;world.presentation.loadingAsset="StageClear.jpg";world.presentation.typoFrame=world.gameMode==1?3:1;audio("Title_TitleIn");changed();persist();break;
  case PlayerEventKind::PickupParticle:if(acquired){acquired->sprite.tile=e.name;atlas(acquired->sprite);acquired->sprite.frame=e.index;acquired->sprite.position=e.position;acquired->velocity=e.velocity;acquired->liveTime=e.life;acquired->scaleVelocity=e.scaleVelocity;startParticle(*acquired,globalTime);acquired=nullptr;}break;
  default:break;
  }
 }
 void event(const EnemyEvent& e)override{
  switch(e.kind){
  case EnemyEventKind::PartsDefinition:if(current&&building)enemyParts(*current,building->parts,nullptr,building->machType);break;
  case EnemyEventKind::KillBoss:if(current&&current->mapEnemyIndex!=SIZE_MAX)world.killMapBoss(current->mapEnemyIndex);else world.killBoss();break;
  case EnemyEventKind::Remove:if(current)current->removed=true;break;
  case EnemyEventKind::AddExperience:if(player){player->addExperience(e.amount,*this);changed();}break;
  case EnemyEventKind::Sound:audio(e.amount,e.pan);break;
  case EnemyEventKind::GaugeUpdate:if(current)current->gauge=e.scale;break;
  case EnemyEventKind::Explosion:if(acquired){acquired->sprite.tile=e.asset?e.asset:"";atlas(acquired->sprite);acquired->sprite.position=e.position;acquired->sprite.scale={e.scale,e.scale};acquired->sprite.frame=e.tileNo;acquired->sprite.blend=e.blend;acquired->liveTime=e.life;acquired->tileAnimation=true;acquired->animationFrames=e.animationFrames;startParticle(*acquired,globalTime);acquired=nullptr;}break;
  default:break;
  }
 }
 bool acquireParticle()override{acquired=world.freeParticle(globalTime);return acquired!=nullptr;}
 bool sweepTerrain(Vector2& p,const Vector2& delta)override{return world.sweepTerrain(p,delta);}
 bool inCamera(const BulletState& b)override{return b.show&&(!(assistedBullet&&assistedBullet->aimTarget)||!b.removed)&&((assistedBullet&&assistedBullet->aimTarget)||visibleOnCanvas(b.pos,{}));}
 std::optional<CombatTarget> targetAt(bool toPlayer,Vector2 p)override{
  if(toPlayer){if(player&&pointInObject(p,player->position,playerBounds()))return 1;return {};}
  for(auto& m:enemies)if(((assistedBullet&&assistedBullet->aimTarget==m.id&&!m.removed&&m.enemy->hp>0)||m.inScreen||(m.enemy->visible&&visibleOnCanvas(m.enemy->position,m.bounds)))&&pointInObject(p,m.enemy->position,m.bounds))return m.id;
  return {};
 }
 bool waitingForBossEncounter(const Machine& m)const{
  return m.enemy&&m.enemy->isBoss&&m.mapEnemyIndex<world.bossEncounterActivated.size()&&!world.bossEncounterActivated[m.mapEnemyIndex];
 }
#if defined(HM_VALIDATION) && HM_VALIDATION
 void observeDamage(CombatTarget id,int32_t requested,int32_t before,int32_t after){
  if(!damageObserver)return;
  const auto* bullet=assistedBullet?std::get_if<BulletState>(&assistedBullet->value):nullptr;
  damageObserver(id,bullet&&bullet->info?std::string_view(bullet->info->name):std::string_view{},requested,before,after);
 }
#endif
 EnemyFrame enemyFrame(const Machine& m)const{
  EnemyFrame f{world.motionWorld(m.enemy->position.x,globalTime),player?player->position:Vector2{},stage.mission,slot.level()};
  f.motion.paused=f.motion.paused||waitingForBossEncounter(m);return f;
 }
 void damage(CombatTarget id,int32_t amount)override{
#if defined(HM_VALIDATION) && HM_VALIDATION
  if(id==1&&player&&validationCheats&&amount>0){++cheatStats.blockedHits;observeDamage(id,amount,player->hp,player->hp);return;}
#endif
  if(assistedBullet&&assistedBullet->aimTarget==id){++aimState.targetHits;assistedBullet->aimTarget=0;}
  if(id==1){if(player){
#if defined(HM_VALIDATION) && HM_VALIDATION
   const auto before=player->hp;
#endif
   player->onDamage(amount,*this);changed();
#if defined(HM_VALIDATION) && HM_VALIDATION
   observeDamage(id,amount,before,player->hp);
#endif
  }return;}
  for(auto& m:enemies)if(m.id==id){auto prev=current;current=&m;
#if defined(HM_VALIDATION) && HM_VALIDATION
   const bool aliveBoss=m.enemy->isBoss&&m.enemy->hp>0;
   const auto before=m.enemy->hp;
#endif
   m.enemy->onDamage(amount,enemyFrame(m),*this);
#if defined(HM_VALIDATION) && HM_VALIDATION
   if(aliveBoss&&m.enemy->hp<=0)++cheatStats.bossKills;
   observeDamage(id,amount,before,m.enemy->hp);
#endif
   current=prev;return;}
 }
 void spawnBullet(const WeaponInfo& info,bool toPlayer,float angle,Vector2 p,double time)override{
#if defined(HM_VALIDATION) && HM_VALIDATION
  if(!toPlayer&&shotObserver)shotObserver(info,angle,p);
#endif
  BulletState b;b.toPlayer=toPlayer;pendingTrail.reset();setBulletFire(b,info,angle,p,time,*this);dynamic.emplace_back(b);auto& node=dynamic.back();node.trailForBullet=std::move(pendingTrail);
  if(!toPlayer&&aimMode==AimMode::automatic){if(auto target=findAimTarget(aimTarget)){
   // Track ownership for offscreen collision and test counters only. Preserve
   // setBulletFire's original position, randomized velocity and angle.
   node.aimTarget=target->id;++aimState.assistedShots;
  }}
 }
 void event(TrailEventKind)override{}
 ParticleState* acquireBulletParticle()override{return world.freeParticle(globalTime);}
 void addBulletTrailPosition(BulletState& bullet,Vector2 position)override{for(auto& n:dynamic)if(std::get_if<BulletState>(&n.value)==&bullet){if(n.trailForBullet)n.trailForBullet->addTrailPos(position,world.gameTime,*this);return;}}
 void event(const CombatEvent& e)override{
  if(e.kind==CombatEventKind::FireSound){audio(e.soundName,e.pan);return;}
  if(e.kind==CombatEventKind::ImpactSound){audio(int(e.sound),e.pan);return;}
  if(e.kind==CombatEventKind::TrailCreate){
   pendingTrail=std::make_shared<GobTrailEffect>(e);
   // Original addValidPos 0x1acd0 reads Tile2D maxS/maxT. The active
   // 40x64 trail textures occupy 40/64 of the padded texture's U range.
   auto size=sizes(e.tile);int pw=1,ph=1;while(pw<size.x&&pw<1024)pw*=2;while(ph<size.y&&ph<1024)ph*=2;
   pendingTrail->maxS=size.x/pw;pendingTrail->maxT=size.y/ph;
   dynamic.emplace_back(pendingTrail);return;
  }
  if(e.kind==CombatEventKind::ChargeChanged){
#if defined(HM_VALIDATION) && HM_VALIDATION
   // Restore the round before tickWeapon can remove a one-round weapon.
   if(validationCheats&&selected>0){++charge;++cheatStats.refilledRounds;}
#endif
   if(selected>0)slot.setInt(0x4c+size_t(selected)*0x1c,charge);changed();return;}
  if(e.kind==CombatEventKind::RemoveWeapon){replaceWeapon=true;return;}
  if(e.kind==CombatEventKind::ReloadGauge||e.kind==CombatEventKind::ReloadComplete||e.kind==CombatEventKind::ChargePreview){SessionEvent out{SessionEventKind::CombatUI};out.code=int(e.kind);out.value=e.value;events.push_back(out);return;}
  if(e.kind==CombatEventKind::TerrainExplosion||e.kind==CombatEventKind::TargetExplosion||e.kind==CombatEventKind::DamageFlash||e.kind==CombatEventKind::MuzzleFlash||e.kind==CombatEventKind::Shell){
   if(!acquired)return;auto& p=*acquired;p.sprite.tile=e.asset?e.asset:std::string(e.tile);atlas(p.sprite);p.sprite.frame=e.tileNo;p.sprite.position=e.pos;p.sprite.blend=e.kind==CombatEventKind::MuzzleFlash?2:e.blend;p.sprite.scale={e.scale,e.scale};p.velocity=e.velocity;p.gravity=e.gravity;p.rotationVelocity=e.rotationSpeed;p.liveTime=e.life;p.tileAnimation=e.animationFrames>0;p.animationFrames=e.animationFrames;
   startParticle(p,globalTime);acquired=nullptr;
  }
 }
 Machine* findAimTarget(uintptr_t id){for(auto& m:enemies)if(m.id==id&&!m.removed&&m.enemy->hp>0&&player&&m.enemy->position.x>player->position.x)return &m;return nullptr;}
 Vector2 aimPoint(const Machine& m)const{return {m.enemy->position.x+(m.bounds.left+m.bounds.right)*.5f,m.enemy->position.y+(m.bounds.bottom+m.bounds.top)*.5f};}
 uintptr_t nearestAimTarget(uintptr_t skip){
  uintptr_t best=0;float distance=INFINITY;
  for(auto& m:enemies){if(m.id==skip||m.removed||m.enemy->hp<=0||!player||m.enemy->position.x<=player->position.x)continue;auto d=aimPoint(m);d.x-=player->position.x;d.y-=player->position.y;float squared=d.x*d.x+d.y*d.y;
   if(squared<distance||(squared==distance&&m.id<best)){distance=squared;best=m.id;}}
  return best;
 }
 void cycleAimTarget(){if(aimMode!=AimMode::automatic)return;
  auto old=aimTarget;auto currentTarget=findAimTarget(old);
  auto distance=[this](const Machine& m){auto p=aimPoint(m);return (p.x-player->position.x)*(p.x-player->position.x)+(p.y-player->position.y)*(p.y-player->position.y);};
  uintptr_t next=0;float best=INFINITY,previous=currentTarget?distance(*currentTarget):-1;
  for(auto& m:enemies){if(m.id==old||m.removed||m.enemy->hp<=0||!player||m.enemy->position.x<=player->position.x)continue;float d=distance(m);
   if((d>previous||(d==previous&&m.id>old))&&(d<best||(d==best&&m.id<next))){next=m.id;best=d;}}
  aimTarget=next?next:nearestAimTarget(old);excludedTarget=aimTarget?0:old;
 }
 bool clearAimPath(Vector2 origin,const AimSolution& solution,const WeaponInfo& info){
  if(info.pierce)return true;auto v=solution.velocity;
  for(int n=0;n<solution.ticks;++n){v.y-=info.g;Vector2 step{v.x*aimTick,v.y*aimTick};if(world.sweepTerrain(origin,step))return false;origin.x+=step.x;origin.y+=step.y;}return true;
 }
 AimSolution aimPath(Vector2 origin,Vector2 target,Vector2 step,const WeaponInfo& info){
  auto low=solveAim(origin,target,step,info.spd,info.g,info.liveTm);
  if(!low||clearAimPath(origin,low,info)||info.g<=0||info.spd<=0)return low;
  auto high=solveAim(origin,target,step,info.spd,info.g,info.liveTm,true);
  return high&&clearAimPath(origin,high,info)?high:low;
 }
 void updateAutoAim(){
  if(aimMode!=AimMode::automatic||!player||world.paused||world.gameState!=0||playerGraph.parts.size()<3||!playerGraph.parts[2])return;
  auto& part=*playerGraph.parts[2];auto& w=part.weapon;const auto& info=*w.weaponInfo;
  if(!findAimTarget(aimTarget))aimTarget=nearestAimTarget(excludedTarget);
  aimState.mode=aimMode;aimState.target=aimTarget;aimState.status=AimStatus::none;aimState.flightTicks=0;
  auto target=findAimTarget(aimTarget);
  if(!target){setWeaponAim(w,.1f);w.fireTarget={player->position.x+200,player->position.y+20};w.isFire=autoFire;aimState.angle=.1f;return;}
  auto center=aimPoint(*target);aimState.position=target->enemy->position;aimState.screenPosition=cameraPoint(target->enemy->position);aimState.bounds=target->bounds;
  AimSolution solution;Vector2 origin{};float originalAngle=w.aimDir,candidateAngle=w.aimDir;
  for(int pass=0;pass<3;++pass){w.aimDir=candidateAngle;auto f=frame(*player,playerGraph,part);refreshWeaponMuzzle(w,f);origin={f.weaponPos.x+w.muzzlePos.x,f.weaponPos.y+w.muzzlePos.y};
   if(info.wpnType==5)origin={info.spd==0?center.x:player->position.x,player->position.y+325};
   solution=aimPath(origin,center,target->aimStep,info);
   if(!solution)break;
   if(info.rotWpn)candidateAngle=solution.angle;
  }
  w.aimDir=originalAngle;refreshWeaponMuzzle(w,frame(*player,playerGraph,part));
  bool clear=solution&&clearAimPath(origin,solution,info);
  if(solution&&info.rotWpn)setWeaponAim(w,solution.angle);
  aimState.angle=info.rotWpn?solution.angle:w.aimDir;aimState.flightTicks=solution.ticks;
  aimState.status=!clear?AimStatus::blocked:(!info.rotWpn||std::fabs(w.aimDir-w.aimDirDest)<.1f)?AimStatus::ready:AimStatus::tracking;
  w.fireTarget=center;w.isFire=autoFire&&clear;
 }
 CombatFrame frame(GobHvMach& owner,Machine& graph,Part& p){
  CombatFrame f;f.time=world.gameTime;f.paused=world.paused||waitingForBossEncounter(graph)||graph.formationEntering;f.playerPos=player?player->position:Vector2{};f.ownerPos=owner.position;f.weaponPos=owner.position;f.rotPos=p.visual.rotatedPosition;f.socket0=socket(p.visual.info,0);f.socket1=socket(p.visual.info,1);f.machType=owner.machType;f.isPlayer=owner.player;f.isMainPlayer=&graph==&playerGraph;f.ownerInScreen=graph.inScreen;f.weaponUpgrade=slot.bytes[0x2d];
  if(graph.behavior&&graph.behavior->mountedWeaponOrigin){auto anchor=rotateAttachment(graph.behavior->muzzle,owner.rotation);f.weaponPos={owner.position.x+anchor.x,owner.position.y+anchor.y};f.rotPos={};f.socket0={};f.socket1={};}
  if(player){f.damageBoost=player->passives.active(1);f.shield=player->passives.active(0);}f.selectedWeapon=int16_t(selected);f.selectedCharge=&charge;return f;
 }
 void removeExhaustedWeapon(){
  replaceWeapon=false;
  // removeCurWeapon0x1c408: discard the old item before deleting its inventory
  // record. This happens before dynamic item pickups in the same world tick.
  if(acquireParticle()){
   auto& p=*acquired;p.sprite.tile=slot.weaponName(size_t(selected))+".png";p.sprite.cols=1;p.sprite.rows=1;p.sprite.frame=0;p.sprite.blend=0;
   p.liveTime=2;p.sprite.position={player->position.x,player->position.y+40};p.sprite.scale={1,1};p.rotationVelocity=15;p.gravity=250;p.velocity={-50,150};
   startParticle(p,globalTime);acquired=nullptr;
  }
  GlobalInfo info(slot);info.removeWeapon(size_t(selected));select(0);audio(5);changed();
 }
 void motionEvents(GobHvMach& owner,Machine& graph,const MotionEvents& result){
  for(const auto& e:result){if(e.kind==MotionEventKind::Damage)damage(graph.id,e.amount);
   else if(e.kind==MotionEventKind::NextStage)world.nextStage(stage,*this);
   else if(e.kind==MotionEventKind::ModelAim){for(auto& p:graph.parts)if(p&&p->armed)setWeaponAim(p->weapon,e.value);}
   else if(e.kind==MotionEventKind::BoosterSmoke){if(acquireParticle()){auto& p=*acquired;p.sprite.tile="SmokeEffect.png";p.sprite.position=owner.position;p.liveTime=.4f;float scale=rnd()*.4f+.2f;p.sprite.scale={scale,scale};p.scaleVelocity=rnd()*1.5f+.5f;float r=rnd()*float(std::numbers::pi);p.sprite.rotation=r+r;p.rotationVelocity=rnd()*5.f-2.5f;p.velocity={0,-100};startParticle(p,globalTime);acquired=nullptr;}}
   else if(e.kind==MotionEventKind::TickChildren){
    graph.inScreen=owner.visible&&inView(owner.position,graph.bounds);
    if(&graph==&playerGraph)updateAutoAim();
    for(size_t n=0;n<graph.parts.size();++n){auto* p=graph.parts[n].get();if(!p||!p->armed)continue;
     tickWeapon(p->weapon,frame(owner,graph,*p),*this);
     if(replaceWeapon){
      // Finish the old call before destroying its Part. The original player
      // weapon tail has no AI/RNG; replacement still precedes item processing.
      removeExhaustedWeapon();auto* replacement=playerGraph.parts[2].get();
      // Original QobBase re-reads childArray.count and visits the appended gun.
      if(replacement&&replacement->armed)tickWeapon(replacement->weapon,frame(*player,playerGraph,*replacement),*this);
     }
    }
    registerGauges(owner,graph);
   }
  }
 }
 void open(StagePosition pos){stage=pos;char key[32];if(world.gameMode==1)std::snprintf(key,sizeof(key),"Survival");else std::snprintf(key,sizeof(key),"Stage%02d",GlobalInfo::mapNumber(stage));
  auto it=data.maps.find(key);if(it==data.maps.end())throw std::runtime_error("Missing original map: "+std::string(key));auto formation=data.formations.find(key);reinforcement=formation==data.formations.end()?nullptr:&formation->second;reinforcementSpawned=false;world.openMap(it->second,world.gameMode,*this);active=true;
 }
 void start(int mode,StagePosition pos,double retainedWorldTime){if(!std::isfinite(retainedWorldTime)||retainedWorldTime<0)throw std::invalid_argument("invalid retained World time");events.clear();unsupported.clear();auto retainedCamera=world.camera;auto retainedWidth=world.canvasWidth;auto retainedHeight=world.canvasHeight;world=WorldState{};world.setCanvasWidth(retainedWidth);world.canvasHeight=retainedHeight;world.camera=retainedCamera;world.gameTime=retainedWorldTime;world.gameMode=mode;world.configureSurvival(slot.level());enemies.clear();dynamic.clear();mapSprites.clear();player.reset();playerGraph.parts.clear();nextID=2;selected=0;charge=0;replaceWeapon=false;acquired=nullptr;particleQueued.fill(false);aimTarget=excludedTarget=0;autoFire=false;aimState={};aimState.mode=aimMode;assistedBullet=nullptr;open(pos);
  // Original openMap0xe3f4 creates objects only. drawVisual follows tick once
  // in renderScene0x21e0; collecting here advances weapon aim/recoil twice.
  draws.clear();render.clear();}
 void spawnReinforcements(){
  if(!reinforcement||reinforcementSpawned||!player||world.paused||world.gameState!=0||player->hp<=0)return;
  bool ready=false;
  if(!reinforcement->boss.empty()){
   for(size_t n=0;n<world.map.enemies.size();++n)if(world.map.enemies[n].name==reinforcement->boss&&world.bossEncounterActivated[n]&&!world.mapEnemiesDefeated[n]){ready=true;break;}
  }else ready=player->position.x>=world.map.right-800;
  if(!ready)return;
  reinforcementSpawned=true;
  float spanX=0,spanY=0;
  for(const auto& unit:reinforcement->units){spanX=std::max(spanX,unit.offset.x);spanY=std::max(spanY,unit.offset.y);}
  const float screenRight=std::max(world.camera.x+world.canvasWidth*.5f,
   std::min(world.frontLine,player->position.x+140+world.canvasWidth*.5f));
  // Translate the whole authored formation. Each aircraft travels the same
  // distance, retaining its X/Y separation until the original flight AI starts.
  const float endLeft=std::min(world.frontLine-90,screenRight-72)-spanX;
  const float spawnLeft=std::max(screenRight+100,player->position.x+180);
  float ground=world.map.bottom;
  // Include body width and terrain crossed during the horizontal entry.
  for(float x=endLeft-60;x<=std::min(world.map.right,spawnLeft+spanX+60);x+=10){
   float h=world.mapHeight(x);if(h!=10000)ground=std::max(ground,h);
  }
  const float floor=ground+45,ceiling=world.map.top-45;
  const float baseY=std::clamp(ground+80,floor,std::max(floor,ceiling-spanY));
  for(const auto& unit:reinforcement->units){const auto offset=unit.offset;
   spawnEnemy({unit.machine,{spawnLeft+offset.x,baseY+offset.y},false});
   auto& machine=enemies.back();machine.formationEntering=true;machine.formationEndX=endLeft+offset.x;machine.inScreen=false;
   machine.entrySpeed=reinforcement->entrySpeed;machine.releaseSpeed=reinforcement->releaseSpeed;
  }
 }
 MotionEvents advanceFormationEntry(Machine& m){
  if(!world.paused){
   m.enemy->speed=m.entrySpeed;m.enemy->velocity.y=0;
  }
  auto events=m.enemy->GobHvMach::tick(enemyFrame(m).motion);
  if(!world.paused&&m.enemy->position.x<=m.formationEndX){
   m.formationEntering=false;m.enemy->speed=m.enemy->speedDest=m.releaseSpeed;
   for(auto& p:m.parts)if(p&&p->armed)p->weapon.resetTime=float(world.gameTime);
  }
  return events;
 }
 void tick(double time,float tilt){
#if defined(HM_VALIDATION) && HM_VALIDATION
  // Invulnerability alone leaves a fallen player descending forever. Rescue
  // only below the map, onto nearby terrain before the active boss/exit gate.
  if(validationCheats&&player&&world.gameState==0&&player->position.y<world.map.bottom+20){
   for(int step=0;step<=100;++step){float x=player->position.x+float(step*5);
    if(x>world.frontLine-40||x>world.map.right-60)break;
    float y=world.mapHeight(x);if(y==10000)continue;
    player->position={x,y+60};player->velocity={};player->velocityTick={};player->currentCol=player->prevCol=-1;player->machState=2;++cheatStats.fallRescues;break;
   }
  }
#endif
  globalTime=time;if(!active)return;playerGraph.gaugeQueued.fill(false);for(auto& m:enemies)m.gaugeQueued.fill(false);
  if(player&&!world.paused&&world.gameState==0)player->setMoveVel(WorldState::moveInput(tilt),player->passives.active(3));
  world.tick(player?std::optional<Vector2>(player->position):std::nullopt,stage,*this,[this]{return random31();});
  spawnReinforcements();
  for(auto it=enemies.begin();it!=enemies.end();){if(it->removed){it=enemies.erase(it);continue;}auto& m=*it;
   if(m.bossActivationPending&&!waitingForBossEncounter(m)&&!world.paused){
    // Prefetch prepares visuals, not elapsed weapon-AI delay. Keep each
    // original random delay; begin its clock when the encounter activates.
    for(auto& p:m.parts)if(p&&p->armed)p->weapon.resetTime=float(world.gameTime);
    m.bossActivationPending=false;
   }
   current=&m;auto before=m.enemy->position;auto motion=m.formationEntering?advanceFormationEntry(m):m.enemy->tick(enemyFrame(m),*this);
   if(!world.paused)m.aimStep={m.enemy->position.x-before.x,m.enemy->position.y-before.y};
   if(m.behavior&&m.behavior->velocityPitch){
    if(!world.paused)m.attitude=velocityPitch(m.attitude,m.enemy->speed,m.enemy->velocity.y,*m.behavior);
    m.enemy->rotation=m.attitude;
    for(auto& p:m.parts)if(p&&p->armed)p->visual.rotatedPosition=rotateAttachment(p->visual.basePosition,m.attitude);
    if(m.behavior->rotateBounds){const auto& b=m.behavior->bounds;ObjectBounds rotatedBounds{};
     for(auto point:{Vector2{b[0],b[1]},Vector2{b[0],b[3]},Vector2{b[2],b[1]},Vector2{b[2],b[3]}}){auto r=rotateAttachment(point,m.attitude);rotatedBounds.left=std::min(rotatedBounds.left,r.x);rotatedBounds.right=std::max(rotatedBounds.right,r.x);rotatedBounds.bottom=std::min(rotatedBounds.bottom,r.y);rotatedBounds.top=std::max(rotatedBounds.top,r.y);}m.bounds=rotatedBounds;}
   }
   motionEvents(*m.enemy,m,motion);current=nullptr;++it;}
  for(size_t n=0;n<world.particles.size();++n){auto& p=world.particles[n];tickParticle(p,globalTime);particleQueued[n]=p.show;}
  if(player){
   if(world.gameMode==0&&safeBackLine<player->position.x-240){safeBackLine=player->position.x-240;if(safeBackLine>removeBgLine+320){float boundary=removeBgLine;float renderBoundary=world.canvasWidth==480?boundary:std::min(boundary,world.camera.x-world.canvasWidth*.5f-60);std::erase_if(mapSprites,[&](const DrawSprite& d){return renderBoundary>d.position.x+mapTileSize(d.tile).x;});if(world.backLine<boundary)world.backLine=boundary;removeBgLine=safeBackLine;}}
   motionEvents(*player,playerGraph,player->GobHvMach::tick(world.motionWorld(player->position.x,globalTime,[this]{return random31();})));
  }
  for(auto it=dynamic.begin();it!=dynamic.end();){bool remove=std::visit([](const auto& n){if constexpr(std::is_same_v<std::decay_t<decltype(n)>,std::shared_ptr<GobTrailEffect>>)return n->removed;else return n.removed;},it->value);if(remove){it=dynamic.erase(it);continue;}
   if(auto b=std::get_if<BulletState>(&it->value)){CombatFrame f;f.time=world.gameTime;f.paused=world.paused;f.playerPos=player?player->position:Vector2{};f.weaponUpgrade=slot.bytes[0x2d];if(player){f.damageBoost=player->passives.active(1);f.shield=player->passives.active(0);}assistedBullet=&*it;tickBullet(*b,f,*this);assistedBullet=nullptr;}
   else if(auto item=std::get_if<GobItemBox>(&it->value)){auto& box=*item;ItemBoxFrame f;f.motion=world.motionWorld(box.position.x,globalTime);f.hasPlayer=bool(player);if(player)f.playerPosition=player->position;box.tick(f,*this);box.inScreen=box.show&&inView(box.position,imageBounds(tileSize("ItemBox.png").x,tileSize("ItemBox.png").y));}
   else std::get<std::shared_ptr<GobTrailEffect>>(it->value)->tick(world.gameTime,world.paused,*this);
   ++it;
  }
  // GuiGame ticks after World even while ClearObjects leaves no player.
  passives.tick(*this);collect();
 }
 void append(DrawSprite d,bool isScreen=false){if(d.tile.empty()||d.tile=="none")return;if(!isScreen)d.position=screen(d.position);render.emplace_back(d);draws.push_back(std::move(d));}
 bool hasGauge(const Machine& graph)const{return &graph==&playerGraph?player&&player->hasHPGauge:graph.enemy&&graph.enemy->hasHPGauge;}
 std::string_view gaugeTile(const Machine& graph)const{return &graph==&playerGraph?"HPGauge.png":graph.enemy->hpMax>=501?"HPGaugeBoss.png":"HPGaugeEnemy.png";}
 Vector2 gaugePoint(const GobHvMach& owner,const Machine& graph,int child)const{
  const bool isPlayer=&graph==&playerGraph;
  const float x=child?(isPlayer?-(graph.gauge*-21.f+21.f):-(graph.enemy->gaugeSize-graph.gauge*graph.enemy->gaugeSize)):0;
  // posY0x79d4 recursively adds each parent's world position. Preserve the
  // binary32 rounding of the background before adding the fill's local Y=1.
  const float backY=owner.position.y+(isPlayer?60.f:graph.enemy->gaugeY);
  return cameraPoint({owner.position.x+x,backY+float(child)},true);
 }
 void registerGauges(const GobHvMach& owner,Machine& graph){
  graph.gaugeQueued.fill(false);if(!owner.visible||graph.removed||!hasGauge(graph))return;
  // QobBase::tick0x7830 registers each QobImage independently of its parent.
  // Bounds do not shrink with the fill's scaleX (setScale0x8234).
  auto size=tileSize(gaugeTile(graph));auto b=imageBounds(size.x,size.y/2);
  for(int child=0;child<2;++child){auto p=gaugePoint(owner,graph,child);graph.gaugeQueued[child]=!(p.x+b.right<0||p.x+b.left>world.canvasWidth+60||p.y+b.top<0||p.y+b.bottom>320);}
 }
 void machineDraw(GobHvMach& owner,Machine& graph){
  if(!owner.visible)return;
  if(hasGauge(graph))for(int child=0;child<2;++child)if(graph.gaugeQueued[child]){
   // draw0x8cf0 reads current position/scale without repeating registration.
   // A later bullet can change the fill or kill its owner in this same frame.
   auto p=gaugePoint(owner,graph,child);DrawSprite d;d.tile=gaugeTile(graph);d.rows=2;d.frame=1-child;d.z=2;
   // Compensate the host-wide quake transform after fixedPos rounding.
   d.position={p.x+cameraOffset.x,p.y+cameraOffset.y};d.scale.x=child?graph.gauge:1;append(std::move(d),true);
  }
  if(!graph.inScreen&&!visibleOnCanvas(owner.position,graph.bounds))return;
  // GobHvMach::draw0x57c0 traverses its existing layer. Original insertion
  // and refreshMyMachLayer rebuild order only when parts change.
  if(graph.drawOrderDirty){graph.drawOrder.clear();for(auto& p:graph.parts)if(p)graph.drawOrder.push_back(p.get());
   std::stable_sort(graph.drawOrder.begin(),graph.drawOrder.end(),[](auto a,auto b){return a->order<b->order;});graph.drawOrderDirty=false;}
  for(auto p:graph.drawOrder){auto d=p->armed?drawWeapon(p->visual,p->weapon,owner.position,owner.player,owner.machType,owner.rotation,owner.engineBounce,1):drawParts(p->visual,owner.position,owner.rotation,owner.engineBounce,owner.machState,owner.speed,globalTime,1);if(p->visual.info&&p->visual.info->tileCnt==0)continue;if(auto f=presentationFrame(p->visual,world.gameTime))applyPartPresentation(d,p->visual,owner.position,owner.rotation,owner.velocity.y,world.gameTime,tileSize(f->tile));append(std::move(d));}

 }
 void collect(){
  draws.clear();render.clear();if(world.hasBackground){DrawSprite d;d.worldTexture=true;d.tile=world.map.background;d.position=world.backgroundPosition;d.scale={world.backgroundDrawScale,world.backgroundDrawScale};
  if(world.canvasHeight>320&&world.backgroundSize.y>1){float scale=std::max({d.scale.x,world.canvasWidth/std::max(1.f,world.backgroundSize.x-1),world.canvasHeight/(world.backgroundSize.y-1)});d.scale={scale,scale};d.position.y=world.camera.y+(world.canvasHeight-320)*.5f;}
  d.z=0;append(d);}
  for(const auto& d:mapSprites){
   // processObjectRegen0xcd00 marks map QobImages fixedPos1.
   auto size=mapTileSize(d.tile);auto bounds=imageBounds(size.x,size.y);auto p=cameraPoint(d.position,true);
   if(p.x+bounds.right<0||p.x+bounds.left>world.canvasWidth+60||p.y+bounds.top<0||p.y+bounds.bottom>world.canvasHeight)continue;
   auto fixed=d;fixed.position={p.x+cameraOffset.x,p.y+cameraOffset.y};append(std::move(fixed),true);
  }
  for(auto& m:enemies)machineDraw(*m.enemy,m);
  for(size_t n=0;n<world.particles.size();++n)if(particleQueued[n]){auto& p=world.particles[n];auto size=tileSize(p.sprite.tile);if(visibleOnCanvas(p.sprite.position,imageBounds(size.x/float(p.sprite.cols),size.y/float(p.sprite.rows))))append(p.sprite);}
  if(player)machineDraw(*player,playerGraph);
  for(auto& node:dynamic){if(auto b=std::get_if<BulletState>(&node.value)){
    // GobBullet::isInCam 0xab00 returns true for QobBase visual registration.
    // Keep the separate super-isInCam collision test in tickBullet unchanged:
    // offscreen smoke RNG, flame toggles and trail samples still run during draw.
    if(b->show&&!b->removed&&b->info)for(auto& d:drawBullet(*b,world.gameTime,globalTime,*this))append(std::move(d));
   }else if(auto box=std::get_if<GobItemBox>(&node.value)){if(box->show&&!box->removed&&box->inScreen)append(box->draw());}
   else {auto& trail=std::get<std::shared_ptr<GobTrailEffect>>(node.value);
    // 0x1b5c0 removes expired trails before 0x7830 visual registration.
    if(!trail->removed){auto mesh=trail->draw();mesh.translation=screen(mesh.translation);if(!mesh.vertices.empty())render.emplace_back(std::move(mesh));}}}
  std::stable_sort(render.begin(),render.end(),[](const auto& a,const auto& b){auto layer=[](const SessionRenderItem& i){auto s=std::get_if<DrawSprite>(&i);return s?s->z:2;};return layer(a)<layer(b);});
  std::stable_sort(draws.begin(),draws.end(),[](const auto& a,const auto& b){return a.z<b.z;});
 }
};
GameSession::GameSession(const GameData& d,SaveSlot& s,TileSize t,Random31 r,PlayerPassives* shared,TileSize worldT):impl_(std::make_unique<Impl>(d,s,std::move(t),std::move(r),shared,std::move(worldT))){}
GameSession::~GameSession()=default;GameSession::GameSession(GameSession&&)noexcept=default;GameSession& GameSession::operator=(GameSession&&)noexcept=default;
void GameSession::start(int m,int mission,int stage){start(m,mission,stage,impl_->world.gameTime);}
void GameSession::start(int m,int mission,int stage,double retainedWorldTime){impl_->start(m,{mission,stage},retainedWorldTime);}
void GameSession::tick(double t,float tilt){impl_->tick(t,tilt);}
void GameSession::prepareHUDPreview(){
 auto& i=*impl_;if(!i.player)return;
 auto ground=[&](float x){float y=i.world.mapHeight(x);return y==10000?0.f:y;};
 float x=i.world.map.left+100;i.player->position={x,ground(x)+10};i.player->visible=true;
 i.world.camera={i.world.map.left+i.world.canvasWidth*.5f,ground(x)+60};
 for(auto& m:i.enemies){bool air=m.definition->machType==1;float ex=i.world.map.left+(air?350:250);m.enemy->position={ex,ground(ex)+(air?130:10)};m.enemy->visible=true;}
 i.world.paused=true;i.collect();i.events.clear();
}
void GameSession::prepareInitialPausedFrame(){
 if(!impl_->world.paused||!impl_->render.empty())return;
 impl_->world.processCamera(impl_->player?std::optional<Vector2>(impl_->player->position):std::nullopt);
 impl_->collect();
}
void GameSession::restoreWorldCamera(Vector2 position){impl_->world.camera=position;}
void GameSession::setCanvasWidth(float width){
 auto& i=*impl_;float old=i.world.canvasWidth;i.world.setCanvasWidth(width);
 if(i.world.paused&&width!=old){float dx=(width-old)*.5f;
  for(auto& item:i.render){if(auto sprite=std::get_if<DrawSprite>(&item))sprite->position.x+=dx;else std::get<TrailMesh>(item).translation.x+=dx;}
  for(auto& sprite:i.draws)sprite.position.x+=dx;
 }
}
void GameSession::setCanvasHeight(float height){if(!std::isfinite(height)||height<320||height>16384)throw std::invalid_argument("Canvas height outside supported range");impl_->world.canvasHeight=height;}
float GameSession::canvasHeight()const{return impl_->world.canvasHeight;}
float GameSession::canvasWidth()const{return impl_->world.canvasWidth;}
void GameSession::setCameraOffset(Vector2 offset){impl_->cameraOffset=offset;}
void GameSession::touch(Vector2 p,int state){auto& i=*impl_;if(!i.player||(i.world.paused&&state!=2))return;auto weapon=i.playerGraph.parts.size()>2?i.playerGraph.parts[2].get():nullptr;if(weapon){if(i.aimMode==AimMode::manual)i.player->setTap(p,state,i.cameraPoint(i.player->position),weapon->visual.rotatedPosition,&weapon->weapon);
 else if(state==0){auto screen=i.cameraPoint(i.player->position);float dx=p.x-screen.x-weapon->visual.rotatedPosition.x,dy=p.y-screen.y-weapon->visual.rotatedPosition.y;
 if(std::fabs(dx)<100&&std::fabs(dy)<60&&i.player->dashVel==0&&i.slot.bytes[0x2f]>2)i.player->dashVel=dx<0?-5.f:5.f;}}}
void GameSession::releaseFire(){impl_->autoFire=false;auto& parts=impl_->playerGraph.parts;if(parts.size()>2&&parts[2])parts[2]->weapon.isFire=false;}
void GameSession::setAimMode(AimMode mode){if(impl_->aimMode==mode)return;releaseFire();impl_->aimMode=mode;impl_->aimTarget=impl_->excludedTarget=0;impl_->aimState={};impl_->aimState.mode=mode;}
AimMode GameSession::aimMode()const{return impl_->aimMode;}
void GameSession::setAutoFire(bool down){auto& i=*impl_;if(i.aimMode!=AimMode::automatic)return;i.autoFire=down&&!i.world.paused&&i.world.gameState==0;if(!i.autoFire)releaseFire();}
void GameSession::cycleAimTarget(){impl_->cycleAimTarget();}
AimSnapshot GameSession::aimSnapshot()const{auto state=impl_->aimState;state.mode=impl_->aimMode;return state;}
void GameSession::booster(){auto& i=*impl_;if(i.player&&!i.world.paused)i.motionEvents(*i.player,i.playerGraph,i.player->setBooster(i.slot.bytes[0x2f],i.player->passives.active(2)));}
void GameSession::selectWeapon(int n){impl_->select(n);}void GameSession::cycleWeapon(int delta){int count=impl_->slot.weaponCount();if(count>0)impl_->select((impl_->selected+delta%count+count)%count);}
bool GameSession::selectUpgrade(int part){return impl_->player&&impl_->player->selectUpgrade(part,*impl_);}void GameSession::confirmUpgrade(){if(impl_->player)impl_->player->confirmUpgrade(*impl_);}
void GameSession::finishUpgradeSlide(){if(impl_->player&&impl_->player->upgradeStep==2){impl_->player->upgradeStep=0;impl_->world.paused=false;}}
void GameSession::resultButton(int id){impl_->world.resultButton(id,*impl_);}void GameSession::setPaused(bool value){impl_->world.paused=value;}
const WorldState& GameSession::world()const{return impl_->world;}const GobHvM_Player* GameSession::player()const{return impl_->player.get();}
StagePosition GameSession::stage()const{return impl_->stage;}int GameSession::selectedWeapon()const{return impl_->selected;}
size_t GameSession::enemyCount()const{return impl_->enemies.size();}size_t GameSession::bulletCount()const{return size_t(std::count_if(impl_->dynamic.begin(),impl_->dynamic.end(),[](const auto& n){return std::holds_alternative<BulletState>(n.value);}));}size_t GameSession::itemCount()const{return size_t(std::count_if(impl_->dynamic.begin(),impl_->dynamic.end(),[](const auto& n){return std::holds_alternative<GobItemBox>(n.value);}));}
#if defined(HM_VALIDATION) && HM_VALIDATION
Vector2 GameSession::validationAimOrigin()const{
 if(!impl_->player)return {};auto p=impl_->cameraPoint(impl_->player->position);if(impl_->playerGraph.parts.size()>2&&impl_->playerGraph.parts[2]){auto r=impl_->playerGraph.parts[2]->visual.rotatedPosition;p.x+=r.x;p.y+=r.y;}return p;
}
WeaponState GameSession::validationWeaponState()const{return impl_->playerGraph.parts.size()>2&&impl_->playerGraph.parts[2]?impl_->playerGraph.parts[2]->weapon:WeaponState{};}
void GameSession::validationObserveShots(ValidationShotObserver observer){impl_->shotObserver=std::move(observer);}
void GameSession::validationObserveDamage(ValidationDamageObserver observer){impl_->damageObserver=std::move(observer);}
void GameSession::validationSpawnEnemy(const EnemyDefinition& enemy){impl_->spawnEnemy(enemy);}
std::vector<GameSession::ValidationEnemySnapshot> GameSession::validationEnemies()const{
 std::vector<ValidationEnemySnapshot> result;for(const auto& m:impl_->enemies)result.push_back({m.id,m.definition->name,m.enemy->position,m.enemy->hp,m.enemy->hpMax,m.enemy->isBoss,m.removed,m.validationSpawnPosition,m.bounds});return result;
}
std::vector<GameSession::ValidationBulletSnapshot> GameSession::validationBullets()const{
 std::vector<ValidationBulletSnapshot> result;
 for(const auto& node:impl_->dynamic)if(const auto* b=std::get_if<BulletState>(&node.value))
  result.push_back({b->info?b->info->name:std::string{},b->toPlayer,b->removed,b->pos,b->vel,b->startTime});
 return result;
}
void GameSession::startValidation(const ValidationScenario& scenario){
 auto destination=validationSpawnPosition(impl_->data,scenario);
 prepareValidationSlot(impl_->slot,impl_->data,scenario);
 impl_->validationCheats=false;impl_->cheatStats={}; // Explicit opt-in per fixture.
 impl_->passives=PlayerPassives{}; // Explicit fixture reset only; never normal start/openMap.
 impl_->start(0,{scenario.mission,scenario.stage},0); // Explicit fixture reset, not a normal screen change.
 if(destination&&impl_->player){impl_->player->position=*destination;impl_->player->velocity={};impl_->player->velocityTick={};impl_->player->currentCol=-1;impl_->player->prevCol=-1;impl_->player->machState=2;
  impl_->world.processCamera(destination);impl_->world.processRegen(destination->x,*impl_);
 }
 if(impl_->player){impl_->world.camera={impl_->player->position.x+140.f,impl_->player->position.y+70.f};impl_->world.processCamera(impl_->player->position);}
 impl_->select(scenario.selectedWeapon);impl_->changed();
}
void GameSession::validationAddExperience(int32_t amount){
 if(amount<0)throw std::invalid_argument("validation EXP grant must be nonnegative");
 if(impl_->player){impl_->player->addExperience(amount,*impl_);impl_->changed();}
}
void GameSession::validationSetCheats(bool enabled){impl_->validationCheats=enabled;impl_->cheatStats={};}
GameSession::ValidationCheatStats GameSession::validationCheatStats()const{return impl_->cheatStats;}
#endif
std::span<const DrawSprite> GameSession::sprites()const{return impl_->draws;}std::span<const SessionRenderItem> GameSession::renderItems()const{return impl_->render;}std::vector<SessionEvent> GameSession::takeEvents(){auto result=std::move(impl_->events);impl_->events.clear();return result;}
}
