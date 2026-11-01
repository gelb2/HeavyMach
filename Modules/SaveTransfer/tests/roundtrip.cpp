#include "../SaveTransfer.hpp"
#include <cassert>
#include <iostream>
#include <random>
using namespace heavymach;using namespace heavymach::transfer;
Snapshot fixture(){Snapshot s;auto&v=s.game.slots[0];v.bytes[8]=1;v.setInt(0x20,12345);v.setInt(0x28,10);v.setInt(0x30,2);v.setWeapon(0,"P_Wpn_Gun01",99);v.setWeapon(1,"P_Wpn_Missile01",17);s.aim=2;s.keys=KeyboardInput::defaults;s.positions[0]=hud::Placement{.25,.75};return s;}
struct Memory:Store{Snapshot state=fixture();std::string old,journal;bool fail=false;Snapshot read()override{return state;}void preserve(std::string_view s)override{old=s;}void pending(std::string_view s)override{journal=s;}void apply(const Snapshot&s)override{state.game=s.game;if(fail)throw std::runtime_error("power loss");state=s;}void finish()override{journal.clear();}};
int main(){auto a=fixture();auto code=encode(a);assert(equal(a,decode(code)));assert(code==encode(decode(code)));assert(equal(a,decode(" \n"+code+"\r\n")));auto current=a;current.game.slots[2]=a.game.slots[0];current.game.slots[2].setInt(0x20,999);auto m=merge(current,a,0,1,false);assert(m.game.slots[2].experience()==999&&m.game.slots[1].experience()==12345);assert(equal(a,merge(a,a)));
 for(size_t i=4;i<code.size();++i){auto broken=code;broken[i]=broken[i]=='A'?'B':'A';bool threw=false;try{decode(broken);}catch(...){threw=true;}assert(threw);}
 for(size_t n=0;n<code.size();++n){bool threw=false;try{decode(code.substr(0,n));}catch(...){threw=true;}assert(threw);}
 auto bad=a;bad.aim=3;bool threw=false;try{encode(bad);}catch(...){threw=true;}assert(threw);
 Memory store;auto target=a;target.game.slots[0].setInt(0x20,54321);target.positions[0]=hud::Placement{.9,.1};store.fail=true;try{restore(store,target);assert(false);}catch(const std::runtime_error&){}assert(!store.journal.empty()&&equal(decode(store.old),a));store.fail=false;resume(store,store.journal);assert(equal(store.state,target)&&store.journal.empty());
 std::mt19937 rng(123);for(int i=0;i<500;++i){auto s=a;s.game.slots[0].setInt(0x20,int(rng()%1000000));s.game.slots[0].setInt(0x4c,int(rng()%500));s.positions[3]=hud::Placement{double(rng())/rng.max(),double(rng())/rng.max()};assert(equal(s,decode(encode(s))));}
 std::cout<<"PASS roundtrip, corruption, truncation, slots, interrupted restore, 500 states\n"<<code<<"\n";
}
