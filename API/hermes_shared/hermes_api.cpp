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
#include "MurmurHash.h"
#include "ScriptStore.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Runtime.h"
#include "hermes/cdp/CDPAgent.h"
#include "hermes/cdp/CDPDebugAPI.h"
#include "hermes/hermes.h"
#include "hermes/inspector/RuntimeAdapter.h"
#include "hermes/inspector/chrome/Registration.h"
#include "hermes_node_api.h"
#include "llvh/Support/raw_os_ostream.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <werapi.h>

#define CHECKED_RUNTIME(runtime) \
  (runtime == nullptr)           \
      ? napi_generic_failure     \
      : reinterpret_cast<facebook::hermes::RuntimeWrapper *>(runtime)

#define CHECKED_CONFIG(config) \
  (config == nullptr)          \
      ? napi_generic_failure   \
      : reinterpret_cast<facebook::hermes::ConfigWrapper *>(config)

#define CHECK_ARG(arg)           \
  if (arg == nullptr) {          \
    return napi_generic_failure; \
  }

#define CHECKED_ENV_RUNTIME(env)          \
  (env == nullptr) ? napi_generic_failure \
                   : facebook::hermes::RuntimeWrapper::from(env)

#define CHECK_STATUS(func)       \
  do {                           \
    napi_status status__ = func; \
    if (status__ != napi_ok) {   \
      return status__;           \
    }                            \
  } while (0)

// Return error status with message.
#define ERROR_STATUS(status, ...)         \
  ::hermes::node_api::setLastNativeError( \
      env_, (status), (__FILE__), (uint32_t)(__LINE__), __VA_ARGS__)

// Return napi_generic_failure with message.
#define GENERIC_FAILURE(...) ERROR_STATUS(napi_generic_failure, __VA_ARGS__)

namespace facebook::react {
extern bool g_isOldInspectorInitialized;
}

namespace facebook::hermes {

union HermesBuildVersionInfo {
  struct {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
    uint16_t revision;
  };
  uint64_t version;
};

constexpr HermesBuildVersionInfo HermesBuildVersion = {HERMES_FILE_VERSION_BIN};

::hermes::vm::Runtime *getVMRuntime(HermesRuntime &runtime) noexcept {
  ::facebook::hermes::IHermes *hermes =
      facebook::jsi::castInterface<::facebook::hermes::IHermes>(&runtime);
  return static_cast<::hermes::vm::Runtime *>(hermes->getVMRuntimeUnsafe());
}

class CrashManagerImpl : public ::hermes::vm::CrashManager {
 public:
  void registerMemory(void *mem, size_t length) override {
    if (length >
        WER_MAX_MEM_BLOCK_SIZE) { // Hermes thinks we should save the whole
                                  // block, but WER allows 64K max
      _largeMemBlocks[(intptr_t)mem] = length;

      auto pieceCount = length / WER_MAX_MEM_BLOCK_SIZE;
      for (size_t i = 0; i < pieceCount; i++) {
        WerRegisterMemoryBlock(
            (char *)mem + i * WER_MAX_MEM_BLOCK_SIZE, WER_MAX_MEM_BLOCK_SIZE);
      }

      WerRegisterMemoryBlock(
          (char *)mem + pieceCount * WER_MAX_MEM_BLOCK_SIZE,
          static_cast<uint32_t>(length - pieceCount * WER_MAX_MEM_BLOCK_SIZE));
    } else {
      WerRegisterMemoryBlock(mem, static_cast<DWORD>(length));
    }
  }

  void unregisterMemory(void *mem) override {
    if (_largeMemBlocks.find((intptr_t)mem) != _largeMemBlocks.end()) {
      // This memory was larger than what WER supports so we split it up into
      // chunks of size WER_MAX_MEM_BLOCK_SIZE
      auto pieceCount = _largeMemBlocks[(intptr_t)mem] / WER_MAX_MEM_BLOCK_SIZE;
      for (size_t i = 0; i < pieceCount; i++) {
        WerUnregisterMemoryBlock((char *)mem + i * WER_MAX_MEM_BLOCK_SIZE);
      }

      WerUnregisterMemoryBlock(
          (char *)mem + pieceCount * WER_MAX_MEM_BLOCK_SIZE);

      _largeMemBlocks.erase((intptr_t)mem);
    } else {
      WerUnregisterMemoryBlock(mem);
    }
  }

  void setCustomData(const char *key, const char *val) override {
    auto strKey = Utf8ToUtf16(key);
    auto strValue = Utf8ToUtf16(val);
    WerRegisterCustomMetadata(strKey.c_str(), strValue.c_str());
  }

  void removeCustomData(const char *key) override {
    auto strKey = Utf8ToUtf16(key);
    WerUnregisterCustomMetadata(strKey.c_str());
  }

  void setContextualCustomData(const char *key, const char *val) override {
    std::wstringstream sstream;
    sstream << "TID" << std::this_thread::get_id() << Utf8ToUtf16(key);

    auto strKey = sstream.str();
    // WER expects valid XML element names, Hermes embeds ':' characters that
    // need to be replaced
    std::replace(strKey.begin(), strKey.end(), L':', L'_');

    auto strValue = Utf8ToUtf16(val);
    WerRegisterCustomMetadata(strKey.c_str(), strValue.c_str());
  }

  void removeContextualCustomData(const char *key) override {
    std::wstringstream sstream;
    sstream << "TID" << std::this_thread::get_id() << Utf8ToUtf16(key);

    auto strKey = sstream.str();
    // WER expects valid XML element names, Hermes embeds ':' characters that
    // need to be replaced
    std::replace(strKey.begin(), strKey.end(), L':', L'_');

    WerUnregisterCustomMetadata(strKey.c_str());
  }

  CallbackKey registerCallback(CallbackFunc cb) override {
    CallbackKey key = static_cast<CallbackKey>((intptr_t)std::addressof(cb));
    _callbacks.insert({key, std::move(cb)});
    return key;
  }

  void unregisterCallback(CallbackKey key) override {
    _callbacks.erase(static_cast<size_t>(key));
  }

  void setHeapInfo(const HeapInformation &heapInfo) override {
    _lastHeapInformation = heapInfo;
  }

  void crashHandler(int fd) const noexcept {
    for (const auto &cb : _callbacks) {
      cb.second(fd);
    }
  }

 private:
  std::wstring Utf8ToUtf16(const char *s) {
    size_t strLength = strnlen_s(
        s, 64); // 64 is maximum key length for WerRegisterCustomMetadata
    size_t requiredSize = 0;

    if (strLength != 0) {
      mbstowcs_s(&requiredSize, nullptr, 0, s, strLength);

      if (requiredSize != 0) {
        std::wstring buffer;
        buffer.resize(requiredSize + sizeof(wchar_t));

        if (mbstowcs_s(&requiredSize, &buffer[0], requiredSize, s, strLength) ==
            0) {
          return buffer;
        }
      }
    }

    return std::wstring();
  }

  HeapInformation _lastHeapInformation;
  std::map<CallbackKey, CallbackFunc> _callbacks;
  std::map<intptr_t, size_t> _largeMemBlocks;
};

void hermesCrashHandler(HermesRuntime &runtime, int fd) {
  ::hermes::vm::Runtime *vmRuntime = getVMRuntime(runtime);

  // Run all callbacks registered to the crash manager
  auto &crashManager = vmRuntime->getCrashManager();
  if (auto *crashManagerImpl =
          dynamic_cast<CrashManagerImpl *>(&crashManager)) {
    crashManagerImpl->crashHandler(fd);
  }

  // Also serialize the current callstack
  auto callstack = vmRuntime->getCallStackNoAlloc();
  llvh::raw_fd_ostream jsonStream(fd, false);
  ::hermes::JSONEmitter json(jsonStream);
  json.openDict();
  json.emitKeyValue("callstack", callstack);
  json.closeDict();
  json.endJSONL();
}

class Task : public ::hermes::node_api::Task {
 public:
  static void run(void *task) {
    reinterpret_cast<Task *>(task)->invoke();
  }

  static void deleteTask(void *task, void * /*deleterData*/) {
    delete reinterpret_cast<Task *>(task);
  }
};

template <typename TLambda>
class LambdaTask : public Task {
 public:
  LambdaTask(TLambda &&lambda) : lambda_(std::move(lambda)) {}

  void invoke() noexcept override {
    lambda_();
  }

 private:
  TLambda lambda_;
};

class TaskRunner : public ::hermes::node_api::TaskRunner {
 public:
  TaskRunner(
      void *data,
      jsr_task_runner_post_task_cb postTaskCallback,
      jsr_data_delete_cb deleteCallback,
      void *deleterData)
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
  void *data_;
  jsr_task_runner_post_task_cb postTaskCallback_;
  jsr_data_delete_cb deleteCallback_;
  void *deleterData_;
};

class ScriptBuffer : public facebook::jsi::Buffer {
 public:
  ScriptBuffer(
      const uint8_t *data,
      size_t size,
      jsr_data_delete_cb deleteCallback,
      void *deleterData)
      : data_(data),
        size_(size),
        deleteCallback_(deleteCallback),
        deleterData_(deleterData) {}

  ~ScriptBuffer() {
    if (deleteCallback_ != nullptr) {
      deleteCallback_(const_cast<uint8_t *>(data_), deleterData_);
    }
  }

  const uint8_t *data() const override {
    return data_;
  }

  size_t size() const override {
    return size_;
  }

  static void deleteBuffer(void * /*data*/, void *scriptBuffer) {
    delete reinterpret_cast<ScriptBuffer *>(scriptBuffer);
  }

 private:
  const uint8_t *data_{};
  size_t size_{};
  jsr_data_delete_cb deleteCallback_{};
  void *deleterData_{};
};

class ScriptCache : public facebook::jsi::PreparedScriptStore {
 public:
  ScriptCache(
      void *data,
      jsr_script_cache_load_cb loadCallback,
      jsr_script_cache_store_cb storeCallback,
      jsr_data_delete_cb deleteCallback,
      void *deleterData)
      : data_(data),
        loadCallback_(loadCallback),
        storeCallback_(storeCallback),
        deleteCallback_(deleteCallback),
        deleterData_(deleterData) {}

  ~ScriptCache() {
    if (deleteCallback_ != nullptr) {
      deleteCallback_(data_, deleterData_);
    }
  }

  std::shared_ptr<const facebook::jsi::Buffer> tryGetPreparedScript(
      const facebook::jsi::ScriptSignature &scriptSignature,
      const facebook::jsi::JSRuntimeSignature &runtimeMetadata,
      const char *prepareTag) noexcept override {
    const uint8_t *buffer{};
    size_t bufferSize{};
    jsr_data_delete_cb bufferDeleteCallback{};
    void *bufferDeleterData{};
    loadCallback_(
        data_,
        scriptSignature.url.c_str(),
        scriptSignature.version,
        runtimeMetadata.runtimeName.c_str(),
        runtimeMetadata.version,
        prepareTag,
        &buffer,
        &bufferSize,
        &bufferDeleteCallback,
        &bufferDeleterData);
    return std::make_shared<ScriptBuffer>(
        buffer, bufferSize, bufferDeleteCallback, bufferDeleterData);
  }

  void persistPreparedScript(
      std::shared_ptr<const facebook::jsi::Buffer> preparedScript,
      const facebook::jsi::ScriptSignature &scriptSignature,
      const facebook::jsi::JSRuntimeSignature &runtimeMetadata,
      const char *prepareTag) noexcept override {
    storeCallback_(
        data_,
        scriptSignature.url.c_str(),
        scriptSignature.version,
        runtimeMetadata.runtimeName.c_str(),
        runtimeMetadata.version,
        prepareTag,
        preparedScript->data(),
        preparedScript->size(),
        [](void * /*data*/, void *deleterData) {
          delete reinterpret_cast<
              std::shared_ptr<const facebook::jsi::Buffer> *>(deleterData);
        },
        new std::shared_ptr<const facebook::jsi::Buffer>(preparedScript));
  }

 private:
  void *data_{};
  jsr_script_cache_load_cb loadCallback_{};
  jsr_script_cache_store_cb storeCallback_{};
  jsr_data_delete_cb deleteCallback_{};
  void *deleterData_{};
};

class ConfigWrapper {
 public:
  napi_status enableDefaultCrashHandler(bool value) {
    enableDefaultCrashHandler_ = value;
    return napi_status::napi_ok;
  }

  napi_status setIntlProvider(uint8_t mode, const void *vtable) {
    intlProviderMode_ = mode;
    intlIcuVtable_ = vtable;
    return napi_status::napi_ok;
  }

  napi_status enableInspector(bool value) {
    enableInspector_ = value;
    return napi_status::napi_ok;
  }

  napi_status setInspectorRuntimeName(std::string name) {
    inspectorRuntimeName_ = std::move(name);
    return napi_status::napi_ok;
  }

  napi_status setInspectorPort(uint16_t port) {
    inspectorPort_ = port;
    return napi_status::napi_ok;
  }

  napi_status setInspectorBreakOnStart(bool value) {
    inspectorBreakOnStart_ = value;
    return napi_status::napi_ok;
  }

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

  napi_status setScriptCache(std::unique_ptr<ScriptCache> scriptCache) {
    scriptCache_ = std::move(scriptCache);
    return napi_status::napi_ok;
  }

  bool enableDefaultCrashHandler() {
    return enableDefaultCrashHandler_;
  }

  bool enableInspector() const {
    return enableInspector_;
  }

  const std::string &inspectorRuntimeName() const {
    return inspectorRuntimeName_;
  }

  uint16_t inspectorPort() {
    return inspectorPort_;
  }

  bool inspectorBreakOnStart() {
    return inspectorBreakOnStart_;
  }

  const std::shared_ptr<TaskRunner> &taskRunner() const {
    return taskRunner_;
  }

  const std::shared_ptr<ScriptCache> &scriptCache() const {
    return scriptCache_;
  }

  const std::function<void(napi_env, napi_value)> &unhandledErrorCallback()
      const {
    return unhandledErrorCallback_;
  }

  ::hermes::vm::RuntimeConfig getRuntimeConfig() const {
    ::hermes::vm::RuntimeConfig::Builder config;
    if (enableDefaultCrashHandler_) {
      auto crashManager = std::make_shared<CrashManagerImpl>();
      config.withCrashMgr(crashManager);
    }
    config.withMicrotaskQueue(explicitMicrotasks_);
    config.withIntlProviderMode(intlProviderMode_);
    config.withIntlIcuVtable(intlIcuVtable_);
    return config.build();
  }

 private:
  bool enableDefaultCrashHandler_{};
  bool enableInspector_{};
  std::string inspectorRuntimeName_;
  uint16_t inspectorPort_{};
  bool inspectorBreakOnStart_{};
  uint8_t intlProviderMode_{0};
  const void *intlIcuVtable_{nullptr};
  bool explicitMicrotasks_{};
  std::function<void(napi_env env, napi_value value)> unhandledErrorCallback_{};
  std::shared_ptr<TaskRunner> taskRunner_;
  std::shared_ptr<ScriptCache> scriptCache_;
};

class HermesRuntime;

class HermesExecutorRuntimeAdapter final
    : public facebook::hermes::inspector::RuntimeAdapter {
 public:
  HermesExecutorRuntimeAdapter(
      std::shared_ptr<facebook::hermes::HermesRuntime> hermesRuntime,
      std::shared_ptr<TaskRunner> taskRunner);

  virtual ~HermesExecutorRuntimeAdapter() = default;
  HermesRuntime &getRuntime() override;
  void tickleJs() override;

 private:
  std::shared_ptr<facebook::hermes::HermesRuntime> hermesJsiRuntime_;
  std::shared_ptr<TaskRunner> taskRunner_;
};

// An implementation of PreparedJavaScript that wraps a BytecodeProvider.
class NodeApiScriptModel final {
 public:
  explicit NodeApiScriptModel(
      std::shared_ptr<const ::hermes::Buffer> bytecodeBuffer,
      std::shared_ptr<const ::hermes::Buffer> sourceBuffer,
      std::string sourceURL)
      : bytecodeBuffer_(std::move(bytecodeBuffer)),
        sourceBuffer_(std::move(sourceBuffer)),
        sourceURL_(std::move(sourceURL)) {}

  const std::shared_ptr<const ::hermes::Buffer> &bytecodeBuffer() const {
    return bytecodeBuffer_;
  }

  const std::shared_ptr<const ::hermes::Buffer> &sourceBuffer() const {
    return sourceBuffer_;
  }

  const std::string &sourceURL() const {
    return sourceURL_;
  }

 private:
  std::shared_ptr<const ::hermes::Buffer> bytecodeBuffer_;
  std::shared_ptr<const ::hermes::Buffer> sourceBuffer_;
  std::string sourceURL_;
};

// Wraps script data as hermes::Buffer
class ScriptDataBuffer final : public ::hermes::Buffer {
 public:
  ScriptDataBuffer(
      const uint8_t *scriptData,
      size_t scriptLength,
      jsr_data_delete_cb scriptDeleteCallback,
      void *deleterData) noexcept
      : Buffer(scriptData, scriptLength),
        scriptDeleteCallback_(scriptDeleteCallback),
        deleterData_(deleterData) {}

  ~ScriptDataBuffer() noexcept override {
    if (scriptDeleteCallback_ != nullptr) {
      scriptDeleteCallback_(const_cast<uint8_t *>(data()), deleterData_);
    }
  }

  ScriptDataBuffer(const ScriptDataBuffer &) = delete;
  ScriptDataBuffer &operator=(const ScriptDataBuffer &) = delete;

 private:
  jsr_data_delete_cb scriptDeleteCallback_{};
  void *deleterData_{};
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

  size_t size() const override {
    return data_.size();
  }

  const uint8_t *data() const override {
    return reinterpret_cast<const uint8_t *>(data_.data());
  }

 private:
  llvh::SmallVector<char, 0> data_;
};

namespace {

class StringBuffer : public ::hermes::Buffer {
 public:
  StringBuffer(std::string s) : s_(std::move(s)) {
    data_ = reinterpret_cast<const uint8_t *>(s_.data());
    size_ = s_.size();
  }

 private:
  std::string s_;
};

/// A class which adapts a jsi buffer to a Hermes buffer.
/// It also provides the ability to create a partial "view" into the buffer.
class BufferAdapter final : public ::hermes::Buffer {
 public:
  explicit BufferAdapter(
      const std::shared_ptr<const ::hermes::Buffer> &buf,
      const uint8_t *data,
      size_t size)
      : buf_(buf) {
    data_ = data;
    size_ = size;
  }

  explicit BufferAdapter(const std::shared_ptr<const ::hermes::Buffer> &buf)
      : BufferAdapter(buf, buf->data(), buf->size()) {}

 private:
  /// The buffer we are "adapting".
  std::shared_ptr<const ::hermes::Buffer> buf_;
};

/// If the buffer contains an embedded terminating zero, shrink it, so it is
/// one past the size, as per the LLVM MemoryBuffer convention. Otherwise, copy
/// it into a new zero-terminated buffer.
std::unique_ptr<BufferAdapter> ensureZeroTerminated(
    const std::shared_ptr<::hermes::Buffer> &buf) {
  size_t size = buf->size();
  const uint8_t *data = buf->data();

  // Check for zero termination
  if (size != 0 && data[size - 1] == 0) {
    return std::make_unique<BufferAdapter>(buf, data, size - 1);
  } else {
    // Copy into a zero-terminated instance.
    return std::make_unique<BufferAdapter>(
        std::make_shared<StringBuffer>(std::string((const char *)data, size)));
  }
}

facebook::hermes::IHermesRootAPI *getHermesRootAPI() {
  // The makeHermesRootAPI returns a singleton.
  return facebook::jsi::castInterface<facebook::hermes::IHermesRootAPI>(
      facebook::hermes::makeHermesRootAPI());
}

} // namespace

class RuntimeWrapper {
 public:
  explicit RuntimeWrapper(const ConfigWrapper &config)
      : hermesJsiRuntime_(makeHermesRuntime(config.getRuntimeConfig())),
        hermesVMRuntime_(*getVMRuntime(*hermesJsiRuntime_)),
        isInspectable_(config.enableInspector()),
        scriptCache_(config.scriptCache()) {
    if (isInspectable_) {
      compileFlags_.debug = true;
    }
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

    if (react::g_isOldInspectorInitialized && config.enableInspector()) {
      auto adapter = std::make_unique<HermesExecutorRuntimeAdapter>(
          hermesJsiRuntime_, config.taskRunner());
      std::string inspectorRuntimeName = config.inspectorRuntimeName();
      if (inspectorRuntimeName.empty()) {
        inspectorRuntimeName = "Hermes";
      }
      debugSessionToken_ = facebook::hermes::inspector::chrome::enableDebugging(
          std::move(adapter), inspectorRuntimeName);
    }
  }

  ~RuntimeWrapper() {
    if (debugSessionToken_ != 0) {
      facebook::hermes::inspector::chrome::disableDebugging(debugSessionToken_);
    }
  }

  static facebook::hermes::RuntimeWrapper *from(napi_env env) {
    if (env == nullptr) {
      return nullptr;
    }

    void *data{};
    ::hermes::node_api::getNodeApiEnvironmentData(
        env, kRuntimeWrapperTag, &data);
    return reinterpret_cast<facebook::hermes::RuntimeWrapper *>(data);
  }

  napi_status dumpCrashData(int32_t fd) {
    hermesCrashHandler(*hermesJsiRuntime_, fd);
    return napi_ok;
  }

  napi_status addToProfiler() {
    hermesJsiRuntime_->registerForProfiling();
    return napi_ok;
  }

  napi_status removeFromProfiler() {
    hermesJsiRuntime_->unregisterForProfiling();
    return napi_ok;
  }

  napi_status getNodeApi(napi_env *env) {
    *env = env_;
    return napi_ok;
  }

  napi_status getDescription(const char **result) noexcept {
    CHECK_ARG(result);
    *result = "Hermes";
    return napi_ok;
  }

  napi_status isInspectable(bool *result) noexcept {
    CHECK_ARG(result);
    *result = isInspectable_;
    return napi_ok;
  }

  napi_status drainMicrotasks(int32_t maxCountHint, bool *result) noexcept {
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
  napi_status runScript(
      napi_value source,
      const char *sourceURL,
      napi_value *result) noexcept {
    CHECK_ARG(source);
    size_t sourceSize{};
    CHECK_STATUS(
        napi_get_value_string_utf8(env_, source, nullptr, 0, &sourceSize));
    std::unique_ptr<char[]> buffer =
        std::unique_ptr<char[]>(new char[sourceSize + 1]);
    CHECK_STATUS(napi_get_value_string_utf8(
        env_, source, buffer.get(), sourceSize + 1, nullptr));

    jsr_prepared_script preparedScript{};
    CHECK_STATUS(createPreparedScript(
        reinterpret_cast<uint8_t *>(buffer.release()),
        sourceSize,
        [](void *data, void * /*deleterData*/) {
          std::unique_ptr<char[]> buf(reinterpret_cast<char *>(data));
        },
        nullptr,
        sourceURL,
        &preparedScript));
    // To delete prepared script after execution.
    std::unique_ptr<NodeApiScriptModel> scriptModel{
        reinterpret_cast<NodeApiScriptModel *>(preparedScript)};

    return runPreparedScript(preparedScript, result);
  }

  /// Compiles source to bytecode with optional cache lookup/persist.
  /// On success, sets outBcProvider (always) and optionally
  /// outBytecodeBuffer (when bytecode can be serialized).
  /// outBytecodeBuffer is null when lazy functions prevent serialization.
  napi_status compileSourceWithCache(
      const std::shared_ptr<::hermes::Buffer> &sourceBuffer,
      const std::string &sourceURL,
      const ::hermes::hbc::CompileFlags &cflags,
      std::unique_ptr<::hermes::hbc::BCProvider> &outBcProvider,
      std::shared_ptr<const ::hermes::Buffer> &outBytecodeBuffer) noexcept {
    // Build cache key if cache is available and not debugging.
    facebook::jsi::ScriptSignature scriptSignature;
    facebook::jsi::JSRuntimeSignature runtimeSignature;
    const char *prepareTag = "perf";
    bool useCache = scriptCache_ && !cflags.debug;

    if (useCache) {
      uint64_t hash{};
      murmurhash(sourceBuffer->data(), sourceBuffer->size(), hash);
      scriptSignature = {sourceURL, hash};
      runtimeSignature = {"Hermes", HermesBuildVersion.version};
    }

    // Try loading from cache.
    if (useCache) {
      auto cached = scriptCache_->tryGetPreparedScript(
          scriptSignature, runtimeSignature, prepareTag);
      if (cached && cached->size() > 0) {
        // Validate cached bytecode is loadable.
        auto cachedHermesBuf = std::make_shared<JsiBuffer>(cached);
        auto bcCheck =
            ::hermes::hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
                std::make_unique<BufferAdapter>(cachedHermesBuf));
        if (bcCheck.first) {
          // Cache hit.
          outBcProvider = std::move(bcCheck.first);
          outBytecodeBuffer = std::move(cachedHermesBuf);
          return ::hermes::node_api::clearLastNativeError(env_);
        }
        // Invalid cache entry; fall through to compile from source.
      }
    }

    // Compile from source.
    llvh::StringRef sourceMap;
    auto bcErr = ::hermes::hbc::createBCProviderFromSrc(
        ensureZeroTerminated(sourceBuffer), sourceURL, sourceMap, cflags);

    if (!bcErr.first) {
      return GENERIC_FAILURE(
          "Compiling JS failed: " + std::move(bcErr.second) +
          ", sourceURL: " + sourceURL);
    }

    outBcProvider = std::move(bcErr.first);

    // Check for lazy functions - serialization cannot handle them.
    for (uint32_t i = 0, e = outBcProvider->getFunctionCount(); i < e; ++i) {
      if (outBcProvider->isFunctionLazy(i)) {
        // Cannot serialize; return with outBytecodeBuffer = null.
        return ::hermes::node_api::clearLastNativeError(env_);
      }
    }

    // Serialize the compiled bytecode.
    llvh::SmallVector<char, 0> bytecodeVec;
    llvh::raw_svector_ostream bcStream(bytecodeVec);
    ::hermes::BytecodeGenerationOptions opts(::hermes::EmitBundle);
    ::hermes::hbc::serializeBytecodeModule(
        *outBcProvider->getBytecodeModule(), {}, bcStream, opts);

    outBytecodeBuffer = std::make_shared<StringBuffer>(
        std::string(bytecodeVec.data(), bytecodeVec.size()));

    // Persist to cache.
    if (useCache) {
      scriptCache_->persistPreparedScript(
          std::make_shared<JsiSmallVectorBuffer>(std::move(bytecodeVec)),
          scriptSignature,
          runtimeSignature,
          prepareTag);
    }

    return ::hermes::node_api::clearLastNativeError(env_);
  }

  napi_status runScriptBuffer(
      const uint8_t *scriptData,
      size_t scriptLength,
      jsr_data_delete_cb scriptDeleteCallback,
      void *deleterData,
      const char *sourceURL,
      napi_value *result) noexcept {
    std::shared_ptr<ScriptDataBuffer> buffer =
        std::make_shared<ScriptDataBuffer>(
            scriptData, scriptLength, scriptDeleteCallback, deleterData);
    std::string sourceURLStr = sourceURL ? sourceURL : "";

    facebook::hermes::IHermesRootAPI *api = getHermesRootAPI();
    bool isBytecode = api->isHermesBytecode(buffer->data(), buffer->size());

    // Construct the BC provider either from buffer or source.
    std::unique_ptr<::hermes::hbc::BCProvider> bcProvider;
    ::hermes::vm::RuntimeModuleFlags runtimeFlags{};
    if (isBytecode) {
      auto bcErr =
          ::hermes::hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
              std::make_unique<BufferAdapter>(buffer));
      if (!bcErr.first) {
        return GENERIC_FAILURE(
            "Compiling JS failed: " + std::move(bcErr.second) +
            ", sourceURL: " + sourceURLStr);
      }
      bcProvider = std::move(bcErr.first);
      runtimeFlags.persistent = true;
    } else {
      std::shared_ptr<const ::hermes::Buffer> bytecodeBuffer;
      CHECK_STATUS(compileSourceWithCache(
          buffer, sourceURLStr, compileFlags_, bcProvider, bytecodeBuffer));
      runtimeFlags.persistent = bcProvider->allowPersistent();
    }

    return ::hermes::node_api::runInNodeApiContext(
        env_,
        [&]() {
          return hermesVMRuntime_.runBytecode(
              std::move(bcProvider),
              runtimeFlags,
              sourceURL,
              ::hermes::vm::Runtime::makeNullHandle<
                  ::hermes::vm::Environment>());
        },
        result);
  }

  napi_status createPreparedScript(
      const uint8_t *scriptData,
      size_t scriptLength,
      jsr_data_delete_cb scriptDeleteCallback,
      void *deleterData,
      const char *sourceURL,
      jsr_prepared_script *result) noexcept {
    std::shared_ptr<ScriptDataBuffer> buffer =
        std::make_shared<ScriptDataBuffer>(
            scriptData, scriptLength, scriptDeleteCallback, deleterData);
    std::string sourceURLStr = sourceURL ? sourceURL : "";

    facebook::hermes::IHermesRootAPI *api = getHermesRootAPI();
    bool isBytecode = api->isHermesBytecode(buffer->data(), buffer->size());

    // If input is already bytecode, store it directly.
    if (isBytecode) {
      *result = reinterpret_cast<jsr_prepared_script>(
          new NodeApiScriptModel(std::move(buffer), nullptr, sourceURLStr));
      return ::hermes::node_api::clearLastNativeError(env_);
    }

    // Check if we should eagerly compile or defer (lazy compilation).
    bool shouldEagerlyCompile = !compileFlags_.lazy ||
        buffer->size() < compileFlags_.preemptiveFileCompilationThreshold;

    if (shouldEagerlyCompile) {
      // Eager compilation with cache support.
      ::hermes::hbc::CompileFlags cflags = compileFlags_;
      cflags.lazy = false;
      std::unique_ptr<::hermes::hbc::BCProvider> bcProvider;
      std::shared_ptr<const ::hermes::Buffer> bytecodeBuffer;
      CHECK_STATUS(compileSourceWithCache(
          buffer, sourceURLStr, cflags, bcProvider, bytecodeBuffer));

      // bytecodeBuffer is always set for eager (no lazy functions possible).
      *result = reinterpret_cast<jsr_prepared_script>(new NodeApiScriptModel(
          std::move(bytecodeBuffer), nullptr, sourceURLStr));
      return ::hermes::node_api::clearLastNativeError(env_);
    }

    // Lazy path: store source only, defer compilation.
    *result = reinterpret_cast<jsr_prepared_script>(new NodeApiScriptModel(
        nullptr, // bytecodeBuffer
        std::move(buffer), // sourceBuffer
        sourceURLStr));
    return ::hermes::node_api::clearLastNativeError(env_);
  }

  napi_status deletePreparedScript(
      jsr_prepared_script preparedScript) noexcept {
    CHECK_ARG(preparedScript);
    delete reinterpret_cast<NodeApiScriptModel *>(preparedScript);
    return ::hermes::node_api::clearLastNativeError(env_);
  }

  napi_status runPreparedScript(
      jsr_prepared_script preparedScript,
      napi_value *result) noexcept {
    CHECK_ARG(preparedScript);
    const NodeApiScriptModel *hermesPrep =
        reinterpret_cast<NodeApiScriptModel *>(preparedScript);

    // Check which path: bytecode or source
    if (hermesPrep->bytecodeBuffer()) {
      // Bytecode path: create BCProvider and execute
      std::pair<std::unique_ptr<::hermes::hbc::BCProvider>, std::string> bcErr =
          ::hermes::hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
              std::make_unique<BufferAdapter>(hermesPrep->bytecodeBuffer()));

      if (!bcErr.first) {
        return GENERIC_FAILURE(
            "Failed to create BCProvider from bytecode: " + bcErr.second);
      }

      ::hermes::vm::RuntimeModuleFlags runtimeFlags{};
      runtimeFlags.persistent = true;
      return ::hermes::node_api::runInNodeApiContext(
          env_,
          [&]() {
            return hermesVMRuntime_.runBytecode(
                std::move(bcErr.first),
                runtimeFlags,
                hermesPrep->sourceURL(),
                ::hermes::vm::Runtime::makeNullHandle<
                    ::hermes::vm::Environment>());
          },
          result);
    } else {
      // Source path: delegate to evaluateJavaScript
      // Each runtime compiles independently, avoiding shared mutable state
      return runScriptBuffer(
          hermesPrep->sourceBuffer()->data(),
          hermesPrep->sourceBuffer()->size(),
          [](void * /*data*/, void *deleterData) {
            std::shared_ptr<const ::hermes::Buffer> buf =
                *reinterpret_cast<std::shared_ptr<const ::hermes::Buffer> *>(
                    deleterData);
            // Let the shared_ptr go out of scope to delete the buffer if needed
          },
          new std::shared_ptr<const ::hermes::Buffer>(
              hermesPrep->sourceBuffer()),
          hermesPrep->sourceURL().c_str(),
          result);
    }
  }

  napi_status initializeNativeModule(
      napi_addon_register_func register_module,
      int32_t api_version,
      napi_value *exports) noexcept {
    return ::hermes::node_api::initializeNodeApiModule(
        hermesVMRuntime_, register_module, api_version, exports);
  }

  napi_status collectGarbage() noexcept {
    hermesVMRuntime_.collect("test");
    return napi_ok;
  }

 public:
  HermesRuntime &getHermesRuntime() {
    return *hermesJsiRuntime_;
  }

 private:
  std::shared_ptr<HermesRuntime> hermesJsiRuntime_;
  ::hermes::vm::Runtime &hermesVMRuntime_;
  napi_env env_;

  // Can we run a debugger?
  bool isInspectable_{};

  // Optional prepared script store.
  std::shared_ptr<facebook::jsi::PreparedScriptStore> scriptCache_{};

  // Flags used by byte code compiler.
  ::hermes::hbc::CompileFlags compileFlags_{};

  facebook::hermes::inspector::chrome::DebugSessionToken debugSessionToken_{};

  static constexpr napi_type_tag kRuntimeWrapperTag{
      0xfa327a491b4b4d20,
      0x94407c81c2d4e8f2};
};

HermesExecutorRuntimeAdapter::HermesExecutorRuntimeAdapter(
    std::shared_ptr<facebook::hermes::HermesRuntime> hermesRuntime,
    std::shared_ptr<TaskRunner> taskRunner)
    : hermesJsiRuntime_(std::move(hermesRuntime)),
      taskRunner_(std::move(taskRunner)) {}

HermesRuntime &HermesExecutorRuntimeAdapter::getRuntime() {
  return *hermesJsiRuntime_;
}

void HermesExecutorRuntimeAdapter::tickleJs() {
  // The queue will ensure that hermesJsiRuntime_ is still valid when this
  // gets invoked.
  taskRunner_->post(
      std::unique_ptr<Task>(new LambdaTask([&runtime = *hermesJsiRuntime_]() {
        auto func =
            runtime.global().getPropertyAsFunction(runtime, "__tickleJs");
        func.call(runtime);
      })));
}

} // namespace facebook::hermes

JSR_API
jsr_create_runtime(jsr_config config, jsr_runtime *runtime) {
  CHECK_ARG(config);
  CHECK_ARG(runtime);
  *runtime = reinterpret_cast<jsr_runtime>(new facebook::hermes::RuntimeWrapper(
      *reinterpret_cast<facebook::hermes::ConfigWrapper *>(config)));
  return napi_ok;
}

JSR_API jsr_delete_runtime(jsr_runtime runtime) {
  CHECK_ARG(runtime);
  delete reinterpret_cast<facebook::hermes::RuntimeWrapper *>(runtime);
  return napi_ok;
}

JSR_API jsr_runtime_get_node_api_env(jsr_runtime runtime, napi_env *env) {
  return CHECKED_RUNTIME(runtime)->getNodeApi(env);
}

JSR_API hermes_dump_crash_data(jsr_runtime runtime, int32_t fd) {
  return CHECKED_RUNTIME(runtime)->dumpCrashData(fd);
}

JSR_API hermes_sampling_profiler_enable() {
  facebook::hermes::getHermesRootAPI()->enableSamplingProfiler();
  return napi_ok;
}

JSR_API hermes_sampling_profiler_disable() {
  facebook::hermes::getHermesRootAPI()->disableSamplingProfiler();
  return napi_ok;
}

JSR_API hermes_sampling_profiler_add(jsr_runtime runtime) {
  return CHECKED_RUNTIME(runtime)->addToProfiler();
}

JSR_API hermes_sampling_profiler_remove(jsr_runtime runtime) {
  return CHECKED_RUNTIME(runtime)->removeFromProfiler();
}

JSR_API hermes_sampling_profiler_dump_to_file(const char *filename) {
  facebook::hermes::getHermesRootAPI()->dumpSampledTraceToFile(filename);
  return napi_ok;
}

JSR_API jsr_create_config(jsr_config *config) {
  CHECK_ARG(config);
  *config = reinterpret_cast<jsr_config>(new facebook::hermes::ConfigWrapper());
  return napi_ok;
}

JSR_API jsr_delete_config(jsr_config config) {
  CHECK_ARG(config);
  delete reinterpret_cast<facebook::hermes::ConfigWrapper *>(config);
  return napi_ok;
}

JSR_API hermes_config_enable_default_crash_handler(
    jsr_config config,
    bool value) {
  return CHECKED_CONFIG(config)->enableDefaultCrashHandler(value);
}

JSR_API hermes_config_set_intl_provider(
    jsr_config config,
    uint8_t mode,
    const struct hermes_icu_vtable *vtable) {
  return CHECKED_CONFIG(config)->setIntlProvider(
      mode, static_cast<const void *>(vtable));
}

JSR_API jsr_config_enable_inspector(jsr_config config, bool value) {
  return CHECKED_CONFIG(config)->enableInspector(value);
}

JSR_API jsr_config_set_inspector_runtime_name(
    jsr_config config,
    const char *name) {
  return CHECKED_CONFIG(config)->setInspectorRuntimeName(name);
}

JSR_API jsr_config_set_inspector_port(jsr_config config, uint16_t port) {
  return CHECKED_CONFIG(config)->setInspectorPort(port);
}

JSR_API jsr_config_set_inspector_break_on_start(jsr_config config, bool value) {
  return CHECKED_CONFIG(config)->setInspectorBreakOnStart(value);
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
    void *task_runner_data,
    jsr_task_runner_post_task_cb task_runner_post_task_cb,
    jsr_data_delete_cb task_runner_data_delete_cb,
    void *deleter_data) {
  return CHECKED_CONFIG(config)->setTaskRunner(
      std::make_unique<facebook::hermes::TaskRunner>(
          task_runner_data,
          task_runner_post_task_cb,
          task_runner_data_delete_cb,
          deleter_data));
}

JSR_API jsr_config_on_unhandled_error(
    jsr_config config,
    void *cb_data,
    jsr_unhandled_error_cb unhandled_error_cb) {
  return CHECKED_CONFIG(config)->setUnhandledErrorCallback(
      [cb_data, unhandled_error_cb](napi_env env, napi_value error) {
        unhandled_error_cb(cb_data, env, error);
      });
}

JSR_API jsr_config_set_script_cache(
    jsr_config config,
    void *script_cache_data,
    jsr_script_cache_load_cb script_cache_load_cb,
    jsr_script_cache_store_cb script_cache_store_cb,
    jsr_data_delete_cb script_cache_data_delete_cb,
    void *deleter_data) {
  return CHECKED_CONFIG(config)->setScriptCache(
      std::make_unique<facebook::hermes::ScriptCache>(
          script_cache_data,
          script_cache_load_cb,
          script_cache_store_cb,
          script_cache_data_delete_cb,
          deleter_data));
}

//=============================================================================
// Node-API extensions to host JS engine and to implement JSI
//=============================================================================

JSR_API jsr_collect_garbage(napi_env env) {
  return CHECKED_ENV_RUNTIME(env)->collectGarbage();
}

JSR_API
jsr_has_unhandled_promise_rejection(napi_env /*env*/, bool * /*result*/) {
  return napi_generic_failure;
}

JSR_API jsr_get_and_clear_last_unhandled_promise_rejection(
    napi_env /*env*/,
    napi_value * /*result*/) {
  return napi_generic_failure;
}

JSR_API jsr_get_description(napi_env env, const char **result) {
  return CHECKED_ENV_RUNTIME(env)->getDescription(result);
}

JSR_API jsr_queue_microtask(napi_env env, napi_value callback) {
  return hermes::node_api::queueMicrotask(env, callback);
}

JSR_API
jsr_drain_microtasks(napi_env env, int32_t max_count_hint, bool *result) {
  return CHECKED_ENV_RUNTIME(env)->drainMicrotasks(max_count_hint, result);
}

JSR_API jsr_is_inspectable(napi_env env, bool *result) {
  return CHECKED_ENV_RUNTIME(env)->isInspectable(result);
}

JSR_API jsr_open_napi_env_scope(napi_env env, jsr_napi_env_scope *scope) {
  return hermes::node_api::openNodeApiScope(
      env, reinterpret_cast<void **>(scope));
}

JSR_API jsr_close_napi_env_scope(napi_env env, jsr_napi_env_scope scope) {
  return hermes::node_api::closeNodeApiScope(
      env, reinterpret_cast<void *>(scope));
}

//-----------------------------------------------------------------------------
// Script preparing and running.
//
// Script is usually converted to byte code, or in other words - prepared - for
// execution. Then, we can run the prepared script.
//-----------------------------------------------------------------------------

JSR_API jsr_run_script(
    napi_env env,
    napi_value source,
    const char *source_url,
    napi_value *result) {
  return CHECKED_ENV_RUNTIME(env)->runScript(source, source_url, result);
}

JSR_API jsr_run_script_buffer(
    napi_env env,
    const uint8_t *script_data,
    size_t script_length,
    jsr_data_delete_cb script_delete_cb,
    void *deleter_data,
    const char *source_url,
    napi_value *result) {
  return CHECKED_ENV_RUNTIME(env)->runScriptBuffer(
      script_data,
      script_length,
      script_delete_cb,
      deleter_data,
      source_url,
      result);
}

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

JSR_API jsr_initialize_native_module(
    napi_env env,
    napi_addon_register_func register_module,
    int32_t api_version,
    napi_value *exports) {
  return CHECKED_ENV_RUNTIME(env)->initializeNativeModule(
      register_module, api_version, exports);
}

//-----------------------------------------------------------------------------
// Modern inspector API implementation.
//-----------------------------------------------------------------------------

using namespace facebook::hermes;

namespace {

template <typename TLambda, typename TFunctor>
struct FunctorAdapter {
  static_assert(sizeof(TLambda) == -1, "Unsupported signature");
};

template <typename TLambda, typename TResult, typename... TArgs>
struct FunctorAdapter<TLambda, TResult(void *, TArgs...)> {
  static TResult invoke(void *data, TArgs... args) {
    return reinterpret_cast<TLambda *>(data)->operator()(args...);
  }
};

template <typename TFunctor, typename TLambda>
inline TFunctor toFunctor(TLambda &&lambda) {
  using TLambdaType = std::remove_reference_t<TLambda>;
  using TAdapter = FunctorAdapter<
      TLambdaType,
      std::remove_pointer_t<
          decltype(std::remove_reference_t<TFunctor>::invoke)>>;
  return TFunctor{
      static_cast<void *>(new TLambdaType(std::forward<TLambdaType>(lambda))),
      &TAdapter::invoke,
      [](void *data) { delete static_cast<TLambdaType *>(data); }};
}

// We use shared_ptr to ensure proper release of functor data even if
// std::function is copied.
template <typename TFunctor>
std::shared_ptr<void> toSharedFuncData(const TFunctor &func) {
  return std::shared_ptr<void>(
      func.data, [release = func.release](void *data) { release(data); });
}

debugger::EnqueueRuntimeTaskFunc toEnqueueRuntimeTaskFunctor(
    const hermes_enqueue_runtime_task_functor &func) {
  return [sharedFuncData = toSharedFuncData(func),
          invoke = func.invoke](debugger::RuntimeTask task) {
    invoke(
        sharedFuncData.get(),
        toFunctor<hermes_run_runtime_task_functor>([task](
                                                       hermes_runtime runtime) {
          RuntimeWrapper *wrapper = reinterpret_cast<RuntimeWrapper *>(runtime);
          task(wrapper->getHermesRuntime());
        }));
  };
}

cdp::OutboundMessageFunc toOutboundMessageFunc(
    const hermes_enqueue_frontend_message_functor &func) {
  return [sharedFuncData = toSharedFuncData(func),
          invoke = func.invoke](const std::string &message) {
    invoke(sharedFuncData.get(), message.c_str(), message.size());
  };
}

hermes_status NAPI_CDECL
create_cdp_debug_api(hermes_runtime runtime, hermes_cdp_debug_api *result) try {
  RuntimeWrapper *wrapper = reinterpret_cast<RuntimeWrapper *>(runtime);

  std::unique_ptr<cdp::CDPDebugAPI> cdpDebugAPI =
      cdp::CDPDebugAPI::create(wrapper->getHermesRuntime());

  *result = reinterpret_cast<hermes_cdp_debug_api>(cdpDebugAPI.release());
  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL
release_cdp_debug_api(hermes_cdp_debug_api cdp_debug_api) try {
  delete reinterpret_cast<cdp::CDPDebugAPI *>(cdp_debug_api);
  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL add_console_message(
    hermes_cdp_debug_api cdp_debug_api,
    double timestamp,
    hermes_console_api_type type,
    const char *args_property_name,
    hermes_stack_trace stack_trace) try {
  cdp::CDPDebugAPI *cdpDebugAPI =
      reinterpret_cast<cdp::CDPDebugAPI *>(cdp_debug_api);
  facebook::jsi::Runtime &runtime = cdpDebugAPI->runtime();

  // Retrieve the arguments array from the global property
  facebook::jsi::Value argsValue =
      runtime.global().getProperty(runtime, args_property_name);

  // Convert jsi::Array back to std::vector<jsi::Value>
  std::vector<facebook::jsi::Value> args;
  if (argsValue.isObject()) {
    facebook::jsi::Array argsArray =
        argsValue.asObject(runtime).asArray(runtime);
    size_t length = argsArray.length(runtime);
    args.reserve(length);

    for (size_t i = 0; i < length; ++i) {
      args.push_back(argsArray.getValueAtIndex(runtime, i));
    }
  }

  // Delete the temporary property immediately
  runtime.global().setProperty(
      runtime, args_property_name, facebook::jsi::Value::undefined());

  // Convert hermes_console_api_type to ConsoleAPIType
  cdp::ConsoleAPIType consoleType;
  switch (type) {
    case hermes_console_api_type_log:
      consoleType = cdp::ConsoleAPIType::kLog;
      break;
    case hermes_console_api_type_debug:
      consoleType = cdp::ConsoleAPIType::kDebug;
      break;
    case hermes_console_api_type_info:
      consoleType = cdp::ConsoleAPIType::kInfo;
      break;
    case hermes_console_api_type_error:
      consoleType = cdp::ConsoleAPIType::kError;
      break;
    case hermes_console_api_type_warning:
      consoleType = cdp::ConsoleAPIType::kWarning;
      break;
    case hermes_console_api_type_dir:
      consoleType = cdp::ConsoleAPIType::kDir;
      break;
    case hermes_console_api_type_dir_xml:
      consoleType = cdp::ConsoleAPIType::kDirXML;
      break;
    case hermes_console_api_type_table:
      consoleType = cdp::ConsoleAPIType::kTable;
      break;
    case hermes_console_api_type_trace:
      consoleType = cdp::ConsoleAPIType::kTrace;
      break;
    case hermes_console_api_type_start_group:
      consoleType = cdp::ConsoleAPIType::kStartGroup;
      break;
    case hermes_console_api_type_start_group_collapsed:
      consoleType = cdp::ConsoleAPIType::kStartGroupCollapsed;
      break;
    case hermes_console_api_type_end_group:
      consoleType = cdp::ConsoleAPIType::kEndGroup;
      break;
    case hermes_console_api_type_clear:
      consoleType = cdp::ConsoleAPIType::kClear;
      break;
    case hermes_console_api_type_assert:
      consoleType = cdp::ConsoleAPIType::kAssert;
      break;
    case hermes_console_api_type_time_end:
      consoleType = cdp::ConsoleAPIType::kTimeEnd;
      break;
    case hermes_console_api_type_count:
      consoleType = cdp::ConsoleAPIType::kCount;
      break;
    default:
      return hermes_status_error;
  }

  // Get the stack trace if provided
  debugger::StackTrace hermesStackTrace;
  if (stack_trace != nullptr) {
    hermesStackTrace = *reinterpret_cast<debugger::StackTrace *>(stack_trace);
  }

  // Create ConsoleMessage and add to CDP
  cdp::ConsoleMessage message(
      timestamp, consoleType, std::move(args), std::move(hermesStackTrace));
  cdpDebugAPI->addConsoleMessage(std::move(message));

  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL create_cdp_agent(
    hermes_cdp_debug_api cdp_debug_api,
    int32_t execution_context_id,
    hermes_enqueue_runtime_task_functor enqueue_runtime_task_callback,
    hermes_enqueue_frontend_message_functor enqueue_frontend_message_callback,
    hermes_cdp_state cdp_state,
    hermes_cdp_agent *result) try {
  std::unique_ptr<cdp::CDPAgent> agent = cdp::CDPAgent::create(
      execution_context_id,
      *reinterpret_cast<cdp::CDPDebugAPI *>(cdp_debug_api),
      toEnqueueRuntimeTaskFunctor(enqueue_runtime_task_callback),
      toOutboundMessageFunc(enqueue_frontend_message_callback),
      cdp_state != nullptr
          ? std::move(*reinterpret_cast<cdp::State *>(cdp_state))
          : cdp::State{});

  *result = reinterpret_cast<hermes_cdp_agent>(agent.release());
  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL release_cdp_agent(hermes_cdp_agent cdp_agent) try {
  delete reinterpret_cast<cdp::CDPAgent *>(cdp_agent);
  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL
cdp_agent_get_state(hermes_cdp_agent cdp_agent, hermes_cdp_state *result) try {
  cdp::CDPAgent *cdpAgent = reinterpret_cast<cdp::CDPAgent *>(cdp_agent);

  std::unique_ptr<cdp::State> state =
      std::make_unique<cdp::State>(cdpAgent->getState());

  *result = reinterpret_cast<hermes_cdp_state>(state.release());
  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL release_cdp_state(hermes_cdp_state cdp_state) try {
  delete reinterpret_cast<cdp::State *>(cdp_state);
  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL cdp_agent_handle_command(
    hermes_cdp_agent cdp_agent,
    const char *json_utf8,
    size_t json_size) try {
  cdp::CDPAgent *cdpAgent = reinterpret_cast<cdp::CDPAgent *>(cdp_agent);

  cdpAgent->handleCommand(std::string(json_utf8, json_size));

  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL
cdp_agent_enable_runtime_domain(hermes_cdp_agent cdp_agent) try {
  cdp::CDPAgent *cdpAgent = reinterpret_cast<cdp::CDPAgent *>(cdp_agent);

  cdpAgent->enableRuntimeDomain();

  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL
cdp_agent_enable_debugger_domain(hermes_cdp_agent cdp_agent) try {
  cdp::CDPAgent *cdpAgent = reinterpret_cast<cdp::CDPAgent *>(cdp_agent);

  cdpAgent->enableDebuggerDomain();

  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL
capture_stack_trace(hermes_runtime runtime, hermes_stack_trace *result) try {
  RuntimeWrapper *wrapper = reinterpret_cast<RuntimeWrapper *>(runtime);

  std::unique_ptr<debugger::StackTrace> stackTrace =
      std::make_unique<debugger::StackTrace>(
          wrapper->getHermesRuntime().getDebugger().captureStackTrace());

  *result = reinterpret_cast<hermes_stack_trace>(stackTrace.release());
  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL
release_stack_trace(hermes_stack_trace stack_trace) try {
  delete reinterpret_cast<debugger::StackTrace *>(stack_trace);
  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL enable_sampling_profiler(hermes_runtime runtime) try {
  IHermesRootAPI *hermesAPI =
      facebook::jsi::castInterface<IHermesRootAPI>(makeHermesRootAPI());
  const uint16_t kHermesSamplingFrequencyHz = 10000;

  hermesAPI->enableSamplingProfiler(kHermesSamplingFrequencyHz);

  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL disable_sampling_profiler(hermes_runtime runtime) try {
  IHermesRootAPI *hermesAPI =
      facebook::jsi::castInterface<IHermesRootAPI>(makeHermesRootAPI());

  hermesAPI->disableSamplingProfiler();

  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL collect_sampling_profile(
    hermes_runtime runtime,
    void *cb_data,
    hermes_on_sampling_profile_info_callback on_info_callback,
    hermes_on_sampling_profile_sample_callback on_sample_callback,
    hermes_on_sampling_profile_frame_callback on_frame_callback,
    hermes_sampling_profile *result) try {
  if (runtime == nullptr || on_info_callback == nullptr ||
      on_sample_callback == nullptr || on_frame_callback == nullptr) {
    return hermes_status_error;
  }

  RuntimeWrapper *wrapper = reinterpret_cast<RuntimeWrapper *>(runtime);
  HermesRuntime &hermesRuntime = wrapper->getHermesRuntime();

  std::unique_ptr<sampling_profiler::Profile> profile =
      std::make_unique<sampling_profiler::Profile>(
          hermesRuntime.dumpSampledTraceToProfile());

  on_info_callback(cb_data, profile->getSamplesCount());

  for (const auto &sample : profile->getSamplesRange()) {
    on_sample_callback(
        cb_data,
        sample.getTimestamp(),
        sample.getThreadId(),
        sample.getCallStackFramesCount());

    for (const auto &frame : sample.getCallStackFramesRange()) {
      hermes_call_stack_frame_kind kind;
      uint32_t scriptId = 0;
      std::string_view functionName;
      std::string_view scriptUrl;
      uint32_t lineNumber = 0;
      uint32_t columnNumber = 0;

      if (std::holds_alternative<
              sampling_profiler::ProfileSampleCallStackJSFunctionFrame>(
              frame)) {
        const auto &jsFrame =
            std::get<sampling_profiler::ProfileSampleCallStackJSFunctionFrame>(
                frame);
        kind = hermes_call_stack_frame_kind_js_function;
        scriptId = jsFrame.getScriptId();
        functionName = jsFrame.getFunctionName();

        if (jsFrame.hasScriptUrl()) {
          scriptUrl = jsFrame.getScriptUrl();
        }

        if (jsFrame.hasFunctionLineNumber()) {
          lineNumber = jsFrame.getFunctionLineNumber();
        }

        if (jsFrame.hasFunctionColumnNumber()) {
          columnNumber = jsFrame.getFunctionColumnNumber();
        }
      } else if (std::holds_alternative<
                     sampling_profiler::
                         ProfileSampleCallStackNativeFunctionFrame>(frame)) {
        const auto &nativeFrame = std::get<
            sampling_profiler::ProfileSampleCallStackNativeFunctionFrame>(
            frame);
        kind = hermes_call_stack_frame_kind_native_function;
        functionName = nativeFrame.getFunctionName();
      } else if (std::holds_alternative<
                     sampling_profiler::
                         ProfileSampleCallStackHostFunctionFrame>(frame)) {
        const auto &hostFrame = std::get<
            sampling_profiler::ProfileSampleCallStackHostFunctionFrame>(frame);
        kind = hermes_call_stack_frame_kind_host_function;
        functionName = hostFrame.getFunctionName();
      } else if (std::holds_alternative<
                     sampling_profiler::ProfileSampleCallStackSuspendFrame>(
                     frame)) {
        const auto &suspendFrame =
            std::get<sampling_profiler::ProfileSampleCallStackSuspendFrame>(
                frame);

        // Only process GC suspend frames, skip debugger frames
        if (suspendFrame.getSuspendFrameKind() ==
            sampling_profiler::ProfileSampleCallStackSuspendFrame::
                SuspendFrameKind::GC) {
          kind = hermes_call_stack_frame_kind_gc;
          functionName = "(garbage collector)";
        } else {
          // Skip debugger suspend frames
          continue;
        }
      } else {
        // Unknown frame type, skip it
        continue;
      }

      on_frame_callback(
          cb_data,
          kind,
          scriptId,
          functionName.data(),
          functionName.size(),
          scriptUrl.data(),
          scriptUrl.size(),
          lineNumber,
          columnNumber);
    }
  }

  *result = reinterpret_cast<hermes_sampling_profile>(profile.release());
  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

hermes_status NAPI_CDECL
release_sampling_profile(hermes_sampling_profile profile) try {
  delete reinterpret_cast<sampling_profiler::Profile *>(profile);
  return hermes_status_ok;
} catch (...) {
  return hermes_status_error;
}

} // namespace

JSR_API hermes_get_inspector_vtable(const hermes_inspector_vtable **vtable) {
  CHECK_ARG(vtable);
  static const hermes_inspector_vtable inspector_vtable{
      nullptr, // reserved0
      nullptr, // reserved1
      nullptr, // reserved2

      create_cdp_debug_api,
      release_cdp_debug_api,

      add_console_message,

      create_cdp_agent,
      release_cdp_agent,

      cdp_agent_get_state,
      release_cdp_state,

      cdp_agent_handle_command,
      cdp_agent_enable_runtime_domain,
      cdp_agent_enable_debugger_domain,

      capture_stack_trace,
      release_stack_trace,

      enable_sampling_profiler,
      disable_sampling_profiler,
      collect_sampling_profile,
      release_sampling_profile,
  };

  *vtable = &inspector_vtable;
  return napi_ok;
}
