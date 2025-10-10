// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// A simple Node.js-like runtime that runs Node-API test scripts.

#ifndef NODE_API_TEST_NODE_LITE_H
#define NODE_API_TEST_NODE_LITE_H

#include <algorithm>
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include "compat.h"
#include "string_utils.h"

#define NAPI_EXPERIMENTAL
#include "js_runtime_api.h"

#define NODE_LITE_CALL(expr)                                                   \
  do {                                                                         \
    napi_status temp_status__ = (expr);                                        \
    if (temp_status__ != napi_status::napi_ok) {                               \
      NodeLiteErrorHandler::OnNodeApiFailed(env, temp_status__);               \
    }                                                                          \
  } while (false)

#define NODE_LITE_ASSERT(expr, ...)                                            \
  do {                                                                         \
    if (!(expr)) {                                                             \
      NodeLiteErrorHandler::OnAssertFailed(                                    \
          env, #expr, FormatString("" __VA_ARGS__).c_str());                   \
    }                                                                          \
  } while (false)

namespace node_api_tests {

// Forward declarations
class NodeLiteModule;
class NodeLiteRuntime;
class NodeLiteTaskRunner;
class NodeApiRefDeleter;
class NodeApiHandleScope;
class NodeApiEnvScope;
class NodeLiteErrorHandler;

struct IEnvHolder {
  virtual ~IEnvHolder() {}
  virtual napi_env getEnv() = 0;
};

class NodeLiteTaskRunner {
 public:
  using QueueEntry = std::pair<uint32_t, std::function<void()>>;

  uint32_t PostTask(std::function<void()>&& task) noexcept;
  void RemoveTask(uint32_t task_id) noexcept;
  void DrainTaskQueue() noexcept;

  static void PostTaskCallback(void* task_runner_data,
                               void* task_data,
                               jsr_task_run_cb task_run_cb,
                               jsr_data_delete_cb task_data_delete_cb,
                               void* deleter_data);

  static void DeleteCallback(void* data, void* /*deleter_data*/);

 private:
  std::list<QueueEntry> task_queue_;
  uint32_t next_task_id_{1};
};

class NodeLiteException : public std::runtime_error {
 public:
  explicit NodeLiteException(napi_status error_status,
                             const char* message) noexcept
      : runtime_error{message}, error_status_{error_status} {}

  napi_status error_status() const noexcept { return error_status_; }

 private:
  napi_status error_status_;
};

class NodeLiteErrorHandler {
 public:
  [[noreturn]] static void OnNodeApiFailed(napi_env env,
                                           napi_status error_status);

  [[noreturn]] static void OnAssertFailed(napi_env env,
                                          char const* expr,
                                          char const* message);

  [[noreturn]] static void ExitWithJSError(napi_env env,
                                           napi_value error) noexcept;

  [[noreturn]] static void ExitWithJSAssertError(napi_env env,
                                                 napi_value error) noexcept;

  [[noreturn]] static void ExitWithMessage(
      const std::string& message,
      std::function<void(std::ostream&)> get_error_details = nullptr) noexcept;
};

// Define NodeApiRef "smart pointer" for napi_ref as unique_ptr with a custom
// deleter.
class NodeApiRefDeleter {
 public:
  NodeApiRefDeleter() noexcept;
  explicit NodeApiRefDeleter(napi_env env) noexcept;

  void operator()(napi_ref ref) noexcept;

 private:
  napi_env env_{};
};

using NodeApiRef = std::unique_ptr<napi_ref__, NodeApiRefDeleter>;

class NodeApiHandleScope {
 public:
  explicit NodeApiHandleScope(napi_env env) noexcept;
  ~NodeApiHandleScope() noexcept;

 private:
  napi_env env_{};
  napi_handle_scope scope_{};
};

class NodeApiEnvScope {
 public:
  explicit NodeApiEnvScope(napi_env env) noexcept;

  ~NodeApiEnvScope() noexcept;

  NodeApiEnvScope(NodeApiEnvScope&& other) noexcept;
  NodeApiEnvScope& operator=(NodeApiEnvScope&& other) noexcept;

  NodeApiEnvScope(const NodeApiEnvScope&) = delete;
  NodeApiEnvScope& operator=(const NodeApiEnvScope&) = delete;

 private:
  napi_env env_{};
  jsr_napi_env_scope scope_{};
};

class NodeLiteModule {
 public:
  using InitModuleCallback =
      std::function<napi_value(napi_env /*env*/, napi_value /*exports*/)>;

  explicit NodeLiteModule(std::filesystem::path module_path) noexcept;
  explicit NodeLiteModule(std::filesystem::path module_path,
                          InitModuleCallback init_module) noexcept;

  napi_value LoadModule(napi_env env);

  NodeLiteModule(const NodeLiteModule&) = delete;
  NodeLiteModule& operator=(const NodeLiteModule&) = delete;

 private:
  napi_value LoadScriptModule(napi_env env);
  napi_value LoadNativeModule(napi_env env);
  std::string ReadModuleFileText(napi_env env);

 private:
  enum class State {
    kNotLoaded,
    kLoading,
    kLoaded,
  };

 private:
  State state_{State::kNotLoaded};
  std::filesystem::path module_path_;
  InitModuleCallback init_module_;
  NodeApiRef exports_;
};

// The Node.js-like runtime that is enough to run Node-API tests.
class NodeLiteRuntime {
  struct PrivateTag {};

 public:
  static std::unique_ptr<NodeLiteRuntime> Create(
      std::shared_ptr<NodeLiteTaskRunner> task_runner,
      std::string js_root,
      std::vector<std::string> args);

  explicit NodeLiteRuntime(PrivateTag tag,
                           std::shared_ptr<NodeLiteTaskRunner> task_runner,
                           std::string js_root,
                           std::vector<std::string> args);

  static void Run(std::vector<std::string> args);

  NodeLiteModule& ResolveModule(const std::string& parent_module_path,
                                const std::string& module_path);

  std::filesystem::path ResolveModulePath(const std::string& parent_module_path,
                                          const std::string& module_path);

  void RunTestScript(const std::string& script_path);

  void AddNativeModule(
      const std::string& module_name,
      std::function<napi_value(napi_env, napi_value)> initModule);

  void HandleUnhandledPromiseRejections();
  void OnExit();
  void OnUncaughtException(napi_value error);

  std::string ProcessStack(std::string const& stack,
                           std::string const& assertMethod);

  static NodeLiteRuntime* GetRuntime(napi_env env);

 private:
  void Initialize();
  void DefineGlobalFunctions();
  void DefineBuiltInModules();

 private:
  std::shared_ptr<NodeLiteTaskRunner> task_runner_;
  std::string js_root_;
  std::vector<std::string> args_;
  std::unique_ptr<IEnvHolder> env_holder_;
  napi_env env_{};
  std::unordered_map<std::string, std::unique_ptr<NodeLiteModule>>
      registered_modules_;
  std::unordered_map<std::string, std::string> node_js_modules_;
  std::vector<NodeApiRef> on_exit_callbacks_;
  std::vector<NodeApiRef> on_uncaughtException_callbacks_;
};

class NodeLitePlatform {
 public:
  static void* LoadFunction(napi_env env,
                            const std::filesystem::path& lib_path,
                            const std::string& function_name) noexcept;
};

using NodeApiCallback =
    std::function<napi_value(napi_env env, span<napi_value> args)>;

// Wraps up Node-API function calls.
// To simplify usage patterns it throws NodeApiException on errors.
class NodeApi {
 public:
  static bool IsExceptionPending(napi_env env);

  static napi_value GetAndClearLastException(napi_env env);

  static void ThrowError(napi_env env, napi_value error);

  static void ThrowError(napi_env env, const char* error_message);

  static napi_value GetNull(napi_env env);

  static napi_value GetUndefined(napi_env env);

  static napi_value GetGlobal(napi_env env);

  static napi_value GetBoolean(napi_env env, bool value);

  static napi_value GetReferenceValue(napi_env env, napi_ref ref);

  static napi_value CreateUInt32(napi_env env, std::uint32_t value);

  static napi_value CreateString(napi_env env, std::string_view value);

  static napi_value CreateStringArray(napi_env env,
                                      std::vector<std::string> const& value);

  static napi_value CreateObject(napi_env env);

  static napi_value CreateExternal(napi_env env, void* data);

  static int32_t GetValueInt32(napi_env env, napi_value value);

  static uint32_t GetValueUInt32(napi_env env, napi_value value);

  static void* GetValueExternal(napi_env env, napi_value value);

  static bool HasProperty(napi_env env,
                          napi_value obj,
                          std::string_view utf8_name);

  static napi_value GetProperty(napi_env env,
                                napi_value obj,
                                std::string_view utf8_name);

  static std::string GetPropertyString(napi_env env,
                                       napi_value obj,
                                       std::string_view utf8_name);

  static int32_t GetPropertyInt32(napi_env env,
                                  napi_value obj,
                                  std::string_view utf8_name);

  static void SetProperty(napi_env env,
                          napi_value obj,
                          std::string_view utf8_name,
                          napi_value value);

  static void SetPropertyUInt32(napi_env env,
                                napi_value obj,
                                std::string_view utf8_name,
                                uint32_t value);

  static void SetPropertyString(napi_env env,
                                napi_value obj,
                                std::string_view utf8_name,
                                std::string_view value);

  static void SetPropertyStringArray(napi_env env,
                                     napi_value obj,
                                     std::string_view utf8_name,
                                     std::vector<std::string> const& value);

  static void SetPropertyNull(napi_env env,
                              napi_value obj,
                              std::string_view utf8_name);

  static void SetMethod(napi_env env,
                        napi_value obj,
                        std::string_view utf8_name,
                        NodeApiCallback cb);

  static bool DeleteProperty(napi_env env,
                             napi_value obj,
                             std::string_view utf8_name);

  static std::string CoerceToString(napi_env env, napi_value value);

  static std::string ToStdString(napi_env env, napi_value value);

  static std::vector<std::string> ToStdStringArray(napi_env env,
                                                   napi_value value);

  static napi_value RunScript(napi_env env, napi_value script);

  static napi_value RunScript(napi_env env,
                              const std::string& code,
                              char const* source_url);

  static napi_valuetype TypeOf(napi_env env, napi_value value);

  static napi_value CallFunction(napi_env env,
                                 napi_value func,
                                 span<napi_value> args);

  static napi_value CreateFunction(napi_env env,
                                   std::string_view name,
                                   NodeApiCallback cb);
};

}  // namespace node_api_tests

#endif  // !NODE_API_TEST_NODE_LITE_H