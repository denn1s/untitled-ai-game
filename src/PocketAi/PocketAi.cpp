#include "PocketAi.h"

#include <print.h>
#include <constants.h>

#include "ECS/Components.h"
#include "ECS/Entity.h"

#include "PocketAi/Ai/AiManager.h"
#include "PocketAi/Systems/AiSystems.h"
#include "PocketAi/Systems/GeneralSystems.h"
#include "PocketAi/Systems/SpriteSystems.h"
#include "PocketAi/Systems/TextSystems.h"
#include "Systems/Systems.h"
#include "Components.h"


PocketAi::PocketAi()
  : Game("Ai", SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE) { }

PocketAi::~PocketAi() {
    // destructor implementation
  // temporarily clean the ai manager here
  AiManager::tearDown();
}

void PocketAi::sceneTransition() {
  delete this->getCurrentScene();
  this->setScene(nullptr);

  if (!this->scenes.empty()) {
    Scene* newScene = this->scenes.front();
    this->setScene(newScene);
    this->scenes.pop();
  }
}

void PocketAi::setup() {
  Scene* logoScene = createLogoScene();
  Scene* creditsScene = createCreditsScene();

  scenes.push(creditsScene); 
  
  setScene(logoScene);
}

Scene* PocketAi::createCreditsScene() {
  Scene* scene = new Scene("CREDITS SCENE", r);
  SpriteComponent sprite = {
        "Backgrounds/credits.png",
        160, 144
  };
  addSetupSystem<SlideShowSetupSystem>(scene, sprite, 5, 3000);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addUpdateSystem<SpriteUpdateSystem>(scene);
  addUpdateSystem<SlideShowUpdateSystem>(scene);
  addRenderSystem<SpriteRenderSystem>(scene);

  addUpdateSystem<SceneTransitionOnSlideUpdateSystem>(
    scene,
    std::bind(&PocketAi::sceneTransition, this)
  );

  return scene;
}

Scene* PocketAi::createLogoScene() {
  Scene* scene = new Scene("LOGO SCENE", r);
  SpriteComponent sprite = {
        "Backgrounds/gamedev.png",
        160, 144,
        0, 0,
        31, 2000,
        PixelShader{ nullptr, "" },
        SDL_GetTicks(),
        true,
        2000
  };
  addSetupSystem<SlideShowSetupSystem>(scene, sprite, 1, 6000);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addUpdateSystem<SpriteUpdateSystem>(scene);
  addUpdateSystem<SlideShowUpdateSystem>(scene);
  addRenderSystem<SpriteRenderSystem>(scene);

  addUpdateSystem<SceneTransitionOnSlideUpdateSystem>(
    scene,
    std::bind(&PocketAi::sceneTransition, this)
  );

  return scene;
}

Scene* PocketAi::createGameplayScene() {
  Scene* scene = new Scene("GAMEPLAY SCENE", r);
  addSetupSystem<CharacterSetupSystem>(scene);
  
  // sprite / ui systems
  addSetupSystem<UiSetupSystem>(scene, renderer);
  addRenderSystem<SpriteRenderSystem>(scene);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addUpdateSystem<SpriteUpdateSystem>(scene);

  // ai systems
  addSetupSystem<AiSetupSystem>(scene);
  addUpdateSystem<AiPromptProcessingSystem>(scene);
  addUpdateSystem<AiPromptPostProcessingSystem>(scene);
  addUpdateSystem<AiEmotionProcessingSystem>(scene);
  addUpdateSystem<UiUpdateSystem>(scene);

  // text systems
  addEventSystem<PlayerTextInputSystem>(scene);
  addSetupSystem<PlayerTextSetupSystem>(scene);
  /* scene->addUpdateSystem<PlayerTextOutputProcessSystem>(); */
  addRenderSystem<PlayerTextRenderSystem>(scene);
  addRenderSystem<SampleRenderSystem>(scene);
  addRenderSystem<PlayerCursorRenderSystem>(scene);
  

  return scene;
}

