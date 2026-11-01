#import <XCTest/XCTest.h>
#import <objc/runtime.h>
#import "../../Sources/iOS/HMGameView.h"
#define HM_VALIDATION 1
#include "../../Sources/Core/GameSession.hpp"
#include "../../Sources/Core/HUDConfiguration.hpp"
#include "../../Sources/Core/KeyboardInput.hpp"
#include <stdexcept>
@interface HMGameView (SaveParity)
- (void)persistSave;
- (void)startSlot:(int)slot mode:(int)mode mission:(int)mission stage:(int)stage;
- (void)frame:(id)sender;
- (void)gameButton:(int)identifier state:(int)state;
- (void)refreshScreen:(int)screen;
- (NSURL*)hudLayoutURL;
- (void)resumeTransfer;
- (BOOL)transferAvailable;
- (NSString*)createTransferCode;
- (void)importTransferCode:(NSString*)code;
- (void)transferNotice:(NSString*)message;
@end
namespace {
template<class T>T& field(id o,const char* name){Ivar i=class_getInstanceVariable(HMGameView.class,name);if(!i)throw std::runtime_error(name);return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>((__bridge void*)o)+ivar_getOffset(i));}
}
@interface HMRealSaveView:HMGameView
@property(nonatomic)NSUInteger writes;
@end
@implementation HMRealSaveView
- (void)persistSave {++self.writes;[super persistSave];}
@end
@interface HMTransferTestView:HMRealSaveView
@property NSURL* isolatedHUD;
@property NSString* transferMessage;
@end
@implementation HMTransferTestView
- (NSURL*)hudLayoutURL {return self.isolatedHUD?:[super hudLayoutURL];}
- (void)transferNotice:(NSString*)message {self.transferMessage=message;}
@end
@interface HMSaveParityTests:XCTestCase
@end
@implementation HMSaveParityTests
- (void)testNativeTransferRoundTripStartupRecoveryAndFailuresPreserveProgress {
 HMTransferTestView* view=[[HMTransferTestView alloc]initWithFrame:CGRectMake(0,0,480,320)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 NSURL* dir=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
 auto fm=NSFileManager.defaultManager;XCTAssertTrue([fm createDirectoryAtURL:dir withIntermediateDirectories:YES attributes:nil error:nil]);
 NSURL* file=[dir URLByAppendingPathComponent:@"HvM.dat"];
 [view setValue:file forKey:@"saveURL"];view.isolatedHUD=[dir URLByAppendingPathComponent:@"HUD.json"];
 auto defaults=NSUserDefaults.standardUserDefaults;id oldKeys=[defaults objectForKey:@"KeyboardBindingsV1"],oldAim=[defaults objectForKey:@"AimModeV1"];
 @try {
  heavymach::SaveData save;heavymach::GlobalInfo info(save.slots[0]);info.selectSlot(1);info.addWeaponItem("P_Wpn_Gun02",99);save.slots[0].setInt(0x20,12345);
  auto bytes=save.encode();NSData* original=[NSData dataWithBytes:bytes.data() length:bytes.size()];XCTAssertTrue([original writeToURL:file atomically:YES]);
  NSMutableArray* positions=[NSMutableArray array];for(unsigned i=0;i<heavymach::hud::groupCount;++i)[positions addObject:i==0?@[@0.25,@0.75]:NSNull.null];
  NSData* hud=[NSJSONSerialization dataWithJSONObject:@{@"version":@1,@"positions":positions} options:0 error:nil];XCTAssertTrue([hud writeToURL:view.isolatedHUD atomically:YES]);
  NSMutableArray* keys=[NSMutableArray array];for(auto key:heavymach::KeyboardInput::defaults)[keys addObject:@(key)];[defaults setObject:keys forKey:@"KeyboardBindingsV1"];[defaults setInteger:2 forKey:@"AimModeV1"];
  [view resumeTransfer];XCTAssertNil([view valueForKey:@"loadError"]);
  XCTAssertFalse([view transferAvailable]);XCTAssertTrue([[view createTransferCode] hasPrefix:@"ERROR:"]);
  [view refreshScreen:3];XCTAssertTrue([view transferAvailable]);
  NSString* code=[view createTransferCode];XCTAssertTrue([code hasPrefix:@"HM1-"]);XCTAssertEqualObjects([NSData dataWithContentsOfURL:file],original);XCTAssertEqual(view.writes,NSUInteger(0));
  NSURL* journal=[dir URLByAppendingPathComponent:@"transfer/restore.hmcode"];
  XCTAssertFalse([fm fileExistsAtPath:[dir URLByAppendingPathComponent:@"transfer"].path]);
  [view importTransferCode:@"HM1-broken"];XCTAssertEqualObjects([NSData dataWithContentsOfURL:file],original);XCTAssertFalse([fm fileExistsAtPath:journal.path]);
  save.slots[0].setInt(0x20,54321);auto changed=save.encode();NSData* changedData=[NSData dataWithBytes:changed.data() length:changed.size()];XCTAssertTrue([changedData writeToURL:file atomically:YES]);
  [view resumeTransfer];[view importTransferCode:code];XCTAssertEqualObjects([NSData dataWithContentsOfURL:file],original);XCTAssertFalse([fm fileExistsAtPath:journal.path]);
  XCTAssertEqual(field<heavymach::SaveData>(view,"_save").slots[0].experience(),12345);
  // The same startup recovery method must finish a previously interrupted restore.
  XCTAssertTrue([changedData writeToURL:file atomically:YES]);[defaults setInteger:1 forKey:@"AimModeV1"];
  XCTAssertTrue([code writeToURL:journal atomically:YES encoding:NSUTF8StringEncoding error:nil]);[view resumeTransfer];
  XCTAssertEqualObjects([NSData dataWithContentsOfURL:file],original);XCTAssertEqualObjects([defaults objectForKey:@"AimModeV1"],@2);XCTAssertEqual(field<heavymach::SaveData>(view,"_save").slots[0].experience(),12345);XCTAssertFalse([fm fileExistsAtPath:journal.path]);
  // A corrupt journal must retain the existing save and remain available for diagnosis.
  XCTAssertTrue([@"HM1-broken" writeToURL:journal atomically:YES encoding:NSUTF8StringEncoding error:nil]);[view resumeTransfer];
  XCTAssertNotNil([view valueForKey:@"loadError"]);XCTAssertEqualObjects([NSData dataWithContentsOfURL:file],original);XCTAssertTrue([fm fileExistsAtPath:journal.path]);
  NSLog(@"HM_TRANSFER_NATIVE export_no_writes=PASS invalid_code=PASS restore=PASS startup_recovery=PASS corrupt_journal_preserves_save=PASS");
 } @finally {
  if(oldKeys)[defaults setObject:oldKeys forKey:@"KeyboardBindingsV1"];else [defaults removeObjectForKey:@"KeyboardBindingsV1"];
  if(oldAim)[defaults setObject:oldAim forKey:@"AimModeV1"];else [defaults removeObjectForKey:@"AimModeV1"];
  [view setApplicationActive:NO];[fm removeItemAtURL:dir error:nil];
 }
}
- (void)testFiringDoesNotWriteUntilOriginalMenuSavePoint {
 HMRealSaveView* view=[[HMRealSaveView alloc]initWithFrame:CGRectMake(0,0,480,320)];
 [[view valueForKey:@"frameTimer"]invalidate];[[view valueForKey:@"motionTimer"]invalidate];
 NSURL* directory=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString] isDirectory:YES];
 XCTAssertTrue([NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil]);
 NSURL* file=[directory URLByAppendingPathComponent:@"HvM.dat"];[view setValue:file forKey:@"saveURL"];
 auto& save=field<heavymach::SaveData>(view,"_save");save={};heavymach::GlobalInfo info(save.slots[0]);info.selectSlot(1);info.addWeaponItem("P_Wpn_Gun02",999);
 [view startSlot:0 mode:0 mission:0 stage:0];
 auto& session=field<std::unique_ptr<heavymach::GameSession>>(view,"_session");XCTAssertTrue(bool(session));if(!session)return;
 session->selectWeapon(4);const_cast<heavymach::GobHvM_Player*>(session->player())->hp=100000;session->touch({470,160},0);
 [view persistSave];XCTAssertEqual(view.writes,NSUInteger(1));NSData* before=[NSData dataWithContentsOfURL:file];XCTAssertEqual(before.length,NSUInteger(3624));
 NSUInteger maximumBullets=0;
 for(int i=0;i<180;++i){session->touch({470,160},1);[view frame:nil];maximumBullets=std::max(maximumBullets,session->bulletCount());}
 XCTAssertGreaterThan(maximumBullets,NSUInteger(0));XCTAssertEqual(view.writes,NSUInteger(1));
 XCTAssertEqualObjects([NSData dataWithContentsOfURL:file],before);
 [view gameButton:100 state:2];[view gameButton:102 state:2];XCTAssertEqual(view.writes,NSUInteger(2));
 NSData* after=[NSData dataWithContentsOfURL:file];auto expected=save.encode();XCTAssertEqualObjects(after,[NSData dataWithBytes:expected.data() length:expected.size()]);
 for(int i=0;i<90;++i)[view frame:nil];XCTAssertEqual(view.writes,NSUInteger(2));
 [view setApplicationActive:NO];XCTAssertEqual(view.writes,NSUInteger(2));
 NSLog(@"HM_SAVE_PARITY real_file=%@ firing_frames=180 writes_during_firing=0 explicit_menu_writes=1",file.path);
 XCTAssertTrue([NSFileManager.defaultManager removeItemAtURL:directory error:nil]);
}
@end
