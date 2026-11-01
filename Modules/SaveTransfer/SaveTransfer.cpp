#include "SaveTransfer.hpp"
#include <bit>
#include <sstream>
#include <set>
#include <cctype>
namespace heavymach::transfer {
namespace {
using Bytes=std::vector<uint8_t>;
constexpr size_t maxPayload=8192,maxCode=24000;
void require(bool ok,const char* message){if(!ok)throw std::invalid_argument(message);}
void put(Bytes& b,uint64_t n,unsigned count){for(unsigned i=0;i<count;++i)b.push_back(uint8_t(n>>(8*i)));}
struct Reader{std::span<const uint8_t>b;size_t i=0;uint64_t get(unsigned n){require(n<=8&&i+n<=b.size(),"Truncated recovery data");uint64_t v=0;for(unsigned j=0;j<n;++j)v|=uint64_t(b[i++])<<(8*j);return v;}};
uint32_t crc(std::span<const uint8_t>b){uint32_t c=~0u;for(auto v:b){c^=v;for(int i=0;i<8;++i)c=(c>>1)^(0xedb88320u&uint32_t(-int(c&1)));}return ~c;}
const std::string alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
std::string base64(const Bytes& b){std::string s;uint32_t acc=0;unsigned bits=0;for(auto v:b){acc=(acc<<8)|v;bits+=8;while(bits>=6){bits-=6;s+=alphabet[(acc>>bits)&63];}}if(bits)s+=alphabet[(acc<<(6-bits))&63];return s;}
Bytes unbase(std::string_view s){Bytes b;uint32_t acc=0;unsigned bits=0;require(s.size()%4!=1,"Invalid recovery code length");for(char c:s){auto p=alphabet.find(c);require(p!=std::string::npos,"Invalid recovery code character");acc=(acc<<6)|uint32_t(p);bits+=6;if(bits>=8){bits-=8;b.push_back(uint8_t(acc>>bits));}}require(!bits||(acc&((1u<<bits)-1))==0,"Noncanonical recovery code");return b;}
// Deterministic PackBits-style encoding; identical native/Wasm output, no library version dependency.
Bytes pack(const Bytes& b){Bytes out;size_t i=0;while(i<b.size()){size_t run=1;while(i+run<b.size()&&b[i+run]==b[i]&&run<128)++run;if(run>=4){out.push_back(uint8_t(128+run-1));out.push_back(b[i]);i+=run;continue;}size_t start=i;i+=run;while(i<b.size()&&i-start<128){run=1;while(i+run<b.size()&&b[i+run]==b[i]&&run<128)++run;if(run>=4)break;size_t take=std::min(run,128-(i-start));i+=take;if(take<run)break;}out.push_back(uint8_t(i-start-1));out.insert(out.end(),b.begin()+start,b.begin()+i);}return out;}
Bytes unpack(std::span<const uint8_t>b,size_t expected){require(expected<=maxPayload,"Recovery data too large");Bytes out;size_t i=0;while(i<b.size()){unsigned ctl=b[i++],n=(ctl&127)+1;require(out.size()+n<=expected,"Invalid recovery data size");if(ctl&128){require(i<b.size(),"Truncated recovery data");out.insert(out.end(),n,b[i++]);}else{require(i+n<=b.size(),"Truncated recovery data");out.insert(out.end(),b.begin()+i,b.begin()+i+n);i+=n;}}require(out.size()==expected,"Truncated recovery data");return out;}
Bytes payload(const Snapshot&s){validate(s);Bytes b;auto game=s.game.encode();b.insert(b.end(),game.begin(),game.end());put(b,s.aim+1,1);put(b,s.keys?1:0,1);if(s.keys)for(auto k:*s.keys)put(b,k,2);put(b,hud::groupCount,1);for(auto p:s.positions){put(b,p?1:0,1);if(p){put(b,std::bit_cast<uint64_t>(p->x==0?0.:p->x),8);put(b,std::bit_cast<uint64_t>(p->y==0?0.:p->y),8);}}return b;}
}
void validate(const Snapshot&s){
 auto bytes=s.game.encode();(void)bytes;
 static constexpr int counts[]={4,5,5,5,5,5,6};
 for(auto& slot:s.game.slots){if(!slot.occupied())continue;
  require(slot.i32(0xc)>=0&&slot.i32(0xc)<7,"Unsupported mission");
  require(slot.i32(0x10)>=0&&slot.i32(0x10)<counts[slot.i32(0xc)],"Unsupported stage");
  require(slot.experience()>=0,"Invalid experience");
  for(size_t p:{size_t(0x2d),size_t(0x2e),size_t(0x2f)})require(slot.bytes[p]<=6,"Invalid tank part level");
  static const std::set<std::string> weapons={"P_Wpn_Canon01","P_Wpn_Canon02","P_Wpn_Canon03","P_Wpn_ElShock","P_Wpn_Gun01","P_Wpn_Gun02","P_Wpn_Gun03","P_Wpn_Gun04","P_Wpn_Gun05","P_Wpn_Gun06","P_Wpn_Gun07","P_Wpn_Laser","P_Wpn_Missile01","P_Wpn_Missile02","P_Wpn_Missile03","P_Wpn_RaderBomb","P_Wpn_RaderLsr","P_Wpn_RaderMsl","P_Wpn_Shotgun01","P_Wpn_Shotgun02"};
  for(int i=0;i<slot.weaponCount();++i){auto name=slot.weaponName(i);require(weapons.contains(name),"Unsupported weapon");require(!name.empty()&&name.size()<24,"Invalid weapon");for(unsigned char c:name)require(std::isalnum(c)||c=='_',"Invalid weapon name");require(slot.weaponCharge(i)>=0,"Invalid ammunition");}
 }
 require(s.aim>=-1&&s.aim<=2,"Invalid aim setting");
 if(s.keys){std::set<unsigned> seen;for(auto k:*s.keys)require(k>0&&k<256&&k!=227&&k!=231&&seen.insert(k).second,"Invalid key bindings");}
 for(auto p:s.positions)if(p)require(std::isfinite(p->x)&&std::isfinite(p->y)&&p->x>=0&&p->x<=1&&p->y>=0&&p->y<=1,"Invalid HUD position");
}
std::string encode(const Snapshot&s){auto raw=payload(s);Bytes b{'H','M',1,1};put(b,raw.size(),4);put(b,crc(raw),4);auto compressed=pack(raw);b.insert(b.end(),compressed.begin(),compressed.end());return "HM1-"+base64(b);}
Snapshot decode(std::string_view input){require(input.size()<=maxCode,"Recovery code too long");std::string code;for(char c:input)if(c!=' '&&c!='\n'&&c!='\r'&&c!='\t')code+=c;require(code.starts_with("HM1-"),"Unsupported recovery code");auto bytes=unbase(std::string_view(code).substr(4));Reader header{bytes};require(header.get(1)=='H'&&header.get(1)=='M'&&header.get(1)==1&&header.get(1)==1,"Unsupported recovery version");auto n=header.get(4),checksum=header.get(4);auto b=unpack(std::span(bytes).subspan(header.i),n);require(crc(b)==checksum,"Recovery code is damaged");require(b.size()>=SaveData::byteCount,"Truncated game save");Snapshot s;s.game=SaveData::decode(std::span(b).first(SaveData::byteCount));Reader r{b,SaveData::byteCount};s.aim=int(r.get(1))-1;auto hasKeys=r.get(1);require(hasKeys<=1,"Invalid key setting flag");if(hasKeys){s.keys=KeyboardInput::defaults;for(auto& k:*s.keys)k=unsigned(r.get(2));}require(r.get(1)==hud::groupCount,"Unsupported HUD version");for(auto& p:s.positions){auto present=r.get(1);require(present<=1,"Invalid HUD flag");if(present){double x=std::bit_cast<double>(r.get(8)),y=std::bit_cast<double>(r.get(8));p=hud::Placement{x,y};}}require(r.i==b.size(),"Unexpected recovery data");validate(s);return s;}
Snapshot merge(const Snapshot&current,const Snapshot&incoming,int source,int target,bool settings){validate(current);validate(incoming);require((source==-1&&target==-1)||(source>=0&&source<3&&target>=0&&target<3),"Invalid recovery slot");Snapshot result=current;if(source==-1)result.game=incoming.game;else{require(incoming.game.slots[source].occupied(),"Selected source slot is empty");result.game.slots[target]=incoming.game.slots[source];}if(settings){result.positions=incoming.positions;result.keys=incoming.keys;result.aim=incoming.aim;}return result;}
bool equal(const Snapshot&a,const Snapshot&b){return payload(a)==payload(b);}
std::string summary(const Snapshot&s){validate(s);std::ostringstream o;for(size_t i=0;i<3;++i){auto& v=s.game.slots[i];o<<"Slot "<<i+1<<": ";if(!v.occupied())o<<"empty";else{o<<"Mission "<<v.i32(0xc)+1<<'-'<<v.i32(0x10)+1<<" | EXP "<<v.experience()<<" | Level "<<v.level()+1<<" | "<<v.weaponCount()<<" weapons";}o<<'\n';}return o.str();}
void Restore::acknowledge(){require(step_!=Step::verify&&step_!=Step::complete,"Unexpected restore acknowledgement");step_=Step(int(step_)+1);}
void Restore::verify(const Snapshot& actual){require(step_==Step::verify,"Unexpected restore verification");require(equal(after_,actual),"Restored data does not match");step_=Step::finish;}
void restore(Store&store,const Snapshot&target){Restore r(store.read(),target);while(r.step()!=Step::complete){switch(r.step()){case Step::preserveBefore:store.preserve(encode(r.before()));break;case Step::writePending:store.pending(encode(r.after()));break;case Step::apply:store.apply(r.after());break;case Step::verify:r.verify(store.read());continue;case Step::finish:store.finish();break;case Step::complete:break;}r.acknowledge();}}
void resume(Store&store,std::string_view code){auto target=decode(code);store.apply(target);require(equal(store.read(),target),"Restored data does not match");store.finish();}
}
