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
    Scene* createJamScene();
    Scene* createTitleScene();
    Scene* createContextScene(int day);
    Scene* createGameplayScene(int day);
    Scene* createConclusionScene(bool isResponse);
    Scene* createEndingScene();
};
