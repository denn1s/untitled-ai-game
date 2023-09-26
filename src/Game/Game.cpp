#include "Game.h"

#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <print.h>

Game::Game(const char* title, int width, int height)
{
  int maxFPS = 60;
  frameDuration = (1.0f / maxFPS) * 1000.0f;  // how many mili seconds in one frame

  // initial frame count variables
  frameCount = 0;
  lastFPSUpdateTime = 0;
  FPS = 0;

  SDL_Init(SDL_INIT_EVERYTHING);
  
  window = SDL_CreateWindow(title, 0, 0, width, height, 0);
  renderer = SDL_CreateRenderer(window, -1, 0);
  
  SDL_SetRenderDrawColor(renderer, 200, 255, 255, 1);

  // Initialize TTF
  if (TTF_Init() == -1) {
    printf("TTF Init Failed: %s\n", TTF_GetError());
    exit(1);
  }

  print("Game Start!");

  screen_width = width;
  screen_height = height;

  isRunning = true;

  frameStartTimestamp = 0;
  frameEndTimestamp = 0;

  currentScene = nullptr;
}

Game::~Game()
{}

void Game::frameStart()
{
  /* std::cout << "---- Frame: " << frameCount << " ----" << std::endl; */
  frameStartTimestamp = SDL_GetTicks();
  if (frameEndTimestamp)
  {
    dT = (frameStartTimestamp - frameEndTimestamp) / 1000.0f;
  }
  else
  {
    dT = 0;
  }
}

void Game::frameEnd()
{
  frameEndTimestamp = SDL_GetTicks();

  float actualFrameDuration = frameEndTimestamp - frameStartTimestamp;

  if (actualFrameDuration < frameDuration)
  {
    SDL_Delay(frameDuration - actualFrameDuration);
  }
  
  frameCount++;
  // Update FPS counter every second
  Uint32 currentTime = SDL_GetTicks();
  if (currentTime - lastFPSUpdateTime > 1000) // 1000 milliseconds in 1 second
  {
    FPS = frameCount / ((currentTime - lastFPSUpdateTime) / 1000.0f);
    lastFPSUpdateTime = currentTime;
    if (FPS > 0) {
      std::ostringstream titleStream;
      titleStream << " FPS: " << FPS; 
      SDL_SetWindowTitle(window, titleStream.str().c_str());
    }
    frameCount = 0; // Reset frame count after updating FPS
  }
}

void Game::handleEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0)
  {
    if (event.type == SDL_QUIT)
    {
      isRunning = false;
    }

    currentScene->processEvents(event);
  }
}

void Game::update()
{
  currentScene->update(dT);
}

void Game::render()
{
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
  SDL_RenderClear(renderer);
  
  currentScene->render(renderer);

  SDL_RenderPresent(renderer);
  /* vprint(FPS); */
}

void Game::clean()
{
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  print("Game Over.");
}

bool Game::running()
{
  return isRunning;
}

void Game::run()
{
  if (currentScene == nullptr) {
    print("No scene was initialized. There's nothing to do.");
    exit(1);
  }

    while (running())
    {
      frameStart();
      handleEvents();
      update();
      render();
      frameEnd();
    }

    clean();
}

void Game::setScene(Scene* newScene) {
  newScene->setup();

  // todo: destroy previous scene?

  currentScene = newScene;

  // todo: move this somewhere else?
}

Scene* Game::getCurrentScene() const {
  return currentScene;
}
