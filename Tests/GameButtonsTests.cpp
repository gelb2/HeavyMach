#include "../Sources/iOS/HMGameButtons.hpp"
#include <cassert>
#include <iostream>
#include <string>
using namespace heavymach::gameui;
int main(){
 Buttons b;
 auto frame=[&](double time){b.beginFrame(time);b.add(1,"APPLY",{10,20,140,40});b.add(2,"CANCEL",{160,20,140,40});};
 frame(1);
 // Original min-exclusive/max-inclusive rectangle edges.
 assert(!b.touch(1,10,30,0));assert(b.touch(1,150,60,0)->id==1);
 assert(b.touch(1,150,60,2)->state==2);
 // A toolbar press cannot turn into another button or a HUD drag.
 assert(b.touch(1,20,30,0)->state==0);frame(1.1);
 assert(b.touch(1,180,30,1)->id==1);
 assert(b.touch(1,180,30,2)->state==3);
 assert(!b.touch(2,20,30,2));
 // A second finger cannot release/click the first finger's button.
 assert(b.touch(1,20,30,0)->state==0);
 assert(b.touch(2,20,30,0)->state==1);
 assert(b.touch(2,20,30,2)->state==1);
 assert(b.touch(1,20,30,2)->state==2);
 // Two separate held controls keep their own touch identity.
 b.touch(1,20,30,0);b.touch(2,180,30,0);
 assert(b.touch(1,20,30,3)->state==3);
 assert(b.touch(2,180,30,2)->id==2);
 // Async save disables the action while pointer is down.
 b.touch(1,20,30,0);b.beginFrame(2);b.add(1,"APPLY",{10,20,140,40},false);
 assert(b.touch(1,20,30,2)->state==3);
 // Removing a screen/button and cancelling focus cannot activate the new one.
 frame(3);b.touch(1,20,30,0);b.cancel();frame(4);
 auto cancelled=b.touch(1,20,30,2);assert(cancelled&&cancelled->id==-1);
 // Closing a panel with finger 2 still consumes finger 1's later release;
 // otherwise the host can send that release to an original OK/DEL button.
 b.touch(1,20,30,0);b.touch(2,180,30,0);
 assert(b.touch(2,180,30,2)->id==2);b.cancel();b.beginFrame(4.1);
 assert(b.touch(1,180,30,1)->id==-1);
 assert(b.touch(1,180,30,2)->id==-1);
 assert(!b.touch(1,180,30,2));
 // Empty modal background also owns the sequence until the finger releases.
 b.consumeTouch(3,0);b.cancel();b.beginFrame(4.15);
 assert(b.touch(3,180,30,2)->id==-1);
 assert(!b.touch(3,180,30,2));
 // OS pointer identifiers can be reused after an interrupted sequence.
 frame(4.2);b.touch(1,20,30,0);b.cancel();frame(4.3);
 assert(b.touch(1,20,30,0)->id==1);assert(b.touch(1,20,30,2)->state==2);
 b.key(0x2b,true);assert(b.focusedId()==1);assert(b.key(0x28,true)->state==0);
 assert(b.key(0x28,true)->state==1);assert(b.key(0x28,false)->state==2);
 b.key(0x2b,true);assert(b.focusedId()==2);b.key(0x2c,true);b.cancel();assert(b.key(0x2c,false)->state==1);
 // Draw work uses existing texture-region submissions only, within bounds.
 b.beginFrame(5);b.add(1,"RESTORE",{10,20,140,40});b.text("SLOT 1\nLEVEL 20",{0,80,130,40});
 int glyphs=0,plates=0;b.draw([&](const char* name,Rect src,Rect dst){
  assert(src.width>0&&src.height>0&&dst.width>0&&dst.height>0);
  if(std::string(name)=="Common_Btn_OK.png")++plates;
  else{assert(std::string(name)=="Extensions/GameMenuGlyphs.png");assert(src.x>=0&&src.x+src.width<=512&&src.y>=0&&src.y+src.height<=144);++glyphs;}
 });assert(plates==3&&glyphs>10);
 // Fitting a long label to the exact available width must not wrap away its
 // last glyph because of floating-point roundoff (HUD / KEYS: YES).
 b.beginFrame(6);b.add(1,"HUD / KEYS: YES",{0,0,140,28});int visible=0;
 b.draw([&](const char* name,Rect,Rect){if(std::string(name)=="Extensions/GameMenuGlyphs.png")++visible;});
 assert(visible==12); // HUD (3), slash (1), KEYS: (5), YES (3).
 std::cout<<"PASS: original edges, drag capture, multi-touch, disabled action, cancellation, keyboard repeat and sprite submissions\n";
}
