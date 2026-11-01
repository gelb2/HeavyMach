#import <XCTest/XCTest.h>
#import <objc/runtime.h>
#import "HMGameView.h"
#import "HMGameHUD.h"
#import "HMGLRenderer.h"
#import "HMMedia.h"
#import "HMOpenALAudio.h"
#import "HMMenus.h"
#import "HMValidationControls.h"
#import "HMValidationPanel.h"
#include "GameSession.hpp"
#include "Fighters.hpp"
#include "GroundAA.hpp"
#include "HMAimOverlay.hpp"
#include "KeyboardInput.hpp"
#include "Viewport.hpp"
#include "TiltState.hpp"
#include "TitleState.hpp"
#include "LoadingTransition.hpp"
#include <algorithm>
#include <chrono>
#include <vector>
#include <set>
@interface HMGameHUD (BackgroundTests)
- (void)drawLayoutBackground;
@end
@interface HMGameView (AppleInputTests)
- (void)frame:(id)sender;
- (BOOL)handleKeyCode:(unsigned)key pressed:(BOOL)down;
- (void)sampleOriginalAcceleration;
- (void)advanceOriginalFrame;
- (void)drawLogicalFrame;
- (void)drawKeyboardIndicators;
- (void)updateCanvasGeometry;
- (void)refreshScreen:(int)screen;
- (void)startSlot:(int)slot mode:(int)mode mission:(int)mission stage:(int)stage;
- (void)routePoint:(CGPoint)point state:(int)state token:(id)token;
- (void)keyboardDisconnected:(NSNotification*)notification;
- (void)setInputAimMode:(heavymach::AimMode)mode;
- (CGRect)aimControlRect:(int)control;
- (void)drawAimControls;
- (void)openHUDEditor:(id)sender;
- (void)hudEditorAction:(NSInteger)tag;
- (CGRect)gameButtonRect:(int)identifier;
- (void)openTransfer:(id)sender;
- (void)closeTransfer;
- (void)setTransferInputText:(NSString*)text;
- (NSString*)transferInputText;
- (NSString*)createTransferCode;
- (void)gameUIAction:(int)identifier state:(int)state;
- (void)openValidation;
- (void)toggleKeyboardSettings;
- (void)closeHUDEditor;
- (void)loadHUDLayout;
- (void)layoutHUDControls;
- (CGRect)safeContentRect;
- (NSURL*)hudLayoutURL;
- (heavymach::hud::Configuration)currentHUDLayout;
- (void)gameButton:(int)identifier state:(int)state;

@end
// Audit the sprites actually submitted by HMGameHUD, including animated frames.
// This recorder lives only in the test bundle and never enters the game runtime.
@interface HMHUDRecordingMedia:HMMedia
@property NSMutableArray<NSDictionary*>* submitted;
@property BOOL partitionBackgrounds;
@property NSString* isolatedGroup;
@end
@implementation HMHUDRecordingMedia
- (void)draw:(NSString*)name x:(CGFloat)x y:(CGFloat)y columns:(NSInteger)cols rows:(NSInteger)rows tile:(NSInteger)tile scaleX:(CGFloat)sx scaleY:(CGFloat)sy rotation:(CGFloat)r alpha:(CGFloat)a blend:(NSInteger)blend {
 if(self.isolatedGroup.length){
  NSString* group=@"";bool top=[name isEqualToString:@"GameUI_Top.png"],bottom=[name isEqualToString:@"GameUI_Btm.png"];
  if(top||bottom){
   CGRect rect=CGRectNull;
   if(top&&[self.isolatedGroup isEqualToString:@"menu"])rect=CGRectMake(0,256,76,37);
   if(top&&[self.isolatedGroup isEqualToString:@"progress"])rect=CGRectMake(0,293,102,27);
   if(bottom&&[self.isolatedGroup isEqualToString:@"jump"])rect=CGRectMake(0,0,72,64);
   if(bottom&&[self.isolatedGroup isEqualToString:@"weapon"])rect=CGRectMake(72,0,88,64);
   if(bottom&&[self.isolatedGroup isEqualToString:@"gauges"])rect=CGRectMake(160,0,174,64);
   if(bottom&&[self.isolatedGroup isEqualToString:@"aim"])rect=CGRectMake(334,0,146,64);
   if(CGRectIsNull(rect))return;
   [self setClipRect:rect];[super draw:name x:x y:y columns:cols rows:rows tile:tile scaleX:sx scaleY:sy rotation:r alpha:a blend:blend];[self resetClip];return;
  }
  if([name isEqualToString:@"GameMenu.png"])group=@"menu";
  else if([name isEqualToString:@"NumSet01.png"]||[name isEqualToString:@"GameUI_Exp.png"])group=@"progress";
  else if([name isEqualToString:@"Btn_Booster.png"])group=@"jump";
  else if([name isEqualToString:@"Btn_Weapon.png"])group=@"weapon";
  else if([name isEqualToString:@"WeaponTile.png"])group=y==35?@"weapon":@"preview";
  else if([name isEqualToString:@"NumSet02b.png"])group=y==26?@"weapon":@"passives";
  else if([name isEqualToString:@"ItemTile.png"])group=@"passives";
  else if([name isEqualToString:@"SelWeapon.png"])group=@"preview";
  else if([name isEqualToString:@"Extensions/AutoAimControls.png"])group=@"aim";
  else if([name hasPrefix:@"GameUI_"])group=@"gauges";
  if(![group isEqualToString:self.isolatedGroup])return;
 }
 [self.submitted addObject:@{@"asset":name,@"x":@(x),@"y":@(y),@"columns":@(cols),@"rows":@(rows),@"tile":@(tile),@"sx":@(sx),@"sy":@(sy),@"rotation":@(r),@"alpha":@(a),@"blend":@(blend)}];
 if(self.partitionBackgrounds&&([name isEqualToString:@"GameUI_Btm.png"]||[name isEqualToString:@"GameUI_Top.png"])){
  // Candidate divisions, at original positions. Scissor rectangles use y-up.
  std::vector<CGRect> regions;
  if([name isEqualToString:@"GameUI_Btm.png"]){int edges[]={0,72,160,334,480};for(int i=0;i<4;++i)regions.push_back(CGRectMake(edges[i],0,edges[i+1]-edges[i],64));}
  else regions={CGRectMake(0,293,102,27),CGRectMake(102,293,378,27),CGRectMake(0,256,76,37),CGRectMake(76,256,404,37)};
  for(auto rect:regions){[self setClipRect:rect];[super draw:name x:x y:y columns:cols rows:rows tile:tile scaleX:sx scaleY:sy rotation:r alpha:a blend:blend];}
  [self resetClip];return;
 }
 [super draw:name x:x y:y columns:cols rows:rows tile:tile scaleX:sx scaleY:sy rotation:r alpha:a blend:blend];
}
@end
#if TARGET_OS_OSX
@interface HMValidationPanel (InputTests)
- (void)start:(id)sender;
@end
#endif
@interface HMInputTestView:HMGameView
@property NSUInteger gameSaveCalls;
@property NSUInteger keySaveCalls;
@property NSUInteger aimSaveCalls;
@end
@implementation HMInputTestView
- (void)saveAimMode {self.aimSaveCalls++;}
- (void)persistSave {self.gameSaveCalls++;}
- (void)saveKeyboardBindings {self.keySaveCalls++;}
@end
namespace {
template<class T>T& storage(id object,const char* name){Ivar v=class_getInstanceVariable(object_getClass(object),name);if(!v)throw std::runtime_error(name);return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>((__bridge void*)object)+ivar_getOffset(v));}
void key(HMGameView* view,unsigned code){[view handleKeyCode:code pressed:YES];[view handleKeyCode:code pressed:NO];}
void render(HMGameView* view){HMGLRenderer* gl=[view valueForKey:@"glRenderer"];HMMedia* media=[view valueForKey:@"media"];[gl setLogicalWidth:[[view valueForKey:@"canvasWidth"]doubleValue] height:[[view valueForKey:@"canvasHeight"]doubleValue]];if([gl beginFrame]){media.glRenderer=gl;[view drawLogicalFrame];[view drawKeyboardIndicators];media.glRenderer=nil;[gl endFrame];}}
void frames(HMGameView* view,int count){for(int i=0;i<count;++i){[view sampleOriginalAcceleration];[view advanceOriginalFrame];render(view);}}
}
#if TARGET_OS_OSX
namespace {
NSDictionary* macFrameMetrics(const std::vector<double>& values) {
 if(values.empty())return @{@"samples":@0};
 auto sorted=values;std::sort(sorted.begin(),sorted.end());double sum=0;for(double value:values)sum+=value;
 return @{@"samples":@(values.size()),@"mean_ms":@(sum/values.size()),@"p95_ms":@(sorted[(sorted.size()*95+99)/100-1]),@"max_ms":@(sorted.back())};
}
}
@interface HMMacTimedInputView:HMInputTestView {
@public std::vector<double> work,intervals;double previous;NSUInteger count,maxBullets;
}
@property XCTestExpectation* finished;
@property BOOL firing;
@end
@implementation HMMacTimedInputView
- (void)frame:(id)sender {
 if(!self.finished){[super frame:sender];return;}
 if(self.firing){auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self,"_session");if(session){session->touch({470,160},1);maxBullets=std::max(maxBullets,session->bulletCount());}}
 auto clock=[] {return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();};
 double begin=clock();[super frame:sender];double elapsed=(clock()-begin)*1000;
 if(count>=30){work.push_back(elapsed);if(previous)intervals.push_back((begin-previous)*1000);}
 previous=begin;
 if(++count==210){[self setApplicationActive:NO];[self.finished fulfill];self.finished=nil;}
}
@end
#endif
@interface HMAppleInputTests:XCTestCase
@property HMGameView* game;
@property NSURL* temp;
@property NSDictionary* preservedSettings;
@end
@implementation HMAppleInputTests
- (void)setUp {
 [super setUp];NSMutableDictionary* preserved=[NSMutableDictionary new];for(NSString* key in @[@"KeyboardBindingsV1",@"AimModeV1",@"fixture.mission",@"fixture.stage",@"fixture.level",@"fixture.grade0",@"fixture.grade1",@"fixture.grade2",@"fixture.near",@"fixture.all",@"fixture.boss",@"fixture.weapon",@"validation.resume.slot",@"validation.resume.mission",@"validation.resume.stage",@"validation.resume.mode"]){preserved[key]=[NSUserDefaults.standardUserDefaults objectForKey:key]?:NSNull.null;}self.preservedSettings=preserved;self.game=[[HMInputTestView alloc]initWithFrame:CGRectMake(0,0,1024,768)];
 [[self.game valueForKey:@"frameTimer"]invalidate];[[self.game valueForKey:@"motionTimer"]invalidate];
 self.temp=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];[NSFileManager.defaultManager createDirectoryAtURL:self.temp withIntermediateDirectories:YES attributes:nil error:nil];
 [self.game setValue:[self.temp URLByAppendingPathComponent:@"HvM.dat"] forKey:@"saveURL"];
 storage<heavymach::hud::Configuration>(self.game,"_hudLayout")={};storage<heavymach::SaveData>(self.game,"_save")={};storage<heavymach::KeyboardInput>(self.game,"_keyboardControls")={};
 storage<heavymach::AimMode>(self.game,"_preferredAimMode")=heavymach::AimMode::manual;
 [self.game setValue:@1 forKey:@"randomState"];[self.game updateCanvasGeometry];
}
- (void)tearDown {[self.game setApplicationActive:NO];self.game=nil;for(NSString* key in self.preservedSettings){id value=self.preservedSettings[key];if(value==NSNull.null)[NSUserDefaults.standardUserDefaults removeObjectForKey:key];else [NSUserDefaults.standardUserDefaults setObject:value forKey:key];}[NSFileManager.defaultManager removeItemAtURL:self.temp error:nil];[super tearDown];}
- (void)testHUDUpperTrimPreservesOriginalPixels {
#if !TARGET_OS_OSX
 HMGLRenderer* gl=[self.game valueForKey:@"glRenderer"];HMMedia* media=[self.game valueForKey:@"media"];HMGameHUD* hud=[[HMGameHUD alloc]initWithMedia:media];
 [gl setLogicalWidth:480 height:320];heavymach::hud::Configuration c;
 auto capture=[&](){[hud setLayout:c editing:YES];XCTAssertTrue([gl beginFrame]);media.glRenderer=gl;[media setDrawingOffset:CGPointZero];[media setDrawingScale:1];[media resetClip];[hud drawLayoutBackground];[gl finish];[gl endFrame];media.glRenderer=nil;return UIImagePNGRepresentation([gl snapshot]);};
 NSData* original=capture();c.positions[heavymach::hud::cheat]=heavymach::hud::Placement{0,0};NSData* partitioned=capture();
 XCTAssertEqualObjects(original,partitioned,@"Custom layout must retain the complete original upper artwork");
 c.positions[heavymach::hud::upperTrim]=heavymach::hud::Placement{0,.5};NSData* moved=capture();XCTAssertNotEqualObjects(original,moved);
 auto a=[XCTAttachment attachmentWithData:moved uniformTypeIdentifier:@"public.png"];a.name=@"HUD_upper_trim_moved";a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];
#endif
}
- (void)testHUDEditorRightToolsTrimAndLegacySave {
 using namespace heavymach::hud;
 [self startFixture];[self.game gameButton:100 state:2];frames(self.game,40);[self.game openHUDEditor:nil];
 XCTAssertFalse([[self.game valueForKey:@"hudShowHits"]boolValue]);
 for(auto size:{CGSizeMake(852,393),CGSizeMake(1180,820),CGSizeMake(640,260)}){
  self.game.frame=CGRectMake(0,0,size.width,size.height);[self.game updateCanvasGeometry];[self.game layoutHUDControls];
  auto c=[self.game currentHUDLayout];
  for(int id:{3000,3001,3002,3003,3004,3005}){CGRect f=[self.game gameButtonRect:id];XCTAssertFalse(CGRectIsEmpty(f));XCTAssertGreaterThanOrEqual(f.origin.y,c.safe.y);XCTAssertLessThanOrEqual(CGRectGetMaxY(f),c.safe.y+c.safe.height);XCTAssertLessThanOrEqual(CGRectGetMaxX(f),c.safe.x+c.safe.width);}
 }
 self.game.frame=CGRectMake(0,0,1180,820);[self.game updateCanvasGeometry];[self.game layoutHUDControls];
 auto& draft=storage<Configuration>(self.game,"_hudDraft");auto before=draft;[self.game setValue:@(upperTrim) forKey:@"hudSelected"];
 [self.game handleKeyCode:0x51 pressed:YES];XCTAssertTrue(draft.positions[upperTrim].has_value());
 for(size_t i=0;i<hits.size();++i){auto a=before.group(hitGroups[i]).hitRect(hits[i]),b=draft.group(hitGroups[i]).hitRect(hits[i]);XCTAssertEqual(a.x,b.x);XCTAssertEqual(a.y,b.y);}
 auto& vp=storage<heavymach::Viewport>(self.game,"_viewport");
 auto tap=[&](int identifier){CGRect r=[self.game gameButtonRect:identifier];CGPoint q=CGPointMake(vp.x+CGRectGetMidX(r)*vp.scale,vp.y+(vp.logicalHeight-CGRectGetMidY(r))*vp.verticalScale());id token=[NSObject new];[self.game routePoint:q state:0 token:token];[self.game routePoint:q state:2 token:token];};
 tap(3003);XCTAssertTrue([[self.game valueForKey:@"hudHideGuides"]boolValue]);XCTAssertFalse(CGRectIsEmpty([self.game gameButtonRect:3005]));
 CGPoint q=CGPointMake(vp.x+vp.logicalWidth/2*vp.scale,vp.y+vp.logicalHeight/2*vp.verticalScale());id token=[NSObject new];auto positions=draft;
 [self.game routePoint:q state:2 token:token];XCTAssertTrue([[self.game valueForKey:@"hudHideGuides"]boolValue]);XCTAssertTrue(positions.samePositions(draft));
 tap(3005);XCTAssertFalse([[self.game valueForKey:@"hudHideGuides"]boolValue]);
 // A tool touch must not start a HUD drag, even if a HUD group lies underneath it.
 tap(3002);XCTAssertFalse([[self.game valueForKey:@"hudEditing"]boolValue]);
 auto saved=storage<Configuration>(self.game,"_hudLayout");XCTAssertTrue(saved.positions[upperTrim].has_value());storage<Configuration>(self.game,"_hudLayout")={};[self.game loadHUDLayout];XCTAssertTrue(saved.samePositions(storage<Configuration>(self.game,"_hudLayout")));
 NSMutableArray* old=[NSMutableArray new];for(int i=0;i<9;++i)[old addObject:NSNull.null];old[weapon]=@[@0,@0.5];
 [[NSJSONSerialization dataWithJSONObject:@{@"version":@1,@"positions":old} options:0 error:nil]writeToURL:[self.game hudLayoutURL] atomically:YES];
 [self.game loadHUDLayout];auto migrated=storage<Configuration>(self.game,"_hudLayout");XCTAssertTrue(migrated.positions[weapon].has_value());XCTAssertFalse(migrated.positions[upperTrim].has_value());
#if !TARGET_OS_OSX
 [self.game openHUDEditor:nil];render(self.game);auto a=[XCTAttachment attachmentWithImage:[(HMGLRenderer*)[self.game valueForKey:@"glRenderer"]snapshot]];a.name=@"HUD_game_buttons";a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];
 for(UIView* view in self.game.subviews)XCTAssertFalse([view isKindOfClass:UIButton.class]);
#else
 for(NSView* view in self.game.subviews)XCTAssertFalse([view isKindOfClass:NSButton.class]);
#endif
}
- (void)testGameTransferButtonsCreateCheckCancelAndRestore {
 [self startFixture];auto expected=storage<heavymach::SaveData>(self.game,"_save");auto bytes=expected.encode();NSURL* save=[self.game valueForKey:@"saveURL"];
 [[NSData dataWithBytes:bytes.data() length:bytes.size()]writeToURL:save atomically:YES];
 [self.game refreshScreen:3];render(self.game);
 auto tap=[&](int identifier){CGRect r=[self.game gameButtonRect:identifier];XCTAssertFalse(CGRectIsEmpty(r));auto vp=storage<heavymach::Viewport>(self.game,"_viewport");CGPoint q=CGPointMake(vp.x+CGRectGetMidX(r)*vp.scale,vp.y+(vp.logicalHeight-CGRectGetMidY(r))*vp.verticalScale());id token=[NSObject new];[self.game routePoint:q state:0 token:token];[self.game routePoint:q state:2 token:token];render(self.game);};
 tap(3100);XCTAssertEqual([[self.game valueForKey:@"transferPage"]intValue],1);
 tap(3110);NSString* code=[self.game transferInputText];XCTAssertTrue([code hasPrefix:@"HM1-"]);
 {auto image=[(HMGLRenderer*)[self.game valueForKey:@"glRenderer"]snapshot];auto a=[XCTAttachment attachmentWithImage:image];a.name=@"Transfer_game_buttons";a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}
 auto before=[[NSData alloc]initWithContentsOfURL:save];tap(3112);XCTAssertEqualObjects(before,[NSData dataWithContentsOfURL:save]);
 [self.game setTransferInputText:@"bad code"];tap(3113);XCTAssertEqual([[self.game valueForKey:@"transferPage"]intValue],1);XCTAssertEqualObjects(before,[NSData dataWithContentsOfURL:save]);
 [self.game setTransferInputText:code];tap(3113);XCTAssertEqual([[self.game valueForKey:@"transferPage"]intValue],2);tap(3116);XCTAssertEqualObjects(before,[NSData dataWithContentsOfURL:save]);
 auto changed=expected;changed.slots[0].setInt(0x20,expected.slots[0].experience()+77);auto changedBytes=changed.encode();[[NSData dataWithBytes:changedBytes.data() length:changedBytes.size()]writeToURL:save atomically:YES];
 tap(3113);tap(3115);[self.game closeTransfer];[self.game openValidation];XCTAssertFalse([[self.game valueForKey:@"validationPanelOpen"]boolValue]);XCTAssertTrue(CGRectIsEmpty([self.game gameButtonRect:3100]));[self.game advanceOriginalFrame];[self.game handleKeyCode:0x3b pressed:YES];[self.game handleKeyCode:0x3b pressed:NO];XCTAssertFalse([[self.game valueForKey:@"keyboardSettingsOpen"]boolValue]);frames(self.game,100);XCTAssertFalse(storage<heavymach::LoadingTransition>(self.game,"_loading").visible());XCTAssertEqualObjects(before,[NSData dataWithContentsOfURL:save]);XCTAssertEqual(storage<heavymach::SaveData>(self.game,"_save").encode(),expected.encode());
 [self.game closeTransfer];XCTAssertEqual([[self.game valueForKey:@"transferPage"]intValue],0);
}
- (void)testGameSettingsAndValidationButtons {
 [self startFixture];render(self.game);auto before=storage<heavymach::SaveData>(self.game,"_save").encode();
 auto tap=[&](int identifier){CGRect r=[self.game gameButtonRect:identifier];XCTAssertFalse(CGRectIsEmpty(r));auto vp=storage<heavymach::Viewport>(self.game,"_viewport");CGPoint q=CGPointMake(vp.x+CGRectGetMidX(r)*vp.scale,vp.y+(vp.logicalHeight-CGRectGetMidY(r))*vp.verticalScale());id token=[NSObject new];[self.game routePoint:q state:0 token:token];[self.game routePoint:q state:2 token:token];render(self.game);};
 tap(3300);XCTAssertTrue([[self.game valueForKey:@"validationPanelOpen"]boolValue]);
 HMValidationPanel* panel=[self.game valueForKey:@"validationPanel"];int level=panel.level;tap(3405);XCTAssertEqual(panel.level,std::min(level+1,99));XCTAssertEqual(before,storage<heavymach::SaveData>(self.game,"_save").encode());
 auto attachment=[&](NSString* name){auto a=[XCTAttachment attachmentWithImage:[(HMGLRenderer*)[self.game valueForKey:@"glRenderer"]snapshot]];a.name=name;a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];};
 attachment(@"Validation_game_buttons");for(int field=0;field<6;++field){tap(3400+field*2);tap(3401+field*2);}tap(3304);XCTAssertEqual(panel.page,1);for(int field=6;field<11;++field){tap(3400+field*2);tap(3401+field*2);}tap(3304);XCTAssertEqual(panel.page,0);tap(3305);XCTAssertFalse([[self.game valueForKey:@"validationPanelOpen"]boolValue]);
 tap(3300);panel=[self.game valueForKey:@"validationPanel"];panel.mission=0;panel.stage=0;panel.level=0;panel.boss=-1;panel.nearLevel=NO;panel.allWeapons=NO;tap(3301);XCTAssertFalse([[self.game valueForKey:@"validationPanelOpen"]boolValue]);frames(self.game,40);tap(3300);auto exp=storage<heavymach::SaveData>(self.game,"_save").slots[0].experience();tap(3303);XCTAssertEqual(storage<heavymach::SaveData>(self.game,"_save").slots[0].experience(),exp+1);tap(3302);XCTAssertFalse([[self.game valueForKey:@"validationPanelOpen"]boolValue]);frames(self.game,40);
 [self.game toggleKeyboardSettings];render(self.game);attachment(@"Keyboard_game_buttons");tap(3206);XCTAssertTrue([[self.game valueForKey:@"capturingKey"]boolValue]);key(self.game,0x14);XCTAssertEqual(storage<heavymach::KeyboardInput>(self.game,"_keyboardControls").bindings[6],0x14u);tap(3220);XCTAssertEqual(storage<heavymach::KeyboardInput>(self.game,"_keyboardControls").bindings,heavymach::KeyboardInput::defaults);for(int i=0;i<16;++i){tap(3200+i);XCTAssertTrue([[self.game valueForKey:@"capturingKey"]boolValue]);key(self.game,0x29);}tap(3221);XCTAssertFalse([[self.game valueForKey:@"keyboardSettingsOpen"]boolValue]);
 [self.game gameButton:100 state:2];frames(self.game,40);[self.game openHUDEditor:nil];render(self.game);attachment(@"HUD_game_tools");
 key(self.game,0x3f);XCTAssertTrue([[self.game valueForKey:@"gameToolsFocus"]boolValue]);key(self.game,0x2b);key(self.game,0x28);XCTAssertTrue([[self.game valueForKey:@"hudHideGuides"]boolValue]);key(self.game,0x3f);XCTAssertFalse([[self.game valueForKey:@"gameToolsFocus"]boolValue]);key(self.game,0x29);XCTAssertFalse([[self.game valueForKey:@"hudEditing"]boolValue]);
}
- (void)testPauseMenuHUDEntryUsesGameTouchAndKeyboard {
 [self startFixture];render(self.game);
 auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");
 [self.game gameButton:104 state:2];XCTAssertFalse([[self.game valueForKey:@"hudEditing"]boolValue]);
 [self.game gameButton:100 state:2];frames(self.game,100);render(self.game);
 HMGameHUD* hud=[self.game valueForKey:@"hud"];XCTAssertEqual([hud receiverAt:CGPointMake(240,190)],104);
 XCTAssertTrue(CGRectIsEmpty([self.game gameButtonRect:3005]));
 auto before=storage<heavymach::SaveData>(self.game,"_save").encode();auto rng=storage<uint32_t>(self.game,"_randomState");
 auto c=[self.game currentHUDLayout];auto logical=c.legacy.apply(heavymach::hud::Point{240,190});
 auto& vp=storage<heavymach::Viewport>(self.game,"_viewport");
 CGPoint point=CGPointMake(vp.x+logical.x*vp.scale,vp.y+(storage<CGFloat>(self.game,"_canvasHeight")-logical.y)*vp.verticalScale());
 id token=[NSObject new];
#if !TARGET_OS_OSX
 auto attach=[&](NSString* name){auto a=[XCTAttachment attachmentWithImage:[(HMGLRenderer*)[self.game valueForKey:@"glRenderer"]snapshot]];a.name=name;a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];};
 attach(@"Pause_menu_HUD_entry");
#endif
 [self.game routePoint:point state:0 token:token];render(self.game);
#if !TARGET_OS_OSX
 attach(@"Pause_menu_HUD_entry_pressed");
#endif
 [self.game routePoint:point state:2 token:token];
 XCTAssertTrue([[self.game valueForKey:@"hudEditing"]boolValue]);XCTAssertTrue(session->world().paused);
 frames(self.game,60);XCTAssertEqual(before,storage<heavymach::SaveData>(self.game,"_save").encode());XCTAssertEqual(rng,storage<uint32_t>(self.game,"_randomState"));
 [self.game closeHUDEditor];render(self.game);XCTAssertTrue(session->world().paused);
 for(int i=0;i<12;++i){[hud moveKeyboardFocus:1];CGRect rect=[hud keyboardFocusRect];if([hud receiverAt:CGPointMake(CGRectGetMidX(rect),CGRectGetMidY(rect))]==104)break;}
 [hud activateKeyboardFocus];XCTAssertTrue([[self.game valueForKey:@"hudEditing"]boolValue]);
 [self.game hudEditorAction:2];
 XCTAssertTrue(session->world().paused);XCTAssertEqual([[self.game valueForKey:@"hudLayoutWrites"]integerValue],0);
 [self.game gameButton:101 state:2];XCTAssertFalse(session->world().paused);
 XCTAssertEqual(((HMInputTestView*)self.game).gameSaveCalls,0u);
}
- (void)testHUDEditorMovesHitsFreezesBattleAndPersistsOnlyApply {
 [self startFixture];render(self.game);[self.game gameButton:100 state:2];frames(self.game,40);
 auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");
 auto before=storage<heavymach::SaveData>(self.game,"_save").encode();auto rng=storage<uint32_t>(self.game,"_randomState");double time=storage<double>(self.game,"_time");auto pos=session->player()->position;
 [self.game openHUDEditor:nil];XCTAssertTrue([[self.game valueForKey:@"hudEditing"]boolValue]);
 auto c=[self.game currentHUDLayout];auto b=c.group(heavymach::hud::weapon).visualBounds;
 auto& vp=storage<heavymach::Viewport>(self.game,"_viewport");auto point=[&](double x,double y){return CGPointMake(vp.x+x*vp.scale,vp.y+(storage<CGFloat>(self.game,"_canvasHeight")-y)*vp.verticalScale());};
 id token=[NSObject new];[self.game routePoint:point(b.x+30,b.y+30) state:0 token:token];
 [self.game routePoint:point(c.safe.x+30,c.safe.y+150) state:1 token:token];
 [self.game routePoint:point(c.safe.x+30,c.safe.y+150) state:2 token:token];
 for(int i=0;i<120;++i){[self.game handleKeyCode:0x09 pressed:YES];[self.game advanceOriginalFrame];}
 XCTAssertEqual(before,storage<heavymach::SaveData>(self.game,"_save").encode());XCTAssertEqual(rng,storage<uint32_t>(self.game,"_randomState"));XCTAssertEqual(time,storage<double>(self.game,"_time"));XCTAssertEqual(pos.x,session->player()->position.x);XCTAssertEqual(pos.y,session->player()->position.y);
 XCTAssertEqual([[self.game valueForKey:@"hudLayoutWrites"]integerValue],0);XCTAssertFalse([NSFileManager.defaultManager fileExistsAtPath:[self.game hudLayoutURL].path]);
 render(self.game);
#if !TARGET_OS_OSX
 auto a=[XCTAttachment attachmentWithImage:[(HMGLRenderer*)[self.game valueForKey:@"glRenderer"]snapshot]];a.name=@"HUD_editor_moved";a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];
#endif
 [self.game hudEditorAction:2];
 XCTAssertFalse([[self.game valueForKey:@"hudEditing"]boolValue]);XCTAssertEqual([[self.game valueForKey:@"hudLayoutWrites"]integerValue],1);
 auto saved=storage<heavymach::hud::Configuration>(self.game,"_hudLayout");XCTAssertTrue(saved.positions[heavymach::hud::weapon].has_value());
 [self.game gameButton:101 state:2];frames(self.game,50);render(self.game);
 c=[self.game currentHUDLayout];auto moved=c.group(heavymach::hud::weapon).transform.apply(heavymach::hud::Point{103,31});
 int old=session->selectedWeapon();id click=[NSObject new];[self.game routePoint:point(moved.x,moved.y) state:0 token:click];[self.game routePoint:point(moved.x,moved.y) state:2 token:click];XCTAssertNotEqual(session->selectedWeapon(),old);
 old=session->selectedWeapon();auto original=c.legacy.apply(heavymach::hud::Point{103,31});[self.game routePoint:point(original.x,original.y) state:0 token:click];[self.game routePoint:point(original.x,original.y) state:2 token:click];XCTAssertEqual(session->selectedWeapon(),old);
 storage<heavymach::hud::Configuration>(self.game,"_hudLayout")={};[self.game loadHUDLayout];XCTAssertTrue(saved.samePositions(storage<heavymach::hud::Configuration>(self.game,"_hudLayout")));
 [self.game gameButton:100 state:2];frames(self.game,40);[self.game openHUDEditor:nil];[self.game hudEditorAction:0];[self.game hudEditorAction:1];XCTAssertTrue(saved.samePositions(storage<heavymach::hud::Configuration>(self.game,"_hudLayout")));XCTAssertEqual([[self.game valueForKey:@"hudLayoutWrites"]integerValue],1);
 [self.game openHUDEditor:nil];[self.game hudEditorAction:0];[self.game hudEditorAction:2];XCTAssertFalse(storage<heavymach::hud::Configuration>(self.game,"_hudLayout").customized());XCTAssertEqual([[self.game valueForKey:@"hudLayoutWrites"]integerValue],2);
}
- (void)testMissionSelectionHasNoHUDEntryAndPauseExamplePreservesSave {
 [self.game refreshScreen:4];render(self.game);
 XCTAssertTrue(CGRectIsEmpty([self.game gameButtonRect:3005]));
 auto menuSave=storage<heavymach::SaveData>(self.game,"_save").encode();
 [self.game openHUDEditor:nil];XCTAssertFalse([[self.game valueForKey:@"hudEditing"]boolValue]);
 XCTAssertEqual(menuSave,storage<heavymach::SaveData>(self.game,"_save").encode());
#if !TARGET_OS_OSX
 auto menu=[XCTAttachment attachmentWithImage:[(HMGLRenderer*)[self.game valueForKey:@"glRenderer"]snapshot]];menu.name=@"Mission_selection_without_HUD_entry";menu.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:menu];
#endif
 [self startFixture];[self.game gameButton:100 state:2];frames(self.game,40);
 auto before=storage<heavymach::SaveData>(self.game,"_save").encode();auto rng=storage<uint32_t>(self.game,"_randomState");
 [self.game openHUDEditor:nil];XCTAssertTrue([[self.game valueForKey:@"hudEditing"]boolValue]);
 [self.game hudEditorAction:4];render(self.game);
 auto& example=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_hudExampleSession");XCTAssertTrue(bool(example));XCTAssertGreaterThan(example->enemyCount(),size_t(0));
 self.game.frame=CGRectMake(0,0,1400,700);[self.game updateCanvasGeometry];render(self.game);
 XCTAssertEqual(before,storage<heavymach::SaveData>(self.game,"_save").encode());XCTAssertEqual(rng,storage<uint32_t>(self.game,"_randomState"));
 [self.game closeHUDEditor];XCTAssertTrue([[self.game valueForKey:@"gameMenu"]boolValue]);XCTAssertEqual([[self.game valueForKey:@"hudLayoutWrites"]integerValue],0);
}
- (void)testHUDEditorSelectionCollisionResizeAndSaveFailure {
 [self startFixture];[self.game gameButton:100 state:2];frames(self.game,40);[self.game openHUDEditor:nil];
 auto c=[self.game currentHUDLayout];auto point=[&](heavymach::hud::Point p){auto vp=storage<heavymach::Viewport>(self.game,"_viewport");return CGPointMake(vp.x+p.x*vp.scale,vp.y+(vp.logicalHeight-p.y)*vp.verticalScale());};
 id token=[NSObject new];auto center=c.group(heavymach::hud::jump).transform.apply(heavymach::hud::Point{34,34});
 [self.game routePoint:point(center) state:0 token:token];[self.game routePoint:point(center) state:2 token:token];XCTAssertFalse(storage<heavymach::hud::Configuration>(self.game,"_hudDraft").customized());
 auto& draft=storage<heavymach::hud::Configuration>(self.game,"_hudDraft");draft.positions[heavymach::hud::weapon]=heavymach::hud::placementForOrigin(heavymach::hud::bounds[heavymach::hud::weapon],c.legacy,c.safe,{c.legacy.offset.x,c.legacy.offset.y});
 [self.game hudEditorAction:2];XCTAssertTrue([[self.game valueForKey:@"hudEditing"]boolValue]);XCTAssertEqual([[self.game valueForKey:@"hudLayoutWrites"]integerValue],0);
 draft.positions={};draft.positions[heavymach::hud::weapon]=heavymach::hud::Placement{0,.5};
 NSURL* original=[self.game valueForKey:@"saveURL"];[self.game setValue:[self.temp URLByAppendingPathComponent:@"missing/HvM.dat"] forKey:@"saveURL"];
 [self.game hudEditorAction:2];XCTAssertTrue([[self.game valueForKey:@"hudEditing"]boolValue]);XCTAssertEqual([[self.game valueForKey:@"hudLayoutWrites"]integerValue],0);
 [self.game setValue:original forKey:@"saveURL"];[self.game hudEditorAction:2];XCTAssertFalse([[self.game valueForKey:@"hudEditing"]boolValue]);
 auto saved=storage<heavymach::hud::Configuration>(self.game,"_hudLayout");[@"invalid json" writeToURL:[self.game hudLayoutURL] atomically:YES encoding:NSUTF8StringEncoding error:nil];[self.game loadHUDLayout];XCTAssertTrue(saved.samePositions(storage<heavymach::hud::Configuration>(self.game,"_hudLayout")));
 self.game.frame=CGRectMake(0,0,1920,640);[self.game updateCanvasGeometry];auto& layout=storage<heavymach::hud::Configuration>(self.game,"_hudLayout");layout.positions={};layout.positions[heavymach::hud::jump]=heavymach::hud::Placement{.43,.5};layout.positions[heavymach::hud::weapon]=heavymach::hud::Placement{.56,.5};[self.game updateCanvasGeometry];XCTAssertTrue([self.game currentHUDLayout].customized());
 self.game.frame=CGRectMake(0,0,960,640);[self.game updateCanvasGeometry];XCTAssertFalse([self.game currentHUDLayout].customized());XCTAssertTrue(layout.customized());
 self.game.frame=CGRectMake(0,0,1920,640);[self.game updateCanvasGeometry];XCTAssertTrue([self.game currentHUDLayout].customized());
}
- (void)testHUDEditorCheatGeometryAndRepeatedClose {
 [self startFixture];[self.game gameButton:100 state:2];frames(self.game,40);
 auto point=[&](heavymach::hud::Point p){auto vp=storage<heavymach::Viewport>(self.game,"_viewport");return CGPointMake(vp.x+p.x*vp.scale,vp.y+(vp.logicalHeight-p.y)*vp.verticalScale());};
 for(int i=0;i<10;++i){
  [self.game openHUDEditor:nil];render(self.game);
  auto c=[self.game currentHUDLayout];auto r=c.group(heavymach::hud::cheat).visualBounds;id token=[NSObject new];
  [self.game routePoint:point({r.x+r.width/2,r.y+r.height/2}) state:0 token:token];[self.game routePoint:point({r.x+r.width/2,r.y+r.height/2}) state:2 token:token];
  XCTAssertFalse(storage<heavymach::hud::Configuration>(self.game,"_hudDraft").customized());
  CGRect controls=((HMValidationControls*)[self.game valueForKey:@"validationControls"]).frame;
  CGRect tool=[self.game gameButtonRect:3002];auto vp=storage<heavymach::Viewport>(self.game,"_viewport");CGRect nativeTool=CGRectMake(vp.x+tool.origin.x*vp.scale,vp.y+(vp.logicalHeight-CGRectGetMaxY(tool))*vp.verticalScale(),tool.size.width*vp.scale,tool.size.height*vp.verticalScale());XCTAssertFalse(CGRectIntersectsRect(controls,nativeTool));
  auto& draft=storage<heavymach::hud::Configuration>(self.game,"_hudDraft");
  [self.game setValue:@(heavymach::hud::cheat) forKey:@"hudSelected"];[self.game handleKeyCode:0x51 pressed:YES];
  XCTAssertFalse(CGRectEqualToRect(controls,((HMValidationControls*)[self.game valueForKey:@"validationControls"]).frame));
  draft.positions={};draft.positions[heavymach::hud::weapon]=heavymach::hud::placementForOrigin(c.localBounds(heavymach::hud::weapon),c.legacy,c.safe,{r.x,r.y});XCTAssertFalse(heavymach::hud::valid(draft,true));
  draft.positions={};draft.positions[heavymach::hud::cheat]=heavymach::hud::Placement{.5,.5};
  [self.game hudEditorAction:0];
  XCTAssertTrue(CGRectEqualToRect(controls,((HMValidationControls*)[self.game valueForKey:@"validationControls"]).frame));
  [self.game closeHUDEditor];XCTAssertFalse(bool(storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_hudExampleSession")));
 }
 XCTAssertEqual([[self.game valueForKey:@"hudLayoutWrites"]integerValue],0);
}
- (void)testHUDMovedButtonsKeepTouchOwnershipAndAimFire {
 [self startFixture];auto& layout=storage<heavymach::hud::Configuration>(self.game,"_hudLayout");layout.positions[heavymach::hud::weapon]=heavymach::hud::Placement{0,.5};layout.positions[heavymach::hud::jump]=heavymach::hud::Placement{1,.5};layout.positions[heavymach::hud::aim]=heavymach::hud::Placement{1,.1};render(self.game);
 auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");session->validationSetCheats(true);
 auto c=[self.game currentHUDLayout];auto point=[&](heavymach::hud::Point p){auto vp=storage<heavymach::Viewport>(self.game,"_viewport");return CGPointMake(vp.x+p.x*vp.scale,vp.y+(vp.logicalHeight-p.y)*vp.verticalScale());};
 auto jump=c.group(heavymach::hud::jump).transform.apply(heavymach::hud::Point{34,34}),weapon=c.group(heavymach::hud::weapon).transform.apply(heavymach::hud::Point{103,31});id touch=[NSObject new];int selected=session->selectedWeapon();
 [self.game routePoint:point(jump) state:0 token:touch];[self.game routePoint:point(weapon) state:1 token:touch];[self.game routePoint:point(weapon) state:2 token:touch];XCTAssertEqual(session->selectedWeapon(),selected);
 [self.game setInputAimMode:heavymach::AimMode::automatic];render(self.game);CGRect fire=[self.game aimControlRect:2];heavymach::hud::Point f{CGRectGetMidX(fire),CGRectGetMidY(fire)};
 [self.game routePoint:point(f) state:0 token:touch];XCTAssertEqual([[self.game valueForKey:@"aimTouches"]count],NSUInteger(1));
 [self.game routePoint:point({f.x,f.y+100}) state:1 token:touch];XCTAssertEqual([[self.game valueForKey:@"aimTouches"]count],NSUInteger(0));[self.game routePoint:point(f) state:2 token:touch];
}
- (void)testHUDMovedMenuJumpAndAimButtons {
 [self startFixture];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");session->validationSetCheats(true);
 auto& layout=storage<heavymach::hud::Configuration>(self.game,"_hudLayout");
 layout.positions[heavymach::hud::menu]=heavymach::hud::Placement{1,.7};layout.positions[heavymach::hud::jump]=heavymach::hud::Placement{0,.4};layout.positions[heavymach::hud::aim]=heavymach::hud::Placement{.5,.4};render(self.game);
 auto c=[self.game currentHUDLayout];XCTAssertTrue(c.customized());
 auto point=[&](heavymach::hud::Point p){auto vp=storage<heavymach::Viewport>(self.game,"_viewport");return CGPointMake(vp.x+p.x*vp.scale,vp.y+(vp.logicalHeight-p.y)*vp.verticalScale());};
 auto tap=[&](heavymach::hud::Point p){id token=[NSObject new];[self.game routePoint:point(p) state:0 token:token];[self.game routePoint:point(p) state:2 token:token];};
 auto before=session->player()->position;tap(c.group(heavymach::hud::jump).transform.apply(heavymach::hud::Point{34,34}));frames(self.game,10);XCTAssertGreaterThan(session->player()->position.y,before.y);
 CGRect mode=[self.game aimControlRect:1];tap({CGRectGetMidX(mode),CGRectGetMidY(mode)});XCTAssertEqual(session->aimMode(),heavymach::AimMode::automatic);
 tap({CGRectGetMidX(mode),CGRectGetMidY(mode)});XCTAssertEqual(session->aimMode(),heavymach::AimMode::manual);
 tap(c.group(heavymach::hud::menu).transform.apply(heavymach::hud::Point{38,279}));XCTAssertTrue(session->world().paused);XCTAssertTrue([[self.game valueForKey:@"gameMenu"]boolValue]);
 [self.game openHUDEditor:nil];auto scene=session->renderItems().size();auto beforeSave=storage<heavymach::SaveData>(self.game,"_save").encode();
 HMMedia* media=[self.game valueForKey:@"media"];render(self.game);auto decodes=[media cacheDiagnostics][@"decodeCount"];
 for(int i=0;i<180;++i){[self.game advanceOriginalFrame];render(self.game);}
 XCTAssertEqualObjects(decodes,[media cacheDiagnostics][@"decodeCount"]);XCTAssertEqual(scene,session->renderItems().size());XCTAssertEqual(beforeSave,storage<heavymach::SaveData>(self.game,"_save").encode());
}
- (void)testHUDMovedGaugesAnimateAndFreezeInEditor {
 [self startFixture];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");session->validationSetCheats(true);
 [self.game gameButton:100 state:2];frames(self.game,40);[self.game openHUDEditor:nil];
 auto c=[self.game currentHUDLayout];auto b=c.group(heavymach::hud::gauges).visualBounds;
 auto point=[&](double x,double y){auto vp=storage<heavymach::Viewport>(self.game,"_viewport");return CGPointMake(vp.x+x*vp.scale,vp.y+(vp.logicalHeight-y)*vp.verticalScale());};
 id token=[NSObject new];
 [self.game routePoint:point(b.x+b.width/2,b.y+b.height/2) state:0 token:token];
 [self.game routePoint:point(b.x+b.width/2,b.y+b.height/2+100) state:1 token:token];
 [self.game routePoint:point(b.x+b.width/2,b.y+b.height/2+100) state:2 token:token];
 [self.game hudEditorAction:2];
 XCTAssertFalse([[self.game valueForKey:@"hudEditing"]boolValue]);
 XCTAssertTrue(storage<heavymach::hud::Configuration>(self.game,"_hudLayout").positions[heavymach::hud::gauges].has_value());
 [self.game gameButton:101 state:2];frames(self.game,50);
 HMGameHUD* hud=[self.game valueForKey:@"hud"];
 // Feed the same HP update used by the player event consumer, then fire through input.
 [hud setHPGauge:1];frames(self.game,40);float healthy=storage<float>(hud,"_hpRotation");
 [hud setHPGauge:.35];frames(self.game,30);XCTAssertNotEqual(healthy,storage<float>(hud,"_hpRotation"));
 storage<CGPoint>(self.game,"_keyboardAim")=CGPointMake(450,220);storage<BOOL>(self.game,"_hasKeyboardAim")=YES;
 [self.game handleKeyCode:9 pressed:YES];float lo=1,hi=0;
 for(int i=0;i<120;++i){frames(self.game,1);float v=storage<float>(self.game,"_reloadGauge");lo=std::min(lo,v);hi=std::max(hi,v);}
 [self.game handleKeyCode:9 pressed:NO];XCTAssertGreaterThan(hi-lo,.1f);
 c=[self.game currentHUDLayout];XCTAssertGreaterThan(c.group(heavymach::hud::gauges).visualBounds.y,b.y+90);
#if !TARGET_OS_OSX
 auto a=[XCTAttachment attachmentWithImage:[(HMGLRenderer*)[self.game valueForKey:@"glRenderer"]snapshot]];a.name=@"HUD_ipad_moved_gauges_firing";a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];
#endif
 [self.game gameButton:100 state:2];frames(self.game,40);[self.game openHUDEditor:nil];
 float hp=storage<float>(hud,"_hpRotation"),gear=storage<float>(hud,"_gear"),reload=storage<float>(self.game,"_reloadGauge");
 frames(self.game,120);XCTAssertEqual(hp,storage<float>(hud,"_hpRotation"));XCTAssertEqual(gear,storage<float>(hud,"_gear"));XCTAssertEqual(reload,storage<float>(self.game,"_reloadGauge"));
 [self.game hudEditorAction:1];XCTAssertEqual([[self.game valueForKey:@"hudLayoutWrites"]integerValue],1);
 NSLog(@"HM_HUD_GAUGES movedY=%.2f reloadRange=%.3f hpChanged=1 frozenFrames=120 layoutWrites=1",c.group(heavymach::hud::gauges).visualBounds.y,hi-lo);
}
- (void)testHUDAssetOwnershipAudit {
#if !TARGET_OS_OSX
 [self startFixture];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");
 session->validationSetCheats(true);session->setAimMode(heavymach::AimMode::automatic);render(self.game);
 HMGLRenderer* gl=[self.game valueForKey:@"glRenderer"];
 auto attach=[&](UIImage* image,NSString* name){XCTAssertNotNil(image);if(image){auto a=[XCTAttachment attachmentWithImage:image];a.name=name;a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}};
 attach([gl snapshot],@"HUD_actual_game");
 HMHUDRecordingMedia* media=[HMHUDRecordingMedia new];media.submitted=[NSMutableArray new];
 NSArray* assets=@[@"GameUI_Top.png",@"GameUI_Btm.png",@"GameMenu.png",@"GameUI_Exp.png",@"NumSet01.png",@"GameUI_Gauge.png",@"GameUI_BlueLight.png",@"GameUI_RedLight.png",@"GameUI_Gear.png",@"Btn_Booster.png",@"Btn_Weapon.png",@"WeaponTile.png",@"NumSet02b.png",@"ItemTile.png",@"SelWeapon.png",@"Extensions/AutoAimControls.png"];
 NSMutableArray* sizes=[NSMutableArray new];
 for(NSString* name in assets){UIImage* im=[media image:name];XCTAssertNotNil(im);if(!im)continue;
  attach(im,[@"HUD_asset_" stringByAppendingString:name.lastPathComponent.stringByDeletingPathExtension]);
  [sizes addObject:@{@"asset":name,@"width":@(im.size.width),@"height":@(im.size.height)}];
 }
 [gl setLogicalWidth:480 height:320];storage<CGPoint>(self.game,"_hudOffset")=CGPointZero;storage<CGFloat>(self.game,"_hudScale")=1;
 HMMedia* original=[self.game valueForKey:@"media"];[self.game setValue:media forKey:@"media"];
 auto slot=storage<heavymach::SaveData>(self.game,"_save").slots[0];slot.setInt(0x28,99);
 auto& data=storage<heavymach::GameData>(self.game,"_data");heavymach::PlayerPassives passives;
 for(auto& p:passives.slots){p.active=true;p.number=999;p.time=30;}
 NSMutableDictionary* samples=[NSMutableDictionary new];NSData* basePixels=nil;
 @try {
  for(NSString* state in @[@"normal",@"partitioned",@"normal_repeat",@"menu_pressed",@"jump_pressed",@"weapon_pressed",@"fire_pressed",@"next_pressed",@"extras",@"manual",@"group_menu",@"group_progress",@"group_jump",@"group_weapon",@"group_gauges",@"group_aim",@"group_passives",@"group_preview"]){
   HMGameHUD* hud=[[HMGameHUD alloc]initWithMedia:media];[hud setHPGauge:.6];
   for(int n=0;n<120;++n)[hud tick:n/60.];
   HMGameHUDSnapshot s;s.slot=&slot;s.data=&data;s.weapon=&data.weapons.at(slot.weaponName(0));s.time=5;s.expGauge=1;s.reloadGauge=.75;s.ammo=999;
   media.isolatedGroup=[state hasPrefix:@"group_"]?[state substringFromIndex:6]:nil;
   if([state isEqualToString:@"extras"]||media.isolatedGroup){s.passives=&passives;[hud showWeaponChange:5];[hud setReloadComplete:5];s.time=5.05;}
   session->setAimMode([state isEqualToString:@"manual"]?heavymach::AimMode::manual:heavymach::AimMode::automatic);
   NSMapTable* aimTouches=[self.game valueForKey:@"aimTouches"];[aimTouches removeAllObjects];
   if([state isEqualToString:@"fire_pressed"])[aimTouches setObject:@2 forKey:@42];
   if([state isEqualToString:@"next_pressed"])[aimTouches setObject:@3 forKey:@42];
   media.partitionBackgrounds=[state isEqualToString:@"partitioned"];
   XCTAssertTrue([gl beginFrame]);media.glRenderer=gl;[media setDrawingOffset:CGPointZero];[media setDrawingScale:1];[media resetClip];
   [hud draw:s]; // Establish the real button receivers before pressing one.
   if([state isEqualToString:@"menu_pressed"])[hud touch:CGPointMake(38,279) state:0];
   if([state isEqualToString:@"jump_pressed"])[hud touch:CGPointMake(34,34) state:0];
   if([state isEqualToString:@"weapon_pressed"])[hud touch:CGPointMake(103,31) state:0];
   [gl endFrame];XCTAssertTrue([gl beginFrame]);[media.submitted removeAllObjects];
   [hud draw:s];[self.game drawAimControls];[gl finish];[gl endFrame];media.glRenderer=nil;
   UIImage* im=[gl snapshot];attach(im,[@"HUD_" stringByAppendingString:state]);
   NSData* png=UIImagePNGRepresentation(im);if([state isEqualToString:@"normal"])basePixels=png;
   if([state isEqualToString:@"partitioned"]||[state isEqualToString:@"normal_repeat"])XCTAssertEqualObjects(png,basePixels,@"Backgrounds must reconstruct the same GPU image");
   samples[state]=[media.submitted copy];XCTAssertEqualObjects([gl diagnostics][@"lastError"],@"");
  }
 } @finally {[self.game setValue:original forKey:@"media"];media.glRenderer=nil;}
 NSData* json=[NSJSONSerialization dataWithJSONObject:@{@"assets":sizes,@"states":samples} options:NSJSONWritingPrettyPrinted error:nil];
 auto a=[XCTAttachment attachmentWithData:json uniformTypeIdentifier:@"public.json"];a.name=@"HUD_draw_inventory";a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];
#endif
}
- (void)testGroundAAMediumSizeAndTurretJoin {
 [self.game setFrame:CGRectMake(0,0,1800,600)];[self.game updateCanvasGeometry];
 ((HMValidationControls*)[self.game valueForKey:@"validationControls"]).selectedSegmentIndex=1;
 auto& data=storage<heavymach::GameData>(self.game,"_data");auto& map=data.maps.at("Stage01");
 map.enemies.clear();map.objects.clear();map.left=0;map.right=3000;map.bottom=-200;map.collisionPaths={{{0,0},{3000,0}}};
 [self startFixture];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");session->validationSetCheats(true);
 float x=session->player()->position.x+150;
 for(const char* name:{"Tank001","GAA_Shilka_M1","GAA_Tunguska_M1","Tank030"}){session->validationSpawnEnemy({name,{x,4},false});x+=140;}
 frames(self.game,40);XCTAssertEqual(session->validationEnemies().size(),size_t(4));
 HMGLRenderer* gl=[self.game valueForKey:@"glRenderer"];
#if TARGET_OS_OSX
 NSImage* picture=[gl snapshot];NSData* png=[[NSBitmapImageRep imageRepWithData:picture.TIFFRepresentation]representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
#else
 NSData* png=UIImagePNGRepresentation([gl snapshot]);
#endif
 XCTAssertNotNil(png);if(png){XCTAttachment* a=[XCTAttachment attachmentWithData:png uniformTypeIdentifier:@"public.png"];a.name=@"GroundAA_Medium_Comparison";a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}
 XCTAssertEqualObjects([gl diagnostics][@"lastError"],@"");
}
- (void)testGroundAAAssemblyRadarWeaponsAndNoShotSave {
 XCTAssertNil([self.game valueForKey:@"loadError"]);
 [self.game setFrame:CGRectMake(0,0,1200,600)];[self.game updateCanvasGeometry];
 ((HMValidationControls*)[self.game valueForKey:@"validationControls"]).selectedSegmentIndex=1;
 HMMedia* media=[self.game valueForKey:@"media"];HMGLRenderer* gl=[self.game valueForKey:@"glRenderer"];
 for(const char* unit:{"GAA_Shilka_M1","GAA_Tunguska_M1"}){
  [self startFixture];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");session->validationSetCheats(true);
  NSUInteger preparedDecodes=[[media cacheDiagnostics][@"decodeCount"] unsignedIntegerValue];
 for(const auto& art:heavymach::groundAAImages()){NSString* name=[NSString stringWithUTF8String:art.name.c_str()];CGSize size=[media imageSize:name scope:@"shared"];XCTAssertEqual(size.width,art.size.x,@"%@",name);XCTAssertEqual(size.height,art.size.y,@"%@",name);}
  XCTAssertEqual([[media cacheDiagnostics][@"decodeCount"] unsignedIntegerValue],preparedDecodes);
  auto pos=session->player()->position;session->validationSpawnEnemy({unit,{pos.x+180,session->world().mapHeight(pos.x+180)+30},false});session->validationSpawnEnemy({"Tank001",{pos.x+60,pos.y+30},false});
  std::set<std::string> radars,bodies;size_t maxBullets=0;NSUInteger writes=((HMInputTestView*)self.game).gameSaveCalls;std::vector<double> work;
  for(int tick=0;tick<420;++tick){auto start=std::chrono::steady_clock::now();frames(self.game,1);if(tick>=60)work.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());maxBullets=std::max(maxBullets,session->bulletCount());
   for(const auto& d:session->sprites())if(d.tile.starts_with("Extensions/GAA_")){XCTAssertTrue(std::isfinite(d.rotation));if(d.tile.find("Radar")!=std::string::npos)radars.insert(d.tile);if(d.tile.find("Body")!=std::string::npos)bodies.insert(d.tile);}
   if(tick==120||tick==240||(tick<120&&tick%5==0)){
#if TARGET_OS_OSX
    NSImage* picture=[gl snapshot];NSData* png=[[NSBitmapImageRep imageRepWithData:picture.TIFFRepresentation]representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
#else
    NSData* png=UIImagePNGRepresentation([gl snapshot]);
#endif
    XCTAssertNotNil(png);if(png){XCTAttachment* a=[XCTAttachment attachmentWithData:png uniformTypeIdentifier:@"public.png"];a.name=[NSString stringWithFormat:@"GroundAA_%s_%d",unit,tick];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}
   }
  }
  XCTAssertEqual(radars.size(),size_t(heavymach::groundAARadarFrames));XCTAssertGreaterThanOrEqual(bodies.size(),size_t(2));XCTAssertGreaterThan(maxBullets,size_t(2));XCTAssertEqual(((HMInputTestView*)self.game).gameSaveCalls,writes);
  session->setPaused(true);double time=session->world().gameTime;frames(self.game,30);XCTAssertEqual(session->world().gameTime,time);session->setPaused(false);frames(self.game,30);XCTAssertGreaterThan(session->world().gameTime,time);
  std::sort(work.begin(),work.end());NSLog(@"HM_GROUND_AA %s radar_frames=%zu body_frames=%zu max_bullets=%zu work_p95_ms=%.3f shot_save_calls=%lu",unit,radars.size(),bodies.size(),maxBullets,work[work.size()*95/100],(unsigned long)(((HMInputTestView*)self.game).gameSaveCalls-writes));
 }
 XCTAssertEqualObjects([gl diagnostics][@"lastError"],@"");
}
- (void)testLuxGoldCanopiesAndAircraftSize {
 HMMedia* media=[HMMedia new];HMGLRenderer* gl=[HMGLRenderer new];XCTAssertNotNil(gl);media.glRenderer=gl;XCTAssertTrue([gl beginFrame]);
 [media draw:@"BG_City01.png" x:240 y:160];
 NSArray<NSString*>* names=@[@"E_Heli_Body_01.png",@"E_Heli_Body_06.png",@"Extensions/Lux_F22.png",@"Extensions/Lux_Su57.png",@"Extensions/Lux_MiG31.png"];
 for(NSUInteger row=0;row<names.count;++row){CGSize size=[media imageSize:names[row]];CGFloat height=size.height;
  if(row>=2){const auto& spec=heavymach::fighterSpecs()[row-2];height=spec.width*spec.aspect;XCTAssertEqual(size.width,spec.width);XCTAssertGreaterThan(size.width,[media imageSize:names[0]].width);XCTAssertLessThan(size.width,[media imageSize:names[1]].width);}
  for(int col=0;col<2;++col)[media draw:names[row] x:150+col*180 y:280-row*55 columns:1 rows:1 tile:0 scaleX:1 scaleY:height/size.height rotation:col*.14 alpha:1];
 }
 [gl finish];[gl endFrame];media.glRenderer=nil;XCTAssertEqualObjects([gl diagnostics][@"lastError"],@"");
#if !TARGET_OS_OSX
 UIImage* picture=[gl snapshot];XCTAssertNotNil(picture);XCTAttachment* attachment=[XCTAttachment attachmentWithImage:picture];attachment.name=@"Lux_Gold_Size_Edges";attachment.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:attachment];
#endif
}
- (void)testLuxEightAircraftBossWaveAndPause {
 ((HMValidationControls*)[self.game valueForKey:@"validationControls"]).selectedSegmentIndex=1;
 auto& data=storage<heavymach::GameData>(self.game,"_data");XCTAssertNil([self.game valueForKey:@"loadError"]);
 const auto& map=data.maps.at("Stage29");auto boss=std::find_if(map.enemies.begin(),map.enemies.end(),[](const auto& e){return e.name=="Boss03";});XCTAssertTrue(boss!=map.enemies.end());if(boss==map.enemies.end())return;
 heavymach::ValidationScenario fixture;fixture.mission=3;fixture.stage=4;fixture.allWeapons=true;fixture.internalLevel=50;fixture.grades={6,6,6};fixture.playerPosition=heavymach::Vector2{boss->position.x-450,boss->position.y};
 storage<std::optional<heavymach::ValidationScenario>>(self.game,"_validationFixture")=fixture;[self.game startSlot:0 mode:0 mission:3 stage:4];
 auto& game=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");XCTAssertTrue(bool(game));if(!game)return;game->validationSetCheats(true);frames(self.game,5);
 auto count=[&](){int n=0;for(const auto& e:game->validationEnemies())if(e.name.starts_with("Lux_")){++n;XCTAssertEqual(e.hpMax,765);}return n;};XCTAssertEqual(count(),8);
 for(const auto& sprite:game->sprites())if(sprite.tile.starts_with("Extensions/Lux_"))XCTAssertGreaterThan(sprite.position.x-60,game->canvasWidth());
 auto previousEntry=game->validationEnemies();
 auto checkFormation=[&]{auto enemies=game->validationEnemies();for(size_t a=0;a<enemies.size();++a){auto sa=heavymach::aircraftSpec(enemies[a].name);if(!enemies[a].name.starts_with("Lux_")||!sa)continue;
  for(size_t b=0;b<a;++b){auto sb=heavymach::aircraftSpec(enemies[b].name);if(!enemies[b].name.starts_with("Lux_")||!sb)continue;
   float dx=std::fabs(enemies[a].position.x-enemies[b].position.x),dy=std::fabs(enemies[a].position.y-enemies[b].position.y);
   XCTAssertTrue(dx>=(sa->width+sb->width)*.5f+10 || dy>=(sa->width*sa->aspect+sb->width*sb->aspect)*.5f+10);
  }
 }};checkFormation();
 std::set<std::string> seen;NSUInteger saved=((HMInputTestView*)self.game).gameSaveCalls;size_t maxBullets=0;std::vector<double> work;
 for(int n=0;n<450;++n){auto start=std::chrono::steady_clock::now();frames(self.game,1);if(n>=30)work.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());maxBullets=std::max(maxBullets,game->bulletCount());
  if(n<150){checkFormation();auto currentEntry=game->validationEnemies();for(const auto& enemy:currentEntry)if(enemy.name.starts_with("Lux_")){auto old=std::find_if(previousEntry.begin(),previousEntry.end(),[&](const auto& e){return e.id==enemy.id;});if(old!=previousEntry.end())XCTAssertLessThanOrEqual(std::fabs(enemy.position.x-old->position.x),2.51f);}previousEntry=std::move(currentEntry);}
#if !TARGET_OS_OSX
  if(n==30||n==60||n==120||n==170||n==200){HMGLRenderer* entryGL=[self.game valueForKey:@"glRenderer"];XCTAttachment* entry=[XCTAttachment attachmentWithImage:[entryGL snapshot]];entry.name=[NSString stringWithFormat:@"Lux_Entry_%03d",n];entry.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:entry];}
#endif
  for(const auto& sprite:game->sprites())if(sprite.tile.starts_with("Extensions/Lux_"))seen.insert(sprite.tile);}
 XCTAssertEqual(seen.size(),size_t(3));XCTAssertGreaterThan(maxBullets,size_t(10));XCTAssertEqual(count(),8);XCTAssertEqual(((HMInputTestView*)self.game).gameSaveCalls,saved);
 auto before=game->validationEnemies();game->setPaused(true);frames(self.game,10);auto after=game->validationEnemies();XCTAssertEqual(before.size(),after.size());for(size_t n=0;n<std::min(before.size(),after.size());++n){XCTAssertEqual(before[n].position.x,after[n].position.x);XCTAssertEqual(before[n].position.y,after[n].position.y);}game->setPaused(false);frames(self.game,2);XCTAssertEqual(count(),8);
 HMGLRenderer* gl=[self.game valueForKey:@"glRenderer"];XCTAssertEqualObjects([gl diagnostics][@"lastError"],@"");
#if !TARGET_OS_OSX
 UIImage* picture=[gl snapshot];XCTAssertNotNil(picture);XCTAttachment* attachment=[XCTAttachment attachmentWithImage:picture];attachment.name=@"Lux_Eight_Boss_Combat";attachment.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:attachment];
#endif
 std::sort(work.begin(),work.end());double sum=0;for(double n:work)sum+=n;NSLog(@"HM_LUX count=%d maxBullets=%zu samples=%zu mean_ms=%.3f p95_ms=%.3f saves=%lu",count(),maxBullets,work.size(),sum/work.size(),work[work.size()*95/100],(unsigned long)(((HMInputTestView*)self.game).gameSaveCalls-saved));
}
- (void)testVTOLExpansionRendersAndUsesSeparateTextures {
 // Reproducible neutral movement; a connected physical device may be tilted.
 ((HMValidationControls*)[self.game valueForKey:@"validationControls"]).selectedSegmentIndex=1;
#if !TARGET_OS_OSX
 CGSize screen=UIScreen.mainScreen.bounds.size;self.game.frame=CGRectMake(0,0,std::max(screen.width,screen.height),std::min(screen.width,screen.height));[self.game updateCanvasGeometry];
#endif
 auto& data=storage<heavymach::GameData>(self.game,"_data");XCTAssertNil([self.game valueForKey:@"loadError"]);
 HMMedia* media=[self.game valueForKey:@"media"];HMGLRenderer* gl=[self.game valueForKey:@"glRenderer"];
 int index=0;const int missions[]={1,2,5},stages[]={1,2,1};const char* maps[]={"Stage06","Stage12","Stage21"};
 for(const auto& spec:heavymach::vtolSpecs()){
  auto& map=data.maps.at(maps[index]);auto enemy=std::find_if(map.enemies.begin(),map.enemies.end(),[&](const auto& e){return e.name==spec.name;});XCTAssertTrue(enemy!=map.enemies.end());if(enemy==map.enemies.end())return;
  heavymach::ValidationScenario fixture;fixture.mission=missions[index];fixture.stage=stages[index];fixture.allWeapons=true;fixture.internalLevel=50;fixture.grades={6,6,6};fixture.playerPosition=heavymach::Vector2{enemy->position.x-130,enemy->position.y-100};
  storage<std::optional<heavymach::ValidationScenario>>(self.game,"_validationFixture")=fixture;
  [self.game startSlot:0 mode:0 mission:fixture.mission stage:fixture.stage];
  auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");XCTAssertTrue(bool(session));if(!session)return;session->validationSetCheats(true);
  bool seen=false;size_t maxBullets=0;std::vector<double> costs;NSUInteger before=((HMInputTestView*)self.game).gameSaveCalls;
  for(int n=0;n<180;++n){auto begin=std::chrono::steady_clock::now();frames(self.game,1);if(n>=30)costs.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count());maxBullets=std::max(maxBullets,session->bulletCount());for(auto& sprite:session->sprites())if(sprite.tile==spec.tile){seen=true;XCTAssertTrue(std::isfinite(sprite.rotation));}}
  XCTAssertTrue(seen,@"%s",spec.name);CGSize size=[media imageSize:[NSString stringWithUTF8String:spec.tile]];XCTAssertEqual(size.width,spec.width);XCTAssertEqualWithAccuracy(size.height,spec.width*spec.aspect,.51);
  XCTAssertEqual(((HMInputTestView*)self.game).gameSaveCalls,before);
#if !TARGET_OS_OSX
  UIImage* picture=[gl snapshot];XCTAssertNotNil(picture);if(picture){XCTAttachment* a=[XCTAttachment attachmentWithImage:picture];a.name=[NSString stringWithUTF8String:spec.name];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}
#endif
  std::sort(costs.begin(),costs.end());double sum=0;for(double c:costs)sum+=c;
  NSLog(@"HM_VTOL %s texture=%gx%g seen=%d bullets=%zu render_mean_ms=%.3f render_p95_ms=%.3f",spec.name,size.width,size.height,seen,maxBullets,sum/costs.size(),costs[costs.size()*95/100]);++index;
 }
}
- (void)testVTOLSizeAndEdgeComparison {
 // Same GPU path and scale as gameplay. Rows: Heli001, Heli006, Yak, Harrier, F35B.
 // Columns show level flight and pitched flight without other enemies hiding the body.
 HMMedia* media=[HMMedia new];HMGLRenderer* gl=[HMGLRenderer new];XCTAssertNotNil(gl);
 media.glRenderer=gl;XCTAssertTrue([gl beginFrame]);
 [media draw:@"BG_City01.png" x:240 y:160];
 NSArray<NSString*>* names=@[@"E_Heli_Body_01.png",@"E_Heli_Body_06.png",@"Extensions/VTOL_Yak141.png",@"Extensions/VTOL_Harrier.png",@"Extensions/VTOL_F35B.png"];
 for(NSUInteger row=0;row<names.count;++row){
  CGSize size=[media imageSize:names[row]];CGFloat height=size.height;
  if(row>=2){const auto& spec=heavymach::vtolSpecs()[row-2];height=spec.width*spec.aspect;XCTAssertGreaterThan(size.width,[media imageSize:names[0]].width);XCTAssertLessThan(size.width,[media imageSize:names[1]].width);}
  for(int col=0;col<2;++col)[media draw:names[row] x:150+col*180 y:280-row*55 columns:1 rows:1 tile:0 scaleX:1 scaleY:height/size.height rotation:col*.14 alpha:1];
 }
 [gl finish];[gl endFrame];media.glRenderer=nil;XCTAssertEqualObjects([gl diagnostics][@"lastError"],@"");
#if !TARGET_OS_OSX
 UIImage* picture=[gl snapshot];XCTAssertNotNil(picture);if(picture){XCTAttachment* a=[XCTAttachment attachmentWithImage:picture];a.name=@"VTOL_Size_Edges";a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}
#endif
}
- (void)startFixture {
 heavymach::ValidationScenario fixture;fixture.allWeapons=true;fixture.internalLevel=4;fixture.grades={4,4,4};fixture.playerPosition=heavymach::Vector2{250,100};
 storage<std::optional<heavymach::ValidationScenario>>(self.game,"_validationFixture")=fixture;
 [self.game startSlot:0 mode:0 mission:0 stage:0];frames(self.game,100);
 XCTAssertNil([self.game valueForKey:@"loadError"]);
}
#if TARGET_OS_OSX
- (void)testMacPresentedFrameCostsWithProductionTimer {
 HMGameView* original=[(NSObject*)NSApp.delegate valueForKey:@"game"];
 [original setApplicationActive:NO];
 NSWindow* window=[[NSWindow alloc]initWithContentRect:NSMakeRect(0,0,1200,800) styleMask:NSWindowStyleMaskBorderless backing:NSBackingStoreBuffered defer:NO];
 window.releasedWhenClosed=NO;
 @try {
 for(NSString* size in @[@"1200x800",@"screen_size"]){
  NSRect rectangle=[size isEqualToString:@"screen_size"]?NSScreen.mainScreen.frame:NSMakeRect(50,50,1200,800);
  [window setFrame:rectangle display:YES];
  for(NSString* scene in @[@"title_animation",@"slot_preview",@"stage01_firing",@"hud_editor",@"transfer_panel"]){
  __weak HMGameView* releasedView;__weak HMMedia* releasedMedia;__weak HMGLRenderer* releasedRenderer;__weak HMOpenALAudio* releasedAudio;
  @autoreleasepool{
   HMMacTimedInputView* view=[[HMMacTimedInputView alloc]initWithFrame:window.contentView.bounds];
   releasedView=view;releasedMedia=[view valueForKey:@"media"];releasedRenderer=[view valueForKey:@"glRenderer"];releasedAudio=releasedMedia.audio;
   [view setApplicationActive:NO];window.contentView=view;[window makeKeyAndOrderFront:nil];[NSApp activateIgnoringOtherApps:YES];
   auto& save=storage<heavymach::SaveData>(view,"_save");save={};for(auto& slot:save.slots)heavymach::GlobalInfo(slot).selectSlot(1);
   [view setValue:@11 forKey:@"randomState"];
   storage<heavymach::KeyboardInput>(view,"_keyboardControls")={};
   [view setValue:@NO forKey:@"resumeRequired"];storage<heavymach::LoadingTransition>(view,"_loading")={};
   if([scene isEqualToString:@"slot_preview"]||[scene isEqualToString:@"transfer_panel"])[view refreshScreen:3];
   if([scene isEqualToString:@"stage01_firing"]||[scene isEqualToString:@"hud_editor"])[view startSlot:0 mode:0 mission:0 stage:0];
   if([scene isEqualToString:@"hud_editor"]){[view setValue:@YES forKey:@"gameMenu"];[view openHUDEditor:nil];XCTAssertTrue([[view valueForKey:@"hudEditing"]boolValue]);}
   if([scene isEqualToString:@"transfer_panel"]){[view setValue:@YES forKey:@"active"];[view openTransfer:nil];[view setValue:@NO forKey:@"active"];}
   view.firing=[scene isEqualToString:@"stage01_firing"];
   view.finished=[self expectationWithDescription:[size stringByAppendingString:scene]];
   XCTestExpectation* done=view.finished;[view setApplicationActive:YES];
   [self waitForExpectations:@[done] timeout:30];[view setApplicationActive:NO];
   HMGLRenderer* gl=[view valueForKey:@"glRenderer"];
   NSDictionary* record=@{@"phase":@"native_mac_production_timer",@"size":size,@"scene":scene,@"view_width":@(view.bounds.size.width),@"view_height":@(view.bounds.size.height),@"scale":@(window.backingScaleFactor),@"frame_work":macFrameMetrics(view->work),@"callback_intervals":macFrameMetrics(view->intervals),@"renderer":[gl diagnostics],@"audio":[[view valueForKey:@"media"] audio].diagnostics,@"max_bullets":@(view->maxBullets),@"game_save_calls":@(view.gameSaveCalls)};
   NSData* json=[NSJSONSerialization dataWithJSONObject:record options:NSJSONWritingSortedKeys error:nil];NSLog(@"HM_MAC_FRAME_PERF %@",[[NSString alloc]initWithData:json encoding:NSUTF8StringEncoding]);
   XCTAssertEqual(view->work.size(),size_t(180));XCTAssertEqualObjects([gl diagnostics][@"lastError"],@"");XCTAssertNil([view valueForKey:@"loadError"]);
   if(view.firing)XCTAssertGreaterThan(view->maxBullets,NSUInteger(0));
   XCTAssertEqual(view.gameSaveCalls,NSUInteger(0));
   XCTAssertLessThan([macFrameMetrics(view->work)[@"p95_ms"]doubleValue],1000.0/60.0,@"Native Mac frame work budget");
   window.contentView=nil;
  }
  XCTAssertNil(releasedView,@"Game view must be released after scene teardown");
  XCTAssertNil(releasedMedia,@"Media must be released after scene teardown");
  XCTAssertNil(releasedRenderer,@"GL renderer must be released after scene teardown");
  XCTAssertNil(releasedAudio,@"OpenAL owner must be released after scene teardown");
  NSLog(@"HM_MAC_TEARDOWN %@ %@ view=%d media=%d renderer=%d audio=%d",size,scene,releasedView==nil,releasedMedia==nil,releasedRenderer==nil,releasedAudio==nil);
  }
 }
 } @finally {[window close];[original.window makeKeyAndOrderFront:nil];[original setApplicationActive:YES];}
}
- (void)testMacNativeFullscreenUsesEntireDrawableAndRestoresWindow {
 NSDictionary* desktop=CFBridgingRelease(CGSessionCopyCurrentDictionary());
 XCTSkipIf([desktop[@"CGSSessionScreenIsLocked"] boolValue],@"macOS desktop is locked; native fullscreen transition requires an unlocked session");
 NSWindow* window=[(NSObject*)NSApp.delegate valueForKey:@"window"];
 HMGameView* view=[(NSObject*)NSApp.delegate valueForKey:@"game"];
 XCTAssertNotNil(window);XCTAssertNotNil(view);if(!window||!view)return;
 NSRect initial=window.frame;
 BOOL wasFull=(window.styleMask&NSWindowStyleMaskFullScreen)!=0;
 @try {
  if(!wasFull){
   XCTestExpectation* entered=[self expectationForNotification:NSWindowDidEnterFullScreenNotification object:window handler:nil];
   [window toggleFullScreen:nil];[self waitForExpectations:@[entered] timeout:15];
  }
  XCTAssertTrue((window.styleMask&NSWindowStyleMaskFullScreen)!=0);
  [window.contentView layoutSubtreeIfNeeded];[view setApplicationActive:YES];[view frame:nil];
  NSRect screen=window.screen.frame,content=window.contentView.bounds,backing=[view convertRectToBacking:view.bounds];
  XCTAssertEqualWithAccuracy(window.frame.size.width,screen.size.width,1);
  XCTAssertEqualWithAccuracy(window.frame.size.height,screen.size.height,1);
  XCTAssertEqualWithAccuracy(content.size.width,view.bounds.size.width,.01);
  XCTAssertEqualWithAccuracy(content.size.height,view.bounds.size.height,.01);
  auto& viewport=storage<heavymach::Viewport>(view,"_viewport");
  XCTAssertEqualWithAccuracy(viewport.x,0,.01);XCTAssertEqualWithAccuracy(viewport.y,0,.01);
  XCTAssertEqualWithAccuracy(viewport.logicalWidth*viewport.scale,content.size.width,.01);
  XCTAssertEqualWithAccuracy(viewport.logicalHeight*viewport.verticalScale(),content.size.height,.01);
  HMGLRenderer* gl=[view valueForKey:@"glRenderer"];
  XCTAssertEqualWithAccuracy([[gl valueForKey:@"displayWidth"]doubleValue],backing.size.width,1);
  XCTAssertEqualWithAccuracy([[gl valueForKey:@"displayHeight"]doubleValue],backing.size.height,1);
  XCTAssertEqualObjects([gl diagnostics][@"lastError"],@"");
  NSLog(@"HM_MAC_FULLSCREEN window=%@ content=%@ view=%@ backing=%@ viewport=(%.3f,%.3f,%.3f,%.3f) renderer=%@",NSStringFromRect(window.frame),NSStringFromRect(content),NSStringFromRect(view.bounds),NSStringFromRect(backing),viewport.x,viewport.y,viewport.logicalWidth*viewport.scale,viewport.logicalHeight*viewport.verticalScale(),[gl diagnostics][@"renderer"]);
 } @finally {
  [view setApplicationActive:NO];
  if(!wasFull&&(window.styleMask&NSWindowStyleMaskFullScreen)){
   XCTestExpectation* exited=[self expectationForNotification:NSWindowDidExitFullScreenNotification object:window handler:nil];
   [window toggleFullScreen:nil];[self waitForExpectations:@[exited] timeout:15];
   XCTAssertEqualWithAccuracy(window.frame.size.width,initial.size.width,1);
   XCTAssertEqualWithAccuracy(window.frame.size.height,initial.size.height,1);
  }
 }
}
- (void)testValidationPanelStartsWithChosenWeaponAtEveryGrade {
 auto& data=storage<heavymach::GameData>(self.game,"_data");HMValidationPanel* panel=[[HMValidationPanel alloc]initWithData:&data];auto weapons=heavymach::validationWeapons(data);
 for(int grade=0;grade<=6;++grade)for(bool allWeapons:{false,true}){
  panel.bodyGrade=panel.topGrade=panel.weaponGrade=grade;panel.allWeapons=allWeapons;
  for(size_t i=0;i<weapons.size();++i){panel.weaponIndex=int(i);auto received=[panel scenario];heavymach::SaveSlot slot;heavymach::prepareValidationSlot(slot,data,received);XCTAssertGreaterThanOrEqual(received.selectedWeapon,0);XCTAssertLessThan(received.selectedWeapon,slot.weaponCount());XCTAssertEqual(slot.weaponName(received.selectedWeapon),weapons[i]);}
 }
}

#endif
- (void)testFullscreenRatiosAndTouchInverse {
 for(CGSize size:{CGSizeMake(1024,768),CGSizeMake(1366,1024),CGSizeMake(1440,900),CGSizeMake(912,420),CGSizeMake(800,900)}){
  self.game.frame=CGRectMake(0,0,size.width,size.height);[self.game updateCanvasGeometry];auto& vp=storage<heavymach::Viewport>(self.game,"_viewport");
  XCTAssertEqualWithAccuracy(vp.x,0,.6);XCTAssertEqualWithAccuracy(vp.y,0,.6);
  XCTAssertEqualWithAccuracy(vp.logicalWidth*vp.scale,size.width,.6);XCTAssertEqualWithAccuracy(vp.logicalHeight*vp.verticalScale(),size.height,.6);
  auto top=vp.logical(1,1),bottom=vp.logical(size.width-1,size.height-1);XCTAssertTrue(top.has_value());XCTAssertTrue(bottom.has_value());
  render(self.game);HMGLRenderer* gl=[self.game valueForKey:@"glRenderer"];XCTAssertNotNil([gl snapshot]);
 }
}
- (void)testPausedResizeAndUpperCanvasAim {
 [self startFixture];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");
 key(self.game,0x29);double time=session->world().gameTime;float x=session->player()->position.x;
 self.game.frame=CGRectMake(0,0,1400,800);[self.game updateCanvasGeometry];render(self.game);
 auto& vp=storage<heavymach::Viewport>(self.game,"_viewport");XCTAssertEqualWithAccuracy(vp.logicalWidth*vp.scale,1400,.6);XCTAssertEqualWithAccuracy(vp.logicalHeight*vp.verticalScale(),800,.6);
 XCTAssertEqual(session->world().gameTime,time);XCTAssertEqual(session->player()->position.x,x);
 self.game.frame=CGRectMake(0,0,1024,768);[self.game updateCanvasGeometry];key(self.game,0x29);frames(self.game,1);
 NSObject* token=[NSObject new];CGPoint point=CGPointMake(vp.x+470*vp.scale,vp.y+2*vp.verticalScale());[self.game routePoint:point state:0 token:token];
 auto origin=session->validationAimOrigin();auto player=session->player()->position;auto target=session->validationWeaponState().fireTarget;
 XCTAssertEqualWithAccuracy(target.x,player.x+470-origin.x,.01);XCTAssertEqualWithAccuracy(target.y,player.y+358-origin.y,.01);
 [self.game routePoint:point state:2 token:token];
}
- (void)testHeightExtensionPreservesAircraftBossMotion {
 auto& data=storage<heavymach::GameData>(self.game,"_data");HMMedia* media=[self.game valueForKey:@"media"];
 auto tiles=[media](std::string_view name){CGSize size=[media imageSize:[NSString stringWithUTF8String:std::string(name).c_str()]];return heavymach::Vector2{float(size.width),float(size.height)};};
 heavymach::SaveSlot a,b;uint32_t ra=1,rb=1;auto random=[](uint32_t& state){state^=state<<13;state^=state>>17;state^=state<<5;return int32_t(state&0x7fffffff);};
 heavymach::GameSession original(data,a,tiles,[&]{return random(ra);}),expanded(data,b,tiles,[&]{return random(rb);});
 heavymach::ValidationScenario fixture;fixture.mission=2;fixture.stage=4;fixture.bossIndex=0;fixture.internalLevel=20;fixture.grades={6,6,6};
 expanded.setCanvasHeight(360);original.startValidation(fixture);expanded.startValidation(fixture);
 for(int frame=0;frame<360;++frame){original.tick((frame+1)/60.,0);expanded.tick((frame+1)/60.,0);auto first=original.validationEnemies(),second=expanded.validationEnemies();XCTAssertEqual(first.size(),second.size());for(size_t i=0;i<std::min(first.size(),second.size());++i){XCTAssertEqualWithAccuracy(first[i].position.x,second[i].position.x,.001);XCTAssertEqualWithAccuracy(first[i].position.y,second[i].position.y,.001);}}
}
- (void)autoAimArenaWithWeapon:(const std::string&)name {
 // Test-only copy of Stage01: flat ground, one real helicopter AI added after
 // the intro. Increased HP permits measuring repeated impacts, not forced hits.
 auto& data=storage<heavymach::GameData>(self.game,"_data");auto& map=data.maps.at("Stage01");
 map.enemies.clear();map.objects.clear();map.collisionPaths={{{0,40},{10000,40}}};
 data.machines.at("Heli001").hpMax=100000;
 heavymach::ValidationScenario fixture;fixture.weapons={name};fixture.grades={4,4,4};fixture.internalLevel=4;fixture.playerPosition=heavymach::Vector2{350,100};
 storage<std::optional<heavymach::ValidationScenario>>(self.game,"_validationFixture")=fixture;
 ((HMValidationControls*)[self.game valueForKey:@"validationControls"]).selectedSegmentIndex=1;
 storage<heavymach::AimMode>(self.game,"_preferredAimMode")=heavymach::AimMode::manual;
 [self.game startSlot:0 mode:0 mission:0 stage:0];frames(self.game,110);
 auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");
 auto& slot=storage<heavymach::SaveData>(self.game,"_save").slots[0];int index=-1;
 for(int n=0;n<slot.weaponCount();++n)if(slot.weaponName(n)==name)index=n;
 XCTAssertGreaterThanOrEqual(index,0);for(int n=0;n<index;++n)key(self.game,0x08);
 auto p=session->player()->position;session->validationSpawnEnemy({"Heli001",{p.x+240,p.y+140},false});
 [self.game setInputAimMode:heavymach::AimMode::automatic];frames(self.game,2);
}
- (void)testAutoAimEveryWeaponWithMovementAndJump {
 auto& data=storage<heavymach::GameData>(self.game,"_data");auto names=heavymach::validationWeapons(data);XCTAssertEqual(names.size(),size_t(20));
 int index=0;
 for(const auto& name:names){
  [self autoAimArenaWithWeapon:name];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");
  auto target=session->aimSnapshot().target;XCTAssertNotEqual(target,uintptr_t(0),@"%s",name.c_str());
  NSUInteger saves=((HMInputTestView*)self.game).gameSaveCalls,aimSaves=((HMInputTestView*)self.game).aimSaveCalls;
  float x=session->player()->position.x,y=session->player()->position.y,maxY=y;bool moved=false;
  int shots=0;session->validationObserveShots([&](const heavymach::WeaponInfo&,float,heavymach::Vector2){++shots;});
  NSObject* fireTouch=[NSObject new];auto r=[self.game aimControlRect:2];auto vp=storage<heavymach::Viewport>(self.game,"_viewport");
  CGPoint firePoint=CGPointMake(vp.x+CGRectGetMidX(r)*vp.scale,vp.y+(vp.logicalHeight-CGRectGetMidY(r))*vp.verticalScale());
  if(index%2)[self.game handleKeyCode:9 pressed:YES];else [self.game routePoint:firePoint state:0 token:fireTouch];
  [self.game handleKeyCode:4 pressed:YES];key(self.game,0x2c);
  for(int frame=0;frame<180;++frame){
   if(frame==25){[self.game handleKeyCode:4 pressed:NO];[self.game handleKeyCode:7 pressed:YES];}
   if(frame==50)[self.game handleKeyCode:7 pressed:NO];
   frames(self.game,1);maxY=std::max(maxY,session->player()->position.y);moved=moved||std::fabs(session->player()->position.x-x)>5;
  }
  if(index%2)[self.game handleKeyCode:9 pressed:NO];else [self.game routePoint:firePoint state:2 token:fireTouch];
  session->validationObserveShots({});auto aim=session->aimSnapshot();
  XCTAssertGreaterThan(shots,0,@"%s",name.c_str());XCTAssertGreaterThan(aim.targetHits,uint64_t(0),@"%s",name.c_str());
  bool damaged=false;for(const auto& enemy:session->validationEnemies())if(enemy.id==target)damaged=enemy.hp<enemy.hpMax;
  XCTAssertTrue(damaged,@"%s",name.c_str());XCTAssertTrue(moved,@"%s",name.c_str());XCTAssertGreaterThan(maxY,y+5,@"%s",name.c_str());
  XCTAssertFalse(session->validationWeaponState().isFire);XCTAssertEqual(((HMInputTestView*)self.game).gameSaveCalls,saves);XCTAssertEqual(((HMInputTestView*)self.game).aimSaveCalls,aimSaves);
  NSLog(@"HM_AUTO_AIM %s shots=%d assisted=%llu hits=%llu jump=%.2f moved=%d saveWrites=%lu",name.c_str(),shots,aim.assistedShots,aim.targetHits,maxY-y,moved,(unsigned long)(((HMInputTestView*)self.game).gameSaveCalls-saves));
#if !TARGET_OS_OSX
  if(index==0||name=="P_Wpn_Canon03"||name=="P_Wpn_RaderLsr"){
   HMGLRenderer* gl=[self.game valueForKey:@"glRenderer"];UIImage* picture=[gl snapshot];if(picture){XCTAttachment* a=[XCTAttachment attachmentWithImage:picture];a.name=[@"AutoAim_" stringByAppendingString:[NSString stringWithUTF8String:name.c_str()]];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];}
  }
#endif
  ++index;
 }
}
- (void)testAutoAimLabelStaysInsideCanvas {
 for(float width:{480.f,700.f})for(float height:{320.f,360.f})for(float x:{-20.f,200.f,width-10})for(float y:{10.f,height-5}){
  for(auto status:{heavymach::AimStatus::tracking,heavymach::AimStatus::ready,heavymach::AimStatus::blocked}){
   auto label=heavymach::aimHUDLabel(x,x+45,y,width,height,status,1);
   auto strokes=heavymach::aimHUDText(label);XCTAssertGreaterThan(strokes.size,size_t(0));
   for(size_t i=0;i<strokes.size;i+=2){XCTAssertGreaterThanOrEqual(strokes.points[i],0);XCTAssertLessThan(strokes.points[i],width);XCTAssertGreaterThan(strokes.points[i+1],64);XCTAssertLessThan(strokes.points[i+1],height);}
   NSString* expected=status==heavymach::AimStatus::ready?@"LOCKED":status==heavymach::AimStatus::blocked?@"BLOCKED":@"TRACK";
   XCTAssertEqualObjects([NSString stringWithUTF8String:label.text],expected);XCTAssertFalse(label.flash);
  }
 }
 XCTAssertTrue(heavymach::aimHUDLabel(100,140,200,480,320,heavymach::AimStatus::ready,.18).flash);
}
- (void)testAutoAimControlsAttachToHUDAndNextTargetTouch {
 [self autoAimArenaWithWeapon:"P_Wpn_Gun01"];
 auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");
 for(CGSize size : {CGSizeMake(1200,900),CGSizeMake(1400,650)}){
  self.game.frame=CGRectMake(0,0,size.width,size.height);[self.game updateCanvasGeometry];
  auto offset=storage<CGPoint>(self.game,"_hudOffset");auto scale=storage<CGFloat>(self.game,"_hudScale");
  for(int button=1;button<=3;++button){CGRect r=[self.game aimControlRect:button];
   XCTAssertEqualWithAccuracy((r.origin.y-offset.y)/scale,10,.01);
   XCTAssertGreaterThanOrEqual((r.origin.x-offset.x)/scale,334);
   XCTAssertLessThanOrEqual((CGRectGetMaxX(r)-offset.x)/scale,480);
   XCTAssertLessThanOrEqual((CGRectGetMaxY(r)-offset.y)/scale,64);
  }
 }
 XCTAssertLessThan(CGRectGetMaxX([self.game aimControlRect:1]),CGRectGetMinX([self.game aimControlRect:3]));
 XCTAssertLessThan(CGRectGetMaxX([self.game aimControlRect:3]),CGRectGetMinX([self.game aimControlRect:2]));
 auto vp=storage<heavymach::Viewport>(self.game,"_viewport");CGRect r=[self.game aimControlRect:3];
 CGPoint next=CGPointMake(vp.x+CGRectGetMidX(r)*vp.scale,vp.y+(vp.logicalHeight-CGRectGetMidY(r))*vp.verticalScale());
 auto first=session->aimSnapshot().target;auto p=session->player()->position;session->validationSpawnEnemy({"Heli001",{p.x+320,p.y+150},false});
 NSObject* finger=[NSObject new];[self.game routePoint:next state:0 token:finger];[self.game routePoint:next state:2 token:finger];frames(self.game,2);
 XCTAssertNotEqual(session->aimSnapshot().target,first);XCTAssertFalse(session->validationWeaponState().isFire);
 HMMedia* media=[self.game valueForKey:@"media"];CGSize sheet=[media imageSize:@"Extensions/AutoAimControls.png"];
 XCTAssertEqual(sheet.width,138);XCTAssertEqual(sheet.height,92);
}
- (void)testAutoAimSingleToggleRestoresManualTouchFire {
 [self autoAimArenaWithWeapon:"P_Wpn_Gun01"];
 auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");
 XCTAssertTrue(CGRectIsNull([self.game aimControlRect:0]));
 XCTAssertTrue(CGRectIsNull([self.game aimControlRect:4]));
 auto vp=storage<heavymach::Viewport>(self.game,"_viewport");auto r=[self.game aimControlRect:1];
 CGPoint a=CGPointMake(vp.x+CGRectGetMidX(r)*vp.scale,vp.y+(vp.logicalHeight-CGRectGetMidY(r))*vp.verticalScale());
 NSObject* toggle=[NSObject new];
 [self.game routePoint:a state:0 token:toggle];[self.game routePoint:a state:2 token:toggle];
 XCTAssertEqual(session->aimMode(),heavymach::AimMode::manual);
 NSObject* world=[NSObject new];CGPoint target=CGPointMake(vp.x+430*vp.scale,vp.y+(vp.logicalHeight-180)*vp.verticalScale());
 [self.game routePoint:target state:0 token:world];frames(self.game,12);
 XCTAssertTrue(session->validationWeaponState().isFire);
 [self.game routePoint:target state:2 token:world];
 [self.game routePoint:a state:0 token:toggle];[self.game routePoint:a state:2 token:toggle];
 XCTAssertEqual(session->aimMode(),heavymach::AimMode::automatic);
 XCTAssertFalse(session->validationWeaponState().isFire);
 // A cancelled toggle must not change the mode.
 [self.game routePoint:a state:0 token:toggle];[self.game routePoint:a state:3 token:toggle];
 XCTAssertEqual(session->aimMode(),heavymach::AimMode::automatic);
}
- (void)testAutoAimModeSwitchAndTouchKeyboardOwnership {
 [self autoAimArenaWithWeapon:std::string("P_Wpn_Gun01")];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");
 auto vp=storage<heavymach::Viewport>(self.game,"_viewport");auto point=[&](int button){auto r=[self.game aimControlRect:button];return CGPointMake(vp.x+CGRectGetMidX(r)*vp.scale,vp.y+(vp.logicalHeight-CGRectGetMidY(r))*vp.verticalScale());};
 NSObject* fire=[NSObject new];NSObject* mode=[NSObject new];NSObject* world=[NSObject new];
 [self.game routePoint:point(2) state:0 token:fire];frames(self.game,12);XCTAssertTrue(session->validationWeaponState().isFire);
 // Releasing an unrelated finger cannot release the dedicated fire button.
 CGPoint empty=CGPointMake(vp.x+vp.logicalWidth*.7*vp.scale,vp.y+vp.logicalHeight*.3*vp.verticalScale());
 [self.game routePoint:empty state:0 token:world];[self.game routePoint:empty state:3 token:world];frames(self.game,2);XCTAssertTrue(session->validationWeaponState().isFire);
 [self.game handleKeyCode:9 pressed:YES];[self.game routePoint:point(2) state:2 token:fire];frames(self.game,2);XCTAssertTrue(session->validationWeaponState().isFire);
 [self.game handleKeyCode:7 pressed:YES];
 [self.game routePoint:point(1) state:0 token:mode];[self.game routePoint:point(1) state:2 token:mode];
 XCTAssertEqual(session->aimMode(),heavymach::AimMode::manual);XCTAssertFalse(session->validationWeaponState().isFire);
 XCTAssertTrue(storage<heavymach::KeyboardInput>(self.game,"_keyboardControls").down(heavymach::KeyAction::right));
 [self.game handleKeyCode:9 pressed:YES];frames(self.game,2);XCTAssertFalse(session->validationWeaponState().isFire);
 [self.game handleKeyCode:9 pressed:NO];[self.game handleKeyCode:9 pressed:YES];frames(self.game,2);XCTAssertTrue(session->validationWeaponState().isFire);
 [self.game setInputAimMode:heavymach::AimMode::off];[self.game handleKeyCode:9 pressed:NO];key(self.game,9);frames(self.game,4);XCTAssertFalse(session->validationWeaponState().isFire);
 [self.game handleKeyCode:7 pressed:NO];[self.game setInputAimMode:heavymach::AimMode::automatic];frames(self.game,2);
 auto first=session->aimSnapshot().target;auto p=session->player()->position;session->validationSpawnEnemy({"Heli001",{p.x+320,p.y+150},false});key(self.game,0x15);frames(self.game,2);XCTAssertNotEqual(session->aimSnapshot().target,first);
 [self.game handleKeyCode:9 pressed:YES];[self.game keyboardDisconnected:nil];frames(self.game,2);XCTAssertFalse(session->validationWeaponState().isFire);
 // Switching mode while the fire finger remains down must discard its capture.
 [self.game routePoint:point(2) state:0 token:fire];frames(self.game,2);XCTAssertTrue(session->validationWeaponState().isFire);
 [self.game routePoint:point(1) state:0 token:mode];[self.game routePoint:point(1) state:2 token:mode];
 [self.game setInputAimMode:heavymach::AimMode::automatic];[self.game routePoint:point(2) state:1 token:fire];frames(self.game,2);XCTAssertFalse(session->validationWeaponState().isFire);
 [self.game routePoint:point(2) state:2 token:fire];[self.game routePoint:point(2) state:0 token:fire];frames(self.game,2);XCTAssertTrue(session->validationWeaponState().isFire);
 [self.game routePoint:point(2) state:3 token:fire];frames(self.game,2);XCTAssertFalse(session->validationWeaponState().isFire);
 XCTAssertEqual(((HMInputTestView*)self.game).keySaveCalls,NSUInteger(0));
}
- (void)testKeyboardMoveAimJumpFireAndRelease {
 [self startFixture];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");
 NSUInteger saves=((HMInputTestView*)self.game).gameSaveCalls;
 float start=session->player()->position.x;[self.game handleKeyCode:7 pressed:YES];frames(self.game,80);XCTAssertGreaterThan(session->player()->position.x,start+10);
 [self.game handleKeyCode:7 pressed:NO];XCTAssertEqual(storage<heavymach::TiltState>(self.game,"_tilt").y,0);
 float right=session->player()->position.x;[self.game handleKeyCode:4 pressed:YES];frames(self.game,60);[self.game handleKeyCode:4 pressed:NO];XCTAssertLessThan(session->player()->position.x,right);
 float y=session->player()->position.y;key(self.game,0x2c);frames(self.game,8);XCTAssertGreaterThan(session->player()->position.y,y);
 [self.game handleKeyCode:0x52 pressed:YES];frames(self.game,10);[self.game handleKeyCode:0x52 pressed:NO];CGPoint aim=storage<CGPoint>(self.game,"_keyboardAim");XCTAssertGreaterThan(aim.y,100);
 [self.game handleKeyCode:9 pressed:YES];frames(self.game,40);XCTAssertTrue(storage<BOOL>(self.game,"_keyboardFire"));XCTAssertGreaterThan(session->bulletCount(),size_t(0));
 [self.game keyboardDisconnected:nil];XCTAssertFalse(storage<BOOL>(self.game,"_keyboardFire"));XCTAssertFalse(storage<heavymach::KeyboardInput>(self.game,"_keyboardControls").moving());
 key(self.game,0x29);XCTAssertTrue(session->world().paused);key(self.game,0x29);XCTAssertFalse(session->world().paused);
 XCTAssertEqual(((HMInputTestView*)self.game).gameSaveCalls,saves);
 XCTAssertEqual(((HMInputTestView*)self.game).keySaveCalls,NSUInteger(0));
}
- (void)testEveryWeaponKeyboardSelectionAndFire {
 [self startFixture];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");auto& data=storage<heavymach::GameData>(self.game,"_data");
 auto names=heavymach::validationWeapons(data);XCTAssertEqual(names.size(),size_t(20));
 for(const auto& name:names){
  heavymach::ValidationScenario fixture;fixture.weapons={name};fixture.grades={4,4,4};fixture.internalLevel=4;
  storage<std::optional<heavymach::ValidationScenario>>(self.game,"_validationFixture")=fixture;[self.game startSlot:0 mode:0 mission:0 stage:0];frames(self.game,110);
  auto& slot=storage<heavymach::SaveData>(self.game,"_save").slots[0];
  key(self.game,0x14);XCTAssertEqual(session->selectedWeapon(),slot.weaponCount()-1);key(self.game,0x08);XCTAssertEqual(session->selectedWeapon(),0);
  int index=-1;for(int i=0;i<slot.weaponCount();++i)if(slot.weaponName(i)==name)index=i;XCTAssertGreaterThanOrEqual(index,0);
  for(int i=0;i<index;++i)key(self.game,0x08);XCTAssertEqual(session->selectedWeapon(),index);
  storage<CGPoint>(self.game,"_keyboardAim")=CGPointMake(450,220);storage<BOOL>(self.game,"_hasKeyboardAim")=YES;
  NSUInteger saves=((HMInputTestView*)self.game).gameSaveCalls;
  int shots=0;session->validationObserveShots([&](const heavymach::WeaponInfo&,float,heavymach::Vector2){++shots;});
  [self.game handleKeyCode:9 pressed:YES];size_t maxBullets=0;for(int n=0;n<240;++n){frames(self.game,1);maxBullets=std::max(maxBullets,session->bulletCount());}
  [self.game handleKeyCode:9 pressed:NO];XCTAssertGreaterThan(shots,0,@"%s",name.c_str());session->validationObserveShots({});XCTAssertNil([self.game valueForKey:@"loadError"]);
  XCTAssertEqual(((HMInputTestView*)self.game).gameSaveCalls,saves,@"%s",name.c_str());XCTAssertEqual(((HMInputTestView*)self.game).keySaveCalls,NSUInteger(0));
  NSLog(@"HM_APPLE_WEAPON %s bullets=%zu playerShots=%d saveCalls=%lu",name.c_str(),maxBullets,shots,(unsigned long)(((HMInputTestView*)self.game).gameSaveCalls-saves));
 }
}
- (void)testKeyboardMenusAndSettings {
 [self.game refreshScreen:3];frames(self.game,10);HMMenus* menus=[self.game valueForKey:@"menus"];
 key(self.game,0x2b);XCTAssertFalse(CGRectIsEmpty([menus keyboardFocusRect]));
 // Navigate by keyboard focus, never activate a hidden button or synthesize a touch.
 for(int i=0;i<12&&!storage<heavymach::LoadingTransition>(self.game,"_loading").visible();++i){
  if(storage<int>(menus,"_keyboardFocus")==1000)key(self.game,0x28);else key(self.game,0x2b);frames(self.game,1);
 }
 XCTAssertTrue(storage<heavymach::LoadingTransition>(self.game,"_loading").visible());frames(self.game,100);
 XCTAssertTrue(storage<BOOL>(self.game,"_showMenu"));
 for(int screen=0;screen<2;++screen){
  for(int i=0;i<40&&!storage<heavymach::LoadingTransition>(self.game,"_loading").visible();++i){if(storage<int>(menus,"_keyboardFocus")==1000)key(self.game,0x28);else key(self.game,0x2b);frames(self.game,1);}
  XCTAssertTrue(storage<heavymach::LoadingTransition>(self.game,"_loading").visible());frames(self.game,120);
 }
 XCTAssertNotEqual(storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session").get(),nullptr);
 key(self.game,0x3b);XCTAssertTrue(storage<BOOL>(self.game,"_keyboardSettingsOpen"));
 key(self.game,0x28);key(self.game,0x1d);XCTAssertEqual(storage<heavymach::KeyboardInput>(self.game,"_keyboardControls").bindings[0],0x1du);
 key(self.game,0x2a);XCTAssertEqual(storage<heavymach::KeyboardInput>(self.game,"_keyboardControls").bindings[0],4u);
 key(self.game,0x29);XCTAssertFalse(storage<BOOL>(self.game,"_keyboardSettingsOpen"));
}
- (void)testTouchFireWhileKeyboardMoves {
 [self startFixture];auto& session=storage<std::unique_ptr<heavymach::GameSession>>(self.game,"_session");auto& vp=storage<heavymach::Viewport>(self.game,"_viewport");
 NSObject* touch=[NSObject new];CGPoint point=CGPointMake(vp.x+470*vp.scale,vp.y+(vp.logicalHeight-220)*vp.verticalScale());
 [self.game handleKeyCode:7 pressed:YES];[self.game routePoint:point state:0 token:touch];frames(self.game,40);
 XCTAssertGreaterThan(session->bulletCount(),size_t(0));XCTAssertTrue(storage<heavymach::KeyboardInput>(self.game,"_keyboardControls").moving());
 [self.game routePoint:point state:2 token:touch];[self.game handleKeyCode:7 pressed:NO];
 XCTAssertFalse(storage<heavymach::KeyboardInput>(self.game,"_keyboardControls").moving());
}
@end
