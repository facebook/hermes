/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT license.
 */

#include "hermes_api.h"
#include <stdexcept>
#include "hermes/BCGen/HBC/BCProvider.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Runtime.h"
#include "hermes/hermes.h"
#include "hermes_node_api.h"
#include "llvh/Support/raw_os_ostream.h"

#define CHECKED_RUNTIME(runtime)                                               \
  (runtime == nullptr)                                                         \
      ? napi_generic_failure                                                   \
      : reinterpret_cast<facebook::hermes::RuntimeWrapper*>(runtime)

#define CHECKED_CONFIG(config)                                                 \
  (config == nullptr)                                                          \
      ? napi_generic_failure                                                   \
      : reinterpret_cast<facebook::hermes::ConfigWrapper*>(config)

#define CHECK_ARG(arg)                                                         \
  if (arg == nullptr) {                                                        \
    return napi_generic_failure;                                               \
  }

#define CHECKED_ENV_RUNTIME(env)                                               \
  (env == nullptr) ? napi_generic_failure                                      \
                   : facebook::hermes::RuntimeWrapper::from(env)

#define CHECK_STATUS(func)                                                     \
  do {                                                                         \
    napi_status status__ = func;                                               \
    if (status__ != napi_ok) {                                                 \
      return status__;                                                         \
    }                                                                          \
  } while (0)

// Return error status with message.
#define ERROR_STATUS(status, ...)                                              \
  ::hermes::node_api::setLastNativeError(                                      \
      env_, (status), (__FILE__), (uint32_t)(__LINE__), __VA_ARGS__)

// Return napi_generic_failure with message.
#define GENERIC_FAILURE(...) ERROR_STATUS(napi_generic_failure, __VA_ARGS__)

namespace facebook::hermes {

::hermes::vm::Runtime* getVMRuntime(HermesRuntime& runtime) noexcept {
  ::facebook::hermes::IHermes* hermes =
      facebook::jsi::castInterface<::facebook::hermes::IHermes>(&runtime);
  return static_cast<::hermes::vm::Runtime*>(hermes->getVMRuntimeUnsafe());
}

class Task : public ::hermes::node_api::Task {
 public:
  static void run(void* task) { reinterpret_cast<Task*>(task)->invoke(); }

  static void deleteTask(void* task, void* /*deleterData*/) {
    delete reinterpret_cast<Task*>(task);
  }
};

template <typename TLambda>
class LambdaTask : public Task {
 public:
  LambdaTask(TLambda&& lambda) : lambda_(std::move(lambda)) {}

  void invoke() noexcept override { lambda_(); }

 private:
  TLambda lambda_;
};

class TaskRunner : public ::hermes::node_api::TaskRunner {
 public:
  TaskRunner(void* data,
             jsr_task_runner_post_task_cb postTaskCallback,
             jsr_data_delete_cb deleteCallback,
             void* deleterData)
      : data_(data),
        postTaskCallback_(postTaskCallback),
        deleteCallback_(deleteCallback),
        deleterData_(deleterData) {}

  ~TaskRunner() {
    if (deleteCallback_ != nullptr) {
      deleteCallback_(data_, deleterData_);
    }
  }

  void post(std::unique_ptr<::hermes::node_api::Task> task) noexcept override {
    postTaskCallback_(
        data_, task.release(), &Task::run, &Task::deleteTask, nullptr);
  }

 private:
  void* data_;
  jsr_task_runner_post_task_cb postTaskCallback_;
  jsr_data_delete_cb deleteCallback_;
  void* deleterData_;
};

class ScriptBuffer : public facebook::jsi::Buffer {
 public:
  ScriptBuffer(const uint8_t* data,
               size_t size,
               jsr_data_delete_cb deleteCallback,
               void* deleterData)
      : data_(data),
        size_(size),
        deleteCallback_(deleteCallback),
        deleterData_(deleterData) {}

  ~ScriptBuffer() {
    if (deleteCallback_ != nullptr) {
      deleteCallback_(const_cast<uint8_t*>(data_), deleterData_);
    }
  }

  const uint8_t* data() const override { return data_; }

  size_t size() const override { return size_; }

  static void deleteBuffer(void* /*data*/, void* scriptBuffer) {
    delete reinterpret_cast<ScriptBuffer*>(scriptBuffer);
  }

 private:
  const uint8_t* data_{};
  size_t size_{};
  jsr_data_delete_cb deleteCallback_{};
  void* deleterData_{};
};
class ConfigWrapper {
 public:
  napi_status setExplicitMicrotasks(bool value) {
    explicitMicrotasks_ = value;
    return napi_status::napi_ok;
  }

  napi_status setUnhandledErrorCallback(
      std::function<void(napi_env, napi_value)> unhandledErrorCallback) {
    unhandledErrorCallback_ = unhandledErrorCallback;
    return napi_status::napi_ok;
  }

  napi_status setTaskRunner(std::unique_ptr<TaskRunner> taskRunner) {
    taskRunner_ = std::move(taskRunner);
    return napi_status::napi_ok;
  }

  const std::shared_ptr<TaskRunner>& taskRunner() const { return taskRunner_; }

  const std::function<void(napi_env, napi_value)>& unhandledErrorCallback()
      const {
    return unhandledErrorCallback_;
  }

  ::hermes::vm::RuntimeConfig getRuntimeConfig() const {
    ::hermes::vm::RuntimeConfig::Builder config;
    config.withMicrotaskQueue(explicitMicrotasks_);
    return config.build();
  }

 private:
  bool explicitMicrotasks_{};
  std::function<void(napi_env env, napi_value value)> unhandledErrorCallback_{};
  std::shared_ptr<TaskRunner> taskRunner_;
};

class HermesRuntime;

// An implementation of PreparedJavaScript that wraps a BytecodeProvider.
class NodeApiScriptModel final {
 public:
  explicit NodeApiScriptModel(
      std::unique_ptr<::hermes::hbc::BCProvider> bcProvider,
      ::hermes::vm::RuntimeModuleFlags runtimeFlags,
      std::string sourceURL,
      bool isBytecode)
      : bcProvider_(std::move(bcProvider)),
        runtimeFlags_(runtimeFlags),
        sourceURL_(std::move(sourceURL)),
        isBytecode_(isBytecode) {}

  std::shared_ptr<::hermes::hbc::BCProvider> bytecodeProvider() const {
    return bcProvider_;
  }

  ::hermes::vm::RuntimeModuleFlags runtimeFlags() const {
    return runtimeFlags_;
  }

  const std::string& sourceURL() const { return sourceURL_; }

  bool isBytecode() const { return isBytecode_; }

 private:
  std::shared_ptr<::hermes::hbc::BCProvider> bcProvider_;
  ::hermes::vm::RuntimeModuleFlags runtimeFlags_;
  std::string sourceURL_;
  bool isBytecode_{false};
};

// Wraps script data as hermes::Buffer
class ScriptDataBuffer final : public ::hermes::Buffer {
 public:
  ScriptDataBuffer(std::string source) noexcept
      : Buffer(reinterpret_cast<const uint8_t*>(source.data()), source.size()),
        source_(std::move(source)) {}

  ScriptDataBuffer(const ScriptDataBuffer&) = delete;
  ScriptDataBuffer& operator=(const ScriptDataBuffer&) = delete;

 private:
  std::string source_;
};

class JsiBuffer final : public ::hermes::Buffer {
 public:
  JsiBuffer(std::shared_ptr<const facebook::jsi::Buffer> buffer) noexcept
      : Buffer(buffer->data(), buffer->size()), buffer_(std::move(buffer)) {}

 private:
  std::shared_ptr<const facebook::jsi::Buffer> buffer_;
};

class JsiSmallVectorBuffer final : public facebook::jsi::Buffer {
 public:
  JsiSmallVectorBuffer(llvh::SmallVector<char, 0> data) noexcept
      : data_(std::move(data)) {}

  size_t size() const override { return data_.size(); }

  const uint8_t* data() const override {
    return reinterpret_cast<const uint8_t*>(data_.data());
  }

 private:
  llvh::SmallVector<char, 0> data_;
};

class RuntimeWrapper {
 public:
  explicit RuntimeWrapper(const ConfigWrapper& config)
      : hermesJsiRuntime_(makeHermesRuntime(config.getRuntimeConfig())),
        hermesVMRuntime_(*getVMRuntime(*hermesJsiRuntime_)) {
    ::hermes::vm::RuntimeConfig runtimeConfig = config.getRuntimeConfig();
    switch (runtimeConfig.getCompilationMode()) {
      case ::hermes::vm::SmartCompilation:
        compileFlags_.lazy = true;
        // (Leaves thresholds at default values)
        break;
      case ::hermes::vm::ForceEagerCompilation:
        compileFlags_.lazy = false;
        break;
      case ::hermes::vm::ForceLazyCompilation:
        compileFlags_.lazy = true;
        compileFlags_.preemptiveFileCompilationThreshold = 0;
        compileFlags_.preemptiveFunctionCompilationThreshold = 0;
        break;
    }

    compileFlags_.enableGenerator = runtimeConfig.getEnableGenerator();
    compileFlags_.emitAsyncBreakCheck =
        runtimeConfig.getAsyncBreakCheckInEval();

    ::hermes::vm::CallResult<napi_env> envRes =
        ::hermes::node_api::getOrCreateNodeApiEnvironment(
            hermesVMRuntime_,
            compileFlags_,
            config.taskRunner(),
            config.unhandledErrorCallback(),
            NAPI_VERSION_EXPERIMENTAL);

    if (envRes.getStatus() == ::hermes::vm::ExecutionStatus::EXCEPTION) {
      throw std::runtime_error("Failed to create Node API environment");
    }
    env_ = envRes.getValue();
    ::hermes::node_api::setNodeApiEnvironmentData(
        env_, kRuntimeWrapperTag, this);
  }

  static facebook::hermes::RuntimeWrapper* from(napi_env env) {
    if (env == nullptr) {
      return nullptr;
    }

    void* data{};
    ::hermes::node_api::getNodeApiEnvironmentData(
        env, kRuntimeWrapperTag, &data);
    return reinterpret_cast<facebook::hermes::RuntimeWrapper*>(data);
  }

  napi_status addToProfiler() {
    hermesJsiRuntime_->registerForProfiling();
    return napi_ok;
  }

  napi_status removeFromProfiler() {
    hermesJsiRuntime_->unregisterForProfiling();
    return napi_ok;
  }

  napi_status getNodeApi(napi_env* env) {
    *env = env_;
    return napi_ok;
  }

  napi_status getDescription(const char** result) noexcept {
    CHECK_ARG(result);
    *result = "Hermes";
    return napi_ok;
  }

  napi_status drainMicrotasks(int32_t maxCountHint, bool* result) noexcept {
    CHECK_ARG(result);
    if (hermesVMRuntime_.hasMicrotaskQueue()) {
      CHECK_STATUS(::hermes::node_api::checkJSErrorStatus(
          env_, hermesVMRuntime_.drainJobs()));
    }

    hermesVMRuntime_.clearKeptObjects();
    *result = true;
    return napi_ok;
  }

  //---------------------------------------------------------------------------
  // Script running
  //---------------------------------------------------------------------------

  // Run script from a string value.
  // The sourceURL is used only for error reporting.
  napi_status runScript(napi_value source,
                        const char* sourceURL,
                        napi_value* result) noexcept {
    CHECK_ARG(source);
    size_t sourceSize{};
    CHECK_STATUS(
        napi_get_value_string_utf8(env_, source, nullptr, 0, &sourceSize));
    std::string buffer(sourceSize, '\0');
    CHECK_STATUS(napi_get_value_string_utf8(
        env_, source, buffer.data(), sourceSize + 1, nullptr));

    std::unique_ptr<NodeApiScriptModel> scriptModel;
    CHECK_STATUS(
        createPreparedScript(std::move(buffer), sourceURL, scriptModel));

    return runPreparedScript(scriptModel.get(), result);
  }

  napi_status createPreparedScript(
      std::string source,
      std::string sourceURL,
      std::unique_ptr<NodeApiScriptModel>& scriptModel) noexcept {
    std::unique_ptr<ScriptDataBuffer> buffer =
        std::make_unique<ScriptDataBuffer>(std::move(source));

    std::pair<std::unique_ptr<::hermes::hbc::BCProvider>, std::string> bcErr{};
    ::hermes::vm::RuntimeModuleFlags runtimeFlags{};

    bool isBytecode = isHermesBytecode(buffer->data(), buffer->size());
    // Save the first few bytes of the buffer so that we can later append them
    // to any error message.
    uint8_t bufPrefix[16];
    const size_t bufSize = buffer->size();
    std::memcpy(
        bufPrefix, buffer->data(), std::min(sizeof(bufPrefix), bufSize));

    // Construct the BC provider either from buffer or source.
    if (isBytecode) {
      bcErr = ::hermes::hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
          std::move(buffer));
      runtimeFlags.persistent = true;
    } else {
      llvh::StringRef sourceMap;
      bcErr = ::hermes::hbc::createBCProviderFromSrc(
          std::move(buffer), sourceURL, sourceMap, compileFlags_);
      if (bcErr.first) {
        runtimeFlags.persistent = bcErr.first->allowPersistent();
      }
    }
    if (!bcErr.first) {
      std::string errorMessage;
      llvh::raw_string_ostream stream(errorMessage);
      stream << " Buffer size: " << bufSize << ", starts with: ";
      for (size_t i = 0; i < sizeof(bufPrefix) && i < bufSize; ++i) {
        stream << llvh::format_hex_no_prefix(bufPrefix[i], 2);
      }
      return GENERIC_FAILURE(
          "Compiling JS failed: ", bcErr.second, stream.str());
    }
    scriptModel.reset(new NodeApiScriptModel(
        std::move(bcErr.first), runtimeFlags, sourceURL, isBytecode));
    return ::hermes::node_api::clearLastNativeError(env_);
  }

  napi_status runPreparedScript(NodeApiScriptModel* preparedScript,
                                napi_value* result) noexcept {
    CHECK_ARG(preparedScript);
    return ::hermes::node_api::runInNodeApiContext(
        env_,
        [this, preparedScript]() {
          return hermesVMRuntime_.runBytecode(
              preparedScript->bytecodeProvider(),
              preparedScript->runtimeFlags(),
              preparedScript->sourceURL(),
              ::hermes::vm::Runtime::makeNullHandle<
                  ::hermes::vm::Environment>());
        },
        result);
  }

  // Internal function to check if buffer contains Hermes VM bytecode.
  static bool isHermesBytecode(const uint8_t* data, size_t len) noexcept {
    return ::hermes::hbc::BCProviderFromBuffer::isBytecodeStream(
        llvh::ArrayRef<uint8_t>(data, len));
  }

  napi_status initializeNativeModule(napi_addon_register_func register_module,
                                     int32_t api_version,
                                     napi_value* exports) noexcept {
    return ::hermes::node_api::initializeNodeApiModule(
        hermesVMRuntime_, register_module, api_version, exports);
  }

  napi_status collectGarbage() noexcept {
    hermesVMRuntime_.collect("test");
    return napi_ok;
  }

 private:
  std::shared_ptr<HermesRuntime> hermesJsiRuntime_;
  ::hermes::vm::Runtime& hermesVMRuntime_;
  napi_env env_;

  // Flags used by byte code compiler.
  ::hermes::hbc::CompileFlags compileFlags_{};

  static constexpr napi_type_tag kRuntimeWrapperTag{0xfa327a491b4b4d20,
                                                    0x94407c81c2d4e8f2};
};

}  // namespace facebook::hermes

JSR_API
jsr_create_runtime(jsr_config config, jsr_runtime* runtime) {
  CHECK_ARG(config);
  CHECK_ARG(runtime);
  *runtime = reinterpret_cast<jsr_runtime>(new facebook::hermes::RuntimeWrapper(
      *reinterpret_cast<facebook::hermes::ConfigWrapper*>(config)));
  return napi_ok;
}

JSR_API jsr_delete_runtime(jsr_runtime runtime) {
  CHECK_ARG(runtime);
  delete reinterpret_cast<facebook::hermes::RuntimeWrapper*>(runtime);
  return napi_ok;
}

JSR_API jsr_runtime_get_node_api_env(jsr_runtime runtime, napi_env* env) {
  return CHECKED_RUNTIME(runtime)->getNodeApi(env);
}

static facebook::hermes::IHermesRootAPI* getHermesRootAPI() {
  // The makeHermesRootAPI returns a singleton.
  return facebook::jsi::castInterface<facebook::hermes::IHermesRootAPI>(
      facebook::hermes::makeHermesRootAPI());
}

JSR_API hermes_sampling_profiler_enable() {
  getHermesRootAPI()->enableSamplingProfiler();
  return napi_ok;
}

JSR_API hermes_sampling_profiler_disable() {
  getHermesRootAPI()->disableSamplingProfiler();
  return napi_ok;
}

JSR_API hermes_sampling_profiler_add(jsr_runtime runtime) {
  return CHECKED_RUNTIME(runtime)->addToProfiler();
}

JSR_API hermes_sampling_profiler_remove(jsr_runtime runtime) {
  return CHECKED_RUNTIME(runtime)->removeFromProfiler();
}

JSR_API hermes_sampling_profiler_dump_to_file(const char* filename) {
  getHermesRootAPI()->dumpSampledTraceToFile(filename);
  return napi_ok;
}

JSR_API jsr_create_config(jsr_config* config) {
  CHECK_ARG(config);
  *config = reinterpret_cast<jsr_config>(new facebook::hermes::ConfigWrapper());
  return napi_ok;
}

JSR_API jsr_delete_config(jsr_config config) {
  CHECK_ARG(config);
  delete reinterpret_cast<facebook::hermes::ConfigWrapper*>(config);
  return napi_ok;
}

JSR_API jsr_config_enable_gc_api(jsr_config /*config*/, bool /*value*/) {
  // We do nothing for now.
  return napi_ok;
}

JSR_API jsr_config_set_explicit_microtasks(jsr_config config, bool value) {
  return CHECKED_CONFIG(config)->setExplicitMicrotasks(value);
}

JSR_API jsr_config_set_task_runner(
    jsr_config config,
    void* task_runner_data,
    jsr_task_runner_post_task_cb task_runner_post_task_cb,
    jsr_data_delete_cb task_runner_data_delete_cb,
    void* deleter_data) {
  return CHECKED_CONFIG(config)->setTaskRunner(
      std::make_unique<facebook::hermes::TaskRunner>(task_runner_data,
                                                     task_runner_post_task_cb,
                                                     task_runner_data_delete_cb,
                                                     deleter_data));
}

JSR_API jsr_config_on_unhandled_error(
    jsr_config config,
    void* cb_data,
    jsr_unhandled_error_cb unhandled_error_cb) {
  return CHECKED_CONFIG(config)->setUnhandledErrorCallback(
      [cb_data, unhandled_error_cb](napi_env env, napi_value error) {
        unhandled_error_cb(cb_data, env, error);
      });
}

//=============================================================================
// Node-API extensions to host JS engine and to implement JSI
//=============================================================================

JSR_API jsr_collect_garbage(napi_env env) {
  return CHECKED_ENV_RUNTIME(env)->collectGarbage();
}

JSR_API
jsr_has_unhandled_promise_rejection(napi_env /*env*/, bool* /*result*/) {
  return napi_generic_failure;
}

JSR_API jsr_get_and_clear_last_unhandled_promise_rejection(
    napi_env /*env*/, napi_value* /*result*/) {
  return napi_generic_failure;
}

JSR_API jsr_get_description(napi_env env, const char** result) {
  return CHECKED_ENV_RUNTIME(env)->getDescription(result);
}

JSR_API jsr_queue_microtask(napi_env env, napi_value callback) {
  return hermes::node_api::queueMicrotask(env, callback);
}

JSR_API
jsr_drain_microtasks(napi_env env, int32_t max_count_hint, bool* result) {
  return CHECKED_ENV_RUNTIME(env)->drainMicrotasks(max_count_hint, result);
}

JSR_API jsr_open_napi_env_scope(napi_env env, jsr_napi_env_scope* scope) {
  return hermes::node_api::openNodeApiScope(env,
                                            reinterpret_cast<void**>(scope));
}

JSR_API jsr_close_napi_env_scope(napi_env env, jsr_napi_env_scope scope) {
  return hermes::node_api::closeNodeApiScope(env,
                                             reinterpret_cast<void*>(scope));
}

//-----------------------------------------------------------------------------
// Script preparing and running.
//
// Script is usually converted to byte code, or in other words - prepared - for
// execution. Then, we can run the prepared script.
//-----------------------------------------------------------------------------

JSR_API jsr_run_script(napi_env env,
                       napi_value source,
                       const char* source_url,
                       napi_value* result) {
  return CHECKED_ENV_RUNTIME(env)->runScript(source, source_url, result);
}

#if 0
JSR_API jsr_create_prepared_script(
    napi_env env,
    const uint8_t *script_data,
    size_t script_length,
    jsr_data_delete_cb script_delete_cb,
    void *deleter_data,
    const char *source_url,
    jsr_prepared_script *result) {
  return CHECKED_ENV_RUNTIME(env)->createPreparedScript(
      script_data,
      script_length,
      script_delete_cb,
      deleter_data,
      source_url,
      result);
}

JSR_API
jsr_delete_prepared_script(napi_env env, jsr_prepared_script prepared_script) {
  return CHECKED_ENV_RUNTIME(env)->deletePreparedScript(prepared_script);
}

JSR_API jsr_prepared_script_run(
    napi_env env,
    jsr_prepared_script prepared_script,
    napi_value *result) {
  return CHECKED_ENV_RUNTIME(env)->runPreparedScript(prepared_script, result);
}
#endif
JSR_API jsr_initialize_native_module(napi_env env,
                                     napi_addon_register_func register_module,
                                     int32_t api_version,
                                     napi_value* exports) {
  return CHECKED_ENV_RUNTIME(env)->initializeNativeModule(
      register_module, api_version, exports);
}
