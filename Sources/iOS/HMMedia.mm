#import "HMResources.h"
#import "HMMedia.h"
#import "HMGLRenderer.h"
#import "HMOpenALAudio.h"
#import <objc/runtime.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#define STBI_NO_STDIO
#include "../../vendor/stb/stb_image.h"
static char HMOriginalRGBAKey;
NSData *HMOriginalImageRGBA(HMImage *image) { return objc_getAssociatedObject(image,&HMOriginalRGBAKey); }
#include <cmath>
#include <climits>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include "TrailRaster.hpp"
#include "AudioContract.hpp"
#include "TextureFormat.hpp"
#include "SpriteRaster.hpp"
struct HMTrailTexture {int width=0,height=0;std::vector<uint8_t> pixels;};
template<class Raster> static void HMDrawRaster(Raster& raster,CGPoint offset,CGFloat scale,BOOL hasClip,CGRect clip) {
    if(raster.empty())return;
    const size_t stride=size_t(raster.bounds.width)*4;
    // CPU output rows are y-up. CGImage display rows are top-down.
    for(int y=0;y<raster.bounds.height/2;++y)
        for(size_t x=0;x<stride;++x)std::swap(raster.rgba[size_t(y)*stride+x],raster.rgba[size_t(raster.bounds.height-1-y)*stride+x]);
    CFDataRef data=CFDataCreate(kCFAllocatorDefault,raster.rgba.data(),raster.rgba.size());
    CGDataProviderRef provider=CGDataProviderCreateWithCFData(data);
    CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();
    CGImageRef output=CGImageCreate(raster.bounds.width,raster.bounds.height,8,32,stride,space,CGBitmapInfo(kCGBitmapByteOrder32Big)|CGBitmapInfo(kCGImageAlphaPremultipliedLast),provider,nullptr,false,kCGRenderingIntentDefault);
    CGColorSpaceRelease(space);CGDataProviderRelease(provider);CFRelease(data);
    if(output){
        CGContextRef ctx=HMCurrentGraphicsContext();CGContextSaveGState(ctx);
        if(hasClip)CGContextClipToRect(ctx,CGRectMake(clip.origin.x,320-CGRectGetMaxY(clip),clip.size.width,clip.size.height));
        CGContextTranslateCTM(ctx,offset.x,320*(1-scale)-offset.y);CGContextScaleCTM(ctx,scale,scale);
        CGContextSetInterpolationQuality(ctx,kCGInterpolationNone);
        [[HMImage imageWithCGImage:output] drawInRect:CGRectMake(raster.bounds.x,320-raster.bounds.y-raster.bounds.height,raster.bounds.width,raster.bounds.height) blendMode:raster.blend==2?kCGBlendModePlusLighter:kCGBlendModeNormal alpha:1];
        CGContextRestoreGState(ctx);CGImageRelease(output);
    }
}
@implementation HMMedia {
    std::unordered_map<std::string,HMTrailTexture> _trailTextures;
    std::unordered_map<std::string,HMTrailTexture> _spriteTextures;
    NSMutableDictionary<NSString *,HMImage *> *_images;
    NSMutableSet<NSString *> *_rgb565Images;
    NSMutableDictionary<NSString *,NSValue *> *_sizes;
    NSUInteger _decodeCount;
    HMOpenALAudio *_audio;
    CGPoint _cameraOffset,_drawingOffset;
    CGFloat _drawingScale;
    BOOL _hasClip;
    CGRect _clipRect;
}
- (instancetype)init {if ((self=[super init])) {_drawingScale=1;_images=[NSMutableDictionary new];_sizes=[NSMutableDictionary new];_resourceScope=@"shared";_rgb565Images=[NSMutableSet new];_audio=[HMOpenALAudio new];if(![self prepareOriginalSounds])NSLog(@"Original OpenAL audio unavailable: %@",[_audio diagnostics]);}return self;}
@synthesize glRenderer=_glRenderer;
- (void)setGlRenderer:(HMGLRenderer *)renderer {
    _glRenderer=renderer;
    [_glRenderer setDrawingOffset:_drawingOffset];[_glRenderer setDrawingScale:_drawingScale];
    if(_hasClip)[_glRenderer setClipRect:_clipRect];else [_glRenderer resetClip];
}
- (void)setCameraOffset:(CGPoint)offset {_cameraOffset=offset;}
- (void)setDrawingOffset:(CGPoint)offset {
    if(!std::isfinite(offset.x)||!std::isfinite(offset.y))return;
    offset.x+=self.presentationOffset.x;offset.y+=self.presentationOffset.y;
    _drawingOffset=offset;[_glRenderer setDrawingOffset:offset];
}
- (void)setDrawingScale:(CGFloat)scale {
    if(!std::isfinite(scale)||scale<=0)return;
    _drawingScale=scale;[_glRenderer setDrawingScale:scale];
}
- (void)setClipRect:(CGRect)rect {
    if(!std::isfinite(rect.origin.x)||!std::isfinite(rect.origin.y)||!std::isfinite(rect.size.width)||!std::isfinite(rect.size.height)||rect.size.width<0||rect.size.height<0)rect=CGRectZero;
    rect=CGRectOffset(rect,self.presentationOffset.x,self.presentationOffset.y);
    _hasClip=YES;_clipRect=rect;[_glRenderer setClipRect:rect];
}
- (void)resetClip {_hasClip=NO;[_glRenderer resetClip];}
- (HMImage *)image:(NSString *)name {
    NSString *key=[self resourceKey:name];
    HMImage *image=_images[key];if(image)return image;
    // Unit PNGs are baked before packaging and use the common image path.
    NSString *fileName=name;
    BOOL extension=[name hasPrefix:@"Extensions/"];
    NSString *path=[HMGameResourceBundle() pathForResource:extension?fileName.lastPathComponent:fileName ofType:nil inDirectory:extension?@"Extensions":@"OriginalImages"];
    NSData *file=path?[NSData dataWithContentsOfFile:path]:nil;
    if(!file.length||file.length>INT_MAX){NSLog(@"Missing original image %@",name);return nil;}
    // Same stb revision/options and float truncation as touchHLE src/image.rs.
    stbi_convert_iphone_png_to_rgb_thread(1);stbi_set_unpremultiply_on_load_thread(1);
    int width=0,height=0,channels=0;
    unsigned char *pixels=stbi_load_from_memory((const stbi_uc*)file.bytes,(int)file.length,&width,&height,&channels,4);
    if(!pixels){NSLog(@"Original image decode failed %@: %s",name,stbi_failure_reason());return nil;}
    const unsigned char pngMagic[]={137,80,78,71,13,10,26,10};
    if(file.length>=8&&!memcmp(file.bytes,pngMagic,8)) {
        for(size_t i=0;i<size_t(width)*height*4;i+=4) {
            float alpha=float(pixels[i+3])/255.f;
            for(int c=0;c<3;++c)pixels[i+c]=static_cast<unsigned char>(float(pixels[i+c])*alpha);
        }
    }
    ++_decodeCount;
    NSData *rgba=[NSData dataWithBytes:pixels length:size_t(width)*height*4];stbi_image_free(pixels);
    BOOL rgb565=channels==3||channels==1;
    CGDataProviderRef provider=CGDataProviderCreateWithCFData((__bridge CFDataRef)rgba);
    CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo flags=CGBitmapInfo(kCGBitmapByteOrder32Big)|CGBitmapInfo(rgb565?kCGImageAlphaNoneSkipLast:kCGImageAlphaPremultipliedLast);
    CGImageRef decoded=CGImageCreate(width,height,8,32,size_t(width)*4,space,flags,provider,nullptr,false,kCGRenderingIntentDefault);
    // Preserve the existing auto-aim control sheet filter; unit art is already sized.
    int spriteWidth=0,spriteHeight=0;
    if([name isEqualToString:@"Extensions/AutoAimControls.png"])spriteWidth=138; // 3 columns of 46 logical pixels, cached once.
    if(decoded&&spriteWidth>0&&width>spriteWidth){
        int smallWidth=spriteWidth,smallHeight=spriteHeight?spriteHeight:std::max(1,int(std::lround(double(height)*spriteWidth/width)));
        NSMutableData *small=[NSMutableData dataWithLength:size_t(smallWidth)*smallHeight*4];
        CGContextRef context=CGBitmapContextCreate(small.mutableBytes,smallWidth,smallHeight,8,size_t(smallWidth)*4,space,kCGBitmapByteOrder32Big|kCGImageAlphaPremultipliedLast);
        if(context){
            CGContextSetInterpolationQuality(context,kCGInterpolationHigh);
            CGContextDrawImage(context,CGRectMake(0,0,smallWidth,smallHeight),decoded);
            CGImageRef reduced=CGBitmapContextCreateImage(context);CGContextRelease(context);
            if(reduced){CGImageRelease(decoded);decoded=reduced;rgba=small;width=smallWidth;height=smallHeight;}
        }
    }
    if(decoded){image=[HMImage imageWithCGImage:decoded];CGImageRelease(decoded);}
    CGColorSpaceRelease(space);CGDataProviderRelease(provider);
    if(image){objc_setAssociatedObject(image,&HMOriginalRGBAKey,rgba,OBJC_ASSOCIATION_RETAIN_NONATOMIC);_images[key]=image;_sizes[key]=HMSizeValue(image.size);if(rgb565)[_rgb565Images addObject:key];}
    return image;
}
- (NSString *)resourceKey:(NSString *)name {return [NSString stringWithFormat:@"%@|%@",self.resourceScope?:@"shared",name];}
- (BOOL)prepareTexture:(NSString *)name renderer:(HMGLRenderer *)renderer {
    NSString *key=[self resourceKey:name];BOOL rgb=[_rgb565Images containsObject:key];
    if([renderer hasTexture:key rgb565:rgb])return YES;
    HMImage *image=[self image:name];if(!image)return NO;
    _sizes[key]=HMSizeValue(image.size);
    if(![renderer prepareImage:image name:key rgb565:[_rgb565Images containsObject:key]])return NO;
    // Texture2D.initWithImage 0x69b0 frees its temporary pixels after upload.
    [_images removeObjectForKey:key];return YES;
}
- (CGSize)imageSize:(NSString *)name {
    NSString *key=[self resourceKey:name];
    if(self.textureRenderer){if(![self prepareTexture:name renderer:self.textureRenderer])return CGSizeZero;return HMValueSize(_sizes[key]);}
    return [self image:name].size;
}
- (CGSize)imageSize:(NSString *)name scope:(NSString *)scope {
    NSString *previous=self.resourceScope;self.resourceScope=scope;
    @try{return [self imageSize:name];}@finally{self.resourceScope=previous;}
}
- (void)releaseScope:(NSString *)scope {
    NSString *prefix=[scope stringByAppendingString:@"|"];
    for(NSString *key in [_sizes.allKeys copy])if([key hasPrefix:prefix]){[_sizes removeObjectForKey:key];[_images removeObjectForKey:key];[_rgb565Images removeObject:key];}
    std::string p(prefix.UTF8String);
    for(auto i=_spriteTextures.begin();i!=_spriteTextures.end();)if(i->first.starts_with(p))i=_spriteTextures.erase(i);else ++i;
    for(auto i=_trailTextures.begin();i!=_trailTextures.end();)if(i->first.starts_with(p))i=_trailTextures.erase(i);else ++i;
    [self.textureRenderer releaseTextureScope:scope];
}
- (NSDictionary *)cacheDiagnostics {
    NSUInteger bytes=0;for(HMImage* image in _images.allValues)bytes+=HMOriginalImageRGBA(image).length;
    return @{@"decodedImages":@(_images.count),@"decodedBytes":@(bytes),@"metadataCount":@(_sizes.count),@"decodeCount":@(_decodeCount),@"keys":_sizes.allKeys};
}
- (BOOL)prepareOriginalSounds { return [_audio prepareOriginalSounds]; }
- (void)sound:(NSString *)name { [self sound:name pan:0]; }
- (void)sound:(NSString *)name pan:(float)pan { [_audio sound:name pan:pan]; }
- (void)draw:(NSString *)name x:(CGFloat)x y:(CGFloat)y {
    [self draw:name x:x y:y columns:1 rows:1 tile:0 scaleX:1 scaleY:1 rotation:0 alpha:1];
}
- (void)draw:(NSString *)name x:(CGFloat)x y:(CGFloat)y columns:(NSInteger)columns rows:(NSInteger)rows tile:(NSInteger)tile scaleX:(CGFloat)sx scaleY:(CGFloat)sy rotation:(CGFloat)r alpha:(CGFloat)a {
    [self draw:name x:x y:y columns:columns rows:rows tile:tile scaleX:sx scaleY:sy rotation:r alpha:a blend:0];
}
- (void)draw:(NSString *)name x:(CGFloat)x y:(CGFloat)y columns:(NSInteger)columns rows:(NSInteger)rows tile:(NSInteger)tile scaleX:(CGFloat)sx scaleY:(CGFloat)sy rotation:(CGFloat)r alpha:(CGFloat)a blend:(NSInteger)blend {
    if (!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(sx)||!std::isfinite(sy)||!std::isfinite(r)||!std::isfinite(a)) return;
    if (columns<1 || rows<1 || tile<0) return;
    NSString *key=[self resourceKey:name];
    if (self.glRenderer) {
        if(![self prepareTexture:name renderer:self.glRenderer])return;
        [self.glRenderer drawImage:nil name:key columns:columns rows:rows tile:tile
            position:CGPointMake(x-_cameraOffset.x,y-_cameraOffset.y) scale:CGSizeMake(sx,sy)
            rotation:r alpha:a blend:blend rgb565:[_rgb565Images containsObject:key]];
        return;
    }
    HMImage *image=[self image:name];if(!image)return;
    auto& texture=_spriteTextures[key.UTF8String];
    if(texture.pixels.empty()) {
        texture.width=int(CGImageGetWidth(image.CGImage));texture.height=int(CGImageGetHeight(image.CGImage));
        texture.pixels.resize(size_t(texture.width)*texture.height*4);
        CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();
        CGContextRef bitmap=CGBitmapContextCreate(texture.pixels.data(),texture.width,texture.height,8,size_t(texture.width)*4,space,CGBitmapInfo(kCGBitmapByteOrder32Big)|CGBitmapInfo(kCGImageAlphaPremultipliedLast));
        CGColorSpaceRelease(space);if(!bitmap){texture.pixels.clear();return;}
        CGContextDrawImage(bitmap,CGRectMake(0,0,texture.width,texture.height),image.CGImage);CGContextRelease(bitmap);
    }
    heavymach::SpriteRasterInput input;
    input.columns=int(columns);input.rows=int(rows);input.tile=int(tile);
    input.position={float(x-_cameraOffset.x),float(y-_cameraOffset.y)};
    input.scale={float(sx),float(sy)};input.rotation=float(r);input.alpha=float(a);input.blend=int(blend);
    auto format=[_rgb565Images containsObject:key]?heavymach::SpriteTextureFormat::RGB565:heavymach::SpriteTextureFormat::PremultipliedRGBA8;
    auto raster=heavymach::rasterizeSprite(input,{texture.width,texture.height,size_t(texture.width)*4,texture.pixels,format});
    HMDrawRaster(raster,_drawingOffset,_drawingScale,_hasClip,_clipRect);
}

- (void)drawRegion:(NSString*)name source:(CGRect)source destination:(CGRect)destination {
    if(!self.glRenderer||![self prepareTexture:name renderer:self.glRenderer])return;
    NSString* key=[self resourceKey:name];
    [self.glRenderer drawRegion:key source:source destination:destination rgb565:[_rgb565Images containsObject:key]];
}
- (void)drawJoinedBackground:(NSString*)name x:(CGFloat)x y:(CGFloat)y scale:(CGFloat)scale rotation:(CGFloat)rotation {
    if(!self.glRenderer){[self draw:name x:x y:y columns:1 rows:1 tile:0 scaleX:scale scaleY:scale rotation:rotation alpha:1];return;}
    if(![self prepareTexture:name renderer:self.glRenderer])return;
    NSString* key=[self resourceKey:name];
    [self.glRenderer drawJoinedBackground:key position:CGPointMake(x-_cameraOffset.x,y-_cameraOffset.y) scale:scale rotation:rotation rgb565:[_rgb565Images containsObject:key]];
}
- (void)drawExpandedPanel:(NSString*)name x:(CGFloat)x y:(CGFloat)y left:(CGFloat)left right:(CGFloat)right top:(CGFloat)top bottom:(CGFloat)bottom {
    if(!self.glRenderer){[self draw:name x:x y:y];return;} // Original 480 comparison path.
    if(![self prepareTexture:name renderer:self.glRenderer])return;
    NSString* key=[self resourceKey:name];
    [self.glRenderer drawExpandedPanel:key position:CGPointMake(x,y) left:left right:right top:top bottom:bottom rgb565:[_rgb565Images containsObject:key]];
}
- (void)drawEdgeContinuation:(NSString*)name x:(CGFloat)x y:(CGFloat)y scale:(CGFloat)scale rotation:(CGFloat)rotation width:(CGFloat)width {
    if(!self.glRenderer||![self prepareTexture:name renderer:self.glRenderer])return;
    NSString* key=[self resourceKey:name];
    [self.glRenderer drawEdgeContinuation:key position:CGPointMake(x-_cameraOffset.x,y-_cameraOffset.y) scale:scale rotation:rotation width:width rgb565:[_rgb565Images containsObject:key]];
}
- (void)drawTrail:(const heavymach::TrailMesh&)mesh {
    if (self.glRenderer) {
        NSString *name=[NSString stringWithUTF8String:mesh.tile.c_str()];
        NSString *key=[self resourceKey:name];
        if([self prepareTexture:name renderer:self.glRenderer])[self.glRenderer drawTrail:mesh image:nil name:key rgb565:[_rgb565Images containsObject:key] cameraOffset:_cameraOffset];
        return;
    }
    auto& texture=_trailTextures[[self resourceKey:[NSString stringWithUTF8String:mesh.tile.c_str()]].UTF8String];
    if (texture.pixels.empty()) {
        HMImage *source=[self image:[NSString stringWithUTF8String:mesh.tile.c_str()]];
        if (!source) return;
        texture.width=int(CGImageGetWidth(source.CGImage));texture.height=int(CGImageGetHeight(source.CGImage));
        texture.pixels.resize(size_t(texture.width)*texture.height*4);
        CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();
        CGContextRef bitmap=CGBitmapContextCreate(texture.pixels.data(),texture.width,texture.height,8,size_t(texture.width)*4,space,kCGBitmapByteOrder32Big|kCGImageAlphaPremultipliedLast);
        CGColorSpaceRelease(space);if(!bitmap){texture.pixels.clear();return;}
        CGContextDrawImage(bitmap,CGRectMake(0,0,texture.width,texture.height),source.CGImage);CGContextRelease(bitmap);
    }
    auto adjusted=mesh;
    adjusted.translation.x-=float(_cameraOffset.x);adjusted.translation.y-=float(_cameraOffset.y);
    auto raster=heavymach::rasterizeTrail(adjusted,{texture.width,texture.height,size_t(texture.width)*4,texture.pixels});
    HMDrawRaster(raster,_drawingOffset,_drawingScale,_hasClip,_clipRect);
}
@end
