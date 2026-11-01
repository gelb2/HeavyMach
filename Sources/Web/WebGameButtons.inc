// Additional controls use the same sprite/pressed/hit procedure as the menus.
EM_JS(void,hm_transfer_field,(double x,double y,double w,double h,int visible),{if(Module.hmTransferField)Module.hmTransferField(x,y,w,h,visible);});
EM_JS(void,hm_storage_retry,(),{if(Module.hmRetryStorage)Module.hmRetryStorage();});
namespace hmweb {
void GameView::storageUIState(const char* status){
    std::string next=status?status:"";if(!next.empty()&&_storageMessage.empty())pauseForInterruption();_storageMessage=std::move(next);_audio->setApplicationActive(_active&&_storageMessage.empty());
}

void GameView::transferUIState(bool ready,int mode,bool busy,int source,int target,bool settings,const char* status){
    if(_transferMode!=mode){cancelGameInput();_gameButtons.cancel();}
    _transferReady=ready;_transferMode=std::clamp(mode,0,2);_transferBusy=busy;
    _transferSource=source;_transferTarget=target;_transferSettings=settings;
    _transferStatus=status?status:"";
}
void GameView::drawGameButtons(){
    using R=heavymach::gameui::Rect;
    _gameButtons.beginFrame(interactionTime());
    _media->presentationOffset={};_media->setCameraOffset({});_media->setDrawingOffset({});_media->setDrawingScale(1);_media->resetClip();_media->resourceScope="shared";
    if(_transferMode!=1||!_storageMessage.empty()||_loading.visible()||!_loadError.empty())hm_transfer_field(0,0,0,0,0);
    if((_loading.state()<6&&_storageMessage.empty())||!_loadError.empty())return;
    R safe{_menuSafeRect.origin.x,_menuSafeRect.origin.y,_menuSafeRect.size.width,_menuSafeRect.size.height};
    if(!_storageMessage.empty()){
        const double w=std::min(420.0,safe.width-16),h=150,x=safe.x+(safe.width-w)/2,y=safe.y+(safe.height-h)/2;
        _gameButtons.panel({x,y,w,h});_gameButtons.text("SAVE FAILED",{x+10,y+122,w-20,18},15,true);
        _gameButtons.text(_storageMessage,{x+12,y+52,w-24,62},12,false);_gameButtons.add(1030,"RETRY",{x+w/2-75,y+10,150,32});
    }else if(_transferMode){
        double s=std::min({1.0,(safe.width-12)/460,(safe.height-12)/300});
        double x=safe.x+(safe.width-460*s)/2,y=safe.y+(safe.height-300*s)/2;
        auto r=[&](double a,double b,double w,double h){return R{x+a*s,y+b*s,w*s,h*s};};
        _gameButtons.panel(r(0,0,460,300));
        _gameButtons.text(_transferMode==2?"RESTORE PROGRESS?":"TRANSFER PROGRESS",r(10,276,440,18),17*s,true);
        if(_transferMode==1){
            R field=r(12,216,436,48);
            hm_transfer_field(_viewport.x+field.x*_viewport.scale,_viewport.y+(_canvasHeight-field.y-field.height)*_viewport.verticalScale(),field.width*_viewport.scale,field.height*_viewport.verticalScale(),1);
            _gameButtons.text(_transferStatus,r(12,151,436,56),10*s,false);
            const char* captions[]={"CREATE CODE","COPY","SAVE CODE FILE","OPEN CODE FILE","CHECK CODE","RESTORE"};
            const int ids[]={901,902,903,904,908,909};
            for(int i=0;i<6;++i)_gameButtons.add(ids[i],captions[i],r(10+(i%3)*150,116-(i/3)*32,140,28),!_transferBusy);
            _gameButtons.add(905,"FROM "+(_transferSource<0?std::string("ALL"):std::to_string(_transferSource+1)),r(10,52,140,28),!_transferBusy);
            _gameButtons.add(906,"TO "+(_transferTarget<0?std::string("ALL"):std::to_string(_transferTarget+1)),r(160,52,140,28),!_transferBusy);
            _gameButtons.add(907,_transferSettings?"HUD / KEYS: YES":"HUD / KEYS: NO",r(310,52,140,28),!_transferBusy);
            _gameButtons.add(910,"CLOSE",r(160,16,140,28),!_transferBusy);
        }else{
            _gameButtons.text(_transferStatus,r(16,88,428,174),12*s,false);
            _gameButtons.text("CURRENT SAVE WILL BE BACKED UP",r(12,60,436,18),11*s,true);
            _gameButtons.add(911,"RESTORE",r(74,18,150,32),!_transferBusy);
            _gameButtons.add(912,"CANCEL",r(236,18,150,32),!_transferBusy);
        }
    }else if(_hudEditing){
        const double w=154,h=32,gap=3,x=safe.x+safe.width-w-6,top=safe.y+safe.height-6;
        _gameButtons.add(805,"TOOLS",{x,top-h,w,h},!_hudSaving);
        if(!_hudHideGuides){
            _gameButtons.panel({x,top-268,w,232});
            _gameButtons.text(_hudEditorText,{x+5,top-82,w-10,42},10,false);
            _gameButtons.text(_hudToolFocus?"F6: SELECT HUD":"F6: SELECT TOOLS",{x+5,top-96,w-10,12},9,true);
            const int ids[]={803,804,800,801,802};
            const std::string names[]={"PREVIEW",_hudExample?"CURRENT SCENE":"SAMPLE SCENE","RESET LAYOUT","CANCEL","APPLY"};
            for(int i=0;i<5;++i)_gameButtons.add(ids[i],names[i],{x,top-132-i*(h+gap),w,h},!_hudSaving);
        }
    }else if(_keyboardSettingsOpen){
        double w=std::min(safe.width-12,460.0),h=std::min(safe.height-12,300.0),x=safe.x+(safe.width-w)/2,y=safe.y+(safe.height-h)/2;
        _gameButtons.panel({x,y,w,h});_gameButtons.text("KEY BINDINGS",{x+8,y+h-24,w-16,18},15,true);
        static const char* names[]={"MOVE LEFT","MOVE RIGHT","AIM LEFT","AIM RIGHT","AIM UP","AIM DOWN","FIRE","JUMP","PREVIOUS WEAPON","NEXT WEAPON","FINE CONTROL","CONFIRM","PAUSE / BACK","NEXT BUTTON","KEY SETTINGS","NEXT TARGET"};
        double row=(h-80)/8,bw=(w-24)/2;
        for(int i=0;i<16;++i){std::string label=std::string(i==int(_settingsSelection)?"> ":"")+names[i]+": "+keyName(_keyboardControls.bindings[i]);_gameButtons.add(1000+i,label,{x+8+(i/8)*(bw+8),y+h-30-(i%8+1)*row,bw,row-2},!_capturingKey);}
        _gameButtons.text(_capturingKey?"PRESS A KEY - ESC CANCELS":"SELECT AN ACTION TO CHANGE ITS KEY",{x+8,y+35,w-16,14},10,true);
        _gameButtons.add(1020,"RESET KEYS",{x+8,y+5,bw,27},!_capturingKey);_gameButtons.add(1021,"CLOSE",{x+16+bw,y+5,bw,27},true);
    }else if(transferAvailable()){
        _gameButtons.add(900,"TRANSFER PROGRESS",{_canvasWidth>480?safe.x+(safe.width-174)/2:113,_canvasWidth>480?safe.y+4:3,174,36},_transferReady);
    }
    _gameButtons.draw([&](const char* image,R source,R dest){_media->drawRegion(image,makeRect(source.x,source.y,source.width,source.height),makeRect(dest.x,dest.y,dest.width,dest.height));});
}
void GameView::gameUIAction(int id,int state){
    if(state!=2)return;
    if(id==1030){hm_storage_retry();return;}
    _media->sound("UI_Click");
    if(id>=800&&id<=805){hudEditorAction(id-800);return;}
    if(id>=900&&id<=912){if(id==900&&(!_transferReady||!transferAvailable()))return;hm_transfer_action(id);return;}
    if(_keyboardSettingsOpen){
        if(id>=1000&&id<1016){_settingsSelection=id-1000;_capturingKey=true;}
        else if(id==1020){_keyboardControls.bindings=KeyboardInput::defaults;saveKeyboardBindings();}
        else if(id==1021)toggleKeyboardSettings();
        refreshKeyboardHelp();
    }
}
std::string GameView::gameUIDebug() const{
    std::ostringstream out;out<<"{\"transfer\":"<<_transferMode<<",\"busy\":"<<int(_transferBusy)<<",\"ready\":"<<int(_transferReady)<<",\"keys\":"<<int(_keyboardSettingsOpen)<<",\"keySelection\":"<<_settingsSelection<<",\"capturing\":"<<int(_capturingKey)<<",\"buttons\":[";
    bool first=true;for(const auto& b:_gameButtons.items()){if(!first)out<<',';first=false;out<<"{\"id\":"<<b.id<<",\"label\":"<<jsonString(b.label)<<",\"enabled\":"<<int(b.enabled)<<",\"rect\":["<<b.bounds.x<<','<<b.bounds.y<<','<<b.bounds.width<<','<<b.bounds.height<<"]}";}
    return out.str()+"],\"graphics\":"+jsonString(_glRenderer?_glRenderer->diagnostics():std::string())+"}";
}
}
