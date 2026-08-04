#include <fstream>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

#include "base64.hpp"
#include "common/log.h"
#include "sampling.h"
#include "llama.h"
#include "ggml.h"
#include "common.h"
#include "arg.h"
#include "console.h"
#include "src/llama-context.h"

struct llava_context {
 struct clip_ctx * ctx_clip = NULL;
 struct llama_context * ctx_llama = NULL;
 struct llama_model * model = NULL;
};


std::mutex ctx_mutex;


// void warmup_func(const std::vector<std::string> &warmup_texts, llama_model *model, llama_context *ctx){
//   for (auto& text : warmup_texts) {
//     llama_token tokens[256];
//     int32_t n_text_chars = static_cast<int32_t>(text.size());
//     int32_t n_tokens = llama_tokenize(&model->vocab, text.c_str(), n_text_chars,
//                                         tokens, 256, true, true);

//     llama_batch batch = llama_batch_get_one(tokens, n_tokens);
//     std::lock_guard<std::mutex> lock(ctx_mutex);
//     llama_decode(ctx, batch);
//   }
// }

// int main(int argc, char** argv) {
//   auto start = std::chrono::steady_clock::now();
//   static common_params* g_params;
//   common_params params;
//   g_params = &params;
//   if (!common_params_parse(argc, argv, params, LLAMA_EXAMPLE_MAIN, print_usage)) {
//       return 1;
//   }

//   std::string llm_model_name_ = "/dev/shm/qwen2.5-1.5b-instruct-q5_k_m.gguf";
//   params.model = llm_model_name_;
//   params.cpuparams.n_threads = 8;
//   params.sampling.temp = 0.5;
//   params.n_predict = 128;

//   std::string value = "/userdata/magicbox/app/ros_ws/install/qwen_llm/share/qwen_llm/config/system_prompt.txt";
//   std::ifstream file(value);
//   if (!file) {
//       throw std::runtime_error(string_format("error: failed to open file '%s'\n", value.c_str()));
//   }
//   // store the external file name in params
//   params.prompt_file = value;
//   std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), back_inserter(params.prompt));
//   if (!params.prompt.empty() && params.prompt.back() == '\n') {
//       params.prompt.pop_back();
//   }

//   common_init();

//   auto & sparams = params.sampling;

//   // save choice to use color for later
//   // (note for later: this is a slightly awkward choice)
//   console::init(params.simple_io, params.use_color);
//   atexit([]() { console::cleanup(); });

//   llama_backend_init();
//   llama_numa_init(params.numa);

//   llama_model * model = nullptr;
//   llama_context * ctx = nullptr;
//   common_sampler * smpl = nullptr;

//   // load the model and apply lora adapter, if any
//   LOG_INF("%s: load the model and apply lora adapter, if any\n", __func__);
//   common_init_result llama_init = common_init_from_params(params);

//   model = llama_init.model.get();
//   ctx = llama_init.context.get();

//   const llama_vocab * vocab = llama_model_get_vocab(model);
//   const bool add_bos = llama_vocab_get_add_bos(vocab) && !params.use_jinja;
//   if (!llama_model_has_encoder(model)) {
//       GGML_ASSERT(!llama_vocab_get_add_eos(vocab));
//   }
//   std::vector<std::string> warmup_texts = {
//       "你好，这是第一段预热文本。",
//       "模型预热第二段，用于激活所有 kernel。",
//       "你叫什么名字。",
//       "介绍一下地瓜机器人。",
//       "最后一段，确保 KV cache 已经填满。"
//   };
//   auto now = std::chrono::steady_clock::now();
//   auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

//   start = std::chrono::steady_clock::now();
//   for (auto& text : warmup_texts) {
//     llama_token tokens[256];
//     int32_t n_text_chars = static_cast<int32_t>(text.size());
//     int32_t n_tokens = llama_tokenize(&model->vocab, text.c_str(), n_text_chars,
//                                         tokens, 256, true, true);

//     llama_batch batch = llama_batch_get_one(tokens, n_tokens);
//     std::lock_guard<std::mutex> lock(ctx_mutex);
//     llama_decode(ctx, batch);
//   }

//   now = std::chrono::steady_clock::now();
//   elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

//   LOG_WRN("%s: warming up the model with an empty run - please wait ... (--no-warmup to disable)\n", __func__);

//   llama_set_warmup(ctx, true);
//   std::vector<llama_token> tmp;
//   llama_token bos = llama_vocab_bos(vocab);
//   llama_token eos = llama_vocab_eos(vocab);

//   // some models (e.g. T5) don't have a BOS token
//   if (bos != LLAMA_TOKEN_NULL) {
//       tmp.push_back(bos);
//   }
//   if (eos != LLAMA_TOKEN_NULL) {
//       tmp.push_back(eos);
//   }
//   if (tmp.empty()) {
//       tmp.push_back(0);
//   }

//   if (llama_model_has_encoder(model)) {
//       llama_encode(ctx, llama_batch_get_one(tmp.data(), tmp.size()));
//       llama_token decoder_start_token_id = llama_model_decoder_start_token(model);
//       if (decoder_start_token_id == LLAMA_TOKEN_NULL) {
//           decoder_start_token_id = bos;
//       }
//       tmp.clear();
//       tmp.push_back(decoder_start_token_id);
//   }
//   if (llama_model_has_decoder(model)) {
//       llama_decode(ctx, llama_batch_get_one(tmp.data(), std::min(tmp.size(), (size_t) params.n_batch)));
//   }
//   llama_kv_cache_clear(ctx);
//   llama_synchronize(ctx);
//   llama_perf_context_reset(ctx);



  // std::cout << "所有预热文本处理完成。" << std::endl;
  // return 0;

// }


static void print_usage(int argc, char ** argv) {
    fprintf(stderr, "usage: %s -m model.gguf [-c n_ctx] [-ngl n_gpu_layers] [-t threads]\n", argv[0]);
}

int main(int argc, char ** argv) {
    std::setlocale(LC_NUMERIC, "C");

    std::string model_path = "/userdata/magicbox/config/qwen2.5-1.5b-instruct-q5_k_m.gguf";
    int ngl = 0;
    int n_ctx = 2048;
    int n_threads = 8;

    if (model_path.empty()) {
        print_usage(argc, argv);
        return 1;
    }

    // ----------------------
    // 日志（仅错误）
    // ----------------------
    llama_log_set([](enum ggml_log_level level, const char * text, void *){
        if (level >= GGML_LOG_LEVEL_ERROR) {
            fprintf(stderr, "%s", text);
        }
    }, nullptr);

    // ----------------------
    // 加载backend
    // ----------------------
    ggml_backend_load_all();

    // ----------------------
    // 加载模型
    // ----------------------
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = ngl;

    llama_model * model = llama_model_load_from_file(model_path.c_str(), model_params);
    if (!model) {
        fprintf(stderr, "error: unable to load model\n");
        return 1;
    }

    const llama_vocab * vocab = llama_model_get_vocab(model);

    // ----------------------
    // 初始化context
    // ----------------------
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = n_ctx;
    ctx_params.n_batch = n_ctx;
    ctx_params.n_threads = n_threads;
    ctx_params.n_threads_batch = n_threads;

    llama_context * ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
        fprintf(stderr, "error: failed to create context\n");
        return 1;
    }

    // ----------------------
    // sampler
    // ----------------------
    llama_sampler * smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(smpl, llama_sampler_init_min_p(0.05f, 1));
    llama_sampler_chain_add(smpl, llama_sampler_init_temp(0.8f));
    llama_sampler_chain_add(smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

    // ----------------------
    // 🔥 Warmup开始
    // ----------------------
    printf("Running warmup...\n");

    std::string prompt = "Hello";

    const bool is_first = true;

    // tokenize（先拿长度）
    int n_prompt_tokens = -llama_tokenize(vocab, prompt.c_str(), prompt.size(), NULL, 0, is_first, true);

    std::vector<llama_token> tokens(n_prompt_tokens);

    if (llama_tokenize(vocab, prompt.c_str(), prompt.size(),
                       tokens.data(), tokens.size(),
                       is_first, true) < 0) {
        fprintf(stderr, "tokenize failed\n");
        return 1;
    }

    // prefill
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());

    if (llama_decode(ctx, batch) != 0) {
        fprintf(stderr, "warmup prefill failed\n");
        return 1;
    }

    // decode阶段（关键！）
    for (int i = 0; i < 8; i++) {
        llama_token token = llama_sampler_sample(smpl, ctx, -1);

        if (llama_vocab_is_eog(vocab, token)) {
            break;
        }

        // 打印一下，防止优化掉
        char buf[128];
        int n = llama_token_to_piece(vocab, token, buf, sizeof(buf), 0, true);
        if (n > 0) {
            printf("%.*s", n, buf);
        }

        batch = llama_batch_get_one(&token, 1);

        if (llama_decode(ctx, batch) != 0) {
            fprintf(stderr, "warmup decode failed\n");
            return 1;
        }
    }
    printf("\nWarmup finished.\n");

    // ----------------------
    // 释放资源
    // ----------------------
    llama_sampler_free(smpl);
    llama_free(ctx);
    llama_model_free(model);

    return 0;
}