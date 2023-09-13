#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <random>
#include "Base.h"

class Baka : public Base {
public:
  Baka(
    const std::string& username,
    const std::string& ainame,
    const std::string& modelFile = "",
    const std::string& promptFile = ""
  );
  ~Baka();
  void initialize() override;
  void process(const std::string& prompt) override;
  void sample() override;
  void retrain(const std::string& promptFile) override;

private:
  std::string initialPrompt;
  std::vector<std::string> prompts;
  void loadPrompts(std::string promptFile);
};

