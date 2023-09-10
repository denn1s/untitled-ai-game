#pragma once

#include <SDL2/SDL.h>
#include <SDL_render.h>

#include "ECS/System.h"

class UiSetupSystem : public SetupSystem {
public:
  UiSetupSystem(SDL_Renderer* renderer);
  void run() override;
  
private:
  SDL_Renderer* renderer;
};

class UiRenderSystem : public RenderSystem {
public:
  void run(SDL_Renderer* renderer) override;
};

class CharacterSetupSystem : public SetupSystem {
public:
  void run() override;
};

class BackgroundSetupSystem : public SetupSystem {
public:
  void run() override;
};

class SpriteRenderSystem : public RenderSystem {
public:
  void run(SDL_Renderer* renderer) override;
};

class SpriteSetupSystem : public SetupSystem {
public:
  SpriteSetupSystem(SDL_Renderer* renderer);
  ~SpriteSetupSystem();

  void run() override;

private:
  SDL_Renderer* renderer;
}; 

class SpriteUpdateSystem : public UpdateSystem {
public:
  void run(double dT);
};

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


