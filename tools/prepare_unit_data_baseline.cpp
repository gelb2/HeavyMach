// One-time authoring capture. Compile against the audited pre-hotfix checkout.
#include "GameData.hpp"
#include "VTOL.hpp"
#include "Fighters.hpp"
#include "GroundAA.hpp"
#include <iostream>
#include <iomanip>
#include <map>
#include <functional>
using namespace heavymach;
void q(std::string_view s){std::cout<<std::quoted(std::string(s));}
void point(Vector2 p){std::cout<<'['<<p.x<<','<<p.y<<']';}
void tree(const std::vector<MachPartDefinition>& ps){std::cout<<'[';bool first=true;for(auto&p:ps){if(!first)std::cout<<',';first=false;std::cout<<'[';q(p.name);std::cout<<','<<int(p.baseSocket)<<','<<int(p.zOrder)<<',';tree(p.children);std::cout<<']';}std::cout<<']';}
template<class T> auto ordered(const T& values){std::map<std::string,const typename T::mapped_type*> out;for(auto&[k,v]:values)out[k]=&v;return out;}
int main(int argc,char**argv){std::cout<<std::setprecision(9);auto o=GameData::loadDirectory(argv[1]),d=o;addVTOLExpansion(d);addFighterExpansion(d);addGroundAAExpansion(d);
std::cout<<"{\"schema\":1,\"baseline\":\"191728e310158a55df1164ee452b19b48fa97e57\",\"weapons\":[";bool first=true;
for(auto&[k,p]:ordered(d.weapons))if(!o.weapons.contains(k)){auto&w=*p;if(!first)std::cout<<',';first=false;std::cout<<'[';q(k);std::cout<<','<<w.wpnType<<','<<w.dmg<<','<<int(w.pierce)<<','<<int(w.shotCnt)<<','<<w.shotDly<<','<<w.delay<<','<<w.spd<<','<<w.spdRand<<','<<w.aimRand<<','<<w.g<<','<<w.charge<<','<<w.liveTm<<','<<int(w.rotWpn)<<','<<int(w.uiTile)<<',';q(w.bulletTile);std::cout<<','<<w.bulletTileNo<<',';q(w.effectType);std::cout<<',';q(w.effectTile);std::cout<<','<<w.effectTileNo<<','<<w.shell<<',';q(w.fireSFX);std::cout<<','<<w.aiTimeScale<<','<<w.burstGroupDelay<<','<<int(w.burstGroupSize)<<']';}
std::cout<<"],\"parts\":[";first=true;for(auto&[k,p]:ordered(d.parts))if(!o.parts.contains(k)){if(!first)std::cout<<',';first=false;std::cout<<'[';q(k);std::cout<<','<<int(p->partsType)<<','<<int(p->tileCnt)<<",[";bool f=true;for(auto&s:p->sockets){if(!f)std::cout<<',';f=false;point(s);}std::cout<<"]]";}
std::cout<<"],\"machines\":[";first=true;for(auto&[k,p]:ordered(d.machines))if(!o.machines.contains(k)){if(!first)std::cout<<',';first=false;std::cout<<'[';q(k);std::cout<<','<<p->hpMax<<','<<p->giveExp<<','<<int(p->machType)<<',';tree(p->parts);std::cout<<']';}
std::cout<<"],\"placements\":{";first=true;for(auto&[k,p]:ordered(d.maps)){auto n=o.maps.at(k).enemies.size();if(n==p->enemies.size())continue;if(!first)std::cout<<',';first=false;q(k);std::cout<<":[";for(size_t i=n;i<p->enemies.size();++i){if(i!=n)std::cout<<',';auto&e=p->enemies[i];std::cout<<'[';q(e.name);std::cout<<',';point(e.position);std::cout<<']';}std::cout<<']';}
std::cout<<"},\"aircraft\":[";first=true;auto emit=[&](auto&s){if(!first)std::cout<<',';first=false;std::cout<<'[';q(s.name);std::cout<<',';q(s.tile);std::cout<<','<<s.width<<','<<s.aspect<<",["<<s.bounds.left<<','<<s.bounds.bottom<<','<<s.bounds.right<<','<<s.bounds.top<<"],";point(s.muzzle);std::cout<<',';point(s.exhaust[0]);std::cout<<','<<s.aftExhaust<<']';};for(auto&s:vtolSpecs())emit(s);for(auto&s:fighterSpecs())emit(s);
std::cout<<"],\"ground_images\":[";first=true;for(auto&a:groundAAImages()){if(!first)std::cout<<',';first=false;std::cout<<'[';q(a.name);std::cout<<',';point(a.size);std::cout<<','<<a.radarAnchorX<<','<<a.radarAnchorY<<']';}
std::cout<<"],\"waves\":[";first=true;for(auto&w:fighterWaves()){if(!first)std::cout<<',';first=false;std::cout<<'[';q(w.map);std::cout<<',';q(w.boss);std::cout<<",[";int at=0;for(size_t type=0;type<3;++type)for(int i=0;i<w.counts[type];++i,++at){if(at)std::cout<<',';std::cout<<'[';q(fighterSpecs()[type].name);std::cout<<',';point(w.positions[at]);std::cout<<']';}std::cout<<"]]";}std::cout<<"]}\n";
}
