// Included by HMGameView: editor UI uses the game's renderer and coordinate model.
- (heavymach::hud::Configuration)currentHUDLayout {
    auto c=_hudEditing?_hudDraft:_hudLayout;c.legacy={{_hudOffset.x,_hudOffset.y},_hudScale};
    c.safe={_menuSafeRect.origin.x,_menuSafeRect.origin.y,_menuSafeRect.size.width,_menuSafeRect.size.height};
    if(!_hudEditing&&!heavymach::hud::valid(c,HM_VALIDATION))c.positions={};return c;
}
- (NSURL*)hudLayoutURL {
#if TARGET_OS_OSX
    NSString* profile=@"mac";
#else
    NSString* profile=UIDevice.currentDevice.userInterfaceIdiom==UIUserInterfaceIdiomPad?@"ipad":@"iphone";
#endif
    return [[_saveURL URLByDeletingLastPathComponent]URLByAppendingPathComponent:[NSString stringWithFormat:@"HUDLayoutV1-%@.json",profile]];
}
- (void)loadHUDLayout {
    NSData* data=[NSData dataWithContentsOfURL:[self hudLayoutURL]];if(!data)return;
    id object=[NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
    if(![object isKindOfClass:NSDictionary.class]||![object[@"version"] isEqual:@1]||![object[@"positions"] isKindOfClass:NSArray.class])return;
    NSArray* values=object[@"positions"];if(values.count!=9&&values.count!=heavymach::hud::groupCount)return;
    auto candidate=_hudLayout;candidate.positions={};
    for(int i=0;i<values.count;++i){id v=values[i];if(v==NSNull.null)continue;
        if(![v isKindOfClass:NSArray.class]||[v count]!=2||![v[0] isKindOfClass:NSNumber.class]||![v[1] isKindOfClass:NSNumber.class])return;
        double x=[v[0] doubleValue],y=[v[1] doubleValue];if(!std::isfinite(x)||!std::isfinite(y)||x<0||x>1||y<0||y>1)return;
        candidate.positions[i]=heavymach::hud::Placement{x,y};
    }
    _hudLayout=candidate;
}
// Cache each small native editor control at creation; the game remains on GL.
- (HMImage*)hudToolImage:(NSString*)title pressed:(BOOL)pressed {
    CGImageRef sheet=[_media image:@"Common_Btn_OK.png"].CGImage;
    if(!sheet)return nil;
    const CGFloat w=168,h=44;size_t sw=CGImageGetWidth(sheet),sh=CGImageGetHeight(sheet)/2;
    CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx=CGBitmapContextCreate(NULL,w*2,h*2,8,w*8,space,kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(space);if(!ctx)return nil;CGContextScaleCTM(ctx,2,2);
    auto part=[&](CGRect source,CGRect dest){CGImageRef crop=CGImageCreateWithImageInRect(sheet,source);if(crop){CGContextDrawImage(ctx,dest,crop);CGImageRelease(crop);}};
    CGFloat y=pressed?sh:0;
    part(CGRectMake(6,y,2,sh),CGRectMake(8,0,w-16,h));
    part(CGRectMake(0,y,8,sh),CGRectMake(0,0,8,h));
    part(CGRectMake(sw-8,y,8,sh),CGRectMake(w-8,0,8,h));
    CTFontRef font=CTFontCreateWithName(CFSTR("DINCondensed-Bold"),22,NULL);
    CGColorRef gold=CGColorCreateGenericRGB(.98,.72,.22,1),ink=CGColorCreateGenericRGB(.08,.07,.04,1);
    NSDictionary* attrs=@{(__bridge NSString*)kCTFontAttributeName:(__bridge id)font,(__bridge NSString*)kCTForegroundColorAttributeName:(__bridge id)ink,(__bridge NSString*)kCTStrokeColorAttributeName:(__bridge id)gold,(__bridge NSString*)kCTStrokeWidthAttributeName:@(-3)};
    NSAttributedString* text=[[NSAttributedString alloc]initWithString:title attributes:attrs];
    CTLineRef line=CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)text);CGFloat asc,desc;
    double width=CTLineGetTypographicBounds(line,&asc,&desc,NULL);CGFloat scale=std::min(1.,(w-20)/std::max(1.,width));
    CGContextSetTextMatrix(ctx,CGAffineTransformIdentity);CGContextTranslateCTM(ctx,(w-width*scale)/2,(h-(asc+desc)*scale)/2+desc*scale-(pressed?1:0));CGContextScaleCTM(ctx,scale,scale);CTLineDraw(line,ctx);
    CFRelease(line);CFRelease(font);CGColorRelease(gold);CGColorRelease(ink);
    CGImageRef image=CGBitmapContextCreateImage(ctx);HMImage* result=image?[HMImage imageWithCGImage:image]:nil;if(image)CGImageRelease(image);CGContextRelease(ctx);return result;
}
- (HMEditorButton*)editorButton:(NSString*)title action:(SEL)action tag:(NSInteger)tag {
    HMImage* normal=[self hudToolImage:title pressed:NO];HMImage* pressed=[self hudToolImage:title pressed:YES];
#if TARGET_OS_OSX
    HMEditorButton* b=[NSButton buttonWithTitle:title target:self action:action];b.bordered=NO;b.imagePosition=NSImageOnly;b.imageScaling=NSImageScaleAxesIndependently;b.image=normal;b.alternateImage=pressed;[b setButtonType:NSButtonTypeMomentaryChange];b.accessibilityLabel=title;
#else
    HMEditorButton* b=[UIButton buttonWithType:UIButtonTypeCustom];[b setBackgroundImage:normal forState:UIControlStateNormal];[b setBackgroundImage:pressed forState:UIControlStateHighlighted];[b addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];b.accessibilityLabel=title;
#endif
    b.tag=tag;return b;
}
- (void)prepareHUDEditor {
    _hudSelected=heavymach::hud::jump;_hudShowHits=NO;
    _hudEntry=[self editorButton:@"SET GAME UI" action:@selector(openHUDEditor:) tag:0];_hudEntry.hidden=YES;[self addSubview:_hudEntry];
    _hudToolbar=[[HMHUDPanelView alloc]initWithFrame:CGRectZero];_hudToolbar.hidden=YES;[self addSubview:_hudToolbar];
    _hudToolsToggle=[self editorButton:@"TOOLS" action:@selector(hudEditorAction:) tag:5];_hudToolsToggle.hidden=YES;[self addSubview:_hudToolsToggle];
#if TARGET_OS_OSX
    _hudEditorLabel=[NSTextField labelWithString:@""];_hudEditorLabel.textColor=[NSColor colorWithRed:.95 green:.77 blue:.4 alpha:1];_hudEditorLabel.drawsBackground=YES;_hudEditorLabel.backgroundColor=[NSColor colorWithWhite:.06 alpha:.94];_hudEditorLabel.maximumNumberOfLines=3;_hudEditorLabel.font=[NSFont systemFontOfSize:12];
#else
    _hudEditorLabel=[UILabel new];_hudEditorLabel.textColor=[UIColor colorWithRed:.95 green:.77 blue:.4 alpha:1];_hudEditorLabel.backgroundColor=[UIColor colorWithWhite:.06 alpha:.94];_hudEditorLabel.font=[UIFont systemFontOfSize:12];_hudEditorLabel.numberOfLines=3;_hudEditorLabel.textAlignment=NSTextAlignmentCenter;_hudEditorLabel.adjustsFontSizeToFitWidth=YES;
#endif
    _hudToolRows=[[HMHUDPanelView alloc]initWithFrame:CGRectZero];_hudToolScroll=[[HMEditorScroll alloc]initWithFrame:CGRectZero];
#if TARGET_OS_OSX
    _hudToolScroll.drawsBackground=NO;_hudToolScroll.hasVerticalScroller=YES;_hudToolScroll.autohidesScrollers=YES;_hudToolScroll.documentView=_hudToolRows;
#else
    _hudToolScroll.alwaysBounceVertical=NO;[_hudToolScroll addSubview:_hudToolRows];
#endif
    [_hudToolbar addSubview:_hudToolScroll];[_hudToolRows addSubview:_hudEditorLabel];NSMutableArray* buttons=[NSMutableArray new];
    NSArray* titles=@[@"RESET LAYOUT",@"CANCEL",@"APPLY",@"PREVIEW",@"SAMPLE SCENE"];
    for(int i=0;i<titles.count;++i){auto b=[self editorButton:titles[i] action:@selector(hudEditorAction:) tag:i];[(i==1||i==2)?_hudToolbar:_hudToolRows addSubview:b];[buttons addObject:b];}_hudEditorButtons=buttons;
}
- (void)updateHUDEntry {
    _transferEntry.hidden=![self transferAvailable];
    CGRect transferSafe=[self safeContentRect];_transferEntry.frame=CGRectMake(CGRectGetMaxX(transferSafe)-176,CGRectGetMinY(transferSafe)+8,168,44);
    // HUD editing remains in the pause menu, including layouts from another device.
    BOOL hidden=YES;
    if(_hudEntry.hidden!=hidden)_hudEntry.hidden=hidden;
}
- (void)updateHUDEditorGeometry {
    BOOL changed=_hudDraft.safe.x!=_menuSafeRect.origin.x||_hudDraft.safe.y!=_menuSafeRect.origin.y||_hudDraft.safe.width!=_menuSafeRect.size.width||_hudDraft.safe.height!=_menuSafeRect.size.height||_hudDraft.legacy.scale!=_hudScale;
    // Editing must not change the live session's render cache on resize.
    if(_hudEditing&&(_hudDraft.safe.width!=_menuSafeRect.size.width||_hudDraft.safe.height!=_menuSafeRect.size.height)){
        _hudDragToken=nil;_hudExample=YES;_hudExampleSession.reset();
    }
    _hudLayout.legacy={{_hudOffset.x,_hudOffset.y},_hudScale};_hudLayout.safe={_menuSafeRect.origin.x,_menuSafeRect.origin.y,_menuSafeRect.size.width,_menuSafeRect.size.height};
#if HM_VALIDATION
    CGRect nsafe=[self safeContentRect];CGRect f=_viewport.x-CGRectGetMinX(nsafe)>=68?CGRectMake(_viewport.x-68,CGRectGetMidY(nsafe)-94,60,188):CGRectMake(CGRectGetMidX(nsafe)-104,CGRectGetMinY(nsafe)+8,208,44);
    auto origin=_hudLayout.legacy.inverse(heavymach::hud::Point{(f.origin.x-_viewport.x)/_viewport.scale,_canvasHeight-(CGRectGetMaxY(f)-_viewport.y)/_viewport.verticalScale()});
    _hudLayout.cheatBounds={origin.x,origin.y,f.size.width/_viewport.scale/_hudScale,f.size.height/_viewport.verticalScale()/_hudScale};_hudDraft.cheatBounds=_hudLayout.cheatBounds;
#endif
    _hudLayoutFallback=!heavymach::hud::valid(_hudLayout,HM_VALIDATION);
    _hudDraft.legacy=_hudLayout.legacy;_hudDraft.safe=_hudLayout.safe;
    if(_hudEditing&&changed)[self layoutHUDControls];
}
- (CGRect)hudNativeRect:(heavymach::hud::Rect)r {
    return CGRectMake(_viewport.x+r.x*_viewport.scale,_viewport.y+(_canvasHeight-r.y-r.height)*_viewport.verticalScale(),r.width*_viewport.scale,r.height*_viewport.verticalScale());
}
- (void)layoutHUDControls {
    CGRect safe=[self safeContentRect];CGFloat w=168,h=44;
    _hudEntry.frame=CGRectMake(CGRectGetMaxX(safe)-w-8,CGRectGetMidY(safe)-22,w,h);
    _hudToolsToggle.frame=CGRectMake(CGRectGetMaxX(safe)-w-8,CGRectGetMinY(safe)+4,w,h);
    CGFloat bodyHeight=48+(_hudFromMenu?2:3)*44,height=std::min(bodyHeight+88,safe.size.height-52);
    _hudToolbar.frame=CGRectMake(CGRectGetMaxX(safe)-w-8,CGRectGetMinY(safe)+48,w,height);
    _hudToolScroll.frame=CGRectMake(0,0,w,std::max(CGFloat(0),height-88));
    _hudToolRows.frame=CGRectMake(0,0,w,bodyHeight);
#if !TARGET_OS_OSX
    _hudToolScroll.contentSize=CGSizeMake(w,bodyHeight);
#endif
    _hudEditorLabel.frame=CGRectMake(0,0,w,48);
    int order[]={3,4,0};CGFloat y=48;
    for(int i:order){auto b=_hudEditorButtons[i];b.hidden=i==4&&_hudFromMenu;if(!b.hidden){b.frame=CGRectMake(0,y,w,44);y+=44;}}
    _hudEditorButtons[1].frame=CGRectMake(0,height-88,w,44);_hudEditorButtons[2].frame=CGRectMake(0,height-44,w,44);
    _hudToolbar.hidden=!_hudEditing||_hudHideGuides;_hudToolsToggle.hidden=!_hudEditing;
    if(_hudEditing){
#if TARGET_OS_OSX
        [self addSubview:_hudToolbar positioned:NSWindowAbove relativeTo:nil];[self addSubview:_hudToolsToggle positioned:NSWindowAbove relativeTo:nil];
#else
        [self bringSubviewToFront:_hudToolbar];[self bringSubviewToFront:_hudToolsToggle];
#endif
    }
#if defined(HM_VALIDATION) && HM_VALIDATION
    auto c=[self currentHUDLayout];
    if(!c.positions[heavymach::hud::cheat]){
        if(_viewport.x-CGRectGetMinX(safe)>=68)_validationControls.frame=CGRectMake(_viewport.x-68,CGRectGetMidY(safe)-94,60,188);
        else _validationControls.frame=CGRectMake(CGRectGetMidX(safe)-104,CGRectGetMinY(safe)+8,208,44);
    }
    else _validationControls.frame=[self hudNativeRect:c.group(heavymach::hud::cheat).visualBounds];
    _validationButton.hidden=_hudEditing;
    _validationControls.hudEditing=_hudEditing;
    _validationControls.hidden=_hudEditing?NO:(!_session||_showMenu);
#endif
    [self updateHUDEntry];if(_hudEditing)[self refreshHUDEditorLabel:nil];
}
- (void)refreshHUDEditorLabel:(NSString*)error {
    NSArray* names=@[@"MENU",@"LEVEL / EXP",@"JUMP",@"WEAPON / AMMO",@"GAUGES",@"AIM CONTROLS",@"PASSIVES",@"WEAPON PREVIEW",@"CHEAT CONTROLS",@"UPPER TRIM"];
    NSString* sceneTitle=_hudExample?@"CURRENT SCENE":@"SAMPLE SCENE";
    HMEditorButton* sceneButton=_hudEditorButtons[4];
    if(![sceneButton.accessibilityLabel isEqualToString:sceneTitle]){
        sceneButton.accessibilityLabel=sceneTitle;
#if TARGET_OS_OSX
        sceneButton.title=sceneTitle;sceneButton.image=[self hudToolImage:sceneTitle pressed:NO];sceneButton.alternateImage=[self hudToolImage:sceneTitle pressed:YES];
#else
        [sceneButton setBackgroundImage:[self hudToolImage:sceneTitle pressed:NO] forState:UIControlStateNormal];[sceneButton setBackgroundImage:[self hudToolImage:sceneTitle pressed:YES] forState:UIControlStateHighlighted];
#endif
    }
    NSString* label=error?:[NSString stringWithFormat:@"HUD LAYOUT · %@\n%@ · Drag to move.",_hudExample?@"SAMPLE":@"PAUSED",names[_hudSelected]];
#if TARGET_OS_OSX
    _hudEditorLabel.stringValue=label;
#else
    _hudEditorLabel.text=label;
#endif
}
- (void)openHUDEditor:(id)sender {
    if(_loading.visible()||_hudEditing||_showMenu||!(_session&&(_gameMenu||_hudLayoutFallback)&&_session->world().gameState==0))return;
    if(_session&&!_gameMenu){_gameMenu=YES;[_hud setMenuVisible:YES];}
    [self cancelGameInput];_hudFromMenu=_showMenu;_hudExample=_hudFromMenu;_hudDraft=_hudLayout;_hudEditing=YES;_hudHideGuides=NO;_hudDragToken=nil;
    _hudToolbar.hidden=NO;
#if HM_VALIDATION
    [self stopValidationDirection];
#endif
    if(_session)_session->setPaused(true);
    [self layoutHUDControls];
#if TARGET_OS_OSX
    [self.window makeFirstResponder:self];
#else
    [self becomeFirstResponder];
#endif
}
- (void)closeHUDEditor {
    _hudEditing=NO;_hudToolbar.hidden=YES;_hudToolsToggle.hidden=YES;_hudDragToken=nil;_hudExampleSession.reset();_hudExampleData={};[_media releaseScope:@"hud-example"];
    [self cancelGameInput];[self noteInteraction];
#if TARGET_OS_OSX
    [self layoutGameSurface];
#else
    [self setNeedsLayout];[self layoutIfNeeded];
#endif
    // Return to the pause menu after editing.
}
- (void)hudEditorAction:(HMEditorButton*)sender {
    if(!_hudEditing)return;
    switch(sender.tag){
      case 0:_hudDraft.positions={};_hudDragToken=nil;break;
      case 1:[self closeHUDEditor];return;
      case 2:{auto c=[self currentHUDLayout];
        if(!heavymach::hud::valid(c,HM_VALIDATION)){[self refreshHUDEditorLabel:@"Move overlapping buttons inside the screen."];return;}
        if(!_hudDraft.samePositions(_hudLayout)){
            NSMutableArray* values=[NSMutableArray new];for(auto p:_hudDraft.positions)[values addObject:p?(id)@[@(p->x),@(p->y)]:(id)NSNull.null];
            NSError* error=nil;NSData* bytes=[NSJSONSerialization dataWithJSONObject:@{@"version":@1,@"positions":values} options:0 error:&error];
            if(!bytes||![bytes writeToURL:[self hudLayoutURL] options:NSDataWritingAtomic error:&error]){[self refreshHUDEditorLabel:@"Could not save. Try APPLY again."];return;}
            ++_hudLayoutWrites;_hudLayout=_hudDraft;
        }
        [self closeHUDEditor];return;
      }
      case 3:_hudHideGuides=YES;break;
      case 4:if(!_hudFromMenu)_hudExample=!_hudExample;break;
      case 5:_hudHideGuides=!_hudHideGuides;break;
    }
    [self layoutHUDControls];
}
- (BOOL)hudEditorKey:(unsigned)code pressed:(BOOL)pressed {
    if(!pressed)return YES;
    if(code==0x29){[self closeHUDEditor];return YES;}
    if(code==0x2b){do {_hudSelected=(_hudSelected+1)%heavymach::hud::groupCount;}while(!HM_VALIDATION&&_hudSelected==heavymach::hud::cheat);_hudHideGuides=NO;[self layoutHUDControls];return YES;}
    auto c=[self currentHUDLayout];auto r=c.group(_hudSelected).visualBounds;double dx=code==0x4f?1:code==0x50?-1:0,dy=code==0x52?1:code==0x51?-1:0;
    if(dx||dy){_hudDraft.positions[_hudSelected]=heavymach::hud::placementForOrigin(c.localBounds(_hudSelected),c.legacy,c.safe,{r.x+dx,r.y+dy});[self layoutHUDControls];}
    return YES;
}
- (void)hudEditorPoint:(CGPoint)p state:(int)state token:(id)token {
    auto c=[self currentHUDLayout];
    if(state==0&&!_hudDragToken){
        int hit=-1;
        for(int j=heavymach::hud::groupCount-1;j>=0;--j){int i=j==0?heavymach::hud::upperTrim:j-1;if(!HM_VALIDATION&&i==heavymach::hud::cheat)continue;auto r=c.group(i).visualBounds;

            if(r.contains({p.x,p.y},heavymach::hud::HitEdges::standard)){hit=i;_hudGrab=CGPointMake(p.x-r.x,p.y-r.y);break;}
        }
        if(hit<0)return;
        _hudHideGuides=YES;_hudSelected=hit;_hudDragToken=token;[self refreshHUDEditorLabel:nil];
    }
    if(token!=_hudDragToken)return;
    if(state<3&&p.x>=0&&p.y>=0){
        auto r=c.group(_hudSelected).visualBounds;double x=p.x-_hudGrab.x,y=p.y-_hudGrab.y;
        if(std::abs(x-r.x)>.01||std::abs(y-r.y)>.01){
            auto original=c.legacy.apply(c.localBounds(_hudSelected));
            if(std::abs(x-original.x)<.01&&std::abs(y-original.y)<.01)_hudDraft.positions[_hudSelected].reset();
            else _hudDraft.positions[_hudSelected]=heavymach::hud::placementForOrigin(c.localBounds(_hudSelected),c.legacy,c.safe,{x,y});
        }
    }
    if(state>=2)_hudDragToken=nil;
    [self layoutHUDControls];
}
- (void)prepareHUDExample {
    if(_hudExampleSession)return;
    _hudExampleData=_data;_hudExampleSlot={};heavymach::GlobalInfo(_hudExampleSlot).selectSlot(1);_hudExampleRandom=12345;
    auto& map=_hudExampleData.maps.at("Stage01");map.enemies={{"Tank001",{map.left+240,100},false},{"Heli001",{map.left+330,190},false}};
    HMMedia* media=_media;__weak HMGameView* owner=self;
    _hudExampleSession=std::make_unique<heavymach::GameSession>(_hudExampleData,_hudExampleSlot,[media](std::string_view name){NSString* key=[[NSString alloc]initWithBytes:name.data() length:name.size() encoding:NSUTF8StringEncoding];CGSize z=[media imageSize:key scope:@"hud-example"];return heavymach::Vector2{float(z.width),float(z.height)};},[owner]{HMGameView* v=owner;if(!v)return int32_t(0);uint32_t x=v->_hudExampleRandom;x^=x<<13;x^=x>>17;x^=x<<5;v->_hudExampleRandom=x;return int32_t(x&0x7fffffff);});
    _hudExampleSession->setCanvasWidth(_canvasWidth);_hudExampleSession->setCanvasHeight(_canvasHeight);_hudExampleSession->start(0,0,0);
    for(int i=1;i<=45;++i){_hudExampleSession->setPaused(false);_hudExampleSession->tick(i/60.,0);_hudExampleSession->takeEvents();}
    _hudExampleSession->prepareHUDPreview();
}
- (void)drawHUDEditor {
    if(_hudExample){[self prepareHUDExample];}
    auto* scene=_hudExample?_hudExampleSession.get():_session.get();if(!scene)return;
    [_media setDrawingOffset:CGPointZero];[_media setDrawingScale:1];[_media resetClip];
    if(!_hudExample)[_media setCameraOffset:CGPointMake(_worldShake.offset.x,_worldShake.offset.y)];
    for(const auto& item:scene->renderItems()){
        if(auto sprite=std::get_if<heavymach::DrawSprite>(&item)){
            _media.resourceScope=_hudExample?@"hud-example":sprite->worldTexture?@"world":@"shared";
            [_media draw:[NSString stringWithUTF8String:sprite->tile.c_str()] x:sprite->position.x y:sprite->position.y columns:sprite->cols rows:sprite->rows tile:sprite->frame scaleX:sprite->scale.x scaleY:sprite->scale.y rotation:sprite->rotation alpha:sprite->alpha blend:sprite->blend];
        }else { _media.resourceScope=_hudExample?@"hud-example":@"shared";[_media drawTrail:std::get<heavymach::TrailMesh>(item)];}
    }
    _media.resourceScope=@"shared";[_media setCameraOffset:CGPointZero];[_media setDrawingOffset:_hudOffset];[_media setDrawingScale:_hudScale];
    [_media setClipRect:CGRectMake(_hudOffset.x,_hudOffset.y,480*_hudScale,320*_hudScale)];
    HMGameHUDSnapshot s;s.slot=_hudExample?&_hudExampleSlot:&_save.slots[_slotIndex];s.data=&_data;s.passives=_hudExample?&_hudExamplePassives:&_passives;s.time=_time;s.ammo=99;s.expGauge=_expGauge;s.reloadGauge=_reloadGauge;
    s.canvasWidth=_canvasWidth;s.canvasHeight=_canvasHeight;s.hudOffset=_hudOffset;s.hudScale=_hudScale;s.selectedWeapon=scene->selectedWeapon();
    if(s.slot->weaponCount()>0){auto w=_data.weapons.find(s.slot->weaponName(s.selectedWeapon));if(w!=_data.weapons.end())s.weapon=&w->second;s.ammo=s.slot->weaponCharge(s.selectedWeapon);}
    [_hud setLayout:[self currentHUDLayout] editing:YES];[_hud draw:s];[self drawAimControls];
    [_media resetClip];[_media setDrawingOffset:CGPointZero];[_media setDrawingScale:1];
}
