#pragma once

#include <SDL2/SDL.h>
#include <SDL_render.h>

#include "ECS/System.h"

class PlayerTextInputSystem : public EventSystem {
public:
  void run(SDL_Event event);
};

class PlayerTextSetupSystem : public SetupSystem {
public:
  void run();
};

class PlayerTextRenderSystem : public RenderSystem {
public:
  void run(SDL_Renderer* renderer);
};

class PlayerCursorRenderSystem : public RenderSystem {
public:
  void run(SDL_Renderer* renderer);
};

class PlayerTextOutputProcessSystem : public UpdateSystem {
public:
  void run(double dT);
};

