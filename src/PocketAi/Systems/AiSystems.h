#pragma once

#include "ECS/System.h"

#include <functional>

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

class AiConversationProgressSystem : public UpdateSystem {
public:
  AiConversationProgressSystem(std::function<void()> changeScene, int day);
  void run(double dT);
private:
  std::function<void()> changeScene; 
  int day;
};

class AiConfessionRequestSetupSystem : public SetupSystem {
public:
  void run();
};

class AiEndingSetupSystem : public SetupSystem {
public:
  void run();
};

