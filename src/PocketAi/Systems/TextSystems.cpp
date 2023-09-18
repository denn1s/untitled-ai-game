#include "TextSystems.h"

#include <SDL_render.h>
#include <SDL_timer.h>
#include <print.h>
#include <constants.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <deque>

#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "Game/Graphics/TextureManager.h"

#include "PocketAi/Components.h"
#include "PocketAi/Ai/AiManager.h"

void PlayerTextSetupSystem::run() {
    short f = 5 * SCALE;

    TTF_Font* font = TTF_OpenFont("assets/Fonts/GamergirlClassic.ttf", f);
    if (!font) {
        print("Failed to load font: %s\n", TTF_GetError());
        exit(1);
    }

    auto& p = scene->player->get<PlayerTextComponent>();
    p.font = font;
    p.fontSize = f;
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
}

void renderLine(SDL_Renderer* renderer, TTF_Font* font, const std::string& line, SDL_Rect& position) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, line.c_str(), {226, 246, 228});
    
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
    SDL_Rect& position
) {
    const int maxLineLength = 28;
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
        if (lines.size() > 7) {
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

    SDL_Rect position = {10 * SCALE, 100 * SCALE, 0, 0};

    std::deque<std::string> lines;

    std::string::size_type start = 0;
    std::string::size_type end = playerTextComponent.text.find('\n');

    while (end != std::string::npos) {
        std::string line = playerTextComponent.text.substr(start, end - start);
        handleLine(renderer, playerTextComponent.font, lines, line, position);

        start = end + 1;
        end = playerTextComponent.text.find('\n', start);
    }

    // Handle the last line (or only line if there are no line breaks)
    std::string line = playerTextComponent.text.substr(start);
    handleLine(renderer, playerTextComponent.font, lines, line, position);

    // Now render the lines
    for (const auto& line : lines) {
        renderLine(renderer, playerTextComponent.font, line, position);
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
 
