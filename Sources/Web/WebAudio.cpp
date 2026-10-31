#include "WebAudio.hpp"
#include "AudioContract.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>

namespace hmweb {
namespace {
uint16_t u16(const uint8_t* p){return uint16_t(p[0])|uint16_t(p[1])<<8;}
uint32_t u32(const uint8_t* p){return uint32_t(p[0])|uint32_t(p[1])<<8|uint32_t(p[2])<<16|uint32_t(p[3])<<24;}
struct PCM {const void* samples=nullptr;ALsizei bytes=0,rate=0;};
// Same RIFF validation as HMOpenALAudio decodeWAV: mono PCM16 only.
bool decodeWAV(const std::vector<uint8_t>& data,PCM& out){
    const uint8_t* p=data.data();size_t n=data.size();
    if(n<12||std::memcmp(p,"RIFF",4)||std::memcmp(p+8,"WAVE",4))return false;
    uint32_t declared=u32(p+4);if(declared<4||size_t(declared)>n-8)return false;
    size_t end=size_t(declared)+8;bool format=false,haveData=false;
    for(size_t pos=12;pos<end;){
        if(end-pos<8)return false;
        uint32_t size=u32(p+pos+4);pos+=8;
        if(size>end-pos)return false;
        if(!std::memcmp(p+pos-8,"fmt ",4)){
            if(format||size<16||u16(p+pos)!=1||u16(p+pos+2)!=1||u16(p+pos+14)!=16||u16(p+pos+12)!=2)return false;
            uint32_t rate=u32(p+pos+4);if(!rate||rate>uint32_t(std::numeric_limits<ALsizei>::max())||uint64_t(rate)*2!=u32(p+pos+8))return false;
            out.rate=ALsizei(rate);format=true;
        }else if(!std::memcmp(p+pos-8,"data",4)){
            if(haveData||!size||(size&1)||size>uint32_t(std::numeric_limits<ALsizei>::max()))return false;
            out.samples=p+pos;out.bytes=ALsizei(size);haveData=true;
        }
        pos+=size;
        if(size&1){if(pos>=end)return false;++pos;}
    }
    return format&&haveData;
}
}

void Audio::recordError(const std::string& message){error_=message;std::fprintf(stderr,"HMOpenALAudio: %s\n",message.c_str());}
Audio::Audio(std::string audioDirectory):directory_(std::move(audioDirectory)){
    device_=alcOpenDevice(nullptr);
    if(!device_){recordError("Cannot open OpenAL device");return;}
    context_=alcCreateContext(device_,nullptr);
    if(!context_||alcMakeContextCurrent(context_)!=ALC_TRUE){recordError("Cannot create or activate OpenAL context");return;}
    const ALchar *vendor=alGetString(AL_VENDOR),*renderer=alGetString(AL_RENDERER),*version=alGetString(AL_VERSION);
    vendor_=vendor?vendor:"";renderer_=renderer?renderer:"";version_=version?version:"";
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);alListener3f(AL_POSITION,0,0,0);alListenerf(AL_GAIN,1);
    if(ALenum e=alGetError();e!=AL_NO_ERROR){char text[64];std::snprintf(text,sizeof(text),"OpenAL initialization error 0x%x",e);recordError(text);}
}
Audio::~Audio(){
    if(context_){
        for(auto& entry:voices_){auto& v=entry.second;alSourceStop(v.source);alDeleteSources(1,&v.source);alDeleteBuffers(1,&v.buffer);}
        alcMakeContextCurrent(nullptr);alcDestroyContext(context_);
    }
    if(device_)alcCloseDevice(device_);
}
bool Audio::loadSoundData(const std::vector<uint8_t>& data,const std::string& name){
    if(name.empty())return false;
    if(!context_){recordError("OpenAL context unavailable");return false;}
    if(voices_.count(name))return true;
    PCM pcm;if(!decodeWAV(data,pcm)){recordError("Invalid mono PCM16 WAV: "+name);return false;}
    Voice voice;alGenBuffers(1,&voice.buffer);alBufferData(voice.buffer,AL_FORMAT_MONO16,pcm.samples,pcm.bytes,pcm.rate);
    alGenSources(1,&voice.source);alSourcei(voice.source,AL_BUFFER,ALint(voice.buffer));
    alSourcef(voice.source,AL_MAX_DISTANCE,heavymach::originalSoundMaxDistance(name));
    alSourcef(voice.source,AL_REFERENCE_DISTANCE,1);alSourcef(voice.source,AL_ROLLOFF_FACTOR,1);alSourcef(voice.source,AL_GAIN,1);
    alSource3f(voice.source,AL_POSITION,0,0,0);
    ALenum error=alGetError();
    if(error!=AL_NO_ERROR){
        if(voice.source)alDeleteSources(1,&voice.source);if(voice.buffer)alDeleteBuffers(1,&voice.buffer);
        char text[256];std::snprintf(text,sizeof(text),"OpenAL load %s error 0x%x",name.c_str(),error);recordError(text);return false;
    }
    voices_.emplace(name,voice);creationOrder_.push_back(name);return true;
}
bool Audio::loadSoundFile(const std::string& name){
    std::ifstream file(directory_+"/"+name+".wav",std::ios::binary);
    std::vector<uint8_t> data;
    if(file)data.assign(std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>());
    return loadSoundData(data,name);
}
bool Audio::prepareOriginalSounds(){
    bool ready=true;
    for(auto key:heavymach::originalInitialSoundNames){
        std::string name(key);
        if(voices_.count(name))continue;
        if(!loadSoundFile(name))ready=false;
    }
    return ready;
}
void Audio::sound(const std::string& name,float sourceX){
    if(!applicationActive_||name.empty()||!std::isfinite(sourceX))return;
    auto found=voices_.find(name);
    if(found==voices_.end()){
        if(!loadSoundFile(name))return;
        found=voices_.find(name);
    }
    if(!context_){recordError("Cannot activate OpenAL for playback");return;}
    auto& voice=found->second;
    // SoundManager 0x13d0c positions first; Start 0x1e9e0 then stops/restarts.
    alSource3f(voice.source,AL_POSITION,sourceX,0,0);alSourceStop(voice.source);alSourcePlay(voice.source);
    ALenum error=alGetError();
    if(error!=AL_NO_ERROR){char text[256];std::snprintf(text,sizeof(text),"OpenAL playback %s error 0x%x",name.c_str(),error);recordError(text);return;}
    voice.starts++;startCount_++;
}
// HeavyMach's original callbacks (0x225c/0x225e) are empty. This is the
// user's newer lifecycle policy: stop on inactivity, never replay old sounds.
void Audio::setApplicationActive(bool active){
    if(applicationActive_==active)return;
    applicationActive_=active;
    if(active||!context_)return;
    for(auto& entry:voices_)alSourceStop(entry.second.source);
    ALenum error=alGetError();
    if(error!=AL_NO_ERROR){char text[64];std::snprintf(text,sizeof(text),"OpenAL stop inactive sources error 0x%x",error);recordError(text);return;}
    ++stopAllCount_;
}
std::string Audio::diagnostics() const{
    char text[512];
    std::snprintf(text,sizeof(text),"backend=Emscripten OpenAL (Web Audio) vendor=%s renderer=%s version=%s available=%d active=%d stopAll=%u buffers=%zu starts=%u error=%s",
        vendor_.c_str(),renderer_.c_str(),version_.c_str(),context_!=nullptr,applicationActive_,stopAllCount_,voices_.size(),startCount_,error_.c_str());
    return text;
}
}
