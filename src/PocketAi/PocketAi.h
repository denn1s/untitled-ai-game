#pragma once

#include "Game/Game.h"

#include <queue>

class PocketAi : public Game {
  public:
    PocketAi();
    ~PocketAi();

    void setup() override;
    void sceneTransition();

  private:
    std::queue<Scene*> scenes;
    Scene* createLogoScene();
    Scene* createCreditsScene();
    Scene* createGameplayScene();
};
