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

class SampleRenderSystem : public RenderSystem {
public:
  void run(SDL_Renderer* renderer);
};

class TextCrawlUpdateSystem : public UpdateSystem {
public:
  TextCrawlUpdateSystem(const std::string& text, int lettersPerSecond);
  void run(double dT);

private:
  std::string text;
  int framesPerLetter;
  int frameCount;
};

class TextCrawlEventSystem : public EventSystem {
public:
  TextCrawlEventSystem(const std::string& text, std::function<void()> changeScene);
  void run(SDL_Event event);

private:
  std::string text;
  std::function<void()> changeScene;
};


