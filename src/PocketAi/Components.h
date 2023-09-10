#pragma once

#include <SDL2/SDL_ttf.h>

#include "ECS/Components.h"
#include "Game/Graphics/Texture.h"
#include "Game/Graphics/PixelShader.h"

struct PlayerTextComponent {
  std::string text;
  TTF_Font* font;
  short fontSize;
};


