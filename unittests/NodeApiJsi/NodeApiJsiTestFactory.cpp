// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <hermes_node_api_jsi/ApiLoaders/HermesApi.h>
#include <hermes_node_api_jsi/NodeApiJsiRuntime.h>
#include <jsi/test/testlib.h>

namespace facebook::jsi {

std::vector<RuntimeFactory> runtimeGenerators() {
  return {[] {
    Microsoft::NodeApiJsi::HermesApi *hermesApi =
        Microsoft::NodeApiJsi::HermesApi::fromLib();
    Microsoft::NodeApiJsi::HermesApi::setCurrent(hermesApi);

    jsr_config config{};
    jsr_runtime runtime{};
    napi_env env{};
    hermesApi->jsr_create_config(&config);
    hermesApi->jsr_config_enable_gc_api(config, true);
    hermesApi->jsr_create_runtime(config, &runtime);
    hermesApi->jsr_delete_config(config);
    hermesApi->jsr_runtime_get_node_api_env(runtime, &env);

    Microsoft::NodeApiJsi::NodeApiEnvScope envScope{env};

    return makeNodeApiJsiRuntime(env, hermesApi, [runtime]() {
      Microsoft::NodeApiJsi::HermesApi::current()->jsr_delete_runtime(runtime);
    });
  }};
}

} // namespace facebook::jsi
