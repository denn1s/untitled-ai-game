#include "Llama.h"
#include "log.h"
#include "build-info.h"
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
  log_disable();

  isInitialized = false;
  params.model = "assets/Models/" + modelFile;
  params.n_ctx = 4096;
  params.n_predict = 4096;
  params.n_batch = 1024;
  params.n_threads = 14;
  params.n_keep = -1;
  params.repeat_last_n = 256;
  params.repeat_penalty = 1.17647f;
  params.temp = 0.6f;
  params.mirostat = 2;
  params.input_prefix = " ";
  params.input_suffix = ainame;
  params.antiprompt.push_back(username);
  params.interactive = true;

  n_remain = params.n_predict;
  n_past = 0;
  n_consumed = 0;

  is_antiprompt = false;
  is_interacting = false;
  input_echo = true;

  initialPrompt = promptFile;
}

Llama::~Llama() {
  llama_free(ctx);
  llama_free_model(model);
  llama_backend_free();
}

void Llama::initialize() {
  std::string prompt = readFromFile("assets/Prompts/" + initialPrompt);
  prompt = std::regex_replace(prompt, std::regex("\\$\\{USERNAME\\}"), username.substr(0, username.size() - 2));
  prompt = std::regex_replace(prompt, std::regex("\\$\\{AINAME\\}"), ainame.substr(0, ainame.size() - 2));
  params.prompt = prompt;

  llama_backend_init(params.numa);
  std::tie(model, ctx) = llama_init_from_gpt_params(params);

  if (model == NULL) {
    fprintf(stderr , "%s: error: unable to load model\n" , __func__);
    exit(1);
  }

  const int n_ctx_train = llama_n_ctx_train(ctx);
  if (params.n_ctx > n_ctx_train) {
    fprintf(stderr, "%s: warning: model was trained on only %d context tokens (%d specified)\n",
            __func__, n_ctx_train, params.n_ctx);
  }

  // print system information
  {
    fprintf(stderr, "\n");
    fprintf(stderr, "ctx_size: %i\n", params.n_ctx);
    fprintf(stderr, "ctx_train: %i\n", n_ctx_train);
    fprintf(stderr, "system_info: n_threads = %d / %d | %s\n",
            params.n_threads, std::thread::hardware_concurrency(), llama_print_system_info());
  }

  const bool add_bos = llama_vocab_type(ctx) == LLAMA_VOCAB_TYPE_SPM;
  tokens_list = ::llama_tokenize(ctx, params.prompt, true);
  max_context_size = llama_n_ctx(ctx);

  if ((int) tokens_list.size() > max_context_size - 4) {
    fprintf(stderr, "%s: error: prompt too long (%d tokens, max %d)\n", __func__, (int) tokens_list.size(), max_context_size - 4);
    exit(1);
  }

  if (params.n_keep < 0 || params.n_keep > (int)tokens_list.size()) {
    params.n_keep = (int)tokens_list.size();
  }

  last_tokens.resize(max_context_size);
  print(last_tokens.size());
  std::fill(last_tokens.begin(), last_tokens.end(), 0);
  const int n_vocab = llama_n_vocab(ctx);
  candidates.reserve(n_vocab);

  isInitialized = true;
}

void Llama::process(const std::string& prompt) {
  if (n_past > 0 && is_interacting) {
    fprintf(stderr, "waiting for user input\n");
    exit(1);

    if (params.input_prefix_bos) {
      fprintf(stderr, "adding input prefix BOS token\n");
      tokens_list.push_back(llama_token_bos(ctx));
    }

    std::string buffer;
    if (!params.input_prefix.empty()) {
      fprintf(stderr, "appending input prefix: '%s'\n", params.input_prefix.c_str());
      buffer += params.input_prefix;
      fprintf(stderr, "buffer after input prefix %s", buffer.c_str());
    }

    std::string line;
    bool another_line = true;
    do {
      another_line = "Can you read me";
      buffer += line;
    } while (another_line);

    // Add tokens to embd only if the input buffer is non-empty
    // Entering a empty line lets the user pass control back
    if (buffer.length() > 1) {
      // append input suffix if any
      if (!params.input_suffix.empty()) {
        fprintf(stderr, "appending input suffix: '%s'\n", params.input_suffix.c_str());
        buffer += params.input_suffix;
        fprintf(stderr, "input_suffix: %s", params.input_suffix.c_str());
      }

      fprintf(stderr, "buffer: '%s'\n", buffer.c_str());

      const size_t original_size = tokens_list.size();

      const auto line_inp = ::llama_tokenize(ctx, buffer, false);
      tokens_list.insert(tokens_list.end(), line_inp.begin(), line_inp.end());

      for (size_t i = original_size; i < tokens_list.size(); ++i) {
        const llama_token token = tokens_list[i];
        output_tokens.push_back(token);
        output_ss << llama_token_to_piece(ctx, token);
      }

      n_remain -= line_inp.size();
      fprintf(stderr, "n_remain: %d\n", n_remain);
    } else {
      fprintf(stderr, "empty line, passing control back\n");
    }

    input_echo = false; // do not echo this again
  }
  if (n_past > 0) {
    is_interacting = false;
  }
}

void Llama::sample() {
  // predict
  if (!embd.empty()) {
    evaluateTokensInBatches();
  }

  addTokensToProcess();
  processTokens();

  // if not currently processing queued inputs;
  if ((int) tokens_list.size() <= n_consumed) {
    checkForAntiPrompt();
    handleEOS();
  }

  // In interactive mode, respect the maximum number of tokens and drop back to user input when reached.
  // We skip this logic when n_predict == -1 (infinite) or -2 (stop at context size).
  if (params.interactive && n_remain <= 0 && params.n_predict >= 0) {
    n_remain = params.n_predict;
    is_interacting = true;
  }
}

void Llama::retrain(const std::string& promptFile) {
  // TODO
}

// PRIVATE METHODS

bool Llama::evaluateTokensInBatches() {
  fprintf(stderr, "Evaluating %d tokens\n", (int)embd.size());
  auto start = std::chrono::high_resolution_clock::now();
  const int max_embd_size = max_context_size - 4;

  if ((int) embd.size() > max_embd_size) {
    const int skipped_tokens = (int) embd.size() - max_embd_size;
    embd.resize(max_context_size - 4);

    fprintf(stderr, "<<input too long: skipped %d token%s>>", skipped_tokens, skipped_tokens != 1 ? "s" : "");
    return false;
  }

  if (n_past + (int) embd.size() > max_context_size) {
    const int n_left = n_past - params.n_keep;
    fprintf(stderr, "context full, swapping: n_past = %d, n_left = %d, n_ctx = %d, n_keep = %d\n", n_past, n_left, max_context_size, params.n_keep);
    n_past = std::max(1, params.n_keep);
    embd.insert(embd.begin(), last_tokens.begin() + max_context_size - n_left/2 - embd.size(), last_tokens.end() - embd.size());
  }

  for (int i = 0; i < (int) embd.size(); i += params.n_batch) {
    int n_eval = (int) embd.size() - i;
    if (n_eval > params.n_batch) {
      n_eval = params.n_batch;
    }

    if (llama_eval(ctx, &embd[i], n_eval, n_past, params.n_threads)) {
      fprintf(stderr, "%s : failed to eval\n", __func__);
      return false;
    }

    n_past += n_eval;
  }

  embd.clear();
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end-start;
  fprintf(stderr, "Processed in %f seconds\n", diff.count());
  return true;
}

void Llama::addTokensToProcess() {
  if ((int) tokens_list.size() <= n_consumed && !is_interacting) {
    const llama_token id = llama_sample_token(ctx, NULL, NULL, params, last_tokens, candidates);

    last_tokens.erase(last_tokens.begin());
    last_tokens.push_back(id);
    embd.push_back(id);

    // echo this to console
    input_echo = true;

    // decrement remaining sampling budget
    --n_remain;
  } else {
    // some user input remains from prompt or interaction, forward it to processing
    while ((int) tokens_list.size() > n_consumed) {
      embd.push_back(tokens_list[n_consumed]);
      last_tokens.erase(last_tokens.begin());
      last_tokens.push_back(tokens_list[n_consumed]);
      ++n_consumed;
      if ((int) embd.size() >= params.n_batch) {
        break;
      }
    }
    fprintf(stderr, "Added %d tokens for processing\n", n_consumed);
  }
}

void Llama::processTokens() {
  if (input_echo) {
    for (auto id : embd) {
      const std::string token_str = llama_token_to_piece(ctx, id);

      if (embd.size() > 1) {
        input_tokens.push_back(id);
        fprintf(stderr, "<<< %s \n", token_str.c_str());
      } else {
        output_tokens.push_back(id);
        output_ss << token_str;
        fprintf(stderr, ">>> %s \n", token_str.c_str());
      }
    }
  }
}

void Llama::checkForAntiPrompt() {
  // check for reverse prompt
  if (!params.antiprompt.empty()) {
    std::string last_output;
    for (auto id : last_tokens) {
      last_output += llama_token_to_piece(ctx, id);
    }

    is_antiprompt = false;
    // Check if each of the reverse prompts appears at the end of the output.
    // If we're not running interactively, the reverse prompt might be tokenized with some following characters
    // so we'll compensate for that by widening the search window a bit.
    for (std::string & antiprompt : params.antiprompt) {
      size_t search_start_pos = last_output.length() > static_cast<size_t>(antiprompt.length())
        ? last_output.length() - static_cast<size_t>(antiprompt.length())
        : 0;

      if (last_output.find(antiprompt, search_start_pos) != std::string::npos) {
        if (params.interactive) {
          is_interacting = true;
        }
        is_antiprompt = true;
        print("we found anti prompt, now we need to wait for user input");
        break;
      }
    }
  }
}

void Llama::handleEOS() {
  if (last_tokens.back() == llama_token_eos(ctx)) {
    fprintf(stderr, "found EOS token\n");

    if (params.interactive) {
      if (!params.antiprompt.empty()) {
        // tokenize and inject first reverse prompt
        const auto first_antiprompt = ::llama_tokenize(ctx, params.antiprompt.front(), false);
        tokens_list.insert(tokens_list.end(), first_antiprompt.begin(), first_antiprompt.end());
        is_antiprompt = true;
      }
      is_interacting = true;
    }
  }
}

