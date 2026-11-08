#include "RuntimeUnitTestSupport.hpp"
#include "GlobalInfo.hpp"
#include <cassert>
#include <iostream>
#include <cmath>
using namespace heavymach;
struct Host:CombatHost{
 std::vector<double> shots;int32_t random31()override{return 0;}bool sweepTerrain(Vector2&,const Vector2&)override{return false;}bool inCamera(const BulletState&)override{return true;}std::optional<CombatTarget> targetAt(bool,Vector2)override{return {};}void damage(CombatTarget,int32_t)override{}bool acquireParticle()override{return false;}void event(const CombatEvent&)override{}void spawnBullet(const WeaponInfo&,bool,float,Vector2,double t)override{shots.push_back(t);}
};
int main(){auto root=std::filesystem::path(__FILE__).parent_path().parent_path();auto before=GameData::loadDirectory(root/"Resources/RawData"),data=GameData::loadDirectory(root/"Resources/RuntimeData",true);auto plan=groundAAPlan(data);int total=0;
 for(const auto& [name,map]:before.maps){auto& after=data.maps.at(name);assert(after.enemies.size()>=map.enemies.size());for(size_t n=0;n<map.enemies.size();++n){const auto&a=map.enemies[n];const auto&b=after.enemies[n];assert(a.name==b.name&&a.position.x==b.position.x&&a.position.y==b.position.y&&a.isBoss==b.isBoss);}}
 for(const auto& row:plan){auto& m=data.maps.at(row.map);int s=0,t=0;for(auto& e:m.enemies){if(e.name.starts_with("GAA_Shilka"))++s;if(e.name.starts_with("GAA_Tunguska"))++t;}assert(s==row.shilka&&t==row.tunguska&&s+t>=1);total+=s+t;std::cout<<row.mission+1<<','<<row.stage+1<<','<<row.map<<','<<row.originalTanks<<','<<s<<','<<t<<'\n';}
 assert(total==152);
 for(int mission=0;mission<7;++mission){int tanks=0,added=0;for(auto& r:plan)if(r.mission==mission){tanks+=r.originalTanks;added+=r.shilka+r.tunguska;}assert(added==std::lround(tanks*1.3));}
 for(const auto& [name,w]:data.weapons)if(isGroundAA(name)){
  const int expected=name.ends_with("_Missile")?2:name.starts_with("GAA_Shilka")?6:4;assert(w.shotCnt==expected);
  if(!name.ends_with("_Missile")){assert(w.effectTile=="BulletEffect.png"&&w.effectType=="Normal"&&w.bulletTile=="none");}
  Host host;WeaponState state;state.weaponInfo=&w;CombatFrame frame;frame.machType=3;frame.time=1;assert(fireWeapon(state,frame,host));
  for(int n=61;n<180;++n){frame.time=double(n)/60;tickWeapon(state,frame,host);}assert(int(host.shots.size())==expected);
  for(size_t n=1;n<host.shots.size();++n)assert(std::abs((host.shots[n]-host.shots[n-1])-w.shotDly)<.018);
  state={};state.weaponInfo=&w;frame.time=4;fireWeapon(state,frame,host);int remaining=state.fireCount;frame.paused=true;frame.time=30;tickWeapon(state,frame,host);assert(state.fireCount==remaining);
 }
 for(const auto& [name,w]:before.weapons){auto& n=data.weapons.at(name);assert(w.shotCnt==n.shotCnt&&w.shotDly==n.shotDly&&w.spd==n.spd&&w.dmg==n.dmg);}
 assert(groundAAPartSize("GAA_Shilka_M1_Turret")->y==groundAAPartSize("GAA_Tunguska_M1_Turret")->y);
 // Body/turret must overlap at their solid join on flat and sloped terrain.
 for(const char* unit:{"GAA_Shilka_M1","GAA_Tunguska_M1"}){
  std::string id=unit;PartsState body,top;body.info=&data.parts.at(id+"_Body");top.info=&data.parts.at(id+"_Turret");
  attachParts(top,&body,1);body.offset=partsSocketPosition(body,0);
  auto b=*groundAAPartSize(id+"_Body"),t=*groundAAPartSize(id+"_Turret");assert(b.x<=85);
  PartsState radar;radar.info=&data.parts.at(id+"_Radar");attachParts(radar,&top,2);
  for(float angle:{-.4f,0.f,.4f})for(float bounce:{-1.f,0.f,1.f}){
   auto bd=drawParts(body,{100,50},angle,bounce,1,1,1),td=drawParts(top,{100,50},angle,bounce,1,1,1);
   float dx=td.position.x-bd.position.x,dy=td.position.y-bd.position.y;Vector2 relative{dx*std::cos(angle)+dy*std::sin(angle),-dx*std::sin(angle)+dy*std::cos(angle)};
   for(int frame=0;frame<groundAARadarFrames;++frame){
    double time=(frame+.25)*groundAARadarPeriod/groundAARadarFrames;
    auto rd=drawParts(radar,{100,50},angle,bounce,1,1,time);configureGroundAADraw(rd,radar,time);
    const auto* art=groundAAImage(rd.tile);assert(art);
    float rx=rd.position.x-td.position.x,ry=rd.position.y-td.position.y;
    Vector2 axle{rx*std::cos(angle)+ry*std::sin(angle)+art->radarAnchorX,-rx*std::sin(angle)+ry*std::cos(angle)+art->radarAnchorY};
    // Actual fixed pedestal socket must stay still through all yaw frames.
    assert(std::abs(axle.x-top.info->sockets[2].x)<.001f);
    assert(std::abs(axle.y-top.info->sockets[2].y)<.001f);
   }
   assert(std::abs(relative.x)<.001f);assert(relative.y-t.y/2<=b.y/2-(id.find("Shilka")!=std::string::npos?3.9f:1.9f));
  }
 }
 for(const auto& art:groundAAImages())assert(std::filesystem::exists(root/"Resources/RuntimeImages/apple"/art.source));
 for(auto name:{"GAA_Shilka_M1_Radar","GAA_Tunguska_M7_Radar"}){PartsState p;p.info=&data.parts.at(name);p.rotation=.15;for(int n=0;n<groundAARadarFrames;++n){DrawSprite d;d.rotation=.15;configureGroundAADraw(d,p,(n+.25)*groundAARadarPeriod/groundAARadarFrames);assert(d.tile==groundAAPartTile(name,n));assert(d.rotation==.15f);}}
 std::cout<<"Ground AA definitions, original spawns, all 35 stages, bursts, pause and radar passed; added="<<total<<'\n';
}
