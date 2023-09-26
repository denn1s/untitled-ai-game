#include "SpriteSystems.h"

#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <print.h>
#include <constants.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "Game/Graphics/TextureManager.h"

#include "PocketAi/Components.h"

UiSetupSystem::UiSetupSystem(SDL_Renderer* renderer)
    : renderer(renderer) { }

void UiSetupSystem::run() {
    TextureManager::LoadTexture("UI/main.png", renderer);
    scene->world = new Entity(scene->r.create(), scene);
    scene->world->addComponent<TransformComponent>(0, 0);
    scene->world->addComponent<SpriteComponent>("UI/main.png", 160, 144, 0, 0);
}

void UiUpdateSystem::run(double dT) {
    auto& uiSpriteComponent = scene->world->get<SpriteComponent>();
    int affection = scene->player->get<PlayerEmotionComponent>().affection;

    uiSpriteComponent.xIndex = static_cast<int>(affection / 16);
}

void BackgroundSetupSystem::run() {
    Entity bg = scene->createEntity("BG", 0, 24 * SCALE);
    bg.addComponent<SpriteComponent>(
        "Backgrounds/starry-sky.png",
         160, 65,
         0,  2,
         8,
         2000
    );
}

SlideShowSetupSystem::SlideShowSetupSystem(const std::string& image, short slideCount, int slideDurationMillis)
    : image(image), slideCount(slideCount), slideDurationMillis(slideDurationMillis) {}

void SlideShowSetupSystem::run() {
    Entity slider = scene->createEntity("SLIDER", 0, 0);
    slider.addComponent<SpriteComponent>(
        image,
        160, 144,
        0, 0,
        31, 2000,
        PixelShader{ nullptr, "" },
        SDL_GetTicks(),
        true,
        2000
    );
    slider.addComponent<SlideShowComponent>(
        slideCount,
        slideDurationMillis,
        SDL_GetTicks()
    );
}

void SlideShowUpdateSystem::run(double dT) {
    Uint32 now = SDL_GetTicks();
    auto view = scene->r.view<SlideShowComponent, SpriteComponent>();

    for(auto entity : view) {
        auto& slideComponent = scene->r.get<SlideShowComponent>(entity);
        auto& spriteComponent = scene->r.get<SpriteComponent>(entity);

        if (slideComponent.slideCount > 0 && slideComponent.slideCount > slideComponent.currentSlide) {
            Uint32 timeSinceLastUpdate = now - slideComponent.lastUpdate;

            if (timeSinceLastUpdate >= slideComponent.slideDurationMillis) {
                slideComponent.lastUpdate = now; 
                slideComponent.currentSlide++;
                spriteComponent.xIndex = slideComponent.currentSlide;
            }
        }
    }
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

            if (spriteComponent.delay != 0) {
                if (timeSinceLastUpdate < spriteComponent.delay) {
                    continue;
                } else {
                    spriteComponent.delay = 0;
                    spriteComponent.lastUpdate = 0;
                    timeSinceLastUpdate = 0;
                }
            }
            
            int framesToUpdate = static_cast<int>(
                timeSinceLastUpdate / 
                spriteComponent.animationDuration * spriteComponent.animationFrames
            );

            if (framesToUpdate > 0) {
                if (!spriteComponent.once || spriteComponent.xIndex < (spriteComponent.animationFrames - 1)) {
                    spriteComponent.xIndex += framesToUpdate;
                    spriteComponent.xIndex %= spriteComponent.animationFrames;
                    spriteComponent.lastUpdate = now;
                }
            }
        }
    }
}

