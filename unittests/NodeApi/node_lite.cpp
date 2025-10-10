// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "node_lite.h"
#include "js_runtime_api.h"
#include <algorithm>
#include <array>
#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <regex>
#include <sstream>
#include "child_process.h"

namespace fs = std::filesystem;

namespace node_api_tests {

namespace {

NodeApiRef MakeNodeApiRef(napi_env env, napi_value value) {
  napi_ref ref{};
  NODE_LITE_CALL(napi_create_reference(env, value, 1, &ref));
  return NodeApiRef(ref, NodeApiRefDeleter(env));
}

template <typename TCallback>
void ThrowJSErrorOnException(napi_env env, TCallback&& callback) noexcept {
  try {
    callback();
  } catch (const NodeLiteException& e) {
    if (e.error_status() == napi_pending_exception) {
      napi_value error = NodeApi::GetAndClearLastException(env);
      NodeApi::ThrowError(env, error);
    } else {
      NodeApi::ThrowError(env, e.what());
    }
  } catch (const std::exception& e) {
    NodeApi::ThrowError(env, e.what());
  }
}

template <typename TCallback>
void ExitOnException(napi_env env, TCallback&& callback) noexcept {
  try {
    callback();
  } catch (const NodeLiteException& e) {
    if (e.error_status() == napi_pending_exception) {
      napi_value error = NodeApi::GetAndClearLastException(env);
      NodeLiteErrorHandler::ExitWithJSError(env, error);
    } else {
      NodeLiteErrorHandler::ExitWithMessage(e.what());
    }
  } catch (const std::exception& e) {
    NodeLiteErrorHandler::ExitWithMessage(e.what());
  }
}

std::string ReadFileText(napi_env env, fs::path file_path) {
  std::ifstream file_stream(file_path.string());
  NODE_LITE_ASSERT(file_stream.is_open(),
                   "Failed to open file: %s. Error: %s",
                   file_path.c_str(),
                   std::strerror(errno));
  std::ostringstream ss;
  ss << file_stream.rdbuf();
  return ss.str();
}

class NodeApiCallbackInfo {
 public:
  NodeApiCallbackInfo(napi_env env, napi_callback_info info) {
    size_t argc{inline_args_.size()};
    napi_value* argv = inline_args_.data();
    NODE_LITE_CALL(
        napi_get_cb_info(env, info, &argc, argv, &this_arg_, &data_));
    if (argc > inline_args_.size()) {
      dynamic_args_ = std::make_unique<napi_value[]>(argc);
      argv = dynamic_args_.get();
      NODE_LITE_CALL(
          napi_get_cb_info(env, info, &argc, argv, &this_arg_, &data_));
    }
    args_ = span<napi_value>(argv, argc);
  }

  span<napi_value> args() const { return args_; }
  napi_value this_arg() const { return this_arg_; }
  void* data() const { return data_; }

 private:
  std::array<napi_value, 16> inline_args_{};
  std::unique_ptr<napi_value[]> dynamic_args_{};
  span<napi_value> args_{};
  napi_value this_arg_{};
  void* data_{};
};

}  // namespace

//=============================================================================
// NodeApiTest implementation
//=============================================================================

std::unique_ptr<IEnvHolder> CreateEnvHolder(
    std::shared_ptr<NodeLiteTaskRunner> taskRunner,
    std::function<void(napi_env, napi_value)> onUnhandledError);

//=============================================================================
// NodeLiteModule implementation
//=============================================================================

using ModuleRegisterFuncCallback = napi_value(NAPI_CDECL*)(napi_env env,
                                                           napi_value exports);
using ModuleApiVersionCallback = int32_t(NAPI_CDECL*)();

NodeLiteModule::NodeLiteModule(std::filesystem::path module_path) noexcept
    : module_path_(std::move(module_path)) {}

NodeLiteModule::NodeLiteModule(std::filesystem::path module_path,
                               InitModuleCallback init_module) noexcept
    : module_path_(std::move(module_path)),
      init_module_(std::move(init_module)) {}

napi_value NodeLiteModule::LoadModule(napi_env env) {
  if (state_ == State::kLoaded) {
    return NodeApi::GetReferenceValue(env, exports_.get());
  }
  if (state_ == State::kLoading) {
    return NodeApi::GetUndefined(env);
  }
  NODE_LITE_ASSERT(state_ == State::kNotLoaded,
                   "Unexpected module '%s' state: %d",
                   module_path_.string().c_str(),
                   static_cast<int32_t>(state_));
  state_ = State::kLoading;
  struct ResetStateIfFailed {
    NodeLiteModule* module_;
    ~ResetStateIfFailed() {
      if (module_->state_ == State::kLoading) {
        module_->state_ = State::kNotLoaded;
      }
    }
  } reset_state_if_failed{this};

  if (init_module_) {
    napi_value exports = NodeApi::CreateObject(env);
    napi_value init_exports = init_module_(env, exports);
    if (init_exports != nullptr &&
        NodeApi::TypeOf(env, init_exports) != napi_undefined) {
      exports = init_exports;
    }
    exports_ = MakeNodeApiRef(env, exports);
  } else if (module_path_.extension() == ".js") {
    exports_ = MakeNodeApiRef(env, LoadScriptModule(env));
  } else if (module_path_.extension() == ".node") {
    exports_ = MakeNodeApiRef(env, LoadNativeModule(env));
  } else {
    NODE_LITE_ASSERT(
        false, "Unsupported module type: %s", module_path_.string().c_str());
  }
  state_ = State::kLoaded;
  return NodeApi::GetReferenceValue(env, exports_.get());
}

napi_value NodeLiteModule::LoadScriptModule(napi_env env) {
  std::string module_func_wrapper =
      "(function(module, exports, require, __filename, __dirname) {";
  module_func_wrapper += ReadModuleFileText(env);

  size_t source_map_index = module_func_wrapper.find("//# sourceMappingURL");
  constexpr const char* module_suffix = "\nreturn module.exports; })\n";
  if (source_map_index != std::string::npos) {
    module_func_wrapper.insert(source_map_index, module_suffix);
  } else {
    module_func_wrapper += module_suffix;
  }

  napi_value module_func = NodeApi::RunScript(
      env, module_func_wrapper, module_path_.string().c_str());

  NODE_LITE_ASSERT(NodeApi::TypeOf(env, module_func) == napi_function);

  napi_value exports = NodeApi::CreateObject(env);
  napi_value file_name = NodeApi::CreateString(env, module_path_.string());
  napi_value dir_name =
      NodeApi::CreateString(env, module_path_.parent_path().string());

  napi_value module_obj = NodeApi::CreateObject(env);
  NodeApi::SetProperty(env, module_obj, "exports", exports);
  NodeApi::SetProperty(env, module_obj, "__filename", file_name);
  NodeApi::SetProperty(env, module_obj, "__dirname", dir_name);

  napi_value require = NodeApi::CreateFunction(
      env, "require", [this](napi_env env, span<napi_value> args) {
        NODE_LITE_ASSERT(args.size() >= 1, "Expected at least one argument");
        std::string module_path = NodeApi::ToStdString(env, args[0]);
        NodeLiteRuntime* runtime = NodeLiteRuntime::GetRuntime(env);
        return runtime
            ->ResolveModule(module_path_.parent_path().string(), module_path)
            .LoadModule(env);
      });

  return NodeApi::CallFunction(
      env, module_func, {module_obj, exports, require, file_name, dir_name});
}

napi_value NodeLiteModule::LoadNativeModule(napi_env env) {
  ModuleApiVersionCallback getModuleApiVersion =
      reinterpret_cast<ModuleApiVersionCallback>(NodeLitePlatform::LoadFunction(
          env, module_path_.c_str(), "node_api_module_get_api_version_v1"));
  int32_t moduleApiVersion = getModuleApiVersion ? getModuleApiVersion() : 8;

  ModuleRegisterFuncCallback moduleRegisterFunc =
      reinterpret_cast<ModuleRegisterFuncCallback>(
          NodeLitePlatform::LoadFunction(
              env, module_path_.c_str(), "napi_register_module_v1"));
  NODE_LITE_ASSERT(moduleRegisterFunc != nullptr,
                   "Failed to find 'napi_register_module_v1' in module: %s",
                   module_path_.c_str());

  napi_value exports{};
  NODE_LITE_CALL(jsr_initialize_native_module(
      env, moduleRegisterFunc, moduleApiVersion, &exports));
  return exports;
}

std::string NodeLiteModule::ReadModuleFileText(napi_env env) {
  return ReadFileText(env, module_path_);
}

//=============================================================================
// NodeLiteRuntime implementation
//=============================================================================

/*static*/ void NodeLiteRuntime::Run(std::vector<std::string> argv) {
  // Convert arguments to vector of strings and skip all options before the JS
  // file name.
  std::vector<std::string> args;
  args.reserve(argv.size());
  bool skipOptions = true;
  if (argv.size() < 2) {
    NodeLiteErrorHandler::ExitWithMessage("", [&](std::ostream& os) {
      os << "Usage: " << argv[0] << " <js_file>";
    });
  }
  args.push_back(argv[0]);
  for (int i = 1; i < argv.size(); i++) {
    if (skipOptions && std::string_view(argv[i]).find("--") == 0) {
      continue;
    }
    skipOptions = false;
    args.push_back(argv[i]);
  }

  std::shared_ptr<NodeLiteTaskRunner> taskRunner =
      std::make_shared<NodeLiteTaskRunner>();

  fs::path exe_path = fs::canonical(argv[0]);

  fs::path test_root_path = exe_path.parent_path();
  fs::path js_root = test_root_path / "test";
  if (!fs::exists(js_root)) {
    test_root_path = test_root_path.parent_path();
    js_root = test_root_path / "test";
  }
  if (!fs::exists(js_root)) {
    NodeLiteErrorHandler::ExitWithMessage("Error: Cannot find test directory.");
  }

  std::string jsFilePath = args[1];
  std::unique_ptr<NodeLiteRuntime> runtime = NodeLiteRuntime::Create(
      std::move(taskRunner), js_root.string(), std::move(args));
  runtime->RunTestScript(jsFilePath);
}

/*static*/ std::unique_ptr<NodeLiteRuntime> NodeLiteRuntime::Create(
    std::shared_ptr<NodeLiteTaskRunner> task_runner,
    std::string js_root,
    std::vector<std::string> args) {
  std::unique_ptr<NodeLiteRuntime> runtime =
      std::make_unique<NodeLiteRuntime>(PrivateTag{},
                                        std::move(task_runner),
                                        std::move(js_root),
                                        std::move(args));
  runtime->Initialize();
  return runtime;
}

NodeLiteRuntime::NodeLiteRuntime(
    PrivateTag,
    std::shared_ptr<NodeLiteTaskRunner> task_runner,
    std::string js_root,
    std::vector<std::string> args)
    : task_runner_(std::move(task_runner)),
      js_root_(std::move(js_root)),
      args_(std::move(args)) {}

void NodeLiteRuntime::Initialize() {
  env_holder_ =
      CreateEnvHolder(task_runner_, [this](napi_env env, napi_value error) {
        NODE_LITE_ASSERT(env == env_,
                         "Unhandled error in different napi_env: %p != %p",
                         env,
                         env_);
        OnUncaughtException(error);
      });
  env_ = env_holder_->getEnv();
  NodeApiEnvScope env_scope{env_};
  NodeApiHandleScope handle_scope{env_};
  DefineBuiltInModules();
  DefineGlobalFunctions();
}

NodeLiteModule& NodeLiteRuntime::ResolveModule(
    const std::string& parent_module_path, const std::string& module_path) {
  napi_env env = env_;
  fs::path fs_module_path = ResolveModulePath(parent_module_path, module_path);
  if (auto it = registered_modules_.find(fs_module_path.string());
      it != registered_modules_.end()) {
    return *it->second;
  }

  if (auto [it, succeeded] = registered_modules_.try_emplace(
          fs_module_path.string(),
          std::make_unique<NodeLiteModule>(fs_module_path.string()));
      succeeded) {
    return *it->second;
  }

  NODE_LITE_ASSERT(
      false, "Failed to register module: %s", fs_module_path.string().c_str());
}

fs::path NodeLiteRuntime::ResolveModulePath(
    const std::string& parent_module_path, const std::string& module_path) {
  napi_env env = env_;
  // 1. See if it is an embedded module such as "assert".
  auto it = node_js_modules_.find(module_path);
  if (it != node_js_modules_.end()) {
    return fs::path(it->second);
  }

  // 2. Check if it is a relative or an absolute path to a module.
  {
    fs::path fs_module_path = fs::path(module_path);
    if (!fs_module_path.is_absolute()) {
      fs::path fs_parent_module_path = fs::path(parent_module_path);
      NODE_LITE_ASSERT(fs_parent_module_path.is_absolute(),
                       "Parent module path '%s' is not absolute",
                       parent_module_path.c_str());
      fs_module_path = fs_parent_module_path / fs_module_path;
    }
    fs_module_path = fs::weakly_canonical(fs_module_path);

    if (fs::exists(fs_module_path) && fs::is_regular_file(fs_module_path)) {
      return fs_module_path;
    }
    if (fs::path result = fs::path(fs_module_path).replace_extension(".js");
        fs::exists(result)) {
      return result;
    }
    if (fs::path result = fs_module_path / "index.js"; fs::exists(result)) {
      return result;
    }
    // See if it is a native module.
    fs::path node_module_path =
        fs::path(fs_module_path).replace_extension(".node");
    if (fs::exists(node_module_path)) {
      return node_module_path;
    }
    // See if the module was prefixed with the parent folder to disambiguate C++
    // project name.
    fs::path fs_parent_folder = fs::path(parent_module_path).filename();
    node_module_path.replace_filename(fs_parent_folder.string() + "_" +
                                      node_module_path.filename().string());
    if (fs::exists(node_module_path)) {
      return node_module_path;
    }
  }

  // 3. Check if it is in the node_modules folder.
  {
    fs::path fs_module_path = fs::weakly_canonical(
        fs::path(js_root_) / "node_modules" / fs::path(module_path));

    if (fs::exists(fs_module_path) && fs::is_regular_file(fs_module_path)) {
      return fs_module_path;
    }
    if (fs::path result = fs::path(fs_module_path).replace_extension(".js");
        fs::exists(result)) {
      return result;
    }
    if (fs::path result = fs_module_path / "index.js"; fs::exists(result)) {
      return result;
    }
  }

  NODE_LITE_ASSERT(
      false, "Cannot resolve module path '%s'", module_path.c_str());
}

void NodeLiteRuntime::AddNativeModule(
    const std::string& module_name,
    std::function<napi_value(napi_env, napi_value)> initModule) {
  napi_env env = env_;
  auto [_, succeeded] = registered_modules_.try_emplace(
      module_name,
      std::make_unique<NodeLiteModule>(module_name, std::move(initModule)));
  NODE_LITE_ASSERT(
      succeeded, "Failed to register module: %s", module_name.c_str());
}

void NodeLiteRuntime::RunTestScript(const std::string& script_path) {
  NodeApiEnvScope env_scope{env_};
  NodeApiHandleScope handle_scope{env_};
  {
    ExitOnException(env_, [this, &script_path]() {
      NodeApiHandleScope scope{env_};
      NodeLiteModule& main_module = ResolveModule(js_root_, script_path);
      main_module.LoadModule(env_);
    });
    ExitOnException(env_, [this]() {
      task_runner_->DrainTaskQueue();
      OnExit();
      on_exit_callbacks_.clear();
      on_uncaughtException_callbacks_.clear();
    });
  }
}

void NodeLiteRuntime::OnExit() {
  for (NodeApiRef& callback_ref : on_exit_callbacks_) {
    napi_value callback = NodeApi::GetReferenceValue(env_, callback_ref.get());
    NodeApi::CallFunction(env_, callback, {NodeApi::CreateUInt32(env_, 0)});
  }
}

void NodeLiteRuntime::OnUncaughtException(napi_value error) {
  bool shouldExit = true;
  for (NodeApiRef& callback_ref : on_uncaughtException_callbacks_) {
    napi_value callback = NodeApi::GetReferenceValue(env_, callback_ref.get());
    napi_value result = NodeApi::CallFunction(
        env_,
        callback,
        {error, NodeApi::CreateString(env_, "uncaughtException")});
    // If at least one callback returns false, we do not exit.
    // TODO: (vmoroz) Investigate the Node.js behavior in that case
    // if (shouldExit && NodeApi::TypeOf(env_, result) == napi_boolean) {
    //  shouldExit = NodeApi::GetBoolean(env_, result);
    //}
    shouldExit = false;
  }

  if (shouldExit) {
    NodeLiteErrorHandler::ExitWithJSError(env_, error);
  }
}

/*static*/ NodeLiteRuntime* NodeLiteRuntime::GetRuntime(napi_env env) {
  napi_value global = NodeApi::GetGlobal(env);
  return static_cast<NodeLiteRuntime*>(NodeApi::GetValueExternal(
      env, NodeApi::GetProperty(env, global, "__NodeLiteRuntime__")));
}

void NodeLiteRuntime::DefineBuiltInModules() {
  napi_env env = env_;
  // Define "assert" module
  {
    fs::path assert_path =
        fs::weakly_canonical(fs::path(js_root_) / "common" / "assert.js");
    std::string assert_path_str = assert_path.string();
    NODE_LITE_ASSERT(fs::exists(assert_path),
                     "Failed to find assert.js file: %s",
                     assert_path_str.c_str());
    node_js_modules_.try_emplace(assert_path_str, assert_path_str);
    node_js_modules_.try_emplace(assert_path.replace_extension().string(),
                                 assert_path_str);
    node_js_modules_.try_emplace("assert", assert_path_str);
    node_js_modules_.try_emplace("node:assert", assert_path_str);
  }

  // Define "child_process" module
  {
    node_js_modules_.try_emplace("child_process", "child_process");
    node_js_modules_.try_emplace("node:child_process", "child_process");
    AddNativeModule("child_process", [this](napi_env env, napi_value exports) {
      NodeApi::SetMethod(
          env_, exports, "spawnSync", [](napi_env env, span<napi_value> args) {
            NODE_LITE_ASSERT(args.size() >= 2,
                             "Expected at least 2 arguments, but got: %zu",
                             args.size());
            std::string command = NodeApi::ToStdString(env, args[0]);
            std::vector<std::string> command_args =
                NodeApi::ToStdStringArray(env, args[1]);
            ProcessResult call_result = SpawnSync(command, command_args);
            napi_value result = NodeApi::CreateObject(env);
            NodeApi::SetPropertyUInt32(
                env, result, "status", call_result.status);
            NodeApi::SetPropertyString(
                env, result, "stderr", call_result.std_error);
            NodeApi::SetPropertyString(
                env, result, "stdout", call_result.std_output);
            NodeApi::SetPropertyNull(env, result, "signal");
            return result;
          });
      return exports;
    });
  }

  // Define "fs" module
  {
    node_js_modules_.try_emplace("fs", "fs");
    node_js_modules_.try_emplace("node:fs", "fs");
    AddNativeModule("fs", [this](napi_env env, napi_value exports) {
      NodeApi::SetMethod(
          env_, exports, "existsSync", [](napi_env env, span<napi_value> args) {
            NODE_LITE_ASSERT(args.size() >= 1, "Expected at least 1 argument");
            fs::path path = fs::path{NodeApi::ToStdString(env, args[0])};
            return NodeApi::GetBoolean(env, fs::exists(path));
          });
      NodeApi::SetMethod(
          env_,
          exports,
          "readFileSync",
          [](napi_env env, span<napi_value> args) {
            NODE_LITE_ASSERT(args.size() >= 1, "Expected at least 1 argument");
            fs::path path = fs::path{NodeApi::ToStdString(env, args[0])};
            return NodeApi::CreateString(env, ReadFileText(env, path));
          });
      return exports;
    });
  }

  // Define "path" module
  {
    node_js_modules_.try_emplace("path", "path");
    node_js_modules_.try_emplace("node:path", "path");
    AddNativeModule("path", [this](napi_env env, napi_value exports) {
      NodeApi::SetMethod(
          env_, exports, "join", [](napi_env env, span<napi_value> args) {
            NODE_LITE_ASSERT(args.size() >= 2,
                             "Expected at least 2 arguments, but got: %zu",
                             args.size());
            fs::path path = fs::path{NodeApi::ToStdString(env, args[0])};
            for (size_t i = 1; i < args.size(); ++i) {
              path /= NodeApi::ToStdString(env, args[i]);
            }
            return NodeApi::CreateString(env, path.string());
          });
      return exports;
    });
  }
}

void NodeLiteRuntime::DefineGlobalFunctions() {
  NodeApiHandleScope scope{env_};
  napi_value global = NodeApi::GetGlobal(env_);

  // Add global.global
  NodeApi::SetProperty(env_, global, "global", global);

  // Add global.__NodeLiteRuntime__
  NodeApi::SetProperty(
      env_, global, "__NodeLiteRuntime__", NodeApi::CreateExternal(env_, this));

  // Remove the global.require defined by Hermes
  NodeApi::DeleteProperty(env_, global, "require");

  // global.gc()
  NodeApi::SetMethod(
      env_, global, "gc", [](napi_env env, span<napi_value> /*args*/) {
        NODE_LITE_CALL(jsr_collect_garbage(env));
        return nullptr;
      });

  auto set_immediate_cb = [](napi_env env, span<napi_value> args) {
    NODE_LITE_ASSERT(args.size() >= 1,
                     "Expected at least 1 argument, but got: %zu",
                     args.size());
    std::shared_ptr<NodeApiRef> callback_ref =
        std::make_shared<NodeApiRef>(MakeNodeApiRef(env, args[0]));
    uint32_t task_id = GetRuntime(env)->task_runner_->PostTask(
        [env, callback_ref = std::move(callback_ref)]() {
          ExitOnException(env, [env, &callback_ref]() {
            NodeApiHandleScope scope{env};
            napi_value callback =
                NodeApi::GetReferenceValue(env, callback_ref->get());
            NodeApi::CallFunction(env, callback, {});
          });
        });
    return NodeApi::CreateUInt32(env, task_id);
  };

  // global.setImmediate()
  NodeApi::SetMethod(env_, global, "setImmediate", set_immediate_cb);

  // global.setTimeout()
  NodeApi::SetMethod(env_, global, "setTimeout", set_immediate_cb);

  // global.clearTimeout()
  NodeApi::SetMethod(
      env_, global, "clearTimeout", [](napi_env env, span<napi_value> args) {
        NODE_LITE_ASSERT(args.size() >= 1,
                         "Expected at least 1 argument, but got: %zu",
                         args.size());
        uint32_t task_id = NodeApi::GetValueUInt32(env, args[0]);
        GetRuntime(env)->task_runner_->RemoveTask(task_id);
        return nullptr;
      });

  // global.process
  {
    napi_value process_obj = NodeApi::CreateObject(env_);
    NodeApi::SetProperty(env_, global, "process", process_obj);

    // process.argv
    NodeApi::SetPropertyStringArray(env_, process_obj, "argv", args_);

    // process.execPath
    NodeApi::SetPropertyString(env_, process_obj, "execPath", args_[0]);

// process.target_config
#ifdef NDEBUG
    NodeApi::SetPropertyString(env_, process_obj, "target_config", "Release");
#else
    NodeApi::SetPropertyString(env_, process_obj, "target_config", "Debug");
#endif

// process.platform
#ifdef WIN32
    NodeApi::SetPropertyString(env_, process_obj, "platform", "win32");
#else
    // TODO: (vmoroz) Add support for other platforms.
    NodeApi::SetPropertyString(env_, process_obj, "platform", "other");
#endif

    // process.exit(exit_code)
    NodeApi::SetMethod(
        env_, process_obj, "exit", [](napi_env env, span<napi_value> args) {
          NODE_LITE_ASSERT(args.size() >= 1,
                           "Expected at least 1 argument, but got: "
                           "%zu",
                           args.size());
          int32_t exit_code = NodeApi::GetValueInt32(env, args[0]);
          exit(exit_code);
          return nullptr;
        });

    // process.on('event_name', callback)
    NodeApi::SetMethod(
        env_, process_obj, "on", [](napi_env env, span<napi_value> args) {
          NODE_LITE_ASSERT(args.size() >= 2,
                           "Expected at least 2 arguments, but got: %zu",
                           args.size());
          std::string event_name = NodeApi::ToStdString(env, args[0]);
          if (event_name == "exit") {
            NODE_LITE_ASSERT(NodeApi::TypeOf(env, args[1]) == napi_function,
                             "Expected function as second argument");
            GetRuntime(env)->on_exit_callbacks_.push_back(
                MakeNodeApiRef(env, args[1]));
          } else if (event_name == "uncaughtException") {
            NODE_LITE_ASSERT(NodeApi::TypeOf(env, args[1]) == napi_function,
                             "Expected function as second argument");
            GetRuntime(env)->on_uncaughtException_callbacks_.push_back(
                MakeNodeApiRef(env, args[1]));
          } else {
            NODE_LITE_ASSERT(false,
                             "Unsupported process event name: %s",
                             event_name.c_str());
          }
          return nullptr;
        });
  }

  // global.console
  {
    napi_value console_obj = NodeApi::CreateObject(env_);
    NodeApi::SetProperty(env_, global, "console", console_obj);

    // console.log()
    NodeApi::SetMethod(
        env_, console_obj, "log", [](napi_env env, span<napi_value> args) {
          NODE_LITE_ASSERT(args.size() >= 1, "Expected at least 1 argument");
          std::string message = NodeApi::ToStdString(env, args[0]);
          std::cout << message << std::endl;
          return nullptr;
        });

    // console.error()
    NodeApi::SetMethod(
        env_,
        console_obj,
        "error",
        [](napi_env env, span<napi_value> args) -> napi_value {
          NODE_LITE_ASSERT(args.size() >= 1, "Expected at least 1 argument");
          std::string message = NodeApi::ToStdString(env, args[0]);
          std::cerr << message << std::endl;
          return nullptr;
        });
  }
}

std::string NodeLiteRuntime::ProcessStack(std::string const& stack,
                                          std::string const& assertMethod) {
  // Split up the stack string into an array of stack frames
  auto stackStream = std::istringstream(stack);
  std::string stackFrame;
  std::vector<std::string> stackFrames;
  while (std::getline(stackStream, stackFrame, '\n')) {
    stackFrames.push_back(std::move(stackFrame));
  }

  // Remove first and last stack frames: one is the error message
  // and another is the module root call.
  if (!stackFrames.empty()) {
    stackFrames.pop_back();
  }
  if (!stackFrames.empty()) {
    stackFrames.erase(stackFrames.begin());
  }

  std::string processedStack;
  bool assertFuncFound = false;
  std::string assertFuncPattern = assertMethod + " (";
  const std::regex locationRE("(\\w+):(\\d+)");
  std::smatch locationMatch;
  // for (auto const& frame : stackFrames) {
  //   if (assertFuncFound) {
  //     std::string processedFrame;
  //     if (std::regex_search(frame, locationMatch, locationRE)) {
  //       if (auto const* scriptInfo =
  //               GetTestScriptInfo(locationMatch[1].str())) {
  //         int32_t cppLine =
  //             scriptInfo->line + std::stoi(locationMatch[2].str()) - 1;
  //         processedFrame = locationMatch.prefix().str() +
  //                          UseSrcFilePath(scriptInfo->filePath.string()) +
  //                          ':' + std::to_string(cppLine) +
  //                          locationMatch.suffix().str();
  //       }
  //     }
  //     processedStack +=
  //         (!processedFrame.empty() ? processedFrame : frame) + '\n';
  //   } else {
  //     auto pos = frame.find(assertFuncPattern);
  //     if (pos != std::string::npos) {
  //       if (frame[pos - 1] == '.' || frame[pos - 1] == ' ') {
  //         assertFuncFound = true;
  //       }
  //     }
  //   }
  // }

  return processedStack;
}

//=============================================================================
// NodeApiRefDeleter implementation
//=============================================================================

NodeApiRefDeleter::NodeApiRefDeleter() noexcept = default;

NodeApiRefDeleter::NodeApiRefDeleter(napi_env env) noexcept : env_(env) {}

void NodeApiRefDeleter::operator()(napi_ref ref) noexcept {
  if (ref == nullptr || env_ == nullptr) {
    return;
  }
  napi_env env = env_;
  NODE_LITE_CALL(napi_delete_reference(env, ref));
}

//=============================================================================
// NodeLiteTaskRunner implementation
//=============================================================================

uint32_t NodeLiteTaskRunner::PostTask(std::function<void()>&& task) noexcept {
  uint32_t task_id = next_task_id_++;
  task_queue_.emplace_back(task_id, std::move(task));
  return task_id;
}

void NodeLiteTaskRunner::RemoveTask(uint32_t task_id) noexcept {
  task_queue_.remove_if(
      [task_id](const std::pair<uint32_t, std::function<void()>>& entry) {
        return entry.first == task_id;
      });
}

void NodeLiteTaskRunner::DrainTaskQueue() noexcept {
  while (!task_queue_.empty()) {
    std::pair<uint32_t, std::function<void()>> task =
        std::move(task_queue_.front());
    task_queue_.pop_front();
    task.second();
  }
}

/*static*/ void NodeLiteTaskRunner::PostTaskCallback(
    void* task_runner_data,
    void* task_data,
    jsr_task_run_cb task_run_cb,
    jsr_data_delete_cb task_data_delete_cb,
    void* deleter_data) {
  NodeLiteTaskRunner* taskRunnerPtr =
      static_cast<std::shared_ptr<NodeLiteTaskRunner>*>(task_runner_data)
          ->get();
  taskRunnerPtr->PostTask(
      [task_run_cb, task_data, task_data_delete_cb, deleter_data]() {
        if (task_run_cb != nullptr) {
          task_run_cb(task_data);
        }
        if (task_data_delete_cb != nullptr) {
          task_data_delete_cb(task_data, deleter_data);
        }
      });
}

/*static*/ void NodeLiteTaskRunner::DeleteCallback(void* data,
                                                   void* /*deleter_data*/) {
  delete static_cast<std::shared_ptr<NodeLiteTaskRunner>*>(data);
}

//=============================================================================
// NodeApiHandleScope implementation
//=============================================================================

NodeApiHandleScope::NodeApiHandleScope(napi_env env) noexcept : env_{env} {
  NODE_LITE_CALL(napi_open_handle_scope(env, &scope_));
}

NodeApiHandleScope::~NodeApiHandleScope() noexcept {
  napi_env env = env_;
  NODE_LITE_CALL(napi_close_handle_scope(env, scope_));
}

//=============================================================================
// NodeApiEnvScope implementation
//=============================================================================

NodeApiEnvScope::NodeApiEnvScope(napi_env env) noexcept : env_{env} {
  NODE_LITE_CALL(jsr_open_napi_env_scope(env, &scope_));
}

NodeApiEnvScope ::~NodeApiEnvScope() noexcept {
  if (env_ != nullptr) {
    napi_env env = env_;
    NODE_LITE_CALL(jsr_close_napi_env_scope(env, scope_));
  }
}

NodeApiEnvScope::NodeApiEnvScope(NodeApiEnvScope&& other) noexcept
    : env_{std::exchange(other.env_, nullptr)},
      scope_{std::exchange(other.scope_, nullptr)} {}

NodeApiEnvScope& NodeApiEnvScope::operator=(NodeApiEnvScope&& other) noexcept {
  if (this != &other) {
    NodeApiEnvScope temp(std::move(*this));
    env_ = std::exchange(other.env_, nullptr);
    scope_ = std::exchange(other.scope_, nullptr);
  }
  return *this;
}

//=============================================================================
// NodeLiteErrorHandler implementation
//=============================================================================

/*static*/ [[noreturn]] void NodeLiteErrorHandler::OnNodeApiFailed(
    napi_env env, napi_status error_code) {
  const char* errorMessage = "An exception is pending";
  if (NodeApi::IsExceptionPending(env)) {
    error_code = napi_pending_exception;
  } else {
    const napi_extended_error_info* error_info{};
    napi_status status = napi_get_last_error_info(env, &error_info);
    if (status != napi_ok) {
      NodeLiteErrorHandler::ExitWithMessage("", [&](std::ostream& os) {
        os << "Failed to get last error info: " << status;
      });
    }
    errorMessage = error_info->error_message;
  }
  throw NodeLiteException(error_code, errorMessage);
}

/*static*/ [[noreturn]] void NodeLiteErrorHandler::OnAssertFailed(
    napi_env env, char const* expr, char const* message) {
  std::string error_message = FormatString("Assert failed: %s.", expr);
  if (message != nullptr) {
    std::string message_str{message};
    if (!message_str.empty()) {
      error_message += " " + message_str;
    }
  }
  napi_status error_code = NodeApi::IsExceptionPending(env)
                               ? napi_pending_exception
                               : napi_generic_failure;

  throw NodeLiteException(error_code, error_message.c_str());
}

/*static*/ [[noreturn]] void NodeLiteErrorHandler::ExitWithJSError(
    napi_env env, napi_value error) noexcept {
  // TODO: protect from stack overflow
  napi_valuetype error_value_type = NodeApi::TypeOf(env, error);
  if (error_value_type == napi_object) {
    std::string name = NodeApi::GetPropertyString(env, error, "name");
    if (name == "AssertionError") {
      ExitWithJSAssertError(env, error);
    }
    std::string message = NodeApi::GetPropertyString(env, error, "message");
    std::string stack = NodeApi::GetPropertyString(env, error, "stack");
    ExitWithMessage("JavaScript error", [&](std::ostream& os) {
      os << "Exception: " << name << '\n'
         << "  Message: " << message << '\n'
         << "Callstack: " << '\n'
         << stack;
    });
  } else {
    std::string message = NodeApi::CoerceToString(env, error);
    ExitWithMessage("JavaScript error",
                    [&](std::ostream& os) { os << "  Message: " << message; });
  }
}

/*static*/ [[noreturn]] void NodeLiteErrorHandler::ExitWithJSAssertError(
    napi_env env, napi_value error) noexcept {
  std::string message = NodeApi::GetPropertyString(env, error, "message");
  std::string method = NodeApi::GetPropertyString(env, error, "method");
  std::string expected = NodeApi::GetPropertyString(env, error, "expected");
  std::string actual = NodeApi::GetPropertyString(env, error, "actual");
  std::string source_file =
      NodeApi::GetPropertyString(env, error, "sourceFile");
  int32_t source_line = NodeApi::GetPropertyInt32(env, error, "sourceLine");
  std::string error_stack =
      NodeApi::GetPropertyString(env, error, "errorStack");
  if (error_stack.empty()) {
    error_stack = NodeApi::GetPropertyString(env, error, "stack");
  }
  std::string method_name = "assert." + method;
  std::stringstream error_details;
  if (method_name != "assert.fail") {
    error_details << " Expected: " << expected << '\n'
                  << "   Actual: " << actual << '\n';
  }

  ExitWithMessage("JavaScript assertion error", [&](std::ostream& os) {
    os << "Exception: " << "AssertionError" << '\n'
       << "   Method: " << method_name << '\n'
       << "  Message: " << message << '\n'
       << error_details.str(/*a filler for formatting*/)
       << "Callstack: " << '\n'
       << error_stack;
  });
}

/*static*/ [[noreturn]] void NodeLiteErrorHandler::ExitWithMessage(
    const std::string& message,
    std::function<void(std::ostream&)> get_error_details) noexcept {
  std::ostringstream details_stream;
  get_error_details(details_stream);
  std::string details = details_stream.str();
  if (!message.empty()) {
    std::cerr << message;
  }
  if (!details.empty()) {
    if (!message.empty()) {
      std::cerr << "\n";
    }
    std::cerr << details;
  }
  std::cerr << std::endl;
  exit(1);
}

//=============================================================================
// NodeApi implementation
//=============================================================================

/*static*/ bool NodeApi::IsExceptionPending(napi_env env) {
  bool result{};
  NODE_LITE_CALL(napi_is_exception_pending(env, &result));
  return result;
}

/*static*/ napi_value NodeApi::GetAndClearLastException(napi_env env) {
  napi_value result{};
  NODE_LITE_CALL(napi_get_and_clear_last_exception(env, &result));
  return result;
}

/*static*/ void NodeApi::ThrowError(napi_env env, napi_value error) {
  NODE_LITE_CALL(napi_throw(env, error));
}

/*static*/ void NodeApi::ThrowError(napi_env env, const char* error_message) {
  NODE_LITE_CALL(napi_throw_error(env, "", error_message));
}

/*static*/ napi_value NodeApi::GetNull(napi_env env) {
  napi_value result{};
  NODE_LITE_CALL(napi_get_null(env, &result));
  return result;
}

/*static*/ napi_value NodeApi::GetUndefined(napi_env env) {
  napi_value result{};
  NODE_LITE_CALL(napi_get_undefined(env, &result));
  return result;
}

/*static*/ napi_value NodeApi::GetGlobal(napi_env env) {
  napi_value result{};
  NODE_LITE_CALL(napi_get_global(env, &result));
  return result;
}

/*static*/ napi_value NodeApi::GetBoolean(napi_env env, bool value) {
  napi_value result{};
  NODE_LITE_CALL(napi_get_boolean(env, value, &result));
  return result;
}

/*static*/ napi_value NodeApi::GetReferenceValue(napi_env env, napi_ref ref) {
  napi_value result{};
  NODE_LITE_CALL(napi_get_reference_value(env, ref, &result));
  return result;
}

/*static*/ napi_value NodeApi::CreateUInt32(napi_env env, std::uint32_t value) {
  napi_value result{};
  NODE_LITE_CALL(napi_create_uint32(env, value, &result));
  return result;
}

/*static*/ napi_value NodeApi::CreateString(napi_env env,
                                            std::string_view value) {
  napi_value result{};
  NODE_LITE_CALL(
      napi_create_string_utf8(env, value.data(), value.size(), &result));
  return result;
}

/*static*/ napi_value NodeApi::CreateStringArray(
    napi_env env, std::vector<std::string> const& value) {
  napi_value result{};
  NODE_LITE_CALL(napi_create_array(env, &result));

  uint32_t index = 0;
  for (const std::string& item : value) {
    NODE_LITE_CALL(
        napi_set_element(env, result, index++, CreateString(env, item)));
  }
  return result;
}

/*static*/ napi_value NodeApi::CreateObject(napi_env env) {
  napi_value result{};
  NODE_LITE_CALL(napi_create_object(env, &result));
  return result;
}

/*static*/ napi_value NodeApi::CreateExternal(napi_env env, void* data) {
  napi_value result{};
  NODE_LITE_CALL(napi_create_external(env, data, nullptr, nullptr, &result));
  return result;
}

/*static*/ int32_t NodeApi::GetValueInt32(napi_env env, napi_value value) {
  int32_t result{};
  NODE_LITE_CALL(napi_get_value_int32(env, value, &result));
  return result;
}

/*static*/ uint32_t NodeApi::GetValueUInt32(napi_env env, napi_value value) {
  uint32_t result{};
  NODE_LITE_CALL(napi_get_value_uint32(env, value, &result));
  return result;
}

/*static*/ void* NodeApi::GetValueExternal(napi_env env, napi_value value) {
  void* result{};
  NODE_LITE_CALL(napi_get_value_external(env, value, &result));
  return result;
}

/*static*/ bool NodeApi::HasProperty(napi_env env,
                                     napi_value obj,
                                     std::string_view utf8_name) {
  bool result{};
  NODE_LITE_CALL(napi_has_named_property(env, obj, utf8_name.data(), &result));
  return result;
}

/*static*/ napi_value NodeApi::GetProperty(napi_env env,
                                           napi_value obj,
                                           std::string_view utf8_name) {
  napi_value result{};
  NODE_LITE_CALL(napi_get_named_property(env, obj, utf8_name.data(), &result));
  return result;
}

/*static*/ std::string NodeApi::GetPropertyString(napi_env env,
                                                  napi_value obj,
                                                  std::string_view utf8_name) {
  if (HasProperty(env, obj, utf8_name)) {
    return ToStdString(env, GetProperty(env, obj, utf8_name));
  } else {
    return "";
  }
}

/*static*/ int32_t NodeApi::GetPropertyInt32(napi_env env,
                                             napi_value obj,
                                             std::string_view utf8_name) {
  return GetValueInt32(env, GetProperty(env, obj, utf8_name));
}

/*static*/ std::string NodeApi::CoerceToString(napi_env env, napi_value value) {
  napi_value str_value;
  NODE_LITE_CALL(napi_coerce_to_string(env, value, &str_value));
  return ToStdString(env, str_value);
}

/*static*/ void NodeApi::SetProperty(napi_env env,
                                     napi_value obj,
                                     std::string_view utf8_name,
                                     napi_value value) {
  NODE_LITE_CALL(napi_set_named_property(env, obj, utf8_name.data(), value));
}

/*static*/ void NodeApi::SetPropertyUInt32(napi_env env,
                                           napi_value obj,
                                           std::string_view utf8_name,
                                           uint32_t value) {
  SetProperty(env, obj, utf8_name, CreateUInt32(env, value));
}

/*static*/ void NodeApi::SetPropertyString(napi_env env,
                                           napi_value obj,
                                           std::string_view utf8_name,
                                           std::string_view value) {
  SetProperty(env, obj, utf8_name, CreateString(env, value));
}

/*static*/ void NodeApi::SetPropertyStringArray(
    napi_env env,
    napi_value obj,
    std::string_view utf8_name,
    std::vector<std::string> const& value) {
  SetProperty(env, obj, utf8_name, CreateStringArray(env, value));
}

/*static*/ void NodeApi::SetPropertyNull(napi_env env,
                                         napi_value obj,
                                         std::string_view utf8_name) {
  SetProperty(env, obj, utf8_name, GetNull(env));
}

/*static*/ void NodeApi::SetMethod(napi_env env,
                                   napi_value obj,
                                   std::string_view utf8_name,
                                   NodeApiCallback cb) {
  NodeApi::SetProperty(env, obj, utf8_name, CreateFunction(env, utf8_name, cb));
}

/*static*/ bool NodeApi::DeleteProperty(napi_env env,
                                        napi_value obj,
                                        std::string_view utf8_name) {
  bool result{};
  NODE_LITE_CALL(
      napi_delete_property(env, obj, CreateString(env, utf8_name), &result));
  return result;
}

/*static*/ std::string NodeApi::ToStdString(napi_env env, napi_value value) {
  size_t str_size{};
  NODE_LITE_CALL(napi_get_value_string_utf8(env, value, nullptr, 0, &str_size));
  std::string result(str_size, '\0');
  NODE_LITE_CALL(napi_get_value_string_utf8(
      env, value, &result[0], str_size + 1, nullptr));
  return result;
}

/*static*/ std::vector<std::string> NodeApi::ToStdStringArray(
    napi_env env, napi_value value) {
  std::vector<std::string> result;
  bool is_array;
  NODE_LITE_CALL(napi_is_array(env, value, &is_array));
  if (is_array) {
    uint32_t length;
    NODE_LITE_CALL(napi_get_array_length(env, value, &length));
    result.reserve(length);
    for (uint32_t i = 0; i < length; i++) {
      napi_value element;
      NODE_LITE_CALL(napi_get_element(env, value, i, &element));
      result.push_back(CoerceToString(env, element));
    }
  }
  return result;
}

/*static*/ napi_value NodeApi::RunScript(napi_env env, napi_value script) {
  napi_value result{};
  NODE_LITE_CALL(napi_run_script(env, script, &result));
  return result;
}

/*static*/ napi_value NodeApi::RunScript(napi_env env,
                                         const std::string& code,
                                         char const* source_url) {
  napi_value script = NodeApi::CreateString(env, code);

  if (source_url != nullptr) {
    napi_value result{};
    NODE_LITE_CALL(jsr_run_script(env, script, source_url, &result));
    return result;
  }
  return RunScript(env, script);
}

/*static*/ napi_valuetype NodeApi::TypeOf(napi_env env, napi_value value) {
  napi_valuetype result{};
  NODE_LITE_CALL(napi_typeof(env, value, &result));
  return result;
}

/*static*/ napi_value NodeApi::CallFunction(napi_env env,
                                            napi_value func,
                                            span<napi_value> args) {
  napi_value result{};
  NODE_LITE_CALL(napi_call_function(
      env, GetUndefined(env), func, args.size(), args.data(), &result));
  return result;
}

/*static*/ napi_value NodeApi::CreateFunction(napi_env env,
                                              std::string_view name,
                                              NodeApiCallback cb) {
  napi_value result{};
  NODE_LITE_CALL(napi_create_function(
      env,
      name.data(),
      name.size(),
      [](napi_env env, napi_callback_info info) {
        napi_value result{};
        ThrowJSErrorOnException(env, [env, info, &result]() {
          NodeApiCallbackInfo callback_info{env, info};
          NodeApiCallback* cb =
              static_cast<NodeApiCallback*>(callback_info.data());
          result = (*cb)(env, callback_info.args());
        });
        return result;
      },
      // TODO: (vmoroz) Find a way to delete it on close.
      new NodeApiCallback(std::move(cb)),
      &result));
  return result;
}

}  // namespace node_api_tests

int main(int argc, char* argv[]) {
  node_api_tests::NodeLiteRuntime::Run(
      std::vector<std::string>(argv, argv + argc));
}
