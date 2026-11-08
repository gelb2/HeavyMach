#include "Media.hpp"
#include "HMCompatibilityPixels.hpp"
#include "TextureFormat.hpp"
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#define STBI_NO_STDIO
#include "stb_image.h"
namespace hm {
static int pot(int n){int v=1;while(v<n)v*=2;return v;}
Media::Media(AAssetManager* a):assets_(a),audio(std::make_unique<Audio>(a)){}
Media::~Media(){detach();}
void Media::attach(ANativeWindow* w){
 detach();window_=w;ANativeWindow_acquire(w);
 display_=eglGetDisplay(EGL_DEFAULT_DISPLAY);
 if(display_==EGL_NO_DISPLAY||!eglInitialize(display_,nullptr,nullptr))throw std::runtime_error("EGL display initialization failed");
 EGLint attrs[]={EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_DEPTH_SIZE,0,EGL_NONE};
 EGLConfig config;EGLint count;if(!eglChooseConfig(display_,attrs,&config,1,&count)||!count)throw std::runtime_error("No GLES 1 RGBA8 window configuration");
 EGLint format;eglGetConfigAttrib(display_,config,EGL_NATIVE_VISUAL_ID,&format);ANativeWindow_setBuffersGeometry(w,0,0,format);
 EGLint contextAttrs[]={EGL_CONTEXT_CLIENT_VERSION,1,EGL_NONE};context_=eglCreateContext(display_,config,EGL_NO_CONTEXT,contextAttrs);
 surface_=eglCreateWindowSurface(display_,config,w,nullptr);
 if(context_==EGL_NO_CONTEXT||surface_==EGL_NO_SURFACE||!eglMakeCurrent(display_,surface_,surface_,context_))throw std::runtime_error("Cannot create GLES window/context");
 ++contextCreations;eglSwapInterval(display_,1);eglQuerySurface(display_,surface_,EGL_WIDTH,&drawableWidth);eglQuerySurface(display_,surface_,EGL_HEIGHT,&drawableHeight);
 const char* ext=reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
 if(!ext||!strstr(ext,"GL_OES_framebuffer_object")||!strstr(ext,"GL_OES_rgb8_rgba8"))throw std::runtime_error("GLES framebuffer/RGBA8 extension unavailable");
}
void Media::detach(){
 _active=false;
 if(display_!=EGL_NO_DISPLAY&&context_!=EGL_NO_CONTEXT){
  // This context has no shared objects. Destroying it releases its entire GPU
  // object set, including after context loss; no GL deletion calls are needed.
  eglMakeCurrent(display_,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);eglDestroyContext(display_,context_);
 }
 for(auto&[n,t]:textures_)t.id=0;
 if(display_!=EGL_NO_DISPLAY){if(surface_!=EGL_NO_SURFACE)eglDestroySurface(display_,surface_);eglTerminate(display_);}
 context_=EGL_NO_CONTEXT;surface_=EGL_NO_SURFACE;display_=EGL_NO_DISPLAY;sceneFBO_=sceneBuffer_=sceneTexture_=0;bufferWidth_=bufferHeight_=0;copied_=false;
 if(window_)ANativeWindow_release(window_);window_=nullptr;
}
void Media::recreateContext(){
 if(!window_)return;auto* w=window_;ANativeWindow_acquire(w);
 try{attach(w);}catch(...){ANativeWindow_release(w);throw;}ANativeWindow_release(w);
}
void Media::handleEGLFailure(const char* operation){
 auto error=eglGetError();
 if(error==EGL_CONTEXT_LOST){recreateContext();return;}
 if(error==EGL_BAD_SURFACE||error==EGL_BAD_NATIVE_WINDOW){detach();return;}
 throw std::runtime_error(std::string(operation)+" failed: "+std::to_string(error));
}
void Media::allocateScene(){
 int w=int(std::ceil(width)),h=int(std::ceil(height));if(w==bufferWidth_&&h==bufferHeight_)return;
 if(sceneFBO_)glDeleteFramebuffersOES(1,&sceneFBO_);if(sceneBuffer_)glDeleteRenderbuffersOES(1,&sceneBuffer_);if(sceneTexture_)glDeleteTextures(1,&sceneTexture_);
 bufferWidth_=w;bufferHeight_=h;copied_=false;
 glGenRenderbuffersOES(1,&sceneBuffer_);glBindRenderbufferOES(GL_RENDERBUFFER_OES,sceneBuffer_);glRenderbufferStorageOES(GL_RENDERBUFFER_OES,GL_RGBA8_OES,w,h);
 glGenFramebuffersOES(1,&sceneFBO_);glBindFramebufferOES(GL_FRAMEBUFFER_OES,sceneFBO_);glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES,GL_COLOR_ATTACHMENT0_OES,GL_RENDERBUFFER_OES,sceneBuffer_);
 if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES)!=GL_FRAMEBUFFER_COMPLETE_OES)throw std::runtime_error("Logical framebuffer allocation failed");
 glGenTextures(1,&sceneTexture_);glBindTexture(GL_TEXTURE_2D,sceneTexture_);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
}
bool Media::begin(double w,double h){
 if(context_==EGL_NO_CONTEXT)return false;
 if(!eglMakeCurrent(display_,surface_,surface_,context_)){handleEGLFailure("eglMakeCurrent");return false;}
 width=w;height=h;allocateScene();_active=true;
 glBindFramebufferOES(GL_FRAMEBUFFER_OES,sceneFBO_);glViewport(0,0,bufferWidth_,bufferHeight_);glDisable(GL_SCISSOR_TEST);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);
 glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glEnable(GL_TEXTURE_2D);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 glEnableClientState(GL_VERTEX_ARRAY);glEnableClientState(GL_TEXTURE_COORD_ARRAY);glEnableClientState(GL_COLOR_ARRAY);glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
 glMatrixMode(GL_PROJECTION);glLoadIdentity();glOrthof(0,float(w),0,float(h),-1,1);glMatrixMode(GL_TEXTURE);glLoadIdentity();glMatrixMode(GL_MODELVIEW);glLoadIdentity();return true;
}
bool Media::present(){
 if(!_active)return false;
 glBindTexture(GL_TEXTURE_2D,sceneTexture_);if(copied_)glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,bufferWidth_,bufferHeight_);else{glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,0,0,bufferWidth_,bufferHeight_,0);copied_=true;}
 glBindFramebufferOES(GL_FRAMEBUFFER_OES,0);glDisable(GL_SCISSOR_TEST);glViewport(0,0,drawableWidth,drawableHeight);glMatrixMode(GL_PROJECTION);glLoadIdentity();glMatrixMode(GL_MODELVIEW);glLoadIdentity();glDisable(GL_BLEND);glDisable(GL_DITHER);glDisableClientState(GL_COLOR_ARRAY);
 GLfloat p[]={-1,-1,-1,1,1,-1,1,-1,-1,1,1,1},uv[]={0,0,0,1,1,0,1,0,0,1,1,1};glVertexPointer(2,GL_FLOAT,0,p);glTexCoordPointer(2,GL_FLOAT,0,uv);glDrawArrays(GL_TRIANGLES,0,6);
 GLenum error=glGetError();if(error!=GL_NO_ERROR)throw std::runtime_error("GLES frame error "+std::to_string(error));bool ok=eglSwapBuffers(display_,surface_);_active=false;if(!ok){handleEGLFailure("eglSwapBuffers");return false;}glEnable(GL_DITHER);++frames;return true;
}
Media::Texture* Media::texture(const std::string& name){
 std::string key=resourceScope+"|"+name;auto found=textures_.find(key);
 if(found==textures_.end()){
  // Baked unit PNGs use the same path and decoder as existing images.
  const std::string& file=name;
  auto bytes=assetBytes(assets_,name.starts_with("Extensions/")?file:"OriginalImages/"+file);
  stbi_convert_iphone_png_to_rgb_thread(1);stbi_set_unpremultiply_on_load_thread(1);
  int w,h,ch;auto*p=stbi_load_from_memory(bytes.data(),int(bytes.size()),&w,&h,&ch,4);if(!p)throw std::runtime_error("Image decode failed: "+name);
  std::vector<uint8_t> rgba(p,p+size_t(w)*h*4);stbi_image_free(p);
  const unsigned char png[]={137,80,78,71,13,10,26,10};if(bytes.size()>=8&&!memcmp(bytes.data(),png,8))for(size_t i=0;i<rgba.size();i+=4){float a=float(rgba[i+3])/255.f;for(int c=0;c<3;++c)rgba[i+c]=uint8_t(float(rgba[i+c])*a);}
  int targetW=0,targetH=0;
  if(name=="Extensions/AutoAimControls.png")targetW=138;
  // Area prefilter averages coverage before the final linear texture sampling.
  if(targetW>0&&w>targetW){targetH=targetH?targetH:std::max(1,int(std::lround(double(h)*targetW/w)));std::vector<uint8_t> small(size_t(targetW)*targetH*4);
   for(int y=0;y<targetH;++y)for(int x=0;x<targetW;++x){double x0=double(x)*w/targetW,x1=double(x+1)*w/targetW,y0=double(y)*h/targetH,y1=double(y+1)*h/targetH;double sum[4]={};
    for(int sy=int(y0);sy<std::ceil(y1);++sy)for(int sx=int(x0);sx<std::ceil(x1);++sx){double weight=(std::min(x1,double(sx+1))-std::max(x0,double(sx)))*(std::min(y1,double(sy+1))-std::max(y0,double(sy)));for(int c=0;c<4;++c)sum[c]+=rgba[(size_t(std::min(h-1,sy))*w+std::min(w-1,sx))*4+c]*weight;}
    for(int c=0;c<4;++c)small[(size_t(y)*targetW+x)*4+c]=uint8_t(std::clamp(std::lround(sum[c]/((x1-x0)*(y1-y0))),0L,255L));}
   rgba=std::move(small);w=targetW;h=targetH;
  }
  heavymach::compatibilityBitmapPixels(rgba.data(),size_t(w)*h);
  Texture t;t.width=w;t.height=h;t.pw=pot(w);t.ph=pot(h);t.rgb=ch==3||ch==1;t.pixels.resize(size_t(t.pw)*t.ph*(t.rgb?2:4));
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){const auto* q=rgba.data()+(size_t(y)*w+x)*4;if(t.rgb){uint16_t v=heavymach::packOriginalRGB565(q[0],q[1],q[2]);std::memcpy(t.pixels.data()+(size_t(y)*t.pw+x)*2,&v,2);}else std::copy_n(q,4,t.pixels.data()+(size_t(y)*t.pw+x)*4);}
  found=textures_.emplace(key,std::move(t)).first;
 }
 auto& t=found->second;
 if(context_!=EGL_NO_CONTEXT&&!t.id){glGenTextures(1,&t.id);glBindTexture(GL_TEXTURE_2D,t.id);glPixelStorei(GL_UNPACK_ALIGNMENT,1);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);glTexImage2D(GL_TEXTURE_2D,0,t.rgb?GL_RGB:GL_RGBA,t.pw,t.ph,0,t.rgb?GL_RGB:GL_RGBA,t.rgb?GL_UNSIGNED_SHORT_5_6_5:GL_UNSIGNED_BYTE,t.pixels.data());++uploads;}
 return &t;
}
CGSize Media::imageSize(const std::string& name){auto*t=texture(name);return {double(t->width),double(t->height)};}
CGSize Media::imageSize(const std::string& name,const std::string& scope){auto prior=resourceScope;resourceScope=scope;try{auto size=imageSize(name);resourceScope=prior;return size;}catch(...){resourceScope=prior;throw;}}
void Media::releaseScope(const std::string& scope){for(auto i=textures_.begin();i!=textures_.end();)if(i->first.starts_with(scope+"|")){if(i->second.id&&context_!=EGL_NO_CONTEXT)glDeleteTextures(1,&i->second.id);i=textures_.erase(i);}else ++i;}
void Media::setClipRect(CGRect r){if(!_active)return;double sx=bufferWidth_/width,sy=bufferHeight_/height;int x=int(std::ceil((r.origin.x+presentationOffset.x)*sx-.5)),y=int(std::ceil((r.origin.y+presentationOffset.y)*sy-.5));int right=int(std::ceil((CGRectGetMaxX(r)+presentationOffset.x)*sx-.5)),top=int(std::ceil((CGRectGetMaxY(r)+presentationOffset.y)*sy-.5));glEnable(GL_SCISSOR_TEST);glScissor(x,y,std::max(0,right-x),std::max(0,top-y));}
void Media::resetClip(){if(_active)glDisable(GL_SCISSOR_TEST);}
void Media::transform(CGPoint p,CGSize s,double r){glLoadIdentity();glTranslatef(_drawingOffset.x,_drawingOffset.y,0);glScalef(_drawingScale,_drawingScale,1);glTranslatef(p.x,p.y,0);glScalef(s.width,s.height,1);glRotatef(r*57.29577951308232,0,0,1);}
void Media::draw(const std::string& name,double x,double y,int columns,int rows,int tile,double sx,double sy,double r,double alpha,int blend){
 if(!_active||columns<1||rows<1||tile<0)return;auto*t=texture(name);tile=std::min(tile,columns*rows-1);
 GLfloat u=float(t->width)/t->pw/columns,v=float(t->height)/t->ph/rows,hw=t->pw*u*.5f,hh=t->ph*v*.5f,u0=tile%columns*u,v0=tile/columns*v;
 GLfloat p[]={-hw,-hh,hw,-hh,-hw,hh,hw,hh},uv[]={u0,v0+v,u0+u,v0+v,u0,v0,u0+u,v0},c[]={1,1,1,float(alpha),1,1,1,float(alpha),1,1,1,float(alpha),1,1,1,float(alpha)};
 glBindTexture(GL_TEXTURE_2D,t->id);if(blend==0||blend==3)glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);else if(blend==2)glBlendFunc(GL_SRC_ALPHA,GL_ONE);else if(blend==1)glBlendFunc(GL_SRC_COLOR,GL_ONE_MINUS_SRC_ALPHA);
 transform({x-camera_.x,y-camera_.y},{sx,sy},r);glVertexPointer(2,GL_FLOAT,0,p);glTexCoordPointer(2,GL_FLOAT,0,uv);glColorPointer(4,GL_FLOAT,0,c);glDrawArrays(GL_TRIANGLE_STRIP,0,4);++_drawCalls;
}

void Media::drawRegion(const std::string& name,CGRect source,CGRect d) {

    if(!_active||CGRectIsEmpty(source)||CGRectIsEmpty(d))return;
    Texture* t=texture(name);if(!t)return;
    GLfloat x=d.origin.x,y=d.origin.y,w=d.size.width,h=d.size.height;
    GLfloat u=source.origin.x/t->pw,v=source.origin.y/t->ph,uw=source.size.width/t->pw,vh=source.size.height/t->ph;
    GLfloat p[]={x,y,x+w,y,x,y+h,x+w,y+h},uv[]={u,v+vh,u+uw,v+vh,u,v,u+uw,v},c[]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    glBindTexture(GL_TEXTURE_2D,t->id);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();glTranslatef(_drawingOffset.x,_drawingOffset.y,0);glScalef(_drawingScale,_drawingScale,1);
    glVertexPointer(2,GL_FLOAT,0,p);glTexCoordPointer(2,GL_FLOAT,0,uv);glColorPointer(4,GL_FLOAT,0,c);glDrawArrays(GL_TRIANGLE_STRIP,0,4);_drawCalls++;
}

void Media::drawJoinedBackground(const std::string& name,double x,double y,double scale,double rotation) {
CGPoint position{x-camera_.x,y-camera_.y};
    if(!_active||!std::isfinite(position.x)||!std::isfinite(position.y)||!std::isfinite(scale)||scale<=0||!std::isfinite(rotation))return;
    Texture* t=texture(name);if(!t)return;
    GLfloat hw=t->width*.5f,hh=t->height*.5f;
    GLfloat xs[]={-hw,-hw+.5f,hw-.5f,hw},ys[]={-hh,-hh+.5f,hh-.5f,hh};
    GLfloat us[]={.5f/t->pw,.5f/t->pw,(t->width-.5f)/t->pw,(t->width-.5f)/t->pw};
    GLfloat vs[]={(t->height-.5f)/t->ph,(t->height-.5f)/t->ph,.5f/t->ph,.5f/t->ph};
    GLfloat positions[108],uv[108],colors[216];int n=0;
    const int corners[][2]={{0,0},{1,0},{0,1},{0,1},{1,0},{1,1}};
    for(int y=0;y<3;++y)for(int x=0;x<3;++x)for(auto& corner:corners){
        int ix=x+corner[0],iy=y+corner[1];positions[n*2]=xs[ix];positions[n*2+1]=ys[iy];uv[n*2]=us[ix];uv[n*2+1]=vs[iy];
        std::fill_n(colors+n*4,4,1.f);++n;
    }
    glBindTexture(GL_TEXTURE_2D,t->id);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();glTranslatef(_drawingOffset.x,_drawingOffset.y,0);glScalef(_drawingScale,_drawingScale,1);glTranslatef(position.x,position.y,0);glScalef(scale,scale,1);glRotatef(rotation*57.29577951308232,0,0,1);
    glVertexPointer(2,GL_FLOAT,0,positions);glTexCoordPointer(2,GL_FLOAT,0,uv);glColorPointer(4,GL_FLOAT,0,colors);glDrawArrays(GL_TRIANGLES,0,n);_drawCalls++;
}

void Media::drawExpandedPanel(const std::string& name,double x,double y,double left,double right,double top,double bottom) {
CGPoint position{x,y};
    if(!_active)return;
    Texture* t=texture(name);if(!t)return;
    // Only the outer 16px metal strips stretch. Interior coordinates/UVs remain 1:1.
    const GLfloat hw=t->width*.5f,hh=t->height*.5f,edge=16;
    const GLfloat xs[]={-hw-float(left),-hw+edge,hw-edge,hw+float(right)};
    const GLfloat ys[]={-hh-float(bottom),-hh+edge,hh-edge,hh+float(top)};
    const GLfloat us[]={1.5f/t->pw,edge/t->pw,(t->width-edge)/t->pw,(t->width-1.5f)/t->pw};
    const GLfloat vs[]={(t->height-1.5f)/t->ph,(t->height-edge)/t->ph,edge/t->ph,1.5f/t->ph};
    GLfloat positions[108],uv[108],colors[216];int n=0;
    const int corners[][2]={{0,0},{1,0},{0,1},{0,1},{1,0},{1,1}};
    for(int y=0;y<3;++y)for(int x=0;x<3;++x)for(auto& c:corners){int ix=x+c[0],iy=y+c[1];positions[n*2]=xs[ix];positions[n*2+1]=ys[iy];uv[n*2]=us[ix];uv[n*2+1]=vs[iy];std::fill_n(colors+n*4,4,1.f);++n;}
    glBindTexture(GL_TEXTURE_2D,t->id);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();glTranslatef(position.x,position.y,0);
    glVertexPointer(2,GL_FLOAT,0,positions);glTexCoordPointer(2,GL_FLOAT,0,uv);glColorPointer(4,GL_FLOAT,0,colors);glDrawArrays(GL_TRIANGLES,0,n);_drawCalls++;
}

void Media::drawEdgeContinuation(const std::string& name,double x,double y,double scale,double rotation,double width) {
CGPoint position{x-camera_.x,y-camera_.y};
    if(!_active||width<=0||!std::isfinite(width)||!std::isfinite(scale)||!std::isfinite(rotation))return;
    Texture* t=texture(name);if(!t)return;
    width=std::min(width,CGFloat(t->width/4));GLfloat hw=t->width*.5f,hh=t->height*.5f;
    glBindTexture(GL_TEXTURE_2D,t->id);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glLoadIdentity();glTranslatef(_drawingOffset.x,_drawingOffset.y,0);glScalef(_drawingScale,_drawingScale,1);glTranslatef(position.x,position.y,0);glScalef(scale,scale,1);glRotatef(rotation*57.29577951308232,0,0,1);
    for(int side:{-1,1}){
        GLfloat left=side<0?-hw-width:hw,right=side<0?-hw:hw+width;
        GLfloat u0=side<0?(width+.5f)/t->pw:(t->width-.5f)/t->pw;
        GLfloat u1=side<0?.5f/t->pw:(t->width-width-.5f)/t->pw;
        GLfloat v=float(t->height)/t->ph,near=side<0?0:1,far=1-near;
        GLfloat positions[]={left,-hh,right,-hh,left,hh,right,hh},uv[]={u0,v,u1,v,u0,0,u1,0};
        GLfloat colors[]={1,1,1,near,1,1,1,far,1,1,1,near,1,1,1,far};
        glVertexPointer(2,GL_FLOAT,0,positions);glTexCoordPointer(2,GL_FLOAT,0,uv);glColorPointer(4,GL_FLOAT,0,colors);glDrawArrays(GL_TRIANGLE_STRIP,0,4);_drawCalls++;
    }
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
}

void Media::drawTrail(const heavymach::TrailMesh& mesh) {
CGPoint offset=camera_;const auto& name=mesh.tile;
    if(!_active||mesh.vertices.size()<3)return;
    Texture *t=texture(name);if(!t)return;
    glBindTexture(GL_TEXTURE_2D,t->id);if(mesh.setBlend)glBlendFunc(mesh.sourceFactor,mesh.destinationFactor);
    glLoadIdentity();glTranslatef(_drawingOffset.x,_drawingOffset.y,0);glScalef(_drawingScale,_drawingScale,1);glTranslatef(mesh.translation.x-offset.x,mesh.translation.y-offset.y,0);
    const auto *vertices=mesh.vertices.data();glVertexPointer(2,GL_FLOAT,sizeof(heavymach::TrailVertex),&vertices[0].position.x);
    glTexCoordPointer(2,GL_FLOAT,sizeof(heavymach::TrailVertex),&vertices[0].uv.x);glColorPointer(4,GL_FLOAT,sizeof(heavymach::TrailVertex),vertices[0].color.data());
    glDrawArrays(GL_TRIANGLE_STRIP,0,GLsizei(mesh.vertices.size()));_drawCalls++;
}

void Media::lines(const float*p,int count,float r,float g,float b,float a){
 if(!_active||count<2)return;glLoadIdentity();glDisable(GL_TEXTURE_2D);glDisableClientState(GL_TEXTURE_COORD_ARRAY);glDisableClientState(GL_COLOR_ARRAY);glColor4f(r,g,b,a);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glLineWidth(1);glVertexPointer(2,GL_FLOAT,0,p);glDrawArrays(GL_LINES,0,count);glEnable(GL_TEXTURE_2D);glEnableClientState(GL_TEXTURE_COORD_ARRAY);glEnableClientState(GL_COLOR_ARRAY);++_drawCalls;
}
std::vector<uint8_t> Media::snapshot(){if(context_==EGL_NO_CONTEXT)return {};glBindFramebufferOES(GL_FRAMEBUFFER_OES,sceneFBO_);std::vector<uint8_t> bottom(size_t(bufferWidth_)*bufferHeight_*4),top(bottom.size());glReadPixels(0,0,bufferWidth_,bufferHeight_,GL_RGBA,GL_UNSIGNED_BYTE,bottom.data());for(int y=0;y<bufferHeight_;++y)std::copy_n(bottom.data()+size_t(bufferHeight_-1-y)*bufferWidth_*4,size_t(bufferWidth_)*4,top.data()+size_t(y)*bufferWidth_*4);return top;}
}
