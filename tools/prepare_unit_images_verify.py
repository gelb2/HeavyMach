#!/usr/bin/env python3
"""Compile the actual image-loader methods in native host fixtures.

The source method bodies are extracted verbatim from baseline/candidate files.
Platform shells (bundle/filesystem/GL upload) are fixtures; this is not an app or
physical-device run. Apple uses real Foundation/CoreGraphics; stb and the pixel
compatibility header are the repository versions. No art is modified.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def method(text,start):
    begin=text.index(start);brace=text.index('{',begin);depth=0
    for i in range(brace,len(text)):
        if text[i]=='{':depth+=1
        elif text[i]=='}':
            depth-=1
            if depth==0:return text[begin:i+1]
    raise ValueError(start)
def run(command):return subprocess.run(list(map(str,command)),text=True,capture_output=True,check=True).stdout

COMMON=r'''
#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <stdexcept>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"
#include "HMCompatibilityPixels.hpp"
using Bytes=std::vector<uint8_t>;
static Bytes readBytes(const std::string& p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("read: "+p);return Bytes(std::istreambuf_iterator<char>(f),{});}
static void assertBytes(const Bytes& a,const Bytes& b,const std::string& name){if(a!=b)throw std::runtime_error("byte mismatch: "+name);}
static int pot(int n){int v=1;while(v<n)v*=2;return v;}
static Bytes uploaded(Bytes rgba,int w,int h){Bytes out(size_t(pot(w))*pot(h)*4);for(int y=0;y<h;y++){auto* q=out.data()+size_t(y)*pot(w)*4;std::copy_n(rgba.data()+size_t(y)*w*4,w*4,q);heavymach::compatibilityBitmapPixels(q,w);}return out;}
namespace heavymach {
struct V2 {float x,y;};
struct GroundAAImage {std::string name,source;std::array<int,4> crop;V2 size;float radarAnchorX,radarAnchorY;};
static const GroundAAImage* groundAAImage(const std::string& name){static const std::vector<GroundAAImage> rows{
#include "GroundAAArt.inc"
};for(const auto& r:rows)if(r.name==name)return &r;return nullptr;}
struct Spec {const char* tile;float width;};
static const std::array<Spec,3>& vtolSpecs(){static const std::array<Spec,3> s{{{"Extensions/VTOL_Yak141.png",112},{"Extensions/VTOL_Harrier.png",108},{"Extensions/VTOL_F35B.png",116}}};return s;}
static const std::array<Spec,3>& fighterSpecs(){static const std::array<Spec,3> s{{{"Extensions/Lux_F22.png",112},{"Extensions/Lux_Su57.png",116},{"Extensions/Lux_MiG31.png",120}}};return s;}
}
'''
APPLE=r'''
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <objc/runtime.h>
static char HMOriginalRGBAKey;
static NSValue* HMSizeValue(CGSize size){return [NSValue valueWithBytes:&size objCType:@encode(CGSize)];}
@interface HMImage:NSObject
@property(nonatomic,assign) CGImageRef CGImage;
@property(nonatomic,readonly) CGSize size;
+(HMImage*)imageWithCGImage:(CGImageRef)image;
@end
@implementation HMImage
+(HMImage*)imageWithCGImage:(CGImageRef)image {HMImage* out=[HMImage new];out.CGImage=CGImageRetain(image);return out;}
-(CGSize)size{return CGSizeMake(CGImageGetWidth(_CGImage),CGImageGetHeight(_CGImage));}
-(void)dealloc{CGImageRelease(_CGImage);}
@end
@interface TestBundle:NSObject
@property(nonatomic,copy) NSString* originals;
@property(nonatomic,copy) NSString* extensions;
-(NSString*)pathForResource:(NSString*)name ofType:(NSString*)type inDirectory:(NSString*)directory;
@end
@implementation TestBundle
-(NSString*)pathForResource:(NSString*)name ofType:(NSString*)type inDirectory:(NSString*)directory{return [([directory isEqualToString:@"Extensions"]?self.extensions:self.originals) stringByAppendingPathComponent:name];}
@end
static TestBundle* bundle;
static TestBundle* HMGameResourceBundle(){return bundle;}
@interface MediaBase:NSObject {
@public NSMutableDictionary* _images; NSMutableDictionary* _sizes; NSMutableSet* _rgb565Images; int _decodeCount;
}
-(NSString*)resourceKey:(NSString*)name;
@end
@implementation MediaBase
-(id)init{if((self=[super init])){_images=[NSMutableDictionary new];_sizes=[NSMutableDictionary new];_rgb565Images=[NSMutableSet new];}return self;}
-(NSString*)resourceKey:(NSString*)name{return name;}
@end
@interface BaselineMedia:MediaBase
-(HMImage*)image:(NSString*)name;
@end
@interface CandidateMedia:MediaBase
-(HMImage*)image:(NSString*)name;
@end
'''
APPLE_MAIN=r'''
static Bytes pixels(HMImage* image){if(!image)throw std::runtime_error("missing image");NSData* d=objc_getAssociatedObject(image,&HMOriginalRGBAKey);return Bytes((const uint8_t*)d.bytes,(const uint8_t*)d.bytes+d.length);}
int main(int argc,char**argv){@autoreleasepool{try{
 if(argc!=6)return 2;std::ifstream names(argv[4]);std::string name;int count=0;auto old=[BaselineMedia new];auto next=[CandidateMedia new];bundle=[TestBundle new];
 while(std::getline(names,name)){
  bundle.originals=[[NSString stringWithUTF8String:argv[1]] stringByAppendingPathComponent:@"Resources/OriginalImages"];bundle.extensions=[[NSString stringWithUTF8String:argv[1]] stringByAppendingPathComponent:@"Art/Extensions"];
  auto a=[old image:[NSString stringWithUTF8String:("Extensions/"+name).c_str()]];auto before=pixels(a);
  bundle.originals=[NSString stringWithUTF8String:argv[2]];auto b=[next image:[NSString stringWithUTF8String:name.c_str()]];auto after=pixels(b);
  if(a.size.width!=b.size.width||a.size.height!=b.size.height)throw std::runtime_error("size mismatch "+name);
  assertBytes(before,after,name);std::string prefix=std::string(argv[3])+"/"+name.substr(0,name.size()-4);
  assertBytes(before,readBytes(prefix+".baseline.rgba"),"baker baseline "+name);assertBytes(uploaded(after,int(b.size.width),int(b.size.height)),readBytes(prefix+".upload.rgba"),"upload "+name);count++;
 }
 printf("{\"profile\":\"apple\",\"actual_loader_method_pairs\":%d,\"decode_and_upload_equal\":true}\n",count);return count==63?0:3;
 }catch(const std::exception& e){fprintf(stderr,"%s\n",e.what());return 1;}}}
'''
WEB=r'''
namespace hmweb {
struct Size{double width,height;};
struct DecodedImage{int width,height;Bytes rgba;};
static bool startsWith(const std::string& s,const char* p){return s.rfind(p,0)==0;}
static std::string lastPathComponent(const std::string& s){return s.substr(s.find_last_of('/')+1);}
static bool readFile(const std::string& p,Bytes& out){out=readBytes(p);return true;}
class MediaBase {public:std::string assetRoot_;std::map<std::string,DecodedImage> images_;std::map<std::string,Size> sizes_;std::set<std::string> rgb565Images_;int decodeCount_=0;std::string resourceKey(const std::string& n){return n;}explicit MediaBase(std::string r):assetRoot_(r){};};
class BaselineMedia:public MediaBase{public:using MediaBase::MediaBase;DecodedImage* image(const std::string& name);};
class CandidateMedia:public MediaBase{public:using MediaBase::MediaBase;DecodedImage* image(const std::string& name);};
'''
WEB_MAIN=r'''
}
int main(int argc,char**argv){try{if(argc!=5)return 2;hmweb::BaselineMedia old(argv[1]);hmweb::CandidateMedia next(argv[2]);std::ifstream names(argv[4]);std::string name;int count=0;
 while(std::getline(names,name)){auto* a=old.image("Extensions/"+name);auto* b=next.image(name);if(a->width!=b->width||a->height!=b->height)throw std::runtime_error("size "+name);assertBytes(a->rgba,b->rgba,name);std::string prefix=std::string(argv[3])+"/"+name.substr(0,name.size()-4);assertBytes(a->rgba,readBytes(prefix+".baseline.rgba"),"baker baseline "+name);assertBytes(uploaded(b->rgba,b->width,b->height),readBytes(prefix+".upload.rgba"),"upload "+name);count++;}
 printf("{\"profile\":\"web\",\"actual_loader_method_pairs\":%d,\"decode_and_upload_equal\":true}\n",count);return count==63?0:3;
 }catch(const std::exception& e){fprintf(stderr,"%s\n",e.what());return 1;}}
'''
ANDROID=r'''
using GLuint=unsigned int;
constexpr int EGL_NO_CONTEXT=0,GL_TEXTURE_2D=0,GL_UNPACK_ALIGNMENT=0,GL_TEXTURE_MIN_FILTER=0,GL_TEXTURE_MAG_FILTER=0,GL_LINEAR=0,GL_TEXTURE_WRAP_S=0,GL_TEXTURE_WRAP_T=0,GL_REPEAT=0,GL_RGB=0,GL_RGBA=0,GL_UNSIGNED_SHORT_5_6_5=0,GL_UNSIGNED_BYTE=0;
static void glGenTextures(int,GLuint*){}static void glBindTexture(int,GLuint){}static void glPixelStorei(int,int){}static void glTexParameteri(int,int,int){}static void glTexImage2D(int,int,int,int,int,int,int,int,const void*){}
namespace heavymach {static uint16_t packOriginalRGB565(uint8_t r,uint8_t g,uint8_t b){return uint16_t((r>>3)<<11|(g>>2)<<5|(b>>3));}}
namespace hm {
static Bytes assetBytes(const std::string& root,const std::string& name){return readBytes(root+"/"+name);}
class MediaBase {public:struct Texture{int width=0,height=0,pw=0,ph=0;bool rgb=false;Bytes pixels;GLuint id=0;};std::string assets_,resourceScope;std::map<std::string,Texture> textures_;int context_=EGL_NO_CONTEXT,uploads=0;explicit MediaBase(std::string r):assets_(r){};};
class BaselineMedia:public MediaBase{public:using MediaBase::MediaBase;Texture* texture(const std::string& name);};
class CandidateMedia:public MediaBase{public:using MediaBase::MediaBase;Texture* texture(const std::string& name);};
'''
ANDROID_MAIN=r'''
}
int main(int argc,char**argv){try{if(argc!=5)return 2;hm::BaselineMedia old(argv[1]);hm::CandidateMedia next(argv[2]);std::ifstream names(argv[4]);std::string name;int count=0;
 while(std::getline(names,name)){auto* a=old.texture("Extensions/"+name);auto* b=next.texture(name);if(a->width!=b->width||a->height!=b->height||a->pw!=b->pw||a->ph!=b->ph||a->rgb!=b->rgb)throw std::runtime_error("size/format "+name);assertBytes(a->pixels,b->pixels,name);std::string prefix=std::string(argv[3])+"/"+name.substr(0,name.size()-4);assertBytes(b->pixels,readBytes(prefix+".upload.rgba"),"baker upload "+name);count++;}
 printf("{\"profile\":\"android\",\"actual_loader_method_pairs\":%d,\"upload_equal\":true}\n",count);return count==63?0:3;
 }catch(const std::exception& e){fprintf(stderr,"%s\n",e.what());return 1;}}
'''
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--baseline-root',type=Path,required=True);p.add_argument('--evidence',type=Path,default=ROOT/'build/unit-images/evidence');a=p.parse_args();old=a.baseline_root.resolve();e=a.evidence.resolve();e.mkdir(parents=True,exist_ok=True)
    m=json.loads((ROOT/'Resources/RuntimeImages/manifest.json').read_text());names=e/'names.txt';names.write_text('\n'.join(x['file'] for x in m['profiles']['apple'])+'\n')
    sources={};source_bodies={}
    rel='Sources/iOS/HMMedia.mm';before=method((old/rel).read_text(),'- (HMImage *)image:');after=method((ROOT/rel).read_text(),'- (HMImage *)image:');sources['apple']=COMMON+APPLE+'\n@implementation BaselineMedia\n'+before+'\n@end\n@implementation CandidateMedia\n'+after+'\n@end\n'+APPLE_MAIN;source_bodies['apple']={'path':rel,'baseline_method_sha256':hashlib.sha256(before.encode()).hexdigest(),'candidate_method_sha256':hashlib.sha256(after.encode()).hexdigest()}
    rel='Sources/Web/WebMedia.cpp';text=(old/rel).read_text();before=method(text,'DecodedImage* Media::image(');after=method((ROOT/rel).read_text(),'DecodedImage* Media::image(');resize=method(text,'std::vector<uint8_t> areaResize(');sources['web']=COMMON+WEB+resize+'\n'+before.replace('Media::image','BaselineMedia::image',1)+'\n'+after.replace('Media::image','CandidateMedia::image',1)+WEB_MAIN;source_bodies['web']={'path':rel,'baseline_method_sha256':hashlib.sha256(before.encode()).hexdigest(),'candidate_method_sha256':hashlib.sha256(after.encode()).hexdigest()}
    rel='Android/app/src/main/cpp/Media.cpp';before=method((old/rel).read_text(),'Media::Texture* Media::texture(');after=method((ROOT/rel).read_text(),'Media::Texture* Media::texture(');sources['android']=COMMON+ANDROID+before.replace('Media::','BaselineMedia::')+'\n'+after.replace('Media::','CandidateMedia::')+ANDROID_MAIN;source_bodies['android']={'path':rel,'baseline_method_sha256':hashlib.sha256(before.encode()).hexdigest(),'candidate_method_sha256':hashlib.sha256(after.encode()).hexdigest()}
    basefiles=e/'baseline-files';basefiles.mkdir(exist_ok=True)
    for name,destination in [('OriginalImages',old/'Resources/OriginalImages'),('Extensions',old/'Art/Extensions')]:
        f=basefiles/name
        if not f.exists():f.symlink_to(destination,target_is_directory=True)
    results=[]
    for profile,source in sources.items():
        code=e/('actual-loader-'+profile+('.mm' if profile=='apple' else '.cpp'));code.write_text(source);exe=e/('actual-loader-'+profile)
        cmd=['xcrun','clang++','-std=c++20','-O2','-ffp-contract=off',code,'-I',old/'vendor/stb','-I',old/'Sources/iOS','-I',old/'Sources/Core','-o',exe]
        if profile=='apple':cmd+=['-fobjc-arc','-framework','Foundation','-framework','CoreGraphics']
        try:run(cmd)
        except subprocess.CalledProcessError as err:raise SystemExit(err.stderr)
        candidate=e/('candidate-'+profile);candidate.mkdir(exist_ok=True);link=candidate/'OriginalImages'
        if not link.exists():link.symlink_to(ROOT/'Resources/RuntimeImages'/profile,target_is_directory=True)
        args=[exe,old if profile=='apple' else basefiles,ROOT/'Resources/RuntimeImages'/profile if profile=='apple' else candidate,e/profile,names]
        if profile=='apple':args+=['unused']
        try:result=json.loads(run(args))
        except subprocess.CalledProcessError as err:raise SystemExit(err.stderr)
        result.update(source_bodies[profile]);result.update({'fixture_sha256':sha(code),'compile_command':list(map(str,cmd)),'command':list(map(str,args))});results.append(result)
    report={'scope':'Verbatim actual pre/post loader method bodies compiled in native fixtures; platform shell mocks, no device/GPU execution','profiles':results,'total_actual_method_pairs':sum(r['actual_loader_method_pairs'] for r in results)}
    (e/'actual-loader-verification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
if __name__=='__main__':main()
