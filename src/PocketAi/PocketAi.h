#pragma once

#include "Game/Game.h"


class PocketAi : public Game {
  public:
    PocketAi();
    ~PocketAi();

  private:
    Scene* createGameplayScene();
};
