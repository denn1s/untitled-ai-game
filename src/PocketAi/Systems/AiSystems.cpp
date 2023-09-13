#include "AiSystems.h"

#include <print.h>

#include "PocketAi/Components.h"
#include "PocketAi/Ai/AiManager.h"

AiSetupSystem::~AiSetupSystem() {
  AiManager::tearDown();
}

void AiSetupSystem::run() {
  AiManager::setUp("Rob: ", "Pocket: ", "baka-low.txt");
}

void AiPromptProcessingSystem::run(double dT) {
  AiManager::run();
}


