#pragma once

#include <SDL2/SDL.h>
#include <SDL_render.h>

#include "ECS/System.h"

class CharacterSetupSystem : public SetupSystem {
public:
  void run() override;
};

