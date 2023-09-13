#pragma once

#include "ECS/System.h"

class AiSetupSystem : public SetupSystem {
public:
  ~AiSetupSystem();
  void run();
};

class AiPromptProcessingSystem : public UpdateSystem {
public:
  void run(double dT);
};



