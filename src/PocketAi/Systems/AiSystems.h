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

class AiPromptPostProcessingSystem : public UpdateSystem {
public:
  void run(double dT);
};

class AiEmotionProcessingSystem : public UpdateSystem {
public:
  void run(double dT);
};


