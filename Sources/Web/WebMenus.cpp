// C++ translation of Sources/iOS/HMMenus.mm. Kept line-for-line with the
// Objective-C++ implementation: constants, binary32 casts, receiver order and
// evaluation order are unchanged. ObjC ivars live in Menus::State with their
// original names; `self` is the owning Menus (for the former block properties).
#include "WebMenus.hpp"
#include "WebMedia.hpp"
#include "HMButtonState.hpp"
#include <map>
#include "GlobalInfo.hpp"
#include "Visuals.hpp"
#include "Motion.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <initializer_list>
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
struct MenuButton { int identifier; Rect bounds; };
// Original Vector2 / GuiSelectMission::_slotPos are binary32 (0x19ba0).
struct MenuPosition {float x,y;};
enum class MenuScreen { slots, mode, mission };
struct SlotPreview {
    bool exists=false;
    heavymach::GobHvMach motion;
    std::array<heavymach::PartsState,3> parts;
    heavymach::WeaponState weapon;
};
// [NSString stringWithFormat:@"<prefix>%02d<suffix>",value] for the formats used here.
std::string format02d(const char* prefix,int value,const char* suffix="") {
    char text[64]; std::snprintf(text,sizeof(text),"%s%02d%s",prefix,value,suffix);
    return text;
}
}

struct Menus::State {
    Menus& self;
    Media& _media;
    int _keyboardFocus=0;
    bool _keyboardFocusVisible=false;
    heavymach::SaveData *_save=nullptr;
    const heavymach::GameData *_data=nullptr;
    MenuScreen _screen=MenuScreen::slots;
    int _slot=0, _gameSlot=0, _mode=0, _mission=0, _stage=0, _top=0, _pressed=0;
    bool _deleteConfirmation=false, _slotSelectionMoved=false;
    std::array<MenuPosition,7> _positions{}, _targets{};
    std::vector<MenuButton> _buttons;
    std::map<int,heavymach::HMButtonState> _buttonStates;
    std::map<int,size_t> _receiverOrder;
    size_t _nextReceiverOrder=0;
    std::array<SlotPreview,3> _previews;
    std::array<std::vector<std::string>,3> _slotTexts;
    std::array<float,3> _slotGauges{};
    double _time=0, _previewWorldTime=0;
    bool _previewWorldPaused=false;
    double _canvasWidth=0;
    Rect _safeRect;
    Point _layoutOffset;
    bool _cancelledPressByLayout=false;

    State(Menus& owner,Media& media,heavymach::SaveData* save,const heavymach::GameData* data)
        : self(owner), _media(media) {
        _canvasWidth=480;_safeRect=makeRect(0,0,480,320); _save=save; _data=data; _pressed=-1; _slotTexts={};
    }
    void setCanvasWidth(double width,Rect safeRect) {
        if(_canvasWidth!=width||!equal(_safeRect,safeRect)) {
            // A resize cannot release an old press onto a newly positioned button.
            _pressed=-1; for(auto& entry:_buttonStates){if(entry.second.pressed)_cancelledPressByLayout=true;entry.second.touch(false,3,_time);}
            _buttons.clear();
        }
        _canvasWidth=width;_safeRect=safeRect;
    }
    void layoutOffset(Point offset) {_layoutOffset=offset;_media.setDrawingOffset(offset);}
    // Reuse exact original pixels through GL clipping; no new crop/bitmap per frame.
    void piece(const std::string& name,Rect source,Point offset) {
        layoutOffset(Point{});
        _media.setClipRect(hmweb::offset(source,offset.x,offset.y));
        _media.draw(name,240+offset.x,160+offset.y);
        _media.resetClip();
    }
    void wideBackdrop() {
        layoutOffset(Point{});
        Size size=_media.imageSize("Extensions/MenuMetal.png");
        double scale=std::max(_canvasWidth/size.width,320/size.height);
        _media.draw("Extensions/MenuMetal.png",_canvasWidth/2,160,1,1,0,scale,scale,0,1);
    }
    void connectedBackdrop(const std::string& extension,const std::string& original) {
        // Each screen has an outpaint of its own original. Discard the generated
        // centre; only the two outer quarters join the immutable 480px background.
        layoutOffset(Point{});
        Size size=_media.imageSize(extension);
        double center=midX(_safeRect),scale=320/size.height;
        double tileWidth=size.width*scale/4;
        for(int side:{-1,1}) {
            double extent=std::max(tileWidth,side<0?center-240:_canvasWidth-center-240);
            _media.draw(extension,center+side*(240+extent/2),160,4,1,side<0?0:3,extent/(size.width/4),scale,0,1);
        }
        // Blend only outside the original bounds, retaining its interior pixels.
        _media.drawEdgeContinuation(original,center,160,1,0,24);
        layoutOffset(makePoint(center-240,0));
    }
    double slotOffset(int i) {
        if(_canvasWidth<=480)return 0;
        double first=minX(_safeRect)+82,last=maxX(_safeRect)-85;
        return first+(last-first)*i/2-(85+155*i);
    }
    void setPreviewWorldTime(double time,bool paused) {_previewWorldTime=time;_previewWorldPaused=paused;}
    void showSlots() {
        _buttonStates.clear(); _buttons.clear();
        _receiverOrder.clear();_nextReceiverOrder=0;
        _receiverOrder[1000]=_nextReceiverOrder++;_receiverOrder[1001]=_nextReceiverOrder++;
        _screen=MenuScreen::slots; _slot=1; _pressed=-1; _deleteConfirmation=false; _slotSelectionMoved=false;
        bool found=false;
        for (int i=0;i<3;++i) if (_save->slots[i].occupied() && (!found || _save->slots[i].i32(4)>_save->slots[_slot].i32(4))) {
            found=true; _slot=i;
        }
        for (int i=0;i<3;++i) refreshPreview(i);
        _receiverOrder[1300]=_nextReceiverOrder++;_receiverOrder[1301]=_nextReceiverOrder++;
    }
    void refreshPreview(int i) {
        // 0x186d8 removes the old slot subtree, then constructs a new button.
        // Its UI receiver is appended after every receiver that already exists.
        _receiverOrder[1100+i]=_nextReceiverOrder++;
        _buttonStates.erase(1100+i);
        auto& preview=_previews[i]; preview=SlotPreview{};
        _slotTexts[i]={};_slotGauges[i]=0;
        if (!_save->slots[i].occupied()) return;
        preview.exists=true;
        const auto& slot=_save->slots[i];
        // GuiSelectSlot.refreshSlot 0x186d8 formats labels once when building the
        // slot's QobImageFont objects; draw only submits those existing glyphs.
        _slotGauges[i]=slot.f32(0x24);
        _slotTexts[i]={format02d("",slot.level()+1),
            dateText(slot.i32(0)),dateText(slot.i32(4)),
            format02d("",slot.bytes[0x2f]+1),
            format02d("",slot.bytes[0x2e]+1),
            format02d("",slot.bytes[0x2d]+1)};
        if(self.previewPlayerCreated)self.previewPlayerCreated();
        std::string names[3]={ format02d("P_Body_",slot.bytes[0x2f]+1),
            format02d("P_Top_",slot.bytes[0x2e]+1),
            format02d("P_Wpn_Gun",slot.bytes[0x2d]+1) };
        for (int j=0;j<3;++j) {
            auto found=_data->parts.find(names[j]);
            if (found!=_data->parts.end()) {preview.parts[j].info=&found->second;if(found->second.tileCnt)_media.imageSize(found->second.tileName(),"shared");}
            heavymach::attachParts(preview.parts[j],j?&preview.parts[j-1]:nullptr,j?1:0);
            // setPartsToMyMach 0x1be84 subtracts .01 after every part assignment.
            preview.motion.rotation-=.01f;
        }
        // refreshSlot -> addParts -> GobMachWeapon.init (0x11d70): one draw per
        // created weapon, even for a stationary/unselected model. The hidden World
        // keeps its previous clock; constructor draws are independent of pause.
        preview.motion.hasWeapon=preview.parts[2].info&&preview.parts[2].info->partsType==2;
        if(preview.motion.hasWeapon){preview.weapon.resetTime=float(_previewWorldTime);preview.weapon.delayTime=float(self.random31?self.random31():0)*0x1p-31f+.5f;}
        auto weapon=_data->weapons.find(names[2]);
        if (weapon!=_data->weapons.end()) preview.weapon.weaponInfo=&weapon->second;
        preview.weapon.aimDirDest=0.1f;
        preview.motion.position={float(82+155*i),166};
        preview.motion.setMoveVel(i==_slot?1:0);
        preview.motion.player=true; preview.motion.useEngineBounce=true;
    }
    void drawPreview(int i) {
        std::string previousScope=_media.resourceScope;_media.resourceScope="shared";
        auto& preview=_previews[i]; auto& motion=preview.motion;
        for (int j=0;j<3;++j) {
            auto sprite=j==2 ? heavymach::drawWeapon(preview.parts[j],preview.weapon,motion.position,true,3,motion.rotation,motion.engineBounce)
                             : heavymach::drawParts(preview.parts[j],motion.position,motion.rotation,motion.engineBounce,0,motion.speed,_time);
            if (!sprite.tile.empty()) _media.draw(sprite.tile,sprite.position.x,sprite.position.y,sprite.cols,sprite.rows,sprite.frame,sprite.scale.x,sprite.scale.y,sprite.rotation,sprite.alpha);
        }
        _media.resourceScope=previousScope;
    }
    void tile(const std::string& name,double x,double y,int columns,int rows,int index,double sx) {
        _media.draw(name,x,y,columns,rows,index,sx,1,0,1);
    }
    void button(const std::string& name,int identifier,double x,double y,int rows,int tile,bool enabled) {
        if(!_receiverOrder.contains(identifier))_receiverOrder[identifier]=_nextReceiverOrder++;
        Size imageSize=_media.imageSize(name);
        auto& motion=_buttonStates[identifier];
        // Every GuiSelectSlot/Mode/Mission button explicitly disables displacement.
        motion.pushDepth=motion.releaseHeight=motion.releaseDelay=0;
        this->tile(name,x,y+motion.yOffset(_time,enabled),1,rows,enabled && rows>1 && motion.pressed ? 1 : tile,1);
        if (enabled && imageSize.width>0) _buttons.push_back({identifier,makeRect(x+_layoutOffset.x-imageSize.width/2,y+_layoutOffset.y-imageSize.height/rows/2,imageSize.width,imageSize.height/rows)});
    }
    void numberText(const std::string& text,double x,double y,double pitch) {
        // ASCII-only labels: characterAtIndex over UTF-16 equals a char loop.
        for (size_t i=0;i<text.size();++i) {
            unsigned char ch=static_cast<unsigned char>(text[i]);
            int tile=ch>='0' && ch<='9' ? ch-'0' : ch=='.'?10:ch=='/'?11:ch==':'?12:ch=='-'?13:-1;
            if (tile>=0 && tile<16) _media.draw("NumSet01.png",x+i*pitch,y,16,1,tile,1,1,0,1,2);
        }
    }
    std::string dateText(int32_t value) {
        time_t stamp=value; struct tm local{}; localtime_r(&stamp,&local);
        char text[32]; strftime(text,sizeof(text),"%Y/%m/%d %H:%M:%S",&local);
        return std::string(text);
    }
    void drawSlots() {
        const bool wide=_canvasWidth>480;
        if(wide) {
            wideBackdrop();
            piece("SelSlot_BG.png",makeRect(0,242,480,78),makePoint(midX(_safeRect)-240,12));
        } else _media.draw("SelSlot_BG.png",240,160);
        double bottom=wide?std::max(double(21),minY(_safeRect)+22):21;
        button("Common_Btn_OK.png",1000,wide?maxX(_safeRect)-58:422,bottom,2,0,true);
        button("Common_Btn_DEL.png",1001,wide?minX(_safeRect)+58:332,wide?bottom:20,2,0,true);
        for (int i=0;i<3;++i) {
            const double dx=i*155;
            Point offset=makePoint(slotOffset(i),wide?minY(_safeRect)+4:0);
            if(wide)piece("SelSlot_BG.png",makeRect(7+dx,43,155,199),offset);
            layoutOffset(offset);
            if (!_previews[i].exists) _media.draw("SelSlot_Empty.png",82+dx,156,1,1,0,0.9,0.9,0,1);
            else {
                float gauge=_slotGauges[i];
                const std::vector<std::string>& labels=_slotTexts[i];
                tile("SelSlot_ExpGauge.png",68+dx+32*gauge,155,1,1,0,gauge);
                // 0x186d8 creates the gauge before the model in the same slot.
                drawPreview(i);
                numberText(labels[0],52+dx,156,7);
                numberText(labels[1],36+dx,106,6);
                numberText(labels[2],36+dx,82,6);
                numberText(labels[3],50+dx,62,7);
                numberText(labels[4],91+dx,62,7);
                numberText(labels[5],132+dx,62,7);
            }
            button("SelSlot_Btn.png",1100+i,85+dx,142,1,0,true);
        }
        layoutOffset(makePoint(slotOffset(_slot),wide?minY(_safeRect)+4:0));
        _media.draw("SelSlot_Sel.png",(_slotSelectionMoved?85:83)+155*_slot,_slotSelectionMoved?142:144,1,1,0,1,1,0,1,2);
        layoutOffset(makePoint(wide?midX(_safeRect)-240:0,0));
        if (_deleteConfirmation) {
            _media.draw("SelSlot_DelSlot.png",240,140);
            button("Common_Btn_YES.png",1300,236,108,2,0,true);
            button("Common_Btn_NO.png",1301,307,108,2,0,true);
        }
    }
    void drawMode() {
        if(_canvasWidth>480)_media.drawJoinedBackground("SelMode_BG.png",240,160,1,0);
        else _media.draw("SelMode_BG.png",240,160);
        button("Common_Btn_OK.png",1000,434,56,2,0,true);
        button("SelMode_Btn_Scenario.png",1001,240,180,2,_mode==0,true);
        button("SelMode_Btn_Survival.png",1002,240,104,2,_mode==1,true);
    }
    void setTop(int top) {
        // Original order permits -1 for a new save (first row centered at y154).
        _top=std::min(std::max(top,0),_save->slots[_slot].i32(0xc)-1);
        for (int i=0;i<7;++i) {
            if (i<_top) _targets[i]={float(220-480*(_top-i)),250};
            else _targets[i]={220,float(i>=_top+3 ? -50 : 250-96*(i-_top))};
        }
    }
    void showMissions() {
        _buttonStates.clear(); _buttons.clear(); _pressed=-1;
        _receiverOrder.clear();_nextReceiverOrder=0;
        _screen=MenuScreen::mission;
        _mission=_save->slots[_slot].i32(0xc); _stage=_save->slots[_slot].i32(0x10);
        for (int i=0;i<7;++i) _positions[i]={220,float(i<3?250-96*i:-50)};
        setTop(_mission-1);
    }
    void showMode() {_receiverOrder.clear();_nextReceiverOrder=0;_buttonStates.clear();_buttons.clear();_slot=_gameSlot;_screen=MenuScreen::mode;_mode=0;_pressed=-1;}
    void startSelectedGame() {if(self.startGame)self.startGame(_gameSlot,_mode,_mission,_stage);}
    void showMissions(int mission,int stage) {
        showMissions(); _mission=mission; _stage=stage; _pressed=-1;
        setTop(_mission-1);
    }
    void drawMissions() {
        if(_canvasWidth>480)_media.drawJoinedBackground("SelMission_BG.png",240,160,1,0);
        else _media.draw("SelMission_BG.png",240,160);
        button("Common_Btn_OK.png",1000,438,_canvasWidth>480?std::max(double(26),minY(_safeRect)+22):26,2,0,true);
        button("SelMission_Btn_Up.png",1001,438,210,2,0,true);
        button("SelMission_Btn_Dn.png",1002,438,110,2,0,true);
        const auto& save=_save->slots[_slot]; int lastMission=save.i32(0xc),lastStage=save.i32(0x10);
        for (int i=0;i<7;++i) {
            if (i>lastMission+1) continue;
            MenuPosition p=_positions[i];
            tile("SelMission_Slot.png",p.x,p.y,1,2,i==_mission,1);
            tile("SelMission_Text.png",p.x+18,p.y+26,1,8,i,1);
            button(format02d("SelMission_TileMission",i+1,".png"),1200+i,p.x-114,p.y+4,3,i>lastMission?2:i==_mission,i<=lastMission);
            if (i<=lastMission) {
                button("SelMission_Btn_Left.png",1100+10*i,p.x+78,p.y+4,2,0,true);
                button("SelMission_Btn_Right.png",1101+10*i,p.x+140,p.y+4,2,0,true);
            }
            for (int j=0;j<heavymach::GlobalInfo::stageCount(i);++j) {
                int tile=(i<lastMission || (i==lastMission && j<=lastStage))?0:2;
                if (i==_mission && j==_stage) tile=1;
                this->tile(format02d("SelMission_Stage",j+1,".png"),p.x-22+35*j,p.y-28,1,3,tile,1);
            }
        }
    }
    void draw() {
        _buttons.clear();layoutOffset(Point{});
        if(_canvasWidth>480&&_screen==MenuScreen::mode)connectedBackdrop("Extensions/ModeWide.png","SelMode_BG.png");
        if(_canvasWidth>480&&_screen==MenuScreen::mission)connectedBackdrop("Extensions/MissionWide.png","SelMission_BG.png");
        switch (_screen) {
            case MenuScreen::slots: drawSlots(); break;
            case MenuScreen::mode: drawMode(); break;
            case MenuScreen::mission: drawMissions(); break;
        }
        layoutOffset(Point{});
    }
    void tick(double time) {
        _time=time;
        if (_screen==MenuScreen::slots&&!_previewWorldPaused) for (int i=0;i<3;++i) if (_previews[i].exists) {
            auto& preview=_previews[i]; preview.motion.setMoveVel(i==_slot ? 1 : 0);
            heavymach::MotionWorld world; world.globalTime=time; world.time=_previewWorldTime;
            // The ObjC block captured self and read self.random31 at call time.
            Menus* owner=&self;
            world.random=[owner] { return owner->random31?owner->random31():0; };
            for (const auto& event:preview.motion.processModel(world))
                if (event.kind==heavymach::MotionEventKind::ModelAim) heavymach::setWeaponAim(preview.weapon,event.value);
        }
        if (_screen==MenuScreen::mission) for (int i=0;i<7;++i) {
            _positions[i].x+=(_targets[i].x-_positions[i].x)/5;
            _positions[i].y+=(_targets[i].y-_positions[i].y)/5;
        }
    }
    void activate(int identifier) {
        std::string sound="UI_Click";
        if (_screen==MenuScreen::slots) {
            if (identifier==1300 && _deleteConfirmation) {
                _save->slots[_slot].bytes[8]=0; refreshPreview(_slot); _deleteConfirmation=false;
                sound="UI_Btn_OK";
            } else if (identifier==1301 && _deleteConfirmation) { _deleteConfirmation=false; sound="UI_Btn_Cancel"; }
            else if (!_deleteConfirmation && identifier==1000) {
                heavymach::GlobalInfo(_save->slots[_slot]).selectSlot(int32_t(std::time(nullptr)));
                if(self.slotSelected)self.slotSelected(_slot);
                if (self.saveChanged) self.saveChanged();
                _gameSlot=_slot; if(self.changeScreen)self.changeScreen(2); sound="UI_Btn_OK";
            } else if (!_deleteConfirmation && identifier==1001) {
                _deleteConfirmation=_save->slots[_slot].occupied(); sound="UI_Btn_OK";
            } else return;
        } else if (_screen==MenuScreen::mode) {
            if (identifier==1001) _mode=0;
            else if (identifier==1002) _mode=1;
            else if (identifier==1000) {
                sound="UI_Btn_OK";
                if (_mode==0) {if(self.changeScreen)self.changeScreen(4);}
                else {_mission=4;_stage=4;if(self.changeScreen)self.changeScreen(6);}
            } else return;
        } else {
            if (identifier==1000) { if (self.changeScreen) self.changeScreen(6); sound="UI_Btn_OK"; }
            else if (identifier==1001) setTop(_top-1);
            else if (identifier==1002) setTop(_top+1);
            else if (identifier>=1200 && identifier<1207) { _mission=identifier-1200; _stage=0; setTop(_top); }
            else if (identifier>=1100 && identifier<1170) {
                int mission=(identifier-1100)/10, direction=(identifier-1100)%10;
                if (_mission!=mission) { _mission=mission; _stage=0; }
                _stage=direction==0 ? std::max(0,_stage-1) : _stage+1;
                if (_mission==_save->slots[_slot].i32(0xc)) _stage=std::min(_stage,_save->slots[_slot].i32(0x10));
                _stage=std::min(_stage,heavymach::GlobalInfo::stageCount(_mission)-1);
                setTop(_top);
            } else return;
        }
        _media.sound(sound);
    }
    void touch(Point logical,int state) {
        if(state==0)_cancelledPressByLayout=false;
        if(_cancelledPressByLayout){if(state>=2)_cancelledPressByLayout=false;return;}
        // QobManager 0xfc18 stops immediately after the first receiver consumes.
        auto receivers=_buttons;
        std::stable_sort(receivers.begin(),receivers.end(),[&](const auto& a,const auto& b){
            return _receiverOrder[a.identifier]<_receiverOrder[b.identifier];
        });
        for(auto it=receivers.rbegin();it!=receivers.rend();++it){
            bool inside=originalButtonContains(it->bounds,logical);
            _buttonStates[it->identifier].touch(inside,state,_time);
            if(!inside)continue;
            int hit=it->identifier;
            if(state==0&&_screen==MenuScreen::slots&&!_deleteConfirmation&&hit>=1100&&hit<1103){
                _slot=hit-1100;_slotSelectionMoved=true;_media.sound("UI_Click");
            }else if(state==2)activate(hit);
            break;
        }
    }
    std::vector<MenuButton> keyboardButtons() {
        auto buttons=_buttons;
        buttons.erase(std::remove_if(buttons.begin(),buttons.end(),[&](const auto& b){
            return midX(b.bounds)<0||midX(b.bounds)>_canvasWidth||midY(b.bounds)<0||midY(b.bounds)>320;
        }),buttons.end());
        std::stable_sort(buttons.begin(),buttons.end(),[](const auto& a,const auto& b){
            if(std::fabs(midY(a.bounds)-midY(b.bounds))>8)return midY(a.bounds)>midY(b.bounds);
            return midX(a.bounds)<midX(b.bounds);
        });
        return buttons;
    }
    void moveKeyboardFocus(int delta) {
        auto buttons=keyboardButtons();if(buttons.empty())return;
        int index=-1;for(size_t i=0;i<buttons.size();++i)if(buttons[i].identifier==_keyboardFocus)index=int(i);
        if(index<0)index=delta<0?int(buttons.size())-1:0;else index=(index+delta+int(buttons.size()))%int(buttons.size());
        _keyboardFocus=buttons[index].identifier;_keyboardFocusVisible=true;
    }
    Rect keyboardFocusRect() {
        if(!_keyboardFocusVisible)return Rect{};
        auto buttons=keyboardButtons();for(const auto& b:buttons)if(b.identifier==_keyboardFocus)return b.bounds;
        if(buttons.empty())return Rect{};_keyboardFocus=buttons.front().identifier;return buttons.front().bounds;
    }
    void activateKeyboardFocus() {
        _keyboardFocusVisible=true;Rect rect=keyboardFocusRect();if(isEmpty(rect))return;
        Point point=makePoint(midX(rect),midY(rect));touch(point,0);touch(point,2);
    }
};

Menus::Menus(Media& media,heavymach::SaveData* save,const heavymach::GameData* data)
    : s(std::make_unique<State>(*this,media,save,data)) {}
Menus::~Menus()=default;
void Menus::setPreviewWorldTime(double time,bool paused) {s->setPreviewWorldTime(time,paused);}
void Menus::showSlots() {s->showSlots();}
void Menus::showMode() {s->showMode();}
void Menus::showMissions() {s->showMissions();}
bool Menus::isMissionSelection() const {return s->_screen==MenuScreen::mission;}
void Menus::startSelectedGame() {s->startSelectedGame();}
void Menus::showMissions(int mission,int stage) {s->showMissions(mission,stage);}
void Menus::setCanvasWidth(double width,Rect safeRect) {s->setCanvasWidth(width,safeRect);}
void Menus::draw() {s->draw();}
void Menus::tick(double time) {s->tick(time);}
void Menus::touch(Point logical,int state) {s->touch(logical,state);}
void Menus::moveKeyboardFocus(int delta) {s->moveKeyboardFocus(delta);}
void Menus::activateKeyboardFocus() {s->activateKeyboardFocus();}
Rect Menus::keyboardFocusRect() {return s->keyboardFocusRect();}
}
