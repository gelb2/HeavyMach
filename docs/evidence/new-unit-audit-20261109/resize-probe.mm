#include <CoreGraphics/CoreGraphics.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "vendor/stb/stb_image.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>
int main(int argc,char**argv){if(argc!=4&&argc!=8)return 2;int w,h,ch;unsigned char*p=stbi_load(argv[1],&w,&h,&ch,4);if(!p)return 3;int dw=atoi(argv[2]),dh=atoi(argv[3]);if(!dh)dh=std::lround(double(h)*dw/w);std::vector<unsigned char>src(p,p+size_t(w)*h*4);stbi_image_free(p);for(size_t i=0;i<src.size();i+=4)for(int c=0;c<3;c++)src[i+c]=static_cast<unsigned char>(float(src[i+c])*(float(src[i+3])/255.f));
if(argc==8){int x=atoi(argv[4]),y=atoi(argv[5]),cw=atoi(argv[6]),chh=atoi(argv[7]);if(x<0||y<0||x+cw>w||y+chh>h)return 4;std::vector<unsigned char>crop(size_t(cw)*chh*4);for(int row=0;row<chh;row++)std::copy_n(src.data()+(size_t(y+row)*w+x)*4,cw*4,crop.data()+size_t(row)*cw*4);src=std::move(crop);w=cw;h=chh;}
CGDataProviderRef provider=CGDataProviderCreateWithData(nullptr,src.data(),src.size(),nullptr);auto space=CGColorSpaceCreateDeviceRGB();auto decoded=CGImageCreate(w,h,8,32,size_t(w)*4,space,kCGBitmapByteOrder32Big|kCGImageAlphaPremultipliedLast,provider,nullptr,false,kCGRenderingIntentDefault);std::vector<unsigned char>apple(size_t(dw)*dh*4),area(apple.size());auto ctx=CGBitmapContextCreate(apple.data(),dw,dh,8,dw*4,space,kCGBitmapByteOrder32Big|kCGImageAlphaPremultipliedLast);CGContextSetInterpolationQuality(ctx,kCGInterpolationHigh);CGContextDrawImage(ctx,CGRectMake(0,0,dw,dh),decoded);
for(int y=0;y<dh;y++)for(int x=0;x<dw;x++){double x0=double(x)*w/dw,x1=double(x+1)*w/dw,y0=double(y)*h/dh,y1=double(y+1)*h/dh,sum[4]={},weight=0;for(int sy=int(y0);sy<std::min(h,int(std::ceil(y1)));sy++)for(int sx=int(x0);sx<std::min(w,int(std::ceil(x1)));sx++){double a=(std::min(x1,double(sx+1))-std::max(x0,double(sx)))*(std::min(y1,double(sy+1))-std::max(y0,double(sy)));for(int c=0;c<4;c++)sum[c]+=src[(size_t(sy)*w+sx)*4+c]*a;weight+=a;}for(int c=0;c<4;c++)area[(size_t(y)*dw+x)*4+c]=uint8_t(std::clamp(std::lround(sum[c]/weight),0L,255L));}
size_t changed=0;long long absSum=0;int maximum=0;for(size_t i=0;i<apple.size();i+=4){bool diff=false;for(int c=0;c<4;c++){int d=abs(int(apple[i+c])-int(area[i+c]));diff|=d>0;absSum+=d;maximum=std::max(maximum,d);}changed+=diff;}printf("{\"source_width\":%d,\"source_height\":%d,\"width\":%d,\"height\":%d,\"changed_pixels\":%zu,\"total_pixels\":%d,\"mean_absolute_channel_delta\":%.6f,\"maximum_channel_delta\":%d}\n",w,h,dw,dh,changed,dw*dh,double(absSum)/apple.size(),maximum);CGContextRelease(ctx);CGImageRelease(decoded);CGColorSpaceRelease(space);CGDataProviderRelease(provider);
}
