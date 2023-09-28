#pragma once

#include <SDL2/SDL.h>
#include <SDL_render.h>

#include "ECS/System.h"

class PlayerTextInputSystem : public EventSystem {
public:
  void run(SDL_Event event) override;
};

class PlayerTextSetupSystem : public SetupSystem {
public:
  PlayerTextSetupSystem(
    int textPositionX = 10,
    int textPositionY = 100,
    int maxLineLength = 28,
    int maxLines = 7,
    SDL_Color textColor = {226, 246, 228}
    /* SDL_Color textColor = { 51,  44,  80} */
  );
  void run() override;
private:
  int textPositionX;
  int textPositionY;
  int maxLineLength;
  int maxLines;
  SDL_Color textColor;
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


