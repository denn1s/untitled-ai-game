#include "AiSystems.h"

#include <algorithm>
#include <random>
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

  int value = (emotionMap.find(emotionComponent.emotion) != emotionMap.end()) ? emotionMap[emotionComponent.emotion] : -1;

  if (value != -1) {
    playerSpriteComponent.xIndex = value;
    
    std::mt19937 mt{static_cast<std::mt19937::result_type>(dT)};
    std::uniform_real_distribution<double> dist(0, 1);
    double random_number = dist(mt);
    playerSpriteComponent.yIndex = random_number < 0.2 ? 1 : random_number < 0.6 ? 0 : 2;
    
    vprint(emotionComponent.emotion);
    emotionComponent.emotion = "";

    if (value == 0) {
      emotionComponent.affection += 5;
    } else if (value == 1) {
      emotionComponent.affection += 7;
    } else if (value == 2) {
      emotionComponent.affection += 10;
    } else if (value == 3) {
      emotionComponent.affection -= 5;
    } else if (value == 4) {
      emotionComponent.affection -= 10;
    } else if (value == 5) {
      emotionComponent.affection -= 15;
    }

    emotionComponent.affection = std::clamp(emotionComponent.affection, 0, 99);

    vprint(emotionComponent.affection);
  }
}

