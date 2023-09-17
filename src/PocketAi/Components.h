#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ECS/Components.h"
#include "Game/Graphics/Texture.h"
#include "Game/Graphics/PixelShader.h"

struct PlayerTextComponent {
  std::string text;
  TTF_Font* font;
  short fontSize;
  SDL_Rect lastLineRect{-1, -1, -1, -1};
};

struct PlayerPromptComponent {
  std::string ainame;
  std::string username;
  std::string currentPrompt = "";
  bool isInteracting = false;
};

struct PlayerEmotionComponent {
  std::string emotion;
  int affection = 40;
  bool isProcessingEmotion = false;
};

