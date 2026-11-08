// Offline preservation baker. These filters reproduce the pre-hotfix media code.
// Build with the baseline checkout's stb and compatibility headers, not Core.
#include <CoreGraphics/CoreGraphics.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"
#include "HMCompatibilityPixels.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <zlib.h>
using Bytes=std::vector<uint8_t>;
static Bytes read(const std::string& p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("read: "+p);return Bytes(std::istreambuf_iterator<char>(f),{});}
static void write(const std::string& p,const Bytes& b){std::ofstream f(p,std::ios::binary);f.write((const char*)b.data(),b.size());if(!f)throw std::runtime_error("write: "+p);}
static Bytes decode(const Bytes& file,int& w,int& h){
 stbi_convert_iphone_png_to_rgb_thread(1);stbi_set_unpremultiply_on_load_thread(1);
 int channels;auto*p=stbi_load_from_memory(file.data(),int(file.size()),&w,&h,&channels,4);
 if(!p)throw std::runtime_error(stbi_failure_reason());Bytes out(p,p+size_t(w)*h*4);stbi_image_free(p);
 for(size_t i=0;i<out.size();i+=4){float a=float(out[i+3])/255.f;for(int c=0;c<3;c++)out[i+c]=static_cast<uint8_t>(float(out[i+c])*a);}return out;
}
// Exact web areaResize implementation from the baseline, including its accumulation order.
static Bytes webResize(const Bytes& src,int sw,int sh,int dw,int dh){
    Bytes out(size_t(dw)*dh*4);
    const double fx=double(sw)/dw,fy=double(sh)/dh;
    for(int y=0;y<dh;++y){
        const double y0=y*fy,y1=(y+1)*fy;
        for(int x=0;x<dw;++x){
            const double x0=x*fx,x1=(x+1)*fx;double acc[4]{},total=0;
            for(int sy=int(y0);sy<std::min(sh,int(std::ceil(y1)));++sy){
                const double wy=std::min(y1,double(sy+1))-std::max(y0,double(sy));if(wy<=0)continue;
                for(int sx=int(x0);sx<std::min(sw,int(std::ceil(x1)));++sx){
                    const double wx=std::min(x1,double(sx+1))-std::max(x0,double(sx));if(wx<=0)continue;
                    const double weight=wx*wy;const uint8_t* p=&src[(size_t(sy)*sw+sx)*4];
                    for(int c=0;c<4;++c)acc[c]+=p[c]*weight;total+=weight;
                }
            }
            for(int c=0;c<4;++c)out[(size_t(y)*dw+x)*4+c]=uint8_t(std::clamp(std::lround(acc[c]/total),0L,255L));
        }
    }
    return out;
}
static Bytes androidResize(const Bytes& rgba,int w,int h,int targetW,int targetH){
 std::vector<uint8_t> small(size_t(targetW)*targetH*4);
 for(int y=0;y<targetH;++y)for(int x=0;x<targetW;++x){double x0=double(x)*w/targetW,x1=double(x+1)*w/targetW,y0=double(y)*h/targetH,y1=double(y+1)*h/targetH;double sum[4]={};
  for(int sy=int(y0);sy<std::ceil(y1);++sy)for(int sx=int(x0);sx<std::ceil(x1);++sx){double weight=(std::min(x1,double(sx+1))-std::max(x0,double(sx)))*(std::min(y1,double(sy+1))-std::max(y0,double(sy)));for(int c=0;c<4;++c)sum[c]+=rgba[(size_t(std::min(h-1,sy))*w+std::min(w-1,sx))*4+c]*weight;}
  for(int c=0;c<4;++c)small[(size_t(y)*targetW+x)*4+c]=uint8_t(std::clamp(std::lround(sum[c]/((x1-x0)*(y1-y0))),0L,255L));}
 return small;
}
static Bytes crop(const Bytes& src,int w,int h,int x,int y,int cw,int ch){
 if(x<0||y<0||cw<1||ch<1||x+cw>w||y+ch>h)throw std::runtime_error("invalid crop");
 Bytes out(size_t(cw)*ch*4);for(int row=0;row<ch;row++)std::copy_n(src.data()+(size_t(row+y)*w+x)*4,cw*4,out.data()+size_t(row)*cw*4);return out;
}
static Bytes appleResize(const Bytes& src,int w,int h,int x,int y,int cw,int ch,int dw,int dh){
 auto provider=CGDataProviderCreateWithData(nullptr,src.data(),src.size(),nullptr);auto space=CGColorSpaceCreateDeviceRGB();
 auto decoded=CGImageCreate(w,h,8,32,size_t(w)*4,space,kCGBitmapByteOrder32Big|kCGImageAlphaPremultipliedLast,provider,nullptr,false,kCGRenderingIntentDefault);
 if(x||y||cw!=w||ch!=h){auto cropped=CGImageCreateWithImageInRect(decoded,CGRectMake(x,y,cw,ch));if(!cropped)throw std::runtime_error("CG crop failed");CGImageRelease(decoded);decoded=cropped;}
 Bytes small(size_t(dw)*dh*4);auto context=CGBitmapContextCreate(small.data(),dw,dh,8,size_t(dw)*4,space,kCGBitmapByteOrder32Big|kCGImageAlphaPremultipliedLast);
 if(!context)throw std::runtime_error("CG context failed");CGContextSetInterpolationQuality(context,kCGInterpolationHigh);CGContextDrawImage(context,CGRectMake(0,0,dw,dh),decoded);
 CGContextRelease(context);CGImageRelease(decoded);CGColorSpaceRelease(space);CGDataProviderRelease(provider);return small;
}
static void u32(Bytes& b,uint32_t v){for(int s=24;s>=0;s-=8)b.push_back(uint8_t(v>>s));}
static void chunk(Bytes& png,const char* name,const Bytes& payload){u32(png,uint32_t(payload.size()));size_t start=png.size();png.insert(png.end(),name,name+4);png.insert(png.end(),payload.begin(),payload.end());u32(png,uint32_t(crc32(0,png.data()+start,uInt(payload.size()+4))));}
static Bytes encodeExact(const Bytes& target,int w,int h){
 static const auto inverse=[](){std::array<std::array<int,256>,256> table{};for(int a=0;a<256;a++){table[a].fill(-1);for(int s=0;s<256;s++){int c=uint8_t(float(s)*(float(a)/255.f));if(table[a][c]<0)table[a][c]=s;}}return table;}();
 Bytes filtered;filtered.reserve(size_t(h)*(size_t(w)*4+1));
 for(int y=0;y<h;y++){filtered.push_back(0);for(int x=0;x<w;x++){auto* p=target.data()+(size_t(y)*w+x)*4;for(int c=0;c<3;c++){if(p[c]>p[3]||inverse[p[3]][p[c]]<0)throw std::runtime_error("target has no valid straight-alpha inverse");filtered.push_back(uint8_t(inverse[p[3]][p[c]]));}filtered.push_back(p[3]);}}
 uLongf n=compressBound(filtered.size());Bytes compressed(n);if(compress2(compressed.data(),&n,filtered.data(),filtered.size(),9)!=Z_OK)throw std::runtime_error("zlib failed");compressed.resize(n);
 Bytes png{137,80,78,71,13,10,26,10},ihdr;u32(ihdr,w);u32(ihdr,h);ihdr.insert(ihdr.end(),{8,6,0,0,0});chunk(png,"IHDR",ihdr);chunk(png,"IDAT",compressed);chunk(png,"IEND",{});return png;
}
static Bytes upload(const Bytes& rgba,int w,int h){int pw=1,ph=1;while(pw<w)pw*=2;while(ph<h)ph*=2;Bytes out(size_t(pw)*ph*4);for(int y=0;y<h;y++){auto* row=out.data()+size_t(y)*pw*4;std::copy_n(rgba.data()+size_t(y)*w*4,w*4,row);heavymach::compatibilityBitmapPixels(row,w);}return out;}
int main(int argc,char**argv){try{
 // profile source x y cropW cropH targetW targetH outputPNG evidencePrefix
 if(argc!=11)throw std::runtime_error("profile source x y cropW cropH targetW targetH outputPNG evidencePrefix");
 int w,h,x=atoi(argv[3]),y=atoi(argv[4]),cw=atoi(argv[5]),ch=atoi(argv[6]),dw=atoi(argv[7]),dh=atoi(argv[8]);auto source=decode(read(argv[2]),w,h);if(cw==0)cw=w;if(ch==0)ch=h;if(dh==0)dh=std::max(1,int(std::lround(double(ch)*dw/cw)));
 std::string profile=argv[1];Bytes target;if(profile=="apple")target=appleResize(source,w,h,x,y,cw,ch,dw,dh);else{auto cropped=crop(source,w,h,x,y,cw,ch);if(profile=="web")target=webResize(cropped,cw,ch,dw,dh);else if(profile=="android")target=androidResize(cropped,cw,ch,dw,dh);else throw std::runtime_error("unknown profile");}
 auto png=encodeExact(target,dw,dh);int rw,rh;auto roundtrip=decode(png,rw,rh);if(rw!=dw||rh!=dh||roundtrip!=target)throw std::runtime_error("PNG/common decode/premultiply roundtrip differs");auto baselineUpload=upload(target,dw,dh),candidateUpload=upload(roundtrip,rw,rh);if(candidateUpload!=baselineUpload)throw std::runtime_error("compatibility/POT upload differs");
 write(argv[9],png);std::string prefix=argv[10];write(prefix+".baseline.rgba",target);write(prefix+".decoded.rgba",roundtrip);write(prefix+".upload.rgba",candidateUpload);
 printf("{\"width\":%d,\"height\":%d,\"decode_equal\":true,\"upload_equal\":true,\"pixels\":%d}\n",dw,dh,dw*dh);
 return 0;
 }catch(const std::exception& e){fprintf(stderr,"%s\n",e.what());return 1;}}
