#pragma once

#include <string>
#include <vector>
#include <sstream>
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
  gpt_params params;
  llama_model* model;
  llama_context* ctx;
  std::vector<llama_token> embd;
  std::vector<llama_token> last_tokens;
  std::vector<llama_token> tokens_list;
  std::vector<llama_token_data> candidates;
  std::string initialPrompt;
  int max_context_size;

  std::vector<int> input_tokens;
  std::vector<int> output_tokens;
  std::ostringstream output_ss;

  int n_remain;
  int n_past;
  int n_consumed;

  bool is_antiprompt;
  bool input_echo;

  bool evaluateTokensInBatches();
  void addTokensToProcess();
  void processTokens();
  void checkForAntiPrompt();
  void handleEOS();
};

