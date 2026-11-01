// These controls use the reconstructed QobButton state, sprite drawing and hit test.
- (CGRect)gameButtonRect:(int)identifier {
    [self prepareGameButtons];auto r=_gameButtons.rect(identifier);
    return r?CGRectMake(r->x,r->y,r->width,r->height):CGRectZero;
}
- (void)prepareGameButtons {
    using heavymach::gameui::Rect;
    _gameButtons.beginFrame(CACurrentMediaTime());
    if(_loadError||_loading.state()!=6)return;
    const double left=CGRectGetMinX(_menuSafeRect)+8,right=CGRectGetMaxX(_menuSafeRect)-8;
    const double bottom=CGRectGetMinY(_menuSafeRect)+8,top=CGRectGetMaxY(_menuSafeRect)-8;
    const double width=right-left,height=top-bottom,cx=(left+right)/2;
    auto button=[&](int id,NSString* label,double x,double y,double w,double h,bool enabled=true){_gameButtons.add(id,label.UTF8String,{x,y,w,h},enabled);};
    if(_transferPage){
        _gameButtons.panel({left,bottom,width,height});
        _gameButtons.text("TRANSFER PROGRESS",{left+8,top-28,width-16,24},18,true);
        if(_transferPage==2){
            _gameButtons.text(_gameUIMessage.UTF8String?:"",{left+16,bottom+90,width-32,height-138},13,false);
            button(3115,@"RESTORE",cx-158,bottom+12,152,40);button(3116,@"CANCEL",cx+6,bottom+12,152,40);
        }else{
            _gameButtons.text(_gameUIMessage.UTF8String?:"CREATE A CODE OR PASTE ONE FROM ANOTHER DEVICE.",{left+12,bottom+104,width-24,std::max(24.,height-190)},12,false);
            double w=std::min(152.,(width-16)/3),x=cx-(3*w+12)/2;
            button(3110,@"CREATE CODE",x,bottom+52,w,40);button(3111,@"COPY",x+w+6,bottom+52,w,40);button(3112,@"CHECK CODE",x+2*(w+6),bottom+52,w,40);
            button(3113,@"RESTORE",cx-w-3,bottom+6,w,40);button(3114,@"CLOSE",cx+3,bottom+6,w,40);
        }return;
    }
#if HM_VALIDATION
    if(_validationPanelOpen){
        _gameButtons.panel({left,bottom,width,height});
        _gameButtons.text("TEST SETTINGS",{left+8,top-26,width-16,22},18,true);
        double w=std::min(440.,width),x=cx-w/2;
        int first=_validationPanel.page?6:0,last=_validationPanel.page?11:6;
        double rowH=std::min(32.,(height-154)/6),y=top-32-rowH;
        for(int i=first;i<last;++i,y-=rowH+2){
            button(3400+i*2,@"-",x,y,36,rowH);
            _gameButtons.text([_validationPanel fieldText:i].UTF8String,{x+40,y,w-80,rowH},std::min(13.,rowH*.48),true);
            button(3401+i*2,@"+",x+w-36,y,36,rowH);
        }
        double bw=(w-12)/3;
        button(3301,@"START TEST",x,bottom+44,bw,36);button(3302,@"RESUME SAVE",x+bw+6,bottom+44,bw,36);button(3303,@"ADD EXP 1",x+2*(bw+6),bottom+44,bw,36,_session!=nullptr);
        button(3304,_validationPanel.page?@"PREVIOUS":@"NEXT",x,bottom+4,bw,36);button(3305,@"CLOSE",x+2*(bw+6),bottom+4,bw,36);
        if(_gameUIMessage.length)_gameButtons.text(_gameUIMessage.UTF8String,{x,bottom+84,w,24},11,false);
        return;
    }
#endif
    if(_keyboardSettingsOpen){
        _gameButtons.panel({left,bottom,width,height});
        _gameButtons.text(_capturingKey?"PRESS A KEY. ESC CANCELS.":"KEY BINDINGS",{left+4,top-26,width-8,24},16,true);
        static const char* names[]={"MOVE LEFT","MOVE RIGHT","AIM LEFT","AIM RIGHT","AIM UP","AIM DOWN","FIRE","JUMP","PREVIOUS WEAPON","NEXT WEAPON","FINE MOVEMENT","CONFIRM","PAUSE / BACK","NEXT BUTTON","KEY SETTINGS","NEXT TARGET"};
        double w=(width-8)/2,h=std::min(30.,(height-76)/8);
        for(int i=0;i<16;++i){NSString* label=[NSString stringWithFormat:@"%s: %@",names[i],[self keyName:_keyboardControls.bindings[i]]];button(3200+i,label,left+(i/8)*(w+8),top-34-(i%8+1)*h,w,h-1);}
        button(3220,@"RESET KEYS",cx-154,bottom,148,34);button(3221,@"CLOSE",cx+6,bottom,148,34);return;
    }
    if(_hudEditing){
        double w=std::min(168.,width*.42),h=std::min(40.,(height-24)/7),x=right-w,y=top-h;
        button(3005,@"TOOLS",x,y,w,h);y-=h+3;
        if(!_hudHideGuides){
            _gameButtons.text(_hudStatus.UTF8String?:(_gameToolsFocus?"TAB / ENTER. F6: DRAG":"DRAG HUD. F6: TOOLS"),{x,y+h-22,w,24},10,true);y-=26;
            button(3003,@"PREVIEW",x,y,w,h);y-=h+3;
            if(!_hudFromMenu){button(3004,_hudExample?@"CURRENT SCENE":@"SAMPLE SCENE",x,y,w,h);y-=h+3;}
            button(3000,@"RESET LAYOUT",x,y,w,h);y-=h+3;
            button(3001,@"CANCEL",x,y,w,h);y-=h+3;
            button(3002,@"APPLY",x,y,w,h);
        }
#if HM_VALIDATION
        [self addValidationDirectionButtons:YES];
#endif
        return;
    }
    if([self transferAvailable])button(3100,@"TRANSFER PROGRESS",(_canvasWidth>480?cx:200)-87,bottom-4,174,36);
#if HM_VALIDATION
    button(3300,@"TEST",[self transferAvailable]?left:right-84,top-32,84,32);
    if(!_validationControls.hidden&&_session&&!_showMenu)[self addValidationDirectionButtons:NO];
#endif
}
#if HM_VALIDATION
- (void)addValidationDirectionButtons:(BOOL)editing {
    // HUD editing draws the same controls as a movable group; only tools receive taps.
    CGRect native=_validationControls.frame;
    auto a=_viewport.logical(CGRectGetMinX(native),CGRectGetMaxY(native));
    double x=a?a->first:0,y=a?a->second:0,w=native.size.width/_viewport.scale,h=native.size.height/_viewport.verticalScale();
    bool vertical=h>w;double extent=((vertical?h:w)-6)/4;
    NSArray* labels=@[@"LEFT",@"STOP",@"RIGHT",@"TILT"];
    for(int i=0;i<4;++i){heavymach::gameui::Rect r=vertical?heavymach::gameui::Rect{x,y+(3-i)*(extent+2),w,extent}:heavymach::gameui::Rect{x+i*(extent+2),y,extent,h};_gameButtons.add(3500+i,[labels[i]UTF8String],r,!editing);}
}
#endif
- (void)drawGameButtons {
    [self prepareGameButtons];
    _media.presentationOffset=CGPointZero;_media.resourceScope=@"shared";
    [_media setCameraOffset:CGPointZero];[_media setDrawingOffset:CGPointZero];[_media setDrawingScale:1];[_media resetClip];
    _gameButtons.draw([&](const char* image,heavymach::gameui::Rect s,heavymach::gameui::Rect d){[_media drawRegion:[NSString stringWithUTF8String:image] source:CGRectMake(s.x,s.y,s.width,s.height) destination:CGRectMake(d.x,d.y,d.width,d.height)];});
    [self layoutTransferInput];
}
- (void)gameUIAction:(int)identifier state:(int)state {
    if(state!=2)return;[self noteInteraction];[_media sound:@"UI_Click"];
    if(identifier>=3000&&identifier<=3005){[self hudEditorAction:identifier-3000];return;}
    if(identifier==3100){[self openTransfer:nil];return;}
    if(identifier>=3110&&identifier<=3116){[self transferAction:identifier];return;}
    if(identifier>=3200&&identifier<3216){_settingsSelection=identifier-3200;_capturingKey=YES;return;}
    if(identifier==3220){_keyboardControls.bindings=heavymach::KeyboardInput::defaults;[self saveKeyboardBindings];return;}
    if(identifier==3221){[self toggleKeyboardSettings];return;}
#if HM_VALIDATION
    if(identifier>=3500&&identifier<3504){_validationControls.selectedSegmentIndex=identifier-3500;[self validationDirectionChanged:_validationControls];return;}
    if(identifier==3300){[self openValidation];return;}
    if(identifier>=3400&&identifier<3422){int f=(identifier-3400)/2;[_validationPanel adjust:f delta:identifier%2?1:-1];if(f==10)[self setWideCanvasEnabled:_validationPanel.wideCanvas];return;}
    if(identifier==3304){_validationPanel.page=1-_validationPanel.page;_gameButtons.cancel();return;}
    if(identifier==3305){[self closeValidation];return;}
    if(identifier==3301){
        try{auto s=[_validationPanel scenario];[_validationPanel saveSettings];[self closeValidation];_resumeRequired=NO;_validationFixture=s;_loadError=nil;_loading={};[self startSlot:0 mode:0 mission:s.mission stage:s.stage];[self persistSave];}
        catch(const std::exception&e){_gameUIMessage=[NSString stringWithUTF8String:e.what()];}return;
    }
    if(identifier==3302){_gameUIMessage=nil;[self closeValidation];_resumeRequired=NO;[self resumeValidationSave];if(_gameUIMessage.length)_validationPanelOpen=YES;return;}
    if(identifier==3303&&_session){_session->validationAddExperience(1);[self consumeGameEvents];_gameUIMessage=@"EXP 1 ADDED.";return;}
#endif
}
