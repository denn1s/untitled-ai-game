#include "GeneralSystems.h"

#include <print.h>
#include <constants.h>

#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "PocketAi/Components.h"


void CharacterSetupSystem::run() {
    scene->player = new Entity(scene->r.create(), scene);
    scene->player->addComponent<TransformComponent>(0, 24 * SCALE);  // ui offset
    scene->player->addComponent<SpriteComponent>("Characters/main.png", 160, 65, 0, 0);
    scene->player->addComponent<PlayerTextComponent>();
    scene->player->addComponent<PlayerEmotionComponent>();
    scene->player->addComponent<PlayerPromptComponent>("Pocket: ", "Rob: ");
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
