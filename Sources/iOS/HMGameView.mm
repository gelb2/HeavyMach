#include "Fighters.hpp"
#include "GroundAA.hpp"
#include "HMAimOverlay.hpp"
#include "HMGameButtons.hpp"
#import "HMResources.h"
#import "HMGameView.h"
#import "HMValidationControls.h"
#import "HMGLRenderer.h"
#import "HMGLPlatform.h"
#include "KeyboardInput.hpp"
#include "HUDConfiguration.hpp"
#import "HMMedia.h"
#import "HMMenus.h"
#import "HMGameHUD.h"
#import "HMOpenALAudio.h"
#if defined(HM_VALIDATION) && HM_VALIDATION
#import "HMValidationPanel.h"
#endif
#if !TARGET_OS_OSX
#import <CoreMotion/CoreMotion.h>
#endif
#import <mach/mach_time.h>
#import <GameController/GameController.h>
#import <TargetConditionals.h>
#include "GameSession.hpp"
#include "TiltState.hpp"
#include "CameraShake.hpp"
#include "TitleState.hpp"
#include "LoadingTransition.hpp"
#include "Viewport.hpp"
#include "CanvasGeometry.hpp"
#include "GlobalInfo.hpp"
#include <ctime>
#include "SaveTransfer.hpp"
#include "FileStore.hpp"

#if !TARGET_OS_OSX
@interface HMGameView () <UITextFieldDelegate>
@end
#endif
@implementation HMGameView {
    heavymach::hud::Configuration _hudLayout,_hudDraft;
    BOOL _gameToolsFocus;
    BOOL _hudLayoutFallback,_hudEditing,_hudFromMenu,_hudExample,_hudHideGuides,_hudShowHits;
    int _hudSelected;
    id _hudDragToken;
    NSMutableSet* _hudEditorTouches;
    CGPoint _hudGrab;
    heavymach::gameui::Buttons _gameButtons;
    NSString *_hudStatus,*_gameUIMessage;
    int _transferPage;
    NSString *_transferPendingCode;
#if TARGET_OS_OSX
    NSTextField *_transferInput;
#else
    UITextField *_transferInput;
#endif
    NSUInteger _hudLayoutWrites;
    std::unique_ptr<heavymach::GameSession> _hudExampleSession;
    heavymach::GameData _hudExampleData;
    heavymach::SaveSlot _hudExampleSlot;
    heavymach::PlayerPassives _hudExamplePassives;
    uint32_t _hudExampleRandom;

    HMMedia *_media;
    #if !TARGET_OS_OSX
    UIGraphicsImageRenderer *_logicalRenderer; // CPU reference, offscreen validation only.
    UILabel *_renderError;
    #else
    NSTextField *_renderError;
    NSObject *_mouseToken;
    #endif
    HMGLRenderer *_glRenderer;
    heavymach::KeyboardInput _keyboardControls;
    BOOL _keyboardSettingsOpen,_capturingKey,_settingsWasPaused;
    unsigned _settingsSelection;


    HMMenus *_menus;
    HMGameHUD *_hud;
    #if !TARGET_OS_OSX
    CMMotionManager *_motion;
    #endif
    NSTimer *_motionTimer;
    heavymach::TiltState _tilt;
    heavymach::CameraShake _titleShake, _worldShake;
    CGPoint _keyboardAim;
    BOOL _hasKeyboardAim,_keyboardAimVisible;
    heavymach::PlayerPassives _passives;
    std::unique_ptr<heavymach::GameSession> _session;
    int _slotIndex, _ammoPreview;
    heavymach::SaveSlot *_originalSelectedSlot, *_originalWeaponSlot;
    int _originalSelectedWeapon, _originalWeaponIndex;
    BOOL _gameMenu, _leftPressed, _rightPressed, _upPressed, _downPressed, _keyboardFire;
    float _hpGauge, _expGauge, _reloadGauge;
    heavymach::GameData _data;
    heavymach::SaveData _save;
    std::shared_ptr<const heavymach::SaveSlot> _pendingSaveSnapshot;
    NSURL *_saveURL;
    BOOL _showMenu;
    int _transferScreen;
    NSString *_screenResourceScope;
    NSUInteger _screenResourceGeneration;
    NSString *_loadError;
    NSTimer *_frameTimer;
    double _nextTimerDue, _nextMotionDue;
    heavymach::TitleState _title;
    heavymach::LoadingTransition _loading;
    heavymach::Viewport _viewport;
    CGFloat _canvasWidth, _canvasHeight, _hudScale;
    CGPoint _hudOffset;
    BOOL _wideCanvasEnabled,_appliedWideCanvas;
    CGRect _menuSafeRect;
    NSHashTable<id> *_worldTouches, *_hudTouches;
    NSMapTable<id,NSNumber*> *_aimTouches,*_hudCapturedReceivers;
    NSHashTable<id> *_aimCancelledTouches;
    std::array<bool,256> _aimBlockedKeys;
    heavymach::AimMode _preferredAimMode;
    uintptr_t _drawnAimTarget;
    double _aimAcquiredTime;
    double _time, _retainedWorldTime;
    uint32_t _randomState;
    heavymach::Vector2 _retainedWorldCamera;
    BOOL _active, _trace, _resumeRequired;
    double _lastInteraction;
    NSHashTable<id> *_heldTouches;
    int _traceSecond;
#if defined(HM_VALIDATION) && HM_VALIDATION
    HMValidationPanel *_validationPanel;
    BOOL _validationPanelOpen,_validationWasPaused;
    HMValidationControls *_validationControls;
    std::optional<heavymach::ValidationScenario> _validationFixture;
#endif
}
#if !TARGET_OS_OSX
+ (Class)layerClass { return CAEAGLLayer.class; }
#else
- (BOOL)isFlipped {return YES;}
- (CGFloat)contentScaleFactor {return self.window.backingScaleFactor?:NSScreen.mainScreen.backingScaleFactor;}
- (void)setErrorText:(NSString*)text {_renderError.stringValue=text?:@"";_renderError.hidden=!text.length;}
#endif
- (instancetype)initWithFrame:(CGRect)frame {
    if ((self=[super initWithFrame:frame])) {
#if TARGET_OS_OSX
        self.wantsBestResolutionOpenGLSurface=YES;
        _mouseToken=[NSObject new];
        _glRenderer=[HMGLRenderer new];
        _renderError=[NSTextField wrappingLabelWithString:@""];
        _renderError.textColor=NSColor.whiteColor;_renderError.hidden=YES;
        [self addSubview:_renderError];
#else
        self.backgroundColor=UIColor.blackColor;
        self.multipleTouchEnabled=YES;
        self.contentScaleFactor=UIScreen.mainScreen.nativeScale;
        ((CAEAGLLayer*)self.layer).opaque=YES;
        ((CAEAGLLayer*)self.layer).drawableProperties=@{kEAGLDrawablePropertyRetainedBacking:@NO,kEAGLDrawablePropertyColorFormat:kEAGLColorFormatRGB565};
        _glRenderer=[HMGLRenderer new];
        _renderError=[[UILabel alloc] initWithFrame:self.bounds];
        _renderError.textColor=HMWhiteColor();_renderError.numberOfLines=0;
        _renderError.userInteractionEnabled=NO;_renderError.hidden=YES;
        [self addSubview:_renderError];
#endif
        NSArray *mapping=[NSUserDefaults.standardUserDefaults arrayForKey:@"KeyboardBindingsV1"];
        if(mapping.count==heavymach::KeyboardInput::count||mapping.count+1==heavymach::KeyboardInput::count){
            auto candidate=_keyboardControls.bindings;bool valid=true;std::array<bool,256> seen{};
            for(unsigned i=0;i<mapping.count;++i){unsigned code=[mapping[i] unsignedIntValue];if(!code||code>=256||code==0xe3||code==0xe7||seen[code]){valid=false;break;}seen[code]=true;candidate[i]=code;}
            if(valid&&mapping.count<candidate.size()&&seen[candidate.back()]){
                for(unsigned code=4;code<256;++code)if(!seen[code]&&code!=0xe3&&code!=0xe7){candidate.back()=code;break;}
            }
            if(valid)_keyboardControls.bindings=candidate;
        }
        [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(keyboardDisconnected:) name:GCKeyboardDidDisconnectNotification object:nil];
        _hudCapturedReceivers=[NSMapTable weakToStrongObjectsMapTable];_heldTouches=[NSHashTable weakObjectsHashTable];_worldTouches=[NSHashTable weakObjectsHashTable];_hudTouches=[NSHashTable weakObjectsHashTable];_lastInteraction=[self interactionTime];
        _aimTouches=[NSMapTable strongToStrongObjectsMapTable];_aimCancelledTouches=[NSHashTable weakObjectsHashTable];
        NSNumber* savedAim=[NSUserDefaults.standardUserDefaults objectForKey:@"AimModeV1"];
        _preferredAimMode=savedAim.integerValue==int(heavymach::AimMode::automatic)?heavymach::AimMode::automatic:heavymach::AimMode::manual;
        _canvasWidth=480;_canvasHeight=320;_hudScale=1;
        _wideCanvasEnabled=YES;_appliedWideCanvas=YES;
        _media=[HMMedia new];_media.textureRenderer=_glRenderer; _active=YES;_ammoPreview=-1;
#if !TARGET_OS_OSX
        UIGraphicsImageRendererFormat *format=[UIGraphicsImageRendererFormat defaultFormat];
        format.scale=1;format.opaque=YES;format.preferredRange=UIGraphicsImageRendererFormatRangeStandard;
        _logicalRenderer=[[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(480,320) format:format];
#endif
        _randomState=uint32_t(mach_absolute_time());
        _trace=[NSProcessInfo.processInfo.environment[@"HM_TRACE"] isEqualToString:@"1"];
#if TARGET_OS_OSX
        NSURL *directory=[[NSFileManager.defaultManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask].firstObject URLByAppendingPathComponent:NSBundle.mainBundle.bundleIdentifier];
        [NSFileManager.defaultManager createDirectoryAtURL:directory withIntermediateDirectories:YES attributes:nil error:nil];
        _saveURL=[directory URLByAppendingPathComponent:@"HvM.dat"];
#else
        _saveURL=[[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject URLByAppendingPathComponent:@"HvM.dat"];
#endif
        try {
            NSString *raw=[HMGameResourceBundle().resourcePath stringByAppendingPathComponent:@"RawData"];
            _data=heavymach::GameData::loadDirectory(raw.fileSystemRepresentation);
            heavymach::addVTOLExpansion(_data);
            heavymach::addFighterExpansion(_data);
            heavymach::addGroundAAExpansion(_data);
            if ([NSFileManager.defaultManager fileExistsAtPath:_saveURL.path]) {
                NSError *error=nil;
                NSData *bytes=[NSData dataWithContentsOfURL:_saveURL options:0 error:&error];
                if (!bytes) throw std::runtime_error(error.localizedDescription.UTF8String);
                _save=heavymach::SaveData::decode({static_cast<const uint8_t *>(bytes.bytes),bytes.length});
            }
        } catch (const std::exception& error) {
            _loadError=[NSString stringWithUTF8String:error.what()];
            NSLog(@"HeavyMach load failed; existing save preserved: %@",_loadError);
        }
        _menus=[[HMMenus alloc] initWithMedia:_media save:&_save data:&_data];
        __weak HMGameView *weakSelf=self;
        _menus.random31=^{return [weakSelf nextOriginalRandom];};
        _menus.slotSelected=^(int slot){HMGameView* view=weakSelf;if(view)view->_originalSelectedSlot=&view->_save.slots.at(slot);};
        _menus.previewPlayerCreated=^{
            HMGameView* view=weakSelf;if(!view)return;
            NSNumber* charge=[view->_hud changeWeaponForSlot:view->_originalSelectedSlot selectedWeapon:view->_originalSelectedWeapon selectedItemSlot:view->_originalWeaponSlot selectedItemIndex:view->_originalWeaponIndex time:view->_time];
            if(charge)view->_ammoPreview=charge.intValue;
        };
        _menus.saveChanged=^{[weakSelf persistSave];};
        _menus.changeScreen=^(int screen){[weakSelf requestScreen:screen];};
        _menus.startGame=^(int slot,int mode,int mission,int stage) {
            [weakSelf startSlot:slot mode:mode mission:mission stage:stage];
        };
        for(NSString* name in @[@"Loading_Top.png",@"Loading_Bottom.png",@"Loading_Left.png",@"Loading_Right.png",@"GameUI_Btm.png",@"GameUI_Top.png",@"GameMenu.png",@"GameMenuSub.png",@"GameUI_Exp.png",@"NumSet01.png",@"GameUI_Gauge.png",@"GameUI_BlueLight.png",@"GameUI_RedLight.png",@"GameUI_Gear.png",@"Btn_Booster.png",@"Btn_Weapon.png",@"WeaponTile.png",@"NumSet02b.png",@"SelWeapon.png",@"ItemTile.png",@"Text_Mission.png",@"NumSet_Mission.png"])@autoreleasepool{[_media imageSize:name scope:@"shared"];}
        _hud=[[HMGameHUD alloc] initWithMedia:_media];
        _hud.button=^(int identifier,int state) {[weakSelf gameButton:identifier state:state];};
        _hud.upgradeClosed=^{[weakSelf finishUpgrade];};
        [self loadHUDLayout];[self resumeTransfer];
        [self prepareHUDEditor];
        [self prepareScreenResources:1];
#if !TARGET_OS_OSX
        _motion=[CMMotionManager new];
        if (_motion.accelerometerAvailable) [_motion startAccelerometerUpdates];
#endif
        // The host polls the latest SDL sample at the original 50 Hz delegate interval.
        [self scheduleOriginalMotionTimer:CACurrentMediaTime()+heavymach::TiltState::interval];
        // Original guest NSTimer @0x20b8 runs through touchHLE's deadline rules.
        [self scheduleOriginalTimer:CACurrentMediaTime()+0.01666666753590107];
        self.accessibilityIdentifier=@"HeavyMach.Restored.GameView";
        self.accessibilityLabel=@"Heavy Mach";
#if defined(HM_VALIDATION) && HM_VALIDATION
        _validationControls=[HMValidationControls new];
#endif
    }
    return self;
}
#if defined(HM_VALIDATION) && HM_VALIDATION
- (void)validationDirectionChanged:(HMValidationControls *)sender {
#if TARGET_OS_OSX
    [self.window makeFirstResponder:self];
#endif
    _leftPressed=NO;_rightPressed=NO;_upPressed=NO;_downPressed=NO;
}
- (void)stopValidationDirection {
    if(_validationControls.selectedSegmentIndex!=3)_validationControls.selectedSegmentIndex=1;
    _leftPressed=NO;_rightPressed=NO;_upPressed=NO;_downPressed=NO;
}
- (void)updateValidationKeyboardDirection {
    _validationControls.selectedSegmentIndex=1+int(_rightPressed)-int(_leftPressed);
}
- (void)openValidation {
    if(_hudEditing||_transferPage||_loading.state()!=6)return;
    [self cancelGameInput];_validationWasPaused=_session&&_session->world().paused;
    if(_session)_session->setPaused(true);
    _validationPanel=[[HMValidationPanel alloc]initWithData:&_data];_validationPanel.wideCanvas=_wideCanvasEnabled;
    _validationPanelOpen=YES;_gameUIMessage=nil;
}
- (void)closeValidation {
    _validationPanelOpen=NO;_gameButtons.cancel();
    if(_session)_session->setPaused(_validationWasPaused||_resumeRequired);
}
- (void)resumeValidationSave {
    // Never applies fixture defaults here. Resume the validation bundle's own save.
    NSUserDefaults *defaults=NSUserDefaults.standardUserDefaults;
    int slot=int([defaults integerForKey:@"validation.resume.slot"]);
    if(slot<0||slot>=3||!_save.slots[slot].occupied()){
        _gameUIMessage=@"NO SAVED GAME. START A TEST FIRST.";
        return;
    }
    _validationFixture.reset();_loading=heavymach::LoadingTransition{};
    int mission=[defaults objectForKey:@"validation.resume.mission"]?int([defaults integerForKey:@"validation.resume.mission"]):_save.slots[slot].i32(0xc);
    int stage=[defaults objectForKey:@"validation.resume.stage"]?int([defaults integerForKey:@"validation.resume.stage"]):_save.slots[slot].i32(0x10);
    int mode=int([defaults integerForKey:@"validation.resume.mode"]);
    NSLog(@"HM_VALIDATION resume slot=%d mission=%d stage=%d exp=%d level=%d",slot,mission,stage,_save.slots[slot].experience(),_save.slots[slot].level());
    [self startSlot:slot mode:mode mission:mission stage:stage];
}
#endif
- (void)persistSave {
    if (_loadError) return;
    try {
#if defined(HM_VALIDATION) && HM_VALIDATION
        if(_session){auto stage=_session->stage();NSUserDefaults *defaults=NSUserDefaults.standardUserDefaults;
            [defaults setInteger:_slotIndex forKey:@"validation.resume.slot"];[defaults setInteger:stage.mission forKey:@"validation.resume.mission"];[defaults setInteger:stage.stage forKey:@"validation.resume.stage"];[defaults setInteger:_session->world().gameMode forKey:@"validation.resume.mode"];}
#endif
        auto saved=_save;
        if(_pendingSaveSnapshot)saved.slots.at(_slotIndex)=*_pendingSaveSnapshot;
        const auto bytes=saved.encode();
        NSError *error=nil;
        if (![[NSData dataWithBytes:bytes.data() length:bytes.size()] writeToURL:_saveURL options:NSDataWritingAtomic error:&error])
            NSLog(@"HeavyMach save failed: %@",error.localizedDescription);
    } catch (const std::exception& error) {NSLog(@"HeavyMach save rejected: %s",error.what());}
}
- (void)startSlot:(int)slot mode:(int)mode mission:(int)mission stage:(int)stage {
    try {
        if(_session){_retainedWorldTime=_session->world().gameTime;_retainedWorldCamera=_session->world().camera;}
        _slotIndex=slot;_originalSelectedSlot=&_save.slots.at(slot);_hasKeyboardAim=NO; _ammoPreview=-1; _gameMenu=NO; _hpGauge=1;_expGauge=0;_reloadGauge=0;
        // Original GameMain keeps GuiGame across screen and slot changes.
        // Its HP needle, gear and closing animations continue while hidden.
        HMMedia *media=_media;
        // Prepare every radar/body frame before combat; animation must not decode PNGs.
        for(const auto& art:heavymach::groundAAImages())@autoreleasepool{
            [media imageSize:[NSString stringWithUTF8String:art.name.c_str()] scope:@"shared"];
        }
        __weak HMGameView *randomOwner=self;
        _session=std::make_unique<heavymach::GameSession>(_data,_save.slots.at(slot),
            [media](std::string_view name) {
                NSString *key=[[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];
                CGSize size=[media imageSize:key scope:@"shared"];
                return heavymach::Vector2{float(size.width),float(size.height)};
            }, [randomOwner]{return [randomOwner nextOriginalRandom];}, &_passives,
            [media](std::string_view name){NSString* key=[[NSString alloc] initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];CGSize size=[media imageSize:key scope:@"world"];return heavymach::Vector2{float(size.width),float(size.height)};});
        // openMap must know the target extent before preparing a new world.
        _showMenu=NO;[self updateCanvasGeometry];
        _session->restoreWorldCamera(_retainedWorldCamera);
#if defined(HM_VALIDATION) && HM_VALIDATION
        if(_validationFixture){auto fixture=*_validationFixture;_validationFixture.reset();_session->startValidation(fixture);}
        else _session->start(mode,mission,stage,_retainedWorldTime);
#else
        _session->start(mode,mission,stage,_retainedWorldTime);
#endif
        _session->setAimMode(_preferredAimMode);
        _showMenu=NO;
        [_hud setGameStart:_time]; [self consumeGameEvents];
        // GameMain.setScreen_Game0x3a1c unpauses after setMission/setSurvival,
        // even when openMap's addExp(0) opens an upgrade from leftover EXP.
        _session->setPaused(false);[self updateCanvasGeometry];
        [self noteInteraction];if(_resumeRequired){[self pauseForInterruption];_session->prepareInitialPausedFrame();}
#if TARGET_OS_OSX
        [self.window makeFirstResponder:self];
#else
        [self becomeFirstResponder];
#endif
    } catch (const std::exception& error) {_loadError=[NSString stringWithUTF8String:error.what()];}
}
- (void)requestScreen:(int)screen {
    // Original GameMain accepts repeated requests even while shutters move.
    _loading.changeScreen(screen);
}
- (void)prepareScreenResources:(int)screen {
    _screenResourceScope=[NSString stringWithFormat:@"screen%lu",(unsigned long)++_screenResourceGeneration];
    NSMutableArray<NSString*> *names=[NSMutableArray new];
    if(screen==1)[names addObjectsFromArray:@[@"Main_BG",@"Main_Episode",@"Main_Title",@"Main_Copyright",@"Main_TapToStart"]];
    if(screen==2)[names addObjectsFromArray:@[@"SelMode_BG",@"Common_Btn_OK",@"SelMode_Btn_Scenario",@"SelMode_Btn_Survival"]];
    if(screen==3){
        [names addObjectsFromArray:@[@"SelSlot_BG",@"Common_Btn_OK",@"Common_Btn_DEL",@"SelSlot_Sel",@"SelSlot_DelSlot",@"Common_Btn_YES",@"Common_Btn_NO",@"NumSet01",@"SelSlot_Btn"]];
        BOOL occupied=NO,empty=NO;for(const auto& slot:_save.slots){occupied|=slot.occupied();empty|=!slot.occupied();}
        if(occupied)[names addObject:@"SelSlot_ExpGauge"];if(empty)[names addObject:@"SelSlot_Empty"];
    }
    if(screen==4){
        [names addObjectsFromArray:@[@"SelMission_BG",@"Common_Btn_OK",@"SelMission_Btn_Up",@"SelMission_Btn_Dn",@"SelMission_Slot",@"SelMission_Text",@"SelMission_Btn_Left",@"SelMission_Btn_Right"]];
        for(int i=1;i<=7;++i)[names addObject:[NSString stringWithFormat:@"SelMission_TileMission%02d",i]];
        for(int i=1;i<=6;++i)[names addObject:[NSString stringWithFormat:@"SelMission_Stage%02d",i]];
    }
    for(NSString *name in names)@autoreleasepool{[_media imageSize:[name stringByAppendingPathExtension:@"png"] scope:_screenResourceScope];}
    if(screen==1)[_media imageSize:@"Extensions/TitleWide.png" scope:_screenResourceScope];
    if(screen==2)[_media imageSize:@"Extensions/ModeWide.png" scope:_screenResourceScope];
    if(screen==3)[_media imageSize:@"Extensions/MenuMetal.png" scope:_screenResourceScope];
    if(screen==4)[_media imageSize:@"Extensions/MissionWide.png" scope:_screenResourceScope];
}
- (void)refreshScreen:(int)screen {
    _transferScreen=screen;
#if defined(HM_VALIDATION) && HM_VALIDATION
    [self stopValidationDirection];
#endif
    // Called synchronously at loading state3, before this frame's child ticks.
    heavymach::StagePosition stage{};BOOL fromGame=bool(_session);
    if(_session){_originalWeaponSlot=&_save.slots.at(_slotIndex);_originalSelectedWeapon=_session->selectedWeapon();_originalWeaponIndex=_originalSelectedWeapon;stage=_session->stage();_retainedWorldTime=_session->world().gameTime;_retainedWorldCamera=_session->world().camera;[_menus setPreviewWorldTime:_retainedWorldTime paused:_session->world().paused];}
    _session.reset();if(screen!=6)_resumeRequired=NO;[_media releaseScope:@"world"];
    NSString* oldScope=_screenResourceScope;[self prepareScreenResources:screen];
    switch(screen){
        case 1:_showMenu=NO;_titleShake={};_title=heavymach::TitleState{};_title.startTime=float(_time);break;
        case 2:_showMenu=YES;[_menus showMode];break;
        case 3:_showMenu=YES;[_menus showSlots];break;
        case 4:_showMenu=YES;
            if(fromGame)[_menus showMissions:stage.mission stage:stage.stage];else [_menus showMissions];
            break;
        case 6:[_menus startSelectedGame];break;
        default:NSLog(@"Unrestored screen ID %d",screen);break;
    }
    if(oldScope)[_media releaseScope:oldScope];
}
- (void)drawLoading {
    if(!_loading.visible())return;
    _media.presentationOffset=CGPointZero;
    // Door is a canvas overlay, independent of HUD safe-area scale and clip.
    [_media resetClip];[_media setDrawingOffset:CGPointZero];[_media setDrawingScale:1];[_media setCameraOffset:CGPointZero];
    if(_canvasWidth>480||_canvasHeight>320){
        for(const auto& panel:_loading.expandedPanels(_canvasWidth,_canvasHeight))
            [_media drawExpandedPanel:[NSString stringWithUTF8String:panel.sprite.tile.c_str()] x:panel.sprite.position.x y:panel.sprite.position.y left:panel.left right:panel.right top:panel.top bottom:panel.bottom];
    }else for(const auto& sprite:_loading.sprites())
        [_media draw:[NSString stringWithUTF8String:sprite.tile.c_str()] x:sprite.position.x y:sprite.position.y columns:sprite.cols rows:sprite.rows tile:sprite.frame scaleX:sprite.scale.x scaleY:sprite.scale.y rotation:sprite.rotation alpha:sprite.alpha blend:sprite.blend];
}
- (void)finishUpgrade {
    if(!_session)return;
    _session->finishUpgradeSlide();
    if(_resumeRequired)[self pauseForInterruption];
}
- (double)interactionTime {return CACurrentMediaTime();}
- (void)noteInteraction {_lastInteraction=[self interactionTime];}
- (void)cancelGameInput {
    _gameButtons.cancel();
#if defined(HM_VALIDATION) && HM_VALIDATION
    [self stopValidationDirection];
#endif
    [_aimTouches removeAllObjects];
    _leftPressed=_rightPressed=_upPressed=_downPressed=_keyboardFire=NO;_hasKeyboardAim=NO;_keyboardAimVisible=NO;
    [_heldTouches removeAllObjects];[_worldTouches removeAllObjects];[_hudTouches removeAllObjects];[_hudCapturedReceivers removeAllObjects];[_hud cancelInput];
    if(_session)_session->releaseFire();
    else if(_showMenu)[_menus touch:CGPointMake(-1000,-1000) state:3];
    _tilt={};_keyboardControls.cancel();
}
- (void)pauseForInterruption {
    [self cancelGameInput];
    if(!_session)return;
    _resumeRequired=YES;
    const auto* player=_session->player();
    // Keep result/upgrade panels; never cover them with an ordinary pause menu.
    const auto& world=_session->world();
    if((world.gameState==0 && (!player||player->upgradeStep==0)) ||
       (world.gameState!=0 && world.presentation.step>=3)){
        _gameMenu=YES;[_hud setMenuVisible:YES];
    }
    _session->setPaused(true);
}
- (void)checkInactivity {
    if(!_active||!_session||_session->world().paused||_session->world().gameState!=0||_loading.visible())return;
    // Foreground inactivity is elapsed real time, not a catch-up game tick.
    if(_heldTouches.count||_keyboardFire||_leftPressed||_rightPressed||_upPressed||_downPressed||std::fabs(_tilt.y)>.03f){[self noteInteraction];return;}
    if([self interactionTime]-_lastInteraction>=120.0)[self pauseForInterruption];
}
- (void)gameButton:(int)identifier state:(int)state {
    if (!_active || !_session) return;
    [self noteInteraction];
    const auto& world=_session->world();
    if (world.presentation.step==2 && (identifier==1000 || identifier==1001)) {
        if (state==2) {_resumeRequired=NO;_session->resultButton(identifier);}return;
    }
    if (identifier==1000 && state==0) {
        if(!world.paused){_session->booster();[_media sound:@"UI_Click"];}
        return;
    }
    if (state!=2) return;
    // GuiGame.handleButton 0x17c4c keeps buttons active and gates actions here.
    if(world.paused && (identifier==100 || identifier==1100))return;
    if(!world.paused && ((identifier>=101&&identifier<=104) ||
                         (identifier>=1200&&identifier<=1202) || identifier==1300))return;
    switch (identifier) {
        case 100:
#if defined(HM_VALIDATION) && HM_VALIDATION
            [self stopValidationDirection];
#endif
            [self cancelGameInput];_gameMenu=YES;[_hud setMenuVisible:YES];_session->setPaused(true);[_media sound:@"UI_Click"];break;
        case 101: _resumeRequired=NO;_tilt={};[self noteInteraction];_gameMenu=NO;[_hud setMenuVisible:NO];_session->setPaused(world.gameState!=0);[_media sound:@"UI_Click"];break;
        case 102: _resumeRequired=NO;_session->setPaused(false);_gameMenu=NO;[_hud setMenuVisible:NO];[_media sound:@"UI_Click"];[self persistSave];[self requestScreen:3];break;
        case 103:
            if(world.paused){
                // 0x17c4c: original CFString bytes at file offset0x24964.
                [_media sound:@"UI_Click"];
#if TARGET_OS_OSX
                [NSWorkspace.sharedWorkspace openURL:[NSURL URLWithString:@"http://www.indiean.com"]];
#else
                [UIApplication.sharedApplication openURL:[NSURL URLWithString:@"http://www.indiean.com"] options:@{} completionHandler:^(BOOL success){
                    if(!success)NSLog(@"Original website URL could not be opened");
                }];
#endif
            }break;
        case 104: if(_gameMenu&&world.gameState==0){[self openHUDEditor:nil];[_media sound:@"UI_Click"];}break;
        case 1100: _session->cycleWeapon();[_media sound:@"Game_ChangeWeapon"];break; // 0x17c4c: release, then sfxID4.
        case 1200:case 1201:case 1202: _session->selectUpgrade(identifier-1200);break;
        case 1300: _resumeRequired=NO;_session->confirmUpgrade();break;
    }
    [self consumeGameEvents];
}
- (void)consumeGameEvents {
    if (!_session) return;
    using namespace heavymach;
    static NSString *const sounds[]={@"UI_Click",@"UI_Btn_OK",@"UI_Btn_Cancel",@"Game_GetItem",@"Game_ChangeWeapon",@"Game_WeaponEnd",@"Game_LevelUp",@"Game_LowHP",@"SFX_HitBody01",@"SFX_HitBody02",@"SFX_HitBody03",@"SFX_HitBody04",@"SFX_GroundHit",@"SFX_ExplGround",@"SFX_ExplMach"};
    _originalWeaponSlot=&_save.slots.at(_slotIndex);_originalSelectedWeapon=_session->selectedWeapon();_originalWeaponIndex=_originalSelectedWeapon;
    const auto events=_session->takeEvents();
    for (const auto& e:events) {
        if (e.kind==SessionEventKind::AudioIndex && e.index>=0 && e.index<15) [_media sound:sounds[e.index] pan:e.pan];
        else if (e.kind==SessionEventKind::AudioName) [_media sound:[NSString stringWithUTF8String:e.text.c_str()] pan:e.pan];
        else if (e.kind==SessionEventKind::PersistSave) {
            _pendingSaveSnapshot=e.savedSlot;[self persistSave];_pendingSaveSnapshot.reset();
        }
        else if (e.kind==SessionEventKind::CombatUI) {
            switch (CombatEventKind(e.code)) {
                case CombatEventKind::ReloadGauge:_reloadGauge=e.value;[_hud setReloadGauge:e.value];break;
                case CombatEventKind::ReloadComplete:_ammoPreview=-1;[_hud setReloadComplete:_time];break;
                case CombatEventKind::ChargePreview:_ammoPreview=int(e.value);break;
                default:break;
            }
        } else if (e.kind==SessionEventKind::PlayerUI) {
            switch (PlayerEventKind(e.code)) {
                case PlayerEventKind::Quake:_worldShake.setQuake(e.value,e.amount,e.delay,_time);break;
                case PlayerEventKind::HPGauge:_hpGauge=e.value;[_hud setHPGauge:e.value];break;
                case PlayerEventKind::UpgradeOpen:[_hud openUpgrade:_save.slots.at(_slotIndex)];break;
                case PlayerEventKind::UpgradeClose:[_hud closeUpgrade];break;
                case PlayerEventKind::ExperienceGauge:_expGauge=e.value;break;
                case PlayerEventKind::WeaponInventory:_ammoPreview=-1;[_hud showWeaponChange:_time];break;
                default:break;
            }
        } else if (e.kind==SessionEventKind::WorldUI) {
            switch (WorldEventKind(e.code)) {
                case WorldEventKind::Quake:_worldShake.setQuake(10,4,.3f,_time);break;
                case WorldEventKind::GameStart:[_hud setGameStart:_time];break;
                case WorldEventKind::ClearTileResources:[_media releaseScope:@"world"];break;
                case WorldEventKind::ChangeScreen:
                    if (_session) [self requestScreen:e.index];break;
                default:break;
            }
        } else if (e.kind==SessionEventKind::Unsupported) NSLog(@"Unrestored behavior: %s",e.text.c_str());
    }
}
- (void)keyboardDisconnected:(NSNotification*)notification {[self cancelGameInput];}
- (NSString*)keyName:(unsigned)code {
    if(code>=4&&code<=29)return [NSString stringWithFormat:@"%c",'A'+code-4];
    if(code>=30&&code<=38)return [NSString stringWithFormat:@"%u",code-29];
    switch(code){case 39:return @"0";case 0x28:return @"Enter";case 0x29:return @"Esc";case 0x2b:return @"Tab";case 0x2c:return @"Space";case 0x4f:return @"RIGHT";case 0x50:return @"LEFT";case 0x51:return @"DOWN";case 0x52:return @"UP";case 0xe1:return @"Shift";case 0xe5:return @"Right Shift";case 0x3b:return @"F2";default:return [NSString stringWithFormat:@"HID %02X",code];}
}
- (void)refreshKeyboardHelp {}
- (void)saveKeyboardBindings {
    NSMutableArray *values=[NSMutableArray new];for(auto key:_keyboardControls.bindings)[values addObject:@(key)];
    [NSUserDefaults.standardUserDefaults setObject:values forKey:@"KeyboardBindingsV1"];
}
- (void)toggleKeyboardSettings {
    if(_loading.state()<6)return;
    if(!_keyboardSettingsOpen){_settingsWasPaused=_session&&_session->world().paused;[self cancelGameInput];if(_session)_session->setPaused(true);}
    else {if(_session)_session->setPaused(_settingsWasPaused||_resumeRequired);[self cancelGameInput];}
    _keyboardSettingsOpen=!_keyboardSettingsOpen;_capturingKey=NO;[self refreshKeyboardHelp];
}
- (void)ensureKeyboardAim {
    _keyboardAimVisible=YES;
    if(_hasKeyboardAim||!_session)return;
    auto player=_session->player();if(!player)return;auto point=heavymach::Vector2{player->position.x-_session->world().camera.x,player->position.y-_session->world().camera.y};point.x+=_canvasWidth/2;point.y+=160;_keyboardAim=CGPointMake(std::min(float(_canvasWidth-8),point.x+100),std::clamp(point.y+20,8.f,312.f));_hasKeyboardAim=YES;
}
- (BOOL)handleKeyCode:(unsigned)code pressed:(BOOL)pressed {
    using A=heavymach::KeyAction;
    if(!_active)return NO;
#if TARGET_OS_OSX
    if(self.window.attachedSheet){[self cancelGameInput];return NO;}
#else
    if(self.window.rootViewController.presentedViewController){[self cancelGameInput];return NO;}
#endif
    if(_transferPage){
#if TARGET_OS_OSX
        if(self.window&&self.window.firstResponder!=self)return NO;
#else
        if(_transferInput.isFirstResponder)return NO;
#endif
        if(code==0x29&&pressed){[self closeTransfer];return YES;}
        [self prepareGameButtons];if(auto e=_gameButtons.key(code,pressed))[self gameUIAction:e->id state:e->state];return YES;
    }
#if HM_VALIDATION
    if(_validationPanelOpen){if(code==0x29&&pressed){[self closeValidation];return YES;}[self prepareGameButtons];if(auto e=_gameButtons.key(code,pressed))[self gameUIAction:e->id state:e->state];return YES;}
#endif
    if(_hudEditing){
        [self prepareGameButtons];
        if(code==0x3f&&pressed){_gameToolsFocus=!_gameToolsFocus;_gameButtons.cancel();return YES;}
        if(_gameToolsFocus){if(code==0x29&&pressed){_gameToolsFocus=NO;return YES;}if(auto e=_gameButtons.key(code,pressed))[self gameUIAction:e->id state:e->state];return YES;}
        return [self hudEditorKey:code pressed:pressed];
    }
    if(code==0x3c&&pressed&&[self transferAvailable]){[self openTransfer:nil];return YES;} // F3 on the slot screen.

    if(code<_aimBlockedKeys.size()&&_aimBlockedKeys[code]){if(!pressed)_aimBlockedKeys[code]=false;return YES;}
    if(_keyboardSettingsOpen){
        if(!_keyboardControls.set(code,pressed)||!pressed)return YES;
        if(_capturingKey){if(code!=0x29&&_keyboardControls.remap(A(_settingsSelection),code))[self saveKeyboardBindings];_capturingKey=NO;}
        else if(code==0x29){[self toggleKeyboardSettings];return YES;}
        else if(code==0x52)_settingsSelection=(_settingsSelection+heavymach::KeyboardInput::count-1)%heavymach::KeyboardInput::count;
        else if(code==0x51||code==0x2b)_settingsSelection=(_settingsSelection+1)%heavymach::KeyboardInput::count;
        else if(code==0x28)_capturingKey=YES;
        else if(code==0x2a){_keyboardControls.bindings=heavymach::KeyboardInput::defaults;[self saveKeyboardBindings];}
        [self refreshKeyboardHelp];return YES;
    }
    if(!_keyboardControls.handles(code))return NO;
    BOOL changed=_keyboardControls.set(code,pressed);if(!changed)return YES;
    [self noteInteraction];
    _leftPressed=_keyboardControls.down(A::left);_rightPressed=_keyboardControls.down(A::right);
    if(_keyboardControls.is(code,A::left)||_keyboardControls.is(code,A::right)){
        if(!_keyboardControls.moving()){_keyboardControls.virtualTilt=0;_tilt={};}
    }
    if(pressed&&_keyboardControls.is(code,A::settings)){[self toggleKeyboardSettings];return YES;}
    if(_loading.visible())return YES;
    if(!pressed&&_keyboardControls.is(code,A::fire)){
        _keyboardFire=NO;
        if(_session&&_session->aimMode()==heavymach::AimMode::automatic){[self refreshAutoFire];return YES;}
        if(_session&&!_worldTouches.count)_session->releaseFire();return YES;
    }
    if(pressed&&_keyboardControls.is(code,A::back)){
        if(_session){if(_gameMenu)[self gameButton:101 state:2];else if(![self worldInputBlocked])[self gameButton:100 state:2];}
        else if(_showMenu)[self requestScreen:3];return YES;
    }
    if(_showMenu||(_session&&[self worldInputBlocked])){
        if(!pressed)return YES;
        int delta=0;
        if(_keyboardControls.is(code,A::focus))delta=_keyboardControls.fine()?-1:1;
        else if(_keyboardControls.is(code,A::aimLeft)||_keyboardControls.is(code,A::aimUp))delta=-1;
        else if(_keyboardControls.is(code,A::aimRight)||_keyboardControls.is(code,A::aimDown))delta=1;
        if(delta){if(_showMenu)[_menus moveKeyboardFocus:delta];else [_hud moveKeyboardFocus:delta];}
        if(_keyboardControls.is(code,A::confirm)){if(_showMenu)[_menus activateKeyboardFocus];else [_hud activateKeyboardFocus];}
        return YES;
    }
    if(_session){
        if(pressed&&_keyboardControls.is(code,A::fire)){
            _keyboardFire=YES;
            if(_session->aimMode()==heavymach::AimMode::manual){[self ensureKeyboardAim];_session->touch({float(_keyboardAim.x),float(_keyboardAim.y)},0);}
            else [self refreshAutoFire];
        }
        if(pressed&&_keyboardControls.is(code,A::nextTarget))_session->cycleAimTarget();
        if(pressed&&_keyboardControls.is(code,A::jump))[self gameButton:1000 state:0];
        if(pressed&&(_keyboardControls.is(code,A::nextWeapon)||_keyboardControls.is(code,A::previousWeapon))){
            _session->cycleWeapon(_keyboardControls.is(code,A::previousWeapon)?-1:1);[_media sound:@"Game_ChangeWeapon"];
        }
        [self consumeGameEvents];
    }else if(pressed&&_keyboardControls.is(code,A::confirm)&&_title.released(_time))[self requestScreen:3];
    return YES;
}
- (void)updateKeyboardAim {
    if(!_session||[self worldInputBlocked]||_keyboardSettingsOpen||_session->aimMode()!=heavymach::AimMode::manual)return;
    if(_keyboardControls.aiming()){
        [self ensureKeyboardAim];float x=_keyboardAim.x,y=_keyboardAim.y;_keyboardControls.aim(x,y,_canvasWidth,_canvasHeight);_keyboardAim=CGPointMake(x,y);[self noteInteraction];
        if(_keyboardFire)_session->touch({x,y},1);
    }
}
- (void)drawKeyboardIndicators {
    if(_loading.visible()||_hudEditing||_keyboardSettingsOpen||_transferPage)return;
#if HM_VALIDATION
    if(_validationPanelOpen)return;
#endif
    CGRect rect=CGRectZero;
    if(_showMenu)rect=CGRectOffset([_menus keyboardFocusRect],0,(_canvasHeight-320)/2);
    else if(_session&&[self worldInputBlocked]){rect=[_hud keyboardFocusRect];rect=CGRectMake(rect.origin.x*_hudScale+_hudOffset.x,rect.origin.y*_hudScale+_hudOffset.y,rect.size.width*_hudScale,rect.size.height*_hudScale);}
    else if(_hasKeyboardAim&&_keyboardAimVisible&&_session&&_session->aimMode()==heavymach::AimMode::manual)rect=CGRectMake(_keyboardAim.x-4,_keyboardAim.y-4,8,8);
    [_glRenderer resetClip];[_glRenderer drawInputOutline:rect];
}
#if TARGET_OS_OSX
- (BOOL)acceptsFirstResponder {return YES;}
- (void)viewDidMoveToWindow {[super viewDidMoveToWindow];if(self.window)[self.window makeFirstResponder:self];}
- (unsigned)HIDCode:(NSEvent*)event {
    static const unsigned codes[128]={
        4,22,7,9,11,10,29,27,6,25,0,5,20,26,8,21,28,23,30,31,32,33,35,34,46,38,36,45,37,39,48,18,
        24,47,12,19,40,15,13,52,14,51,49,54,56,17,16,55,43,44,53,42,0,41,231,227,225,57,226,224,229,230,228,0,
        108,99,0,85,0,87,0,83,0,0,0,84,88,0,86,109,110,103,98,89,90,91,92,93,94,95,111,96,97,0,0,0,
        62,63,64,60,65,66,0,68,0,104,107,105,0,67,0,69,0,106,73,74,75,76,61,77,59,78,58,80,79,81,82,0};
    return event.keyCode<128?codes[event.keyCode]:0;
}
- (void)keyDown:(NSEvent*)event {if(event.modifierFlags&NSEventModifierFlagCommand){[self cancelGameInput];[super keyDown:event];return;}unsigned code=[self HIDCode:event];if(!code||![self handleKeyCode:code pressed:YES])[super keyDown:event];}
- (void)keyUp:(NSEvent*)event {unsigned code=[self HIDCode:event];if(!code||![self handleKeyCode:code pressed:NO])[super keyUp:event];}
- (void)flagsChanged:(NSEvent*)event {unsigned code=[self HIDCode:event];NSEventModifierFlags flag=code==0xe1||code==0xe5?NSEventModifierFlagShift:code==0xe0||code==0xe4?NSEventModifierFlagControl:NSEventModifierFlagOption;[self handleKeyCode:code pressed:(event.modifierFlags&flag)!=0];}
- (BOOL)resignFirstResponder {[self cancelGameInput];return [super resignFirstResponder];}
#else
- (BOOL)canBecomeFirstResponder {return YES;}
- (void)didMoveToWindow {[super didMoveToWindow];if(self.window)[self becomeFirstResponder];}
- (void)pressesBegan:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event {
    NSMutableSet *unhandled=[NSMutableSet new];for(UIPress* press in presses){if(!press.key||(press.key.modifierFlags&UIKeyModifierCommand)||![self handleKeyCode:unsigned(press.key.keyCode) pressed:YES])[unhandled addObject:press];}if(unhandled.count)[super pressesBegan:unhandled withEvent:event];
}
- (void)pressesEnded:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event {
    NSMutableSet *unhandled=[NSMutableSet new];for(UIPress* press in presses)if(!press.key||![self handleKeyCode:unsigned(press.key.keyCode) pressed:NO])[unhandled addObject:press];if(unhandled.count)[super pressesEnded:unhandled withEvent:event];
}
- (void)pressesCancelled:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event {[self cancelGameInput];[super pressesCancelled:presses withEvent:event];}
#endif
- (void)scheduleOriginalMotionTimer:(double)due {
    _nextMotionDue=due;
    __weak HMGameView *owner=self;
    _motionTimer=[NSTimer timerWithTimeInterval:heavymach::TiltState::interval repeats:NO block:^(NSTimer *timer) {
        HMGameView *view=owner;if(!view||!view->_active)return;
        constexpr double interval=heavymach::TiltState::interval;
        double now=CACurrentMediaTime();
        double next=view->_nextMotionDue+interval*std::ceil(std::max((now-view->_nextMotionDue)/interval,1.0));
        [view sampleOriginalAcceleration];
        if(view->_active)[view scheduleOriginalMotionTimer:next];
    }];
    _motionTimer.fireDate=[NSDate dateWithTimeIntervalSinceNow:std::max(due-CACurrentMediaTime(),0.0)];
    [NSRunLoop.mainRunLoop addTimer:_motionTimer forMode:NSDefaultRunLoopMode];
}
- (void)scheduleOriginalTimer:(double)due {
    _nextTimerDue=due;
    __weak HMGameView *owner=self;
    _frameTimer=[NSTimer timerWithTimeInterval:0.01666666753590107 repeats:NO block:^(NSTimer *timer) {
        HMGameView *view=owner;if(!view||!view->_active)return;
        // ns_timer.rs handle_timer advances BEFORE the callback, with ceil
        // (not the native repeating timer's skipped-fire rescheduling).
        constexpr double interval=0.01666666753590107;
        double now=CACurrentMediaTime();
        double next=view->_nextTimerDue+interval*std::ceil(std::max((now-view->_nextTimerDue)/interval,1.0));
        [view frame:timer];
        if(view->_active)[view scheduleOriginalTimer:next];
    }];
    _frameTimer.fireDate=[NSDate dateWithTimeIntervalSinceNow:std::max(due-CACurrentMediaTime(),0.0)];
    [NSRunLoop.mainRunLoop addTimer:_frameTimer forMode:NSDefaultRunLoopMode];
}
- (int32_t)nextOriginalRandom {
    // touchHLE libc/stdlib.rs random(): shared xorshift32, not Darwin random().
    uint32_t state=std::max(_randomState,uint32_t(1));
    state^=state<<13;state^=state>>17;state^=state<<5;
    _randomState=state;return int32_t(state&0x7fffffff);
}
#if !TARGET_OS_OSX
- (UIInterfaceOrientation)accelerationInterfaceOrientation {
    return self.window.windowScene.interfaceOrientation;
}
#endif
- (void)sampleOriginalAcceleration {
    if(!_active)return;
    if(_keyboardControls.moving()){
        float y=_keyboardControls.sample();
        // Digital release has no physical sensor settling; stop immediately.
        if(!_keyboardControls.moving())_tilt={};else _tilt.sample(0,y,-std::sqrt(1-y*y));
        return;
    }
#if TARGET_OS_OSX
    float x=0,y=0,z=-1;
#else
    CMAccelerometerData *sample=_motion.accelerometerData;
    CMAcceleration acceleration=sample?sample.acceleration:CMAcceleration{0,0,-1};
    // SDL CoreMotion stores m/s^2 in float, then touchHLE converts back to g.
    constexpr float gravity=9.80665f;
    float x=-float(-acceleration.x*gravity)/gravity;
    float y=-float(-acceleration.y*gravity)/gravity;
    float z=-float(-acceleration.z*gravity)/gravity;
    // CoreMotion axes stay attached to the portrait device body. The guest's
    // fixed landscape-left UI uses +Y toward screen right; our UI also rotates
    // to landscape-right, where those body X/Y axes point the opposite way.
    // Normalize real samples before filtering. Screen-relative manual inputs
    // below already use the guest axes and must not be flipped again.
    if([self accelerationInterfaceOrientation]==UIInterfaceOrientationLandscapeRight){x=-x;y=-y;}
#endif
#if defined(HM_VALIDATION) && HM_VALIDATION
    // Same path on physical iPhone and Simulator; feed the original filter.
    if(_validationControls.selectedSegmentIndex!=3 || _leftPressed||_rightPressed||_upPressed||_downPressed){
        x=float(int(_downPressed)-int(_upPressed))*.2f;
        y=(_leftPressed||_rightPressed)?float(int(_rightPressed)-int(_leftPressed))*.2f:
          (_validationControls.selectedSegmentIndex==3?0.f:float(_validationControls.selectedSegmentIndex-1)*.2f);
        z=-std::sqrt(1.f-x*x-y*y);
    }
#if !TARGET_OS_OSX
    else if(!_motion.accelerometerAvailable){x=0;y=0;z=-1;}
#endif
#elif TARGET_OS_SIMULATOR
    if(_leftPressed||_rightPressed||_upPressed||_downPressed){
        x=float(int(_downPressed)-int(_upPressed))*.2f;
        y=float(int(_rightPressed)-int(_leftPressed))*.2f;
        z=-std::sqrt(1.f-x*x-y*y);
    }
#if !TARGET_OS_OSX
    else if(!_motion.accelerometerAvailable){x=0;y=0;z=-1;}
#endif
#endif
    _tilt.sample(x,y,z);
}
- (void)dealloc {[NSNotificationCenter.defaultCenter removeObserver:self];[_frameTimer invalidate];[_motionTimer invalidate];
#if !TARGET_OS_OSX
    [_motion stopAccelerometerUpdates];
#endif
}
- (void)setApplicationActive:(BOOL)active {
    // Resign-active happens before background. The duplicate background call
    // must not issue GL commands after the app has already become inactive.
    if(_active==active)return;
    if(!active){
        if(_loading.visible())_resumeRequired=YES;
        [self pauseForInterruption];
        [_frameTimer invalidate];_frameTimer=nil;[_motionTimer invalidate];_motionTimer=nil;

#if !TARGET_OS_OSX
    [_motion stopAccelerometerUpdates];
#endif
[_media.audio setApplicationActive:NO];
        [_glRenderer finish];_active=NO;
    }else{
        _active=YES;_tilt={};[self noteInteraction];[_media.audio setApplicationActive:YES];
#if !TARGET_OS_OSX
        if(_motion.accelerometerAvailable)[_motion startAccelerometerUpdates];
#endif
        [self scheduleOriginalTimer:CACurrentMediaTime()+0.01666666753590107];
        [self scheduleOriginalMotionTimer:CACurrentMediaTime()+heavymach::TiltState::interval];
        // Pause/menu state is intentionally retained until an explicit action.
    }
}
- (void)setWideCanvasEnabled:(BOOL)enabled {
    if(_wideCanvasEnabled==enabled)return;
    [self cancelGameInput];_wideCanvasEnabled=enabled;[self updateCanvasGeometry];
}
- (void)updateCanvasGeometry {
    CGRect bounds=self.bounds;
    const int dw=int(std::lround(bounds.size.width*self.contentScaleFactor));
    const int dh=int(std::lround(bounds.size.height*self.contentScaleFactor));
    if(!_session||!_session->world().paused)_appliedWideCanvas=_wideCanvasEnabled;
    auto canvas=heavymach::CanvasGeometry::forDrawable(dw,dh,_appliedWideCanvas);
    // A paused scene keeps its cached draw commands; apply mode changes before
    // the next simulation tick, never by collecting stateful visuals twice.

    _canvasWidth=canvas.width;_canvasHeight=canvas.height;
    _viewport=heavymach::Viewport::drawable(dw,dh,self.contentScaleFactor,_canvasWidth,_canvasHeight);
    _viewport.x+=bounds.origin.x;_viewport.y+=bounds.origin.y;
    _hudScale=1;_hudOffset=CGPointMake((_canvasWidth-480)/2,(_canvasHeight-320)/2);
    _menuSafeRect=CGRectMake(0,0,_canvasWidth,_canvasHeight);
    if((_canvasWidth>480||_canvasHeight>320) && _viewport.scale>0){
        CGRect safe=[self safeContentRect];
        const CGFloat left=std::max(CGFloat(0),(CGRectGetMinX(safe)-_viewport.x)/_viewport.scale);
        const CGFloat right=std::min(_canvasWidth,(CGRectGetMaxX(safe)-_viewport.x)/_viewport.scale);
        const CGFloat bottom=std::max(CGFloat(0),_canvasHeight-(CGRectGetMaxY(safe)-_viewport.y)/_viewport.verticalScale());
        const CGFloat top=std::min(_canvasHeight,_canvasHeight-(CGRectGetMinY(safe)-_viewport.y)/_viewport.verticalScale());
        _menuSafeRect=CGRectMake(left,bottom,right-left,top-bottom);
        _hudScale=std::max(CGFloat(.01),std::min({CGFloat(1),(right-left)/480,(top-bottom)/320}));
        _hudOffset=CGPointMake((left+right-480*_hudScale)/2,(bottom+top-320*_hudScale)/2);
    }
    [self updateHUDEditorGeometry];
    [self layoutHUDControls];
    [_menus setCanvasWidth:_canvasWidth safeRect:CGRectOffset(_menuSafeRect,0,-(_canvasHeight-320)/2)];
#if defined(HM_VALIDATION) && HM_VALIDATION
    _validationControls.hidden=!_hudEditing&&(!_session||_showMenu);
#endif
    if(!_hudEditing&&_session&&_session->canvasHeight()!=float(_canvasHeight))_session->setCanvasHeight(float(_canvasHeight));
    if(!_hudEditing&&_session&&_session->canvasWidth()!=float(_canvasWidth))_session->setCanvasWidth(float(_canvasWidth));
}
- (BOOL)worldInputBlocked {
    if(!_session)return YES;
    return _loading.visible()||_gameMenu||_session->world().paused||_session->world().gameState!=0;
}
- (CGRect)safeContentRect {
#if TARGET_OS_OSX
    return self.bounds;
#else
    return UIEdgeInsetsInsetRect(self.bounds,self.safeAreaInsets);
#endif
}
#if TARGET_OS_OSX
- (void)layout {[super layout];[self layoutGameSurface];}
- (void)layoutGameSurface {
#else
- (void)layoutSubviews {
    [super layoutSubviews];
#endif
    [self updateCanvasGeometry];
    if(_keyboardSettingsOpen)[self refreshKeyboardHelp];
#if defined(HM_VALIDATION) && HM_VALIDATION
    CGRect safe=[self safeContentRect];
    if(_viewport.x-CGRectGetMinX(safe)>=68)
        _validationControls.frame=CGRectMake(_viewport.x-68,CGRectGetMidY(safe)-94,60,188);
    else _validationControls.frame=CGRectMake(CGRectGetMidX(safe)-104,CGRectGetMinY(safe)+8,208,44);
#endif
    [self layoutHUDControls];
    // Original EAGLView::layoutSubviews (0x27dc) only resizes its surface.
    // Drawing here would advance stateful menu weapon visuals outside the timer.
}
// Original IPA renderScene @0x21e0 increments this exact double once, then
// ticks and draws once. A wall-time accumulator can alternate zero/two ticks
// on adjacent display callbacks and visibly stutter even at a steady 60 Hz.
- (void)advanceOriginalFrame {
    if(_hudEditing||(_transferPage&&_loading.state()==6))return;
#if HM_VALIDATION
    if(_validationPanelOpen)return;
#endif
    _time+=0.01666666753590107; // Original constant 0x3f91111120000000.
    if(_session)_worldShake.tick(_time);
    else if(!_showMenu)_titleShake.tick(_time);
    if (_loadError||_keyboardSettingsOpen) return;
    _loading.tick([self](heavymach::LoadingEvent event){
        switch(event.kind){
            case heavymach::LoadingEventKind::CloseSound:[self->_media sound:@"Loading_Close"];break;
            case heavymach::LoadingEventKind::OpenSound:[self->_media sound:@"Loading_Open"];break;
            case heavymach::LoadingEventKind::RefreshScreen:[self refreshScreen:event.screen];break;
        }
    });
    if (_loadError) return;
    if (_showMenu) {[_menus tick:_time];_passives.tick();[_hud tick:_time];return;}
    if (_session) {
        [self updateKeyboardAim];
        // Pause menus and interrupted upgrades must also freeze passive timers.
        if(_gameMenu||(_resumeRequired&&_session->world().gameState==0)){[_hud tick:_time];return;}
        try {
            float input=_tilt.y;
            // Samples have already been mapped to the original landscape axes.
            _session->setCameraOffset(_worldShake.offset);
            _session->tick(_time,input);[self consumeGameEvents];[_hud tick:_time];
            if (_trace && _session && int(_time)!=_traceSecond) {
                _traceSecond=int(_time);const auto *p=_session->player();auto stage=_session->stage();
                NSLog(@"HM_TRACE t=%d mission=%d stage=%d x=%.2f y=%.2f hp=%d exp=%d level=%d enemies=%zu bullets=%zu gameState=%d paused=%d fireKey=%d",_traceSecond,stage.mission,stage.stage,p?p->position.x:0,p?p->position.y:0,p?p->hp:0,_save.slots[_slotIndex].experience(),_save.slots[_slotIndex].level(),_session->enemyCount(),_session->bulletCount(),_session->world().gameState,_session->world().paused,_keyboardFire);
            }
        } catch (const std::exception& error) {_loadError=[NSString stringWithUTF8String:error.what()];}
        return;
    }
    _passives.tick();[_hud tick:_time];
    for (auto event:_title.tick(_time,[self]{return uint32_t([self nextOriginalRandom]);})) {
        switch (event) {
            case heavymach::TitleEvent::titleIn: [_media sound:@"Title_TitleIn"]; break;
            case heavymach::TitleEvent::bangTitle:_titleShake.setQuake(10,4,.3f,_time);[_media sound:@"Title_Bang"];break;
            case heavymach::TitleEvent::bangEpisode:_titleShake.setQuake(6,4,.3f,_time);[_media sound:@"Title_Bang"];break;
            case heavymach::TitleEvent::tapSound: [_media sound:@"Title_TapToStart"]; break;
        }
    }
}
- (void)frame:(id)sender {
    (void)sender;
    if (!_active) return;
    [self updateCanvasGeometry];[self updateHUDEntry];if(!_hudEditing)[self checkInactivity];
    [self advanceOriginalFrame];
    [self renderOpenGLFrame];
}
// Production submits directly to CAEAGLLayer. Readback is never used to present.
- (void)renderOpenGLFrame {
    if (!_active || !self.window) return;
#if TARGET_OS_OSX
    _renderError.frame=CGRectInset(self.bounds,24,24);
    [self setErrorText:_loadError ?: (_glRenderer?nil:@"OpenGL renderer unavailable")];
    if(!_glRenderer||_loadError)return;
    if(![_glRenderer setLogicalWidth:_canvasWidth height:_canvasHeight]||![_glRenderer beginFrame]){[self setErrorText:@"Game canvas allocation failed"];return;}
    _media.glRenderer=_glRenderer;
    @try {[self drawLogicalFrame];[self drawKeyboardIndicators];if(![_glRenderer presentToLayer:self viewport:CGRectMake(_viewport.x,_viewport.y,_canvasWidth*_viewport.scale,_canvasHeight*_viewport.verticalScale())])[self setErrorText:@"OpenGL presentation failed"];}
    @finally {_media.glRenderer=nil;[_glRenderer endFrame];}
#else
    _renderError.frame=CGRectInset(self.bounds,24,24);
    _renderError.text=_loadError ?: (_glRenderer?nil:@"OpenGL ES renderer unavailable");
    _renderError.hidden=(_renderError.text==nil);
    if (!_glRenderer || _loadError) return;
    if(![_glRenderer setLogicalWidth:_canvasWidth height:_canvasHeight]){_renderError.text=@"Game canvas allocation failed";_renderError.hidden=NO;return;}
    if (![_glRenderer beginFrame]) { _renderError.text=@"OpenGL ES context unavailable";_renderError.hidden=NO;return; }
    _media.glRenderer=_glRenderer;
    @try {
        [self drawLogicalFrame];[self drawKeyboardIndicators];
        if(![_glRenderer presentToLayer:(CAEAGLLayer*)self.layer viewport:CGRectMake(_viewport.x,_viewport.y,_canvasWidth*_viewport.scale,_canvasHeight*_viewport.verticalScale())]) {
            _renderError.text=@"OpenGL ES presentation failed";_renderError.hidden=NO;
        }
    } @finally { _media.glRenderer=nil;[_glRenderer endFrame]; }
#endif
}
// The CPU path is kept as a comparison oracle for existing offscreen tests.
#if !TARGET_OS_OSX
- (void)drawRect:(CGRect)rect {
    if (self.window) return; // The original timer exclusively owns onscreen drawing.
    CGContextRef ctx=HMCurrentGraphicsContext();
    CGContextSetFillColorWithColor(ctx,HMBlackColor().CGColor); CGContextFillRect(ctx,self.bounds);
    // Expanded diagnostics use the same GPU scene. CPU raster remains a 480 oracle.
    if(_canvasWidth>480||_canvasHeight>320){
        if([_glRenderer setLogicalWidth:_canvasWidth height:_canvasHeight]&&[_glRenderer beginFrame]){
            _media.glRenderer=_glRenderer;[self drawLogicalFrame];_media.glRenderer=nil;[_glRenderer endFrame];
            [[_glRenderer snapshot]drawInRect:CGRectMake(_viewport.x,_viewport.y,_canvasWidth*_viewport.scale,_canvasHeight*_viewport.verticalScale())];
        }
        return;
    }
    // Compose once at the original framebuffer resolution. Scaling individual
    // translucent sprites before blending changes their overlapping pixels.
    UIImage *frame=[_logicalRenderer imageWithActions:^(UIGraphicsImageRendererContext *rendererContext) {
        [self drawLogicalFrame];
    }];
    CGContextSaveGState(ctx);CGContextSetInterpolationQuality(ctx,kCGInterpolationNone);
    [frame drawInRect:CGRectMake(_viewport.x,_viewport.y,480*_viewport.scale,_canvasHeight*_viewport.verticalScale())];
    CGContextRestoreGState(ctx);
}
#endif
- (void)drawLogicalFrame {
    [self drawLogicalScene];[self drawGameButtons];
}
- (void)drawLogicalScene {
    _media.presentationOffset=CGPointZero;
    CGContextRef ctx=_media.glRenderer?nullptr:HMCurrentGraphicsContext();
    if(ctx){CGContextSetFillColorWithColor(ctx,HMBlackColor().CGColor);CGContextFillRect(ctx,CGRectMake(0,0,480,320));}
    [_media setCameraOffset:CGPointZero];[_media setDrawingOffset:CGPointZero];[_media setDrawingScale:1];[_media resetClip];_media.resourceScope=@"shared";
    if(ctx){CGContextSaveGState(ctx);CGContextClipToRect(ctx,CGRectMake(0,0,480,320));}
    if (_loadError) {
        if(ctx)[_loadError drawInRect:CGRectMake(24,100,432,180) withAttributes:@{NSForegroundColorAttributeName:HMWhiteColor(),NSFontAttributeName:HMSystemFont(14)}];
        if(ctx)CGContextRestoreGState(ctx); return;
    }
    if(_hudEditing){[self drawHUDEditor];if(ctx)CGContextRestoreGState(ctx);return;}
    if (_showMenu) {_media.resourceScope=_screenResourceScope;
        if(_canvasHeight>320){CGSize size=[_media imageSize:@"Extensions/MenuMetal.png"];CGFloat scale=std::max(_canvasWidth/size.width,_canvasHeight/size.height);[_media draw:@"Extensions/MenuMetal.png" x:_canvasWidth/2 y:_canvasHeight/2 columns:1 rows:1 tile:0 scaleX:scale scaleY:scale rotation:0 alpha:1];}
        _media.presentationOffset=CGPointMake(0,(_canvasHeight-320)/2);[_menus draw];_media.presentationOffset=CGPointZero;_media.resourceScope=@"shared";[self drawLoading];if(ctx)CGContextRestoreGState(ctx);return;}
    if (_session) {
        const auto& world=_session->world();
        if(world.paused&&world.hasBackground){NSString *asset=[NSString stringWithUTF8String:world.map.background.c_str()];_media.resourceScope=@"world";CGSize size=[_media imageSize:asset];CGFloat scale=std::max(_canvasWidth/std::max(CGFloat(1),size.width-1),_canvasHeight/std::max(CGFloat(1),size.height-1));[_media draw:asset x:_canvasWidth/2 y:_canvasHeight/2 columns:1 rows:1 tile:0 scaleX:scale scaleY:scale rotation:0 alpha:1];}
        [_media setCameraOffset:CGPointMake(_worldShake.offset.x,_worldShake.offset.y)];
        for (const auto& item:_session->renderItems()) {
            if (const auto *sprite=std::get_if<heavymach::DrawSprite>(&item)){
                _media.resourceScope=sprite->worldTexture?@"world":@"shared";
                [_media draw:[NSString stringWithUTF8String:sprite->tile.c_str()] x:sprite->position.x y:sprite->position.y columns:sprite->cols rows:sprite->rows tile:sprite->frame scaleX:sprite->scale.x scaleY:sprite->scale.y rotation:sprite->rotation alpha:sprite->alpha blend:sprite->blend];}
            else {_media.resourceScope=@"shared";[_media drawTrail:std::get<heavymach::TrailMesh>(item)];}
        }
        [_media setCameraOffset:CGPointZero];_media.resourceScope=@"shared";
        [_media setDrawingOffset:_hudOffset];[_media setDrawingScale:_hudScale];
        [_media setClipRect:CGRectMake(_hudOffset.x,_hudOffset.y,480*_hudScale,320*_hudScale)];
        HMGameHUDSnapshot snapshot;
        snapshot.canvasWidth=_canvasWidth;snapshot.canvasHeight=_canvasHeight;snapshot.hudOffset=_hudOffset;snapshot.hudScale=_hudScale;
        snapshot.slot=&_save.slots[_slotIndex];snapshot.data=&_data;snapshot.passives=&_passives;
        snapshot.hpGauge=_hpGauge;snapshot.expGauge=_expGauge;snapshot.reloadGauge=_reloadGauge;
        snapshot.selectedWeapon=_session->selectedWeapon();
        if(snapshot.selectedWeapon>=0 && snapshot.selectedWeapon<snapshot.slot->weaponCount()) {
            auto name=snapshot.slot->weaponName(snapshot.selectedWeapon);
            auto found=_data.weapons.find(name);if(found!=_data.weapons.end())snapshot.weapon=&found->second;
            snapshot.ammo=_ammoPreview>=0?_ammoPreview:snapshot.slot->weaponCharge(snapshot.selectedWeapon);
        }
        snapshot.presentation=&world.presentation;snapshot.paused=world.paused;snapshot.mode=world.gameMode;
        auto stage=_session->stage();snapshot.mission=stage.mission;snapshot.stage=stage.stage;
        snapshot.menuVisible=_gameMenu;snapshot.time=_time;
        if(auto player=_session->player()) {
            snapshot.upgradeVisible=player->upgradeStep==1;
            snapshot.upgradeSelection=player->upgradeSelection;
        }
        [_hud setLayout:[self currentHUDLayout] editing:NO];
        [_hud draw:snapshot];
        [self drawAimControls];
        [self drawLoading];[_media resetClip];[_media setDrawingOffset:CGPointZero];[_media setDrawingScale:1];if(ctx)CGContextRestoreGState(ctx);return;
    }
    _media.resourceScope=_screenResourceScope;
    if(_canvasHeight>320){CGSize size=[_media imageSize:@"Main_BG.png"];CGFloat scale=std::max(_canvasWidth/(size.width-1),_canvasHeight/(size.height-1));[_media draw:@"Main_BG.png" x:_canvasWidth/2 y:_canvasHeight/2 columns:1 rows:1 tile:0 scaleX:scale scaleY:scale rotation:0 alpha:1];}
    _media.presentationOffset=CGPointMake(0,(_canvasHeight-320)/2);[_media setDrawingOffset:CGPointZero];
    if(_canvasWidth>480) {
        CGSize size=[_media imageSize:@"Extensions/TitleWide.png"];
        // Draw only the two OUTER quarters of the generated candidate. Its center
        // never enters the frame; the original owns that moving rectangle.
        [_media setCameraOffset:CGPointMake(_titleShake.offset.x,_titleShake.offset.y)];
        CGFloat extensionScale=320/size.height;
        CGFloat distance=240+size.width/8*extensionScale;
        for(int side:{-1,1}) {
            CGFloat dx=side*distance*_title.bgScale;
            [_media draw:@"Extensions/TitleWide.png" x:_canvasWidth/2+_title.camX+std::cos(_title.bgRotate)*dx y:160+_title.camY+std::sin(_title.bgRotate)*dx columns:4 rows:1 tile:side<0?0:3 scaleX:extensionScale*_title.bgScale scaleY:extensionScale*_title.bgScale rotation:_title.bgRotate alpha:1];
        }
        [_media setDrawingOffset:CGPointMake((_canvasWidth-480)/2,0)];
        [_media drawEdgeContinuation:@"Main_BG.png" x:240+_title.camX y:160+_title.camY scale:_title.bgScale rotation:_title.bgRotate width:20];
    }
    [_media setCameraOffset:CGPointMake(_titleShake.offset.x,_titleShake.offset.y)];
    if(_canvasWidth>480)[_media drawJoinedBackground:@"Main_BG.png" x:240+_title.camX y:160+_title.camY scale:_title.bgScale rotation:_title.bgRotate];
    else [_media draw:@"Main_BG.png" x:240+_title.camX y:160+_title.camY columns:1 rows:1 tile:0 scaleX:_title.bgScale scaleY:_title.bgScale rotation:_title.bgRotate alpha:1];
    if(_canvasWidth>480)[_media setDrawingOffset:CGPointMake(CGRectGetMidX(_menuSafeRect)-240,0)];
    if (_title.showTitle) [_media draw:@"Main_Title.png" x:240 y:80 columns:1 rows:1 tile:0 scaleX:_title.titleScale scaleY:_title.titleScale rotation:0 alpha:1];
    if (_title.showEpisode) [_media draw:@"Main_Episode.png" x:240 y:290 columns:1 rows:1 tile:0 scaleX:_title.episodeScale scaleY:_title.episodeScale rotation:0 alpha:1];
    [_media draw:@"Main_Copyright.png" x:240 y:_canvasWidth>480?std::max(CGFloat(12),CGRectGetMinY(_menuSafeRect)+8):12];
    if (_title.showStart) [_media draw:@"Main_TapToStart.png" x:240 y:140];
    [_media setCameraOffset:CGPointZero];
    [_media setDrawingOffset:CGPointZero];
    _media.resourceScope=@"shared";[self drawLoading];
    if(ctx)CGContextRestoreGState(ctx);
}
// One rectangle is consumed by both rendering and hit testing. Coordinates are
// absolute logical canvas points; none of these controls enter world touch().
- (CGRect)aimControlRect:(int)control {
    if(control<1||control>3)return CGRectNull; // Removed manual/correction buttons have no hit area.
    // Mount to the same 480-unit HUD rail as booster/weapon, including safe-area scale.
    int slot=control==1?0:control==3?1:2; // A, next target, fire.
    CGRect r=CGRectMake(334+slot*48,10,46,46);
    auto c=[self currentHUDLayout];auto q=c.group(heavymach::hud::aim).hitRect({r.origin.x,r.origin.y,r.size.width,r.size.height});
    return CGRectMake(q.x,q.y,q.width,q.height);
}
- (void)saveAimMode { [NSUserDefaults.standardUserDefaults setInteger:int(_preferredAimMode) forKey:@"AimModeV1"]; }
- (void)setInputAimMode:(heavymach::AimMode)mode {
    if(!_session||_session->aimMode()==mode)return;
    [self cancelAimFireInput];_session->setAimMode(mode);_preferredAimMode=mode;[self saveAimMode];
}
- (void)cancelAimFireInput {
    for(id token in _heldTouches)[_aimCancelledTouches addObject:token];
    auto keys=_keyboardControls;auto tilt=_tilt;
    unsigned fire=keys.bindings[unsigned(heavymach::KeyAction::fire)];
    _aimBlockedKeys[fire]=_aimBlockedKeys[fire]||keys.keys[fire];keys.keys[fire]=false;
    [self cancelGameInput];_keyboardControls=keys;_tilt=tilt;
    _leftPressed=keys.down(heavymach::KeyAction::left);_rightPressed=keys.down(heavymach::KeyAction::right);
}
- (void)refreshAutoFire {
    if(!_session)return;bool fire=_keyboardFire;
    for(NSNumber* action in _aimTouches.objectEnumerator)if(action.intValue==2){fire=true;break;}
    _session->setAutoFire(fire);
}
- (BOOL)routeAimPoint:(CGPoint)p state:(int)state token:(id)token {
    if(!_session)return NO;
    NSNumber* captured=[_aimTouches objectForKey:token];
    if(captured){
        if(state==1&&captured.intValue==2&&!CGRectContainsPoint([self aimControlRect:2],p)){[_aimTouches removeObjectForKey:token];[_aimCancelledTouches addObject:token];[self refreshAutoFire];return YES;}
        if(state>=2){int action=captured.intValue;[_aimTouches removeObjectForKey:token];
            if(action==2)[self refreshAutoFire];
            else if(state==2&&![self worldInputBlocked]&&CGRectContainsPoint([self aimControlRect:action],p)){
                if(action==3)_session->cycleAimTarget();
                else if(action==1)[self setInputAimMode:_session->aimMode()==heavymach::AimMode::automatic?heavymach::AimMode::manual:heavymach::AimMode::automatic];
            }
        }
        return YES;
    }
    if(state!=0||[self worldInputBlocked]||_loading.visible())return NO;
    int count=_session->aimMode()==heavymach::AimMode::automatic?4:2;
    for(int action=1;action<count;++action)if(CGRectContainsPoint([self aimControlRect:action],p)){
        [_aimTouches setObject:@(action) forKey:token];if(action==2)[self refreshAutoFire];return YES;
    }
    return NO;
}
- (void)drawAimControls {
    if((!_session&&!_hudEditing)||(!_hudEditing&&[self worldInputBlocked])||_loading.visible())return;
    [_media resetClip];[_media setDrawingOffset:CGPointZero];[_media setDrawingScale:1];
    auto mode=_hudEditing?heavymach::AimMode::automatic:_session->aimMode();int count=mode==heavymach::AimMode::automatic?4:2;
    for(int action=1;action<count;++action){auto r=[self aimControlRect:action];float x=CGRectGetMidX(r),y=CGRectGetMidY(r);
        bool selected=(action==1&&mode==heavymach::AimMode::automatic);
        for(NSNumber* held in _aimTouches.objectEnumerator)if(held.intValue==action)selected=true;
        NSString* sheet=@"Extensions/AutoAimControls.png";CGSize size=[_media imageSize:sheet];
        [_media draw:sheet x:x y:y columns:3 rows:2 tile:(selected?3:0)+action-1 scaleX:r.size.width/std::max(CGFloat(1),size.width/3) scaleY:r.size.height/std::max(CGFloat(1),size.height/2) rotation:0 alpha:1];
    }
    if(_hudEditing||mode!=heavymach::AimMode::automatic)return;auto aim=_session->aimSnapshot();if(!aim.target){_drawnAimTarget=0;return;}
    if(_drawnAimTarget!=aim.target){_drawnAimTarget=aim.target;_aimAcquiredTime=_time;}
    float gap=4+12*std::max(0.,1-(_time-_aimAcquiredTime)/.15);
    float left=aim.screenPosition.x+aim.bounds.left-gap,right=aim.screenPosition.x+aim.bounds.right+gap;
    float bottom=aim.screenPosition.y+aim.bounds.bottom-gap,top=aim.screenPosition.y+aim.bounds.top+gap;
    float red=aim.status==heavymach::AimStatus::ready?.2f:aim.status==heavymach::AimStatus::blocked?.55f:1;
    float green=aim.status==heavymach::AimStatus::ready?1:.65f,blue=aim.status==heavymach::AimStatus::blocked?.65f:.2f;
    if(right<0||left>_canvasWidth||top<0||bottom>_canvasHeight){
        float cx=(left+right)/2,cy=(bottom+top)/2;
        float x=std::clamp(cx,12.f,float(_canvasWidth-12)),y=std::clamp(cy,12.f,float(_canvasHeight-12));
        float angle=std::atan2(cy-y,cx-x),dx=std::cos(angle),dy=std::sin(angle);
        float lines[]={x-10*dx+6*dy,y-10*dy-6*dx,x,y,x,y,x-10*dx-6*dy,y-10*dy+6*dx};
        [_glRenderer drawAimLines:lines count:4 red:red green:green blue:blue];
    }else {
        float length=8;float lines[]={left,bottom+length,left,bottom,left,bottom,left+length,bottom,right-length,bottom,right,bottom,right,bottom,right,bottom+length,left,top-length,left,top,left,top,left+length,top,right-length,top,right,top,right,top,right,top-length};
        [_glRenderer drawAimLines:lines count:16 red:red green:green blue:blue];
        auto badge=heavymach::aimHUDLabel(left,right,top,float(_canvasWidth),float(_canvasHeight),aim.status,float(_time-_aimAcquiredTime));
        float leader[]={badge.anchorX,badge.anchorY,badge.elbowX,badge.elbowY,badge.elbowX,badge.elbowY,badge.endX,badge.elbowY};
        [_glRenderer drawAimLines:leader count:4 red:red green:green blue:blue];
        auto text=heavymach::aimHUDText(badge);
        // A small dark offset keeps the stencil readable over bright backgrounds.
        auto shadow=text;for(size_t i=0;i<shadow.size;i+=2){shadow.points[i]+=.7f;shadow.points[i+1]-=.7f;}
        [_glRenderer drawAimLines:shadow.points.data() count:int(shadow.size/2) red:.06f green:.08f blue:.08f];
        float flash=badge.flash?1:0;
        [_glRenderer drawAimLines:text.points.data() count:int(text.size/2) red:std::max(red,flash) green:std::max(green,flash) blue:std::max(blue,flash)];
    }
}
- (void)routePoint:(CGPoint)point state:(int)state token:(id)touch {
    if(!_active||_loadError)return;
    auto gp=_viewport.logical(point.x,point.y);[self prepareGameButtons];
    if(auto e=_gameButtons.touch((int64_t)(__bridge void*)touch,gp?gp->first:-1000,gp?gp->second:-1000,state)){[self gameUIAction:e->id state:e->state];return;}
    if(_transferPage||_keyboardSettingsOpen){_gameButtons.consumeTouch((int64_t)(__bridge void*)touch,state);return;}
#if HM_VALIDATION
    if(_validationPanelOpen){_gameButtons.consumeTouch((int64_t)(__bridge void*)touch,state);return;}
#endif
    if(_hudEditing){if(!_hudEditorTouches)_hudEditorTouches=[NSMutableSet set];if(state==0)[_hudEditorTouches addObject:touch];if(state>=2)[_hudEditorTouches removeObject:touch];auto q=_viewport.logical(point.x,point.y);[self hudEditorPoint:q?CGPointMake(q->first,q->second):CGPointMake(-1000,-1000) state:state token:touch];return;}
    _keyboardAimVisible=NO;
    if([_aimCancelledTouches containsObject:touch]){if(state>=2)[_aimCancelledTouches removeObject:touch];return;}
        auto logical=_viewport.logical(point.x,point.y);
        if(state>=2)[_heldTouches removeObject:touch];
        if(logical){[self noteInteraction];if(state==0)[_heldTouches addObject:touch];}
        if([self routeAimPoint:logical?CGPointMake(logical->first,logical->second):CGPointMake(-1000,-1000) state:state token:touch])return;
        if(_session&&(_canvasWidth>480||_canvasHeight>320||_hudLayout.customized())){
            const CGPoint p=logical?CGPointMake(logical->first,logical->second):CGPointMake(-1000,-1000);
            const CGPoint hp=CGPointMake((p.x-_hudOffset.x)/_hudScale,(p.y-_hudOffset.y)/_hudScale);
            const BOOL insideHUD=_hudLayout.customized()||(hp.x>=0&&hp.x<=480&&hp.y>=0&&hp.y<=320);
            if(state==0){
                if(insideHUD&&[_hud touch:hp state:0]){[_hudTouches addObject:touch];if(_hudLayout.customized())[_hudCapturedReceivers setObject:@([_hud receiverAt:hp]) forKey:touch];}
                else if(logical&&![self worldInputBlocked]){[_worldTouches addObject:touch];_session->touch({float(p.x),float(p.y)},0);_keyboardAim=p;_hasKeyboardAim=YES;}
            }else if([_hudTouches containsObject:touch]){
                NSNumber* receiver=[_hudCapturedReceivers objectForKey:touch];
                [_hud touch:insideHUD?hp:CGPointMake(-1000,-1000) state:state receiver:receiver?receiver.intValue:-1];
                if(state>=2){[_hudTouches removeObject:touch];[_hudCapturedReceivers removeObjectForKey:touch];}
            }else if([_worldTouches containsObject:touch]){
                if(state>=2){
                    [_worldTouches removeObject:touch];
                    if(!_worldTouches.count){
                        if(state==2&&logical)_session->touch({float(p.x),float(p.y)},2);
                        else if(_session->aimMode()==heavymach::AimMode::manual)_session->releaseFire();
                    }
                }
                else if(logical&&![self worldInputBlocked]){_session->touch({float(p.x),float(p.y)},state);_keyboardAim=p;_hasKeyboardAim=YES;}
            }
            [self consumeGameEvents];return;
        }
        if (_session) {
            const CGPoint p=logical?CGPointMake(logical->first,logical->second):CGPointMake(-1000,-1000);
            // QobManager @0xfc18 dispatches each event in reverse receiver order.
            // There is no per-touch ownership; GobWorld @0xd92c receives only
            // events that no visible HUD button consumed.
            if(![_hud touch:p state:state]) {
                if(logical){_keyboardAim=p;_hasKeyboardAim=YES;}
                if(state>=2&&(state==3||!logical)){if(_session->aimMode()==heavymach::AimMode::manual)_session->releaseFire();}
                else _session->touch({float(p.x),float(p.y)},state>=2?2:state);
            }
            [self consumeGameEvents];return;
        }
        if (!logical) {if (_showMenu && state>=2) [_menus touch:CGPointMake(-1000,-1000) state:3];return;}
        if (_showMenu) {[_menus touch:CGPointMake(logical->first,logical->second-(_canvasHeight-320)/2) state:state];return;}
        if (state==2 && _title.released(_time)) [self requestScreen:3];
}
#if !TARGET_OS_OSX
- (void)routeTouches:(NSSet<UITouch *> *)touches state:(int)state {
    for(UITouch *touch in touches)[self routePoint:[touch locationInView:self] state:state token:touch];
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {[self routeTouches:touches state:0];}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {[self routeTouches:touches state:1];}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {[self routeTouches:touches state:2];}
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {[self routeTouches:touches state:3];}
#else
- (void)mouseDown:(NSEvent*)event {[self.window makeFirstResponder:self];[self routePoint:[self convertPoint:event.locationInWindow fromView:nil] state:0 token:_mouseToken];}
- (void)mouseDragged:(NSEvent*)event {[self routePoint:[self convertPoint:event.locationInWindow fromView:nil] state:1 token:_mouseToken];}
- (void)mouseUp:(NSEvent*)event {[self routePoint:[self convertPoint:event.locationInWindow fromView:nil] state:2 token:_mouseToken];}
#endif
#include "HMHUDEditor.inc"
#include "HMTransfer.inc"
#include "HMGameButtons.inc"
@end
