#include "TextureManager.h"
#include <iostream>
#include <print.h>

std::map<std::string, Texture*> TextureManager::textures;

Texture* TextureManager::LoadTexture(const std::string& fileName, SDL_Renderer* renderer, PixelShader shader) {
    if (textures.count(fileName + shader.name) > 0) {
        return textures[fileName + shader.name];
    }
    
    Texture* tex = new Texture(renderer);
    tex->load("assets/" + fileName, shader);
    
    textures[fileName + shader.name] = tex;    
    return tex;
}

void TextureManager::UnloadTexture(const std::string& fileName, const std::string& shaderName) {
    if (textures.count(fileName + shaderName) > 0) {
        delete textures[fileName + shaderName];
        textures.erase(fileName + shaderName);
    }
}

Texture* TextureManager::GetTexture(const std::string& fileName, const std::string& shaderName) {
    if(textures.count(fileName + shaderName) > 0) {
        return textures[fileName + shaderName];
    }
    
    return nullptr;
}
