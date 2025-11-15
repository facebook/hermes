// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "hermes_api.h"
#include "node_lite.h"

namespace node_api_tests {

class HermesRuntimeHolder : public IEnvHolder {
 public:
  HermesRuntimeHolder(
      std::shared_ptr<NodeLiteTaskRunner> taskRunner,
      std::function<void(napi_env, napi_value)> onUnhandledError) noexcept
      : onUnhandledError_(std::move(onUnhandledError)) {
    jsr_config config{};
    jsr_create_config(&config);
    jsr_config_enable_gc_api(config, true);
    std::shared_ptr<NodeLiteTaskRunner>* taskRunnerPtr =
        new std::shared_ptr<NodeLiteTaskRunner>(std::move(taskRunner));
    jsr_config_set_task_runner(config,
                               taskRunnerPtr,
                               NodeLiteTaskRunner::PostTaskCallback,
                               NodeLiteTaskRunner::DeleteCallback,
                               nullptr);
    jsr_config_on_unhandled_error(config, this, onUnhandledErrorCallback);
    jsr_create_runtime(config, &runtime_);
    jsr_delete_config(config);
    jsr_runtime_get_node_api_env(runtime_, &env_);
  }

  ~HermesRuntimeHolder() { jsr_delete_runtime(runtime_); }

  HermesRuntimeHolder(const HermesRuntimeHolder&) = delete;
  HermesRuntimeHolder& operator=(const HermesRuntimeHolder&) = delete;

  napi_env getEnv() override { return env_; }

  static void onUnhandledErrorCallback(void* data,
                                       napi_env env,
                                       napi_value error) {
    HermesRuntimeHolder* holder = static_cast<HermesRuntimeHolder*>(data);
    if (holder && holder->onUnhandledError_) {
      holder->onUnhandledError_(env, error);
    }
  }

 private:
  jsr_runtime runtime_{};
  napi_env env_{};
  std::function<void(napi_env, napi_value)> onUnhandledError_;
};

std::unique_ptr<IEnvHolder> CreateEnvHolder(
    std::shared_ptr<NodeLiteTaskRunner> taskRunner,
    std::function<void(napi_env, napi_value)> onUnhandledError) {
  return std::unique_ptr<IEnvHolder>(new HermesRuntimeHolder(
      std::move(taskRunner), std::move(onUnhandledError)));
}

}  // namespace node_api_tests
