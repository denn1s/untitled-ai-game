#include "Llama.h"
#include "PocketAi/Ai/AiManager.h"
#include "log.h"
#include <SDL_timer.h>
#include <print.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <regex>
#include <format>
#include <chrono>

std::string readFromFile(const std::string& filename) {
  std::ifstream inFile(filename);
  std::stringstream buffer;
  buffer << inFile.rdbuf();
  return buffer.str();
}

Llama::Llama(
  const std::string& username,
  const std::string& ainame,
  const std::string& modelFile,
  const std::string& promptFile
) {
  this->username = username;
  this->ainame = ainame;
  initialPrompt = promptFile;

  params.model = "assets/Models/" + modelFile;
  params.n_ctx = 4096;
  params.n_predict = 128;
  params.n_batch = 1024;
  params.n_threads = 14;
  params.n_keep = -1;
  params.input_prefix = " ";
  params.input_suffix = ainame;
  params.antiprompt.push_back(username);
  params.interactive = true;

  n_remain = params.n_predict;
  n_past = 0;
  n_consumed = 0;

  is_antiprompt = false;
  input_echo = true;


  llama_sampling_params & sparams = params.sparams;

  if (params.logits_all) {
    printf("\n************\n");
    printf("%s: please use the 'perplexity' tool for perplexity calculations\n", __func__);
    printf("************\n\n");

    exit(0);
  }

  if (params.embedding) {
    printf("\n************\n");
    printf("%s: please use the 'embedding' tool for embedding calculations\n", __func__);
    printf("************\n\n");

    exit(0);
  }

  if (params.n_ctx != 0 && params.n_ctx < 8) {
    LOG_TEE("%s: warning: minimum context size is 8, using minimum size.\n", __func__);
    params.n_ctx = 8;
  }

  if (params.rope_freq_base != 0.0) {
    LOG_TEE("%s: warning: changing RoPE frequency base to %g.\n", __func__, params.rope_freq_base);
  }

  if (params.rope_freq_scale != 0.0) {
    LOG_TEE("%s: warning: scaling RoPE frequency by %g.\n", __func__, params.rope_freq_scale);
  }

  LOG_TEE("%s: build = %d (%s)\n",      __func__, LLAMA_BUILD_NUMBER, LLAMA_COMMIT);
  LOG_TEE("%s: built with %s for %s\n", __func__, LLAMA_COMPILER, LLAMA_BUILD_TARGET);

  if (params.seed == LLAMA_DEFAULT_SEED) {
    params.seed = time(NULL);
  }

  LOG_TEE("%s: seed  = %u\n", __func__, params.seed);

  std::mt19937 rng(params.seed);

  LOG("%s: llama backend init\n", __func__);
  llama_backend_init();
  llama_numa_init(params.numa);
  LOG("%s: load the model and apply lora adapter, if any\n", __func__);
  std::tie(model, ctx) = llama_init_from_gpt_params(params);

  if (sparams.cfg_scale > 1.f) {
    struct llama_context_params lparams = llama_context_params_from_gpt_params(params);
    ctx_guidance = llama_new_context_with_model(model, lparams);
  }

  if (model == NULL) {
    LOG_TEE("%s: error: unable to load model\n", __func__);
    exit(1);
  }

  const int n_ctx_train = llama_n_ctx_train(model);
  const int n_ctx = llama_n_ctx(ctx);
  LOG("n_ctx: %d\n", n_ctx);

  if (n_ctx > n_ctx_train) {
      LOG_TEE("%s: warning: model was trained on only %d context tokens (%d specified)\n",
              __func__, n_ctx_train, n_ctx);
  }

  LOG_TEE("%s: chat template example: %s\n", __func__, llama_chat_format_example(model, params.chat_template).c_str());

  // print system information
  {
      LOG_TEE("\n");
      LOG_TEE("%s\n", gpt_params_get_system_info(params).c_str());
  }
}

Llama::~Llama() {
  llama_free(ctx);
  llama_free_model(model);
  llama_backend_free();
}

void Llama::initialize() {
  std::string prompt = readFromFile("assets/Prompts/" + initialPrompt);
  prompt = std::regex_replace(prompt, std::regex("\\$\\{USERNAME\\}"), username.substr(0, username.size() - 1));
  prompt = std::regex_replace(prompt, std::regex("\\$\\{AINAME\\}"), ainame.substr(0, ainame.size() - 1));
  params.prompt = prompt;




  isInitialized = true;
}

void Llama::process(const std::string& prompt) {
  std::cout << "prompt: " << prompt << std::endl;
}

void Llama::sample() {
  std::cout << "sample: " << std::endl;
}

void Llama::retrain(const std::string& promptFile) {
  std::string prompt = readFromFile("assets/Prompts/" + promptFile);
  prompt = std::regex_replace(prompt, std::regex("\\$\\{USERNAME\\}"), username.substr(0, username.size() - 1));
  prompt = std::regex_replace(prompt, std::regex("\\$\\{AINAME\\}"), ainame.substr(0, ainame.size() - 1));
  n_remain = params.n_predict;
  /* n_past = 0; */
  /* n_consumed = 0; */

  is_antiprompt = false;
  input_echo = true;

  print("This is the new prompt '", prompt, "'");
  process(prompt);
}

