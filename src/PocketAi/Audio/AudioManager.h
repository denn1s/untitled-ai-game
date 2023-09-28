#include <fmod.hpp>
#include <string>

class AudioManager {
private:
    static FMOD::System* system;
    static FMOD::Sound* sound;
    static FMOD::Channel* channel;
    static FMOD_RESULT result;

public:
    static void Init();
    static void PlaySong(const std::string& filePath);
    static void Cleanup();
};
