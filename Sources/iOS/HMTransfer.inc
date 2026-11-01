- (BOOL)transferAvailable {
 return _transferScreen==3&&_showMenu&&!_session&&!_hudEditing&&!_keyboardSettingsOpen&&!_loading.visible()&&!_loadError;
}
- (heavymach::transfer::Snapshot)transferSnapshot {
 heavymach::transfer::Snapshot s;s.game=_save;s.positions=_hudLayout.positions;s.keys=_keyboardControls.bindings;s.aim=int(_preferredAimMode);return s;
}
- (heavymach::transfer::FileStore)transferStore {
 using namespace heavymach::transfer;
 auto root=std::filesystem::path(_saveURL.path.fileSystemRepresentation).parent_path();
 return FileStore(root/"transfer",[self]{
  Snapshot s;NSData* bytes=[NSData dataWithContentsOfURL:self->_saveURL];
  if(bytes)s.game=heavymach::SaveData::decode({(const uint8_t*)bytes.bytes,bytes.length});
  NSData* h=[NSData dataWithContentsOfURL:[self hudLayoutURL]];
  if(h){NSDictionary* obj=[NSJSONSerialization JSONObjectWithData:h options:0 error:nil];NSArray* a=obj[@"positions"];if(![a isKindOfClass:NSArray.class]||a.count>heavymach::hud::groupCount)throw std::invalid_argument("Invalid HUD save");for(NSUInteger i=0;i<a.count;++i)if(a[i]!=NSNull.null)s.positions[i]=heavymach::hud::Placement{[a[i][0] doubleValue],[a[i][1] doubleValue]};}
  auto d=NSUserDefaults.standardUserDefaults;NSArray* keys=[d arrayForKey:@"KeyboardBindingsV1"];if(keys){auto k=heavymach::KeyboardInput::defaults;if(keys.count!=k.size()&&keys.count+1!=k.size())throw std::invalid_argument("Invalid key save");for(NSUInteger i=0;i<keys.count;++i)k[i]=[keys[i] unsignedIntValue];if(keys.count<k.size()&&std::find(k.begin(),k.end()-1,k.back())!=k.end()-1){for(unsigned c=4;c<256;++c)if(c!=227&&c!=231&&std::find(k.begin(),k.end()-1,c)==k.end()-1){k.back()=c;break;}}s.keys=k;}
  NSNumber* aim=[d objectForKey:@"AimModeV1"];s.aim=aim?aim.intValue:1;return s;
 },[self](const Snapshot&s){
  auto b=s.game.encode();atomicFile(self->_saveURL.path.fileSystemRepresentation,{(const char*)b.data(),b.size()});
  NSMutableArray* positions=[NSMutableArray array];for(auto p:s.positions)[positions addObject:p?@[@(p->x),@(p->y)]:NSNull.null];
  NSData* h=[NSJSONSerialization dataWithJSONObject:@{@"version":@1,@"positions":positions} options:0 error:nil];if(!h)throw std::runtime_error("Could not encode HUD save");atomicFile([self hudLayoutURL].path.fileSystemRepresentation,{(const char*)h.bytes,h.length});
  NSMutableArray* keys=[NSMutableArray array];for(auto k:s.keys.value_or(heavymach::KeyboardInput::defaults))[keys addObject:@(k)];auto d=NSUserDefaults.standardUserDefaults;[d setObject:keys forKey:@"KeyboardBindingsV1"];[d setInteger:s.aim forKey:@"AimModeV1"];if(![d synchronize])throw std::runtime_error("Could not save controls");
 });
}
- (void)resumeTransfer {
 try{auto store=[self transferStore];store.recover();auto s=store.read();_save=s.game;_hudLayout.positions=s.positions;_keyboardControls.bindings=s.keys.value_or(heavymach::KeyboardInput::defaults);_preferredAimMode=s.aim==2?heavymach::AimMode::automatic:heavymach::AimMode::manual;}catch(const std::exception&e){_loadError=[NSString stringWithUTF8String:e.what()];}
}
- (NSString*)createTransferCode {
 if(![self transferAvailable])return @"ERROR: Open save slot selection first.";try{auto store=[self transferStore];return [NSString stringWithUTF8String:heavymach::transfer::encode(store.read()).c_str()];}catch(const std::exception&e){return [@"ERROR: " stringByAppendingString:[NSString stringWithUTF8String:e.what()]];}
}
- (void)importTransferCode:(NSString*)code {
 if(![self transferAvailable])return;
 try{using namespace heavymach::transfer;auto target=decode(code.UTF8String);
  auto store=[self transferStore];store.recover();restore(store,target);[self resumeTransfer];[self requestScreen:3];[self transferNotice:@"Progress restored."];
 }catch(const std::exception&e){[self transferNotice:[NSString stringWithUTF8String:e.what()]];}
}
- (void)transferNotice:(NSString*)message {
#if TARGET_OS_OSX
 NSAlert* a=[NSAlert new];a.messageText=message;[a addButtonWithTitle:@"OK"];[a beginSheetModalForWindow:self.window completionHandler:nil];
#else
 UIViewController* vc=self.window.rootViewController;while(vc.presentedViewController)vc=vc.presentedViewController;UIAlertController* a=[UIAlertController alertControllerWithTitle:@"TRANSFER PROGRESS" message:message preferredStyle:UIAlertControllerStyleAlert];[a addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:nil]];[vc presentViewController:a animated:YES completion:nil];
#endif
}
- (void)openTransfer:(id)sender {
 if(![self transferAvailable])return;
#if TARGET_OS_OSX
 NSAlert* a=[NSAlert new];a.messageText=@"TRANSFER PROGRESS";a.informativeText=@"Create a code or paste one to restore all three save slots.";NSTextField* input=[[NSTextField alloc]initWithFrame:NSMakeRect(0,0,440,70)];input.placeholderString=@"HM1-…";input.stringValue=[NSPasteboard.generalPasteboard stringForType:NSPasteboardTypeString]?:@"";a.accessoryView=input;[a addButtonWithTitle:@"CREATE CODE"];[a addButtonWithTitle:@"RESTORE"];[a addButtonWithTitle:@"CANCEL"];
 [a beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse response){if(response==NSAlertFirstButtonReturn){NSString* code=[self createTransferCode];if(![code hasPrefix:@"ERROR:"]){[NSPasteboard.generalPasteboard clearContents];[NSPasteboard.generalPasteboard setString:code forType:NSPasteboardTypeString];[self transferNotice:@"Code copied."];}else [self transferNotice:code];}else if(response==NSAlertSecondButtonReturn)[self confirmTransfer:input.stringValue];}];
#else
 UIAlertController* a=[UIAlertController alertControllerWithTitle:@"TRANSFER PROGRESS" message:@"Create a code or paste one to restore all three save slots." preferredStyle:UIAlertControllerStyleAlert];[a addTextFieldWithConfigurationHandler:^(UITextField*f){f.placeholder=@"HM1-…";f.autocorrectionType=UITextAutocorrectionTypeNo;f.smartDashesType=UITextSmartDashesTypeNo;f.smartQuotesType=UITextSmartQuotesTypeNo;f.spellCheckingType=UITextSpellCheckingTypeNo;f.autocapitalizationType=UITextAutocapitalizationTypeNone;}];
 [a addAction:[UIAlertAction actionWithTitle:@"CREATE CODE" style:UIAlertActionStyleDefault handler:^(UIAlertAction*x){NSString* code=[self createTransferCode];if([code hasPrefix:@"ERROR:"])[self transferNotice:code];else{UIPasteboard.generalPasteboard.string=code;[self transferNotice:@"Code copied."];}}]];
 [a addAction:[UIAlertAction actionWithTitle:@"RESTORE" style:UIAlertActionStyleDefault handler:^(UIAlertAction*x){[self confirmTransfer:a.textFields.firstObject.text];}]];[a addAction:[UIAlertAction actionWithTitle:@"CANCEL" style:UIAlertActionStyleCancel handler:nil]];[self.window.rootViewController presentViewController:a animated:YES completion:nil];
#endif
}
- (void)confirmTransfer:(NSString*)code {
 NSString* message;try{message=[NSString stringWithUTF8String:heavymach::transfer::summary(heavymach::transfer::decode(code.UTF8String)).c_str()];}catch(const std::exception&e){[self transferNotice:[NSString stringWithUTF8String:e.what()]];return;}
#if TARGET_OS_OSX
 NSAlert*a=[NSAlert new];a.messageText=@"RESTORE PROGRESS?";a.informativeText=message;[a addButtonWithTitle:@"RESTORE"];[a addButtonWithTitle:@"CANCEL"];[a beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse r){if(r==NSAlertFirstButtonReturn)[self importTransferCode:code];}];
#else
 UIAlertController*a=[UIAlertController alertControllerWithTitle:@"RESTORE PROGRESS?" message:message preferredStyle:UIAlertControllerStyleAlert];[a addAction:[UIAlertAction actionWithTitle:@"RESTORE" style:UIAlertActionStyleDefault handler:^(UIAlertAction*x){[self importTransferCode:code];}]];[a addAction:[UIAlertAction actionWithTitle:@"CANCEL" style:UIAlertActionStyleCancel handler:nil]];[self.window.rootViewController presentViewController:a animated:YES completion:nil];
#endif
}
