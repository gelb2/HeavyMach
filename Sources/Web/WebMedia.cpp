#include "WebMedia.hpp"
#include "WebAudio.hpp"
#include "WebGLRenderer.hpp"
#include "Fighters.hpp"
#include "GroundAA.hpp"
#include "VTOL.hpp"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#define STBI_NO_STDIO
#include "../../vendor/stb/stb_image.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>

namespace hmweb {
namespace {
bool startsWith(const std::string& s,const char* prefix){return s.rfind(prefix,0)==0;}
std::string lastPathComponent(const std::string& s){auto p=s.find_last_of('/');return p==std::string::npos?s:s.substr(p+1);}
bool readFile(const std::string& path,std::vector<uint8_t>& out){
    std::ifstream file(path,std::ios::binary);if(!file)return false;
    out.assign(std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>());return true;
}
// iOS reduces the new aircraft/auto-aim sheets once with CoreGraphics
// kCGInterpolationHigh. CoreGraphics is unavailable here; an area-average
// filter over the premultiplied pixels is the web substitute (documented in
// docs/web-port-20261028.md). It runs once per load, never per frame.
std::vector<uint8_t> areaResize(const std::vector<uint8_t>& src,int sw,int sh,int dw,int dh){
    std::vector<uint8_t> out(size_t(dw)*dh*4);
    const double fx=double(sw)/dw,fy=double(sh)/dh;
    for(int y=0;y<dh;++y){
        const double y0=y*fy,y1=(y+1)*fy;
        for(int x=0;x<dw;++x){
            const double x0=x*fx,x1=(x+1)*fx;double acc[4]{},total=0;
            for(int sy=int(y0);sy<std::min(sh,int(std::ceil(y1)));++sy){
                const double wy=std::min(y1,double(sy+1))-std::max(y0,double(sy));if(wy<=0)continue;
                for(int sx=int(x0);sx<std::min(sw,int(std::ceil(x1)));++sx){
                    const double wx=std::min(x1,double(sx+1))-std::max(x0,double(sx));if(wx<=0)continue;
                    const double w=wx*wy;const uint8_t* p=&src[(size_t(sy)*sw+sx)*4];
                    for(int c=0;c<4;++c)acc[c]+=p[c]*w;total+=w;
                }
            }
            for(int c=0;c<4;++c)out[(size_t(y)*dw+x)*4+c]=uint8_t(std::clamp(std::lround(acc[c]/total),0L,255L));
        }
    }
    return out;
}
}

Media::Media(Audio& audio,std::string assetRoot):audio_(audio),assetRoot_(std::move(assetRoot)){
    if(!audio_.prepareOriginalSounds())std::fprintf(stderr,"Original OpenAL audio unavailable: %s\n",audio_.diagnostics().c_str());
}
void Media::setGlRenderer(GLRenderer* renderer){
    glRenderer_=renderer;if(!glRenderer_)return;
    glRenderer_->setDrawingOffset(drawingOffset_);glRenderer_->setDrawingScale(drawingScale_);
    if(hasClip_)glRenderer_->setClipRect(clipRect_);else glRenderer_->resetClip();
}
void Media::setCameraOffset(Point offset){cameraOffset_=offset;}
void Media::setDrawingOffset(Point offset){
    if(!std::isfinite(offset.x)||!std::isfinite(offset.y))return;
    offset.x+=presentationOffset.x;offset.y+=presentationOffset.y;
    drawingOffset_=offset;if(glRenderer_)glRenderer_->setDrawingOffset(offset);
}
void Media::setDrawingScale(double scale){
    if(!std::isfinite(scale)||scale<=0)return;
    drawingScale_=scale;if(glRenderer_)glRenderer_->setDrawingScale(scale);
}
void Media::setClipRect(Rect rect){
    if(!std::isfinite(rect.origin.x)||!std::isfinite(rect.origin.y)||!std::isfinite(rect.size.width)||!std::isfinite(rect.size.height)||rect.size.width<0||rect.size.height<0)rect=Rect{};
    rect=offset(rect,presentationOffset.x,presentationOffset.y);
    hasClip_=true;clipRect_=rect;if(glRenderer_)glRenderer_->setClipRect(rect);
}
void Media::resetClip(){hasClip_=false;if(glRenderer_)glRenderer_->resetClip();}
std::string Media::resourceKey(const std::string& name) const{return (resourceScope.empty()?std::string("shared"):resourceScope)+"|"+name;}

DecodedImage* Media::image(const std::string& name){
    const std::string key=resourceKey(name);
    if(auto found=images_.find(key);found!=images_.end())return &found->second;
    // Generated extensions are separate resources; never replace OriginalImages.
    // Ground AA parts are crops of shared source sheets (GroundAAArt.inc).
    const auto* groundArt=heavymach::groundAAImage(name);
    const std::string fileName=groundArt?groundArt->source:name;
    const bool extension=startsWith(name,"Extensions/");
    const std::string path=assetRoot_+(extension?"/Extensions/"+lastPathComponent(fileName):"/OriginalImages/"+fileName);
    std::vector<uint8_t> file;
    if(!readFile(path,file)||file.empty()||file.size()>size_t(INT_MAX)){std::fprintf(stderr,"Missing original image %s\n",name.c_str());return nullptr;}
    // Same stb revision/options and float truncation as touchHLE src/image.rs.
    stbi_convert_iphone_png_to_rgb_thread(1);stbi_set_unpremultiply_on_load_thread(1);
    int width=0,height=0,channels=0;
    unsigned char* pixels=stbi_load_from_memory(file.data(),int(file.size()),&width,&height,&channels,4);
    if(!pixels){std::fprintf(stderr,"Original image decode failed %s: %s\n",name.c_str(),stbi_failure_reason());return nullptr;}
    const unsigned char pngMagic[]={137,80,78,71,13,10,26,10};
    if(file.size()>=8&&!std::memcmp(file.data(),pngMagic,8)){
        for(size_t i=0;i<size_t(width)*height*4;i+=4){
            float alpha=float(pixels[i+3])/255.f;
            for(int c=0;c<3;++c)pixels[i+c]=static_cast<unsigned char>(float(pixels[i+c])*alpha);
        }
    }
    ++decodeCount_;
    DecodedImage decoded;decoded.width=width;decoded.height=height;
    decoded.rgba.assign(pixels,pixels+size_t(width)*height*4);stbi_image_free(pixels);
    const bool rgb565=channels==3||channels==1;
    // Prefilter new aircraft to their logical drawing size once. Sampling a
    // 512px texture into ~110 scene pixels skipped thin edges and fine details.
    int spriteWidth=0,spriteHeight=0;
    // HMMedia: CGImageCreateWithImageInRect on the decoded sheet, then one resize to the logical size.
    if(groundArt){
        const auto& r=groundArt->crop;
        if(r[0]>=0&&r[1]>=0&&r[2]>0&&r[3]>0&&r[0]+r[2]<=width&&r[1]+r[3]<=height){
            std::vector<uint8_t> cropped(size_t(r[2])*r[3]*4);
            for(int y=0;y<r[3];++y)std::memcpy(&cropped[size_t(y)*r[2]*4],&decoded.rgba[(size_t(r[1]+y)*width+r[0])*4],size_t(r[2])*4);
            decoded.rgba=std::move(cropped);decoded.width=width=r[2];decoded.height=height=r[3];
        }
        spriteWidth=int(groundArt->size.x);spriteHeight=int(groundArt->size.y);
    }
    if(startsWith(name,"Extensions/VTOL_"))for(const auto& spec:heavymach::vtolSpecs())
        if(name==spec.tile){spriteWidth=int(std::lround(spec.width));break;}
    if(startsWith(name,"Extensions/Lux_"))for(const auto& spec:heavymach::fighterSpecs())
        if(name==spec.tile){spriteWidth=int(std::lround(spec.width));break;}
    if(name=="Extensions/AutoAimControls.png")spriteWidth=138; // 3 columns of 46 logical pixels, cached once.
    if(spriteWidth>0&&width>spriteWidth){
        int smallWidth=spriteWidth,smallHeight=spriteHeight?spriteHeight:std::max(1,int(std::lround(double(height)*spriteWidth/width)));
        decoded.rgba=areaResize(decoded.rgba,width,height,smallWidth,smallHeight);decoded.width=smallWidth;decoded.height=smallHeight;
    }
    auto& stored=images_[key]=std::move(decoded);
    sizes_[key]={double(stored.width),double(stored.height)};
    if(rgb565)rgb565Images_.insert(key);
    return &stored;
}
bool Media::prepareTexture(const std::string& name,GLRenderer& renderer){
    const std::string key=resourceKey(name);const bool rgb=rgb565Images_.contains(key);
    if(renderer.hasTexture(key,rgb))return true;
    DecodedImage* decoded=image(name);if(!decoded)return false;
    sizes_[key]={double(decoded->width),double(decoded->height)};
    if(!renderer.prepareImage(*decoded,key,rgb565Images_.contains(key)))return false;
    // Texture2D.initWithImage 0x69b0 frees its temporary pixels after upload.
    images_.erase(key);return true;
}
Size Media::imageSize(const std::string& name){
    const std::string key=resourceKey(name);
    if(textureRenderer){if(!prepareTexture(name,*textureRenderer))return {};return sizes_[key];}
    DecodedImage* decoded=image(name);return decoded?Size{double(decoded->width),double(decoded->height)}:Size{};
}
Size Media::imageSize(const std::string& name,const std::string& scope){
    std::string previous=resourceScope;resourceScope=scope;
    Size size=imageSize(name);resourceScope=previous;return size;
}
void Media::releaseScope(const std::string& scope){
    const std::string prefix=scope+"|";
    for(auto i=sizes_.begin();i!=sizes_.end();){if(i->first.starts_with(prefix))i=sizes_.erase(i);else ++i;}
    for(auto i=images_.begin();i!=images_.end();){if(i->first.starts_with(prefix))i=images_.erase(i);else ++i;}
    for(auto i=rgb565Images_.begin();i!=rgb565Images_.end();){if(i->starts_with(prefix))i=rgb565Images_.erase(i);else ++i;}
    if(textureRenderer)textureRenderer->releaseTextureScope(scope);
}
void Media::sound(const std::string& name,float pan){audio_.sound(name,pan);}
void Media::draw(const std::string& name,double x,double y){draw(name,x,y,1,1,0,1,1,0,1);}
void Media::draw(const std::string& name,double x,double y,long columns,long rows,long tile,double sx,double sy,double r,double a,long blend){
    if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(sx)||!std::isfinite(sy)||!std::isfinite(r)||!std::isfinite(a))return;
    if(columns<1||rows<1||tile<0)return;
    if(!glRenderer_)return; // The CPU comparison raster is not part of the web product.
    const std::string key=resourceKey(name);
    if(!prepareTexture(name,*glRenderer_))return;
    glRenderer_->drawImage(key,columns,rows,tile,{x-cameraOffset_.x,y-cameraOffset_.y},{sx,sy},r,a,blend,rgb565Images_.contains(key));
}
void Media::drawRegion(const std::string& name,Rect source,Rect destination){
    if(!glRenderer_||!prepareTexture(name,*glRenderer_))return;
    const std::string key=resourceKey(name);
    glRenderer_->drawRegion(key,source,destination,rgb565Images_.contains(key));
}
void Media::drawJoinedBackground(const std::string& name,double x,double y,double scale,double rotation){
    if(!glRenderer_)return;
    if(!prepareTexture(name,*glRenderer_))return;
    const std::string key=resourceKey(name);
    glRenderer_->drawJoinedBackground(key,{x-cameraOffset_.x,y-cameraOffset_.y},scale,rotation,rgb565Images_.contains(key));
}
void Media::drawExpandedPanel(const std::string& name,double x,double y,double left,double right,double top,double bottom){
    if(!glRenderer_)return;
    if(!prepareTexture(name,*glRenderer_))return;
    const std::string key=resourceKey(name);
    glRenderer_->drawExpandedPanel(key,{x,y},left,right,top,bottom,rgb565Images_.contains(key));
}
void Media::drawEdgeContinuation(const std::string& name,double x,double y,double scale,double rotation,double width){
    if(!glRenderer_||!prepareTexture(name,*glRenderer_))return;
    const std::string key=resourceKey(name);
    glRenderer_->drawEdgeContinuation(key,{x-cameraOffset_.x,y-cameraOffset_.y},scale,rotation,width,rgb565Images_.contains(key));
}
void Media::drawTrail(const heavymach::TrailMesh& mesh){
    if(!glRenderer_)return;
    const std::string key=resourceKey(mesh.tile);
    if(prepareTexture(mesh.tile,*glRenderer_))glRenderer_->drawTrail(mesh,key,rgb565Images_.contains(key),cameraOffset_);
}
}
