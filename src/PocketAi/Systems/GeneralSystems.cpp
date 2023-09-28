#include "GeneralSystems.h"

#include <print.h>
#include <constants.h>

#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "PocketAi/Components.h"

#include "PocketAi/Audio/AudioManager.h"

CharacterSetupSystem::~CharacterSetupSystem() {
    delete scene->player;
    scene->player = nullptr;
}

void CharacterSetupSystem::run() {
    if (scene->player == nullptr) {
        scene->player = new Entity(scene->r.create(), scene);
        scene->player->addComponent<TransformComponent>(0, 24 * SCALE);  // ui offset
        scene->player->addComponent<SpriteComponent>("Characters/main.png", 160, 65, 0, 0);
        scene->player->addComponent<PlayerEmotionComponent>();
        scene->player->addComponent<PlayerPromptComponent>("Pocket: ", "Rob: ");
    }
}

SceneTransitionOnSlideUpdateSystem::SceneTransitionOnSlideUpdateSystem(std::function<void()> changeScene)
  : changeScene(changeScene) { }

void SceneTransitionOnSlideUpdateSystem::run(double dT) {
    auto view = scene->r.view<SlideShowComponent>();

    for (auto entity : view) {
        auto& slideComponent = scene->r.get<SlideShowComponent>(entity);
   
        if (slideComponent.currentSlide >= slideComponent.slideCount) {
          changeScene();
        }
    }
}

PressStartEventSystem::PressStartEventSystem(std::function<void()> changeScene)
: changeScene(changeScene) { }

void PressStartEventSystem::run(SDL_Event event) {
    if (event.key.keysym.sym == SDLK_RETURN) {
        changeScene();
    }
}

void AffectionSetupSystem::run() {
    auto& registry = scene->r;
    registry.ctx().emplace<AffectionComponent>(60);
}

ConversationSetupSystem::ConversationSetupSystem(int maxLines)
    : maxLines(maxLines) { }

void ConversationSetupSystem::run() {
    scene->player->addComponent<ConversationComponent>(maxLines, 0, 100);
}

void MusicSetupSystem::run() {
    AudioManager::Init();
};

MusicPlaySystem::MusicPlaySystem(const std::string& song)
    : song(song) { }

void MusicPlaySystem::run() {
    AudioManager::PlaySong(song);
}


