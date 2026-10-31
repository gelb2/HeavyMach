#pragma once
// C++ translation of Sources/iOS/HMMedia for the Emscripten build.
// Only the GPU submission path is ported. The CPU SpriteRaster/TrailRaster
// branch of HMMedia is a 480x320 comparison oracle and is not a product path
// (docs/rendering-policy.md), so it has no web counterpart.
#include "WebTypes.hpp"
#include "Trail.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hmweb {
class GLRenderer;
class Audio;

struct DecodedImage {int width=0,height=0;std::vector<uint8_t> rgba;};

class Media {
public:
    // assetRoot holds OriginalImages/, Extensions/ and Audio/ (packaged data).
    Media(Audio& audio,std::string assetRoot);
    // Non-null only while submitting a frame (HMMedia.glRenderer).
    void setGlRenderer(GLRenderer* renderer);
    GLRenderer* glRenderer() const {return glRenderer_;}
    // Texture owner used for size queries outside a frame (HMMedia.textureRenderer).
    GLRenderer* textureRenderer=nullptr;
    std::string resourceScope="shared";
    Point presentationOffset;

    Size imageSize(const std::string& name);
    Size imageSize(const std::string& name,const std::string& scope);
    void releaseScope(const std::string& scope);
    // Decoded (premultiplied RGBA) pixels of one image for host UI, e.g. the HUD editor buttons.
    const DecodedImage* decodedImage(const std::string& name){return image(name);}
    Audio& audio(){return audio_;}

    void setCameraOffset(Point offset);
    void setDrawingOffset(Point offset);
    void setDrawingScale(double scale);
    // Absolute y-up canvas clip.
    void setClipRect(Rect rect);
    void resetClip();

    void drawEdgeContinuation(const std::string& name,double x,double y,double scale,double rotation,double width);
    void drawJoinedBackground(const std::string& name,double x,double y,double scale,double rotation);
    void drawExpandedPanel(const std::string& name,double x,double y,double left,double right,double top,double bottom);
    void drawRegion(const std::string& name,Rect source,Rect destination);
    void drawTrail(const heavymach::TrailMesh& mesh);

    void sound(const std::string& name,float pan=0);
    void draw(const std::string& name,double x,double y);
    void draw(const std::string& name,double x,double y,long columns,long rows,long tile,double sx,double sy,double r,double a,long blend=0);

    unsigned decodeCount() const {return decodeCount_;}
private:
    std::string resourceKey(const std::string& name) const;
    DecodedImage* image(const std::string& name);
    bool prepareTexture(const std::string& name,GLRenderer& renderer);

    Audio& audio_;
    std::string assetRoot_;
    GLRenderer* glRenderer_=nullptr;
    std::unordered_map<std::string,DecodedImage> images_;
    std::unordered_set<std::string> rgb565Images_;
    std::unordered_map<std::string,Size> sizes_;
    unsigned decodeCount_=0;
    Point cameraOffset_,drawingOffset_;
    double drawingScale_=1;
    bool hasClip_=false;
    Rect clipRect_;
};
}
