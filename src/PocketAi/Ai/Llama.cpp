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
  params.multiline_input = false;

  n_remain = params.n_predict;
  n_past = 0;
  n_consumed = 0;

  is_antiprompt = false;
  input_echo = true;
  sparams = params.sparams;

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

  n_ctx_train = llama_n_ctx_train(model);
  n_ctx = llama_n_ctx(ctx);
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

  add_bos = llama_should_add_bos_token(model);
  GGML_ASSERT(llama_add_eos_token(model) != 1);
  LOG("add_bos: %d\n", add_bos);
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

  {
    LOG("Tokenize the initial prompt\n");
    std::cout << "Initial prompt: " << prompt << std::endl;
    embd_inp = ::llama_tokenize(ctx, prompt, true, true);

    LOG("prompt: \"%s\"\n", log_tostr(prompt));
    LOG("tokens: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd_inp).c_str());
  }

  // Should not run without any tokens
  if (embd_inp.empty()) {
    embd_inp.push_back(llama_token_bos(model));
    LOG("embd_inp was considered empty and bos was added: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd_inp).c_str());
  }

  // Tokenize negative prompt
  guidance_offset = 0;
  original_prompt_len = 0;
  if (ctx_guidance) {
    LOG("cfg_negative_prompt: \"%s\"\n", log_tostr(sparams.cfg_negative_prompt));

    guidance_inp = ::llama_tokenize(ctx_guidance, sparams.cfg_negative_prompt, true, true);
    LOG("guidance_inp tokenized: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx_guidance, guidance_inp).c_str());

    std::vector<llama_token> original_inp = ::llama_tokenize(ctx, params.prompt, true, true);
    LOG("original_inp tokenized: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, original_inp).c_str());

    original_prompt_len = original_inp.size();
    guidance_offset = (int)guidance_inp.size() - original_prompt_len;
    LOG("original_prompt_len: %s", log_tostr(original_prompt_len));
    LOG("guidance_offset:     %s", log_tostr(guidance_offset));
  }

  if ((int) embd_inp.size() > n_ctx - 4) {
    LOG_TEE("%s: error: prompt is too long (%d tokens, max %d)\n", __func__, (int) embd_inp.size(), n_ctx - 4);
    exit(1);
  }

  // number of tokens to keep when resetting context
  if (params.n_keep < 0 || params.n_keep > (int) embd_inp.size()) {
    params.n_keep = (int)embd_inp.size();
  } else {
    params.n_keep += add_bos; // always keep the BOS token
  }

  if (params.conversation) {
    params.interactive_first = true;
  }

  // enable interactive mode if interactive start is specified
  if (params.interactive_first) {
    params.interactive = true;
  }

  if (params.verbose_prompt) {
    LOG_TEE("\n");
    LOG_TEE("%s: prompt: '%s'\n", __func__, params.prompt.c_str());
    LOG_TEE("%s: number of tokens in prompt = %zu\n", __func__, embd_inp.size());
    for (int i = 0; i < (int) embd_inp.size(); i++) {
      LOG_TEE("%6d -> '%s'\n", embd_inp[i], llama_token_to_piece(ctx, embd_inp[i]).c_str());
    }

    if (ctx_guidance) {
      LOG_TEE("\n");
      LOG_TEE("%s: negative prompt: '%s'\n", __func__, sparams.cfg_negative_prompt.c_str());
      LOG_TEE("%s: number of tokens in negative prompt = %zu\n", __func__, guidance_inp.size());
      for (int i = 0; i < (int) guidance_inp.size(); i++) {
        LOG_TEE("%6d -> '%s'\n", guidance_inp[i], llama_token_to_piece(ctx, guidance_inp[i]).c_str());
      }
    }

    if (params.n_keep > add_bos) {
      LOG_TEE("%s: static prompt based on n_keep: '", __func__);
      for (int i = 0; i < params.n_keep; i++) {
        LOG_TEE("%s", llama_token_to_piece(ctx, embd_inp[i]).c_str());
      }
      LOG_TEE("'\n");
    }
    LOG_TEE("\n");
  }

  if (params.interactive) {
    LOG_TEE("%s: interactive mode on.\n", __func__);

    if (!params.antiprompt.empty()) {
      for (const auto & antiprompt : params.antiprompt) {
        LOG_TEE("Reverse prompt: '%s'\n", antiprompt.c_str());
        if (params.verbose_prompt) {
          auto tmp = ::llama_tokenize(ctx, antiprompt, false, true);
          for (int i = 0; i < (int) tmp.size(); i++) {
            LOG_TEE("%6d -> '%s'\n", tmp[i], llama_token_to_piece(ctx, tmp[i]).c_str());
          }
        }
      }
    }

    if (params.input_prefix_bos) {
      LOG_TEE("Input prefix with BOS\n");
    }

    if (!params.input_prefix.empty()) {
      LOG_TEE("Input prefix: '%s'\n", params.input_prefix.c_str());
      if (params.verbose_prompt) {
        auto tmp = ::llama_tokenize(ctx, params.input_prefix, true, true);
        for (int i = 0; i < (int) tmp.size(); i++) {
          LOG_TEE("%6d -> '%s'\n", tmp[i], llama_token_to_piece(ctx, tmp[i]).c_str());
        }
      }
    }

    if (!params.input_suffix.empty()) {
      LOG_TEE("Input suffix: '%s'\n", params.input_suffix.c_str());
      if (params.verbose_prompt) {
        auto tmp = ::llama_tokenize(ctx, params.input_suffix, false, true);
        for (int i = 0; i < (int) tmp.size(); i++) {
          LOG_TEE("%6d -> '%s'\n", tmp[i], llama_token_to_piece(ctx, tmp[i]).c_str());
        }
      }
    }
  }
  LOG_TEE("sampling: \n%s\n", llama_sampling_print(sparams).c_str());
  LOG_TEE("sampling order: \n%s\n", llama_sampling_order_print(sparams).c_str());
  LOG_TEE("generate: n_ctx = %d, n_batch = %d, n_predict = %d, n_keep = %d\n", n_ctx, params.n_batch, params.n_predict, params.n_keep);

  // group-attention state
  // number of grouped KV tokens so far (used only if params.grp_attn_n > 1)
  ga_i = 0;
  ga_n = params.grp_attn_n;
  ga_w = params.grp_attn_w;

  if (ga_n != 1) {
    GGML_ASSERT(ga_n > 0 && "grp_attn_n must be positive");                     // NOLINT
    GGML_ASSERT(ga_w % ga_n == 0 && "grp_attn_w must be a multiple of grp_attn_n");     // NOLINT
    //GGML_ASSERT(n_ctx_train % ga_w == 0     && "n_ctx_train must be a multiple of grp_attn_w");    // NOLINT
    //GGML_ASSERT(n_ctx >= n_ctx_train * ga_n && "n_ctx must be at least n_ctx_train * grp_attn_n"); // NOLINT
    LOG_TEE("self-extend: n_ctx_train = %d, grp_attn_n = %d, grp_attn_w = %d\n", n_ctx_train, ga_n, ga_w);
  }
  LOG_TEE("\n\n");

  if (params.interactive) {
    LOG_TEE("== Running in interactive mode. ==\n");
    is_interacting = params.interactive_first;
  }

  is_antiprompt = false;
  input_echo = true;
  display = true;

  n_past = 0;
  n_remain = params.n_predict;
  n_consumed = 0;
  n_past_guidance = 0;

  display = params.display_prompt;
  antiprompt_ids.reserve(params.antiprompt.size());
  for (const std::string & antiprompt : params.antiprompt) {
    antiprompt_ids.emplace_back(::llama_tokenize(ctx, antiprompt, false, true));
  }

  struct llama_sampling_context * ctx_sampling = llama_sampling_init(sparams);
  if (!ctx_sampling) {
    fprintf(stderr, "%s: failed to initialize sampling subsystem\n", __func__);
    exit(1);
  }

  isInitialized = true;
}

void Llama::process(const std::string& prompt) {
  std::cout << "prompt: " << prompt << std::endl;

  if (!embd.empty()) {
    int max_embd_size = n_ctx - 4;

    // Ensure the input doesn't exceed the context size by truncating embd if necessary.
    if ((int) embd.size() > max_embd_size) {
      const int skipped_tokens = (int) embd.size() - max_embd_size;
      embd.resize(max_embd_size);

      printf("<<input too long: skipped %d token%s>>", skipped_tokens, skipped_tokens != 1 ? "s" : "");
      fflush(stdout);
    }

    if (ga_n == 1) {
      if (n_past + (int) embd.size() + std::max<int>(0, guidance_offset) >= n_ctx) {
        if (params.n_predict == -2) {
          LOG_TEE("\n\n%s: context full and n_predict == -%d => stopping\n", __func__, params.n_predict);
          return;
        }

        const int n_left    = n_past - params.n_keep;
        const int n_discard = n_left/2;

        LOG("context full, swapping: n_past = %d, n_left = %d, n_ctx = %d, n_keep = %d, n_discard = %d\n",
            n_past, n_left, n_ctx, params.n_keep, n_discard);

        llama_kv_cache_seq_rm (ctx, 0, params.n_keep            , params.n_keep + n_discard);
        llama_kv_cache_seq_add(ctx, 0, params.n_keep + n_discard, n_past, -n_discard);

        n_past -= n_discard;

        if (ctx_guidance) {
          n_past_guidance -= n_discard;
        }

        LOG("after swap: n_past = %d, n_past_guidance = %d\n", n_past, n_past_guidance);

        LOG("embd: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd).c_str());
      }
    } else {
      // context extension via Self-Extend
      while (n_past >= ga_i + ga_w) {
        const int ib = (ga_n*ga_i)/ga_w;
        const int bd = (ga_w/ga_n)*(ga_n - 1);
        const int dd = (ga_w/ga_n) - ib*bd - ga_w;

        LOG("\n");
        LOG("shift: [%6d, %6d] + %6d -> [%6d, %6d]\n", ga_i, n_past, ib*bd, ga_i + ib*bd, n_past + ib*bd);
        LOG("div:   [%6d, %6d] / %6d -> [%6d, %6d]\n", ga_i + ib*bd, ga_i + ib*bd + ga_w, ga_n, (ga_i + ib*bd)/ga_n, (ga_i + ib*bd + ga_w)/ga_n);
        LOG("shift: [%6d, %6d] + %6d -> [%6d, %6d]\n", ga_i + ib*bd + ga_w, n_past + ib*bd, dd, ga_i + ib*bd + ga_w + dd, n_past + ib*bd + dd);

        llama_kv_cache_seq_add(ctx, 0, ga_i,                n_past,              ib*bd);
        llama_kv_cache_seq_div(ctx, 0, ga_i + ib*bd,        ga_i + ib*bd + ga_w, ga_n);
        llama_kv_cache_seq_add(ctx, 0, ga_i + ib*bd + ga_w, n_past + ib*bd,      dd);

        n_past -= bd;

        ga_i += ga_w/ga_n;

        LOG("\nn_past_old = %d, n_past = %d, ga_i = %d\n\n", n_past + bd, n_past, ga_i);
      }
    }

    if (ctx_guidance) {
      int input_size = 0;
      llama_token * input_buf = NULL;

      if (n_past_guidance < (int) guidance_inp.size()) {
        // Guidance context should have the same data with these modifications:
        //
        // * Replace the initial prompt
        // * Shift everything by guidance_offset
        embd_guidance = guidance_inp;
        if (embd.begin() + original_prompt_len < embd.end()) {
          embd_guidance.insert(
            embd_guidance.end(),
            embd.begin() + original_prompt_len,
            embd.end()
          );
        }

        input_buf  = embd_guidance.data();
        input_size = embd_guidance.size();

        LOG("guidance context: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd_guidance).c_str());
      } else {
        input_buf  = embd.data();
        input_size = embd.size();
      }

      for (int i = 0; i < input_size; i += params.n_batch) {
        int n_eval = std::min(input_size - i, params.n_batch);
        if (llama_decode(ctx_guidance, llama_batch_get_one(input_buf + i, n_eval, n_past_guidance, 0))) {
          LOG_TEE("%s : failed to eval\n", __func__);
          return;
        }

        n_past_guidance += n_eval;
      }
    }

    for (int i = 0; i < (int) embd.size(); i += params.n_batch) {
      int n_eval = (int) embd.size() - i;
      if (n_eval > params.n_batch) {
        n_eval = params.n_batch;
      }

      LOG("eval: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd).c_str());

      if (llama_decode(ctx, llama_batch_get_one(&embd[i], n_eval, n_past, 0))) {
        LOG_TEE("%s : failed to eval\n", __func__);
        return 1;
      }

      n_past += n_eval;

      LOG("n_past = %d\n", n_past);
      // Display total tokens alongside total time
      if (params.n_print > 0 && n_past % params.n_print == 0) {
        LOG_TEE("\n\033[31mTokens consumed so far = %d / %d \033[0m\n", n_past, n_ctx);
      }
    }

    if (!embd.empty() && !path_session.empty()) {
      session_tokens.insert(session_tokens.end(), embd.begin(), embd.end());
      n_session_consumed = session_tokens.size();
    }
  }

  embd.clear();
  embd_guidance.clear();
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

