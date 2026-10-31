// C++ translation of Sources/iOS/HMGameHUD.mm (+ HMGameHUD.h) for the
// Emscripten build. Statement order, numeric constants and float/double types
// follow HMGameHUD.mm line by line; former ivars live in GameHUD::State.
// Objective-C ivars are zero-initialised by the runtime, so every State member
// that HMGameHUD did not set in -initWithMedia: is value-initialised to zero.
#include "WebGameHUD.hpp"
#include "WebMedia.hpp"
#include "HMButtonState.hpp"
#include <map>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numbers>
#include <string>
#include <vector>

namespace hmweb {
namespace {
// QobButton 0x87f8 rejects <= minimum and > maximum, in binary32.
// CGRectContainsPoint instead includes minimum and excludes maximum.
bool originalButtonContains(Rect bounds,Point point) {
    const float x=float(point.x),y=float(point.y);
    return x>float(minX(bounds))&&x<=float(maxX(bounds))&&
           y>float(minY(bounds))&&y<=float(maxY(bounds));
}
struct HUDButton {int identifier,key;Rect bounds;};
// unichar is uint16_t; the HUD only formats ASCII digits and separators.
int glyph(char16_t c) {return c>='0'&&c<='9'?c-'0':c=='.'?10:c=='/'?11:c==':'?12:c=='-'?13:-1;}
// [NSString stringWithFormat:] with %d / %02d / %d-%d produces the same ASCII
// text as snprintf for int arguments.
std::string format(const char *pattern,int a) {
    char buffer[32];std::snprintf(buffer,sizeof buffer,pattern,a);return buffer;
}
std::string format(const char *pattern,int a,int b) {
    char buffer[32];std::snprintf(buffer,sizeof buffer,pattern,a,b);return buffer;
}
}

struct GameHUD::State {
    GameHUD& _owner;
    Media& _media;
    heavymach::hud::Configuration _layout;
    int _layoutGroup=0;
    bool _editingLayout=false;
    int _keyboardFocus=0;
    bool _keyboardFocusVisible=false;
    std::vector<HUDButton> _buttons;
    std::map<int,heavymach::HMButtonState> _buttonStates;
    int _pressed=0,_menuStep=0,_upgradeStep=0;
    bool _modal=false,_upgradeVisible=false;
    double _time=0;
    // 0x169b0, 0x1767c, 0x178f0 store g_time after truncdfsf.
    float _startTime=0,_reloadTime=0,_weaponTime=0;
    float _hp=0,_hpDest=0,_hpRotation=0,_gear=0,_gearVelocity=0;
    std::array<float,4> _menuX{};
    float _upgradeX=0;
    std::array<uint8_t,3> _upgradeGrades{};

    State(GameHUD& owner,Media& media):_owner(owner),_media(media) {
        _layoutGroup=-1;_pressed=-1;_startTime=_reloadTime=_weaponTime=-1;_menuX.fill(-240);_hpDest=0;_upgradeX=-240;
    }
    void useLayoutGroup(int group) {
        _layoutGroup=group;
        if(!_layout.customized())return;
        auto t=group<0?_layout.legacy:_layout.group(group).transform;
        _media.setDrawingOffset(makePoint(t.offset.x,t.offset.y));_media.setDrawingScale(t.scale);
        if(group<0){auto r=_layout.legacy.apply(heavymach::hud::Rect{0,0,480,320});_media.setClipRect(makeRect(r.x,r.y,r.width,r.height));}
        else {auto r=_layout.group(group).visualBounds;_media.setClipRect(makeRect(r.x,r.y,r.width,r.height));}
    }
    void drawLayoutBackground() {
        using namespace heavymach::hud;
        if(!_layout.customized()){image("GameUI_Btm.png",240,32);image("GameUI_Top.png",240,288);return;}
        const int ids[]={jump,weapon,gauges,aim,progress,menu};
        const hmweb::Rect sources[]={makeRect(0,0,72,64),makeRect(72,0,88,64),makeRect(160,0,174,64),makeRect(334,0,146,64),makeRect(0,0,102,27),makeRect(0,27,76,37)};
        for(int n=0;n<6;++n){int g=ids[n];useLayoutGroup(g);hmweb::Rect src=sources[n];bool top=n>=4;
            hmweb::Rect dest=makeRect(src.origin.x,top?320-maxY(src):64-maxY(src),src.size.width,src.size.height);
            _media.drawRegion(top?"GameUI_Top.png":"GameUI_Btm.png",src,dest);
            // Existing riveted vertical metal border closes the exposed rail ends.
            if(n<4&&_layout.positions[g]){for(double x:{minX(dest),maxX(dest)-4})_media.drawRegion("GameUI_Top.png",makeRect(0,0,4,27),makeRect(x,0,4,19));}
            if(g==progress&&_layout.positions[g])_media.drawRegion("GameUI_Top.png",makeRect(0,0,4,27),makeRect(98,293,4,27));
        }
        // Remaining upper artwork belongs to a movable decoration, never a game receiver.
        useLayoutGroup(upperTrim);
        _media.drawRegion("GameUI_Top.png",makeRect(102,0,378,27),makeRect(102,293,378,27));
        _media.drawRegion("GameUI_Top.png",makeRect(76,27,404,37),makeRect(76,256,404,37));
        useLayoutGroup(-1);
    }

    void sprite(const std::string& name,double x,double y,int columns,int rows,int tile,double sx,double sy,double rotation,double alpha,int blend) {
        // Tile2D clamps oversized indices; this is observable at maximum upgrades.
        _media.draw(name,x,y,columns,rows,std::clamp(tile,0,columns*rows-1),sx,sy,rotation,alpha,blend);
    }
    void image(const std::string& name,double x,double y) {
        sprite(name,x,y,1,1,0,1,1,0,1,0);
    }
    void font(const std::string& name,const std::string& text,double x,double y,double pitch,double align,int columns,int rows,double alpha) {
        // NSString.length is NSUInteger; size_t keeps the same unsigned arithmetic.
        double left=x-pitch*(text.size()?text.size()-1:0)*align;
        for (size_t i=0;i<text.size();++i) {
            int tile=glyph(char16_t(static_cast<unsigned char>(text[i])));
            if(tile>=0)sprite(name,left+i*pitch,y,columns,rows,tile,1,1,0,alpha,2);
        }
    }
    void buttonImage(const std::string& name,int identifier,double x,double y,int columns,int rows,int tile,bool enabled) {
        // Result button1000 and booster1000 are distinct original QobButton objects.
        int key=identifier+(name=="StageClearBtn.png"?10000:0);
        auto& motion=_buttonStates[key];
        motion.pushDepth=motion.releaseHeight=motion.releaseDelay=0;motion.pushDelay=.1f;
        if(identifier==103){motion.pushDepth=1;motion.pushDelay=motion.releaseDelay=.01f;}
        if(identifier>=1200&&identifier<=1202)motion.releaseDelay=.1f;
        if(identifier==1300){motion.pushDepth=motion.releaseHeight=1;motion.releaseDelay=.1f;}
        int frame=motion.pressed&&enabled?tile+1:tile;
        sprite(name,x,y+motion.yOffset(_time,enabled),columns,rows,frame,1,1,0,1,0);
        if(enabled){Size z=_media.imageSize(name);if(z.width>0){Rect b=makeRect(x-z.width/columns/2,y-z.height/rows/2,z.width/columns,z.height/rows);
            if(_layoutGroup>=0&&_layout.customized()){auto t=_layout.group(_layoutGroup).transform;auto q=_layout.legacy.inverse(t.apply(heavymach::hud::Point{b.origin.x,b.origin.y}));b.origin=makePoint(q.x,q.y);}
            _buttons.push_back({identifier,key,b});}}
    }
    // Compose SET GAME UI from the original menu's small-cap letters. Both
    // states sample the existing atlas; no new font, bitmap or per-frame rasterization.
    void drawHUDLayoutButton(double x,double y) {
        auto& motion=_buttonStates[104];
        motion.pushDepth=motion.releaseHeight=motion.releaseDelay=0;motion.pushDelay=.1f;
        const int stateY=motion.pressed?40:0;
        const double bottom=y-20+motion.yOffset(_time,true);
        struct Letter {int x,y,width;};
        // S, E, T, space, G, A, M, E, space, U, I.
        // VISIT is one pixel lower in its atlas row; align its T/I and plate edges.
        constexpr Letter letters[]={{25,80,16},{66,0,13},{68,161,13},{0,0,8},
            {146,0,17},{163,0,15},{178,0,16},{195,0,12},{0,0,8},{93,0,15},{88,161,7}};
        double width=0;for(auto letter:letters)width+=letter.width+1;
        double left=x-(width-1)/2;
        // Tile the blank plate around the letters, never beneath their translucent
        // pixels. Overdrawing a plate would darken every letter rectangle twice.
        auto blank=[&](double origin,double length){if(length>0)_media.drawRegion("GameMenuSub.png",makeRect(20,stateY,1,40),makeRect(origin,bottom,length,40));};
        _media.drawRegion("GameMenuSub.png",makeRect(0,stateY,20,40),makeRect(x-128,bottom,20,40));
        blank(x-108,left-(x-108));
        for(auto letter:letters){
            if(letter.x)_media.drawRegion("GameMenuSub.png",makeRect(letter.x,letter.y+stateY,letter.width,40),makeRect(left,bottom,letter.width,40));
            else blank(left,letter.width);
            left+=letter.width;blank(left,1);++left;
        }
        blank(left,x+108-left);
        _media.drawRegion("GameMenuSub.png",makeRect(236,stateY,20,40),makeRect(x+108,bottom,20,40));
        _buttons.push_back({104,104,makeRect(x-128,y-20,256,40)});
    }
    void drawPassive(const GameHUDSnapshot& s) {
        if(!s.passives)return;
        for(int i=0;i<4;++i)if(_editingLayout||s.passives->slots[i].active){
            sprite("ItemTile.png",10,200+i*16,7,4,i+5,.5,.5,0,1,0);
            font("NumSet02b.png",format("%02d",(s.passives->slots[i].active?s.passives->slots[i].number:999)),24,200+i*16,6,0,16,1,1);
        }
    }
    void drawWeaponPreview(const GameHUDSnapshot& s) {
        if(!s.slot||!s.data||(_weaponTime<0&&!_editingLayout))return;
        float elapsed=float(_time-_weaponTime),alpha=elapsed<=1?1:std::max(0.f,2.f-elapsed);
        if(_editingLayout)alpha=1;
        if(alpha<=0)return;
        int first=std::max(0,std::min(s.selectedWeapon-2,s.slot->weaponCount()-5));
        for(int i=0;i<5&&first+i<s.slot->weaponCount();++i){
            auto weapon=s.data->weapons.find(s.slot->weaponName(first+i));if(weapon==s.data->weapons.end())continue;
            double scale=first+i==s.selectedWeapon?1.4:1;
            sprite("WeaponTile.png",196+i*43,290,7,4,weapon->second.uiTile,scale,scale,0,alpha,0);
            if(first+i==s.selectedWeapon)sprite("SelWeapon.png",196+i*43,290,1,1,0,1,1,0,alpha,2);
        }
    }
    void drawMissionIntro(const GameHUDSnapshot& s) {
        if(s.mode!=0||_startTime<0)return;
        float elapsed=float(_time-_startTime);if(elapsed<0||elapsed>=4)return;
        float alpha=elapsed<1?elapsed*.5f:elapsed>=3?.5f-(elapsed-3.f)*.5f:.5f;
        sprite("Text_Mission.png",190,250,1,1,0,1,1,0,alpha,2);
        font("NumSet_Mission.png",format("%d-%d",s.mission+1,s.stage+1),290,250,16,0,8,2,alpha);
    }
    void drawUpgrade(const GameHUDSnapshot& s) {
        if(_upgradeStep==0)return;
        image("Upgrade_BG.png",_upgradeX,180);
        if(s.upgradeSelection>=0&&s.upgradeSelection<3)
            sprite("Upgrade_Sel.png",_upgradeX+101*s.upgradeSelection-101,172,1,2,1,1,1,0,1,0);
        const std::array<std::string,3> names={"Upgrade_Body.png","Upgrade_Top.png","Upgrade_Weapon.png"};
        for(int i=0;i<3;++i){
            sprite(names[i],_upgradeX+100*(i-1),196,1,7,_upgradeGrades[i]+1,1,1,0,1,0);
            if(_upgradeGrades[i]<6)buttonImage("Upgrade_Sel.png",1200+i,_upgradeX+101*(i-1),172,1,2,0,true);
            for(int grade=0;grade<=_upgradeGrades[i]&&grade<7;++grade)
                sprite("Upgrade_Gauge.png",_upgradeX-127+101*i+grade*9,135,7,1,grade,1,1,0,1,0);
        }
        if(s.upgradeSelection>=0&&s.upgradeSelection<3){
            int next=_upgradeGrades[s.upgradeSelection]+1;
            sprite("Upgrade_Gauge.png",_upgradeX-127+101*s.upgradeSelection+next*9,135,7,1,next,1,1,0,1,0);
        }
        buttonImage("Common_Btn_OK.png",1300,_upgradeX+54,95,1,2,0,true);
    }
    void drawPresentation(const GameHUDSnapshot& s) {
        if(!s.presentation||s.presentation->step==7)return;
        const auto& p=*s.presentation;
        if(!p.loadingAsset.empty()){
            // stringWithUTF8String: stops at the first NUL, as c_str() does.
            const std::string asset(p.loadingAsset.c_str());
            // 0xb8f0 / 0xbf74 pass the original JPEG name to ResMgr_Tile.
            // HMMedia reads OriginalImages directly; no derived PNG suffix.
            if(s.canvasWidth>480||s.canvasHeight>320){
                // The illustration covers the world, independently of the safe-area HUD.
                // Preserve aspect ratio; only the illustration may crop vertically.
                Size size=_media.imageSize(asset);
                // LINEAR/REPEAT sprite textures have zero POT padding. Keep the
                // screen inside the outer texel centres so padding cannot darken its edges.
                double scale=size.width>1&&size.height>1?std::max(s.canvasWidth/(size.width-1),s.canvasHeight/(size.height-1)):1;
                _media.resetClip();_media.setDrawingOffset(Point{});_media.setDrawingScale(1);
                sprite(asset,s.canvasWidth/2,s.canvasHeight/2,1,1,0,scale,scale,0,p.alpha,0);
                _media.setDrawingOffset(s.hudOffset);_media.setDrawingScale(s.hudScale);
                _media.setClipRect(makeRect(s.hudOffset.x,s.hudOffset.y,480*s.hudScale,320*s.hudScale));
            }else sprite(asset,240,160,1,1,0,1,1,0,p.alpha,0);
        }
        sprite("ResultTypo.png",180,280,1,5,p.typoFrame,p.scale,p.scale,p.rotation,p.typoAlpha,2);
        // Parent loading image (240,160), button base (0,buttonY),
        // child centers (-120,-134)/(140,-134), from 0xb8f0.
        if(p.typoFrame==0||p.typoFrame==4){
            buttonImage("StageClearBtn.png",1000,120,26+p.buttonY,1,4,0,p.step==2);
            if(!p.missionEnd)buttonImage("StageClearBtn.png",1001,380,26+p.buttonY,1,4,2,p.step==2);
        }
    }
    void draw(const GameHUDSnapshot& s) {
        _time=s.time;_buttons.clear();
        bool result=s.presentation&&s.presentation->step!=7;
        _modal=s.paused||_menuStep||_upgradeStep||result;
        useLayoutGroup(heavymach::hud::passives);drawPassive(s);useLayoutGroup(-1);if(!_editingLayout)drawMissionIntro(s);useLayoutGroup(heavymach::hud::preview);drawWeaponPreview(s);useLayoutGroup(-1);
        drawLayoutBackground();
        useLayoutGroup(heavymach::hud::menu);
        buttonImage("GameMenu.png",100,38,279,1,2,0,true);
        useLayoutGroup(heavymach::hud::progress);
        sprite("GameUI_Exp.png",27+35*s.expGauge,300,1,1,0,s.expGauge,1,0,1,0);
        font("NumSet01.png",format("%02d",s.slot?s.slot->level()+1:1),40,313,7,0,16,1,1);
        useLayoutGroup(heavymach::hud::gauges);
        sprite("GameUI_Gauge.png",239,25,1,1,0,1,1,_hpRotation,1,0);
        float reloadElapsed=float(_time-_reloadTime);
        if(_reloadTime>=0&&reloadElapsed<.15f)sprite("GameUI_BlueLight.png",287,22,1,1,0,1,1,0,1.f+reloadElapsed/-.15f,2);
        sprite("GameUI_Gauge.png",287,22,1,1,0,1,1,s.reloadGauge*2*std::numbers::pi,1,0);
        sprite("GameUI_RedLight.png",265,35,1,1,0,1,1,0,std::min(1.f,std::cos(float(10*_time))+1.f),2);
        sprite("GameUI_Gear.png",324,3,1,1,0,1,1,_gear,1,0);
        useLayoutGroup(heavymach::hud::jump);
        buttonImage("Btn_Booster.png",1000,34,34,2,1,0,true);
        useLayoutGroup(heavymach::hud::weapon);
        buttonImage("Btn_Weapon.png",1100,103,31,2,1,0,true);
        sprite("WeaponTile.png",106,35,7,4,s.weapon?s.weapon->uiTile:0,1,1,0,1,0);
        font("NumSet02b.png",format("%02d",s.ammo),120,26,6,1,16,1,1);
        useLayoutGroup(-1);
        if(_editingLayout)return;
        drawUpgrade(s);drawPresentation(s);
        // An interrupted stage transition must leave its resume menu visible.
        if(_menuStep){
            buttonImage("GameMenuSub.png",101,_menuX[0],235,1,6,0,true);
            drawHUDLayoutButton(_menuX[1],190);
            buttonImage("GameMenuSub.png",102,_menuX[2],145,1,6,2,true);
            buttonImage("GameMenuSub.png",103,_menuX[3],100,1,6,4,true);
        }
    }
    void tick(double time) {
        _time=time;_gear+=_gearVelocity;_gearVelocity+=(0-_gearVelocity)/20;
        // 0x151a0 leaves the image at rotation zero. 0x161d0 updates it only
        // when HP actually changes, then stores the result as binary32.
        if(_hpDest!=_hp){
            _hp+=(_hpDest-_hp)*.125f;if(std::abs(_hp-_hpDest)<.01f)_hp=_hpDest;
            _hpRotation=float(double(_hp)*-5.183627803521752-0.6000000238418579);
        }
        if(_menuStep)for(int i=0;i<4;++i)_menuX[i]+=((_menuStep==1?220+20*i:720)-_menuX[i])/(7+2*i);
        if(_menuStep==2&&_menuX[3]>710)_menuStep=0;
        if(_upgradeStep){_upgradeX+=((_upgradeStep==1?240:660)-_upgradeX)/5;
            if(_upgradeStep==2&&_upgradeX>659){_upgradeStep=0;if(_owner.upgradeClosed)_owner.upgradeClosed();}}
    }
    int receiverAt(Point logical) {
        for(auto i=_buttons.rbegin();i!=_buttons.rend();++i)if(originalButtonContains(i->bounds,logical))return i->key;
        return -1;
    }
    bool touch(Point logical,int state) {return touch(logical,state,-1);}
    bool touch(Point logical,int state,int receiver) {
        // 0x82b8 registers buttons at construction; 0xfc18 walks that array in
        // reverse and stops at the first hit. Drawing order is not receiver order.
        auto rank=[](const HUDButton& button){
            if(button.key>=10000)return 10+button.identifier-1000;
            if(button.identifier==100)return 0;
            if(button.identifier>=101&&button.identifier<=103)return button.identifier-100;
            if(button.identifier==1000)return 4;
            if(button.identifier==1100)return 5;
            if(button.identifier>=1200&&button.identifier<=1202)return 6+button.identifier-1200;
            return 9;
        };
        auto receivers=_buttons;
        std::stable_sort(receivers.begin(),receivers.end(),[&](const auto& a,const auto& b){return rank(a)<rank(b);});
        for(auto it=receivers.rbegin();it!=receivers.rend();++it){
            if(receiver>=0&&it->key!=receiver)continue;
            bool inside=originalButtonContains(it->bounds,logical);
            _buttonStates[it->key].touch(inside,state,_time);
            if(!inside)continue;
            if((state==0||state==2)&&_owner.button)_owner.button(it->identifier,state);
            return true;
        }
        // GuiGame inherits QobBase.onTap (0x81b4), which does not consume empty space.
        return false;
    }
    std::vector<HUDButton> keyboardButtons() {
        auto buttons=_buttons;
        buttons.erase(std::remove_if(buttons.begin(),buttons.end(),[&](const auto& b){
            return midX(b.bounds)<0||midX(b.bounds)>480||midY(b.bounds)<0||midY(b.bounds)>320;
        }),buttons.end());
        std::stable_sort(buttons.begin(),buttons.end(),[](const auto& a,const auto& b){
            if(std::fabs(midY(a.bounds)-midY(b.bounds))>8)return midY(a.bounds)>midY(b.bounds);
            return midX(a.bounds)<midX(b.bounds);
        });
        return buttons;
    }
    void moveKeyboardFocus(int delta) {
        auto buttons=keyboardButtons();if(buttons.empty())return;
        int index=-1;for(size_t i=0;i<buttons.size();++i)if(buttons[i].key==_keyboardFocus)index=int(i);
        if(index<0)index=delta<0?int(buttons.size())-1:0;else index=(index+delta+int(buttons.size()))%int(buttons.size());
        _keyboardFocus=buttons[index].key;_keyboardFocusVisible=true;
    }
    Rect keyboardFocusRect() {
        if(!_keyboardFocusVisible)return Rect{};
        auto buttons=keyboardButtons();for(const auto& b:buttons)if(b.key==_keyboardFocus)return b.bounds;
        if(buttons.empty())return Rect{};_keyboardFocus=buttons.front().key;return buttons.front().bounds;
    }
    void activateKeyboardFocus() {
        _keyboardFocusVisible=true;Rect rect=keyboardFocusRect();if(isEmpty(rect))return;
        Point point=makePoint(midX(rect),midY(rect));touch(point,0);touch(point,2);
    }
};

GameHUD::GameHUD(Media& media):s(std::make_unique<State>(*this,media)) {}
GameHUD::~GameHUD()=default;
void GameHUD::setLayout(const heavymach::hud::Configuration& layout,bool editing) {s->_layout=layout;s->_editingLayout=editing;}
void GameHUD::setGameStart(double time) {s->_startTime=float(time);}
void GameHUD::setReloadComplete(double time) {s->_reloadTime=float(time);}
void GameHUD::showWeaponChange(double time) {s->_weaponTime=float(time);}
std::optional<int> GameHUD::changeWeaponForSlot(heavymach::SaveSlot* slot,int selectedWeapon,
                                                heavymach::SaveSlot* itemSlot,int itemIndex,double time) {
    // GuiGame.changeWeapon 0x178f0 first guards GlobalInfo.slotData. The item
    // pointer is independent of that pointer and persists through screen changes.
    if(!slot)return std::nullopt;
    showWeaponChange(time);
    if(!itemSlot||itemIndex<0||size_t(itemIndex)>=heavymach::SaveSlot::weaponCapacity)return std::nullopt;
    const size_t chargeOffset=0x4c+size_t(itemIndex)*0x1c;
    if(uint16_t(selectedWeapon)==0)itemSlot->setInt(chargeOffset,99);
    return itemSlot->i32(chargeOffset);
}
void GameHUD::setHPGauge(float value) {s->_hpDest=value;}
void GameHUD::setReloadGauge(float value) {
    (void)value;
    // 0x1762c: every reload event restarts the gear, including repeated values.
    s->_gearVelocity=.15f;
}
void GameHUD::openUpgrade(const heavymach::SaveSlot& slot) {
    // 0x16a98 recreates the panel synchronously, before GuiGame.tick.
    // 0x16a98 removes the prior panel and constructs new QobButton instances.
    for(int identifier:{1200,1201,1202,1300})s->_buttonStates.erase(identifier);
    s->_upgradeVisible=true;s->_upgradeStep=1;s->_upgradeX=-240;
    s->_upgradeGrades={slot.bytes[0x2f],slot.bytes[0x2e],slot.bytes[0x2d]};
}
void GameHUD::closeUpgrade() {s->_upgradeVisible=false;s->_upgradeStep=2;}
void GameHUD::setMenuVisible(bool visible) {
    s->_menuStep=visible?1:2;if(visible)s->_menuX.fill(-240);
}
void GameHUD::draw(const GameHUDSnapshot& snapshot) {s->draw(snapshot);}
void GameHUD::tick(double time) {s->tick(time);}
void GameHUD::cancelInput() {s->_pressed=-1;s->_buttonStates.clear();}
int GameHUD::receiverAt(Point logical) {return s->receiverAt(logical);}
bool GameHUD::touch(Point logical,int state) {return s->touch(logical,state);}
bool GameHUD::touch(Point logical,int state,int receiver) {return s->touch(logical,state,receiver);}
void GameHUD::moveKeyboardFocus(int delta) {s->moveKeyboardFocus(delta);}
void GameHUD::activateKeyboardFocus() {s->activateKeyboardFocus();}
Rect GameHUD::keyboardFocusRect() {return s->keyboardFocusRect();}
}
