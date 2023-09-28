#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_pixels.h>
#include <SDL_stdinc.h>

#include "ECS/Components.h"
#include "Game/Graphics/Texture.h"
#include "Game/Graphics/PixelShader.h"

struct PlayerTextComponent {
  int x = 0;
  int y = 0;
  SDL_Color color{226, 246, 228};
  int maxLineLength = 28;
  int maxLines = 7;
  TTF_Font* font = nullptr;
  short fontSize = 8;
  SDL_Rect lastLineRect{-1, -1, -1, -1};
  std::string text = "";
};

struct PlayerPromptComponent {
  std::string ainame;
  std::string username;
  std::string currentPrompt = "";
  bool isInteracting = false;
};

struct PlayerEmotionComponent {
  std::string emotion;
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
  Uint32 minLetterTime = 0;
  Uint32 lastLetterTime = 0;
};

struct AffectionComponent {
  int affection = 60;
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
