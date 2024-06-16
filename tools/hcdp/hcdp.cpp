/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ENABLE_DEBUGGER

#include <iostream>

int main(void) {
  std::cout << "hcdp compiled without Hermes debugger enabled." << std::endl;
  return EXIT_FAILURE;
}

#else // defined(HERMES_ENABLE_DEBUGGER)

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <hermes/Parser/JSONParser.h>
#include <hermes/Support/JSONEmitter.h>
#include <hermes/Support/SerialExecutor.h>
#include <hermes/cdp/CDPAgent.h>
#include <hermes/cdp/CDPDebugAPI.h>
#include <hermes/cdp/MessageTypes.h>
#include <hermes/hermes.h>

#include "IPC.h"
#include "JSONHelpers.h"

namespace hermes {

namespace fbhermes = ::facebook::hermes;
namespace cdp = fbhermes::cdp;
namespace message = fbhermes::cdp::message;
using namespace facebook::hermes;
using namespace facebook;
using namespace ::hermes::parser;

namespace {

static uint32_t nextExecutionContextId = 1;

/// Manages a runtime executing a script on a thread.
class RuntimeInstance {
 public:
  /// Create a new CDP-debuggable runtime, and execute the specified
  /// script on a new runtime thread.
  explicit RuntimeInstance(std::string scriptSource, std::string scriptUrl)
      : executor_(std::make_unique<SerialExecutor>()),
        runtime_(
            fbhermes::makeHermesRuntime(::hermes::vm::RuntimeConfig::Builder()
                                            .withEnableSampleProfiling(true)
                                            .build())) {
    cdpDebugAPI_ = cdp::CDPDebugAPI::create(*runtime_);

    // Install console.log handler
    HermesRuntime &hermesRt = *runtime_.get();
    auto console = jsi::Object(hermesRt);
    auto nameID = jsi::PropNameID::forUtf8(hermesRt, "log");
    console.setProperty(
        hermesRt,
        nameID,
        jsi::Function::createFromHostFunction(
            hermesRt,
            nameID,
            1,
            [cdpDebugAPI = cdpDebugAPI_.get(), &hermesRt](
                jsi::Runtime &runtime,
                const jsi::Value &,
                const jsi::Value *argsPtr,
                size_t count) {
              std::vector<jsi::Value> args;
              for (size_t i = 0; i < count; ++i) {
                args.emplace_back(runtime, argsPtr[i]);
              }
              const auto now = std::chrono::system_clock::now();
              double timestamp =
                  std::chrono::duration_cast<std::chrono::seconds>(
                      now.time_since_epoch())
                      .count();
              cdpDebugAPI->addConsoleMessage(
                  {timestamp,
                   cdp::ConsoleAPIType::kLog,
                   std::move(args),
                   hermesRt.getDebugger().captureStackTrace()});
              return jsi::Value::undefined();
            }));
    runtime_->global().setProperty(hermesRt, "console", console);

    executor_->add([this,
                    source = std::move(scriptSource),
                    url = std::move(scriptUrl)]() mutable {
      fbhermes::HermesRuntime::DebugFlags flags{};
      runtime_->debugJavaScript(std::move(source), std::move(url), flags);
    });
  }

  ~RuntimeInstance() {
    // Wait for tasks to complete
    executor_.reset();

    // Tear down the debug API and runtime.
    cdpDebugAPI_.reset();
    runtime_.reset();
  }

  /// Get the underlying Hermes runtime.
  fbhermes::HermesRuntime &runtime() {
    return *runtime_;
  }

  /// Gets the CDP debugging interface for this runtime instance.
  cdp::CDPDebugAPI &cdpDebugAPI() {
    return *cdpDebugAPI_;
  }

  /// Add a task to be executed by the runtime thread.
  void addTask(std::function<void()> task) {
    executor_->add(std::move(task));
  }

 private:
  std::unique_ptr<SerialExecutor> executor_;
  std::unique_ptr<fbhermes::HermesRuntime> runtime_;
  std::unique_ptr<cdp::CDPDebugAPI> cdpDebugAPI_;
};

/// Thread-safe class for storing and later retrieving a message ID
class SynchronizedMessageId {
 public:
  void setId(long long id) {
    std::lock_guard<std::mutex> lock(mutex_);
    id_ = id;
  }

  void clearId() {
    std::lock_guard<std::mutex> lock(mutex_);
    id_ = std::nullopt;
  }

  std::optional<long long> getId() {
    std::lock_guard<std::mutex> lock(mutex_);
    return id_;
  }

 private:
  std::mutex mutex_{};
  std::optional<long long> id_{};
};
} // namespace

/// Read a script from the specified path.
static std::optional<std::string> readScript(const char *path) {
  std::ifstream stream(path);
  if (!stream.is_open()) {
    return std::nullopt;
  }
  return std::string{
      std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

/// Run a script, launching the VM and carrying out CDPAgent sessions as
/// clients connect and disconnect.
static void debugScript(std::string scriptSource, std::string scriptUrl) {
  // Start a runtime thread to execute the script.
  std::unique_ptr<RuntimeInstance> runtimeInstance =
      std::make_unique<RuntimeInstance>(
          std::move(scriptSource), std::move(scriptUrl));

  uint32_t executionContextId = nextExecutionContextId++;

  std::shared_ptr<SynchronizedMessageId> runtimeEnableMessageId =
      std::make_shared<SynchronizedMessageId>();

  // Process IPC messages
  std::unordered_map<ClientID, std::unique_ptr<cdp::CDPAgent>> agents;
  while (std::optional<IPCCommand> ipc = receiveIPC()) {
    ClientID clientID = ipc.value().clientID;
    switch (ipc.value().type) {
      case kConnectIPCType: {
        // Client connected; start an agent for it.
        agents.emplace(
            clientID,
            cdp::CDPAgent::create(
                executionContextId,
                runtimeInstance->cdpDebugAPI(),
                [&runtimeInstance](
                    std::function<void(fbhermes::HermesRuntime &)> task) {
                  // Enqueue any runtime tasks on the runtime thread.
                  fbhermes::HermesRuntime &runtime = runtimeInstance->runtime();
                  runtimeInstance->addTask(
                      [&runtime, task = std::move(task)]() { task(runtime); });
                },
                [clientID, executionContextId, runtimeEnableMessageId](
                    const std::string &message) {
                  // Emit executionContextCreated notification, which is
                  // required for the Console tab to work.
                  if (runtimeEnableMessageId->getId().has_value()) {
                    auto id = getResponseId(message);
                    if (id.has_value() &&
                        id.value() == runtimeEnableMessageId->getId().value()) {
                      message::runtime::ExecutionContextCreatedNotification
                          note;
                      message::runtime::ExecutionContextDescription desc;
                      desc.id = executionContextId;
                      desc.name = "main";
                      note.context = std::move(desc);
                      sendIPC(kMessageIPCType, clientID, note.toJsonStr());
                      runtimeEnableMessageId->clearId();
                    }
                  }
                  // Forward message to the client, via an IPC message.
                  sendIPC(kMessageIPCType, clientID, message);
                }));
      } break;

      case kMessageIPCType: {
        // Message from a client; handle it via the client's agent.
        auto agent = agents.find(clientID);
        if (agent == agents.end()) {
          // Received a message for an agent that was previously-destroyed,
          // or one that never existed.
          throw std::runtime_error("No such agent");
        }
        std::unique_ptr<message::Request> command =
            message::Request::fromJson(ipc.value().message);
        // Save Runtime.enable's message ID so that we could intercept the
        // response and emit executionContextCreated notification.
        if (command->method == "Runtime.enable") {
          runtimeEnableMessageId->setId(command->id);
        }
        agent->second->handleCommand(ipc.value().message);
      } break;

      case kDisconnectIPCType: {
        // Client disconnected; destroy its agent.
        agents.erase(clientID);
      } break;

      default:
        throw std::runtime_error("Unknown IPC type");
    }
  }
}

/// Returns the filename from a given path, or the entire path if no filename
/// delimiter could be found.
static std::string getFilename(const std::string &path) {
  size_t lastSlashIndex = path.find_last_of('/');
  if (lastSlashIndex == std::string::npos) {
    lastSlashIndex = path.find_last_of('\\');
    if (lastSlashIndex == std::string::npos) {
      return path;
    }
  }
  return path.substr(lastSlashIndex + 1);
}

} // namespace hermes

using namespace hermes;

int main(int argc, char **argv) {
  // Unbuffer stdout to avoid storing large lines (e.g. profiling results).
  setbuf(stdout, nullptr);

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " scriptPath" << std::endl;
    return EXIT_FAILURE;
  }
  const char *scriptPath = argv[1];
  std::optional<std::string> script = readScript(scriptPath);
  if (!script) {
    std::cerr << "Failed to read script at " << scriptPath << std::endl;
  }
  std::string filename = getFilename(scriptPath);
  debugScript(std::move(*script), std::move(filename));
  return EXIT_SUCCESS;
}

#endif // defined(HERMES_ENABLE_DEBUGGER)
