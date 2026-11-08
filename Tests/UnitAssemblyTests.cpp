#include "RuntimeUnitTestSupport.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "Trail.hpp"
#include <variant>
#include <memory>
#define private public
#include "GameSession.hpp"
#undef private
#include "../Sources/Core/GameSession.cpp"
#include <cassert>
#include <iostream>
#include <set>
using namespace heavymach;
namespace {
bool near(float a,float b){return std::abs(a-b)<.002f;}
ObjectBounds drawnBounds(const DrawSprite& d,Vector2 texture){
 const float w=texture.x/d.cols*d.scale.x*.5f,h=texture.y/d.rows*d.scale.y*.5f;
 const float x=std::abs(std::cos(d.rotation)*w)+std::abs(std::sin(d.rotation)*h);
 const float y=std::abs(std::sin(d.rotation)*w)+std::abs(std::cos(d.rotation)*h);
 return {d.position.x-x,d.position.y-y,d.position.x+x,d.position.y+y};
}
}
int main(){
 const auto root=std::filesystem::path(__FILE__).parent_path().parent_path();
 const auto original=GameData::loadDirectory(root/"Resources/RawData");
 const auto data=GameData::loadDirectory(root/"Resources/RuntimeData",true);
 const auto& heli=data.machines.at("Ka52");const auto& euro=data.machines.at("Lux_Eurofighter");
 assert(heli.hpMax==original.machines.at("Heli006").hpMax&&heli.giveExp==original.machines.at("Heli006").giveExp&&heli.machType==1);
 assert(euro.hpMax==data.machines.at("Lux_F22").hpMax&&euro.giveExp==data.machines.at("Lux_F22").giveExp&&euro.machType==1);
 assert(heli.behavior&&heli.behavior->customGauge&&heli.behavior->gaugeY==-18);
 assert(!heli.behavior->fixedBounds&&!heli.behavior->velocityPitch&&!heli.behavior->mountedWeaponOrigin&&!heli.behavior->independentRandom);
 assert(euro.behavior&&!euro.behavior->mountedWeaponOrigin);
 for(auto key:{"Ka52_Cannon","Ka52_Missile","Lux_Euro_Missile"}){
  const auto& p=data.parts.at(key);assert(p.partsType==2&&p.tileCnt==1&&p.sockets.size()==2);
  assert(data.weapons.at(key).rotWpn==1&&data.weapons.at(key).shotCnt>0);
 }
 const auto& cannon=data.weapons.at("Ka52_Cannon");const auto& priorGun=original.weapons.at("E_Tank_Wpn_01");
 assert(cannon.dmg==priorGun.dmg&&cannon.spd==priorGun.spd&&cannon.delay==priorGun.delay&&cannon.shotCnt==priorGun.shotCnt);
 const auto& missile=data.weapons.at("Ka52_Missile");const auto& priorMissile=original.weapons.at("E_Wpn_ASM");
 assert(missile.dmg==priorMissile.dmg&&missile.spd==priorMissile.spd&&missile.delay==priorMissile.delay&&missile.shotCnt==priorMissile.shotCnt);
 const auto& lux=data.weapons.at("Lux_Euro_Missile");const auto& priorLux=data.weapons.at("Lux_F22_Missile");
 assert(lux.dmg==priorLux.dmg&&lux.spd==priorLux.spd&&lux.delay==priorLux.delay&&lux.shotCnt==priorLux.shotCnt&&lux.shotDly==priorLux.shotDly&&lux.aiTimeScale==priorLux.aiTimeScale);
 for(const auto& [name,map]:data.maps)for(const auto& e:map.enemies)assert(e.name!="Ka52"&&e.name!="Lux_Eurofighter");
 for(const auto& [name,wave]:data.formations)for(const auto& e:wave.units)assert(e.machine!="Ka52"&&e.machine!="Lux_Eurofighter");
 const std::array<std::string,7> files{{"Ka52_Body.png","Ka52_RotorUpper.png","Ka52_RotorLower.png","Ka52_Cannon.png","Ka52_Missile.png","Lux_Eurofighter.png","Lux_Euro_Missile.png"}};
 for(const auto& file:files)assert(std::filesystem::exists(root/"Resources/RuntimeImages/apple"/file));
 auto dims=[&](std::string_view key){return contractTileSize(root,key);};
 for(const auto& key:{"Ka52_RotorUpper","Ka52_RotorLower"}){
  auto& p=data.parts.at(key);assert(p.partsType==3&&p.tileCnt==2&&!p.presentation);
  PartsState state;state.info=&p;std::set<int> frames;
  for(int tick=0;tick<60;++tick){auto d=drawParts(state,{},0,0,2,0,double(tick)/60,0);frames.insert(d.frame);assert(d.cols==2&&d.tile==std::string(key)+".png");}
  assert((frames==std::set<int>{0,1}));
 }
 for(const std::string name:{"Ka52","Lux_Eurofighter"}){
  SaveSlot slot;GlobalInfo(slot).selectSlot(0);slot.setInt(0x28,50);
  uint32_t seed=5;GameSession game(data,slot,dims,[&]{seed=seed*1664525+1013904223;return int32_t(seed>>1);});
  game.start(0,1,1);auto& i=*game.impl_;i.enemies.clear();i.world.enemiesSpawned.assign(i.world.map.enemies.size(),true);i.world.gameState=0;i.world.paused=false;i.player->hp=100000;
  auto start=i.player->position;i.spawnEnemy({name,{start.x+160,start.y+100},false});auto& m=i.enemies.back();
  assert(m.parts.size()==(name=="Ka52"?5:3));assert(m.enemy->hasWeapon);
  std::set<std::string> wanted=name=="Ka52"?std::set<std::string>{"Ka52_Cannon","Ka52_Missile"}:std::set<std::string>{"Lux_Euro_Missile"};
  std::set<std::string> fired,drawn;std::set<int> upperFrames,lowerFrames;
#if defined(HM_VALIDATION) && HM_VALIDATION
  std::set<std::string> impacting;
  game.validationObserveDamage([&](uintptr_t target,std::string_view weapon,int32_t amount,int32_t before,int32_t after){
   if(target==1&&wanted.contains(std::string(weapon))){assert(before-after==amount);impacting.insert(std::string(weapon));}
  });
#endif
  const auto begin=m.enemy->position;bool moved=false;int persisted=0,gaugeChecks=0;
  for(int n=1;n<=1200;++n){
   game.tick(double(n)/60,0);
   moved|=std::abs(m.enemy->position.x-begin.x)>1||std::abs(m.enemy->position.y-begin.y)>1;
   for(const auto& node:i.dynamic)if(auto b=std::get_if<BulletState>(&node.value))if(b->toPlayer&&b->info&&wanted.contains(b->info->name))fired.insert(b->info->name);
#if defined(HM_VALIDATION) && HM_VALIDATION
   const auto stateBefore=seed;const auto hpBefore=i.player->hp;const auto countBefore=i.dynamic.size();
   const auto observed=game.validationBullets();
   assert(seed==stateBefore&&i.player->hp==hpBefore&&i.dynamic.size()==countBefore);
   for(const auto& bullet:observed)assert(!bullet.weapon.empty());
#endif
   for(const auto& d:game.sprites()){
    drawn.insert(d.tile);
    if(d.tile=="Ka52_RotorUpper.png")upperFrames.insert(d.frame);
    if(d.tile=="Ka52_RotorLower.png")lowerFrames.insert(d.frame);
   }
   if(name=="Ka52")for(const auto& gauge:game.sprites())if(gauge.tile=="HPGaugeEnemy.png"){
    const auto g=drawnBounds(gauge,dims(gauge.tile));
    for(const auto& part:game.sprites())if(part.tile=="Ka52_Body.png"||part.tile=="Ka52_Cannon.png"||part.tile=="Ka52_Missile.png"){
     const auto p=drawnBounds(part,dims(part.tile));
     const bool separate=g.right<=p.left||g.left>=p.right||g.top<=p.bottom||g.bottom>=p.top;
     if(!separate)std::cerr<<"Gauge overlap tick="<<n<<" part="<<part.tile<<" gauge="<<g.left<<','<<g.bottom<<','<<g.right<<','<<g.top<<" part="<<p.left<<','<<p.bottom<<','<<p.right<<','<<p.top<<" angle="<<part.rotation<<'\n';
     assert(separate);++gaugeChecks;
    }
   }
   for(const auto& event:game.takeEvents())if(event.kind==SessionEventKind::PersistSave)++persisted;
  }
  std::cout<<name<<" fired="<<fired.size()<<" moved="<<moved<<" playerHP="<<i.player->hp<<" persisted="<<persisted<<'\n';
  assert(fired==wanted&&moved&&i.player->hp<100000&&persisted==0);
#if defined(HM_VALIDATION) && HM_VALIDATION
  assert(impacting==wanted);game.validationObserveDamage({});std::cout<<name<<" attributed real weapon impacts="<<impacting.size()<<" PASS\n";
#endif
  for(auto& key:wanted)assert(drawn.contains(key+".png"));
  assert(drawn.contains(name=="Ka52"?"Ka52_Body.png":"Lux_Eurofighter.png"));
  if(name=="Ka52"){assert((upperFrames==std::set<int>{0,1})&&(lowerFrames==std::set<int>{0,1})&&gaugeChecks>0);std::cout<<"Ka52 gauge/body/weapon nonoverlap checks="<<gaugeChecks<<'\n';}
  const auto beforePause=m.enemy->position;game.setPaused(true);game.tick(21,0);assert(near(m.enemy->position.x,beforePause.x)&&near(m.enemy->position.y,beforePause.y));game.setPaused(false);
  // Muzzle must follow the visible weapon's real socket and common rotation.
  for(float angle:{-.18f,0.f,.18f})for(auto& p:m.parts)if(p->armed){
   m.enemy->rotation=angle;p->weapon.aimDir=2.9f;p->weapon.aimDirDest=3.05f;p->weapon.fireOffset={};
   auto d=drawWeapon(p->visual,p->weapon,m.enemy->position,false,1,angle,0,1);
   auto frame=i.frame(*m.enemy,m,*p);refreshWeaponMuzzle(p->weapon,frame);
   const auto muzzle=rotateAttachment(p->visual.info->sockets[1],d.rotation);
   assert(near(frame.weaponPos.x+p->weapon.muzzlePos.x,d.position.x+muzzle.x));
   assert(near(frame.weaponPos.y+p->weapon.muzzlePos.y,d.position.y+muzzle.y));
  }
  // Every existing player weapon uses real collision/damage against each new
  // target. Starting inside its hitbox isolates damage handling from aiming.
  m.enemy->position={i.player->position.x+150,i.player->position.y+100};m.inScreen=true;int hits=0;
  for(const auto& [key,weapon]:data.weapons)if(key.starts_with("P_")&&data.parts.contains(key)){
   m.enemy->hp=100000;BulletState b;b.info=&weapon;b.pos={m.enemy->position.x+(m.bounds.left+m.bounds.right)*.5f,m.enemy->position.y+(m.bounds.bottom+m.bounds.top)*.5f};b.toPlayer=false;b.startTime=0;
   CombatFrame frame;frame.time=1;frame.weaponUpgrade=slot.bytes[0x2d];frame.playerPos=i.player->position;
   tickBullet(b,frame,i);if(m.enemy->hp!=100000-weapon.dmg-(weapon.dmg/8)*frame.weaponUpgrade)std::cerr<<name<<" hit failure "<<key<<" hp="<<m.enemy->hp<<" pos="<<b.pos.x<<','<<b.pos.y<<" bounds="<<m.bounds.left<<','<<m.bounds.bottom<<','<<m.bounds.right<<','<<m.bounds.top<<" camera="<<i.world.camera.x<<','<<i.world.camera.y<<'\n';assert(m.enemy->hp==100000-weapon.dmg-(weapon.dmg/8)*frame.weaponUpgrade);++hits;
  }
  assert(hits==20);i.damage(m.id,m.enemy->hp);assert(m.removed&&m.enemy->hp==0);
  std::cout<<name<<" native parts/weapons/muzzle/animation/pause + 20 player damage cases + destruction PASS\n";
 }
}
