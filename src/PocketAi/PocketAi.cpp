#include "PocketAi.h"

#include <print.h>
#include <constants.h>

#include "ECS/Entity.h"

#include "PocketAi/Ai/AiManager.h"
#include "PocketAi/Systems/AiSystems.h"
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

void PocketAi::setup() {
  Scene* logoScene = createLogoScene();
  setScene(logoScene);
}

Scene* PocketAi::createLogoScene() {
  Scene* scene = new Scene("LOGO SCENE", r);
  addSetupSystem<SlideShowSetupSystem>(scene, "Backgrounds/gamedev.png", 1, 10000);
  addSetupSystem<SpriteSetupSystem>(scene, renderer);
  addUpdateSystem<SpriteUpdateSystem>(scene);
  /* addUpdateSystem<SlideShowUpdateSystem>(scene); */
  addRenderSystem<SpriteRenderSystem>(scene);
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

