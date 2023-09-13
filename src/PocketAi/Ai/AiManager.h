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
  static void setUp(std::string userLabel, std::string aiLabel, std::string promptFile, std::string modelFile = "");
  static void run();
  static void tearDown();
private:
  static Base* model;
  static tbb::task_group taskGroup;
};
