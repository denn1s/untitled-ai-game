#pragma once

#include "Game/Game.h"


class PocketAi : public Game {
  public:
    PocketAi();
    ~PocketAi();

    void setup() override;

  private:
    Scene* createLogoScene();
    Scene* createGameplayScene();
};
