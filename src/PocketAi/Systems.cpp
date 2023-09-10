#include "Systems.h"

#include <print.h>
#include <constants.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "Game/Graphics/TextureManager.h"
#include "TextProcessingQueue.h"

#include "Components.h"

UiSetupSystem::UiSetupSystem(SDL_Renderer* renderer)
    : renderer(renderer) { }

void UiSetupSystem::run() {
    TextureManager::LoadTexture("UI/main.png", renderer);
}

void UiRenderSystem::run(SDL_Renderer* renderer) {
    Texture* texture = TextureManager::GetTexture("UI/main.png");

    texture->render(
        0, 0,
        SCREEN_WIDTH * SCALE,
        SCREEN_HEIGHT * SCALE
    );
}

void CharacterSetupSystem::run() {
    scene->player = new Entity(scene->r.create(), scene);
    scene->player->addComponent<TransformComponent>(0, 24 * SCALE);  // ui offset
    scene->player->addComponent<SpriteComponent>("Characters/main.png");
    scene->player->addComponent<PlayerTextComponent>();
}

void BackgroundSetupSystem::run() {
    Entity bg = scene->createEntity("BG", 0, 24 * SCALE);
    bg.addComponent<SpriteComponent>(
        "Backgrounds/starry-sky.png",
         160, 65,
         0,  0,
         8,
         2000
    );
}

void SpriteRenderSystem::run(SDL_Renderer* renderer) {
    auto view = scene->r.view<TransformComponent, SpriteComponent>();

    for(auto entity : view) {
        const auto spriteComponent = view.get<SpriteComponent>(entity);
        const auto transformComponent = view.get<TransformComponent>(entity);

        Texture* texture = TextureManager::GetTexture(spriteComponent.name, spriteComponent.shader.name);
        const int width = spriteComponent.w > 0 ? spriteComponent.w : texture->width;
        const int height = spriteComponent.h > 0 ? spriteComponent.h : texture->height;

        SDL_Rect clip = {
            spriteComponent.xIndex * width,
            spriteComponent.yIndex * height,
            width,
            height,
        };

        texture->render(
            transformComponent.x,
            transformComponent.y,
            width * SCALE,
            height * SCALE,
            &clip
        );
    }
}

SpriteSetupSystem::~SpriteSetupSystem() {
    auto view = scene->r.view<SpriteComponent>();

    for(auto entity : view) {
        const auto spriteComponent = view.get<SpriteComponent>(entity);

        TextureManager::UnloadTexture(spriteComponent.name, spriteComponent.shader.name);
    }
}

SpriteSetupSystem::SpriteSetupSystem(SDL_Renderer* renderer)
    : renderer(renderer) { }

void SpriteSetupSystem::run() {
    auto view = scene->r.view<SpriteComponent>();

    for(auto entity : view) {
        const auto spriteComponent = view.get<SpriteComponent>(entity);

        TextureManager::LoadTexture(spriteComponent.name, renderer, spriteComponent.shader);
    }
}

void SpriteUpdateSystem::run(double dT) {
    auto view = scene->r.view<SpriteComponent>();

    Uint32 now = SDL_GetTicks();

    for(auto entity : view) {
        auto& spriteComponent = view.get<SpriteComponent>(entity);

        if (spriteComponent.animationFrames > 0) {
            if (!spriteComponent.lastUpdate) {
                spriteComponent.lastUpdate = SDL_GetTicks();
                continue;
            }

            float timeSinceLastUpdate = now - spriteComponent.lastUpdate;

            int framesToUpdate = static_cast<int>(
                timeSinceLastUpdate / 
                spriteComponent.animationDuration * spriteComponent.animationFrames
            );

            if (framesToUpdate > 0) {
                spriteComponent.xIndex += framesToUpdate;
                spriteComponent.xIndex %= spriteComponent.animationFrames;
                spriteComponent.lastUpdate = now;            
            }
        }
    }
}

void PlayerTextSetupSystem::run() {
    short fontSize = 4 * SCALE;

    TTF_Font* font = TTF_OpenFont("assets/Fonts/GamergirlClassic.ttf", fontSize);
    if (!font) {
        print("Failed to load font: %s\n", TTF_GetError());
        exit(1);
    }

    auto& playerTextComponent = scene->player->get<PlayerTextComponent>();
    playerTextComponent.font = font;
    playerTextComponent.fontSize = fontSize;
}

void PlayerTextInputSystem::run(SDL_Event event) {
    auto& playerTextComponent = scene->player->get<PlayerTextComponent>();

    if (event.type == SDL_TEXTINPUT) {
        playerTextComponent.text += event.text.text;
    } else  if (event.type == SDL_KEYDOWN && !playerTextComponent.text.empty()) {
        if (event.key.keysym.sym == SDLK_BACKSPACE) {
            playerTextComponent.text.pop_back();
        } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
            requestQueue.push(playerTextComponent.text);
            playerTextComponent.text.clear();
        }
    }
}

void PlayerTextRenderSystem::run(SDL_Renderer* renderer) {
    auto& playerTextComponent = scene->player->get<PlayerTextComponent>();

    if (playerTextComponent.text.empty()) {
        return;  // text has 0 length
    }
playerTextComponent.text = "HELLO WORLD";
    SDL_Color color = {226, 246, 228};
    SDL_Rect position = {10 * SCALE, 100 * SCALE, 0, 0};

    SDL_Surface* textSurface = TTF_RenderText_Solid(
        playerTextComponent.font,
        playerTextComponent.text.c_str(),
        color
    );

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    position.w = textSurface->w;
    position.h = textSurface->h;

    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, textTexture, NULL, &position);
    SDL_DestroyTexture(textTexture);
}

