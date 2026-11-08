#pragma once
// Test-only expected aircraft dimensions/waves. Production definitions come only
// from the serialized runtime pack; this fixture never registers game objects.
#include "World.hpp"
#include "ItemBox.hpp"
#include "GlobalInfo.hpp"
#include <fstream>
#include <set>
namespace heavymach {
struct VTOLSpec {const char* name;const char* tile;const char* statsSource;const char* weapon;float width,aspect;ObjectBounds bounds;Vector2 muzzle;std::array<Vector2,1> exhaust;bool aftExhaust=false;};
struct FighterWave {const char* map;const char* boss;std::array<int,3> counts;std::array<Vector2,8> positions;};
inline const std::array<VTOLSpec,3>& vtolSpecs(){
 static const std::array<VTOLSpec,3> specs{{
  {"VTOL_Yak141","VTOL_Yak141.png","Heli004","E_Wpn_ASM",112,724.f/2172,{-52.8f,-9.6f,52,13.6f},{-24,-8},{{{28,-8}}}},
  {"VTOL_Harrier","VTOL_Harrier.png","Heli008","E_Boss_Wpn_01",108,773.f/2034,{-60*108.f/132,-13*108.f/132,61*108.f/132,17*108.f/132},{-39*108.f/132,-10*108.f/132},{{{12*108.f/132,-10*108.f/132}}}},
  {"VTOL_F35B","VTOL_F35B.png","E0604","E_0604_Wpn",116,773.f/2033,{-68*116.f/148,-14*116.f/148,67*116.f/148,17*116.f/148},{-20*116.f/148,-11*116.f/148},{{{55*116.f/148,-8*116.f/148}}}}
 }};return specs;
}
inline const VTOLSpec* vtolSpec(std::string_view name){for(const auto& s:vtolSpecs())if(name==s.name)return &s;return nullptr;}
inline const std::array<VTOLSpec,3>& fighterSpecs(){
 static const std::array<VTOLSpec,3> specs{{
  {"Lux_F22","Lux_F22.png","Heli010","Lux_F22_Missile",112,827.f/1902,{-53,-11,53,14},{-18,-10},{{{53,-7}}},true},
  {"Lux_Su57","Lux_Su57.png","Heli010","Lux_Su57_Missile",116,756.f/2079,{-55,-10,55,13},{-16,-9},{{{53,-4}}},true},
  {"Lux_MiG31","Lux_MiG31.png","Heli010","Lux_MiG31_Missile",120,724.f/2172,{-56,-10,56,14},{-12,-9},{{{55,-3}}},true}
 }};return specs;
}
inline const VTOLSpec* aircraftSpec(std::string_view name){
 static const VTOLSpec euro{"Lux_Eurofighter","Lux_Eurofighter.png","Lux_F22","Lux_Euro_Missile",116,35.f/116,{-55,-14,55,17},{-23.5f,-9.5f},{{{51,-8}}},true};
 if(name==euro.name)return &euro;
 for(const auto& s:fighterSpecs())if(name==s.name)return &s;return vtolSpec(name);}
inline const std::array<FighterWave,14>& fighterWaves(){
 // Authored spacing, like campaign EnemyDefinition positions. Aircraft are
 // 112..120 wide and at most 49 high; 140 X / 70 Y leaves visible separation.
 static constexpr std::array<Vector2,8> pair{{{0,0},{0,70}}};
 static constexpr std::array<Vector2,8> four{{{0,0},{0,70},{140,0},{140,70}}};
 static constexpr std::array<Vector2,8> six{{{0,0},{0,70},{0,140},{140,0},{140,70},{140,140}}};
 static constexpr std::array<Vector2,8> eight{{{0,0},{0,70},{0,140},{140,0},{140,70},{140,140},{280,0},{280,70}}};
 static const std::array<FighterWave,14> waves{{
  {"Stage04","Boss02",{1,1,0},pair},{"Stage09","Boss04",{1,1,2},four},
  {"Stage14","Boss05",{2,2,2},six},{"Stage29","Boss03",{3,3,2},eight},
  {"Stage19","Boss01",{3,2,3},eight},{"Stage24","Boss06",{2,3,3},eight},
  {"Stage35","Boss07",{3,3,2},eight},
  {"Stage03","",{2,1,1},four},{"Stage08","",{1,2,1},four},
  {"Stage13","",{1,1,2},four},{"Stage28","",{2,1,1},four},
  {"Stage18","",{1,2,1},four},{"Stage23","",{1,1,2},four},
  {"Stage34","",{2,1,1},four}
 }};return waves;
}
inline const FighterWave* fighterWave(std::string_view map){for(const auto& w:fighterWaves())if(map==w.map)return &w;return nullptr;}

inline Vector2 vtolRotate(Vector2 p,float a){return rotateAttachment(p,a);}
inline float vtolPitch(float previous,float x,float y){MachineBehavior b;b.horizontalDegrees=8;b.verticalDegrees=3;return velocityPitch(previous,x,y,b);}
inline bool isGroundAA(std::string_view n){return n.starts_with("GAA_Shilka_M")||n.starts_with("GAA_Tunguska_M");}
inline const GameData& contractData(){static const auto d=GameData::loadDirectory(std::filesystem::path(__FILE__).parent_path().parent_path()/"Resources/RuntimeData",true);return d;}
inline constexpr int groundAARadarFrames=24;
inline constexpr double groundAARadarPeriod=2.;
struct TestGroundImage{std::string name,source;Vector2 size;float radarAnchorX,radarAnchorY;};
inline const std::vector<TestGroundImage>& groundAAImages(){static const auto images=[] {std::vector<TestGroundImage> out;std::set<std::string> seen;for(auto& [name,p]:contractData().parts)if(isGroundAA(name)&&p.presentation)for(auto& f:p.presentation->frames)if(seen.insert(f.tile).second)out.push_back({f.tile,f.tile,p.presentation->logicalSize,-f.offset.x,-f.offset.y});return out;}();return images;}
inline const TestGroundImage* groundAAImage(std::string_view key){for(auto& a:groundAAImages())if(a.name==key)return &a;return nullptr;}
inline std::optional<Vector2> groundAAPartSize(std::string_view name){auto i=contractData().parts.find(std::string(name));if(i==contractData().parts.end()||!i->second.presentation)return {};return i->second.presentation->logicalSize;}
inline std::string groundAAPartTile(std::string_view name,int frame=0){return contractData().parts.at(std::string(name)).presentation->frames.at(size_t(frame)).tile;}
inline void configureGroundAADraw(DrawSprite& d,const PartsState&p,double t){applyPartPresentation(d,p,{},p.rotation,0,t,p.info->presentation->logicalSize);}
struct TestGroundPlacement{int mission,stage,originalTanks,shilka,tunguska;std::string map;};
inline std::vector<TestGroundPlacement> groundAAPlan(const GameData& d){std::vector<TestGroundPlacement> rows;for(int m=0;m<7;++m)for(int s=0;s<GlobalInfo::stageCount(m);++s){char key[32];snprintf(key,sizeof(key),"Stage%02d",GlobalInfo::mapNumber({m,s}));TestGroundPlacement r{m,s,0,0,0,key};for(auto&e:d.maps.at(key).enemies){if(e.name.starts_with("Tank")&&!e.isBoss)++r.originalTanks;if(e.name.starts_with("GAA_Shilka"))++r.shilka;if(e.name.starts_with("GAA_Tunguska"))++r.tunguska;}rows.push_back(r);}return rows;}
inline Vector2 contractTileSize(const std::filesystem::path& root,std::string_view key){auto path=root/"Resources/RuntimeImages/apple"/key;if(!std::filesystem::exists(path))path=root/"Resources/Images"/key;std::ifstream f(path,std::ios::binary);unsigned char b[24]{};f.read((char*)b,24);if(f.gcount()!=24)return {40,12};auto be=[&](int n){return uint32_t(b[n])<<24|uint32_t(b[n+1])<<16|uint32_t(b[n+2])<<8|b[n+3];};return {float(be(16)),float(be(20))};}
}
