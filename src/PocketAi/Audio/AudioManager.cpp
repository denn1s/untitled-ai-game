#include <print.h>
#include "AudioManager.h"

// Initialize static members
FMOD::System* AudioManager::system = nullptr;
FMOD::Sound* AudioManager::sound = nullptr;
FMOD::Channel* AudioManager::channel = 0;
FMOD_RESULT AudioManager::result;



void AudioManager::Init() {
    FMOD::System_Create(&system);
    result = system->init(32, FMOD_INIT_NORMAL, 0);
}

void AudioManager::PlaySong(const std::string& filePath) {
    if (sound != nullptr) {
        sound->release();
        sound = nullptr;
    }

    print("playing sound", filePath);

    result = system->createSound(("assets/" + filePath).c_str(), FMOD_DEFAULT, 0, &sound);  // Replace with your own audio file
    result = sound->setMode(FMOD_LOOP_NORMAL);
    result = system->playSound(sound, 0, false, &channel);
}

void AudioManager::Cleanup() {
    if (sound != nullptr) {
        sound->release();
        sound = nullptr;
    }

    system->close();
    system->release();
}
