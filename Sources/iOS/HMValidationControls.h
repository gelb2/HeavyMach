#pragma once
#if defined(HM_VALIDATION) && HM_VALIDATION
#import "HMPlatform.h"
// Validation input state. The game draws and receives every control.
@interface HMValidationControls : NSObject
@property(nonatomic) BOOL hudEditing;
@property(nonatomic) BOOL hidden;
@property(nonatomic) CGRect frame;
@property(nonatomic) NSInteger selectedSegmentIndex;
@property(nonatomic,readonly) NSInteger numberOfSegments;
@end
#endif
