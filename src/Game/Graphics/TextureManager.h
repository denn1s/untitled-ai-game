#pragma once
#include "Texture.h"
#include "PixelShader.h"
#include <map>
#include <string>

class TextureManager {
  public:
    static Texture* LoadTexture(const std::string& fileName, SDL_Renderer* renderer, PixelShader shader = {nullptr, ""});
    static void UnloadTexture(const std::string& fileName, const std::string& shaderName = "");
    static Texture* GetTexture(const std::string& fileName, const std::string& shaderName = "");
  private:
    static std::map<std::string, Texture*> textures;
};
