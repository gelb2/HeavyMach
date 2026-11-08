#import <XCTest/XCTest.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CommonCrypto/CommonDigest.h>
#import <TargetConditionals.h>
#import "HMMedia.h"
#import "HMResources.h"
#include "HMCompatibilityPixels.hpp"
// Isolated test decoder: same stb revision/options as the pre-hotfix loader.
// Static linkage prevents symbols/flags from changing the production decoder.
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "../../vendor/stb/stb_image.h"
#include <algorithm>
#include <climits>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace {
using Bytes=std::vector<uint8_t>;
static NSString* sha(NSData* data){
 if(!data)return @"missing";
 unsigned char digest[CC_SHA256_DIGEST_LENGTH];CC_SHA256(data.bytes,CC_LONG(data.length),digest);
 NSMutableString* result=[NSMutableString stringWithCapacity:64];for(auto byte:digest)[result appendFormat:@"%02x",byte];return result;
}
static NSData* bytesData(const Bytes& bytes){return [NSData dataWithBytes:bytes.data() length:bytes.size()];}
static Bytes decodeBaseline(NSData* file,int& width,int& height){
 if(!file.length||file.length>INT_MAX)throw std::runtime_error("Missing/oversized baseline source");
 stbi_convert_iphone_png_to_rgb_thread(1);stbi_set_unpremultiply_on_load_thread(1);
 int channels=0;auto*p=stbi_load_from_memory((const stbi_uc*)file.bytes,int(file.length),&width,&height,&channels,4);
 if(!p)throw std::runtime_error(stbi_failure_reason());
 Bytes rgba(p,p+size_t(width)*height*4);stbi_image_free(p);
 for(size_t i=0;i<rgba.size();i+=4){float alpha=float(rgba[i+3])/255.f;for(int c=0;c<3;++c)rgba[i+c]=static_cast<uint8_t>(float(rgba[i+c])*alpha);}
 return rgba;
}
// Original HMMedia crop + High interpolation, executed on the current Apple OS.
// No expected bitmap is substituted for this calculation.
static Bytes baselineHigh(NSData* file,NSArray* crop,int dw,int dh){
 int width=0,height=0;auto rgba=decodeBaseline(file,width,height);
 if(crop.count!=4||dw<1||dh<1)throw std::runtime_error("Invalid image contract");
 int x=[crop[0] intValue],y=[crop[1] intValue],cw=[crop[2] intValue],ch=[crop[3] intValue];if(!cw)cw=width;if(!ch)ch=height;
 if(x<0||y<0||cw<1||ch<1||x+cw>width||y+ch>height)throw std::runtime_error("Invalid baseline crop");
 auto provider=CGDataProviderCreateWithData(nullptr,rgba.data(),rgba.size(),nullptr);auto space=CGColorSpaceCreateDeviceRGB();
 auto decoded=CGImageCreate(width,height,8,32,size_t(width)*4,space,CGBitmapInfo(kCGBitmapByteOrder32Big)|CGBitmapInfo(kCGImageAlphaPremultipliedLast),provider,nullptr,false,kCGRenderingIntentDefault);
 if(!decoded){CGColorSpaceRelease(space);CGDataProviderRelease(provider);throw std::runtime_error("Baseline CGImage creation failed");}
 if(x||y||cw!=width||ch!=height){auto cropped=CGImageCreateWithImageInRect(decoded,CGRectMake(x,y,cw,ch));CGImageRelease(decoded);decoded=cropped;}
 if(!decoded){CGColorSpaceRelease(space);CGDataProviderRelease(provider);throw std::runtime_error("Baseline CG crop failed");}
 Bytes result(size_t(dw)*dh*4);
 auto context=CGBitmapContextCreate(result.data(),dw,dh,8,size_t(dw)*4,space,CGBitmapInfo(kCGBitmapByteOrder32Big)|CGBitmapInfo(kCGImageAlphaPremultipliedLast));
 if(!context){CGImageRelease(decoded);CGColorSpaceRelease(space);CGDataProviderRelease(provider);throw std::runtime_error("Baseline CG bitmap failed");}
 CGContextSetInterpolationQuality(context,kCGInterpolationHigh);CGContextDrawImage(context,CGRectMake(0,0,dw,dh),decoded);
 CGContextRelease(context);CGImageRelease(decoded);CGColorSpaceRelease(space);CGDataProviderRelease(provider);return result;
}
static Bytes uploadContract(NSData* rgba,int width,int height){
 if(rgba.length!=size_t(width)*height*4)throw std::runtime_error("Decoded byte size mismatch");
 int pw=1,ph=1;while(pw<width)pw*=2;while(ph<height)ph*=2;Bytes result(size_t(pw)*ph*4);
 for(int y=0;y<height;++y){auto* row=result.data()+size_t(y)*pw*4;std::copy_n((const uint8_t*)rgba.bytes+size_t(y)*width*4,width*4,row);heavymach::compatibilityBitmapPixels(row,width);}return result;
}
}

@interface HMUnitImagePreservationTests:XCTestCase
@end
@implementation HMUnitImagePreservationTests
- (void)testAll63UnitImagesMatchDeviceBaselineHighFilter {
 NSBundle* tests=[NSBundle bundleForClass:self.class];
 NSString* fixture=[tests.resourcePath stringByAppendingPathComponent:@"UnitImagePreservationFixtures"];
 NSData* manifestData=[NSData dataWithContentsOfFile:[fixture stringByAppendingPathComponent:@"manifest.json"]];
 XCTAssertNotNil(manifestData,@"Test-only fixture staging is required");if(!manifestData)return;
 NSError* error=nil;NSDictionary* manifest=[NSJSONSerialization JSONObjectWithData:manifestData options:0 error:&error];
 XCTAssertNil(error);XCTAssertTrue([manifest isKindOfClass:NSDictionary.class]);if(error||![manifest isKindOfClass:NSDictionary.class])return;
 NSArray* rows=manifest[@"profiles"][@"apple"];XCTAssertEqual(rows.count,NSUInteger(63));if(rows.count!=63)return;
 HMMedia* media=[HMMedia new];media.resourceScope=@"unit-image-preservation";
 NSMutableArray* results=[NSMutableArray new];NSUInteger completed=0;
 for(NSDictionary* row in rows){@autoreleasepool{NSString* name=row[@"file"];try{
  XCTAssertEqualObjects(name,name.lastPathComponent);NSString* sourceName=[row[@"source"] lastPathComponent];
  NSData* source=[NSData dataWithContentsOfFile:[[fixture stringByAppendingPathComponent:@"sources"] stringByAppendingPathComponent:sourceName]];
  NSString* sourceHash=sha(source);XCTAssertEqualObjects(sourceHash,row[@"source_sha256"],@"Baseline source %@",name);
  if(!source||![sourceHash isEqual:row[@"source_sha256"]])continue;
  NSString* outputPath=[HMGameResourceBundle() pathForResource:name ofType:nil inDirectory:@"OriginalImages"];
  NSData* output=outputPath?[NSData dataWithContentsOfFile:outputPath]:nil;NSString* outputHash=sha(output);
  XCTAssertEqualObjects(outputHash,row[@"sha256"],@"Packaged candidate %@",name);if(!output||![outputHash isEqual:row[@"sha256"]])continue;
  int width=[row[@"width"] intValue],height=[row[@"height"] intValue];auto before=baselineHigh(source,row[@"source_crop"],width,height);
  HMImage* image=[media image:name];XCTAssertNotNil(image,@"Actual media load %@",name);if(!image)continue;
  XCTAssertEqual(CGImageGetWidth(image.CGImage),size_t(width));XCTAssertEqual(CGImageGetHeight(image.CGImage),size_t(height));
  NSData* expected=bytesData(before);NSData* actual=HMOriginalImageRGBA(image);XCTAssertNotNil(actual);if(!actual)continue;
  BOOL same=[expected isEqualToData:actual];XCTAssertTrue(same,@"Device CGHigh differs from packaged common loader: %@ (baseline %@, candidate %@)",name,sha(expected),sha(actual));
  auto baselineUpload=uploadContract(expected,width,height),candidateUpload=uploadContract(actual,width,height);BOOL sameUpload=baselineUpload==candidateUpload;
  XCTAssertTrue(sameUpload,@"Host compatibility/POT contract differs: %@",name);
  [results addObject:@{@"file":name,@"source_sha256":sourceHash,@"packaged_sha256":outputHash,@"width":@(width),@"height":@(height),@"device_baseline_rgba_sha256":sha(expected),@"candidate_decoded_rgba_sha256":sha(actual),@"recorded_macos_baseline_rgba_sha256":row[@"baseline_premultiplied_sha256"],@"device_baseline_upload_contract_sha256":sha(bytesData(baselineUpload)),@"candidate_upload_contract_sha256":sha(bytesData(candidateUpload)),@"rgba_equal":@(same),@"upload_contract_equal":@(sameUpload)}];
  if(!same){XCTAttachment* a=[XCTAttachment attachmentWithData:expected uniformTypeIdentifier:@"public.data"];a.name=[name stringByAppendingString:@".device-baseline.rgba"];a.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:a];XCTAttachment* b=[XCTAttachment attachmentWithData:actual uniformTypeIdentifier:@"public.data"];b.name=[name stringByAppendingString:@".candidate.rgba"];b.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:b];}
  ++completed;
 }catch(const std::exception& e){XCTFail(@"Image %@: %s",name,e.what());}}}
 XCTAssertEqual(completed,NSUInteger(63));
#if TARGET_OS_SIMULATOR
 NSString* target=@"ios-simulator";
#elif TARGET_OS_IPHONE
 NSString* target=@"ios-device";
#else
 NSString* target=@"native-macos";
#endif
 NSDictionary* report=@{@"schema":@1,@"os":NSProcessInfo.processInfo.operatingSystemVersionString,@"target":target,@"scope":@"CoreGraphics High executed in this test target vs actual candidate HMMedia decoded RGBA; compatibility/POT bytes are computed, GPU framebuffer verification is separate",@"manifest_sha256":sha(manifestData),@"completed":@(completed),@"images":results};
 NSData* encoded=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted error:nil];
 XCTAttachment* attachment=[XCTAttachment attachmentWithData:encoded uniformTypeIdentifier:@"public.json"];attachment.name=@"unit-image-preservation-device-63.json";attachment.lifetime=XCTAttachmentLifetimeKeepAlways;[self addAttachment:attachment];
}
@end
