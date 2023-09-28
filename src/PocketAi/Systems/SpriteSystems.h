#pragma once

#include <SDL2/SDL.h>
#include <SDL_render.h>

#include "ECS/Components.h"
#include "ECS/System.h"

class UiSetupSystem : public SetupSystem {
public:
  UiSetupSystem(SDL_Renderer* renderer, std::string spriteFile, int day = 0);
  ~UiSetupSystem();
  void run() override;
  
private:
  SDL_Renderer* renderer;
  std::string spriteFile;
  int day;
};

class UiUpdateSystem : public UpdateSystem {
public:
  void run(double dT) override;
};

class BackgroundSetupSystem : public SetupSystem {
public:
  BackgroundSetupSystem(int day = 0);
  ~BackgroundSetupSystem();
  void run() override;

private:
  int day;
  Entity* bg;
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
  SlideShowSetupSystem(SpriteComponent sprite, short slideCount = 0, int slideDurationMillis = -1);
  ~SlideShowSetupSystem();
  void run() override;

private:
  Entity* slider;  // we keep this reference to delete it later
  SpriteComponent sprite;
  short slideCount;
  int slideDurationMillis;
};

class SlideShowUpdateSystem : public UpdateSystem {
public:
  void run(double dT) override;
};

class BlushSetupSystem : public SetupSystem {
public:
  BlushSetupSystem(SDL_Renderer* renderer);
  void run() override;
private:
  SDL_Renderer* renderer;
};

class BlushRenderSystem : public RenderSystem {
public:
  void run(SDL_Renderer* renderer) override;
};


