#import "HMValidationControls.h"
#if defined(HM_VALIDATION) && HM_VALIDATION
@implementation HMValidationControls
- (instancetype)init {if((self=[super init]))_selectedSegmentIndex=3;return self;}
- (NSInteger)numberOfSegments {return 4;}
- (void)setSelectedSegmentIndex:(NSInteger)value {if(value>=0&&value<4)_selectedSegmentIndex=value;}
@end
#endif
