#pragma once

#include <string>
#include <vector>
#include "llama.h"
#include "common/common.h"

#include "./Base.h"

class Llama : public Base {
public:
  Llama(
    const std::string& username,
    const std::string& ainame,
    const std::string& modelFile = "",
    const std::string& promptFile = ""
  );
  ~Llama();
  void initialize() override;
  void process(const std::string& prompt) override;
  void sample() override;
  void retrain(const std::string& promptFile = "") override;

private:
  std::string initialPrompt;
  gpt_params params;
  llama_model* model;
  llama_context* ctx;
  llama_context* ctx_guidance;
  std::vector<llama_chat_msg> chat_msgs;

  int n_remain;
  int n_past;
  int n_consumed;

  bool is_antiprompt;
  bool input_echo;
};

