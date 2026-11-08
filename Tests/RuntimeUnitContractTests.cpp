#include "GameData.hpp"
#include "Visuals.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <functional>
using namespace heavymach;
int main(){
 const auto root=std::filesystem::path(__FILE__).parent_path().parent_path();
 auto original=GameData::loadDirectory(root/"Resources/RawData"),d=GameData::loadDirectory(root/"Resources/RuntimeData",true);
 bool missingFileRejected=false;try{GameData::loadDirectory(root/"Resources/RawData",true);}catch(const std::runtime_error&){missingFileRejected=true;}assert(missingFileRejected);
 assert(original.weapons.size()==160&&original.parts.size()==294&&original.machines.size()==720);
 assert(d.weapons.size()==186&&d.parts.size()==379&&d.machines.size()==742&&d.maps.size()==43&&d.formations.size()==14);
 size_t prior=0,now=0;for(auto&[key,map]:original.maps){auto& after=d.maps.at(key);prior+=map.enemies.size();now+=after.enemies.size();assert(after.left==map.left&&after.right==map.right&&after.top==map.top&&after.bottom==map.bottom&&after.background==map.background);assert(after.objects.size()==map.objects.size()&&after.collisionPaths.size()==map.collisionPaths.size());for(size_t i=0;i<map.enemies.size();++i){const auto&a=map.enemies[i];const auto&b=after.enemies[i];assert(a.name==b.name&&a.position.x==b.position.x&&a.position.y==b.position.y&&a.isBoss==b.isBoss);}}
 assert(prior==1182&&now==1562);
 auto images=d.presentationImages();assert(images.size()==65);for(auto& name:images)assert(name.find('/')==std::string::npos);
 for(auto&[name,m]:original.machines)assert(!d.machines.at(name).behavior);
 for(auto&[name,p]:original.parts)assert(!d.parts.at(name).presentation);
 for(auto&[name,w]:original.weapons){auto& n=d.weapons.at(name);assert(n.aiTimeScale==1&&n.burstGroupSize==0&&n.burstGroupDelay==0);}
 std::function<void(const std::vector<MachPartDefinition>&)> refs=[&](const auto& nodes){for(auto& n:nodes){auto& p=d.parts.at(n.name);if(p.partsType==2)assert(d.weapons.contains(p.name));refs(n.children);}};
 for(auto& [name,m]:d.machines)if(!original.machines.contains(name))refs(m.parts);
 size_t frameCount=0;for(auto& [name,p]:d.parts)if(p.presentation){PartsState state;state.info=&p;auto& v=*p.presentation;if(v.animation==PartAnimation::WorldLoop){assert(v.frames.size()==24&&v.period==2);for(size_t i=0;i<24;++i){auto f=presentationFrame(state,(i+.25)*2/24);assert(f==&v.frames[i]);++frameCount;}assert(presentationFrame(state,2)==&v.frames[0]);}}
 assert(frameCount==7*48);
 std::ifstream file(root/"Resources/RuntimeData/UnitExtensions.hmx",std::ios::binary);std::vector<uint8_t> bytes{std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};
 auto plain=d;plain.formations.clear();for(auto&[n,p]:plain.parts)p.presentation.reset();for(auto&[n,m]:plain.machines)m.behavior.reset();
 auto rejects=[&](const std::vector<uint8_t>& b){auto candidate=plain;
 try{candidate.applyExtensions(b);return false;}catch(const std::runtime_error&){return true;}};
 assert(rejects({}));auto bad=bytes;bad[0]='?';assert(rejects(bad));bad=bytes;bad.resize(16);assert(rejects(bad));bad=bytes;bad.push_back(0);assert(rejects(bad));
 bool missingRejected=false;try{original.applyExtensions(bytes);}catch(const std::runtime_error&){missingRejected=true;}assert(missingRejected);
 bool duplicateRejected=false;try{d.applyExtensions(bytes);}catch(const std::runtime_error&){duplicateRejected=true;}assert(duplicateRejected);
 std::cout<<"Native-format runtime pack, original defaults, 22 machine contracts, 336 radar frames, invalid contracts PASS\n";
}
