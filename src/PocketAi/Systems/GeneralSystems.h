#pragma once

#include <SDL2/SDL.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <functional>

#include "ECS/System.h"

class CharacterSetupSystem : public SetupSystem {
public:
  /* CharacterSetupSystem(); */
  ~CharacterSetupSystem();
  void run() override;
};

class SceneTransitionOnSlideUpdateSystem : public UpdateSystem {
public:
  SceneTransitionOnSlideUpdateSystem(std::function<void()> changeScene);
  void run(double dT) override;
private:
  std::function<void()> changeScene;
};

class PressStartEventSystem : public EventSystem {
public:
  PressStartEventSystem(std::function<void()> changeScene);
  void run(SDL_Event event) override;
private:
  std::function<void()> changeScene;
};

class AffectionSetupSystem : public SetupSystem {
public:
  void run() override;
};

class ConversationSetupSystem : public SetupSystem {
public:
  ConversationSetupSystem(int maxLines);
  void run() override;
private:
  int maxLines;
};

