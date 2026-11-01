#include "SaveTransfer.hpp"
#include <memory>
using namespace heavymach;using namespace heavymach::transfer;
namespace {Snapshot input,output;std::unique_ptr<Restore> transaction;std::string result,error;std::array<uint8_t,SaveData::byteCount> game;template<class F>int call(F f){try{f();error.clear();return 1;}catch(const std::exception&e){error=e.what();return 0;}}}
extern "C" {
const char* hm_st_error(){return error.c_str();}
int hm_st_begin(const uint8_t*b,int size,int aim){return call([&]{if(size!=SaveData::byteCount)throw std::invalid_argument("Invalid game size");input=Snapshot{};input.game=SaveData::decode({b,size_t(size)});input.aim=aim<0?1:aim;});}
int hm_st_hud(int i,double x,double y){return call([&]{if(i<0||i>=hud::groupCount)throw std::invalid_argument("Invalid HUD group");input.positions[i]=hud::Placement{x,y};});}
int hm_st_key(int i,int k){return call([&]{if(i<0||i>=KeyboardInput::count)throw std::invalid_argument("Invalid key action");if(!input.keys)input.keys=KeyboardInput::defaults;(*input.keys)[i]=unsigned(k);});}
const char* hm_st_encode(){if(!call([]{result=encode(input);}))return nullptr;return result.c_str();}
int hm_st_decode(const char*s){return call([&]{output=decode(s);});}
const uint8_t* hm_st_game(){game=output.game.encode();return game.data();}
int hm_st_aim(){return output.aim;}
int hm_st_key_at(int i){return output.keys&&i>=0&&i<KeyboardInput::count?int((*output.keys)[i]):-1;}
double hm_st_hud_at(int i,int axis){return i>=0&&i<hud::groupCount&&output.positions[i]?(axis?output.positions[i]->y:output.positions[i]->x):-1.;}
const char* hm_st_summary(){if(!call([]{result=summary(output);}))return nullptr;return result.c_str();}
int hm_st_restore(int source,int target,int settings){return call([&]{transaction=std::make_unique<Restore>(input,merge(input,output,source,target,settings));output=transaction->after();});}
int hm_st_step(){return transaction?int(transaction->step()):-1;}
const char* hm_st_before(){if(!call([]{if(!transaction)throw std::invalid_argument("No restore");result=encode(transaction->before());}))return nullptr;return result.c_str();}
const char* hm_st_after(){if(!call([]{if(!transaction)throw std::invalid_argument("No restore");result=encode(transaction->after());}))return nullptr;return result.c_str();}
int hm_st_ack(){return call([]{if(!transaction)throw std::invalid_argument("No restore");transaction->acknowledge();});}
int hm_st_verify(){return call([]{if(!transaction)throw std::invalid_argument("No restore");transaction->verify(input);});}
}
