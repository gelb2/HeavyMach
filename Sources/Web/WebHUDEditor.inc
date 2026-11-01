// {"version":1,"positions":[null|[x,y],...]} -> out[2*i],out[2*i+1]; null -> NaN.
EM_JS(int,hm_parse_hud_layout,(const char* text,double* out,int capacity),{
    try{
        const o=JSON.parse(UTF8ToString(text));
        if(!o||o.version!==1||!Array.isArray(o.positions)||o.positions.length>capacity)return -1;
        const base=out>>3;
        for(let i=0;i<o.positions.length;i++){
            const p=o.positions[i];
            if(p===null){HEAPF64[base+2*i]=NaN;HEAPF64[base+2*i+1]=NaN;continue;}
            if(!Array.isArray(p)||p.length!==2||typeof p[0]!=='number'||typeof p[1]!=='number')return -1;
            HEAPF64[base+2*i]=p[0];HEAPF64[base+2*i+1]=p[1];
        }
        return o.positions.length;
    }catch(e){return -1;}
});
namespace hmweb {
namespace {
std::string jsonString(const std::string& text){
    std::string out="\"";
    for(unsigned char c:text){
        if(c=='"'||c=='\\'){out+='\\';out+=char(c);}
        else if(c=='\n')out+="\\n";
        else if(c<0x20){char hex[8];std::snprintf(hex,sizeof(hex),"\\u%04x",c);out+=hex;}
        else out+=char(c);
    }
    return out+"\"";
}

}
heavymach::hud::Configuration GameView::currentHUDLayout() const{
    auto c=_hudEditing?_hudDraft:_hudLayout;c.legacy={{_hudOffset.x,_hudOffset.y},_hudScale};
    c.safe={_menuSafeRect.origin.x,_menuSafeRect.origin.y,_menuSafeRect.size.width,_menuSafeRect.size.height};
    if(!_hudEditing&&!heavymach::hud::valid(c,false))c.positions={};return c;
}
// iOS: Documents/HUDLayoutV1-<ipad|iphone|mac>.json beside HvM.dat. The browser
// keeps one layout per origin and browser storage: HUDLayoutV1-web.json.
std::string GameView::hudLayoutPath() const{
    return std::filesystem::path(_savePath).parent_path().string()+"/HUDLayoutV1-web.json";
}
void GameView::loadHUDLayout(){
    std::ifstream file(hudLayoutPath(),std::ios::binary);if(!file)return;
    std::string text((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    std::array<double,2*heavymach::hud::groupCount> values{};
    const int count=hm_parse_hud_layout(text.c_str(),values.data(),heavymach::hud::groupCount);
    if(count!=9&&count!=heavymach::hud::groupCount)return;
    auto candidate=_hudLayout;candidate.positions={};
    for(int i=0;i<count;++i){
        double x=values[2*i],y=values[2*i+1];if(std::isnan(x)&&std::isnan(y))continue;
        if(!std::isfinite(x)||!std::isfinite(y)||x<0||x>1||y<0||y>1)return;
        candidate.positions[i]=heavymach::hud::Placement{x,y};
    }
    _hudLayout=candidate;
}
void GameView::prepareHUDEditor(){_hudSelected=heavymach::hud::jump;}
void GameView::updateHUDEntry(){_hudEntryHidden=true;}
void GameView::updateHUDEditorGeometry(){
    bool changed=_hudDraft.safe.x!=_menuSafeRect.origin.x||_hudDraft.safe.y!=_menuSafeRect.origin.y||_hudDraft.safe.width!=_menuSafeRect.size.width||_hudDraft.safe.height!=_menuSafeRect.size.height||_hudDraft.legacy.scale!=_hudScale;
    // Editing must not change the live session's render cache on resize.
    if(_hudEditing&&(_hudDraft.safe.width!=_menuSafeRect.size.width||_hudDraft.safe.height!=_menuSafeRect.size.height)){
        _hudDragToken.reset();_hudExample=true;_hudExampleSession.reset();
    }
    _hudLayout.legacy={{_hudOffset.x,_hudOffset.y},_hudScale};_hudLayout.safe={_menuSafeRect.origin.x,_menuSafeRect.origin.y,_menuSafeRect.size.width,_menuSafeRect.size.height};
    _hudLayoutFallback=!heavymach::hud::valid(_hudLayout,false);
    _hudDraft.legacy=_hudLayout.legacy;_hudDraft.safe=_hudLayout.safe;
    if(_hudEditing&&changed)layoutHUDControls();
}
void GameView::layoutHUDControls(){
    updateHUDEntry();if(_hudEditing)refreshHUDEditorLabel(nullptr);
    publishHUDControls();
}
void GameView::publishHUDControls(){}
void GameView::refreshHUDEditorLabel(const char* error){
    static const char* names[]={"MENU","LEVEL / EXP","JUMP","WEAPON / AMMO","GAUGES","AIM CONTROLS","PASSIVES","WEAPON PREVIEW","CHEAT CONTROLS","UPPER TRIM"};
    if(error){_hudEditorText=error;return;}
    _hudEditorText=std::string(_hudExample?"SAMPLE":"PAUSED")+"\n"+names[_hudSelected]+"\nDRAG TO MOVE";
}
void GameView::hudControl(int tag){
    if(!_active||!_rendererReady)return;
    if(tag<0)openHUDEditor();else hudEditorAction(tag);
}
void GameView::openHUDEditor(){
    if(_loading.visible()||_hudEditing||_showMenu||!(_session&&(_gameMenu||_hudLayoutFallback)&&_session->world().gameState==0))return;
    if(_session&&!_gameMenu){_gameMenu=true;_hud->setMenuVisible(true);}
    cancelGameInput();_hudFromMenu=_showMenu;_hudExample=_hudFromMenu;_hudDraft=_hudLayout;_hudEditing=true;_hudHideGuides=false;_hudDragToken.reset();
    if(_session)_session->setPaused(true);
    layoutHUDControls();
}
void GameView::closeHUDEditor(){
    for(long token:_hudEditorTouches)_gameButtons.consumeTouch(token,0);
    _hudEditorTouches.clear();
    _hudEditing=false;_hudDragToken.reset();_hudExampleSession.reset();_hudExampleData={};_media->releaseScope("hud-example");
    cancelGameInput();noteInteraction();
    layoutHUDControls();
    // Return to the pause menu after editing.
}
void GameView::hudEditorAction(int tag){
    if(!_hudEditing||_hudSaving)return;
    switch(tag){
      case 0:_hudDraft.positions={};_hudDragToken.reset();break;
      case 1:closeHUDEditor();return;
      case 2:{auto c=currentHUDLayout();
        if(!heavymach::hud::valid(c,false)){refreshHUDEditorLabel("Move overlapping buttons inside the screen.");publishHUDControls();return;}
        if(!_hudDraft.samePositions(_hudLayout)){
            std::string json="{\"version\":1,\"positions\":[";
            for(int i=0;i<heavymach::hud::groupCount;++i){
                if(i)json+=",";const auto& p=_hudDraft.positions[i];
                if(!p){json+="null";continue;}
                char value[80];std::snprintf(value,sizeof(value),"[%.17g,%.17g]",p->x,p->y);json+=value;
            }
            json+="]}";
            // NSDataWritingAtomic: temporary file, then rename; IDBFS sync like HvM.dat.
            const std::string path=hudLayoutPath(),temporary=path+".tmp";
            bool written=false;
            {std::ofstream file(temporary,std::ios::binary|std::ios::trunc);file.write(json.data(),std::streamsize(json.size()));written=bool(file);}
            std::error_code error;if(written)std::filesystem::rename(temporary,path,error);
            if(!written||error){refreshHUDEditorLabel("Could not save. Try APPLY again.");publishHUDControls();return;}
            _hudSaving=true;refreshHUDEditorLabel("Saving...");publishHUDControls();
            hm_sync_hud_save();return;
        }
        closeHUDEditor();return;
      }
      case 3:_hudHideGuides=true;break;
      case 4:if(!_hudFromMenu)_hudExample=!_hudExample;break;
      case 5:_hudHideGuides=!_hudHideGuides;break;
    }
    layoutHUDControls();
}
bool GameView::hudEditorKey(unsigned code,bool pressed){
    if(!pressed||_hudSaving)return true;
    if(code==0x29){closeHUDEditor();return true;}
    if(code==0x2b){do{_hudSelected=(_hudSelected+1)%heavymach::hud::groupCount;}while(_hudSelected==heavymach::hud::cheat);_hudHideGuides=false;layoutHUDControls();return true;}
    auto c=currentHUDLayout();auto r=c.group(_hudSelected).visualBounds;double dx=code==0x4f?1:code==0x50?-1:0,dy=code==0x52?1:code==0x51?-1:0;
    if(dx||dy){_hudDraft.positions[_hudSelected]=heavymach::hud::placementForOrigin(c.localBounds(_hudSelected),c.legacy,c.safe,{r.x+dx,r.y+dy});layoutHUDControls();}
    return true;
}
void GameView::hudEditorPoint(Point p,int state,long token){
    if(_hudSaving)return;
    auto c=currentHUDLayout();
    if(state==0&&!_hudDragToken){
        int hit=-1;
        for(int j=heavymach::hud::groupCount-1;j>=0;--j){int i=j==0?heavymach::hud::upperTrim:j-1;if(i==heavymach::hud::cheat)continue;auto r=c.group(i).visualBounds;
            if(r.contains({p.x,p.y},heavymach::hud::HitEdges::standard)){hit=i;_hudGrab={p.x-r.x,p.y-r.y};break;}
        }
        if(hit<0)return;
        _hudHideGuides=true;_hudSelected=hit;_hudDragToken=token;refreshHUDEditorLabel(nullptr);
    }
    if(!_hudDragToken||token!=*_hudDragToken)return;
    if(state<3&&p.x>=0&&p.y>=0){
        auto r=c.group(_hudSelected).visualBounds;double x=p.x-_hudGrab.x,y=p.y-_hudGrab.y;
        if(std::abs(x-r.x)>.01||std::abs(y-r.y)>.01){
            auto original=c.legacy.apply(c.localBounds(_hudSelected));
            if(std::abs(x-original.x)<.01&&std::abs(y-original.y)<.01)_hudDraft.positions[_hudSelected].reset();
            else _hudDraft.positions[_hudSelected]=heavymach::hud::placementForOrigin(c.localBounds(_hudSelected),c.legacy,c.safe,{x,y});
        }
    }
    if(state>=2)_hudDragToken.reset();
    layoutHUDControls();
}
void GameView::prepareHUDExample(){
    if(_hudExampleSession)return;
    _hudExampleData=_data;_hudExampleSlot={};GlobalInfo(_hudExampleSlot).selectSlot(1);_hudExampleRandom=12345;
    auto& map=_hudExampleData.maps.at("Stage01");map.enemies={{"Tank001",{map.left+240,100},false},{"Heli001",{map.left+330,190},false}};
    Media* media=_media.get();
    _hudExampleSession=std::make_unique<GameSession>(_hudExampleData,_hudExampleSlot,
        [media](std::string_view name){Size size=media->imageSize(std::string(name),"hud-example");return Vector2{float(size.width),float(size.height)};},
        [this]{uint32_t x=_hudExampleRandom;x^=x<<13;x^=x>>17;x^=x<<5;_hudExampleRandom=x;return int32_t(x&0x7fffffff);});
    _hudExampleSession->setCanvasWidth(float(_canvasWidth));_hudExampleSession->setCanvasHeight(float(_canvasHeight));_hudExampleSession->start(0,0,0);
    for(int i=1;i<=45;++i){_hudExampleSession->setPaused(false);_hudExampleSession->tick(i/60.,0);_hudExampleSession->takeEvents();}
    _hudExampleSession->prepareHUDPreview();
}
void GameView::drawHUDEditor(){
    if(_hudExample)prepareHUDExample();
    auto* scene=_hudExample?_hudExampleSession.get():_session.get();if(!scene)return;
    _media->setDrawingOffset({});_media->setDrawingScale(1);_media->resetClip();
    if(!_hudExample)_media->setCameraOffset({_worldShake.offset.x,_worldShake.offset.y});
    for(const auto& item:scene->renderItems()){
        if(const auto* sprite=std::get_if<DrawSprite>(&item)){
            _media->resourceScope=_hudExample?"hud-example":sprite->worldTexture?"world":"shared";
            _media->draw(sprite->tile,sprite->position.x,sprite->position.y,sprite->cols,sprite->rows,sprite->frame,sprite->scale.x,sprite->scale.y,sprite->rotation,sprite->alpha,sprite->blend);
        }else{_media->resourceScope=_hudExample?"hud-example":"shared";_media->drawTrail(std::get<TrailMesh>(item));}
    }
    _media->resourceScope="shared";_media->setCameraOffset({});_media->setDrawingOffset(_hudOffset);_media->setDrawingScale(_hudScale);
    _media->setClipRect(makeRect(_hudOffset.x,_hudOffset.y,480*_hudScale,320*_hudScale));
    GameHUDSnapshot s;s.slot=_hudExample?&_hudExampleSlot:&_save.slots[_slotIndex];s.data=&_data;s.passives=_hudExample?&_hudExamplePassives:&_passives;s.time=_time;s.ammo=99;s.expGauge=_expGauge;s.reloadGauge=_reloadGauge;
    s.canvasWidth=_canvasWidth;s.canvasHeight=_canvasHeight;s.hudOffset=_hudOffset;s.hudScale=_hudScale;s.selectedWeapon=scene->selectedWeapon();
    if(s.slot->weaponCount()>0){auto w=_data.weapons.find(s.slot->weaponName(s.selectedWeapon));if(w!=_data.weapons.end())s.weapon=&w->second;s.ammo=s.slot->weaponCharge(s.selectedWeapon);}
    _hud->setLayout(currentHUDLayout(),true);_hud->draw(s);drawAimControls();
    _media->resetClip();_media->setDrawingOffset({});_media->setDrawingScale(1);
}
}
