#pragma once
// C++ translation of Sources/iOS/HMOpenALAudio. On the web it links against
// Emscripten's OpenAL 1.1 implementation (Web Audio backend) instead of the
// vendored OpenAL Soft 1.22.2; buffer/source reuse, distance model and the
// position -> stop -> play order are unchanged.
#include <AL/al.h>
#include <AL/alc.h>
#include <map>
#include <string>
#include <vector>

namespace hmweb {
class Audio {
public:
    explicit Audio(std::string audioDirectory);
    ~Audio();
    bool prepareOriginalSounds();
    void setApplicationActive(bool active);
    void sound(const std::string& name,float sourceX);
    std::string diagnostics() const;
private:
    struct Voice {ALuint buffer=0,source=0;unsigned starts=0;};
    void recordError(const std::string& message);
    bool loadSoundData(const std::vector<uint8_t>& data,const std::string& name);
    bool loadSoundFile(const std::string& name);
    std::string directory_;
    ALCdevice* device_=nullptr;
    ALCcontext* context_=nullptr;
    std::map<std::string,Voice> voices_;
    std::vector<std::string> creationOrder_;
    std::string error_,vendor_,renderer_,version_;
    unsigned startCount_=0,stopAllCount_=0;
    bool applicationActive_=true;
};
}
