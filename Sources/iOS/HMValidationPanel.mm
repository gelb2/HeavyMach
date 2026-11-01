#import "HMValidationPanel.h"
#if defined(HM_VALIDATION) && HM_VALIDATION
#include "GlobalInfo.hpp"
#include <algorithm>
@implementation HMValidationPanel {
 const heavymach::GameData* _data;
 std::vector<heavymach::ValidationStage> _stages;
 std::vector<std::string> _weapons;
}
- (instancetype)initWithData:(const heavymach::GameData*)data {
 if((self=[super init])){
  _data=data;_stages=heavymach::validationStages(*data);_weapons=heavymach::validationWeapons(*data);
  auto d=NSUserDefaults.standardUserDefaults;_mission=std::clamp(int([d integerForKey:@"fixture.mission"]),0,7);_stage=std::clamp(int([d integerForKey:@"fixture.stage"]),0,heavymach::GlobalInfo::stageCount(_mission)-1);_level=std::clamp(int([d integerForKey:@"fixture.level"]),0,99);
  _bodyGrade=std::clamp(int([d integerForKey:@"fixture.grade0"]),0,6);_topGrade=std::clamp(int([d integerForKey:@"fixture.grade1"]),0,6);_weaponGrade=std::clamp(int([d integerForKey:@"fixture.grade2"]),0,6);
  _nearLevel=[d boolForKey:@"fixture.near"];_allWeapons=[d boolForKey:@"fixture.all"];_boss=[d objectForKey:@"fixture.boss"]?int([d integerForKey:@"fixture.boss"]):-1;
  NSString* weapon=[d stringForKey:@"fixture.weapon"]?:@"P_Wpn_Gun01";auto i=std::find(_weapons.begin(),_weapons.end(),weapon.UTF8String);_weaponIndex=i==_weapons.end()?0:int(i-_weapons.begin());[self clampStage];
 }return self;
}
- (const heavymach::ValidationStage*)selectedStage {for(const auto& s:_stages)if(s.mission==_mission&&s.stage==_stage)return &s;return nullptr;}
- (void)clampStage {_stage=std::clamp(_stage,0,heavymach::GlobalInfo::stageCount(_mission)-1);auto s=[self selectedStage];if(!s||_boss>=int(s->bosses.size()))_boss=-1;}
- (void)adjust:(int)field delta:(int)d {
 auto wrap=[](int x,int n){return n?(x%n+n)%n:0;};
 switch(field){case 0:_mission=std::clamp(_mission+d,0,7);[self clampStage];break;case 1:_stage=std::clamp(_stage+d,0,heavymach::GlobalInfo::stageCount(_mission)-1);[self clampStage];break;case 2:_level=std::clamp(_level+d,0,99);break;case 3:_bodyGrade=std::clamp(_bodyGrade+d,0,6);break;case 4:_topGrade=std::clamp(_topGrade+d,0,6);break;case 5:_weaponGrade=std::clamp(_weaponGrade+d,0,6);break;case 6:_nearLevel=!_nearLevel;break;case 7:_allWeapons=!_allWeapons;break;case 8:_weaponIndex=wrap(_weaponIndex+d,int(_weapons.size()));break;case 9:{auto s=[self selectedStage];_boss=std::clamp(_boss+d,-1,s?int(s->bosses.size())-1:-1);break;}case 10:_wideCanvas=!_wideCanvas;break;}
}
- (NSString*)fieldText:(int)field {
 switch(field){case 0:return [NSString stringWithFormat:@"MISSION %d",_mission+1];case 1:return [NSString stringWithFormat:@"STAGE %d",_stage+1];case 2:return [NSString stringWithFormat:@"LEVEL %d",_level+1];case 3:return [NSString stringWithFormat:@"BODY %d",_bodyGrade];case 4:return [NSString stringWithFormat:@"TURRET %d",_topGrade];case 5:return [NSString stringWithFormat:@"WEAPON GRADE %d",_weaponGrade];case 6:return _nearLevel?@"NEXT LEVEL EXP 1: ON":@"NEXT LEVEL EXP 1: OFF";case 7:return _allWeapons?@"ALL WEAPONS: ON":@"ALL WEAPONS: OFF";case 8:return _weapons.empty()?@"NO WEAPON":[NSString stringWithUTF8String:_weapons[_weaponIndex].c_str()];case 9:{auto s=[self selectedStage];return _boss<0?@"BOSS: NORMAL START":[NSString stringWithFormat:@"BOSS %d: %s",_boss+1,s->bosses[_boss].name.c_str()];}case 10:return _wideCanvas?@"FULL WIDTH: ON":@"FULL WIDTH: OFF";default:return @"";}
}
- (heavymach::ValidationScenario)scenario {
 heavymach::ValidationScenario s;s.mission=_mission;s.stage=_stage;s.internalLevel=_level;s.grades={uint8_t(_bodyGrade),uint8_t(_topGrade),uint8_t(_weaponGrade)};s.nearLevelUp=_nearLevel;s.allWeapons=_allWeapons;if(_boss>=0)s.bossIndex=size_t(_boss);if(!_weapons.empty())s.weapons.emplace_back(_weapons[_weaponIndex]);
 heavymach::SaveSlot probe;heavymach::prepareValidationSlot(probe,*_data,s);if(!s.weapons.empty())for(int i=0;i<probe.weaponCount();++i)if(probe.weaponName(i)==s.weapons.front()){s.selectedWeapon=i;break;}return s;
}
- (void)saveSettings {
 auto d=NSUserDefaults.standardUserDefaults;[d setInteger:_mission forKey:@"fixture.mission"];[d setInteger:_stage forKey:@"fixture.stage"];[d setInteger:_level forKey:@"fixture.level"];[d setInteger:_bodyGrade forKey:@"fixture.grade0"];[d setInteger:_topGrade forKey:@"fixture.grade1"];[d setInteger:_weaponGrade forKey:@"fixture.grade2"];[d setBool:_nearLevel forKey:@"fixture.near"];[d setBool:_allWeapons forKey:@"fixture.all"];[d setInteger:_boss forKey:@"fixture.boss"];if(!_weapons.empty())[d setObject:[NSString stringWithUTF8String:_weapons[_weaponIndex].c_str()] forKey:@"fixture.weapon"];
}
@end
#endif
