- (BOOL)transferAvailable {
 return _transferScreen==3&&_showMenu&&!_session&&!_hudEditing&&!_keyboardSettingsOpen&&_loading.state()==6&&!_loadError;
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
- (void)transferNotice:(NSString*)message {_gameUIMessage=message;_transferPage=1;[self layoutTransferInput];}
- (NSString*)transferInputText {
#if TARGET_OS_OSX
 return _transferInput.stringValue?:@"";
#else
 return _transferInput.text?:@"";
#endif
}
- (void)setTransferInputText:(NSString*)text {
#if TARGET_OS_OSX
 _transferInput.stringValue=text?:@"";
#else
 _transferInput.text=text?:@"";
#endif
}
- (void)layoutTransferInput {
 if(!_transferInput)return;
 _transferInput.hidden=_transferPage!=1||_loading.visible()||!_active;
 double x=CGRectGetMinX(_menuSafeRect)+20,y=CGRectGetMaxY(_menuSafeRect)-82,w=_menuSafeRect.size.width-40;
 _transferInput.frame=CGRectMake(_viewport.x+x*_viewport.scale,_viewport.y+(_canvasHeight-y-34)*_viewport.verticalScale(),w*_viewport.scale,34*_viewport.verticalScale());
}
- (void)finishTransferTyping {
#if TARGET_OS_OSX
 [self.window makeFirstResponder:self];
#else
 [_transferInput resignFirstResponder];[self becomeFirstResponder];
#endif
}
#if !TARGET_OS_OSX
- (BOOL)textFieldShouldReturn:(UITextField*)textField {[self finishTransferTyping];return YES;}
#endif
- (void)openTransfer:(id)sender {
 if(![self transferAvailable])return;
 [self cancelGameInput];_transferPage=1;_transferPendingCode=nil;_gameUIMessage=@"CREATE A CODE OR PASTE ONE TO RESTORE YOUR SAVE SLOTS.";
 if(!_transferInput){
#if TARGET_OS_OSX
  _transferInput=[[NSTextField alloc]initWithFrame:CGRectZero];_transferInput.placeholderString=@"HM1-...";_transferInput.target=self;_transferInput.action=@selector(finishTransferTyping);_transferInput.font=[NSFont monospacedSystemFontOfSize:16 weight:NSFontWeightRegular];_transferInput.textColor=NSColor.whiteColor;_transferInput.backgroundColor=[NSColor colorWithWhite:.08 alpha:1];_transferInput.bezeled=NO;_transferInput.bordered=YES;_transferInput.drawsBackground=YES;_transferInput.placeholderAttributedString=[[NSAttributedString alloc]initWithString:@"HM1-..." attributes:@{NSForegroundColorAttributeName:[NSColor colorWithWhite:.7 alpha:1]}];
#else
  _transferInput=[[UITextField alloc]initWithFrame:CGRectZero];_transferInput.delegate=self;_transferInput.placeholder=@"HM1-...";_transferInput.returnKeyType=UIReturnKeyDone;_transferInput.autocorrectionType=UITextAutocorrectionTypeNo;_transferInput.smartDashesType=UITextSmartDashesTypeNo;_transferInput.smartQuotesType=UITextSmartQuotesTypeNo;_transferInput.spellCheckingType=UITextSpellCheckingTypeNo;_transferInput.autocapitalizationType=UITextAutocapitalizationTypeNone;_transferInput.font=[UIFont monospacedSystemFontOfSize:16 weight:UIFontWeightRegular];_transferInput.textColor=UIColor.whiteColor;_transferInput.backgroundColor=[UIColor colorWithWhite:.08 alpha:1];_transferInput.attributedPlaceholder=[[NSAttributedString alloc]initWithString:@"HM1-..." attributes:@{NSForegroundColorAttributeName:[UIColor colorWithWhite:.7 alpha:1]}];
#endif
  _transferInput.accessibilityLabel=@"Recovery code";[self addSubview:_transferInput];
 }
 [self layoutTransferInput];
}
- (void)closeTransfer {
 [self finishTransferTyping];_transferPage=0;_transferPendingCode=nil;_gameUIMessage=nil;_gameButtons.cancel();[self layoutTransferInput];
}
- (void)confirmTransfer:(NSString*)code {
 try{_gameUIMessage=[NSString stringWithUTF8String:heavymach::transfer::summary(heavymach::transfer::decode(code.UTF8String)).c_str()];_transferPendingCode=[code copy];_transferPage=2;[self finishTransferTyping];[self layoutTransferInput];}
 catch(const std::exception&e){[self transferNotice:[NSString stringWithUTF8String:e.what()]];}
}
- (void)transferAction:(int)action {
 if(![self transferAvailable])return;
 [self finishTransferTyping];
 if(action==3110){NSString* code=[self createTransferCode];if([code hasPrefix:@"ERROR:"])[self transferNotice:code];else{[self setTransferInputText:code];_gameUIMessage=@"CODE CREATED. COPY IT TO ANOTHER DEVICE.";}}
 else if(action==3111){NSString* code=[self transferInputText];
#if TARGET_OS_OSX
  [NSPasteboard.generalPasteboard clearContents];[NSPasteboard.generalPasteboard setString:code forType:NSPasteboardTypeString];
#else
  UIPasteboard.generalPasteboard.string=code;
#endif
  _gameUIMessage=@"CODE COPIED.";
 }else if(action==3112){try{_gameUIMessage=[NSString stringWithUTF8String:heavymach::transfer::summary(heavymach::transfer::decode([self transferInputText].UTF8String)).c_str()];}catch(const std::exception&e){_gameUIMessage=[NSString stringWithUTF8String:e.what()];}}
 else if(action==3113)[self confirmTransfer:[self transferInputText]];
 else if(action==3114)[self closeTransfer];
 else if(action==3115){NSString* code=_transferPendingCode;_transferPendingCode=nil;_transferPage=1;[self importTransferCode:code];}
 else if(action==3116){_transferPendingCode=nil;_transferPage=1;[self layoutTransferInput];}
}
