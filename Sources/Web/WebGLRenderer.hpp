#pragma once
// WebGL2 counterpart of Sources/iOS/HMGLRenderer.
//
// WebGL has no fixed-function pipeline, so the OpenGL ES 1.1 state used by the
// iOS renderer is reproduced explicitly: float modelview/projection matrices,
// GL_TEXTURE_ENV_MODE REPLACE/MODULATE for RGBA and RGB(565) textures, the
// original glBlendFunc table, scissor clipping, GL_DITHER, a W x H RGBA8 scene
// renderbuffer copied into a reusable LINEAR/CLAMP texture, and a final
// aspect-fit quad. Sprite textures stay POT, LINEAR/REPEAT, with zero padding.
#include "WebTypes.hpp"
#include "WebMedia.hpp"
#include "Trail.hpp"
#include <GLES3/gl3.h>
#include <map>
#include <string>
#include <vector>

namespace hmweb {
class GLRenderer {
public:
    // Requires a current WebGL2 context. Returns false when GL setup fails.
    ~GLRenderer();
    bool init();
    // Resize between frames. Invalid/unsupported sizes leave the previous canvas intact.
    bool setLogicalSize(double width,double height);
    // Final drawing transform: local * scale + offset. Clip uses absolute y-up canvas coordinates.
    void setDrawingOffset(Point offset);
    void setDrawingScale(double scale);
    void setClipRect(Rect rect);
    void resetClip();
    bool beginFrame();
    void endFrame();
    bool prepareImage(const DecodedImage& image,const std::string& name,bool rgb565);
    bool hasTexture(const std::string& name,bool rgb565) const;
    void releaseTextureScope(const std::string& scope);
    void drawImage(const std::string& name,long columns,long rows,long tile,Point position,Size scale,double rotation,double alpha,long blend,bool rgb565);
    void drawEdgeContinuation(const std::string& name,Point position,double scale,double rotation,double width,bool rgb565);
    void drawJoinedBackground(const std::string& name,Point position,double scale,double rotation,bool rgb565);
    void drawExpandedPanel(const std::string& name,Point position,double left,double right,double top,double bottom,bool rgb565);
    // HMGLRenderer drawRegion: source in texture pixels (top-left origin), destination in local y-up coordinates.
    void drawRegion(const std::string& name,Rect source,Rect destination,bool rgb565);
    void drawTrail(const heavymach::TrailMesh& mesh,const std::string& name,bool rgb565,Point cameraOffset);
    // viewport: CSS pixels, top-left origin, inside a canvas of cssWidth x cssHeight
    // whose drawing buffer is drawableWidth x drawableHeight device pixels.
    bool present(double cssWidth,double cssHeight,int drawableWidth,int drawableHeight,Rect viewport);
    void drawInputOutline(Rect rect);
    void drawAimLines(const float* points,int count,float red,float green,float blue);
    void finish();
    std::string diagnostics() const;
    const std::string& lastError() const {return lastError_;}
private:
    struct Texture {GLuint id=0;GLsizei width=0,height=0,pw=0,ph=0;bool rgb565=false;};
    struct Vertex {float x,y,u,v,r,g,b,a;};
    enum class Env {replace,modulate};
    void recordError(const std::string& message);
    void applyClip();
    Texture* texture(const std::string& name,bool rgb565);
    void setSpriteBlend(long blend);
    // modelview = T(offset) S(drawingScale) T(position) S(scale) R(rotation)
    void spriteMatrix(float out[16],Point position,double sx,double sy,double rotation,bool drawingTransform) const;
    void submit(const Vertex* vertices,size_t count,GLenum mode,const float modelview[16],const Texture* texture,Env env);
    void drawLineSegments(const float* points,int pointCount,bool loop,float red,float green,float blue);

    GLuint program_=0,vbo_=0,vao_=0;
    GLint uMatrix_=-1,uTexture_=-1,uMode_=-1;
    GLuint sceneFBO_=0,sceneBuffer_=0,sceneTexture_=0;
    bool sceneTextureAllocated_=false;
    double logicalWidth_=480,logicalHeight_=320,drawingScale_=1;
    GLsizei bufferWidth_=480,bufferHeight_=320;
    Point drawingOffset_;
    Rect clipRect_;
    bool hasClip_=false,active_=false;
    std::map<std::string,Texture> textures_;
    std::string lastError_,vendor_,renderer_,version_;
    unsigned drawCalls_=0,uploads_=0,frames_=0,sceneReallocations_=0;
};
}
