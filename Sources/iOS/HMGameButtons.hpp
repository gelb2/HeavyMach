#pragma once
// Original QobButton state/rectangle procedure, shared by the added game menus.
// Hosts submit these regions through their existing game sprite renderer.
#include "HMButtonState.hpp"
#include "HMGameGlyphMetrics.hpp"
#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace heavymach::gameui {
struct Rect {double x=0,y=0,width=0,height=0;};
inline bool contains(Rect r,double x,double y) {
    return float(x)>float(r.x)&&float(x)<=float(r.x+r.width)&&
           float(y)>float(r.y)&&float(y)<=float(r.y+r.height);
}
struct Event {int id=0,state=3;};
struct Item {int id;std::string label;Rect bounds;bool enabled;};
class Buttons {
    struct Text {std::string value;Rect bounds;double height;bool centered;};
    std::vector<Item> items_;
    std::vector<Text> texts_;
    std::vector<Rect> panels_;
    std::map<int,HMButtonState> states_;
    std::map<std::int64_t,int> captured_;
    std::set<std::int64_t> ignored_;
    double time_=0;
    int focus_=-1,keyDown_=-1;
    bool focusVisible_=false;
    const Item* item(int id)const {auto i=std::find_if(items_.begin(),items_.end(),[=](const Item&b){return b.id==id;});return i==items_.end()?nullptr:&*i;}
    template<class Draw> static void plate(Draw& draw,Rect r,bool pressed) {
        // Common_Btn_OK: original 62x39 normal/pressed rows. Its blank plate
        // is repeated between the original eight-pixel metal edges.
        const double row=pressed?39:0,edge=std::min(8.,r.width/2);
        draw("Common_Btn_OK.png",Rect{0,row,8,39},Rect{r.x,r.y,edge,r.height});
        if(r.width>2*edge)draw("Common_Btn_OK.png",Rect{6,row,2,39},Rect{r.x+edge,r.y,r.width-2*edge,r.height});
        draw("Common_Btn_OK.png",Rect{54,row,8,39},Rect{r.x+r.width-edge,r.y,edge,r.height});
    }
    static unsigned char upper(unsigned char c){return c>='a'&&c<='z'?c-'a'+'A':c;}
    static double advance(unsigned char c,double h){return c==' '?h*.38:(glyphWidths[upper(c)-32]+1)*h/20;}
    static double textWidth(const std::string&s,double h){double w=0;for(unsigned char c:s)if(c>=32&&c<127)w+=advance(c,h);return w;}
    template<class Draw> static void label(Draw& draw,const Text&t) {
        if(t.bounds.width<=0||t.bounds.height<=0||t.height<=0)return;
        const double h=std::min(t.height,t.bounds.height);
        std::vector<std::string> lines;std::string line;double width=0;
        for(unsigned char c:t.value){if(c=='\n'){lines.push_back(line);line.clear();width=0;continue;}
            if(c<32||c>=127)continue;
            double next=advance(c,h);
            if(width+next>t.bounds.width+1e-6&&!line.empty()){lines.push_back(line);line.clear();width=0;}
            line.push_back(char(upper(c)));width+=next;
        }
        lines.push_back(line);
        double top=t.bounds.y+t.bounds.height;
        for(auto&s:lines){if(top-h<t.bounds.y-1e-6)break;
            double x=t.bounds.x+(t.centered?std::max(0.,(t.bounds.width-textWidth(s,h))/2):0);
            for(unsigned char c:s){if(c!=' '){const int index=c-32;double w=glyphWidths[index];
                draw("Extensions/GameMenuGlyphs.png",Rect{double(index%16*32),double(index/16*24),w,20},Rect{x,top-h,w*h/20,h});}
                x+=advance(c,h);
            }top-=h*1.18;
        }
    }

public:
    void beginFrame(double time){time_=std::max(time,0.000001);items_.clear();texts_.clear();panels_.clear();}
    void add(int id,std::string text,Rect r,bool enabled=true){
        if(r.width<=0||r.height<=0)return;
        auto&m=states_[id];m.pushDepth=m.releaseHeight=m.releaseDelay=0;m.pushDelay=.1f;
        items_.push_back({id,std::move(text),r,enabled});
    }
    void text(std::string value,Rect r,double height=12,bool centered=false){texts_.push_back({std::move(value),r,height,centered});}
    void panel(Rect r){panels_.push_back(r);}
    const std::vector<Item>& items()const{return items_;}
    std::optional<Rect> rect(int id)const{auto*b=item(id);if(!b)return {};return b->bounds;}
    bool contains(double x,double y)const{for(auto&b:items_)if(b.enabled&&gameui::contains(b.bounds,x,y))return true;return false;}
    int focusedId()const{return focus_;}
    void cancel(){for(auto&[id,s]:states_)s.touch(false,3,time_);for(const auto& [token,id]:captured_)ignored_.insert(token);captured_.clear();keyDown_=-1;focusVisible_=false;}
    std::optional<Event> touch(std::int64_t token,double x,double y,int phase){
        focusVisible_=false;
        if(phase==0){
            ignored_.erase(token); // A fresh sequence may reuse a platform pointer ID.
            if(captured_.count(token))return Event{captured_[token],1};
            for(auto it=items_.rbegin();it!=items_.rend();++it){if(!it->enabled||!gameui::contains(it->bounds,x,y))continue;
                // One original button object has one press state. A second
                // finger cannot steal or duplicate the first finger's click.
                for(auto&c:captured_)if(c.second==it->id){ignored_.insert(token);return Event{-1,1};}
                captured_[token]=it->id;states_[it->id].touch(true,0,time_);return Event{it->id,0};
            }return {};
        }
        if(ignored_.count(token)){if(phase>=2)ignored_.erase(token);return Event{-1,1};}
        auto c=captured_.find(token);if(c==captured_.end())return {};
        const int id=c->second;auto*b=item(id);const bool inside=b&&b->enabled&&gameui::contains(b->bounds,x,y);
        states_[id].touch(inside,phase,time_);
        if(phase>=2){captured_.erase(c);return Event{id,phase==2&&inside?2:3};}
        return Event{id,1};
    }
    std::optional<Event> key(unsigned code,bool pressed){
        if(code==0x2b){if(pressed){std::vector<int> ids;for(auto&b:items_)if(b.enabled)ids.push_back(b.id);
                if(!ids.empty()){auto i=std::find(ids.begin(),ids.end(),focus_);focus_=i==ids.end()||++i==ids.end()?ids.front():*i;focusVisible_=true;}}
            return Event{-1,1};}
        if(code!=0x28&&code!=0x2c)return {};
        if(pressed){if(keyDown_>=0)return Event{keyDown_,1};auto*b=item(focus_);if(!b||!b->enabled){auto i=std::find_if(items_.begin(),items_.end(),[](const Item&b){return b.enabled;});if(i==items_.end())return Event{-1,1};focus_=i->id;}
            keyDown_=focus_;states_[focus_].touch(true,0,time_);focusVisible_=true;return Event{focus_,0};}
        if(keyDown_<0)return Event{-1,1};const int id=keyDown_;keyDown_=-1;auto*b=item(id);bool active=b&&b->enabled;states_[id].touch(active,2,time_);return Event{id,active?2:3};
    }
    template<class Draw> void draw(Draw draw){
        // Keep original border thickness; stretching a whole 39px button to
        // panel height would turn its bevel into large silver bands.
        for(auto&r:panels_){
            const double e=std::min({6.,r.width/2,r.height/2});
            const double sx[]={0,8,56},sw[]={6,1,6},sy[]={33,18,0},sh[]={6,1,6};
            const double dx[]={r.x,r.x+e,r.x+r.width-e},dw[]={e,r.width-2*e,e};
            const double dy[]={r.y,r.y+e,r.y+r.height-e},dh[]={e,r.height-2*e,e};
            for(int y=0;y<3;++y)for(int x=0;x<3;++x)if(dw[x]>0&&dh[y]>0)
                draw("Common_Btn_OK.png",Rect{sx[x],sy[y],sw[x],sh[y]},Rect{dx[x],dy[y],dw[x],dh[y]});
        }
        for(auto&t:texts_)label(draw,t);
        for(auto&b:items_){auto&m=states_[b.id];auto r=b.bounds;r.y+=m.yOffset(time_,b.enabled);
            plate(draw,r,m.pressed&&b.enabled);
            const double padding=std::min(10.,r.width*.12);
            double height=std::min(15.,r.height*.46);height=std::min(height,height*(r.width-2*padding)/std::max(1.,textWidth(b.label,height)));
            Rect textBounds{r.x+padding,r.y+(r.height-height)/2,r.width-2*padding,height};
            label(draw,{b.label,textBounds,height,true});
            if(focusVisible_&&b.id==focus_)label(draw,{">",{r.x+2,r.y+(r.height-12)/2,9,12},12,false});
            if(!b.enabled)label(draw,{"-",{r.x+2,r.y+(r.height-12)/2,9,12},12,false});
        }
    }
};
}
