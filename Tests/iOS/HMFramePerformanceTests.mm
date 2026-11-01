#import <XCTest/XCTest.h>
#import <objc/runtime.h>
#import <CoreMotion/CoreMotion.h>
#import "../../Sources/iOS/HMGameView.h"
#import "../../Sources/iOS/HMMenus.h"
#include "../../Sources/Core/TitleState.hpp"
#include "../../Sources/Core/LoadingTransition.hpp"
#import "../../Sources/iOS/HMMedia.h"
#import "../../Sources/iOS/HMGLRenderer.h"
#import "../../Sources/iOS/HMOpenALAudio.h"
#define HM_VALIDATION 1
#import "../../Sources/iOS/HMValidationControls.h"
#include "../../Sources/Core/GameSession.hpp"
#include <algorithm>
#include <ctime>
#include <vector>
#include <stdexcept>
#include "../../Sources/Core/Viewport.hpp"
#include "../../Sources/Core/TiltState.hpp"

@interface HMGameView (FramePerformance)
- (void)frame:(CADisplayLink*)link;
- (void)drawLogicalFrame;
- (void)drawLoading;
- (void)sampleOriginalAcceleration;
- (UIInterfaceOrientation)accelerationInterfaceOrientation;
- (int32_t)nextOriginalRandom;
- (void)refreshScreen:(int)screen;
- (void)startSlot:(int)slot mode:(int)mode mission:(int)mission stage:(int)stage;
- (void)advanceOriginalFrame;
- (void)gameButton:(int)identifier state:(int)state;
- (void)gameUIAction:(int)identifier state:(int)state;
- (CGRect)gameButtonRect:(int)identifier;
- (void)routePoint:(CGPoint)point state:(int)state token:(id)token;
- (void)checkInactivity;
- (void)setWideCanvasEnabled:(BOOL)enabled;
- (void)updateCanvasGeometry;
- (void)routeTouches:(NSSet<UITouch*>*)touches state:(int)state;
- (BOOL)handleKeyCode:(unsigned)code pressed:(BOOL)pressed;
- (void)setInputAimMode:(heavymach::AimMode)mode;
@end
namespace {
template<class T>T& perfStorage(id object,const char* name){
 Ivar ivar=class_getInstanceVariable(HMGameView.class,name);
 if(!ivar)throw std::runtime_error(name);
 return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>((__bridge void*)object)+ivar_getOffset(ivar));
}
double cpuMilliseconds(){return 1000.0*double(std::clock())/CLOCKS_PER_SEC;}
NSDictionary* metrics(const std::vector<double>& values){
 if(values.empty())return @{@"frames":@0,@"error":@"no samples"};
 auto sorted=values;std::sort(sorted.begin(),sorted.end());double sum=0;int over=0;
 for(double x:values){sum+=x;if(x>1000.0/60)++over;}
 return @{ @"mean_ms":@(sum/values.size()),@"p95_ms":@(sorted[(sorted.size()*95+99)/100-1]),@"max_ms":@(sorted.back()),@"over_60hz_budget":@(over),@"frames":@(values.size()) };
}
void emit(NSDictionary* record){NSData* json=[NSJSONSerialization dataWithJSONObject:record options:NSJSONWritingSortedKeys error:nil];NSLog(@"HM_FRAME_PERF %@",[[NSString alloc] initWithData:json encoding:NSUTF8StringEncoding]);}
}
@interface HMPerformanceFrame:NSObject
@property(nonatomic)CFTimeInterval timestamp;
@end
@implementation HMPerformanceFrame
@end
@interface HMDirectionTestKey:NSObject
@property(nonatomic)UIKeyboardHIDUsage keyCode;
@property(nonatomic)UIKeyModifierFlags modifierFlags;
@end
@implementation HMDirectionTestKey
@end
@interface HMDirectionTestPress:NSObject
@property(nonatomic,strong)HMDirectionTestKey* key;
@end
@implementation HMDirectionTestPress
@end
@interface HMPerformanceView:HMGameView
@end
@implementation HMPerformanceView
- (void)persistSave {} // No fixture or user save writes.
- (void)saveAimMode {} // Test option changes never persist.
@end
// Physical CoreMotion samples are in portrait body axes, independent of UI rotation.
@interface HMTestAcceleration:NSObject
@property(nonatomic)CMAcceleration suppliedAcceleration;
@end
@implementation HMTestAcceleration
- (CMAcceleration)acceleration {return self.suppliedAcceleration;}
@end
@interface HMTestMotion:NSObject
@property(nonatomic,strong)HMTestAcceleration* suppliedSample;
@end
@implementation HMTestMotion
- (BOOL)isAccelerometerAvailable {return YES;}
- (CMAccelerometerData*)accelerometerData {return (CMAccelerometerData*)self.suppliedSample;}
- (void)stopAccelerometerUpdates {}
- (void)startAccelerometerUpdates {}
@end
@interface HMPhysicalTiltView:HMPerformanceView
@property(nonatomic)UIInterfaceOrientation suppliedOrientation;
@end
@implementation HMPhysicalTiltView
- (UIInterfaceOrientation)accelerationInterfaceOrientation {return self.suppliedOrientation;}
@end
@interface HMLifecycleView:HMPerformanceView
@property(nonatomic)double suppliedTime;
@end
@implementation HMLifecycleView
- (double)interactionTime {return self.suppliedTime;}
@end
@interface HMCanvasTestView:HMPerformanceView
@property(nonatomic)UIEdgeInsets suppliedInsets;
@end
@implementation HMCanvasTestView
- (UIEdgeInsets)safeAreaInsets {return self.suppliedInsets;}
@end
@interface HMCanvasTouch:NSObject
@property(nonatomic)CGPoint point;
@end
@implementation HMCanvasTouch
- (CGPoint)locationInView:(UIView*)view {return self.point;}
@end
@interface HMPerformanceSilentMedia:HMMedia
@property(nonatomic)NSUInteger soundCalls;
@property(nonatomic)NSUInteger weaponFireCalls;
@property(nonatomic,copy)NSString* lastSound;
@end
@implementation HMPerformanceSilentMedia
- (void)sound:(NSString*)name {self.lastSound=name;++self.soundCalls;if([name hasPrefix:@"Wpn_"])++self.weaponFireCalls;}
- (void)sound:(NSString*)name pan:(float)pan {++self.soundCalls;if([name hasPrefix:@"Wpn_"])++self.weaponFireCalls;}
@end
// Display-link harness runs the actual presentation path and real audio.
@interface HMPerformanceAudibleMedia:HMMedia {
@public std::vector<double> soundTimes;
}
@end
@implementation HMPerformanceAudibleMedia
- (void)sound:(NSString*)name pan:(float)pan {double start=CACurrentMediaTime();[super sound:name pan:pan];soundTimes.push_back((CACurrentMediaTime()-start)*1000);}
@end
@interface HMDisplayProbe:NSObject {
@public std::vector<double> intervals,work,coldWork; CFTimeInterval previous; int count; NSUInteger maxBullets;
}
@property(nonatomic,strong) HMPerformanceView* view;
@property(nonatomic,strong) XCTestExpectation* done;
@property(nonatomic,strong) CADisplayLink* link;
@property(nonatomic)BOOL firing;
- (void)step:(CADisplayLink*)link;
@end
@implementation HMDisplayProbe
- (void)step:(CADisplayLink*)link {
 double now=CACurrentMediaTime();
 if(self.firing){auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(self.view,"_session");if(session){session->touch({470,160},1);maxBullets=std::max(maxBullets,session->bulletCount());}}
 double start=CACurrentMediaTime();[self.view frame:link];double elapsed=(CACurrentMediaTime()-start)*1000;
 if(count<30)coldWork.push_back(elapsed);
 if(count>=30){work.push_back(elapsed);if(previous)intervals.push_back((now-previous)*1000);}
 previous=now;++count;
 if(count==210){[link invalidate];self.link=nil;[self.done fulfill];}
}
@end
@interface HMProductionTimerView:HMPerformanceView {
@public std::vector<double> timerIntervals,timerWork,copyTimes,quadTimes,swapTimes,waitTimes;int timerCount;double previousTimer;
}
@property(nonatomic,strong)XCTestExpectation* timerDone;
@property(nonatomic)BOOL firing;
@property(nonatomic)BOOL autoAimExercise;
@end
@implementation HMProductionTimerView
- (void)frame:(id)sender {
 if(!self.timerDone){[super frame:sender];return;}
 double now=CACurrentMediaTime();
 if(self.autoAimExercise){
  if(timerCount==0)[self handleKeyCode:9 pressed:YES];
  if(timerCount==30)[self handleKeyCode:4 pressed:YES];
  if(timerCount==70){[self handleKeyCode:4 pressed:NO];[self handleKeyCode:7 pressed:YES];}
  if(timerCount==90){[self handleKeyCode:0x2c pressed:YES];[self handleKeyCode:0x2c pressed:NO];}
  if(timerCount==110)[self handleKeyCode:7 pressed:NO];
  if(timerCount==130){[self handleKeyCode:0x08 pressed:YES];[self handleKeyCode:0x08 pressed:NO];}
  if(timerCount==180)[self handleKeyCode:9 pressed:NO];
 }
 if(self.firing){auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(self,"_session");if(session)session->touch({470,160},1);}
 [super frame:sender];
 if(timerCount>=30){timerIntervals.push_back((now-previousTimer)*1000);timerWork.push_back((CACurrentMediaTime()-now)*1000);}
 if(timerCount>=30){NSDictionary* t=[[self valueForKey:@"glRenderer"]diagnostics][@"presentation_timings_ms"];copyTimes.push_back([t[@"copy"] doubleValue]);quadTimes.push_back([t[@"quad"] doubleValue]);swapTimes.push_back([t[@"swap"] doubleValue]);waitTimes.push_back([t[@"wait_and_restore"] doubleValue]);}
 previousTimer=now;
 if(++timerCount==210){[self setApplicationActive:NO];[[self valueForKey:@"frameTimer"]invalidate];[self.timerDone fulfill];self.timerDone=nil;}
}
@end
@interface HMFramePerformanceTests:XCTestCase
@end
@implementation HMFramePerformanceTests
- (void)testTitleAndSelectionScreensCoverWideAndTallCanvas {
 for(CGSize size:{CGSizeMake(695,320),CGSizeMake(480,360),CGSizeMake(960,320),CGSizeMake(569,320)}){@autoreleasepool{
  HMPerformanceView* view=[[HMPerformanceView alloc]initWithFrame:CGRectMake(0,0,size.width,size.height)];
  [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
  perfStorage<heavymach::SaveData>(view,"_save")={};[view updateCanvasGeometry];
  HMMedia* media=[view valueForKey:@"media"];HMGLRenderer* gl=[view valueForKey:@"glRenderer"];media.glRenderer=gl;
  XCTAssertTrue([gl setLogicalWidth:size.width height:size.height]);
  for(int screen=1;screen<=4;++screen){[view refreshScreen:screen];
   if(screen==1){auto& title=perfStorage<heavymach::TitleState>(view,"_title");title.bgScale=1;title.bgRotate=0;title.camX=title.camY=0;title.showTitle=title.showEpisode=title.showStart=true;title.titleScale=title.episodeScale=1;}
   XCTAssertTrue([gl beginFrame]);[view drawLogicalFrame];[gl finish];[gl endFrame];UIImage* picture=[gl snapshot];
   size_t w=CGImageGetWidth(picture.CGImage),h=CGImageGetHeight(picture.CGImage);std::vector<uint8_t> bytes(w*h*4);auto cs=CGColorSpaceCreateDeviceRGB();auto context=CGBitmapContextCreate(bytes.data(),w,h,8,w*4,cs,kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);CGContextDrawImage(context,CGRectMake(0,0,w,h),picture.CGImage);CGContextRelease(context);CGColorSpaceRelease(cs);
   for(size_t y:{size_t(0),h-1})for(size_t x:{size_t(0),w-1}){auto p=(y*w+x)*4;XCTAssertGreaterThan(int(bytes[p])+bytes[p+1]+bytes[p+2],0,@"Black uncovered corner screen%d %gx%g",screen,size.width,size.height);}
   if(size.width==695||size.height==360){XCTAttachment* a=[XCTAttachment attachmentWithImage:picture];a.name=[NSString stringWithFormat:@"screen%d-%gx%g",screen,size.width,size.height];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}
  }
  media.glRenderer=nil;[view setApplicationActive:NO];
 }}
}
- (void)testDoorOpeningRevealsWorldAtExpandedEdges {
 for(CGSize size:{CGSizeMake(695,320),CGSizeMake(480,360)}){@autoreleasepool{
  HMPerformanceView* view=[[HMPerformanceView alloc]initWithFrame:CGRectMake(0,0,size.width,size.height)];
  [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
  perfStorage<heavymach::SaveData>(view,"_save")={};[view updateCanvasGeometry];[view startSlot:0 mode:0 mission:0 stage:0];[view advanceOriginalFrame];
  auto& loading=perfStorage<heavymach::LoadingTransition>(view,"_loading");loading.changeScreen(6);
  HMMedia* media=[view valueForKey:@"media"];HMGLRenderer* gl=[view valueForKey:@"glRenderer"];media.glRenderer=gl;
  XCTAssertTrue([gl setLogicalWidth:size.width height:size.height]);std::vector<uint8_t> opening;
  for(int state:{3,5,6}){
   for(int n=0;n<120&&loading.state()!=state;++n)loading.tick({});XCTAssertEqual(loading.state(),state);
   // State 5 starts the vertical opening; sample after its middle has cleared.
   if(state==5)for(int n=0;n<12;++n)loading.tick({});
   XCTAssertTrue([gl beginFrame]);[view drawLogicalFrame];[gl finish];[gl endFrame];UIImage* picture=[gl snapshot];
   size_t w=CGImageGetWidth(picture.CGImage),h=CGImageGetHeight(picture.CGImage);std::vector<uint8_t> bytes(w*h*4);auto cs=CGColorSpaceCreateDeviceRGB();auto c=CGBitmapContextCreate(bytes.data(),w,h,8,w*4,cs,kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);CGContextDrawImage(c,CGRectMake(0,0,w,h),picture.CGImage);CGContextRelease(c);CGColorSpaceRelease(cs);
   if(state==5)opening=bytes;
   if(state==6)for(size_t x:{size_t(0),w-1}){size_t p=(h/2*w+x)*4;XCTAssertGreaterThan(int(bytes[p])+bytes[p+1]+bytes[p+2],0);}
   if(state==6){size_t p=(h/2*w+w/2)*4;for(int channel=0;channel<3;++channel)XCTAssertEqual(opening[p+channel],bytes[p+channel],@"World missing behind opening door");}
   XCTAttachment* a=[XCTAttachment attachmentWithImage:picture];a.name=[NSString stringWithFormat:@"door-world-state%d-%gx%g",state,size.width,size.height];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];
  }
  media.glRenderer=nil;[view setApplicationActive:NO];
 }}
}
- (void)testClosedDoorCoversCanvasOutsideHUDInMenusAndGame {
 auto pixels=[](UIImage* image){size_t w=CGImageGetWidth(image.CGImage),h=CGImageGetHeight(image.CGImage);std::vector<uint8_t> p(w*h*4);auto cs=CGColorSpaceCreateDeviceRGB();auto c=CGBitmapContextCreate(p.data(),w,h,8,w*4,cs,kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);CGContextDrawImage(c,CGRectMake(0,0,w,h),image.CGImage);CGContextRelease(c);CGColorSpaceRelease(cs);return p;};
 for(CGSize size:{CGSizeMake(695,320),CGSizeMake(480,360),CGSizeMake(960,320),CGSizeMake(569,320),CGSizeMake(320.0*912/420,320)})for(int game=0;game<2;++game){@autoreleasepool{
  HMPerformanceView* view=[[HMPerformanceView alloc]initWithFrame:CGRectMake(0,0,size.width,size.height)];
  [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
  if(game){perfStorage<heavymach::SaveData>(view,"_save")={};[view startSlot:0 mode:0 mission:0 stage:0];}
  perfStorage<CGFloat>(view,"_canvasWidth")=size.width;perfStorage<CGFloat>(view,"_canvasHeight")=size.height;
  auto& loading=perfStorage<heavymach::LoadingTransition>(view,"_loading");loading.changeScreen(game?3:6);
  for(int tick=0;tick<100&&loading.state()!=3;++tick)loading.tick({});XCTAssertEqual(loading.state(),3);
  HMMedia* media=[view valueForKey:@"media"];HMGLRenderer* gl=[view valueForKey:@"glRenderer"];[gl setLogicalWidth:size.width height:size.height];media.glRenderer=gl;
  std::vector<uint8_t> first;
  for(int pass=0;pass<2;++pass){
   UIGraphicsBeginImageContextWithOptions(CGSizeMake(2,2),YES,1);[(pass?UIColor.cyanColor:UIColor.magentaColor) setFill];UIRectFill(CGRectMake(0,0,2,2));UIImage* marker=UIGraphicsGetImageFromCurrentImageContext();UIGraphicsEndImageContext();
   [media resetClip];[media setCameraOffset:CGPointZero];[media setDrawingOffset:CGPointZero];[media setDrawingScale:1];
   XCTAssertTrue([gl beginFrame]);[gl drawImage:marker name:[NSString stringWithFormat:@"marker%d",pass] columns:1 rows:1 tile:0 position:CGPointMake(size.width/2,size.height/2) scale:CGSizeMake(size.width,size.height) rotation:0 alpha:1 blend:0 rgb565:YES];
   if(game){[media setDrawingOffset:CGPointMake((size.width-480)/2,0)];[media setDrawingScale:.9];[media setClipRect:CGRectMake((size.width-432)/2,16,432,288)];}
   [view drawLoading];[gl finish];[gl endFrame];UIImage* snapshot=[gl snapshot];auto current=pixels(snapshot);
   if(!pass)first=current;else {size_t exposed=0;for(size_t n=0;n<current.size();n+=4)if(std::abs(int(current[n])-first[n])>1||std::abs(int(current[n+1])-first[n+1])>1||std::abs(int(current[n+2])-first[n+2])>1){if(exposed<8)NSLog(@"HM_DOOR_GAP x=%zu y=%zu first=%d,%d,%d second=%d,%d,%d",n/4%CGImageGetWidth(snapshot.CGImage),n/4/CGImageGetWidth(snapshot.CGImage),first[n],first[n+1],first[n+2],current[n],current[n+1],current[n+2]);++exposed;}XCTAssertEqual(exposed,size_t(0),@"Door exposed %zu background pixels at %gx%g game=%d",exposed,size.width,size.height,game);NSLog(@"HM_DOOR_COVER %gx%g game=%d exposed=%zu",size.width,size.height,game,exposed);
    XCTAttachment* a=[XCTAttachment attachmentWithImage:snapshot];a.name=[NSString stringWithFormat:@"door-closed-%gx%g-game%d",size.width,size.height,game];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}
  }
  media.glRenderer=nil;[view setApplicationActive:NO];
 }}
}
- (void)testExpandedTitleAndSlotsPreserveOriginalArtAndUseMovedHitTargets {
 for(NSValue* value in @[[NSValue valueWithCGSize:CGSizeMake(480,320)],[NSValue valueWithCGSize:CGSizeMake(912,420)],[NSValue valueWithCGSize:CGSizeMake(956,440)]])for(int direction=0;direction<2;++direction){@autoreleasepool{
  CGSize size=value.CGSizeValue;HMCanvasTestView* view=[[HMCanvasTestView alloc]initWithFrame:CGRectMake(0,0,size.width,size.height)];
  [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
  view.suppliedInsets=size.width>480?(direction?UIEdgeInsetsMake(0,0,21,62):UIEdgeInsetsMake(0,62,21,0)):UIEdgeInsetsZero;
  [view layoutSubviews];CGFloat width=[[view valueForKey:@"canvasWidth"]doubleValue];
  auto& title=perfStorage<heavymach::TitleState>(view,"_title");title.bgScale=1;title.bgRotate=0;title.camX=title.camY=0;
  title.showTitle=title.showEpisode=title.showStart=false;
  HMGLRenderer* gl=[view valueForKey:@"glRenderer"];HMMedia* media=[view valueForKey:@"media"];
  XCTAssertTrue([gl setLogicalWidth:width height:320]);media.glRenderer=gl;
  auto render=[&](){XCTAssertTrue([gl beginFrame]);[view drawLogicalFrame];[gl finish];[gl endFrame];return [gl snapshot];};
  auto bytes=[](UIImage* image){CFDataRef data=CGDataProviderCopyData(CGImageGetDataProvider(image.CGImage));const uint8_t* p=CFDataGetBytePtr(data);std::vector<uint8_t> out(p,p+CFDataGetLength(data));CFRelease(data);return out;};
  UIImage* titleImage=render();auto actual=bytes(titleImage);
  XCTAssertTrue([gl beginFrame]);[media setDrawingOffset:CGPointZero];[media setDrawingScale:1];[media resetClip];[media setCameraOffset:CGPointZero];
  media.resourceScope=[view valueForKey:@"screenResourceScope"];
  [media setDrawingOffset:CGPointMake((width-480)/2,0)];[media draw:@"Main_BG.png" x:240 y:160];[gl finish];[gl endFrame];auto expected=bytes([gl snapshot]);
  int w=int(std::ceil(width)),maxDelta=0;
  for(int y=45;y<275;++y)for(int x=int(width/2)-225;x<int(width/2)+225;++x)for(int c=0;c<3;++c){size_t i=(y*w+x)*4+c;maxDelta=std::max(maxDelta,std::abs(int(actual[i])-int(expected[i])));}
  // Same one-channel rasterization rounding contract as the existing GL and
  // result-image oracles. Original asset bytes are verified separately by SHA.
  XCTAssertLessThanOrEqual(maxDelta,1,@"Original central artwork changed");
  if(width>480){int edgeBlack=0;for(int y=10;y<310;++y)for(int x:{4,w-5}){size_t i=(y*w+x)*4;if(actual[i]+actual[i+1]+actual[i+2]==0)++edgeBlack;}XCTAssertEqual(edgeBlack,0);}
  title.showTitle=title.showEpisode=title.showStart=true;title.titleScale=title.episodeScale=1;
  UIImage* decorated=render();XCTAttachment* a=[XCTAttachment attachmentWithImage:decorated];a.name=[NSString stringWithFormat:@"wide-title-%g-%d",size.width,direction];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];
  auto& save=perfStorage<heavymach::SaveData>(view,"_save");save={};for(auto& slot:save.slots){slot.bytes[8]=1;slot.setInt(4,1);}
  [view refreshScreen:3];[view updateCanvasGeometry];HMMenus* menus=[view valueForKey:@"menus"];
  UIImage* slots=render();a=[XCTAttachment attachmentWithImage:slots];a.name=[NSString stringWithFormat:@"wide-slots-%g-%d",size.width,direction];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];
  CGRect safe=perfStorage<CGRect>(view,"_menuSafeRect");
  auto tap=[&](CGPoint p){auto& vp=perfStorage<heavymach::Viewport>(view,"_viewport");HMCanvasTouch* t=[HMCanvasTouch new];t.point=CGPointMake(vp.x+p.x*vp.scale,vp.y+(320-p.y)*vp.verticalScale());NSSet* touches=[NSSet setWithObject:t];[view routeTouches:touches state:0];[view routeTouches:touches state:2];};
  for(int slot=0;slot<3;++slot){CGFloat x=width>480?CGRectGetMinX(safe)+82+(CGRectGetWidth(safe)-167)*slot/2:85+155*slot;tap(CGPointMake(x,142+(width>480?CGRectGetMinY(safe)+4:0)));XCTAssertEqual([[menus valueForKey:@"slot"]intValue],slot);render();}
  CGFloat bottom=width>480?std::max(CGFloat(21),CGRectGetMinY(safe)+22):20;
  tap(CGPointMake(width>480?CGRectGetMinX(safe)+58:332,bottom));XCTAssertTrue([[menus valueForKey:@"deleteConfirmation"]boolValue]);render();
  tap(CGPointMake((width>480?CGRectGetMidX(safe)-240:0)+307,108));XCTAssertFalse([[menus valueForKey:@"deleteConfirmation"]boolValue]);XCTAssertTrue(save.slots[2].occupied());
  render();tap(CGPointMake(width>480?CGRectGetMaxX(safe)-58:422,bottom));
  XCTAssertEqual([[menus valueForKey:@"gameSlot"]intValue],2);
  NSUInteger decodes=[media.cacheDiagnostics[@"decodeCount"]unsignedIntegerValue];for(int i=0;i<12;++i)render();XCTAssertEqual([media.cacheDiagnostics[@"decodeCount"]unsignedIntegerValue],decodes);
  XCTAssertEqual([media.cacheDiagnostics[@"decodedBytes"]unsignedIntegerValue],NSUInteger(0));
  for(int i=0;i<160;++i)[view advanceOriginalFrame];render();
  CGFloat center=width>480?CGRectGetMidX(safe):240;
  tap(CGPointMake(center,104));XCTAssertEqual([[menus valueForKey:@"mode"]intValue],1);render();
  tap(CGPointMake(center,180));XCTAssertEqual([[menus valueForKey:@"mode"]intValue],0);render();
  a=[XCTAttachment attachmentWithImage:render()];a.name=[NSString stringWithFormat:@"wide-mode-%g-%d",size.width,direction];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];
  tap(CGPointMake(center+194,56));for(int i=0;i<160;++i)[view advanceOriginalFrame];render();
  a=[XCTAttachment attachmentWithImage:render()];a.name=[NSString stringWithFormat:@"wide-mission-%g-%d",size.width,direction];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];
  tap(CGPointMake(center+198,width>480?std::max(CGFloat(26),CGRectGetMinY(safe)+22):26));
  for(int i=0;i<110;++i)[view advanceOriginalFrame];
  XCTAssertTrue(bool(perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session")));
  XCTAssertEqualObjects([gl diagnostics][@"lastError"],@"");
  NSLog(@"HM_MENU_EXTENSION width=%g direction=%d centralDelta=%d decodeCount=%lu diagnostics=%@",width,direction,maxDelta,(unsigned long)decodes,[gl diagnostics]);
  [view setApplicationActive:NO];
 }}
}
// Full burst checks use every original player weapon, including all three sky strikes.
- (void)testAllWeaponsKeepTargetsAcrossTouchRelease {
 for(NSValue* value in @[[NSValue valueWithCGSize:CGSizeMake(480,320)],[NSValue valueWithCGSize:CGSizeMake(912,420)],[NSValue valueWithCGSize:CGSizeMake(956,440)]]){@autoreleasepool{
  CGSize size=value.CGSizeValue;
  HMCanvasTestView* view=[[HMCanvasTestView alloc]initWithFrame:CGRectMake(0,0,size.width,size.height)];
  [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
  perfStorage<heavymach::SaveData>(view,"_save")={};[view startSlot:0 mode:0 mission:0 stage:0];[view layoutSubviews];
  auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
  auto& data=perfStorage<heavymach::GameData>(view,"_data");
  auto& slot=perfStorage<heavymach::SaveData>(view,"_save").slots[0];
  const auto names=heavymach::validationWeapons(data);XCTAssertGreaterThan(names.size(),size_t(3));
  for(const auto& name:names)for(int mode=0;mode<6;++mode){
   heavymach::ValidationScenario scenario;scenario.weapons={name};scenario.internalLevel=99;scenario.grades={6,6,0};
   session->startValidation(scenario);int selected=-1;for(int i=0;i<slot.weaponCount();++i)if(slot.weaponName(i)==name)selected=i;
   XCTAssertGreaterThanOrEqual(selected,0);session->selectWeapon(selected);
   for(int tick=0;tick<30;++tick)[view advanceOriginalFrame];
   const auto& info=data.weapons.at(name);int charge=slot.weaponCharge(selected),shots=0,badTargets=0,badTrajectories=0;
   heavymach::Vector2 expected{};
   session->validationObserveShots([&](const heavymach::WeaponInfo& weapon,float angle,heavymach::Vector2 position){
    ++shots;auto target=session->validationWeaponState().fireTarget;
    if(std::abs(target.x-expected.x)>.002f||std::abs(target.y-expected.y)>.002f)++badTargets;
    if(weapon.wpnType==5){
     if(weapon.spd==0){if(std::abs(position.x-expected.x)>50.001f)++badTrajectories;}
     else {float aim=std::atan2(expected.y-position.y,expected.x-position.x);if(std::abs(angle-aim)>.10001f)++badTrajectories;}
    }
   });
   HMCanvasTouch* touch=[HMCanvasTouch new];touch.point=CGPointMake(size.width-(mode==0?size.width*.18:12),size.height*.35);
   NSSet* touches=[NSSet setWithObject:touch];[view routeTouches:touches state:0];expected=session->validationWeaponState().fireTarget;
   XCTAssertTrue(session->validationWeaponState().isFire,@"%s width%g mode%d",name.c_str(),size.width,mode);
   // Wait for the first real shot, then release while the original burst continues.
   for(int tick=0;tick<120&&shots==0;++tick)[view advanceOriginalFrame];
   XCTAssertGreaterThan(shots,0,@"%s",name.c_str());
   if(mode==0){touch.point=CGPointMake(touch.point.x,touch.point.y+size.height*.15);[view routeTouches:touches state:1];expected=session->validationWeaponState().fireTarget;}
   // Real release uses current camera/muzzle; outside/cancel must retain last world target.
   if(mode<2){auto logical=perfStorage<heavymach::Viewport>(view,"_viewport").logical(touch.point.x,touch.point.y);auto origin=session->validationAimOrigin();auto p=session->player()->position;expected={p.x+float(logical->first)-origin.x,p.y+float(logical->second)-origin.y};}
   if(mode==2)touch.point=CGPointMake(size.width+30,touch.point.y);
   if(mode==4){[view gameButton:100 state:2];[view gameButton:101 state:2];}
   else if(mode==5){HMDirectionTestPress* press=[HMDirectionTestPress new];press.key=[HMDirectionTestKey new];press.key.keyCode=UIKeyboardHIDUsageKeyboardF;[view pressesEnded:(id)[NSSet setWithObject:press] withEvent:nil];[view routeTouches:touches state:3];}
   else [view routeTouches:touches state:mode==3?3:2];
   auto released=session->validationWeaponState();
   XCTAssertFalse(released.isFire);XCTAssertEqualWithAccuracy(released.fireTarget.x,expected.x,.002,@"%s width%g mode%d",name.c_str(),size.width,mode);
   XCTAssertEqualWithAccuracy(released.fireTarget.y,expected.y,.002);
   const int ticks=int(std::ceil((info.shotCnt*std::max(.017f,info.shotDly)+info.delay+.5f)*60));
   for(int tick=0;tick<ticks;++tick)[view advanceOriginalFrame];
   XCTAssertEqual(shots,info.shotCnt,@"%s width%g mode%d",name.c_str(),size.width,mode);
   XCTAssertEqual(badTargets,0,@"%s width%g mode%d",name.c_str(),size.width,mode);XCTAssertEqual(badTrajectories,0,@"%s",name.c_str());
   XCTAssertEqual(slot.weaponCharge(selected),selected?charge-1:charge,@"%s",name.c_str());
   emit(@{@"check":@"all_weapon_burst",@"weapon":@(name.c_str()),@"width":@(size.width),@"mode":@(mode),@"shots":@(shots),@"expected_shots":@(info.shotCnt),@"bad_targets":@(badTargets),@"bad_trajectories":@(badTrajectories)});
   session->validationObserveShots({});
  }
  [view setApplicationActive:NO];
 }}
}
- (void)testResultIllustrationsCoverExpandedCanvas {
 for(NSValue* value in @[[NSValue valueWithCGSize:CGSizeMake(480,320)],[NSValue valueWithCGSize:CGSizeMake(912,420)],[NSValue valueWithCGSize:CGSizeMake(956,440)]]){@autoreleasepool{
  CGSize size=value.CGSizeValue;
  HMCanvasTestView* view=[[HMCanvasTestView alloc]initWithFrame:CGRectMake(0,0,size.width,size.height)];view.suppliedInsets=UIEdgeInsetsMake(0,62,21,0);
  [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
  perfStorage<heavymach::SaveData>(view,"_save")={};[view startSlot:0 mode:0 mission:0 stage:0];[view layoutSubviews];
  for(int tick=0;tick<90;++tick)[view advanceOriginalFrame];
  auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
  auto& world=const_cast<heavymach::WorldState&>(session->world());world.paused=true;
  HMGLRenderer* gl=[view valueForKey:@"glRenderer"];HMMedia* media=[view valueForKey:@"media"];
  float width=session->canvasWidth();XCTAssertTrue([gl setLogicalWidth:width height:320]);
  auto pixels=[](UIImage* image){CFDataRef data=CGDataProviderCopyData(CGImageGetDataProvider(image.CGImage));const auto* p=CFDataGetBytePtr(data);std::vector<uint8_t> bytes(p,p+CFDataGetLength(data));CFRelease(data);return bytes;};
  NSMutableArray* assets=[NSMutableArray arrayWithObject:@"StageClear.jpg"];for(int mission=1;mission<=7;++mission){[assets addObject:[NSString stringWithFormat:@"GameLoading_%02d.jpg",mission]];[assets addObject:[NSString stringWithFormat:@"GameLoading_%02dE.jpg",mission]];}
  for(NSString* asset in assets){
   world.presentation={};auto& p=world.presentation;p.step=2;p.alpha=1;p.scale=1;p.rotation=-1;p.buttonY=0;p.loadingAsset=asset.UTF8String;p.typoFrame=4;
   media.glRenderer=gl;XCTAssertTrue([gl beginFrame]);[view drawLogicalFrame];[gl finish];[gl endFrame];UIImage* actualImage=[gl snapshot];auto actual=pixels(actualImage);
   [media setCameraOffset:CGPointZero];[media setDrawingOffset:CGPointZero];[media setDrawingScale:1];[media resetClip];
   CGSize original=[media imageSize:asset];double scale=width>480?std::max(double(width)/(original.width-1),320./(original.height-1)):1;
   XCTAssertTrue([gl beginFrame]);[media draw:asset x:width/2 y:160 columns:1 rows:1 tile:0 scaleX:scale scaleY:scale rotation:0 alpha:1];[gl finish];[gl endFrame];auto expected=pixels([gl snapshot]);
   // Same one-channel rounding contract as HMGLReferenceTests' independent CGL oracle.
   size_t w=std::ceil(width);int maximumDelta=0;
   for(int y=100;y<220;++y)for(size_t x=4;x<w-4;++x)if(x<35||x>w-36)for(int c=0;c<3;++c){size_t index=(y*w+x)*4+c;maximumDelta=std::max(maximumDelta,std::abs(int(actual[index])-int(expected[index])));}
   XCTAssertLessThanOrEqual(maximumDelta,1,@"%@ width%g differs from full-canvas image",asset,width);
   if([asset isEqualToString:@"GameLoading_01E.jpg"]){XCTAttachment* capture=[XCTAttachment attachmentWithImage:actualImage];capture.name=[NSString stringWithFormat:@"result-full-canvas-%g",width];capture.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:capture];}
   // Exact coverage oracle: same dimensions, opaque constant color has no image interpolation noise.
   UIGraphicsBeginImageContextWithOptions(original,YES,1);[UIColor.magentaColor setFill];UIRectFill(CGRectMake(0,0,original.width,original.height));UIImage* marker=UIGraphicsGetImageFromCurrentImageContext();UIGraphicsEndImageContext();
   [gl releaseTextureScope:@"shared"];
   XCTAssertTrue([gl prepareImage:marker name:[@"shared|" stringByAppendingString:asset] rgb565:YES]);
   XCTAssertTrue([gl beginFrame]);[view drawLogicalFrame];[gl finish];[gl endFrame];auto covered=pixels([gl snapshot]);
   CGPoint offset=perfStorage<CGPoint>(view,"_hudOffset");CGFloat hudScale=perfStorage<CGFloat>(view,"_hudScale");
   size_t uncovered=0,checked=0;
   for(int y=0;y<320;++y)for(size_t x=0;x<w;++x){double lx=(x+.5)*width/w,ly=319.5-y;
    bool outside=lx<offset.x||lx>offset.x+480*hudScale||ly<offset.y||ly>offset.y+320*hudScale;
    if(outside||(y>=100&&y<220&&(x>=4&&x<35||x>w-36&&x<w-4))){++checked;size_t index=(y*w+x)*4;if(covered[index]!=255||covered[index+1]!=0||covered[index+2]!=255)++uncovered;}
   }
   XCTAssertGreaterThan(checked,size_t(1000));XCTAssertEqual(uncovered,size_t(0),@"%@ width%g exposes world outside result",asset,width);
   world.gameState=1;
   // Existing result button stays at the safe-area HUD coordinate after the image draw.
   auto viewport=perfStorage<heavymach::Viewport>(view,"_viewport");HMCanvasTouch* button=[HMCanvasTouch new];button.point=CGPointMake(viewport.x+(offset.x+120*hudScale)*viewport.scale,viewport.y+(320-offset.y-26*hudScale)*viewport.verticalScale());
   NSSet* buttonTouches=[NSSet setWithObject:button];[view routeTouches:buttonTouches state:0];[view routeTouches:buttonTouches state:2];XCTAssertEqual(world.presentation.step,3,@"%@ result button",asset);
   [gl releaseTextureScope:@"shared"];
   media.glRenderer=nil;
  }
  [view setApplicationActive:NO];
 }}
}
- (void)testNewWideSessionClampsRetainedCameraBeforeOpeningMap {
 HMPerformanceView* view=[[HMPerformanceView alloc]initWithFrame:CGRectMake(0,0,912,420)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 perfStorage<heavymach::SaveData>(view,"_save")={};
 perfStorage<heavymach::Vector2>(view,"_retainedWorldCamera")={10000,160};
 [view startSlot:0 mode:0 mission:0 stage:0];
 auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
 XCTAssertGreaterThan(session->canvasWidth(),480);
 XCTAssertEqualWithAccuracy(session->world().camera.x,session->world().map.left+session->canvasWidth()/2,.001);
 XCTAssertLessThan(session->enemyCount(),size_t(10));
 [view setApplicationActive:NO];
}
- (void)testMissionThreeAircraftWaitsForOriginalEncounter {
 for(NSValue* value in @[[NSValue valueWithCGSize:CGSizeMake(480,320)],[NSValue valueWithCGSize:CGSizeMake(912,420)],[NSValue valueWithCGSize:CGSizeMake(956,440)]]){@autoreleasepool{
  CGSize size=value.CGSizeValue;HMCanvasTestView* view=[[HMCanvasTestView alloc]initWithFrame:CGRectMake(0,0,size.width,size.height)];
  [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
  perfStorage<heavymach::SaveData>(view,"_save")={};[view startSlot:0 mode:0 mission:2 stage:4];[view layoutSubviews];
  auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
  auto& data=perfStorage<heavymach::GameData>(view,"_data");
  XCTAssertEqual(heavymach::GlobalInfo::mapNumber({2,4}),14);
  XCTAssertTrue(data.machines.at("Boss05").parts.front().name=="E_Boss02");
  heavymach::ValidationScenario scenario;scenario.mission=2;scenario.stage=4;scenario.internalLevel=99;scenario.grades={6,6,0};
  float x=530,y=session->world().mapHeight(x);scenario.playerPosition=heavymach::Vector2{x,y==10000?180:y+40};
  session->startValidation(scenario);session->validationSetCheats(true);
  bool expanded=session->canvasWidth()>480;
  auto aircraft=[&](){auto enemies=session->validationEnemies();decltype(enemies) result;for(const auto& e:enemies)if(e.boss&&e.name=="Boss05")result.push_back(e);return result;};
  for(int tick=1;tick<=1800;++tick){session->tick(double(tick)/60,0);session->takeEvents();auto plane=aircraft();
   XCTAssertEqual(plane.size(),expanded?size_t(1):size_t(0));
   if(!plane.empty()){XCTAssertEqual(plane[0].position.x,1180);XCTAssertEqual(plane[0].position.y,27.75f);}
   XCTAssertEqual(session->world().bossEncounter,0);
  }
  for(int tick=1801;tick<=2400;++tick){session->tick(double(tick)/60,tick<=2100?.2f:0.f);session->takeEvents();}
  auto plane=aircraft();XCTAssertEqual(plane.size(),size_t(1));XCTAssertEqual(session->world().bossEncounter,1);
  if(!plane.empty())XCTAssertTrue(plane[0].position.x!=1180||plane[0].position.y!=27.75f);
  XCTAssertEqual(session->world().map.right,1459);XCTAssertEqual(session->world().frontLine,1330);
  bool rendered=false;for(const auto& sprite:session->sprites())rendered|=sprite.tile=="E_Boss02.png";XCTAssertTrue(rendered);
  HMGLRenderer* gpu=[view valueForKey:@"glRenderer"];HMMedia* media=[view valueForKey:@"media"];
  XCTAssertTrue([gpu setLogicalWidth:session->canvasWidth() height:320]);XCTAssertTrue([gpu beginFrame]);media.glRenderer=gpu;[view drawLogicalFrame];media.glRenderer=nil;[gpu finish];[gpu endFrame];
  XCTAssertEqualObjects(gpu.diagnostics[@"lastError"],@"");
  XCTAttachment* capture=[XCTAttachment attachmentWithImage:[gpu snapshot]];capture.name=[NSString stringWithFormat:@"mission3-stage5-aircraft-%gx%g",size.width,size.height];capture.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:capture];
  [view setApplicationActive:NO];
 }}
}
- (void)testExpandedAirAndProMaxCanvasPreservesHeightAndKeepsHUDInsideSafeArea {
 for(NSValue* size in @[[NSValue valueWithCGSize:CGSizeMake(912,420)],[NSValue valueWithCGSize:CGSizeMake(956,440)]]){
  for(NSNumber* reversed in @[@NO,@YES]){@autoreleasepool{
   const CGSize dimensions=size.CGSizeValue;
   HMCanvasTestView* view=[[HMCanvasTestView alloc]initWithFrame:CGRectMake(0,0,dimensions.width,dimensions.height)];
   view.suppliedInsets=reversed.boolValue?UIEdgeInsetsMake(0,0,21,62):UIEdgeInsetsMake(0,62,21,0);
   [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
   perfStorage<heavymach::SaveData>(view,"_save")={};[view startSlot:0 mode:0 mission:0 stage:0];[view layoutSubviews];
   auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
   auto& viewport=perfStorage<heavymach::Viewport>(view,"_viewport");
   XCTAssertEqualWithAccuracy(session->canvasWidth(),320*dimensions.width/dimensions.height,.001);
   XCTAssertEqualWithAccuracy(viewport.x,0,.001);XCTAssertEqualWithAccuracy(viewport.y,0,.001);
   XCTAssertEqualWithAccuracy(viewport.logicalWidth*viewport.scale,dimensions.width,.001);
   XCTAssertEqualWithAccuracy(320*viewport.verticalScale(),dimensions.height,.001);
   CGPoint offset=perfStorage<CGPoint>(view,"_hudOffset");CGFloat scale=perfStorage<CGFloat>(view,"_hudScale");
   CGRect safe=UIEdgeInsetsInsetRect(view.bounds,view.safeAreaInsets);
   CGRect hud=CGRectMake(viewport.x+offset.x*viewport.scale,viewport.y+(320-offset.y-320*scale)*viewport.verticalScale(),480*scale*viewport.scale,320*scale*viewport.verticalScale());
   XCTAssertTrue(CGRectContainsRect(CGRectInset(safe,-.01,-.01),hud));
   for(int i=0;i<90;++i)[view advanceOriginalFrame];
   HMGLRenderer* gpu=[view valueForKey:@"glRenderer"];HMMedia* media=[view valueForKey:@"media"];
   XCTAssertTrue([gpu setLogicalWidth:session->canvasWidth() height:320]);XCTAssertTrue([gpu beginFrame]);media.glRenderer=gpu;[view drawLogicalFrame];media.glRenderer=nil;[gpu endFrame];
   UIImage* image=[gpu snapshot];XCTAssertEqual(CGImageGetWidth(image.CGImage),size_t(std::ceil(session->canvasWidth())));XCTAssertEqual(CGImageGetHeight(image.CGImage),size_t(320));
   XCTAssertEqualObjects(gpu.diagnostics[@"lastError"],@"");
   XCTAttachment* capture=[XCTAttachment attachmentWithImage:image];capture.name=[NSString stringWithFormat:@"canvas-%gx%g-reversed%@",dimensions.width,dimensions.height,reversed];capture.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:capture];
   [view setApplicationActive:NO];
  }}
 }
}
- (void)testExpandedEdgeTouchFiresAndOutsideReleaseAndModalStopIt {
 HMCanvasTestView* view=[[HMCanvasTestView alloc]initWithFrame:CGRectMake(0,0,912,420)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 perfStorage<heavymach::SaveData>(view,"_save")={};[view startSlot:0 mode:0 mission:0 stage:0];[view layoutSubviews];
 auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
 for(int i=0;i<30;++i)[view advanceOriginalFrame];
 HMCanvasTouch* touch=[HMCanvasTouch new];touch.point=CGPointMake(900,210);NSSet* touches=[NSSet setWithObject:touch];
 [view routeTouches:touches state:0];
 size_t maximum=0;for(int i=0;i<40;++i){[view advanceOriginalFrame];maximum=std::max(maximum,session->bulletCount());}
 XCTAssertGreaterThan(maximum,size_t(0));XCTAssertEqual(((NSHashTable*)[view valueForKey:@"worldTouches"]).count,NSUInteger(1));
 touch.point=CGPointMake(950,210);[view routeTouches:touches state:2];
 XCTAssertEqual(((NSHashTable*)[view valueForKey:@"worldTouches"]).count,NSUInteger(0));
 [view gameButton:100 state:2];touch.point=CGPointMake(900,210);[view routeTouches:touches state:0];
 XCTAssertEqual(((NSHashTable*)[view valueForKey:@"worldTouches"]).count,NSUInteger(0));
 const double time=session->world().gameTime;for(int i=0;i<30;++i)[view advanceOriginalFrame];XCTAssertEqual(session->world().gameTime,time);
 [view routeTouches:touches state:3];[view setApplicationActive:NO];
}
- (void)testCanvasComparisonModeChangesOnResumeWithoutExtraTick {
 HMPerformanceView* view=[[HMPerformanceView alloc]initWithFrame:CGRectMake(0,0,912,420)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 perfStorage<heavymach::SaveData>(view,"_save")={};[view startSlot:0 mode:0 mission:0 stage:0];
 auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");[view advanceOriginalFrame];
 [view gameButton:100 state:2];double time=session->world().gameTime;float width=session->canvasWidth();
 [view setWideCanvasEnabled:NO];XCTAssertEqual(session->canvasWidth(),width);XCTAssertEqual(session->world().gameTime,time);
 [view gameButton:101 state:2];[view updateCanvasGeometry];XCTAssertEqual(session->canvasWidth(),480);XCTAssertEqual(session->world().gameTime,time);
 [view advanceOriginalFrame];XCTAssertGreaterThan(session->world().gameTime,time);
 [view setApplicationActive:NO];
}
- (void)testWeaponSwitchReleasePlaysOriginalSoundOnlyWhenAllowed {
 HMPerformanceView* view=[[HMPerformanceView alloc]initWithFrame:CGRectMake(0,0,480,320)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 perfStorage<heavymach::SaveData>(view,"_save")={};[view startSlot:0 mode:0 mission:0 stage:0];
 HMPerformanceSilentMedia* media=[HMPerformanceSilentMedia new];[view setValue:media forKey:@"media"];
 [view gameButton:1100 state:0];[view gameButton:1100 state:1];[view gameButton:1100 state:3];
 XCTAssertEqual(media.soundCalls,NSUInteger(0));
 [view gameButton:1100 state:2];XCTAssertEqual(media.soundCalls,NSUInteger(1));
 XCTAssertEqualObjects(media.lastSound,@"Game_ChangeWeapon");
 [view gameButton:100 state:2];NSUInteger before=media.soundCalls;
 [view gameButton:1100 state:2];XCTAssertEqual(media.soundCalls,before);
 [view setApplicationActive:NO];
}
- (void)testResignBackgroundAndReturnKeepGameFrozenUntilExplicitResume {
 HMLifecycleView* view=[[HMLifecycleView alloc]initWithFrame:CGRectMake(0,0,912,420)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 perfStorage<heavymach::SaveData>(view,"_save")={};[view startSlot:0 mode:0 mission:0 stage:0];
 auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
 [view advanceOriginalFrame];const double time=session->world().gameTime;
 const auto position=session->player()->position;
 [view setValue:@YES forKey:@"keyboardFire"];[view setValue:@YES forKey:@"rightPressed"];
 [view setApplicationActive:NO];
 XCTAssertTrue(session->world().paused);XCTAssertTrue([[view valueForKey:@"gameMenu"]boolValue]);
 XCTAssertFalse([[view valueForKey:@"keyboardFire"]boolValue]);XCTAssertFalse([[view valueForKey:@"rightPressed"]boolValue]);
 XCTAssertNil([view valueForKey:@"frameTimer"]);XCTAssertNil([view valueForKey:@"motionTimer"]);
 HMOpenALAudio* audio=((HMMedia*)[view valueForKey:@"media"]).audio;
 NSUInteger stops=[audio.diagnostics[@"stopAllCount"]unsignedIntegerValue];
 [view setApplicationActive:NO];XCTAssertEqual([audio.diagnostics[@"stopAllCount"]unsignedIntegerValue],stops);
 view.suppliedTime=7200;[view setApplicationActive:YES];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 XCTAssertTrue(session->world().paused);
 for(int i=0;i<120;++i)[view advanceOriginalFrame];
 XCTAssertEqual(session->world().gameTime,time);XCTAssertEqual(session->player()->position.x,position.x);
 [view gameButton:101 state:2];XCTAssertFalse(session->world().paused);[view advanceOriginalFrame];
 XCTAssertGreaterThan(session->world().gameTime,time);XCTAssertLessThan(session->world().gameTime,time+.02);
 [view setApplicationActive:NO];
}
- (void)testForegroundInactivityUsesRealTimeAndHeldInputResetsDeadline {
 HMLifecycleView* view=[[HMLifecycleView alloc]initWithFrame:CGRectMake(0,0,480,320)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 perfStorage<heavymach::SaveData>(view,"_save")={};[view startSlot:0 mode:0 mission:0 stage:0];
 auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
 view.suppliedTime=119.99;[view checkInactivity];XCTAssertFalse(session->world().paused);
 [view setValue:@YES forKey:@"keyboardFire"];view.suppliedTime=180;[view checkInactivity];XCTAssertFalse(session->world().paused);
 [view setValue:@NO forKey:@"keyboardFire"];view.suppliedTime=299.99;[view checkInactivity];XCTAssertFalse(session->world().paused);
 view.suppliedTime=300;[view checkInactivity];XCTAssertTrue(session->world().paused);
 XCTAssertTrue([[view valueForKey:@"gameMenu"]boolValue]);
 [view gameButton:101 state:2];view.suppliedTime=419;[view checkInactivity];XCTAssertFalse(session->world().paused);
 [view setApplicationActive:NO];
}
- (void)testInterruptedLoadingHasInitialWorldAndStageTransitionCanResume {
 HMLifecycleView* view=[[HMLifecycleView alloc]initWithFrame:CGRectMake(0,0,480,320)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 perfStorage<heavymach::SaveData>(view,"_save")={};[view setValue:@YES forKey:@"resumeRequired"];
 [view startSlot:0 mode:0 mission:0 stage:0];
 auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
 XCTAssertTrue(session->world().paused);XCTAssertFalse(session->renderItems().empty());XCTAssertEqual(session->world().gameTime,0);
 [view gameButton:101 state:2];
 auto& world=const_cast<heavymach::WorldState&>(session->world());
 world.gameState=1;world.paused=true;world.presentation.step=6;world.presentation.alpha=.001;
 [view setApplicationActive:NO];[view setApplicationActive:YES];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 for(int i=0;i<60;++i)[view advanceOriginalFrame];
 XCTAssertEqual(world.presentation.step,6);XCTAssertTrue([[view valueForKey:@"gameMenu"]boolValue]);
 [view gameButton:101 state:2];[view advanceOriginalFrame];
 XCTAssertEqual(world.gameState,0);XCTAssertFalse(world.paused);XCTAssertFalse([[view valueForKey:@"resumeRequired"]boolValue]);
 [view advanceOriginalFrame];XCTAssertGreaterThan(world.gameTime,0);
 [view setApplicationActive:NO];
}
- (void)testPhysicalTiltMovesTowardLowerScreenEdgeInBothLandscapeOrientations {
 for(NSNumber* orientation in @[@(UIInterfaceOrientationLandscapeLeft),@(UIInterfaceOrientationLandscapeRight)]){@autoreleasepool{
  HMPhysicalTiltView* view=[[HMPhysicalTiltView alloc]initWithFrame:CGRectMake(0,0,912,420)];
  view.suppliedOrientation=(UIInterfaceOrientation)orientation.integerValue;
  [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
  [(CMMotionManager*)[view valueForKey:@"motion"]stopAccelerometerUpdates];
  HMTestMotion* motion=[HMTestMotion new];motion.suppliedSample=[HMTestAcceleration new];[view setValue:motion forKey:@"motion"];
  auto& save=perfStorage<heavymach::SaveData>(view,"_save");save={};[view startSlot:0 mode:0 mission:0 stage:0];
  auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");XCTAssertTrue(bool(session));if(!session)return;
  // Home-left: portrait +Y points right. Home-right: portrait +Y points left.
  const double rightY=view.suppliedOrientation==UIInterfaceOrientationLandscapeLeft?.2:-.2;
  motion.suppliedSample.suppliedAcceleration=CMAcceleration{0,rightY,-.98};
  const float start=session->player()->position.x;
  for(int i=0;i<120;++i){[view sampleOriginalAcceleration];[view advanceOriginalFrame];}
  const float right=session->player()->position.x;
  XCTAssertGreaterThan(right,start+10,@"Right edge down, orientation %@",orientation);
  motion.suppliedSample.suppliedAcceleration=CMAcceleration{0,-rightY,-.98};
  for(int i=0;i<90;++i){[view sampleOriginalAcceleration];[view advanceOriginalFrame];}
  XCTAssertLessThan(session->player()->position.x,right-10,@"Left edge down, orientation %@",orientation);
  // Screen-relative buttons must not be reversed a second time.
  HMValidationControls* controls=[view valueForKey:@"validationControls"];controls.selectedSegmentIndex=2;
  perfStorage<heavymach::TiltState>(view,"_tilt")={};[view sampleOriginalAcceleration];
  XCTAssertGreaterThan(perfStorage<heavymach::TiltState>(view,"_tilt").y,0);
  [view setApplicationActive:NO];
 }}
}
- (void)testValidationDirectionControlsMoveRealPlayerAndReleaseOnBackground {
 HMPerformanceView* view=[[HMPerformanceView alloc]initWithFrame:CGRectMake(0,0,912,420)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 [(CMMotionManager*)[view valueForKey:@"motion"]stopAccelerometerUpdates];
 auto& save=perfStorage<heavymach::SaveData>(view,"_save");save={};
 [view startSlot:0 mode:0 mission:0 stage:0];XCTAssertNil([view valueForKey:@"loadError"]);
 auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
 XCTAssertTrue(bool(session));if(!session)return;
 HMValidationControls* controls=[view valueForKey:@"validationControls"];
 XCTAssertEqual(controls.numberOfSegments,NSInteger(4));XCTAssertEqual(controls.selectedSegmentIndex,NSInteger(3));
 const float start=session->player()->position.x;
 auto tap=[&](int identifier){CGRect r=[view gameButtonRect:identifier];XCTAssertFalse(CGRectIsEmpty(r));auto vp=perfStorage<heavymach::Viewport>(view,"_viewport");CGPoint p=CGPointMake(vp.x+CGRectGetMidX(r)*vp.scale,vp.y+(vp.logicalHeight-CGRectGetMidY(r))*vp.verticalScale());id token=[NSObject new];[view routePoint:p state:0 token:token];[view routePoint:p state:2 token:token];};
 tap(3502);
 for(int i=0;i<120;++i){[view sampleOriginalAcceleration];[view advanceOriginalFrame];}
 const float right=session->player()->position.x;XCTAssertGreaterThan(right,start+10);
 tap(3500);
 for(int i=0;i<90;++i){[view sampleOriginalAcceleration];[view advanceOriginalFrame];}
 XCTAssertLessThan(session->player()->position.x,right-10);
 [view gameUIAction:3500+1 state:2];
 for(int i=0;i<100;++i)[view sampleOriginalAcceleration];
 XCTAssertEqualWithAccuracy(perfStorage<heavymach::TiltState>(view,"_tilt").y,0,.00001);
 [view gameUIAction:3500+2 state:2];
 [view gameButton:100 state:2];XCTAssertEqual(controls.selectedSegmentIndex,NSInteger(1));
 [view gameButton:101 state:2];XCTAssertEqual(controls.selectedSegmentIndex,NSInteger(1));
 [view gameUIAction:3500+2 state:2];
 [view setApplicationActive:NO];XCTAssertEqual(controls.selectedSegmentIndex,NSInteger(1));
 [view gameUIAction:3500+3 state:2];
 [view setApplicationActive:NO];XCTAssertEqual(controls.selectedSegmentIndex,NSInteger(3));
}
- (void)testValidationKeyboardReleaseAndCancellationClearDirection {
 HMPerformanceView* view=[[HMPerformanceView alloc]initWithFrame:CGRectMake(0,0,912,420)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 HMDirectionTestPress* press=[HMDirectionTestPress new];press.key=[HMDirectionTestKey new];press.key.keyCode=UIKeyboardHIDUsageKeyboardD;
 NSSet* presses=[NSSet setWithObject:press];
 HMValidationControls* controls=[view valueForKey:@"validationControls"];
 [view pressesBegan:presses withEvent:nil];[view sampleOriginalAcceleration];
 XCTAssertEqual(controls.selectedSegmentIndex,NSInteger(3));XCTAssertGreaterThan(perfStorage<heavymach::TiltState>(view,"_tilt").y,0);
 [view pressesEnded:presses withEvent:nil];XCTAssertEqual(controls.selectedSegmentIndex,NSInteger(3));
 XCTAssertFalse([[view valueForKey:@"rightPressed"]boolValue]);
 [view pressesBegan:presses withEvent:nil];[view pressesCancelled:presses withEvent:nil];
 XCTAssertEqual(controls.selectedSegmentIndex,NSInteger(3));XCTAssertFalse([[view valueForKey:@"rightPressed"]boolValue]);
 [view setApplicationActive:NO];
}
- (void)testAutoAimPresentedFramesCompareManualPrediction {
 UIWindowScene* scene=nil;for(UIScene* s in UIApplication.sharedApplication.connectedScenes)if([s isKindOfClass:UIWindowScene.class]){scene=(UIWindowScene*)s;break;}
 XCTAssertNotNil(scene);if(!scene)return;
 UIWindow* old=scene.keyWindow;HMGameView* host=[old.rootViewController.view isKindOfClass:HMGameView.class]?(HMGameView*)old.rootViewController.view:nil;[host setApplicationActive:NO];
 NSMutableDictionary* costs=[NSMutableDictionary new];
 @try {for(NSString* name in @[@"manual",@"prediction"]){@autoreleasepool{
  HMProductionTimerView* view=[[HMProductionTimerView alloc]initWithFrame:scene.coordinateSpace.bounds];
  auto& data=perfStorage<heavymach::GameData>(view,"_data");auto& map=data.maps.at("Stage01");map.enemies.clear();map.objects.clear();map.collisionPaths={{{0,40},{10000,40}}};data.machines.at("Heli001").hpMax=100000;
  perfStorage<heavymach::SaveData>(view,"_save")={};[view setValue:@1 forKey:@"randomState"];
  perfStorage<heavymach::AimMode>(view,"_preferredAimMode")=heavymach::AimMode::manual;
  heavymach::ValidationScenario fixture;fixture.allWeapons=true;fixture.grades={4,4,4};fixture.internalLevel=4;fixture.playerPosition=heavymach::Vector2{350,100};
  perfStorage<std::optional<heavymach::ValidationScenario>>(view,"_validationFixture")=fixture;
  [view startSlot:0 mode:0 mission:0 stage:0];for(int n=0;n<110;++n)[view advanceOriginalFrame];
  auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");auto p=session->player()->position;
  session->validationSpawnEnemy({"Heli001",{p.x+240,p.y+140},false});
  if(![name isEqual:@"manual"])[view setInputAimMode:heavymach::AimMode::automatic];
  ((HMValidationControls*)[view valueForKey:@"validationControls"]).selectedSegmentIndex=1;
  UIWindow* window=[[UIWindow alloc]initWithWindowScene:scene];UIViewController* vc=[UIViewController new];vc.view=view;window.rootViewController=vc;[window makeKeyAndVisible];[view layoutIfNeeded];
  view.autoAimExercise=YES;view.timerDone=[self expectationWithDescription:name];[self waitForExpectations:@[view.timerDone] timeout:20];
  NSDictionary* work=metrics(view->timerWork);costs[name]=work;auto aim=session->aimSnapshot();
  emit(@{@"phase":@"autoaim_production_timer",@"candidate":name,@"frame_work":work,@"callback_intervals":metrics(view->timerIntervals),@"assisted_shots":@(aim.assistedShots),@"target_hits":@(aim.targetHits),@"renderer":[[view valueForKey:@"glRenderer"]diagnostics]});
  XCTAssertEqual(view->timerWork.size(),size_t(180));XCTAssertNil([view valueForKey:@"loadError"]);XCTAssertEqualObjects([[view valueForKey:@"glRenderer"]diagnostics][@"lastError"],@"");
  XCTAssertLessThan([work[@"p95_ms"]doubleValue],1000.0/60.0);
  HMGLRenderer* aimRenderer=[view valueForKey:@"glRenderer"];UIImage* image=[aimRenderer snapshot];XCTAssertNotNil(image);if(image){XCTAttachment* a=[XCTAttachment attachmentWithImage:image];a.name=[@"iPad_presented_" stringByAppendingString:name];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}
  [view setApplicationActive:NO];window.hidden=YES;window.rootViewController=nil;
 }}}
 @finally {[old makeKeyAndVisible];[host setApplicationActive:YES];}
 emit(@{@"phase":@"autoaim_p95_added_cost",@"prediction_ms":@([costs[@"prediction"][@"p95_ms"]doubleValue]-[costs[@"manual"][@"p95_ms"]doubleValue])});
}
- (void)testLuxWavePresentedFramesOnDeviceCanvas {
 UIWindowScene* scene=nil;for(UIScene* s in UIApplication.sharedApplication.connectedScenes)if([s isKindOfClass:UIWindowScene.class]){scene=(UIWindowScene*)s;break;}
 XCTAssertNotNil(scene);if(!scene)return;
 UIWindow* old=scene.keyWindow;HMGameView* host=(HMGameView*)old.rootViewController.view;[host setApplicationActive:NO];
 @try {for(int warmup:{5,240}){@autoreleasepool{
  HMProductionTimerView* view=[[HMProductionTimerView alloc]initWithFrame:scene.coordinateSpace.bounds];
  [view setApplicationActive:NO];[view setWideCanvasEnabled:YES];
  perfStorage<heavymach::SaveData>(view,"_save")={};
  auto& data=perfStorage<heavymach::GameData>(view,"_data");const auto& map=data.maps.at("Stage29");
  auto boss=std::find_if(map.enemies.begin(),map.enemies.end(),[](const auto& e){return e.name=="Boss03";});XCTAssertTrue(boss!=map.enemies.end());if(boss==map.enemies.end())continue;
  heavymach::ValidationScenario fixture;fixture.mission=3;fixture.stage=4;fixture.allWeapons=true;fixture.internalLevel=50;fixture.grades={6,6,6};fixture.playerPosition=heavymach::Vector2{boss->position.x-450,boss->position.y};
  perfStorage<std::optional<heavymach::ValidationScenario>>(view,"_validationFixture")=fixture;
  UIWindow* window=[[UIWindow alloc]initWithWindowScene:scene];UIViewController* vc=[UIViewController new];vc.view=view;window.rootViewController=vc;[window makeKeyAndVisible];[view layoutIfNeeded];[view updateCanvasGeometry];
  [view startSlot:0 mode:0 mission:3 stage:4];
  auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");session->validationSetCheats(true);
  ((HMValidationControls*)[view valueForKey:@"validationControls"]).selectedSegmentIndex=1;
  for(int n=0;n<warmup;++n)[view advanceOriginalFrame];
  view.firing=YES;view.timerDone=[self expectationWithDescription:@"Lux real timer"];[view setApplicationActive:YES];[self waitForExpectations:@[view.timerDone] timeout:20];
  int jets=0;for(const auto& e:session->validationEnemies())if(e.name.starts_with("Lux_"))++jets;
  NSDictionary* work=metrics(view->timerWork),*intervals=metrics(view->timerIntervals);
  std::vector<double> activeWork;for(size_t n=0;n<view->timerWork.size();++n)activeWork.push_back(view->timerWork[n]-view->waitTimes[n]);
  NSDictionary* active=metrics(activeWork);
  emit(@{@"phase":@"lux_production_timer",@"warmup":@(warmup),@"jets":@(jets),@"canvas_width":@(session->canvasWidth()),@"frame_work":work,@"callback_intervals":intervals,@"work_before_wait":active,@"copy":metrics(view->copyTimes),@"quad":metrics(view->quadTimes),@"swap":metrics(view->swapTimes),@"wait_and_restore":metrics(view->waitTimes)});
  XCTAssertEqual(view->timerWork.size(),size_t(180));XCTAssertEqual(jets,8);
  // presentToLayer deliberately waits to cap presentation at 60 Hz. Keep
  // that wall time above, but apply the work budget to time before the wait.
  XCTAssertLessThan([active[@"p95_ms"]doubleValue],1000.0/60.0);XCTAssertLessThan([intervals[@"p95_ms"]doubleValue],20.0);
  HMGLRenderer* gl=[view valueForKey:@"glRenderer"];XCTAssertEqualObjects([gl diagnostics][@"lastError"],@"");
  UIImage* image=[gl snapshot];XCTAssertNotNil(image);if(image){XCTAttachment* a=[XCTAttachment attachmentWithImage:image];a.name=[NSString stringWithFormat:@"Lux_Device_Canvas_%d",warmup];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}
  [view setApplicationActive:NO];window.hidden=YES;window.rootViewController=nil;
 }}}
 @finally {[old makeKeyAndVisible];[host setApplicationActive:YES];}
}
- (void)testProductionNSTimerTitleMenuAndFiring {
 UIWindowScene* scene=nil;for(UIScene* s in UIApplication.sharedApplication.connectedScenes)if([s isKindOfClass:UIWindowScene.class]){scene=(UIWindowScene*)s;break;}
 XCTAssertNotNil(scene);if(!scene)return;
 UIWindow* old=scene.keyWindow;HMGameView* host=(HMGameView*)old.rootViewController.view;[host setApplicationActive:NO];
 for(NSString* name in @[@"title",@"menu",@"firing_original",@"firing",@"empty_menu"]){@autoreleasepool{
  HMProductionTimerView* view=[[HMProductionTimerView alloc]initWithFrame:scene.coordinateSpace.bounds];
  auto& save=perfStorage<heavymach::SaveData>(view,"_save");save={};
  if(![name isEqual:@"empty_menu"])for(auto& slot:save.slots)heavymach::GlobalInfo(slot).selectSlot(1);
  if([name isEqual:@"menu"]||[name isEqual:@"empty_menu"])[view refreshScreen:3];
  if([name hasPrefix:@"firing"]){if([name isEqual:@"firing_original"])[view setWideCanvasEnabled:NO];[view startSlot:0 mode:0 mission:0 stage:0];view.firing=YES;}
  UIWindow* window=[[UIWindow alloc]initWithWindowScene:scene];UIViewController* vc=[UIViewController new];vc.view=view;window.rootViewController=vc;[window makeKeyAndVisible];[view layoutIfNeeded];
  view.timerDone=[self expectationWithDescription:name];[self waitForExpectations:@[view.timerDone] timeout:20];
  XCTAssertEqual(view->timerWork.size(),size_t(180));
  emit(@{@"phase":@"production_NSTimer_180_frames",@"scene":name,@"frame_work":metrics(view->timerWork),@"callback_intervals":metrics(view->timerIntervals),@"renderer":[[view valueForKey:@"glRenderer"]diagnostics]});
  emit(@{@"phase":@"presentation_cost_breakdown",@"scene":name,@"copy":metrics(view->copyTimes),@"quad":metrics(view->quadTimes),@"swap":metrics(view->swapTimes),@"wait_and_restore":metrics(view->waitTimes)});
  if([name isEqual:@"empty_menu"]){
   // This is an in-process unit test. Read our GL buffer without requesting
   // XCTest UI automation authorization or modifying the presentation path.
   HMGLRenderer* renderer=[view valueForKey:@"glRenderer"];UIImage* image=[renderer snapshot];XCTAssertNotNil(image);
   XCTAttachment* capture=[XCTAttachment attachmentWithImage:image];capture.name=@"native-empty-menu-logical";capture.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:capture];
  }
  XCTAssertEqualObjects([[view valueForKey:@"glRenderer"]diagnostics][@"lastError"],@"");
  [view setApplicationActive:NO];[[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];window.hidden=YES;window.rootViewController=nil;
 }}
 [old makeKeyAndVisible];[host setApplicationActive:YES];
}
- (void)testCompatibilityRandomSequence {
 HMPerformanceView* view=[[HMPerformanceView alloc] initWithFrame:CGRectMake(0,0,480,320)];
 [[view valueForKey:@"frameTimer"] invalidate];[[view valueForKey:@"motionTimer"] invalidate];
 [view setValue:@1 forKey:@"randomState"];
 const int32_t expected[]={270369,67634689,499951813,307599695,251205585};
 for(int32_t value:expected)XCTAssertEqual([view nextOriginalRandom],value);
 [view setValue:@0 forKey:@"randomState"];XCTAssertEqual([view nextOriginalRandom],270369);
 [view setApplicationActive:NO];
}
- (void)testSimulatorArrowsUseOriginalFiftyHzFilteredAcceleration {
 HMPerformanceView* view=[[HMPerformanceView alloc] initWithFrame:CGRectMake(0,0,912,420)];
 [[view valueForKey:@"frameTimer"] invalidate];[[view valueForKey:@"motionTimer"] invalidate];
 [(CMMotionManager*)[view valueForKey:@"motion"] stopAccelerometerUpdates];
 [view setValue:@YES forKey:@"rightPressed"];
 [view sampleOriginalAcceleration];
 auto& tilt=perfStorage<heavymach::TiltState>(view,"_tilt");
 XCTAssertEqual(tilt.y,float(double(.2f)*double(.1f)));
 [view setValue:@NO forKey:@"rightPressed"];[view sampleOriginalAcceleration];
 XCTAssertEqual(tilt.y,float(double(float(double(.2f)*double(.1f)))*(1.0-double(.1f))));
 [view setApplicationActive:NO];
}
- (void)testProductionTimerAndFullScreenTouchViewport {
 HMPerformanceView* view=[[HMPerformanceView alloc] initWithFrame:CGRectMake(0,0,912,420)];
 NSTimer* timer=[view valueForKey:@"frameTimer"];
 XCTAssertTrue([timer isKindOfClass:NSTimer.class]);
 // A one-shot timer exposes timeInterval=0; the guest deadline is stored separately.
 XCTAssertGreaterThan([[view valueForKey:@"nextTimerDue"] doubleValue],CACurrentMediaTime());
 XCTAssertEqualWithAccuracy(view.contentScaleFactor,UIScreen.mainScreen.nativeScale,1e-9);
 XCTAssertEqualObjects(((CAEAGLLayer*)view.layer).drawableProperties[kEAGLDrawablePropertyColorFormat],kEAGLColorFormatRGB565);
 [view layoutSubviews];
 auto& viewport=perfStorage<heavymach::Viewport>(view,"_viewport");
 XCTAssertEqualWithAccuracy(viewport.x,0,1e-6);
 XCTAssertEqualWithAccuracy(viewport.y,0,1e-6);
 auto fullEdge=viewport.logical(912,0);XCTAssertTrue(fullEdge.has_value());
 XCTAssertEqualWithAccuracy(fullEdge->first,[[view valueForKey:@"canvasWidth"]doubleValue],1e-4);
 [view setWideCanvasEnabled:NO];
 XCTAssertEqualWithAccuracy(viewport.x,141,1e-9);
 auto point=viewport.logical(141,420);XCTAssertTrue(point.has_value());
 XCTAssertEqual(point->first,0);XCTAssertEqual(point->second,0);
 [view setApplicationActive:NO];double paused=[[view valueForKey:@"time"] doubleValue];
 [[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:.05]];
 XCTAssertEqual([[view valueForKey:@"time"] doubleValue],paused);
 [view setApplicationActive:YES];
 [[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:.1]];
 double ticks=([[view valueForKey:@"time"] doubleValue]-paused)/0.01666666753590107;
 XCTAssertGreaterThan(ticks,0);XCTAssertEqualWithAccuracy(ticks,std::round(ticks),1e-8);
 [view setApplicationActive:NO];[timer invalidate];
}
- (void)testOriginalRenderSceneAdvancesExactlyOncePerFrame {
 // Original IPA 0x21e0 renderScene: g_time += 0.01666666753590107,
 // then one GameMain tick and one drawVisual, including after delayed timers.
 const double originalStep=0.01666666753590107;
 for(NSString* mode in @[@"ideal60",@"timestamp_jitter",@"missed_refresh"]){
  HMPerformanceView* view=[[HMPerformanceView alloc] initWithFrame:CGRectMake(0,0,912,420)];
  [[view valueForKey:@"frameTimer"] invalidate];[[view valueForKey:@"motionTimer"] invalidate];
  [(CMMotionManager*)[view valueForKey:@"motion"] stopAccelerometerUpdates];
  XCTAssertNil([view valueForKey:@"loadError"]);
  HMPerformanceFrame* frame=[HMPerformanceFrame new];double timestamp=100;int mismatches=0;
  for(int n=0;n<600;++n){
   timestamp+=1.0/60.0;
   if([mode isEqualToString:@"missed_refresh"]&&n%20==0)timestamp+=1.0/60.0;
   frame.timestamp=timestamp+([mode isEqualToString:@"timestamp_jitter"]?(n%2?.0003:-.0003):0);
   double before=[[view valueForKey:@"time"] doubleValue];
   [view frame:(CADisplayLink*)frame];
   double advance=[[view valueForKey:@"time"] doubleValue]-before;
   if(std::abs(advance-originalStep)>1e-8)++mismatches;
  }
  NSLog(@"HM_TICK_COMPARISON mode=%@ callbacks=600 wrong_tick_counts=%d",mode,mismatches);
  XCTAssertEqual(mismatches,0,@"Original renderScene must advance exactly once for each rendered frame (%@)",mode);
  [view setApplicationActive:NO];
 }
}
- (void)testActualOffscreenTitleMenusAndFiringFrameCosts {
 // This measures real UIKit rendering without attaching a view or injecting UI
 // input. Audio playback is excluded; actual HMMedia image/raster code is used.
 for(NSString* scene in @[@"title_animation",@"slot_preview",@"stage01_firing"]){@autoreleasepool{
  double setupStart=CACurrentMediaTime();
  HMPerformanceView* view=[[HMPerformanceView alloc] initWithFrame:CGRectMake(0,0,912,420)];
  [view setWideCanvasEnabled:NO]; // CPU reference is explicitly the original 480 canvas.
  [[view valueForKey:@"frameTimer"] invalidate];[[view valueForKey:@"motionTimer"] invalidate];
  [(CMMotionManager*)[view valueForKey:@"motion"] stopAccelerometerUpdates];
  NSString* loadError=[view valueForKey:@"loadError"];
  XCTAssertNil(loadError,@"Actual resource load must succeed: %@",loadError);if(loadError)continue;
  HMPerformanceSilentMedia* media=[HMPerformanceSilentMedia new];
  [view setValue:media forKey:@"media"];
  // The menus and HUD retain their original media; replace those references too.
  [[view valueForKey:@"menus"] setValue:media forKey:@"media"];
  [[view valueForKey:@"hud"] setValue:media forKey:@"media"];
  auto& save=perfStorage<heavymach::SaveData>(view,"_save");save={};
  for(auto& slot:save.slots)heavymach::GlobalInfo(slot).selectSlot(1);
  [view setValue:@11 forKey:@"randomState"]; // Repeatable gameplay stream after constructors' timestamp seed.
  if([scene isEqualToString:@"slot_preview"])[view refreshScreen:3];
  if([scene isEqualToString:@"stage01_firing"]){
   [view startSlot:0 mode:0 mission:0 stage:0];
   auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
   XCTAssertTrue(bool(session));if(!session)continue;
   // Public gameplay touch API, without UIKit input injection.
   session->touch({470,160},0);
  }
  [view layoutSubviews];
  UIGraphicsImageRendererFormat* format=[UIGraphicsImageRendererFormat defaultFormat];
  format.scale=3;format.opaque=YES;format.preferredRange=UIGraphicsImageRendererFormatRangeStandard;
  UIGraphicsImageRenderer* target=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(912,420) format:format];
  HMPerformanceFrame* frame=[HMPerformanceFrame new];frame.timestamp=1;
  std::vector<double> ticks,draws,total,cpuTicks,cpuDraws;
  NSUInteger maxBullets=0,maxRenderItems=0;double setupMs=(CACurrentMediaTime()-setupStart)*1000;
  for(int n=0;n<121;++n){@autoreleasepool{


   double t0=CACurrentMediaTime(),c0=cpuMilliseconds();
   if([scene isEqualToString:@"stage01_firing"]){
    auto& firing=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
    if(firing)firing->touch({470,160},1);
   }
   [view frame:(CADisplayLink*)frame];
   double c1=cpuMilliseconds(),t1=CACurrentMediaTime();
   UIImage* image=[target imageWithActions:^(UIGraphicsImageRendererContext* context){[view drawRect:view.bounds];}];
   double c2=cpuMilliseconds(),t2=CACurrentMediaTime();
   XCTAssertTrue(image.CGImage!=nullptr);
   auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
   if(session){maxBullets=std::max(maxBullets,session->bulletCount());maxRenderItems=std::max(maxRenderItems,session->renderItems().size());}
   double tick=(t1-t0)*1000,draw=(t2-t1)*1000;
   if(n==0)emit(@{@"scene":scene,@"phase":@"first_frame",@"setup_ms":@(setupMs),@"tick_wall_ms":@(tick),@"draw_wall_ms":@(draw),@"total_wall_ms":@(tick+draw),@"tick_process_cpu_ms":@(c1-c0),@"draw_process_cpu_ms":@(c2-c1)});
   else {ticks.push_back(tick);draws.push_back(draw);total.push_back(tick+draw);cpuTicks.push_back(c1-c0);cpuDraws.push_back(c2-c1);}
  }}
  emit(@{@"scene":scene,@"phase":@"warm_120_frames",@"tick_wall":metrics(ticks),@"draw_wall":metrics(draws),@"total_wall":metrics(total),@"tick_process_cpu":metrics(cpuTicks),@"draw_process_cpu":metrics(cpuDraws),@"weapon_fire_sound_events":@(media.weaponFireCalls),@"max_bullets":@(maxBullets),@"max_render_items":@(maxRenderItems),@"suppressed_sound_calls":@(media.soundCalls),@"audio_playback":@"excluded",@"budget_ms":@(1000.0/60),@"resolution":@"logical480x320 -> view912x420pt -> bitmap2736x1260 scale3",@"safe_area":@"zero (offscreen view, full viewport)",@"cpu_scope":@"process (includes other threads)",@"draw_scope":@"actual drawRect plus offscreen target allocation/composition",@"cold_scope":@"fresh media cache; process/OS cache not reset"});
  if([scene isEqualToString:@"stage01_firing"]){XCTAssertGreaterThan(maxBullets,NSUInteger(0),@"Firing workload must produce real bullets");XCTAssertGreaterThan(media.weaponFireCalls,NSUInteger(0),@"Real firing must emit weapon sound events");}
  [view setApplicationActive:NO];
 }}
}
- (void)testActualOpenGLTitleMenusAndFiringFrameCosts {
 // This measures real UIKit rendering without attaching a view or injecting UI
 // input. Audio playback is excluded; actual HMMedia image/raster code is used.
 for(NSString* scene in @[@"title_animation",@"slot_preview",@"stage01_firing"]){@autoreleasepool{
  double setupStart=CACurrentMediaTime();
  HMPerformanceView* view=[[HMPerformanceView alloc] initWithFrame:CGRectMake(0,0,912,420)];
  [[view valueForKey:@"frameTimer"] invalidate];[[view valueForKey:@"motionTimer"] invalidate];
  [(CMMotionManager*)[view valueForKey:@"motion"] stopAccelerometerUpdates];
  NSString* loadError=[view valueForKey:@"loadError"];
  XCTAssertNil(loadError,@"Actual resource load must succeed: %@",loadError);if(loadError)continue;
  HMPerformanceSilentMedia* media=[HMPerformanceSilentMedia new];
  [view setValue:media forKey:@"media"];
  // The menus and HUD retain their original media; replace those references too.
  [[view valueForKey:@"menus"] setValue:media forKey:@"media"];
  [[view valueForKey:@"hud"] setValue:media forKey:@"media"];
  auto& save=perfStorage<heavymach::SaveData>(view,"_save");save={};
  for(auto& slot:save.slots)heavymach::GlobalInfo(slot).selectSlot(1);
  [view setValue:@11 forKey:@"randomState"]; // Repeatable gameplay stream after constructors' timestamp seed.
  if([scene isEqualToString:@"slot_preview"])[view refreshScreen:3];
  if([scene isEqualToString:@"stage01_firing"]){
   [view startSlot:0 mode:0 mission:0 stage:0];
   auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
   XCTAssertTrue(bool(session));if(!session)continue;
   // Public gameplay touch API, without UIKit input injection.
   session->touch({470,160},0);
  }
  [view layoutSubviews];
  HMGLRenderer* gpu=[view valueForKey:@"glRenderer"];XCTAssertNotNil(gpu);if(!gpu)continue;
  HMPerformanceFrame* frame=[HMPerformanceFrame new];frame.timestamp=1;
  std::vector<double> ticks,draws,total,cpuTicks,cpuDraws;
  NSUInteger maxBullets=0,maxRenderItems=0;double setupMs=(CACurrentMediaTime()-setupStart)*1000;
  for(int n=0;n<121;++n){@autoreleasepool{


   double t0=CACurrentMediaTime(),c0=cpuMilliseconds();
   if([scene isEqualToString:@"stage01_firing"]){
    auto& firing=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
    if(firing)firing->touch({470,160},1);
   }
   [view frame:(CADisplayLink*)frame];
   double c1=cpuMilliseconds(),t1=CACurrentMediaTime();
   XCTAssertTrue([gpu beginFrame]);media.glRenderer=gpu;
   [view drawLogicalFrame];[gpu finish]; // GPU completion included, not just submission.
   media.glRenderer=nil;[gpu endFrame];
   double c2=cpuMilliseconds(),t2=CACurrentMediaTime();
   if(n==0){
    UIImage* image=[gpu snapshot];XCTAssertTrue(image.CGImage!=nullptr);
    XCTAttachment* attachment=[XCTAttachment attachmentWithImage:image];attachment.name=[scene stringByAppendingString:@"-OpenGL"];attachment.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:attachment];
   }
   auto& session=perfStorage<std::unique_ptr<heavymach::GameSession>>(view,"_session");
   if(session){maxBullets=std::max(maxBullets,session->bulletCount());maxRenderItems=std::max(maxRenderItems,session->renderItems().size());}
   double tick=(t1-t0)*1000,draw=(t2-t1)*1000;
   if(n==0)emit(@{@"scene":scene,@"phase":@"first_frame",@"setup_ms":@(setupMs),@"tick_wall_ms":@(tick),@"draw_wall_ms":@(draw),@"total_wall_ms":@(tick+draw),@"tick_process_cpu_ms":@(c1-c0),@"draw_process_cpu_ms":@(c2-c1)});
   else {ticks.push_back(tick);draws.push_back(draw);total.push_back(tick+draw);cpuTicks.push_back(c1-c0);cpuDraws.push_back(c2-c1);}
  }}
  emit(@{@"scene":scene,@"phase":@"warm_120_frames",@"tick_wall":metrics(ticks),@"draw_wall":metrics(draws),@"total_wall":metrics(total),@"tick_process_cpu":metrics(cpuTicks),@"draw_process_cpu":metrics(cpuDraws),@"weapon_fire_sound_events":@(media.weaponFireCalls),@"max_bullets":@(maxBullets),@"max_render_items":@(maxRenderItems),@"suppressed_sound_calls":@(media.soundCalls),@"audio_playback":@"excluded",@"budget_ms":@(1000.0/60),@"resolution":@"logical480x320 GPU framebuffer",@"safe_area":@"zero (offscreen view, full viewport)",@"cpu_scope":@"process (includes other threads)",@"draw_scope":@"actual drawLogicalFrame OpenGL ES1.1 + glFinish (GPU completion), no drawable presentation/readback",@"cold_scope":@"fresh media cache; process/OS cache not reset"});
  if([scene isEqualToString:@"stage01_firing"]){XCTAssertGreaterThan(maxBullets,NSUInteger(0),@"Firing workload must produce real bullets");XCTAssertGreaterThan(media.weaponFireCalls,NSUInteger(0),@"Real firing must emit weapon sound events");}
  NSLog(@"HM_GL_DIAGNOSTICS %@",[gpu diagnostics]);
  XCTAssertEqualObjects([gpu diagnostics][@"lastError"],@"");
  XCTAssertLessThan([metrics(draws)[@"p95_ms"] doubleValue],1000.0/60.0,@"Simulator render budget; not iPhone FPS certification");
  [view setApplicationActive:NO];
 }}
}
- (void)testOnscreenOpenGLCadenceAudioAndResume {
 UIWindowScene* scene=nil;for(UIScene* candidate in UIApplication.sharedApplication.connectedScenes)if([candidate isKindOfClass:UIWindowScene.class]){scene=(UIWindowScene*)candidate;break;}
 XCTAssertNotNil(scene);if(!scene)return;
 UIWindow* oldWindow=scene.keyWindow;HMGameView* host=[oldWindow.rootViewController.view isKindOfClass:HMGameView.class]?(HMGameView*)oldWindow.rootViewController.view:nil;
 [host setApplicationActive:NO];
 for(NSString* name in @[@"title_animation",@"slot_preview",@"stage01_firing"]){@autoreleasepool{
  HMPerformanceView* view=[[HMPerformanceView alloc] initWithFrame:scene.coordinateSpace.bounds];
  [[view valueForKey:@"frameTimer"] invalidate];[[view valueForKey:@"motionTimer"] invalidate];[(CMMotionManager*)[view valueForKey:@"motion"] stopAccelerometerUpdates];
  HMPerformanceAudibleMedia* audible=[HMPerformanceAudibleMedia new];XCTAssertTrue([audible prepareOriginalSounds]);XCTAssertEqual([[audible.audio diagnostics][@"bufferCount"] intValue],31);[view setValue:audible forKey:@"media"];[[view valueForKey:@"menus"] setValue:audible forKey:@"media"];[[view valueForKey:@"hud"] setValue:audible forKey:@"media"];
  auto& save=perfStorage<heavymach::SaveData>(view,"_save");save={};for(auto& slot:save.slots)heavymach::GlobalInfo(slot).selectSlot(1);[view setValue:@11 forKey:@"randomState"];
  if([name isEqualToString:@"slot_preview"])[view refreshScreen:3];
  if([name isEqualToString:@"stage01_firing"])[view startSlot:0 mode:0 mission:0 stage:0];
  UIWindow* window=[[UIWindow alloc] initWithWindowScene:scene];UIViewController* controller=[UIViewController new];controller.view=view;window.rootViewController=controller;[window makeKeyAndVisible];[view layoutIfNeeded];
  HMDisplayProbe* probe=[HMDisplayProbe new];probe.view=view;probe.firing=[name isEqualToString:@"stage01_firing"];probe.done=[self expectationWithDescription:name];
  probe.link=[CADisplayLink displayLinkWithTarget:probe selector:@selector(step:)];probe.link.preferredFrameRateRange=CAFrameRateRangeMake(60,60,60);[probe.link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
  [self waitForExpectations:@[probe.done] timeout:15];[probe.link invalidate];probe.link=nil;
  XCTAssertEqual(probe->work.size(),size_t(180));
  HMGLRenderer* gpu=[view valueForKey:@"glRenderer"];
  emit(@{@"phase":@"onscreen_180_frames_after_30_warmup",@"scene":name,@"audio_playback":@"enabled",@"frame_work":metrics(probe->work),@"first_30_frame_work":metrics(probe->coldWork),@"callback_intervals":metrics(probe->intervals),@"renderer":[gpu diagnostics],@"audio_backend":[audible.audio diagnostics],@"max_bullets":@(probe->maxBullets),@"scope":@"simulator real display link + EAGL presentation; callback cadence is not physical scanout"});
  XCTAssertEqualObjects([audible.audio diagnostics][@"error"],@"");if(probe.firing)XCTAssertGreaterThan([[audible.audio diagnostics][@"startCount"] intValue],0);
  if(!audible->soundTimes.empty())emit(@{@"phase":@"audio_call_cost",@"scene":name,@"sound_calls":metrics(audible->soundTimes)});
  XCTAssertLessThan([metrics(probe->work)[@"p95_ms"] doubleValue],1000.0/60.0,@"Full simulator frame budget with audio");
  XCTAssertEqualObjects([gpu diagnostics][@"lastError"],@"");if(probe.firing)XCTAssertGreaterThan(probe->maxBullets,NSUInteger(0));
  XCTAttachment* shot=[XCTAttachment attachmentWithImage:[gpu snapshot]];shot.name=[name stringByAppendingString:@"-presented-logical"];shot.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:shot];
  NSUInteger before=[[gpu diagnostics][@"frames"] unsignedIntegerValue];[view layoutSubviews];[view drawRect:view.bounds];XCTAssertEqual([[gpu diagnostics][@"frames"] unsignedIntegerValue],before,@"Layout and UIKit redraw must not add a game draw outside the original timer");[view setApplicationActive:NO];[view frame:(CADisplayLink*)[HMPerformanceFrame new]];XCTAssertEqual([[gpu diagnostics][@"frames"] unsignedIntegerValue],before);
  [view setApplicationActive:YES];[view frame:nil];XCTAssertGreaterThan([[gpu diagnostics][@"frames"] unsignedIntegerValue],before);[view setApplicationActive:NO];
  window.hidden=YES;window.rootViewController=nil;probe.view=nil;
 }}
 [oldWindow makeKeyAndVisible];[host setApplicationActive:YES];
}

@end
