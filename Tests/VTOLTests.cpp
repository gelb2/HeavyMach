#include "World.hpp"
#include "GroundAA.hpp"
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
int main(){
 const auto root=std::filesystem::path(__FILE__).parent_path().parent_path();auto original=GameData::loadDirectory(root/"Resources/RawData"),data=original;addVTOLExpansion(data);
 size_t added=0,originalEntries=0;int changed=0;
 for(auto& [key,map]:original.maps){auto& after=data.maps.at(key);assert(after.enemies.size()>=map.enemies.size());originalEntries+=map.enemies.size();
  for(size_t n=0;n<map.enemies.size();++n){auto& a=map.enemies[n];auto& b=after.enemies[n];assert(a.name==b.name&&a.position.x==b.position.x&&a.position.y==b.position.y&&a.isBoss==b.isBoss);}
  if(after.enemies.size()>map.enemies.size())++changed;
  for(size_t n=map.enemies.size();n<after.enemies.size();++n){auto& e=after.enemies[n];assert(vtolSpec(e.name)&&!e.isBoss);const float margin=std::min(300.f,(map.right-map.left)*.15f);assert(e.position.x>map.left+margin&&e.position.x<map.right-margin);for(auto& old:map.enemies)if(old.isBoss)assert(std::abs(old.position.x-e.position.x)>=600);++added;}
 }
 assert(added==152&&changed==35);
 // Independent ground-AA planner supplies the requested per-stage baseline.
 std::array<int,3> totals{};std::array<std::array<int,3>,7> missions{};
 for(const auto& row:groundAAPlan(original)){
  std::array<int,3> counts{};std::vector<float> xs;
  for(const auto& e:data.maps.at(row.map).enemies)for(size_t k=0;k<3;++k)if(e.name==vtolSpecs()[k].name){++counts[k];++totals[k];++missions[row.mission][k];xs.push_back(e.position.x);}
  assert(counts[0]+counts[1]+counts[2]==row.shilka+row.tunguska);
  assert(*std::max_element(counts.begin(),counts.end())-*std::min_element(counts.begin(),counts.end())<=1);
  std::sort(xs.begin(),xs.end());for(size_t n=1;n<xs.size();++n)assert(xs[n]-xs[n-1]>=100);
  // Exercise actual World regeneration at original and expanded viewport widths.
  for(float width:{480.f,696.f}){
   struct SpawnHost:WorldHost{int aircraft=0;Vector2 tileSize(std::string_view)override{return {480,320};}void spawnMapObject(const MapObjectDefinition&,int)override{}void spawnEnemy(const EnemyDefinition&e)override{if(vtolSpec(e.name))++aircraft;}void spawnPlayer(Vector2)override{}void event(const WorldEvent&)override{}} host;
   WorldState world;world.setCanvasWidth(width);world.openMap(data.maps.at(row.map),0,host);
   for(float x=world.map.left;x<=world.map.right+500;x+=10)world.processRegen(x,host);
   assert(host.aircraft==row.shilka+row.tunguska);
   world.processRegen(world.map.right+1000,host);assert(host.aircraft==row.shilka+row.tunguska);
  }
  std::cout<<row.mission+1<<','<<row.stage+1<<','<<row.map<<','<<row.shilka<<','<<row.tunguska<<','<<counts[0]<<','<<counts[1]<<','<<counts[2]<<'\n';
 }
 for(const auto& counts:missions){assert(*std::min_element(counts.begin(),counts.end())>0);assert(*std::max_element(counts.begin(),counts.end())-*std::min_element(counts.begin(),counts.end())<=1);}
 assert((totals==std::array<int,3>{51,51,50}));
 // All platform loaders use VTOL -> fighters -> ground AA. Preserve that output.
 auto full=data;addFighterExpansion(full);addGroundAAExpansion(full);
 auto baseline=original;addFighterExpansion(baseline);addGroundAAExpansion(baseline);
 for(const auto& [key,map]:baseline.maps){std::vector<EnemyDefinition> actual;for(const auto&e:full.maps.at(key).enemies)if(!vtolSpec(e.name))actual.push_back(e);assert(actual.size()==map.enemies.size());for(size_t n=0;n<actual.size();++n){auto&a=actual[n];auto&b=map.enemies[n];assert(a.name==b.name&&a.position.x==b.position.x&&a.position.y==b.position.y&&a.isBoss==b.isBoss);}}
addVTOLExpansion(data);size_t total=0;for(auto& [key,m]:data.maps)total+=m.enemies.size();assert(total==originalEntries+152);
 auto dims=[&](std::string_view name){auto path=name.starts_with("Extensions/")?root/"Art"/name:root/"Resources/Images"/name;std::ifstream f(path,std::ios::binary);unsigned char b[24]{};f.read((char*)b,24);if(f.gcount()!=24)return Vector2{40,12};auto be=[&](int n){return uint32_t(b[n])<<24|uint32_t(b[n+1])<<16|uint32_t(b[n+2])<<8|b[n+3];};return Vector2{float(be(16)),float(be(20))};};
 for(auto& spec:vtolSpecs()){
  assert(spec.width>dims("E_Heli_Body_01.png").x&&spec.width<dims("E_Heli_Body_06.png").x);
  assert(data.machines.at(spec.name).hpMax==original.machines.at(spec.statsSource).hpMax);
  SaveSlot slot;GlobalInfo(slot).selectSlot(0);slot.setInt(0x28,50);uint32_t seed=5;auto rng=[&]{seed=seed*1664525+1013904223;return int32_t(seed>>1);};GameSession game(data,slot,dims,rng);game.start(0,1,1);auto& i=*game.impl_;i.enemies.clear();i.world.enemiesSpawned.assign(i.world.map.enemies.size(),true);i.world.gameState=0;i.world.paused=false;i.player->hp=100000;
  auto pos=i.player->position;auto seedBefore=seed;i.spawnEnemy({spec.name,{pos.x+160,pos.y+100},false});auto& m=i.enemies.back();assert(seed==seedBefore);assert(m.parts.size()==2&&m.parts[1]->weapon.weaponInfo==&data.weapons.at(spec.weapon));
  bool fired=false,moved=false,pitched=false,drawn=false;auto begin=m.enemy->position;int persisted=0;
  for(int n=1;n<=1200;++n){game.tick(double(n)/60,0);for(auto& node:i.dynamic)if(auto b=std::get_if<BulletState>(&node.value))if(b->info==&data.weapons.at(spec.weapon)&&b->toPlayer)fired=true;
   moved|=std::abs(m.enemy->position.x-begin.x)>1;pitched|=std::abs(m.enemy->rotation)>.01f;
   for(auto& d:game.sprites())if(d.tile==spec.tile){drawn=true;assert(std::abs(d.scale.x*dims(spec.tile).x-spec.width)<.01f);}
   for(auto& e:game.takeEvents())if(e.kind==SessionEventKind::PersistSave)++persisted;
  }
  assert(fired&&moved&&pitched&&drawn&&persisted==0);
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
 std::cout<<"Preserved "<<originalEntries<<" original entries; 152 additions across 35 maps PASS\n";
}
