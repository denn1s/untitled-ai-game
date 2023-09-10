#include "PocketAi.h"

#include <print.h>
#include <constants.h>

#include "ECS/Entity.h"

#include "Systems.h"
#include "Components.h"


PocketAi::PocketAi()
  : Game("Ai", SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE) {
  Scene* gameplayScene = createGameplayScene();
  setScene(gameplayScene);
}

PocketAi::~PocketAi() {
    // destructor implementation
}

Scene* PocketAi::createGameplayScene() {
  Scene* scene = new Scene("GAMEPLAY SCENE");

  // ui systems
  scene->addSetupSystem<UiSetupSystem>(renderer);
  scene->addRenderSystem<UiRenderSystem>();

  // sprite systems
  scene->addSetupSystem<CharacterSetupSystem>();
  scene->addRenderSystem<SpriteRenderSystem>();
  scene->addSetupSystem<BackgroundSetupSystem>();

  scene->addSetupSystem<SpriteSetupSystem>(renderer);
  scene->addUpdateSystem<SpriteUpdateSystem>();

  // font
  scene->addEventSystem<PlayerTextInputSystem>();
  scene->addSetupSystem<PlayerTextSetupSystem>();
  scene->addRenderSystem<PlayerTextRenderSystem>();
  

  return scene;
}

