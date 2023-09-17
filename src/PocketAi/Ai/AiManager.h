#pragma once
#include "Base.h"
#include "Baka.h"
#include "Llama.h"

#include <oneapi/tbb/task_group.h>
#include <string>
#include <tbb/concurrent_queue.h>
#include <tbb/task_group.h>

enum Smarts {
  BAKA,
  LLAMA
};

class AiManager {
public:
  static tbb::concurrent_queue<std::string> requestQueue;
  static tbb::concurrent_queue<std::string> responseQueue;
  static void setUp(
    const std::string& userLabel,
    const std::string& aiLabel,
    const std::string& promptFile,
    const std::string& modelFile = ""
  );
  static void run();
  static void tearDown();
  static bool endsWithAntiPrompt(const std::string& str);
private:
  static std::string antiprompt; 
  static Base* model;
  static tbb::task_group taskGroup;
};
