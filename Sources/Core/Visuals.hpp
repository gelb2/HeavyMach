#pragma once
#include "Combat.hpp"
#include <functional>
namespace heavymach {
// World coordinates: +Y up. Rotation radians. Renderer applies camera and Y flip.
struct DrawSprite {
 std::string tile; int frame=0,cols=1,rows=1;
 Vector2 position{},scale{1,1}; float rotation=0,alpha=1; int blend=0,z=0;
 bool worldTexture=false; // GobWorld owns map background/object tiles separately.
};
struct ParticleState {
 DrawSprite sprite; Vector2 velocity{},velocityTick{};
 float gravity=0,radius=10,rotationVelocity=0,scaleVelocity=0,velocityAdd=0;
 float liveTime=0,startTime=0,groundPosition=-1000;
 const ColVector* collision=nullptr;
 int animationFrames=0; bool tileAnimation=false,selfRemove=false,show=false,removed=false;
};
void resetParticle(ParticleState&,double time);
void startParticle(ParticleState&,double time);
void bounceParticle(ParticleState&,const ColVector&);
// Base Qob tick (children) is a host callback; no pause check exists in this override.
void tickParticle(ParticleState&,double time,const std::function<void()>& baseTick={});
struct PartsState {
 const PartsInfo* info=nullptr; const PartsState* base=nullptr; uint8_t baseSocket=0;
 Vector2 basePosition{},offset{},rotatedPosition{};
 float rotation=999,previousX=0; // 0x114d8 sentinel forces first anchor refresh.
 int tileFrame=0;
};
// Common optional presentation contract; original parts have no override.
const PartFrame* presentationFrame(const PartsState&,double worldTime);
void applyPartPresentation(DrawSprite&,const PartsState&,Vector2 ownerPosition,
                           float ownerRotation,float verticalVelocity,double worldTime,
                           Vector2 textureSize);
Vector2 rotateAttachment(Vector2,float);
float velocityPitch(float previous,float horizontal,float vertical,const MachineBehavior&);
Vector2 partsSocketPosition(const PartsState&,uint8_t socket);
void attachParts(PartsState&,const PartsState* base,uint8_t socket);
void refreshPartsRotation(PartsState&,float parentRotation);
DrawSprite drawParts(PartsState&,Vector2 worldPosition,float parentRotation,float engineBounce,
                     int machState,float speed,double time,int z=0);
// Calls Combat's aim smoothing exactly once; do not additionally call refreshWeaponAim.
void refreshWeaponVisual(PartsState&,WeaponState&,bool player,int machType,float parentRotation);
DrawSprite drawWeapon(PartsState&,WeaponState&,Vector2 worldPosition,bool player,int machType,
                      float parentRotation,float engineBounce,int z=0);
}
