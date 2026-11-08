#include "World.hpp"
#include "RuntimeUnitTestSupport.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "ItemBox.hpp"
#include "Trail.hpp"
#include <variant>
#include <memory>
#define private public
#include "GameSession.hpp"
#undef private
#include "../Sources/Core/GameSession.cpp"
#include <cassert>
#include <fstream>
#include <iostream>
using namespace heavymach;
struct BurstHost:CombatHost {
 std::vector<double> times;int randomCalls=0;
 int32_t random31()override{++randomCalls;return 0;}
 bool sweepTerrain(Vector2&,const Vector2&)override{return false;}
 bool inCamera(const BulletState&)override{return true;}
 std::optional<CombatTarget> targetAt(bool,Vector2)override{return {};}
 void damage(CombatTarget,int32_t)override{}
 bool acquireParticle()override{return false;}
 void event(const CombatEvent&)override{}
 void spawnBullet(const WeaponInfo&,bool,float,Vector2,double t)override{times.push_back(t);}
};
int main(){
 const auto root=std::filesystem::path(__FILE__).parent_path().parent_path();auto original=GameData::loadDirectory(root/"Resources/RawData"),data=GameData::loadDirectory(root/"Resources/RuntimeData",true);
 // Definitions are additive; original weapons and all authored spawn entries stay intact.
 for(const auto& [key,map]:original.maps){const auto& after=data.maps.at(key);assert(after.enemies.size()>=map.enemies.size());for(size_t n=0;n<map.enemies.size();++n){const auto&a=map.enemies[n];const auto&b=after.enemies[n];assert(a.name==b.name&&a.isBoss==b.isBoss&&a.position.x==b.position.x&&a.position.y==b.position.y);}for(const auto&e:after.enemies)assert(!e.name.starts_with("Lux_"));}
 for(const auto& [key,w]:original.weapons){const auto& n=data.weapons.at(key);assert(w.spd==n.spd&&w.shotCnt==n.shotCnt&&w.shotDly==n.shotDly&&w.delay==n.delay&&n.aiTimeScale==1&&n.burstGroupSize==0);}
 assert(data.formations.size()==14);
 for(size_t n=0;n<3;++n){const auto& spec=fighterSpecs()[n];const auto& w=data.weapons.at(spec.weapon);assert(w.spd==200&&std::abs(w.delay-5.f/1.1f)<.00001f&&w.shotCnt==(n==2?6:3));
  const auto& prior=original.weapons.at(n==2?"E_0604_Wpn":"E_Wpn_ASM");assert(w.dmg==prior.dmg&&w.g==prior.g&&w.liveTm==prior.liveTm&&w.aimRand==prior.aimRand&&w.spdRand==prior.spdRand);
  BurstHost host;WeaponState state;state.weaponInfo=&w;CombatFrame f;f.machType=3;f.isPlayer=false;f.isMainPlayer=false;f.time=1;
  assert(fireWeapon(state,f,host));for(int tick=61;tick<300;++tick){f.time=double(tick)/60;tickWeapon(state,f,host);}
  assert(host.times.size()==w.shotCnt);for(size_t i=1;i<host.times.size();++i){double expected=n==2&&i%2==0?1./1.1:.5/1.1;double delta=host.times[i]-host.times[i-1];assert(delta>=expected-.001&&delta<=expected+1./60+.001);}
  // Pausing mid-burst does not dispatch or advance the weapon state.
  state={};state.weaponInfo=&w;f.time=10;fireWeapon(state,f,host);int remaining=state.fireCount;f.paused=true;f.time=99;tickWeapon(state,f,host);assert(state.fireCount==remaining);f.paused=false;
  // The three AI delay phases scale without touching original definitions.
  state={};state.weaponInfo=&w;f.machType=1;f.ownerPos={200,0};f.weaponPos=f.ownerPos;f.time=1;processWeaponAI(state,f,host);assert(std::abs(state.delayTime-.3f/1.1f)<.00001f);
  f.time=2;processWeaponAI(state,f,host);assert(std::abs(state.delayTime-.05f/1.1f)<.00001f);
  f.time=3;processWeaponAI(state,f,host);assert(std::abs(state.delayTime-.5f/1.1f)<.00001f);
 }
 auto dims=[&](std::string_view name){return contractTileSize(root,name);};
 // All fourteen real map triggers; wide prefetch and pause must not emit a wave.
 int totalWaves=0,totalJets=0;
 for(int mission=0;mission<7;++mission)for(int stage=0;stage<GlobalInfo::stageCount(mission);++stage){char key[32];snprintf(key,sizeof(key),"Stage%02d",GlobalInfo::mapNumber({mission,stage}));auto wave=fighterWave(key);if(!wave)continue;
  for(auto canvas:{Vector2{480,320},Vector2{480,360},Vector2{695,320},Vector2{569,320},Vector2{900,320}}){float width=canvas.x;for(float jump:{0.f,200.f}){SaveSlot save;GlobalInfo(save).selectSlot(0);uint32_t seed=9;GameSession game(data,save,dims,[&]{seed=seed*1664525+1013904223;return int32_t(seed>>1);});game.setCanvasWidth(width);game.start(0,mission,stage);auto& i=*game.impl_;i.world.canvasHeight=canvas.y;
   auto jets=[&]{int n=0;for(auto& e:i.enemies)if(e.definition->name.starts_with("Lux_"))++n;return n;};assert(jets()==0);
   float trigger=i.world.map.right-800;if(wave->boss[0]){auto boss=std::find_if(i.world.map.enemies.begin(),i.world.map.enemies.end(),[&](const auto& e){return e.name==wave->boss;});assert(boss!=i.world.map.enemies.end());trigger=boss->position.x-490;}
   i.player->position.x=trigger-40;i.world.processRegen(i.player->position.x,i);
   // Reaching this section requires defeating any earlier boss arena.
   for(size_t n=0;n<i.world.map.enemies.size();++n)if(i.world.bossEncounterActivated[n]&&!i.world.mapEnemiesDefeated[n]&&i.world.map.enemies[n].name!=wave->boss)i.world.killMapBoss(n);
   i.spawnReinforcements();assert(jets()==0);
   i.player->position.x=trigger;i.world.processRegen(trigger,i);i.world.paused=true;i.spawnReinforcements();assert(jets()==0);i.world.paused=false;
   float playerGround=i.world.mapHeight(trigger);i.player->position.y=(playerGround==10000?i.world.map.bottom:playerGround)+30+jump;
   for(int c=0;c<60;++c)i.world.processCamera(i.player->position);
   int bossCount=i.world.bossCount;auto seedBefore=seed;i.spawnReinforcements();assert(seed==seedBefore);int expected=0;for(int n:wave->counts)expected+=n;assert(jets()==expected&&i.world.bossCount==bossCount);
   // Simultaneous flight catches the old convergence on one shared endpoint.
   std::vector<GameSession::Impl::Machine*> formation;
   for(auto& e:i.enemies)if(e.definition->name.starts_with("Lux_"))formation.push_back(&e);
   std::vector<Vector2> starts;for(auto* e:formation)starts.push_back(e->enemy->position);
   auto separated=[&]{for(size_t a=0;a<formation.size();++a){auto& e=*formation[a];
     assert(e.enemy->position.y+aircraftSpec(e.definition->name)->width*aircraftSpec(e.definition->name)->aspect*.5f<i.world.map.top);
     auto ground=i.world.mapHeight(e.enemy->position.x);assert(ground==10000||e.enemy->position.y-aircraftSpec(e.definition->name)->width*aircraftSpec(e.definition->name)->aspect*.5f>ground);
     for(size_t b=0;b<a;++b){auto& other=*formation[b];
      float dx=std::abs(e.enemy->position.x-other.enemy->position.x),dy=std::abs(e.enemy->position.y-other.enemy->position.y);
      assert(dx>=(aircraftSpec(e.definition->name)->width+aircraftSpec(other.definition->name)->width)*.5f+10 || dy>=(aircraftSpec(e.definition->name)->width*aircraftSpec(e.definition->name)->aspect+aircraftSpec(other.definition->name)->width*aircraftSpec(other.definition->name)->aspect)*.5f+10);
     }
   }};
   separated();int entryTicks=0;
   while(formation.front()->formationEntering&&entryTicks++<1000){for(auto* e:formation)i.advanceFormationEntry(*e);separated();
    for(auto* e:formation)assert(e->formationEntering==formation.front()->formationEntering);
   }
   assert(entryTicks<1000);
   for(size_t n=0;n<formation.size();++n){formation[n]->enemy->position=starts[n];formation[n]->formationEntering=true;}
   for(auto& e:i.enemies)if(e.definition->name.starts_with("Lux_")){assert(!e.enemy->isBoss&&e.enemy->hp==765);assert(e.enemy->position.x-aircraftSpec(e.definition->name)->width*.5f>i.world.camera.x+width*.5f);
    assert(e.formationEntering);auto start=e.enemy->position;auto beforeRng=seed;
    i.world.paused=true;i.advanceFormationEntry(e);assert(e.enemy->position.x==start.x&&e.formationEntering);i.world.paused=false;
    int steps=0;while(e.formationEntering&&steps++<1000){float before=e.enemy->position.x;i.advanceFormationEntry(e);assert(before-e.enemy->position.x>0&&before-e.enemy->position.x<=2.501f);}
    assert(!e.formationEntering&&steps<1000&&seed==beforeRng);assert(e.enemy->position.x<=e.formationEndX);
    assert(e.enemy->position.y<i.world.map.top);auto ground=i.world.mapHeight(e.enemy->position.x);assert(ground==10000||e.enemy->position.y>ground+10);}
   game.setCanvasWidth(width==480?900:480);i.spawnReinforcements();i.world.paused=true;i.world.paused=false;i.spawnReinforcements();assert(jets()==expected);
   game.start(0,mission,stage);assert(!i.reinforcementSpawned&&jets()==0);
   if(width==480&&canvas.y==320&&jump==0){++totalWaves;totalJets+=expected;}
  }}
 }
 {SaveSlot save;GlobalInfo(save).selectSlot(0);GameSession game(data,save,dims,[]{return 0;});game.start(1,0,0);auto& i=*game.impl_;assert(!i.reinforcement);i.player->position.x=i.world.map.right;i.spawnReinforcements();assert(!i.reinforcementSpawned);}
 assert(totalWaves==14&&totalJets==72);std::cout<<"14 wave triggers, 72 additions; original map entries preserved PASS\n";
 for(auto& spec:fighterSpecs()){
  assert(spec.width>dims("E_Heli_Body_01.png").x&&spec.width<dims("E_Heli_Body_06.png").x);
  assert(data.machines.at(spec.name).hpMax==765);
  SaveSlot slot;GlobalInfo(slot).selectSlot(0);slot.setInt(0x28,50);uint32_t seed=5;auto rng=[&]{seed=seed*1664525+1013904223;return int32_t(seed>>1);};GameSession game(data,slot,dims,rng);game.start(0,1,1);auto& i=*game.impl_;i.enemies.clear();i.world.enemiesSpawned.assign(i.world.map.enemies.size(),true);i.world.gameState=0;i.world.paused=false;i.player->hp=100000;
  auto pos=i.player->position;auto seedBefore=seed;i.spawnEnemy({spec.name,{pos.x+160,pos.y+100},false});auto& m=i.enemies.back();assert(seed==seedBefore);assert(m.parts.size()==3&&m.parts[1]->weapon.weaponInfo==&data.weapons.at(spec.weapon));
  for(int c=0;c<60;++c)i.world.processCamera(i.player->position);m.formationEntering=true;m.formationEndX=pos.x+160;m.enemy->position.x=std::max(m.formationEndX+200,i.world.camera.x+i.world.canvasWidth*.5f+80);
  bool fired=false,moved=false,pitched=false,drawn=false;auto begin=m.enemy->position;int persisted=0;
  for(int n=1;n<=1200;++n){game.tick(double(n)/60,0);for(auto& node:i.dynamic)if(auto b=std::get_if<BulletState>(&node.value))if(b->info==&data.weapons.at(spec.weapon)&&b->toPlayer){assert(!m.formationEntering);fired=true;}
   moved|=std::abs(m.enemy->position.x-begin.x)>1;pitched|=std::abs(m.enemy->rotation)>.01f;
   for(auto& d:game.sprites())if(d.tile==spec.tile){drawn=true;assert(std::abs(d.scale.x*dims(spec.tile).x-spec.width)<.01f);}
   for(auto& e:game.takeEvents())if(e.kind==SessionEventKind::PersistSave)++persisted;
  }
  std::cerr<<spec.name<<" fired="<<fired<<" moved="<<moved<<" pitched="<<pitched<<" drawn="<<drawn<<" saves="<<persisted<<" entry="<<m.formationEntering<<" x="<<m.enemy->position.x<<" end="<<m.formationEndX<<" player="<<i.player->position.x<<" hp="<<m.enemy->hp<<"\n";assert(fired&&moved&&pitched&&drawn&&persisted==0&&!m.formationEntering);
  // Exactly one aft exhaust remains; rotation/paused animation uses the same draw path.
  m.inScreen=true;m.enemy->visible=true;i.draws.clear();i.render.clear();i.machineDraw(*m.enemy,m);
  size_t flames=0;for(auto& d:i.draws)if(d.tile=="BulletEffect2.png")++flames;assert(flames==1);
  assert(spec.bounds.right-spec.bounds.left<spec.width);

  auto oldRot=m.enemy->rotation;game.setPaused(true);game.tick(30,0);assert(m.enemy->rotation==oldRot);
  for(float rot:{-.18f,0.f,.18f}){m.enemy->rotation=rot;auto r=vtolRotate(spec.muzzle,rot);m.parts[1]->visual.rotatedPosition=r;auto frame=i.frame(*m.enemy,m,*m.parts[1]);assert(std::abs(frame.weaponPos.x-m.enemy->position.x-r.x)<.001f&&std::abs(frame.weaponPos.y-m.enemy->position.y-r.y)<.001f);
   frame.paused=false;frame.time=40;auto& weapon=m.parts[1]->weapon;weapon.fireTime=39;weapon.fireDelay=0;weapon.fireCount=1;weapon.muzzlePos={};i.current=&m;tickWeapon(weapon,frame,i);i.current=nullptr;
   auto firedBullet=std::get_if<BulletState>(&i.dynamic.back().value);assert(firedBullet&&std::abs(firedBullet->pos.x-frame.weaponPos.x)<.001f&&std::abs(firedBullet->pos.y-frame.weaponPos.y)<.001f);}
  // Every original player weapon reaches the new target through actual bullet
  // collision/damage; projectile starts inside the hitbox to isolate hit handling.
  int weaponHits=0;m.enemy->position={i.player->position.x+150,i.player->position.y+100};m.inScreen=true;
  for(auto& [name,weapon]:data.weapons)if(name.starts_with("P_")&&data.parts.contains(name)){
   m.enemy->hp=100000;BulletState b;b.info=&weapon;b.pos=m.enemy->position;b.toPlayer=false;b.startTime=0;
   CombatFrame f;f.time=1;f.weaponUpgrade=slot.bytes[0x2d];f.playerPos=i.player->position;
   tickBullet(b,f,i);assert(m.enemy->hp==100000-weapon.dmg-(weapon.dmg/8)*f.weaponUpgrade);++weaponHits;
  }
  assert(weaponHits==20);std::cout<<spec.name<<" player weapon hit cases="<<weaponHits<<" PASS\n";
  i.damage(m.id,m.enemy->hp);assert(m.removed&&m.enemy->hp==0);std::cout<<spec.name<<" fire/move/pitch/render/pause/damage/save PASS\n";
 }
 assert(vtolPitch(0,-.5f,0)>0&&vtolPitch(0,.5f,0)<0&&vtolPitch(0,0,20)<0&&vtolPitch(0,0,-20)>0);

}
