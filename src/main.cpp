/*
#include "PocketAi/PocketAi.h"

int main()
{
    PocketAi ai = PocketAi();
    ai.run();
}
*/


#include "build-info.h"

#include "common/common.h"
#include "llama.h"
#include "log.h"

#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <regex>
#include <format>

std::string readFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    return buffer.str();
}

int main() {
  gpt_params params;

  std::string USERNAME = "John";
  std::string AINAME = "Pocket";

  params.model = "assets/Models/model.gguf";
  params.n_ctx = 4096;
  params.n_predict = 64;
  params.n_batch = 64;
  params.n_keep = -1;
  params.repeat_last_n = 256;
  params.repeat_penalty = 1.17647f;
  params.temp = 0.6f;
  params.mirostat = 2;
  params.input_prefix = " ";
  params.input_suffix = std::format("{}:", AINAME);
  params.antiprompt.push_back(std::format("{}:", USERNAME));
  params.interactive = false;

  std::string prompt = readFromFile("assets/Prompts/short.txt");
  prompt = std::regex_replace(prompt, std::regex("\\$\\{USERNAME\\}"), USERNAME);
  prompt = std::regex_replace(prompt, std::regex("\\$\\{AINAME\\}"), AINAME);
  params.prompt = prompt;

  llama_backend_init(params.numa);

  llama_model* model;
  llama_context* ctx;

  std::tie(model, ctx) = llama_init_from_gpt_params(params);

  if (model == NULL) {
    fprintf(stderr , "%s: error: unable to load model\n" , __func__);
    return 1;
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
  fprintf(stderr, "add_bos: %d\n", add_bos);

  std::vector<llama_token> tokens_list;
  tokens_list = ::llama_tokenize(ctx, params.prompt, true);
  const int max_context_size = llama_n_ctx(ctx);
  const int max_tokens_list_size = max_context_size - 4;

  if ((int) tokens_list.size() > max_tokens_list_size) {
    fprintf(stderr, "%s: error: prompt too long (%d tokens, max %d)\n", __func__, (int) tokens_list.size(), max_tokens_list_size);
    return 1;
  }

  {
    fprintf(stderr, "prompt: \"%s\"\n", log_tostr(params.prompt));
    fprintf(stderr, "tokens: %i\n", tokens_list.size());
  }

  if (params.n_keep < 0 || params.n_keep > (int)tokens_list.size() || params.instruct) {
    params.n_keep = (int)tokens_list.size();
  }

  std::vector<llama_token> last_tokens(max_context_size);
  std::fill(last_tokens.begin(), last_tokens.end(), 0);


  std::string path_session = params.path_prompt_cache;
  std::vector<llama_token> session_tokens;

  fprintf(stderr, "path_session: %s\n", log_tostr(path_session));

  std::vector<llama_token> embd;
  const int n_vocab = llama_n_vocab(ctx);

  std::vector<llama_token_data> candidates;
  candidates.reserve(n_vocab);
 

  std::vector<int> input_tokens;
  std::vector<int> output_tokens;
  std::ostringstream output_ss;

  int n_remain = params.n_predict;
  int n_past = 0;
  int n_session_consumed = 0;
  int n_consumed = 0;
  
  bool is_antiprompt = false;
  bool is_interacting = false;
  bool input_echo = true;
 
  while (n_remain != 0 && !is_antiprompt) {
    // predict
    if (!embd.empty()) {
      int max_embd_size = max_context_size - 4;

      // Ensure the input doesn't exceed the context size by truncating embd if necessary.
      if ((int) embd.size() > max_embd_size) {
        const int skipped_tokens = (int) embd.size() - max_embd_size;
        embd.resize(max_embd_size);

        fprintf(stderr, "<<input too long: skipped %d token%s>>", skipped_tokens, skipped_tokens != 1 ? "s" : "");
        exit(1);
      }

      // infinite text generation via context swapping
      // if we run out of context:
      // - take the n_keep first tokens from the original prompt (via n_past)
      // - take half of the last (n_ctx - n_keep) tokens and recompute the logits in batches
      if (n_past + (int) embd.size() > max_context_size) {
        if (params.n_predict == -2) {
          fprintf(stderr, "\n\n%s: context full and n_predict == -%d => stopping\n", __func__, params.n_predict);
          break;
        }

        const int n_left = n_past - params.n_keep;
        fprintf(stderr, "context full, swapping: n_past = %d, n_left = %d, n_ctx = %d, n_keep = %d\n", n_past, n_left, max_context_size, params.n_keep);

        // always keep the first token - BOS
        n_past = std::max(1, params.n_keep);

        fprintf(stderr, "after swap: n_past = %d\n", n_past);

        // insert n_left/2 tokens at the start of embd from last_tokens
        embd.insert(embd.begin(), last_tokens.begin() + max_context_size - n_left/2 - embd.size(), last_tokens.end() - embd.size());

        fprintf(stderr, "embd: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd));
        path_session.clear();
      }

      // try to reuse a matching prefix from the loaded session instead of re-eval (via n_past)
      if (n_session_consumed < (int) session_tokens.size()) {
        size_t i = 0;
        for ( ; i < embd.size(); i++) {
          if (embd[i] != session_tokens[n_session_consumed]) {
            session_tokens.resize(n_session_consumed);
            break;
          }

          n_past++;
          n_session_consumed++;

          if (n_session_consumed >= (int) session_tokens.size()) {
            ++i;
            break;
          }
        }
        if (i > 0) {
          embd.erase(embd.begin(), embd.begin() + i);
        }
      }

      // evaluate tokens in batches
      for (int i = 0; i < (int) embd.size(); i += params.n_batch) {
        int n_eval = (int) embd.size() - i;
        if (n_eval > params.n_batch) {
          n_eval = params.n_batch;
        }

        fprintf(stderr, "eval: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd));

        if (llama_eval(ctx, &embd[i], n_eval, n_past, params.n_threads)) {
          fprintf(stderr, "%s : failed to eval\n", __func__);
          exit(1);
        }

        n_past += n_eval;

        fprintf(stderr, "n_past = %d\n", n_past);
      }

      if (!embd.empty() && !path_session.empty()) {
        session_tokens.insert(session_tokens.end(), embd.begin(), embd.end());
        n_session_consumed = session_tokens.size();
      }
    }

    embd.clear();

    if ((int) tokens_list.size() <= n_consumed && !is_interacting) {
      const llama_token id = llama_sample_token(ctx, NULL, NULL, params, last_tokens, candidates);

      last_tokens.erase(last_tokens.begin());
      last_tokens.push_back(id);

      /* fprintf(stderr, "last: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, last_tokens)); */

      embd.push_back(id);

      // echo this to console
      input_echo = true;

      // decrement remaining sampling budget
      --n_remain;

      fprintf(stderr, "n_remain: %d\n", n_remain);
    } else {
      // some user input remains from prompt or interaction, forward it to processing
      fprintf(stderr, "tokens_list.size(): %d, n_consumed: %d\n", (int) tokens_list.size(), n_consumed);
      while ((int) tokens_list.size() > n_consumed) {
        embd.push_back(tokens_list[n_consumed]);
        last_tokens.erase(last_tokens.begin());
        last_tokens.push_back(tokens_list[n_consumed]);
        ++n_consumed;
        if ((int) embd.size() >= params.n_batch) {
          break;
        }
      }
    }

    // display text
    if (input_echo) {
      for (auto id : embd) {
        const std::string token_str = llama_token_to_piece(ctx, id);
        fprintf(stderr, "token_str: \"%s\"", token_str.c_str());

        if (embd.size() > 1) {
          input_tokens.push_back(id);
        } else {
          output_tokens.push_back(id);
          output_ss << token_str;
        }
      }
      fflush(stdout);
    }

    // if not currently processing queued inputs;
    if ((int) tokens_list.size() <= n_consumed) {
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
          size_t extra_padding = params.interactive ? 0 : 2;
          size_t search_start_pos = last_output.length() > static_cast<size_t>(antiprompt.length() + extra_padding)
            ? last_output.length() - static_cast<size_t>(antiprompt.length() + extra_padding)
            : 0;

          if (last_output.find(antiprompt, search_start_pos) != std::string::npos) {
            if (params.interactive) {
              is_interacting = true;
            }
            is_antiprompt = true;
            break;
          }
        }

        if (is_antiprompt) {
          fprintf(stderr, "found antiprompt: %s\n", last_output.c_str());
        }
      }

      // deal with end of text token in interactive mode
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
        } else if (params.instruct) {
          is_interacting = true;
        }
      }

      if (n_past > 0 && is_interacting) {
        fprintf(stderr, "waiting for user input\n");
         
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
          fprintf(stderr, "input tokens: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, line_inp));

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

    // end of text token
    if (!embd.empty() && embd.back() == llama_token_eos(ctx) && !(params.instruct || params.interactive)) {
      fprintf(stderr, " [end of text]\n");
      break;
    }

    // In interactive mode, respect the maximum number of tokens and drop back to user input when reached.
    // We skip this logic when n_predict == -1 (infinite) or -2 (stop at context size).
    if (params.interactive && n_remain <= 0 && params.n_predict >= 0) {
      n_remain = params.n_predict;
      is_interacting = true;
    }
  }


  llama_free(ctx);
  llama_free_model(model);

  llama_backend_free();

  fprintf(stderr, "\n\n");

  return 0;
}
