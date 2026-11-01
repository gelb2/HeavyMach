// Browser entry point (counterpart of Sources/iOS/Application.mm).
// The page shell (Web/shell.html) owns DOM events and calls the exported hooks.
#include "WebGameView.hpp"
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <cstdio>
#include <memory>

namespace {
std::unique_ptr<hmweb::GameView> view;
std::string debugText;

// ?fps=30 (test only) delivers every second display callback, like Safari in
// iOS Low Power Mode; the game must still advance 60 ticks per second.
EM_JS(int,hm_callback_divisor,(),{const m=/[?&]fps=([0-9]+)/.exec(location.search);if(!m)return 1;const fps=Number(m[1]);return fps>0&&fps<60?Math.round(60/fps):1;});
int callbackDivisor=1,callbackCount=0;
void animationFrame(){
    if(callbackDivisor>1&&(callbackCount++%callbackDivisor))return;
    if(view)view->animationFrame(emscripten_get_now()/1000.0);
}
// NSTimer-style one-shot 50 Hz accelerometer deadline, independent of display callbacks.
void motionTick(void*){
    if(view)view->motionTimer(emscripten_get_now()/1000.0);
    emscripten_set_timeout(motionTick,1000.0*double(.02f)/2,nullptr);
}
}

extern "C" {
EMSCRIPTEN_KEEPALIVE int hm_transfer_available(){return view&&view->transferAvailable()?1:0;}
EMSCRIPTEN_KEEPALIVE void hm_resize(double cssWidth,double cssHeight,double ratio,double left,double top,double right,double bottom,int angle){
    if(!view)return;
    hmweb::HostSurface surface;surface.cssWidth=cssWidth;surface.cssHeight=cssHeight;surface.devicePixelRatio=ratio;
    surface.safeLeft=left;surface.safeTop=top;surface.safeRight=right;surface.safeBottom=bottom;surface.orientationAngle=angle;
    view->setSurface(surface);
}
EMSCRIPTEN_KEEPALIVE void hm_pointer(int token,double x,double y,int state){if(view)view->pointer(token,x,y,state);}
EMSCRIPTEN_KEEPALIVE int hm_key(int code,int pressed){return view&&code>0&&code<256&&view->key(unsigned(code),pressed!=0)?1:0;}
EMSCRIPTEN_KEEPALIVE void hm_keyboard_blur(){if(view)view->blurKeyboard();}
EMSCRIPTEN_KEEPALIVE void hm_motion(int available,double x,double y,double z){if(view)view->setAcceleration({available!=0,x,y,z});}
// active 1; 0 an interruption (hidden, blur, pagehide); 2 touch-device portrait only.
EMSCRIPTEN_KEEPALIVE void hm_active(int state){if(view)view->setApplicationActive(state==1,state==2);}
EMSCRIPTEN_KEEPALIVE void hm_context_lost(){if(view)view->loseRenderer();}
EMSCRIPTEN_KEEPALIVE int hm_context_restored(){return view&&view->restoreRenderer()?1:0;}
EMSCRIPTEN_KEEPALIVE void hm_hud_save_result(int success){if(view)view->hudSaveResult(success!=0);}
EMSCRIPTEN_KEEPALIVE void hm_hud_control(int tag){if(view)view->hudControl(tag);}
EMSCRIPTEN_KEEPALIVE const char* hm_debug_state(){debugText=view?view->debugState():"{}";return debugText.c_str();}
}

int main(){
    EmscriptenWebGLContextAttributes attributes;emscripten_webgl_init_context_attributes(&attributes);
    attributes.majorVersion=2;attributes.minorVersion=0;attributes.alpha=false;attributes.depth=false;attributes.stencil=false;
    attributes.antialias=false;attributes.premultipliedAlpha=false;attributes.preserveDrawingBuffer=false;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context=emscripten_webgl_create_context("#canvas",&attributes);
    view=std::make_unique<hmweb::GameView>("/assets","/persist");
    if(context<=0){std::fprintf(stderr,"WebGL2 unavailable (%d)\n",int(context));EM_ASM({if(Module.hmFatal)Module.hmFatal('이 브라우저는 WebGL2를 지원하지 않습니다.');});return 0;}
    emscripten_webgl_make_context_current(context);
    if(!view->initRenderer())return 0;
    EM_ASM({if(Module.hmReady)Module.hmReady();});
    emscripten_set_timeout(motionTick,1000.0*double(.02f),nullptr);
    callbackDivisor=hm_callback_divisor();
    emscripten_set_main_loop(animationFrame,0,false);
    return 0;
}
