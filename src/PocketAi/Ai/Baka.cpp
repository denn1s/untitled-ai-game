#include "Baka.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <regex>
#include <print.h>
#include "AiManager.h"

Baka::Baka(
  const std::string& username,
  const std::string& ainame,
  const std::string& modelFile,
  const std::string& promptFile
) {
  this->username = username;
  this->ainame = ainame;
  this->initialPrompt = promptFile;
  
  isInitialized = false;
}

Baka::~Baka() {}

void Baka::initialize() {
  loadPrompts(this->initialPrompt);
  std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // simulate delay
  isInitialized = true;
}

void Baka::process(const std::string& prompt) {
  print("Got user input: ", prompt);
  // we don't do anything with the user prompt yet
}

void Baka::sample() {
  if (!prompts.empty()) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, prompts.size() - 1);

    int random_index = dist(mt);
    std::string prompt = prompts[random_index] + "\n" + username;
    prompt = std::regex_replace(prompt, std::regex("\\$\\{USERNAME\\}"), username.substr(0, username.size() - 2));
    prompt = std::regex_replace(prompt, std::regex("\\$\\{AINAME\\}"), ainame.substr(0, ainame.size() - 2));

    for (size_t i = 0; i < prompt.size(); i+=3) {
      std::string substring = prompt.substr(i, 3);
      AiManager::responseQueue.push(substring);
      std::this_thread::sleep_for(std::chrono::milliseconds(300)); // simulate delay
    }
  }
}

void Baka::retrain(const std::string& promptFile) {
  loadPrompts(promptFile);
  std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // simulate delay
}

void Baka::loadPrompts(std::string promptFile) {
  std::ifstream file("assets/Prompts/" + promptFile);
  if (file.is_open()) {
    std::string line;
    while (getline(file, line)) {
      prompts.push_back(line);
    }
    file.close();
  }
}

