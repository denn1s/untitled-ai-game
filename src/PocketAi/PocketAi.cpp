#include "PocketAi.h"

#include <print.h>
#include <constants.h>

#include "ECS/Entity.h"

#include "PocketAi/Ai/AiManager.h"
#include "PocketAi/Systems/AiSystems.h"
#include "PocketAi/Systems/TextSystems.h"
#include "Systems/Systems.h"
#include "Components.h"


PocketAi::PocketAi()
  : Game("Ai", SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE) {
  Scene* gameplayScene = createGameplayScene();
  setScene(gameplayScene);
}

PocketAi::~PocketAi() {
    // destructor implementation
  // temporarily clean the ai manager here
  AiManager::tearDown();
}

Scene* PocketAi::createGameplayScene() {
  Scene* scene = new Scene("GAMEPLAY SCENE");
  scene->addSetupSystem<CharacterSetupSystem>();
  
  // sprite / ui systems
  scene->addSetupSystem<UiSetupSystem>(renderer);
  scene->addRenderSystem<SpriteRenderSystem>();
  scene->addSetupSystem<BackgroundSetupSystem>();
  scene->addSetupSystem<SpriteSetupSystem>(renderer);
  scene->addUpdateSystem<SpriteUpdateSystem>();

  // ai systems
  scene->addSetupSystem<AiSetupSystem>();
  scene->addUpdateSystem<AiPromptProcessingSystem>();
  scene->addUpdateSystem<AiPromptPostProcessingSystem>();
  scene->addUpdateSystem<AiEmotionProcessingSystem>();
  scene->addUpdateSystem<UiUpdateSystem>();

  // text systems
  scene->addEventSystem<PlayerTextInputSystem>();
  scene->addSetupSystem<PlayerTextSetupSystem>();
  /* scene->addUpdateSystem<PlayerTextOutputProcessSystem>(); */
  scene->addRenderSystem<PlayerTextRenderSystem>();
  scene->addRenderSystem<SampleRenderSystem>();
  scene->addRenderSystem<PlayerCursorRenderSystem>();
  

  return scene;
}

