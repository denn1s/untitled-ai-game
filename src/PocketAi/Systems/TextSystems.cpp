#include "TextSystems.h"

#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <print.h>
#include <constants.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <deque>
#include <functional>

#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "Game/Graphics/TextureManager.h"

#include "PocketAi/Components.h"
#include "PocketAi/Ai/AiManager.h"

PlayerTextSetupSystem::PlayerTextSetupSystem(int textPositionX, int textPositionY, int maxLineLength, int maxLines, SDL_Color textColor)
    : textPositionX(textPositionX), textPositionY(textPositionY), maxLineLength(maxLineLength), maxLines(maxLines), textColor(textColor) { }

void PlayerTextSetupSystem::run() {
    short fontSize = 5 * SCALE;

    TTF_Font* font = TTF_OpenFont("assets/Fonts/GamergirlClassic.ttf", fontSize);
    if (!font) {
        print("Failed to load font: %s\n", TTF_GetError());
        exit(1);
    }

    if (scene->player == nullptr) {
        scene->player = new Entity(scene->r.create(), scene);
    }

    auto& p = scene->player->get<PlayerTextComponent>(
        textPositionX,
        textPositionY,
        textColor,
        maxLineLength,
        maxLines,
        font,
        fontSize
    );
}

void PlayerTextInputSystem::run(SDL_Event event) {
    auto& playerTextComponent = scene->player->get<PlayerTextComponent>();
    auto& playerPromptComponent = scene->player->get<PlayerPromptComponent>();

    if (!playerPromptComponent.isInteracting) {
        // we ignore all input if we are not interacting
        return;
    }

    if (event.type == SDL_TEXTINPUT) {
        playerTextComponent.text += event.text.text;
    } else  if (event.type == SDL_KEYDOWN && !playerTextComponent.text.empty()) {
        if (playerTextComponent.text == playerPromptComponent.currentPrompt) {
            // we don't allow edition if the prompt is the original
            return;
        }
        if (event.key.keysym.sym == SDLK_BACKSPACE) {
            playerTextComponent.text.pop_back();
        } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
            std::size_t pos = playerTextComponent.text.rfind(playerPromptComponent.username);
            if(pos != std::string::npos) {
                playerTextComponent.text += "\n";
                std::string prompt = playerTextComponent.text.substr(pos + playerPromptComponent.username.size());
                playerPromptComponent.isInteracting = false;       
                /* AiManager::requestQueue.push(prompt); */
                AiManager::requestQueue.push("/neutral " + prompt);  // slight hack to make her used to answering with emotions
            }
            /* playerTextComponent.text.clear(); */
        }
    }
    if (event.key.keysym.sym == SDLK_ESCAPE) {
        // small hack to unstuck the systems
        print("trying to unstuck");
        playerTextComponent.text += "\n";
        std::string prompt = "\nSorry, can you repeat that?";
        playerPromptComponent.isInteracting = true;  // this actually should be false, but since this is a safeguard      
        AiManager::requestQueue.push("Rob: /confused " + prompt);  // slight hack to make her used to answering with emotions
    }

}

void renderLine(SDL_Renderer* renderer, TTF_Font* font, const std::string& line, SDL_Rect& position, SDL_Color color) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, line.c_str(), color);
    
    if(textSurface == nullptr) {
        // Handle error
        return;
    }
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    position.w = textSurface->w;
    position.h = textSurface->h;

    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, textTexture, NULL, &position);
    SDL_DestroyTexture(textTexture);

    position.y += position.h; // Move to the next line
}

void handleLine(
    SDL_Renderer* renderer,
    TTF_Font* font,
    std::deque<std::string>& lines,
    const std::string& line,
    SDL_Rect& position,
    int maxLineLength,
    int maxLines
) {
    std::string::size_type start = 0;

    while (start < line.size()) {
        std::string::size_type end = start + maxLineLength <= line.size()
            ? line.rfind(' ', start + maxLineLength)
            : line.size();

        // If no whitespace was found, just split at the max length
        if (end == std::string::npos || end <= start) {
            end = start + maxLineLength;
        }

        lines.push_back(line.substr(start, end - start));

        // If we have more than 8 lines, remove the oldest one
        if (lines.size() > maxLines) {
            lines.pop_front();
        }

        start = end != line.size() ? end + 1 : end;
    }
}

void PlayerTextRenderSystem::run(SDL_Renderer* renderer) {
    auto& playerTextComponent = scene->player->get<PlayerTextComponent>();

    if (playerTextComponent.text.empty()) {
        return;  // text has 0 length
    }

    SDL_Rect position = {playerTextComponent.x * SCALE, playerTextComponent.y * SCALE, 0, 0};

    std::deque<std::string> lines;

    std::string::size_type start = 0;
    std::string::size_type end = playerTextComponent.text.find('\n');

    while (end != std::string::npos) {
        std::string line = playerTextComponent.text.substr(start, end - start);
        handleLine(
            renderer,
            playerTextComponent.font,
            lines,
            line,
            position,
            playerTextComponent.maxLineLength,
            playerTextComponent.maxLines
        );

        start = end + 1;
        end = playerTextComponent.text.find('\n', start);
    }

    // Handle the last line (or only line if there are no line breaks)
    std::string line = playerTextComponent.text.substr(start);
    handleLine(
        renderer,
        playerTextComponent.font,
        lines,
        line,
        position,
        playerTextComponent.maxLineLength,
        playerTextComponent.maxLines
    );

    // Now render the lines
    for (const auto& line : lines) {
        renderLine(renderer, playerTextComponent.font, line, position, playerTextComponent.color);
    }

    int text_width;
    int text_height;
    TTF_SizeText(playerTextComponent.font, lines.back().c_str(), &text_width, &text_height);
    playerTextComponent.lastLineRect.x = position.x;
    playerTextComponent.lastLineRect.y = position.y;
    playerTextComponent.lastLineRect.w = text_width;
    playerTextComponent.lastLineRect.h = text_height;
}

void PlayerCursorRenderSystem::run(SDL_Renderer* renderer) {
    bool blink = (SDL_GetTicks() / 500) % 2 == 0;

    if (!blink) {
        return;
    }

    const auto playerTextComponent = scene->player->get<PlayerTextComponent>();
    
    SDL_Rect r = {
        playerTextComponent.lastLineRect.x + playerTextComponent.lastLineRect.w + (1 * SCALE),
        playerTextComponent.lastLineRect.y - (1 * SCALE),
        3 * SCALE,
        1 * SCALE,
    };

    if (r.x / SCALE > 150) {
        r.x = 10 * SCALE;
        r.y += playerTextComponent.lastLineRect.h;
    }

    if (r.x > 0 && r.y > 0 && r.w > 0 && r.h > 0) {
        SDL_SetRenderDrawColor(renderer, 226, 246, 228, 255);
        SDL_RenderFillRect(renderer, &r);
    }
}

TextCrawlUpdateSystem::TextCrawlUpdateSystem(const std::string& text, int lettersPerSecond)
    : text(text), frameCount(0) {
    framesPerLetter = 60 / lettersPerSecond;
}

void TextCrawlUpdateSystem::run(double dT) {
    auto& playerTextComponent = scene->player->get<PlayerTextComponent>();

    if (playerTextComponent.text.size() < text.size() && ++frameCount >= framesPerLetter) {
      if (playerTextComponent.text.size() < text.size()) {
        playerTextComponent.text += text[playerTextComponent.text.size()];
      }
      frameCount = 0;
    }
}

TextCrawlEventSystem::TextCrawlEventSystem(const std::string& text, std::function<void()> changeScene)
    : text(text), changeScene(changeScene) { }

void TextCrawlEventSystem::run(SDL_Event event) {
    auto& playerTextComponent = scene->player->get<PlayerTextComponent>();

    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
        if (playerTextComponent.text.size() < text.size()) {
            playerTextComponent.text = text;
        } else {
            print("next scene!");
            changeScene();
        }
    }
}
