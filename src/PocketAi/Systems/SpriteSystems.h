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

class UiUpdateSystem : public UpdateSystem {
public:
  void run(double dT) override;
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
  void run(double dT) override;
};

class SlideShowSetupSystem : public SetupSystem {
public:
  SlideShowSetupSystem(const std::string& image, short slideCount, int slideDurationMillis);
  void run() override;

private:
  std::string image;
  short slideCount;
  int slideDurationMillis;
};

class SlideShowUpdateSystem : public UpdateSystem {
public:
  void run(double dT) override;
};

