#pragma once
#if defined(HM_VALIDATION) && HM_VALIDATION
#import "HMPlatform.h"
#include "ValidationScenario.hpp"
// Settings model only; HMGameView draws this panel with original game buttons.
@interface HMValidationPanel : NSObject
- (instancetype)initWithData:(const heavymach::GameData *)data;
@property(nonatomic) int mission,stage,level,boss,weaponIndex,page;
@property(nonatomic) BOOL nearLevel,allWeapons,wideCanvas;
@property(nonatomic) int bodyGrade,topGrade,weaponGrade;
- (void)adjust:(int)field delta:(int)delta;
- (NSString*)fieldText:(int)field;
- (heavymach::ValidationScenario)scenario;
- (void)saveSettings;
@end
#endif
