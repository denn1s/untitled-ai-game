#pragma once
#include "Game/Graphics/Texture.h"

struct Terrain {
  Texture* texture = nullptr;  // water, dirt, grass, etc
  int x = 0;  // x coordinate in the tilemap
  int y = 0;  // y component in the tilemap
};

struct Tile {
  Terrain up;
  Terrain down;
  bool needsAutoTiling = true;
  bool isWalkable = true;
};
