// Independent before/after observer. Compile this SAME file against both trees.
// Private access is limited to deterministic fixture setup and read-only state
// observation; no copied AI, collision, weapon or drawing implementation.
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
#include "GameSession.cpp"
#ifdef HM_UNIT_CONTRACT_BASELINE
#include "GroundAA.hpp"
#endif
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
using namespace heavymach;
static std::filesystem::path root;
static std::map<std::string,Vector2> dimensions;
static Vector2 pngSize(std::filesystem::path path){
 std::ifstream f(path,std::ios::binary);unsigned char b[24]{};f.read((char*)b,24);
 if(f.gcount()!=24||b[0]!=137||b[1]!='P')throw std::runtime_error("Missing PNG dimensions: "+path.string());
 auto be=[&](int n){return uint32_t(b[n])<<24|uint32_t(b[n+1])<<16|uint32_t(b[n+2])<<8|b[n+3];};
 return {float(be(16)),float(be(20))};
}
static Vector2 sizeFor(std::string_view view){
 std::string name(view);if(auto it=dimensions.find(name);it!=dimensions.end())return it->second;
 Vector2 size{};
#ifdef HM_UNIT_CONTRACT_BASELINE
 if(auto art=groundAAImage(name)){dimensions[name]=art->size;return art->size;}
 if(name.starts_with("Extensions/VTOL_")||name.starts_with("Extensions/Lux_")){
  auto large=pngSize(root/"Art"/name);auto spec=aircraftSpec(name.substr(11,name.size()-15));
  if(!spec)throw std::runtime_error("Unknown aircraft tile "+name);
  size={spec->width,float(std::lround(large.y*spec->width/large.x))};
 }else
#endif
 if(std::filesystem::exists(root/"Resources/RuntimeImages/apple"/name))size=pngSize(root/"Resources/RuntimeImages/apple"/name);
 else if(name.starts_with("Extensions/"))size=pngSize(root/"Art"/name);
 else{
  auto path=root/"Resources/Images"/name;
  if(!std::filesystem::exists(path))path+=".png";
  size=pngSize(path);
 }
 dimensions[name]=size;return size;
}
static std::string tileRole(std::string s){
 if(s.starts_with("Extensions/VTOL_")||s.starts_with("Extensions/Lux_")||s.starts_with("Extensions/GAA_"))s.erase(0,11);
 return s;
}
static void vec(Vector2 v){std::cout<<'\t'<<v.x<<'\t'<<v.y;}
static void weapon(const WeaponState&w){std::cout<<'\t'<<w.weaponInfo->name;vec(w.muzzlePos);vec(w.fireOffset);vec(w.weaponVec);vec(w.fireTarget);std::cout<<'\t'<<w.aimDir<<'\t'<<w.aimDirDest<<'\t'<<w.fireTime<<'\t'<<w.fireCount<<'\t'<<w.fireDelay<<'\t'<<w.isFire<<'\t'<<w.isReload<<'\t'<<w.aiStep<<'\t'<<w.resetTime<<'\t'<<w.delayTime;}
template<class M> static uint32_t machineRandom(const M&m){
 if constexpr(requires{m.vtolRandom;})return m.vtolRandom;
 else if constexpr(requires{m.privateRandom;})return m.privateRandom;
 else static_assert(sizeof(M)==0,"Update observer for renamed per-machine RNG; never silently omit it");
}
static void capture(GameSession&g,const std::string&test,int tick,uint32_t rng,uint64_t calls){
 auto&i=*g.impl_;
 auto key=[&](const char*k,int n=0){std::cout<<test<<'\t'<<tick<<'\t'<<k<<'\t'<<n;};
 key("S");std::cout<<'\t'<<rng<<'\t'<<calls<<'\t'<<i.world.gameTime<<'\t'<<i.world.gameState<<'\t'<<i.world.paused;vec(i.world.camera);vec(i.player->position);vec(i.player->velocity);std::cout<<'\t'<<i.player->hp<<'\t'<<i.player->rotation<<'\t'<<i.player->machState<<'\t'<<g.bulletCount()<<'\t'<<g.itemCount()<<'\n';
 int index=0;for(const auto&m:i.enemies){key("E",index++);std::cout<<'\t'<<m.definition->name;vec(m.enemy->position);vec(m.enemy->velocity);std::cout<<'\t'<<m.enemy->rotation<<'\t'<<m.enemy->speed<<'\t'<<m.enemy->hp<<'\t'<<m.enemy->hpMax<<'\t'<<m.enemy->machState<<'\t'<<m.enemy->engineBounce<<'\t'<<m.removed<<'\t'<<m.inScreen<<'\t'<<m.bounds.left<<'\t'<<m.bounds.bottom<<'\t'<<m.bounds.right<<'\t'<<m.bounds.top<<'\t'<<machineRandom(m)<<'\n';
  int w=0;for(const auto&p:m.parts)if(p->armed){key("W",index*100+w++);weapon(p->weapon);std::cout<<'\n';}}
 index=0;for(const auto&n:i.dynamic)if(auto b=std::get_if<BulletState>(&n.value)){key("B",index++);std::cout<<'\t'<<b->info->name<<'\t'<<b->toPlayer;vec(b->pos);vec(b->vel);vec(b->velTick);std::cout<<'\t'<<b->rotate<<'\t'<<b->alpha<<'\t'<<b->removed<<'\t'<<b->show<<'\t'<<b->startTime<<'\n';}
 index=0;for(const auto&r:g.renderItems()){
  if(auto d=std::get_if<DrawSprite>(&r)){auto dim=sizeFor(d->tile);key("D",index++);std::cout<<'\t'<<tileRole(d->tile)<<'\t'<<d->frame;vec(d->position);std::cout<<'\t'<<(dim.x/d->cols*d->scale.x)<<'\t'<<(dim.y/d->rows*d->scale.y)<<'\t'<<d->rotation<<'\t'<<d->alpha<<'\t'<<d->blend<<'\t'<<d->z<<'\t'<<d->worldTexture<<'\n';}
  else{auto&t=std::get<TrailMesh>(r);key("T",index++);std::cout<<'\t'<<t.tile;vec(t.translation);std::cout<<'\t'<<t.blend<<'\t'<<t.sourceFactor<<'\t'<<t.destinationFactor<<'\t'<<t.setBlend;for(auto&v:t.vertices){vec(v.position);vec(v.uv);for(float c:v.color)std::cout<<'\t'<<c;}std::cout<<'\n';}}
 index=0;for(auto&e:g.takeEvents()){key("V",index++);std::cout<<'\t'<<int(e.kind)<<'\t'<<e.code<<'\t'<<e.index<<'\t'<<e.amount<<'\t'<<e.value<<'\t'<<e.pan<<'\t'<<e.delay;vec(e.position);std::cout<<'\t'<<e.active<<'\t'<<e.text<<'\n';}
}
int main(int argc,char**argv){try{
 if(argc!=2)throw std::runtime_error("usage: UnitContractTrace checkout");root=argv[1];std::cout<<std::hexfloat;
 auto original=GameData::loadDirectory(root/"Resources/RawData");
#ifdef HM_UNIT_CONTRACT_BASELINE
 auto data=original;addVTOLExpansion(data);addFighterExpansion(data);addGroundAAExpansion(data);
#else
 auto data=GameData::loadDirectory(root/"Resources/RuntimeData",true);
#endif
 // Original spawn order is compared separately from isolated combat fixtures.
 std::vector<std::string> maps;for(auto&[name,map]:data.maps)maps.push_back(name);std::sort(maps.begin(),maps.end());
 for(auto&name:maps){int n=0;for(auto&e:data.maps.at(name).enemies){std::cout<<"MAP:"<<name<<"\t0\tMAP\t"<<n++<<'\t'<<e.name;vec(e.position);std::cout<<'\t'<<e.isBoss<<'\n';}}
 // Test public World regeneration with the real campaign maps. The requirement
 // is all three types per MISSION, at least one per stage, 152 total; it is NOT
 // three types in every stage. Repeated regeneration must not duplicate spawns.
 for(int mission=0;mission<7;++mission)for(int stage=0;stage<GlobalInfo::stageCount(mission);++stage){
  char mapName[16];snprintf(mapName,sizeof(mapName),"Stage%02d",GlobalInfo::mapNumber({mission,stage}));
  for(int width:{480,900}){
   struct Host final:WorldHost{std::array<int,3> counts{};Vector2 tileSize(std::string_view)override{return {480,320};}void spawnMapObject(const MapObjectDefinition&,int)override{}void spawnPlayer(Vector2)override{}void event(const WorldEvent&)override{}void spawnEnemy(const EnemyDefinition&e)override{if(e.name=="VTOL_Yak141")++counts[0];if(e.name=="VTOL_Harrier")++counts[1];if(e.name=="VTOL_F35B")++counts[2];}}host;
   WorldState world;world.setCanvasWidth(width);world.openMap(data.maps.at(mapName),0,host);
   for(float x=world.map.left;x<=world.map.right+500;x+=10)world.processRegen(x,host);
   auto first=host.counts;world.processRegen(world.map.right+1000,host);if(host.counts!=first)throw std::runtime_error("Duplicate regeneration");
   std::cout<<"CAMPAIGN:"<<mission<<":"<<stage<<"\t0\tPOLICY\t"<<width<<'\t'<<mapName;for(int n:first)std::cout<<'\t'<<n;std::cout<<'\n';
  }
 }
 const char*units[]={"Tank001","Heli001","E0505","VTOL_Yak141","VTOL_Harrier","VTOL_F35B","Lux_F22","Lux_Su57","Lux_MiG31","GAA_Shilka_M1","GAA_Tunguska_M1"};
 for(auto name:units)for(int terrain=0;terrain<2;++terrain)for(uint32_t seed:{1u,987654u}){
  auto fixture=data;auto&map=fixture.maps.at("Stage01");map.enemies.clear();map.objects.clear();map.left=0;map.right=6000;map.bottom=-200;map.top=1000;map.collisionPaths=(terrain&&fixture.machines.at(name).machType!=1)?std::vector<std::vector<Vector2>>{{{0,0},{500,0},{950,70},{6000,70}}}:std::vector<std::vector<Vector2>>{{{0,0},{6000,0}}};
  SaveSlot save;uint32_t rng=seed;uint64_t calls=0;GameSession game(fixture,save,sizeFor,[&]{++calls;rng=rng*1664525u+1013904223u;return int32_t(rng>>1);});game.setCanvasWidth(terrain?900:480);
  ValidationScenario scenario;scenario.playerPosition=Vector2{400,20};scenario.allWeapons=true;scenario.internalLevel=20;game.startValidation(scenario);
  auto&i=*game.impl_;i.enemies.clear();i.world.enemiesSpawned.assign(i.world.map.enemies.size(),true);i.world.gameState=0;game.setPaused(false);i.player->hp=100000;
  bool air=fixture.machines.at(name).machType==1;game.validationSpawnEnemy({name,{650,i.world.mapHeight(650)+(air?140.f:10.f)},false});
  std::string test=std::string(name)+":"+std::to_string(terrain)+":"+std::to_string(seed);bool moved=false,enemyBullet=false,playerBullet=false,damaged=false;auto start=i.enemies.front().enemy->position;int initialHP=i.enemies.front().enemy->hp;
  for(int tick=0;tick<=900;++tick){
   if(tick==150)game.setPaused(true);if(tick==180)game.setPaused(false);
   if(tick==240)game.booster();
   // Live muzzle-to-target trajectories, never bullets injected in a hitbox.
   if(tick>=360&&tick%60==0&&!i.enemies.empty()){auto&m=i.enemies.front();game.touch(i.screen({m.enemy->position.x,m.enemy->position.y+15}),0);}
   if(tick==840)game.releaseFire();
   game.tick(tick/60.,tick>=260&&tick<340?.04f:0.f);
   for(auto&m:i.enemies){moved|=std::abs(m.enemy->position.x-start.x)>1;damaged|=m.enemy->hp<initialHP;}
   for(auto&node:i.dynamic)if(auto b=std::get_if<BulletState>(&node.value)){enemyBullet|=b->toPlayer;playerBullet|=!b->toPlayer;}
   capture(game,test,tick,rng,calls);
  }
  std::cout<<test<<"\t901\tCOVERAGE\t0\t"<<moved<<'\t'<<enemyBullet<<'\t'<<playerBullet<<'\t'<<damaged<<'\n';
  if(!moved||!enemyBullet||!playerBullet||!damaged)throw std::runtime_error("Missing live behavior coverage (move/enemy/player/damage): "+test+" "+std::to_string(moved)+std::to_string(enemyBullet)+std::to_string(playerBullet)+std::to_string(damaged));
  std::cout<<test<<"\t901\tSAVE\t0\t";for(auto b:save.bytes)std::cout<<std::hex<<std::setw(2)<<std::setfill('0')<<int(b);std::cout<<std::dec<<'\n';
 }
 return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
