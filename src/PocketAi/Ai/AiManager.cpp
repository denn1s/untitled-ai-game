#include "AiManager.h"

#include <string>
#include <print.h>
#include <tbb/task_group.h>

tbb::concurrent_queue<std::string> AiManager::requestQueue;
tbb::concurrent_queue<std::string> AiManager::responseQueue;
Base* AiManager::model = nullptr;
tbb::task_group AiManager::taskGroup;
std::string AiManager::antiprompt;

void AiManager::setUp(
  const std::string& userLabel,
  const std::string& aiLabel,
  const std::string& promptFile,
  const std::string& modelFile
) {
  Smarts smarts = BAKA;
  if (!modelFile.empty()) {
    smarts = LLAMA;
  }
  switch(smarts) {
    case BAKA:
      model = new Baka(userLabel, aiLabel, modelFile, promptFile);
      print("baka");
      break;
    case LLAMA:
      model = new Llama(userLabel, aiLabel, modelFile, promptFile);
      print("llama");
      break;
  }
  antiprompt = userLabel + " ";

  taskGroup.run([] {
    // this is extremely slow, minutes even
    model->initialize();

    // we sample once right at the start so the conversation opens by the ai
    model->sample();
  });
}

void AiManager::run() {
  if (model->isInitialized) {
    std::string prompt;
    if (requestQueue.try_pop(prompt)) {
      taskGroup.run([prompt] { 
        // we process user input first, this consumes one item 
        model->process(prompt); 

        // we take one sample, this is kinda slow
        model->sample();
      });
    }
  }
}

void AiManager::tearDown() {
  taskGroup.wait();
}

bool AiManager::endsWithAntiPrompt(const std::string& str) {
  if (str.length() >= antiprompt.length()) {
    return (0 == str.compare(str.length() - antiprompt.length(), antiprompt.length(), antiprompt));
  } else {
    return false;
  }
}

