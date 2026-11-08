#include "World.hpp"
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
#include "RuntimeUnitTestSupport.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
using namespace heavymach;
struct TerrainHost:WorldHost {Vector2 tileSize(std::string_view)override{return {480,320};}void spawnMapObject(const MapObjectDefinition&,int)override{}void spawnEnemy(const EnemyDefinition&)override{}void spawnPlayer(Vector2)override{}void event(const WorldEvent&)override{}};
int main(){const auto root=std::filesystem::path(__FILE__).parent_path().parent_path();auto data=GameData::loadDirectory(root/"Resources/RuntimeData",true);int landed=0;
 for(const auto& row:groundAAPlan(data)){TerrainHost host;WorldState world;world.openMap(data.maps.at(row.map),0,host);
  for(const auto& spawn:world.map.enemies)if(isGroundAA(spawn.name)){
   GobHvMach enemy;enemy.machType=0;enemy.machState=2;enemy.position=spawn.position;enemy.useEngineBounce=true;
   for(int n=0;n<120;++n){world.gameTime=(n+1)/60.;auto events=enemy.tick(world.motionWorld(enemy.position.x,world.gameTime,[]{return 0;}));for(auto& event:events){if(event.kind==MotionEventKind::Damage)std::cerr<<row.map<<" spawn="<<spawn.position.x<<","<<spawn.position.y<<" falls to="<<enemy.position.x<<","<<enemy.position.y<<"\n";assert(event.kind!=MotionEventKind::Damage);}}
   if(enemy.machState!=1)std::cerr<<row.map<<" "<<spawn.name<<" failed landing "<<spawn.position.x<<','<<spawn.position.y<<'\n';assert(enemy.machState==1);const auto& contact=world.ground.at(size_t(enemy.currentCol)).collision;Vector2 relative{enemy.position.x-contact.origin.x,enemy.position.y-contact.origin.y};assert(std::abs(contact.normal.dotProduct(relative))<1);++landed;
   for(float direction:{-.6f,.6f}){enemy.speed=direction;for(int n=0;n<90;++n){world.gameTime+=1./60;enemy.tick(world.motionWorld(enemy.position.x,world.gameTime,[]{return 0;}));assert(std::isfinite(enemy.position.x)&&std::isfinite(enemy.rotation));assert(enemy.position.y>world.map.bottom);}}
  }
 }
 std::cout<<"all "<<landed<<" authored new spawn points landed and moved left/right on original terrain\n";
 auto dims=[&](std::string_view name){return contractTileSize(root,name);};
 // Actual session collision/damage path, all twenty player weapons, both units.
 // Flat fixture isolates hit registration from terrain interception and aiming.
 auto& map=data.maps.at("Stage01");map.enemies.clear();map.objects.clear();map.left=0;map.right=3000;map.bottom=-200;map.collisionPaths={{{0,0},{3000,0}}};
 // Compare actual movement/draw offsets with original ground enemies.
 for(const char* unit:{"Tank001","Tank030","E0505","GAA_Shilka_M1","GAA_Tunguska_M1"})for(float speed:{-.6f,.6f}){
  SaveSlot save;GlobalInfo(save).selectSlot(0);GameSession game(data,save,dims,[]{return 1;});game.setCanvasWidth(900);game.start(0,0,0);auto& i=*game.impl_;i.enemies.clear();i.spawnEnemy({unit,{400,4},false});auto& m=i.enemies.back();
  for(int n=0;n<60;++n){i.world.gameTime+=1./60;m.enemy->GobHvMach::tick(i.world.motionWorld(m.enemy->position.x,i.world.gameTime,[]{return 1;}));}
  float low=100,high=-100,bodyLow=100,bodyHigh=-100;std::set<int> bodyFrames;float start=m.enemy->position.x;
  for(int n=0;n<120;++n){i.world.gameTime+=1./60;i.globalTime=i.world.gameTime;m.enemy->speed=speed;m.enemy->GobHvMach::tick(i.world.motionWorld(m.enemy->position.x,i.world.gameTime,[]{return 1;}));i.collect();low=std::min(low,m.enemy->engineBounce);high=std::max(high,m.enemy->engineBounce);
   for(auto& p:m.parts)if(p->visual.info->partsType==0){auto d=drawParts(p->visual,m.enemy->position,m.enemy->rotation,m.enemy->engineBounce,m.enemy->machState,m.enemy->speed,i.globalTime);float relative=d.position.y-m.enemy->position.y;bodyLow=std::min(bodyLow,relative);bodyHigh=std::max(bodyHigh,relative);bodyFrames.insert(p->visual.tileFrame);}
  }
  std::cout<<"motion,"<<unit<<","<<speed<<",distance="<<m.enemy->position.x-start<<",bounce="<<low<<":"<<high<<",body_y="<<bodyLow<<":"<<bodyHigh<<",frames="<<bodyFrames.size()<<'\n';
  if(isGroundAA(unit)){assert(high-low>1.3f);assert(bodyHigh-bodyLow>1.3f);assert(bodyFrames.size()==2);}
 }
 std::vector<std::string> weapons;for(const auto& [name,w]:data.weapons)if(name.starts_with("P_")&&data.parts.contains(name))weapons.push_back(name);std::sort(weapons.begin(),weapons.end());
 int hits=0;
 for(const char* unit:{"GAA_Shilka_M1","GAA_Tunguska_M1"})for(const auto& name:weapons){
  SaveSlot save;GlobalInfo(save).selectSlot(0);uint32_t rng=123;GameSession game(data,save,dims,[&]{rng=rng*1664525+1013904223;return int32_t(rng>>1);});game.setCanvasWidth(900);game.start(0,0,0);auto& i=*game.impl_;i.player->position={100,0};i.enemies.clear();i.world.enemiesSpawned.assign(i.world.map.enemies.size(),true);i.spawnEnemy({unit,{300,0},false});i.world.gameTime=1;i.globalTime=1;i.collect();auto& enemy=i.enemies.back();int hp=enemy.enemy->hp;
  auto& w=data.weapons.at(name);i.spawnBullet(w,false,0,{300,20},1);
  for(int tick=0;tick<30&&!enemy.removed&&enemy.enemy->hp==hp;++tick){i.world.gameTime+=1./60;for(auto& node:i.dynamic)if(auto bullet=std::get_if<BulletState>(&node.value)){CombatFrame frame;frame.time=i.world.gameTime;frame.weaponUpgrade=0;tickBullet(*bullet,frame,i);}}
  if(enemy.enemy->hp>=hp)std::cerr<<unit<<' '<<name<<" no hit\n";assert(enemy.enemy->hp<hp);++hits;
  i.damage(enemy.id,enemy.enemy->hp);assert(enemy.enemy->hp==0&&enemy.removed);
 }
 assert(weapons.size()==20);std::cout<<hits<<" player weapon/unit collision cases and destruction passed\n";
}
