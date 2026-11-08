#include "GameData.hpp"
#include "VTOL.hpp"
#include "Fighters.hpp"
#include "GroundAA.hpp"
#include <iostream>
#include <map>
#include <functional>
using namespace heavymach;
void counts(const char* name,const GameData& d){size_t spawns=0;for(auto& [k,m]:d.maps)spawns+=m.enemies.size();std::cout<<name<<",weapons="<<d.weapons.size()<<",parts="<<d.parts.size()<<",machines="<<d.machines.size()<<",maps="<<d.maps.size()<<",spawn_entries="<<spawns<<'\n';}
int main(int argc,char** argv){auto original=GameData::loadDirectory(argv[1]), d=original;counts("original",d);addVTOLExpansion(d);counts("vtol",d);addFighterExpansion(d);counts("fighters",d);addGroundAAExpansion(d);counts("groundaa",d);
 std::map<std::string,int> spawned;for(auto& [k,m]:d.maps)for(auto& e:m.enemies)if(!original.machines.contains(e.name))++spawned[e.name];
 std::map<std::string,const MachDefinition*> machines;for(auto&[name,m]:d.machines)if(!original.machines.contains(name))machines[name]=&m;
 for(auto& [name,m]:machines){std::cout<<"unit,"<<name<<",hp="<<m->hpMax<<",exp="<<m->giveExp<<",type="<<int(m->machType)<<",map_spawns="<<spawned[name]<<'\n';std::function<void(const std::vector<MachPartDefinition>&,int)> visit=[&](auto& defs,int depth){for(auto& p:defs){auto& info=d.parts.at(p.name);std::cout<<"part,"<<name<<","<<depth<<","<<p.name<<",type="<<int(info.partsType)<<",tiles="<<int(info.tileCnt)<<",socket="<<int(p.baseSocket)<<",z="<<int(p.zOrder)<<",tile="<<info.tileName()<<'\n';visit(p.children,depth+1);}};visit(m->parts,0);}
 for(auto&w:fighterWaves()){std::cout<<"fighter_wave,"<<w.map<<","<<w.boss;for(auto c:w.counts)std::cout<<","<<c;std::cout<<'\n';}
 for(auto&s:vtolSpecs()){auto&w=d.weapons.at(s.weapon);std::cout<<"vtol_weapon,"<<s.name<<","<<w.name<<",damage="<<w.dmg<<",speed="<<w.spd<<",count="<<int(w.shotCnt)<<",tile="<<w.bulletTile<<",effect="<<w.effectTile<<'\n';}
}
