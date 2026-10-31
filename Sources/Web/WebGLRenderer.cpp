#include "WebGLRenderer.hpp"
#include "HMCompatibilityPixels.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <emscripten/emscripten.h>

namespace hmweb {
namespace {
GLsizei pot(GLsizei n){GLsizei p=1;while(p<n)p*=2;return p;}
std::string glString(GLenum name){const auto p=glGetString(name);return p?reinterpret_cast<const char*>(p):"unavailable";}

// Column-major 4x4 helpers following the OpenGL ES 1.1 matrix-stack semantics:
// each glTranslatef/glScalef/glRotatef right-multiplies the current matrix in binary32.
void identity(float m[16]){std::fill_n(m,16,0.f);m[0]=m[5]=m[10]=m[15]=1;}
void multiply(float out[16],const float a[16],const float b[16]){
    float r[16];
    for(int c=0;c<4;++c)for(int row=0;row<4;++row){
        float sum=0;for(int k=0;k<4;++k)sum+=a[k*4+row]*b[c*4+k];r[c*4+row]=sum;
    }
    std::copy_n(r,16,out);
}
void translate(float m[16],float x,float y){float t[16];identity(t);t[12]=x;t[13]=y;multiply(m,m,t);}
void scale(float m[16],float x,float y){float t[16];identity(t);t[0]=x;t[5]=y;multiply(m,m,t);}
// glRotatef(angle,0,0,1): degrees converted back to radians by the GL implementation.
void rotate(float m[16],float degrees){
    const float radians=degrees*float(M_PI/180.0),c=std::cos(radians),s=std::sin(radians);
    float t[16];identity(t);t[0]=c;t[1]=s;t[4]=-s;t[5]=c;multiply(m,m,t);
}

const char* vertexShader=R"(#version 300 es
layout(location=0) in vec2 aPosition;
layout(location=1) in vec2 aUV;
layout(location=2) in vec4 aColor;
uniform mat4 uMatrix;
out vec2 vUV;
out vec4 vColor;
void main(){vUV=aUV;vColor=aColor;gl_Position=uMatrix*vec4(aPosition,0.0,1.0);}
)";
// GL_TEXTURE_ENV_MODE for a single texture unit (OpenGL ES 1.1 spec table 3.15):
// 0 REPLACE/RGBA: C=Ct A=At     1 REPLACE/RGB: C=Ct A=Af
// 2 MODULATE/RGBA: C=CfCt A=AfAt  3 MODULATE/RGB: C=CfCt A=Af   4 texturing disabled
const char* fragmentShader=R"(#version 300 es
precision highp float;
in vec2 vUV;
in vec4 vColor;
uniform sampler2D uTexture;
uniform int uMode;
out vec4 fragColor;
void main(){
    if(uMode==4){fragColor=vColor;return;}
    vec4 t=texture(uTexture,vUV);
    if(uMode==0)fragColor=t;
    else if(uMode==1)fragColor=vec4(t.rgb,vColor.a);
    else if(uMode==2)fragColor=t*vColor;
    else fragColor=vec4(t.rgb*vColor.rgb,vColor.a);
}
)";
GLuint compile(GLenum type,const char* source,std::string& error){
    GLuint shader=glCreateShader(type);glShaderSource(shader,1,&source,nullptr);glCompileShader(shader);
    GLint ok=0;glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);
    if(!ok){char log[1024]{};glGetShaderInfoLog(shader,sizeof(log),nullptr,log);error=log;glDeleteShader(shader);return 0;}
    return shader;
}
}

void GLRenderer::recordError(const std::string& message){lastError_=message;std::fprintf(stderr,"HMGLRenderer: %s\n",message.c_str());}

GLRenderer::~GLRenderer(){
    // During context loss these calls also release Emscripten's JS handle table.
    // The browser has already discarded the GPU storage in that case.
    for(const auto& [key,tex]:textures_)if(tex.id)glDeleteTextures(1,&tex.id);
    if(sceneTexture_)glDeleteTextures(1,&sceneTexture_);
    if(sceneFBO_)glDeleteFramebuffers(1,&sceneFBO_);
    if(sceneBuffer_)glDeleteRenderbuffers(1,&sceneBuffer_);
    if(vbo_)glDeleteBuffers(1,&vbo_);
    if(vao_)glDeleteVertexArrays(1,&vao_);
    if(program_)glDeleteProgram(program_);
}

bool GLRenderer::init(){
    vendor_=glString(GL_VENDOR);renderer_=glString(GL_RENDERER);version_=glString(GL_VERSION);
    std::string error;
    GLuint vs=compile(GL_VERTEX_SHADER,vertexShader,error),fs=vs?compile(GL_FRAGMENT_SHADER,fragmentShader,error):0;
    if(!vs||!fs){recordError("Shader compile failed: "+error);return false;}
    program_=glCreateProgram();glAttachShader(program_,vs);glAttachShader(program_,fs);glLinkProgram(program_);
    glDeleteShader(vs);glDeleteShader(fs);
    GLint linked=0;glGetProgramiv(program_,GL_LINK_STATUS,&linked);
    if(!linked){recordError("Shader link failed");return false;}
    uMatrix_=glGetUniformLocation(program_,"uMatrix");uTexture_=glGetUniformLocation(program_,"uTexture");uMode_=glGetUniformLocation(program_,"uMode");
    glGenVertexArrays(1,&vao_);glBindVertexArray(vao_);
    glGenBuffers(1,&vbo_);glBindBuffer(GL_ARRAY_BUFFER,vbo_);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(8));
    glEnableVertexAttribArray(2);glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(16));
    // Match touchHLE EAGL: renderbuffer -> reusable copy texture -> drawable.
    glGenTextures(1,&sceneTexture_);glBindTexture(GL_TEXTURE_2D,sceneTexture_);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glGenRenderbuffers(1,&sceneBuffer_);glBindRenderbuffer(GL_RENDERBUFFER,sceneBuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_RGBA8,bufferWidth_,bufferHeight_);
    sceneReallocations_=1;
    glGenFramebuffers(1,&sceneFBO_);glBindFramebuffer(GL_FRAMEBUFFER,sceneFBO_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_RENDERBUFFER,sceneBuffer_);
    const bool ok=glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE&&glGetError()==GL_NO_ERROR;
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    if(!ok){recordError("Logical framebuffer creation failed");return false;}
    return true;
}

bool GLRenderer::setLogicalSize(double width,double height){
    if(active_||!std::isfinite(width)||!std::isfinite(height)||width<=0||height<=0)return false;
    if(width==logicalWidth_&&height==logicalHeight_)return true;
    GLint maxTexture=0,maxBuffer=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTexture);glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,&maxBuffer);
    if(std::ceil(width)>std::min(maxTexture,maxBuffer)||std::ceil(height)>std::min(maxTexture,maxBuffer))return false;
    const GLsizei w=GLsizei(std::ceil(width)),h=GLsizei(std::ceil(height));
    if(w!=bufferWidth_||h!=bufferHeight_){
        // Allocate a replacement first so a failed allocation preserves the old scene.
        GLuint buffer=0,fbo=0;glGenRenderbuffers(1,&buffer);glBindRenderbuffer(GL_RENDERBUFFER,buffer);
        glRenderbufferStorage(GL_RENDERBUFFER,GL_RGBA8,w,h);
        glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_RENDERBUFFER,buffer);
        const bool ok=glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE&&glGetError()==GL_NO_ERROR;
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        if(!ok){glDeleteFramebuffers(1,&fbo);glDeleteRenderbuffers(1,&buffer);return false;}
        glDeleteFramebuffers(1,&sceneFBO_);glDeleteRenderbuffers(1,&sceneBuffer_);
        sceneFBO_=fbo;sceneBuffer_=buffer;bufferWidth_=w;bufferHeight_=h;sceneTextureAllocated_=false;sceneReallocations_++;
    }
    logicalWidth_=width;logicalHeight_=height;return true;
}
void GLRenderer::setDrawingOffset(Point offset){if(std::isfinite(offset.x)&&std::isfinite(offset.y))drawingOffset_=offset;}
void GLRenderer::setDrawingScale(double s){if(std::isfinite(s)&&s>0)drawingScale_=s;}
void GLRenderer::applyClip(){
    if(!hasClip_){glDisable(GL_SCISSOR_TEST);return;}
    Rect r=intersection(clipRect_,makeRect(0,0,logicalWidth_,logicalHeight_));
    glEnable(GL_SCISSOR_TEST);
    if(isNull(r)||isEmpty(r)){glScissor(0,0,0,0);return;}
    const double sx=bufferWidth_/logicalWidth_,sy=bufferHeight_/logicalHeight_;
    // Choose pixel centres within the requested logical rectangle.
    const GLint x=GLint(std::ceil(minX(r)*sx-.5)),y=GLint(std::ceil(minY(r)*sy-.5));
    const GLint right=GLint(std::ceil(maxX(r)*sx-.5)),top=GLint(std::ceil(maxY(r)*sy-.5));
    glScissor(x,y,std::max(0,right-x),std::max(0,top-y));
}
void GLRenderer::setClipRect(Rect rect){
    if(!std::isfinite(rect.origin.x)||!std::isfinite(rect.origin.y)||!std::isfinite(rect.size.width)||!std::isfinite(rect.size.height)||rect.size.width<0||rect.size.height<0)rect=Rect{};
    clipRect_=rect;hasClip_=true;if(active_)applyClip();
}
void GLRenderer::resetClip(){hasClip_=false;if(active_)applyClip();}

bool GLRenderer::beginFrame(){
    if(active_){recordError("Nested beginFrame");return false;}
    active_=true;drawCalls_=0;
    glBindFramebuffer(GL_FRAMEBUFFER,sceneFBO_);glViewport(0,0,bufferWidth_,bufferHeight_);
    glDisable(GL_SCISSOR_TEST);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glEnable(GL_DITHER);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);applyClip();
    glUseProgram(program_);glBindVertexArray(vao_);glBindBuffer(GL_ARRAY_BUFFER,vbo_);
    glActiveTexture(GL_TEXTURE0);glUniform1i(uTexture_,0);
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);glPixelStorei(GL_PACK_ALIGNMENT,1);return true;
}
void GLRenderer::endFrame(){
    if(!active_)return;
    GLenum error=glGetError();if(error!=GL_NO_ERROR){char text[64];std::snprintf(text,sizeof(text),"OpenGL error 0x%x",error);recordError(text);}
    glFlush();active_=false;frames_++;
}

GLRenderer::Texture* GLRenderer::texture(const std::string& name,bool rgb565){
    auto found=textures_.find(name+(rgb565?"#565":"#rgba"));
    return found==textures_.end()?nullptr:&found->second;
}
bool GLRenderer::hasTexture(const std::string& name,bool rgb565) const{return textures_.contains(name+(rgb565?"#565":"#rgba"));}
bool GLRenderer::prepareImage(const DecodedImage& image,const std::string& name,bool rgb565){
    const std::string key=name+(rgb565?"#565":"#rgba");
    if(textures_.contains(key))return true;
    const size_t w=size_t(image.width),h=size_t(image.height);GLint maxSize=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize);
    if(!w||!h||w>size_t(maxSize)||h>size_t(maxSize)||image.rgba.size()!=w*h*4){recordError("Invalid texture dimensions");return false;}
    Texture t;t.width=GLsizei(w);t.height=GLsizei(h);t.pw=pot(t.width);t.ph=pot(t.height);t.rgb565=rgb565;
    std::vector<uint8_t> pixels(size_t(t.pw)*t.ph*4);
    // touchHLE CGImage pixels are already premultiplied, top row first.
    // Preserve the bytes and zero POT padding without another color conversion.
    for(size_t y=0;y<h;++y){
        auto* row=pixels.data()+y*size_t(t.pw)*4;
        std::copy_n(image.rgba.data()+y*w*4,w*4,row);
        heavymach::compatibilityBitmapPixels(row,w);
    }
    GLint alignment=4;glGetIntegerv(GL_UNPACK_ALIGNMENT,&alignment);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glGenTextures(1,&t.id);glBindTexture(GL_TEXTURE_2D,t.id);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    if(rgb565){
        std::vector<uint16_t> packed(size_t(t.pw)*t.ph);
        for(size_t i=0;i<packed.size();++i)packed[i]=uint16_t((pixels[i*4]>>3)<<11|(pixels[i*4+1]>>2)<<5|(pixels[i*4+2]>>3));
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB565,t.pw,t.ph,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,packed.data());
    }else glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,t.pw,t.ph,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT,alignment);
    GLenum error=glGetError();
    if(error!=GL_NO_ERROR){glDeleteTextures(1,&t.id);char text[256];std::snprintf(text,sizeof(text),"Texture upload %s failed 0x%x",name.c_str(),error);recordError(text);return false;}
    uploads_++;textures_.emplace(key,t);return true;
}
void GLRenderer::releaseTextureScope(const std::string& scope){
    const std::string prefix=scope+"|";
    for(auto i=textures_.begin();i!=textures_.end();){
        if(i->first.starts_with(prefix)){glDeleteTextures(1,&i->second.id);i=textures_.erase(i);}else ++i;
    }
}

void GLRenderer::setSpriteBlend(long blend){
    // Original sprite blend 1 is SRC_COLOR/ONE_MINUS_SRC_ALPHA; unknown values inherit state.
    if(blend==0||blend==3)glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    else if(blend==2)glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    else if(blend==1)glBlendFunc(GL_SRC_COLOR,GL_ONE_MINUS_SRC_ALPHA);
}
void GLRenderer::spriteMatrix(float m[16],Point position,double sx,double sy,double rotation,bool drawingTransform) const{
    identity(m);
    if(drawingTransform){translate(m,float(drawingOffset_.x),float(drawingOffset_.y));scale(m,float(drawingScale_),float(drawingScale_));}
    translate(m,float(position.x),float(position.y));scale(m,float(sx),float(sy));rotate(m,float(rotation*57.29577951308232));
}
void GLRenderer::submit(const Vertex* vertices,size_t count,GLenum mode,const float modelview[16],const Texture* t,Env env){
    float projection[16];identity(projection);
    // glOrthof(0,W,0,H,-1,1)
    projection[0]=float(2/logicalWidth_);projection[5]=float(2/logicalHeight_);projection[10]=-1;projection[12]=-1;projection[13]=-1;
    float matrix[16];multiply(matrix,projection,modelview);
    glUniformMatrix4fv(uMatrix_,1,GL_FALSE,matrix);
    int m=4;
    if(t){glBindTexture(GL_TEXTURE_2D,t->id);m=(env==Env::replace?0:2)+(t->rgb565?1:0);}
    glUniform1i(uMode_,m);
    glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(count*sizeof(Vertex)),vertices,GL_STREAM_DRAW);
    glDrawArrays(mode,0,GLsizei(count));drawCalls_++;
}

void GLRenderer::drawLineSegments(const float* points,int count,bool loop,float red,float green,float blue){
    // glLineWidth(2): WebGL implementations clamp line width to 1, so each
    // segment is expanded into a 2-pixel-wide quad in scene buffer pixels.
    std::vector<Vertex> v;
    const double sx=bufferWidth_/logicalWidth_,sy=bufferHeight_/logicalHeight_;
    auto segment=[&](float x0,float y0,float x1,float y1){
        double dx=(x1-x0)*sx,dy=(y1-y0)*sy,len=std::hypot(dx,dy);
        if(len<=0)return;
        float nx=float(-dy/len/sx),ny=float(dx/len/sy); // 1 pixel each side
        Vertex a{x0+nx,y0+ny,0,0,red,green,blue,1},b{x0-nx,y0-ny,0,0,red,green,blue,1};
        Vertex c{x1+nx,y1+ny,0,0,red,green,blue,1},d{x1-nx,y1-ny,0,0,red,green,blue,1};
        v.insert(v.end(),{a,b,c,c,b,d});
    };
    if(loop)for(int i=0;i<count;++i){int j=(i+1)%count;segment(points[i*2],points[i*2+1],points[j*2],points[j*2+1]);}
    else for(int i=0;i+1<count;i+=2)segment(points[i*2],points[i*2+1],points[i*2+2],points[i*2+3]);
    if(v.empty())return;
    float m[16];identity(m);
    submit(v.data(),v.size(),GL_TRIANGLES,m,nullptr,Env::replace);
}
void GLRenderer::drawAimLines(const float* points,int count,float red,float green,float blue){
    if(!active_||count<2)return;
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    drawLineSegments(points,count,false,red,green,blue);
}
void GLRenderer::drawInputOutline(Rect rect){
    if(!active_||isEmpty(rect))return;
    float points[]={float(minX(rect)),float(minY(rect)),float(maxX(rect)),float(minY(rect)),float(maxX(rect)),float(maxY(rect)),float(minX(rect)),float(maxY(rect))};
    drawLineSegments(points,4,true,1,1,0);
}

void GLRenderer::drawImage(const std::string& name,long columns,long rows,long tile,Point position,Size s,double rotation,double alpha,long blend,bool rgb565){
    if(!active_||columns<1||rows<1||tile<0||!std::isfinite(position.x)||!std::isfinite(position.y)||!std::isfinite(s.width)||!std::isfinite(s.height)||!std::isfinite(rotation)||!std::isfinite(alpha))return;
    Texture* t=texture(name,rgb565);if(!t)return;
    tile=std::min(tile,columns*rows-1);
    float u=float(t->width)/t->pw/columns,v=float(t->height)/t->ph/rows,hw=t->pw*u*.5f,hh=t->ph*v*.5f;
    float u0=(tile%columns)*u,v0=(tile/columns)*v;const float a=float(alpha);
    Vertex vertices[]={{-hw,-hh,u0,v0+v,1,1,1,a},{hw,-hh,u0+u,v0+v,1,1,1,a},{-hw,hh,u0,v0,1,1,1,a},{hw,hh,u0+u,v0,1,1,1,a}};
    setSpriteBlend(blend);
    float m[16];spriteMatrix(m,position,s.width,s.height,rotation,true);
    submit(vertices,4,GL_TRIANGLE_STRIP,m,t,Env::replace);
}
void GLRenderer::drawRegion(const std::string& name,Rect source,Rect d,bool rgb565){
    if(!active_||isEmpty(source)||isEmpty(d))return;
    Texture* t=texture(name,rgb565);if(!t)return;
    // CGFloat (double) arithmetic, then GLfloat storage, as in HMGLRenderer.
    const float x=float(d.origin.x),y=float(d.origin.y),w=float(d.size.width),h=float(d.size.height);
    const float u=float(source.origin.x/t->pw),v=float(source.origin.y/t->ph),uw=float(source.size.width/t->pw),vh=float(source.size.height/t->ph);
    Vertex vertices[]={{x,y,u,v+vh,1,1,1,1},{x+w,y,u+uw,v+vh,1,1,1,1},{x,y+h,u,v,1,1,1,1},{x+w,y+h,u+uw,v,1,1,1,1}};
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    // glLoadIdentity; glTranslatef(drawingOffset); glScalef(drawingScale)
    float m[16];identity(m);translate(m,float(drawingOffset_.x),float(drawingOffset_.y));scale(m,float(drawingScale_),float(drawingScale_));
    // beginFrame sets GL_REPLACE and drawEdgeContinuation restores it; drawRegion inherits REPLACE.
    submit(vertices,4,GL_TRIANGLE_STRIP,m,t,Env::replace);
}
void GLRenderer::drawJoinedBackground(const std::string& name,Point position,double s,double rotation,bool rgb565){
    if(!active_||!std::isfinite(position.x)||!std::isfinite(position.y)||!std::isfinite(s)||s<=0||!std::isfinite(rotation))return;
    Texture* t=texture(name,rgb565);if(!t)return;
    float hw=t->width*.5f,hh=t->height*.5f;
    float xs[]={-hw,-hw+.5f,hw-.5f,hw},ys[]={-hh,-hh+.5f,hh-.5f,hh};
    float us[]={.5f/t->pw,.5f/t->pw,(t->width-.5f)/t->pw,(t->width-.5f)/t->pw};
    float vs[]={(t->height-.5f)/t->ph,(t->height-.5f)/t->ph,.5f/t->ph,.5f/t->ph};
    Vertex vertices[54];int n=0;
    const int corners[][2]={{0,0},{1,0},{0,1},{0,1},{1,0},{1,1}};
    for(int y=0;y<3;++y)for(int x=0;x<3;++x)for(auto& corner:corners){
        int ix=x+corner[0],iy=y+corner[1];vertices[n++]={xs[ix],ys[iy],us[ix],vs[iy],1,1,1,1};
    }
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    float m[16];spriteMatrix(m,position,s,s,rotation,true);
    submit(vertices,size_t(n),GL_TRIANGLES,m,t,Env::replace);
}
void GLRenderer::drawExpandedPanel(const std::string& name,Point position,double left,double right,double top,double bottom,bool rgb565){
    if(!active_)return;
    Texture* t=texture(name,rgb565);if(!t)return;
    // Only the outer 16px metal strips stretch. Interior coordinates/UVs remain 1:1.
    const float hw=t->width*.5f,hh=t->height*.5f,edge=16;
    const float xs[]={-hw-float(left),-hw+edge,hw-edge,hw+float(right)};
    const float ys[]={-hh-float(bottom),-hh+edge,hh-edge,hh+float(top)};
    const float us[]={1.5f/t->pw,edge/t->pw,(t->width-edge)/t->pw,(t->width-1.5f)/t->pw};
    const float vs[]={(t->height-1.5f)/t->ph,(t->height-edge)/t->ph,edge/t->ph,1.5f/t->ph};
    Vertex vertices[54];int n=0;
    const int corners[][2]={{0,0},{1,0},{0,1},{0,1},{1,0},{1,1}};
    for(int y=0;y<3;++y)for(int x=0;x<3;++x)for(auto& c:corners){int ix=x+c[0],iy=y+c[1];vertices[n++]={xs[ix],ys[iy],us[ix],vs[iy],1,1,1,1};}
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    float m[16];identity(m);translate(m,float(position.x),float(position.y));
    submit(vertices,size_t(n),GL_TRIANGLES,m,t,Env::replace);
}
void GLRenderer::drawEdgeContinuation(const std::string& name,Point position,double s,double rotation,double width,bool rgb565){
    if(!active_||width<=0||!std::isfinite(width)||!std::isfinite(s)||!std::isfinite(rotation))return;
    Texture* t=texture(name,rgb565);if(!t)return;
    width=std::min(width,double(t->width/4));float hw=t->width*.5f,hh=t->height*.5f;
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    float m[16];spriteMatrix(m,position,s,s,rotation,true);
    for(int side:{-1,1}){
        float l=side<0?-hw-float(width):hw,r=side<0?-hw:hw+float(width);
        float u0=side<0?(float(width)+.5f)/t->pw:(t->width-.5f)/t->pw;
        float u1=side<0?.5f/t->pw:(t->width-float(width)-.5f)/t->pw;
        float v=float(t->height)/t->ph,near=side<0?0:1,far=1-near;
        Vertex vertices[]={{l,-hh,u0,v,1,1,1,near},{r,-hh,u1,v,1,1,1,far},{l,hh,u0,0,1,1,1,near},{r,hh,u1,0,1,1,1,far}};
        // The only GL_MODULATE draw; the env returns to GL_REPLACE afterwards.
        submit(vertices,4,GL_TRIANGLE_STRIP,m,t,Env::modulate);
    }
}
void GLRenderer::drawTrail(const heavymach::TrailMesh& mesh,const std::string& name,bool rgb565,Point cameraOffset){
    if(!active_||mesh.vertices.size()<3)return;
    Texture* t=texture(name,rgb565);if(!t)return;
    if(mesh.setBlend)glBlendFunc(mesh.sourceFactor,mesh.destinationFactor);
    float m[16];identity(m);
    translate(m,float(drawingOffset_.x),float(drawingOffset_.y));scale(m,float(drawingScale_),float(drawingScale_));
    translate(m,float(mesh.translation.x-cameraOffset.x),float(mesh.translation.y-cameraOffset.y));
    std::vector<Vertex> vertices;vertices.reserve(mesh.vertices.size());
    for(const auto& p:mesh.vertices)vertices.push_back({p.position.x,p.position.y,p.uv.x,p.uv.y,p.color[0],p.color[1],p.color[2],p.color[3]});
    submit(vertices.data(),vertices.size(),GL_TRIANGLE_STRIP,m,t,Env::replace);
}

bool GLRenderer::present(double cssWidth,double cssHeight,int drawableWidth,int drawableHeight,Rect viewport){
    if(!active_||cssWidth<=0||cssHeight<=0||drawableWidth<=0||drawableHeight<=0||isEmpty(viewport))return false;
    glBindFramebuffer(GL_FRAMEBUFFER,sceneFBO_);
    glBindTexture(GL_TEXTURE_2D,sceneTexture_);
    if(sceneTextureAllocated_)glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,bufferWidth_,bufferHeight_);
    else {glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,0,0,bufferWidth_,bufferHeight_,0);sceneTextureAllocated_=true;}
    glDisable(GL_SCISSOR_TEST);glBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,drawableWidth,drawableHeight);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);
    const double sx=drawableWidth/cssWidth,sy=drawableHeight/cssHeight;
    glViewport(GLint(std::lround(viewport.origin.x*sx)),GLint(std::lround((cssHeight-maxY(viewport))*sy)),GLsizei(std::lround(viewport.size.width*sx)),GLsizei(std::lround(viewport.size.height*sy)));
    const GLboolean dither=glIsEnabled(GL_DITHER);glDisable(GL_DITHER);glDisable(GL_BLEND);
    Vertex vertices[]={{-1,-1,0,0,1,1,1,1},{-1,1,0,1,1,1,1,1},{1,-1,1,0,1,1,1,1},{1,-1,1,0,1,1,1,1},{-1,1,0,1,1,1,1,1},{1,1,1,1,1,1,1,1}};
    float identityMatrix[16];identity(identityMatrix);
    glUniformMatrix4fv(uMatrix_,1,GL_FALSE,identityMatrix);glUniform1i(uMode_,0);glBindTexture(GL_TEXTURE_2D,sceneTexture_);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STREAM_DRAW);glDrawArrays(GL_TRIANGLES,0,6);
    if(dither)glEnable(GL_DITHER);
    glEnable(GL_BLEND);glBindFramebuffer(GL_FRAMEBUFFER,sceneFBO_);glViewport(0,0,bufferWidth_,bufferHeight_);applyClip();
    return true;
}
void GLRenderer::finish(){glFinish();}
std::string GLRenderer::diagnostics() const{
    char text[512];
    std::snprintf(text,sizeof(text),"backend=WebGL2 (GLES1.1 state emulation) logical=%gx%g buffer=%dx%d reallocations=%u vendor=%s renderer=%s version=%s textures=%zu uploads=%u drawCalls=%u frames=%u lastError=%s",
        logicalWidth_,logicalHeight_,bufferWidth_,bufferHeight_,sceneReallocations_,vendor_.c_str(),renderer_.c_str(),version_.c_str(),textures_.size(),uploads_,drawCalls_,frames_,lastError_.c_str());
    return text;
}
}
