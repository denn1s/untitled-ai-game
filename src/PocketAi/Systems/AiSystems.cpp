#include "AiSystems.h"

#include <SDL_timer.h>
#include <algorithm>
#include <random>
#include <print.h>
#include <string>

#include "ECS/Components.h"
#include "PocketAi/Components.h"
#include "PocketAi/Ai/AiManager.h"

AiSetupSystem::~AiSetupSystem() {
  /* AiManager::tearDown(); */
}

void AiSetupSystem::run() {
  AiManager::setUp( "Rob:", "Pocket:", "initial.txt", "model" );
  /* AiManager::setUp( "Rob:", "Pocket:", "baka-high.txt"); */
}

void AiPromptProcessingSystem::run(double dT) {
  AiManager::run();
}

void AiPromptPostProcessingSystem::run(double dT) {
  Uint32 now = SDL_GetTicks();  
  auto& conversationComponent = scene->player->get<ConversationComponent>();

  if (conversationComponent.lastLetterTime == 0) {
    conversationComponent.lastLetterTime = now;
  } else if (now - conversationComponent.lastLetterTime < conversationComponent.minLetterTime) {
    // we delay a bit so letters dont show too fast
    return;
  } else {
    conversationComponent.lastLetterTime = now;
  }

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
          conversationComponent.countConversations++;
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
  auto& affection = scene->r.ctx().get<AffectionComponent>().affection;

  int value = (emotionMap.find(emotionComponent.emotion) != emotionMap.end()) ? emotionMap[emotionComponent.emotion] : -1;

  if (value != -1) {
    playerSpriteComponent.xIndex = value;
    
    std::mt19937 mt{static_cast<std::mt19937::result_type>(SDL_GetTicks())};
    std::uniform_int_distribution<int> dist(0, 4);
    int random_number = dist(mt);
    playerSpriteComponent.yIndex = std::clamp(random_number, 1, 3) - 1;
    
    vprint(emotionComponent.emotion);
    emotionComponent.emotion = "";

    if (value == 0) {
      affection += 15;
    } else if (value == 1) {
      affection += 17;
    } else if (value == 2) {
      affection += 20;
    } else if (value == 3) {
      affection -= 5;
    } else if (value == 4) {
      affection -= 10;
    } else if (value == 5) {
      affection -= 15;
    }

    affection = std::clamp(affection, 0, 99);

    vprint(affection);
  }
}

AiConversationProgressSystem::AiConversationProgressSystem(std::function<void()> changeScene, int day)
  : changeScene(changeScene), day(day) { }

void AiConversationProgressSystem::run(double dT) {
  auto& conversationComponent = scene->player->get<ConversationComponent>();
  auto& playerPromptComponent = scene->player->get<PlayerPromptComponent>();
  const auto affection = scene->r.ctx().get<AffectionComponent>().affection;

  if (playerPromptComponent.isInteracting && conversationComponent.countConversations > conversationComponent.maxConversations) {
    /* AiManager::responseQueue.push("(You hear the bells ring)\n"); */
    /* AiManager::responseQueue.push("\nPocket: Anyways. Looks like its time for class. See you later!\n"); */
    playerPromptComponent.isInteracting = false;      
    SDL_Delay(3000);
    changeScene(); // but after we fade out? 
    /* print("we must retrain using", "day" + std::to_string(day + 1) + ".txt"); */
    if (day > 0) {
      AiManager::retrain("day" + std::to_string(day + 1) + ".txt");
    }

/*     if (affection < 20) { */
/*       AiManager::retrain("roll-low-" + std::to_string(day) + ".txt"); */
/*     } else if (affection < 60) { */
/*       AiManager::retrain("roll-mid-" + std::to_string(day) + ".txt"); */
/*     } else { */
/*       AiManager::retrain("roll-hig-" + std::to_string(day) + ".txt"); */
/*     } */
  }
}

void AiConfessionRequestSetupSystem::run() {
    AiManager::retrain("confesion.txt");
}

void AiEndingSetupSystem::run() {
  auto view = scene->r.view<SpriteComponent>();
  const auto affection = scene->r.ctx().get<AffectionComponent>().affection;

  print("affection at ending is", affection);
  int ending = 0;  // 0: bad end, 1: normal end, 2: good end
  if (affection < 20) {
    ending = 2; 
  } else if (affection < 80) {
    ending = 1;
  } else {
    ending = 0;
  }

  for(auto entity : view) {
    auto& spriteComponent = scene->r.get<SpriteComponent>(entity);
    spriteComponent.xIndex = ending;
  }
}



