#include "Visuals.hpp"
#include <cmath>
#include <algorithm>
#include <numbers>
#include <stdexcept>
namespace heavymach {
namespace {
constexpr float dt=1.0f/60.0f;
Vector2 rotated(Vector2 p,float rotation){
 float a=static_cast<float>(std::atan2(double(p.y),double(p.x))+double(rotation));
 float len=std::sqrt(p.x*p.x+p.y*p.y);
 return {static_cast<float>(std::cos(double(a))*double(len)),static_cast<float>(std::sin(double(a))*double(len))};
}
DrawSprite partsSprite(const PartsState& p,int z){
 DrawSprite d;d.z=z;if(p.info){d.tile=p.info->tileCnt?p.info->tileName():"PartsDummy.png";d.cols=p.info->tileCnt?p.info->tileCnt:1;}return d;
}
int shortTime(double value){
 // Original converts double to int32 then sign-extends low 16 bits.
 if(!std::isfinite(value)||value<-2147483648.0||value>=2147483648.0)throw std::out_of_range("Original animation int32 conversion overflow unresolved");
 unsigned low=static_cast<unsigned>(static_cast<int32_t>(value))&65535u;
 int s=low>=32768?int(low)-65536:int(low);return std::abs(s);
}
}
Vector2 rotateAttachment(Vector2 p,float a){return {p.x*std::cos(a)-p.y*std::sin(a),p.x*std::sin(a)+p.y*std::cos(a)};}
float velocityPitch(float previous,float x,float y,const MachineBehavior& b){
 constexpr float rad=3.14159265358979323846f/180;
 float target=std::clamp(-x/b.horizontalSpeed,-1.f,1.f)*b.horizontalDegrees*rad-std::clamp(y/b.verticalSpeed,-1.f,1.f)*b.verticalDegrees*rad;
 return previous+(target-previous)/b.easing;
}
const PartFrame* presentationFrame(const PartsState& p,double time){
 if(!p.info||!p.info->presentation)return nullptr;const auto& v=*p.info->presentation;
 int frame=p.tileFrame;if(v.animation==PartAnimation::WorldLoop)frame=int(std::floor(std::fmod(std::max(0.,time),double(v.period))*v.frames.size()/v.period));
 if(frame<0||size_t(frame)>=v.frames.size())throw std::runtime_error("Presentation frame outside contract");return &v.frames[size_t(frame)];
}
void applyPartPresentation(DrawSprite& d,const PartsState& p,Vector2 pos,float angle,float vertical,double time,Vector2 texture){
 auto frame=presentationFrame(p,time);if(!frame)return;const auto& v=*p.info->presentation;
 d.tile=frame->tile;d.cols=v.columns;d.rows=1;d.frame=0;
 d.position.x+=frame->offset.x*std::cos(p.rotation)-frame->offset.y*std::sin(p.rotation);
 d.position.y+=frame->offset.x*std::sin(p.rotation)+frame->offset.y*std::cos(p.rotation);
 if(v.logicalSize.x>0&&v.logicalSize.y>0)d.scale={v.logicalSize.x/std::max(1.f,texture.x),v.logicalSize.y/std::max(1.f,texture.y)};
 if(v.effect==PartEffect::PulsedExhaust){
  float length=v.length+std::clamp(vertical/v.velocityReference,-1.f,1.f)*v.velocityAmplitude+std::sin(float(time)*v.frequency)*v.amplitude;
  auto local=rotateAttachment(v.aft?Vector2{v.nozzle.x+length*.5f,v.nozzle.y}:Vector2{v.nozzle.x,v.nozzle.y-length*.5f},angle);
  d.position={pos.x+local.x,pos.y+local.y};d.rotation=angle+(v.aft?float(std::numbers::pi):float(std::numbers::pi/2));
  d.scale={length/std::max(1.f,texture.x/v.columns),v.thickness/std::max(1.f,texture.y)};d.blend=2;d.alpha=v.alpha;
 }
}
// 0x92b4: reset deliberately preserves alpha/show, as does the original pool reset.
void resetParticle(ParticleState& p,double time){
 p.sprite.frame=0;p.sprite.position={};p.velocity={};p.gravity=0;p.sprite.scale={1,1};
 p.sprite.rotation=0;p.radius=10;p.rotationVelocity=0;p.scaleVelocity=0;p.velocityAdd=0;
 p.liveTime=0;p.tileAnimation=false;p.animationFrames=0;p.startTime=float(time);
 p.sprite.blend=2;p.collision=nullptr;p.groundPosition=-1000;p.selfRemove=false;
}
void startParticle(ParticleState& p,double time){p.startTime=float(time);p.show=true;}
// 0x9a14: normal component plus damped tangent, then .3; negative spin snaps to zero.
void bounceParticle(ParticleState& p,const ColVector& c){
 float dot=p.velocity.x*c.normal.x+p.velocity.y*c.normal.y;
 Vector2 n{c.normal.x*std::fabs(dot),c.normal.y*std::fabs(dot)};
 Vector2 tangent{(p.velocity.x+n.x)*.7f,(p.velocity.y+n.y)*.7f};
 p.velocity={(n.x+tangent.x)*.3f,(n.y+tangent.y)*.3f};
 p.rotationVelocity=float(double(p.rotationVelocity)*.4);
 if(p.rotationVelocity<.1f)p.rotationVelocity=0;
}
// 0x968c processFly + 0x94a8 tick.
void tickParticle(ParticleState& p,double time,const std::function<void()>& baseTick){
 if(!p.show)return;
 p.velocity.y=p.velocity.y+(-dt*p.gravity);
 p.velocityTick={p.velocity.x*dt,p.velocity.y*dt};
 bool collided=false;
 if(p.collision){auto contact=p.collision->checkCollideVelColRadius(p.sprite.position,p.velocityTick,p.radius);
  if(contact){p.sprite.position={contact->x+p.collision->normal.x*p.radius,contact->y+p.collision->normal.y*p.radius};bounceParticle(p,*p.collision);collided=true;}}
 if(!collided){p.sprite.position.x+=p.velocityTick.x;p.sprite.position.y+=p.velocityTick.y;
  if(p.sprite.position.y<p.groundPosition&&p.velocity.y<0){p.sprite.position.y=p.groundPosition;p.velocity.y*= -.3f;p.velocity.x*=.7f;p.rotationVelocity*=.5f;}}
 p.sprite.rotation+=dt*p.rotationVelocity;p.sprite.scale.x+=dt*p.scaleVelocity;p.sprite.scale.y+=dt*p.scaleVelocity;
 if(p.velocityAdd!=0){p.velocity.x+=p.velocity.x*p.velocityAdd;p.velocity.y+=p.velocity.y*p.velocityAdd;}
 if(p.liveTime!=0){float elapsed=float(time-double(p.startTime));
  if(!(p.liveTime>elapsed)){p.show=false;if(p.selfRemove)p.removed=true;}
  else {p.sprite.alpha=1.0f-elapsed/p.liveTime;if(p.tileAnimation)p.sprite.frame=static_cast<int16_t>(int((1.0f-p.sprite.alpha)*float(p.animationFrames)+.5f));}}
 if(baseTick)baseTick();
}
// 0x11818: socket zero denotes sprite center offset, not the attachment point.
Vector2 partsSocketPosition(const PartsState& p,uint8_t socket){
 Vector2 v=p.base?partsSocketPosition(*p.base,p.baseSocket):Vector2{};
 if(!p.info||p.info->sockets.empty())return v;
 auto anchor=p.info->sockets[0];
 if(socket==0){v.x-=anchor.x;v.y-=anchor.y;}
 else if(socket<p.info->sockets.size()){v.x+=p.info->sockets[socket].x-anchor.x;v.y+=p.info->sockets[socket].y-anchor.y;}
 return v;
}
void attachParts(PartsState& p,const PartsState* base,uint8_t socket){p.base=base;p.baseSocket=socket;p.offset=partsSocketPosition(p,0);p.rotatedPosition=p.offset;p.basePosition=base?partsSocketPosition(*base,socket):Vector2{};}
void refreshPartsRotation(PartsState& p,float angle){if(p.rotation!=angle){p.rotation=angle;p.rotatedPosition=angle==0?p.offset:rotated(p.offset,angle);}}
// 0x11a48. Animation state changes during drawing, as in the binary.
DrawSprite drawParts(PartsState& p,Vector2 pos,float angle,float bounce,int state,float speed,double time,int z){
 refreshPartsRotation(p,angle);if(p.info){int type=p.info->partsType,count=p.info->tileCnt;
 if(type==0&&count>1){if(state==1){if(std::fabs(pos.x-p.previousX)>3){if(++p.tileFrame>=count)p.tileFrame=0;p.previousX=pos.x;}}
 else if(state==0&&speed!=0)p.tileFrame=shortTime(double(speed)*time*50.0)%2;}
 else if(type==3||type==4){if(count==0)throw std::invalid_argument("Animated part has zero tile count");
 if(type==4)p.rotation=float(double(p.rotation)-std::fmod(.5*time,2*std::numbers::pi));
 p.tileFrame=shortTime(200*time)%count;bounce*=.5f;}}
 auto d=partsSprite(p,z);d.frame=p.tileFrame;d.rotation=p.rotation;d.position={pos.x+p.rotatedPosition.x,pos.y+p.rotatedPosition.y+bounce};return d;
}
// 0x13530: parent anchor rotation and aim offset have independent update gates.
void refreshWeaponVisual(PartsState& p,WeaponState& w,bool player,int machType,float angle){
 if(p.rotation!=angle){p.rotation=angle;p.rotatedPosition=rotated(p.basePosition,angle);}
 if(w.aimDir==w.aimDirDest)return;
 p.offset=p.info&&!p.info->sockets.empty()?p.info->sockets[0]:Vector2{};
 refreshWeaponAim(w,player,angle);
 if(w.weaponInfo&&!w.weaponInfo->rotWpn&&!player)return;
 float a=float(std::atan2(double(p.offset.y),double(p.offset.x))+double(w.aimDir));
 if(machType!=3)a=float(double(a)-std::numbers::pi);
 float len=std::sqrt(p.offset.x*p.offset.x+p.offset.y*p.offset.y);
 w.weaponVec={std::cos(a),std::sin(a)};p.offset={len*w.weaponVec.x,len*w.weaponVec.y};
}
// 0x13874: recoil decays once per draw, not once per simulation tick.
DrawSprite drawWeapon(PartsState& p,WeaponState& w,Vector2 pos,bool player,int machType,float angle,float bounce,int z){
 refreshWeaponVisual(p,w,player,machType,angle);float rot=w.aimDir;
 if(w.weaponInfo&&!w.weaponInfo->rotWpn&&!player)rot=float(std::numbers::pi);
 if(machType!=3)rot=float(double(rot)-std::numbers::pi);
 auto d=partsSprite(p,z);d.rotation=rot;d.position={pos.x+p.rotatedPosition.x-p.offset.x+w.fireOffset.x,pos.y+p.rotatedPosition.y-p.offset.y+w.fireOffset.y+bounce*.5f};
 w.fireOffset.x+=(0-w.fireOffset.x)/5.0f;w.fireOffset.y+=(0-w.fireOffset.y)/5.0f;return d;
}
}
