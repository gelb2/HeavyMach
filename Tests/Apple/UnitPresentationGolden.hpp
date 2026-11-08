#pragma once
// Golden presentation dimensions from the pre-hotfix 191728e3 build.
// This fixture is test-only and does not register or render any game unit.
#include <array>
#include <string_view>
namespace unitGolden {
struct Aircraft {const char* name;const char* tile;float width,aspect;};
inline const std::array<Aircraft,3> vtol{{
 {"VTOL_Yak141","VTOL_Yak141.png",112,724.f/2172},
 {"VTOL_Harrier","VTOL_Harrier.png",108,773.f/2034},
 {"VTOL_F35B","VTOL_F35B.png",116,773.f/2033}}};
inline const std::array<Aircraft,3> fighters{{
 {"Lux_F22","Lux_F22.png",112,827.f/1902},
 {"Lux_Su57","Lux_Su57.png",116,756.f/2079},
 {"Lux_MiG31","Lux_MiG31.png",120,724.f/2172}}};
inline const Aircraft* aircraft(std::string_view name){for(const auto& a:vtol)if(a.name==name)return &a;for(const auto& a:fighters)if(a.name==name)return &a;return nullptr;}
struct Image {const char* name;int width,height;};
inline const std::array<Image,63> images{{
 {"VTOL_Yak141.png",112,37},
 {"VTOL_Harrier.png",108,41},
 {"VTOL_F35B.png",116,44},
 {"Lux_F22.png",112,49},
 {"Lux_Su57.png",116,42},
 {"Lux_MiG31.png",120,40},
 {"GAA_Shilka_Body.png",80,18},
 {"GAA_Shilka_Body2.png",80,18},
 {"GAA_Shilka_Turret.png",51,29},
 {"GAA_Shilka_Gun.png",32,6},
 {"GAA_Tunguska_Body.png",85,22},
 {"GAA_Tunguska_Body2.png",85,22},
 {"GAA_Tunguska_Turret.png",50,29},
 {"GAA_Tunguska_Gun.png",37,7},
 {"GAA_Tunguska_Missile.png",30,15},
 {"GAA_Shilka_Radar0.png",16,16},
 {"GAA_Shilka_Radar1.png",16,16},
 {"GAA_Shilka_Radar2.png",16,16},
 {"GAA_Shilka_Radar3.png",16,16},
 {"GAA_Shilka_Radar4.png",16,16},
 {"GAA_Shilka_Radar5.png",16,16},
 {"GAA_Shilka_Radar6.png",16,16},
 {"GAA_Shilka_Radar7.png",16,16},
 {"GAA_Shilka_Radar8.png",16,16},
 {"GAA_Shilka_Radar9.png",16,16},
 {"GAA_Shilka_Radar10.png",16,16},
 {"GAA_Shilka_Radar11.png",16,16},
 {"GAA_Shilka_Radar12.png",16,16},
 {"GAA_Shilka_Radar13.png",16,16},
 {"GAA_Shilka_Radar14.png",16,16},
 {"GAA_Shilka_Radar15.png",16,16},
 {"GAA_Shilka_Radar16.png",16,16},
 {"GAA_Shilka_Radar17.png",16,16},
 {"GAA_Shilka_Radar18.png",16,16},
 {"GAA_Shilka_Radar19.png",16,16},
 {"GAA_Shilka_Radar20.png",16,16},
 {"GAA_Shilka_Radar21.png",16,16},
 {"GAA_Shilka_Radar22.png",16,16},
 {"GAA_Shilka_Radar23.png",16,16},
 {"GAA_Tunguska_Radar0.png",21,21},
 {"GAA_Tunguska_Radar1.png",21,21},
 {"GAA_Tunguska_Radar2.png",21,21},
 {"GAA_Tunguska_Radar3.png",21,21},
 {"GAA_Tunguska_Radar4.png",21,21},
 {"GAA_Tunguska_Radar5.png",21,21},
 {"GAA_Tunguska_Radar6.png",21,21},
 {"GAA_Tunguska_Radar7.png",21,21},
 {"GAA_Tunguska_Radar8.png",21,21},
 {"GAA_Tunguska_Radar9.png",21,21},
 {"GAA_Tunguska_Radar10.png",21,21},
 {"GAA_Tunguska_Radar11.png",21,21},
 {"GAA_Tunguska_Radar12.png",21,21},
 {"GAA_Tunguska_Radar13.png",21,21},
 {"GAA_Tunguska_Radar14.png",21,21},
 {"GAA_Tunguska_Radar15.png",21,21},
 {"GAA_Tunguska_Radar16.png",21,21},
 {"GAA_Tunguska_Radar17.png",21,21},
 {"GAA_Tunguska_Radar18.png",21,21},
 {"GAA_Tunguska_Radar19.png",21,21},
 {"GAA_Tunguska_Radar20.png",21,21},
 {"GAA_Tunguska_Radar21.png",21,21},
 {"GAA_Tunguska_Radar22.png",21,21},
 {"GAA_Tunguska_Radar23.png",21,21}
}};
}
