#import <AppKit/AppKit.h>
#import "HMGameView.h"
@interface HMNativeAppDelegate : NSObject <NSApplicationDelegate,NSWindowDelegate>
@property(nonatomic,strong) NSWindow *window;
@property(nonatomic,strong) HMGameView *game;
@end
@implementation HMNativeAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification*)notification {
 self.window=[[NSWindow alloc]initWithContentRect:NSMakeRect(0,0,1200,800) styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];
 self.window.title=@"Heavy Mach";self.window.delegate=self;self.window.contentMinSize=NSMakeSize(640,420);self.window.collectionBehavior=NSWindowCollectionBehaviorFullScreenPrimary;
 self.game=[[HMGameView alloc]initWithFrame:self.window.contentView.bounds];self.game.autoresizingMask=NSViewWidthSizable|NSViewHeightSizable;self.window.contentView=self.game;
 [self.window center];[self.window makeKeyAndOrderFront:nil];[NSApp activateIgnoringOtherApps:YES];
 NSMenu* bar=[NSMenu new];NSMenuItem* item=[NSMenuItem new];[bar addItem:item];NSMenu* app=[NSMenu new];[app addItemWithTitle:@"Full Screen" action:@selector(toggleFullScreen:) keyEquivalent:@"f"];[app addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];item.submenu=app;
 // AppKit text editing commands resolve through the active field editor.
 // The game view does not implement these selectors.
 NSMenuItem* editItem=[[NSMenuItem alloc]initWithTitle:@"Edit" action:nil keyEquivalent:@""];[bar addItem:editItem];
 NSMenu* edit=[[NSMenu alloc]initWithTitle:@"Edit"];
 [edit addItemWithTitle:@"Cut" action:@selector(cut:) keyEquivalent:@"x"];
 [edit addItemWithTitle:@"Copy" action:@selector(copy:) keyEquivalent:@"c"];
 [edit addItemWithTitle:@"Paste" action:@selector(paste:) keyEquivalent:@"v"];
 [edit addItemWithTitle:@"Select All" action:@selector(selectAll:) keyEquivalent:@"a"];
 editItem.submenu=edit;NSApp.mainMenu=bar;
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender{return YES;}
- (void)applicationDidBecomeActive:(NSNotification*)n {[self.game setApplicationActive:YES];}
- (void)applicationWillResignActive:(NSNotification*)n {[self.game setApplicationActive:NO];}
- (void)windowDidResignKey:(NSNotification*)n {[self.game setApplicationActive:NO];}
- (void)windowDidBecomeKey:(NSNotification*)n {[self.game setApplicationActive:YES];}
@end
int main(int argc,const char**argv){@autoreleasepool {NSApplication* app=NSApplication.sharedApplication;[app setActivationPolicy:NSApplicationActivationPolicyRegular];HMNativeAppDelegate* delegate=[HMNativeAppDelegate new];app.delegate=delegate;[app run];}return 0;}
