#include "AiSystems.h"

#include <print.h>

#include "ECS/Components.h"
#include "PocketAi/Components.h"
#include "PocketAi/Ai/AiManager.h"

AiSetupSystem::~AiSetupSystem() {
  AiManager::tearDown();
}

void AiSetupSystem::run() {
  AiManager::setUp( "Rob:", "Pocket:", "initial.txt", "model" );
  /* AiManager::setUp( "Rob:", "Pocket:", "baka-none.txt"); */
}

void AiPromptProcessingSystem::run(double dT) {
  AiManager::run();
}

void AiPromptPostProcessingSystem::run(double dT) {
  std::string output;
  if (AiManager::responseQueue.try_pop(output)) {
    auto& textComponent = scene->player->get<PlayerTextComponent>();
    auto& promptComponent = scene->player->get<PlayerPromptComponent>();
    auto& emotionComponent = scene->player->get<PlayerEmotionComponent>();

    if (emotionComponent.isProcessingEmotion) {
      if (output == " ") {  // we are at the end of an emotion
        emotionComponent.isProcessingEmotion = false;
      } else {
        emotionComponent.emotion += output;
      }
    } else {
      if (output == "/") {
        emotionComponent.isProcessingEmotion = true;
      } else {
        textComponent.text += output;

        // we check if there is an antiprompt at the end of the prompt.
        if (AiManager::endsWithAntiPrompt(textComponent.text)) {
          promptComponent.isInteracting = true; 
          promptComponent.currentPrompt = textComponent.text;
        }
      }
    }
  }
}

std::unordered_map<std::string, int> emotionMap {
  {"neutral", 0},
  {"happy", 1},
  {"playful", 2},
  {"confused", 3},
  {"disgusted", 4},
  {"angry", 5}
};

void AiEmotionProcessingSystem::run(double dT) {
  auto& emotionComponent = scene->player->get<PlayerEmotionComponent>();
  auto& playerSpriteComponent = scene->player->get<SpriteComponent>();

  print("emotion", emotionComponent.emotion);

  int value = (emotionMap.find(emotionComponent.emotion) != emotionMap.end()) ? emotionMap[emotionComponent.emotion] : -1;

  if (value != -1) {
    playerSpriteComponent.xIndex = value;
    playerSpriteComponent.yIndex = rand() % 5 < 2 ? 0 : rand() % 5 < 4 ? 1 : 2;
    emotionComponent.emotion = "";
  }
}

