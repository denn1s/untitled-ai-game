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
  llama_sampling_params sparams;
  llama_model* model;
  llama_context* ctx;
  llama_context* ctx_guidance;
  struct llama_sampling_context* ctx_sampling;
  std::vector<llama_chat_msg> chat_msgs;
  std::vector<llama_token> embd_inp;
  std::vector<llama_token> embd;
  std::vector<llama_token> embd_guidance;
  std::vector<std::vector<llama_token>> antiprompt_ids;
  std::vector<llama_token> guidance_inp;

  std::vector<int> input_tokens;
  std::vector<int> output_tokens;
  std::ostringstream output_ss;
  std::ostringstream assistant_ss;

  int n_remain;
  int n_past;
  int n_consumed;
  int n_past_guidance;
  int n_ctx;
  int n_ctx_train;

  int ga_i;
  int ga_n;
  int ga_w;
  int guidance_offset;
  int original_prompt_len;

  bool is_antiprompt;
  bool input_echo;
  bool is_interacting;
  bool add_bos;
  bool display;

  void contextRotation();
  bool evaluateTokensInBatches();
  void addTokensToProcess();
  void processTokens();
  void handleEOT();
};

