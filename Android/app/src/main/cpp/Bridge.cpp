#include "Runtime.hpp"
#include <jni.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <android/log.h>
namespace {
hm::Runtime* runtime(jlong p){return reinterpret_cast<hm::Runtime*>(p);}
std::string text(JNIEnv* e,jstring s){if(!s)return {};const char*p=e->GetStringUTFChars(s,nullptr);if(!p)return {};std::string result(p);e->ReleaseStringUTFChars(s,p);return result;}
template<class F>void run(jlong p,F f){if(!p)return;try{f(*runtime(p));}catch(const std::exception& e){runtime(p)->error=e.what();__android_log_print(ANDROID_LOG_ERROR,"HeavyMach","%s",e.what());runtime(p)->active(false);}}
}
extern "C" JNIEXPORT jlong JNICALL Java_com_gelb2_heavymach_Native_create(JNIEnv*e,jobject,jobject assets,jstring files){try{return reinterpret_cast<jlong>(new hm::Runtime(AAssetManager_fromJava(e,assets),text(e,files)));}catch(const std::exception& x){e->ThrowNew(e->FindClass("java/lang/IllegalStateException"),x.what());return 0;}}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_heavymach_Native_surface(JNIEnv*e,jobject,jlong p,jobject surface){ANativeWindow*w=surface?ANativeWindow_fromSurface(e,surface):nullptr;run(p,[&](auto&r){r.surface(w);});if(w)ANativeWindow_release(w);}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_heavymach_Native_active(JNIEnv*,jobject,jlong p,jboolean on){run(p,[&](auto&r){r.active(on);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_heavymach_Native_insets(JNIEnv*,jobject,jlong p,jint l,jint t,jint right,jint b){run(p,[&](auto&r){r.insets(l,t,right,b);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_heavymach_Native_frame(JNIEnv*,jobject,jlong p,jdouble time){run(p,[&](auto&r){if(r.error.empty())r.frame(time);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_heavymach_Native_sensor(JNIEnv*,jobject,jlong p,jdouble x,jdouble y,jdouble z){run(p,[&](auto&r){r.sensor(x,y,z);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_heavymach_Native_touch(JNIEnv*,jobject,jlong p,jint id,jint state,jfloat x,jfloat y){run(p,[&](auto&r){r.touch(id,state,x,y);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_heavymach_Native_key(JNIEnv*,jobject,jlong p,jint key,jboolean down){run(p,[&](auto&r){r.key(unsigned(key),down);});}
extern "C" JNIEXPORT void JNICALL Java_com_gelb2_heavymach_Native_command(JNIEnv*e,jobject,jlong p,jstring command,jint a,jint b,jint c,jint d){auto name=text(e,command);run(p,[&](auto&r){try{r.command(name,a,b,c,d);}catch(const std::invalid_argument& x){r.notice=x.what();}});}
extern "C" JNIEXPORT jstring JNICALL Java_com_gelb2_heavymach_Native_ui(JNIEnv*e,jobject,jlong p){std::string value="{}";run(p,[&](auto&r){value=r.ui();});return e->NewStringUTF(value.c_str());}

extern "C" JNIEXPORT jstring JNICALL Java_com_gelb2_heavymach_Native_transfer(JNIEnv*e,jobject,jlong p,jstring action,jstring code,jint source,jint target){
 std::string result;try{if(!p)throw std::runtime_error("Game is not ready");result=runtime(p)->transfer(text(e,action),text(e,code),source,target);}catch(const std::exception&x){result=std::string("ERROR: ")+x.what();}return e->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT void JNICALL Java_com_gelb2_heavymach_Native_text(JNIEnv*e,jobject,jlong p,jstring name,jstring value){auto n=text(e,name),v=text(e,value);run(p,[&](auto&r){r.inputText(n,v);});}
