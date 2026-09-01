/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef JSI_UNSTABLE
#include "Worker.h"
#include "Intrinsics.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/hermes.h"
#include "llvh/Support/Compiler.h"

#include <thread>

namespace facebook {
namespace hermes {
namespace {

/// Worker-specific Native state, used to mark an Object as a Worker instance.
/// These Worker instances are modeled after HTML WebWorkers.
/// Workers will create a new Hermes Runtime and a new thread upon creation to
/// execute the user-provided Javascript code. It also communicates with the
/// parent runtime through message queues, which are guarded by a mutex. Upon
/// Worker termination (via GC or terminate/close calls), the Worker will quit
/// its execution and the thread will be joined.
class WorkerNativeState : public jsi::NativeState {
 public:
  WorkerNativeState(std::unique_ptr<jsi::Runtime> workerRuntime)
      : workerRuntime(std::move(workerRuntime)) {};
  ~WorkerNativeState();

  /// Start and assign the Worker a new thread to run \p script using the Worker
  /// runtime.
  void startWorkerThread(std::string script);

  /// Worker runtime. Created when the Worker is created and used to execute all
  /// JS for the Worker.
  std::unique_ptr<jsi::Runtime> workerRuntime;

 private:
  /// Worker thread used to run all JS execution and event loop processing in
  /// the worker. When the Worker is terminated, the thread is joined.
  std::thread workerThread_;
};

/// Request the \p workerRuntime to stop execution. May be called multiple
/// times.
void setTerminationState(jsi::Runtime &workerRuntime) {
  // Request the Worker runtime to terminate the execution at a convenient
  // time. The timeout exception thrown doesn't matter because the Worker
  // will be terminated.
  auto *hermesInterface = jsi::castInterface<IHermes>(&workerRuntime);
  assert(hermesInterface && "IHermes is not supported");
  hermesInterface->asyncTriggerTimeout();
}

WorkerNativeState::~WorkerNativeState() {
  setTerminationState(*workerRuntime);
  // If the Worker thread is still active, wait for it to finish.
  if (workerThread_.joinable()) {
    workerThread_.join();
  }
}

void WorkerNativeState::startWorkerThread(std::string script) {
  // workerRuntime will outlive this lambda scope because we explicitly join
  // the worker thread in the WorkerNativeState destructor. Thus, it is safe
  // to capture the workerRuntime here.
  workerThread_ = std::thread(
      [scriptCopy = std::move(script), workerRuntime = workerRuntime.get()]() {
        try {
          workerRuntime->evaluateJavaScript(
              std::make_unique<jsi::StringBuffer>(std::move(scriptCopy)), "");
        } catch (const jsi::JSIException &) {
          // Script failed. End worker thread.
          // This is different from HTML behavior, where Worker report these
          // error events, and an 'onerror' handler can be attached to the
          // Worker to handle the event in the main thread. However, that event
          // handling is not implemented yet.
          return;
        }
      });
}

/// Called by the JS constructor in `11-Worker.js` to mark the first argument.
/// The arguments in \p args must be provided in the following order:
/// 1. the object to be marked as Worker
/// 2. the script to be executed by the Worker
jsi::Value initializeWorker(
    jsi::Runtime &rt,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  // This is only called by the Worker extension script in `11-Worker.js`, so we
  // can guarantee that the argument count is 2.
  assert(count == 2);

  // Verify the user-provided script argument
  if (LLVM_UNLIKELY(!args[1].isString())) {
    throwTypeError(
        rt, "Must provide the source code as a String for the Worker to run");
  }

  // Mark the self object as a Worker
  // This is provided by the `11-Worker.js` script, so it should always be an
  // Object.
  auto self = args[0].asObject(rt);
  auto *api = jsi::castInterface<IHermesRootAPI>(makeHermesRootAPI());
  auto workerRuntime = api->makeHermesRuntime(::hermes::vm::RuntimeConfig());
  auto workerNativeState =
      std::make_shared<WorkerNativeState>(std::move(workerRuntime));
  self.setNativeState(rt, workerNativeState);

  // Start the worker thread
  std::string script = args[1].asString(rt).utf8(rt);
  workerNativeState->startWorkerThread(std::move(script));
  return jsi::Value::undefined();
}

} // namespace

void installWorker(jsi::Runtime &rt, jsi::Object &extensions) {
  // Set up function specified in `11-Worker.js`.
  jsi::Function setup = extensions.getPropertyAsFunction(rt, "Worker");

  jsi::Function initWorker = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "initWorker"), 2, initializeWorker);

  setup.call(rt, initWorker);
}

} // namespace hermes
} // namespace facebook
#endif
