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

struct SlideShowComponent {
  short slideCount;
  int slideDurationMillis = 0;
  Uint32 lastUpdate = 0;
  short currentSlide = 0;
};

struct ConversationComponent {
  int maxConversations;
  int countConversations;
};


/*
struct FadeComponent {
  SDL_Color fadeColor;
  Uint32 fadeTimeMillis;
  short alpha = 255;
  bool direction = 1; // 0 out, 1 in
  SDL_Surface* screen = nullptr;
  SDL_Surface* fade = nullptr;
};
*/
