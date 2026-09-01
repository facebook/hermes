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

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace facebook {
namespace hermes {
namespace {

using Message = std::shared_ptr<jsi::Serialized>;
/// Stores some resources shared between a specific Worker and the main
/// thread/event loop thread. Everything in this struct is guarded by the state
/// mutex;
struct WorkerState {
  /// Mutex to guard all the resources in this WorkerState
  std::mutex stateMutex{};
  /// Used to put the Worker thread to sleep when there's nothing to do in the
  /// Worker event loop, and signals the thread should wake up for processing.
  std::condition_variable toWorkerCondition{};
  /// Once true, worker will be terminated at the earliest convenience.
  bool terminated{false};
  /// Messages to Worker from the parent runtime.
  std::deque<Message> toWorkerQueue;
};

/// Worker-specific Native state, used to mark an Object as a Worker instance.
/// These Worker instances are modeled after HTML WebWorkers.
/// Workers will create a new Hermes Runtime and a new thread upon creation to
/// execute the user-provided Javascript code. It also communicates with the
/// parent runtime through message queues, which are guarded by a mutex. Upon
/// Worker termination (via GC or terminate/close calls), the Worker will quit
/// its execution and the thread will be joined.
class WorkerNativeState : public jsi::NativeState {
 public:
  WorkerNativeState(
      std::shared_ptr<WorkerState> workerState,
      std::unique_ptr<jsi::Runtime> workerRuntime)
      : workerState(std::move(workerState)),
        workerRuntime(std::move(workerRuntime)) {}
  ~WorkerNativeState();

  /// Start and assign the Worker a new thread to run \p script using the Worker
  /// runtime.
  void startWorkerThread(std::string script);

  /// State specific to the Worker. This has shared ownership between the
  /// Worker Native State, the Worker thread, and event-loop task (which can run
  /// after the Worker has been GC'd).
  std::shared_ptr<WorkerState> workerState;

  /// Worker runtime. Created when the Worker is created and used to execute all
  /// JS for the Worker.
  std::unique_ptr<jsi::Runtime> workerRuntime;

 private:
  /// Worker thread used to run all JS execution and event loop processing in
  /// the worker. When the Worker is terminated, the thread is joined.
  std::thread workerThread_;
};

/// Returns true if \p is a Worker instance, false otherwise.
inline bool isWorkerInstance(jsi::Runtime &rt, const jsi::Value &self) {
  return self.isObject() &&
      self.asObject(rt).hasNativeState<WorkerNativeState>(rt);
}

/// Retrieves the \p handlerName property on \p object if it exists as a
/// Callable. Otherwise, return undefined.
jsi::Value getHandler(
    jsi::Runtime &rt,
    const jsi::Object &obj,
    const jsi::PropNameID &handlerName) {
  auto handlerRes = obj.getProperty(rt, handlerName);
  if (!handlerRes.isObject() || !handlerRes.asObject(rt).isFunction(rt)) {
    return jsi::Value::undefined();
  }
  return handlerRes;
}

/// Deserializes the serialized \p message into the provided \p runtime. Then,
/// call the \p handler with the deserialized value as the argument.
void processMessageWithHandler(
    jsi::Runtime &rt,
    Message &&message,
    const jsi::Function &handler) {
  auto serializationInterface = jsi::castInterface<jsi::ISerialization>(&rt);
  assert(serializationInterface && "ISerialization is not supported");
  jsi::Value deserialized = serializationInterface->deserialize(message);
  handler.call(rt, deserialized);
}

/// Acquires the \p workerState.mutex and resources in \p workerState to
/// indicate the Worker is terminated. Request the \p workerRuntime to stop
/// execution. If \p notifyWorker is called, then also notifies the worker to
/// wake up. May be called by the Worker thread or main thread/event loop
/// thread, and may be called multiple times.
void setTerminationState(
    const std::shared_ptr<WorkerState> &workerState,
    jsi::Runtime &workerRuntime,
    bool notifyWorker) {
  std::lock_guard<std::mutex> lock(workerState->stateMutex);
  if (workerState->terminated) {
    // Already terminated. Just return.
    return;
  }

  // Set the terminated flag. In the Worker's event-loop, it will check for
  // this flag and exit the event-loop.
  workerState->terminated = true;

  // Request the Worker runtime to terminate the execution at a convenient
  // time. The timeout exception thrown doesn't matter because the Worker
  // will be terminated.
  auto *hermesInterface = jsi::castInterface<IHermes>(&workerRuntime);
  assert(hermesInterface && "IHermes is not supported");
  hermesInterface->asyncTriggerTimeout();

  // Once terminated, no messages will be processed by the Worker. Discard
  // queue.
  workerState->toWorkerQueue.clear();

  if (notifyWorker) {
    workerState->toWorkerCondition.notify_all();
  }
}

WorkerNativeState::~WorkerNativeState() {
  setTerminationState(workerState, *workerRuntime, true);
  // If the Worker thread is still active, wait for it to finish.
  if (workerThread_.joinable()) {
    workerThread_.join();
  }
}

void WorkerNativeState::startWorkerThread(std::string script) {
  // workerRuntime will outlive this lambda scope because we explicitly join
  // the worker thread in the WorkerNativeState destructor. Thus, it is safe
  // to capture the workerRuntime here.
  workerThread_ = std::thread([scriptCopy = std::move(script),
                               workerRuntime = workerRuntime.get(),
                               workerState = workerState]() {
    try {
      workerRuntime->evaluateJavaScript(
          std::make_unique<jsi::StringBuffer>(std::move(scriptCopy)), "");
      std::unique_lock<std::mutex> lock(workerState->stateMutex);

      // While the Worker isn't terminated, it is allowed to run the event loop.
      while (!workerState->terminated) {
        // 1. Worker thread went to sleep, and another thread has terminated
        // the worker. Worker should wake up and terminate. The message queue is
        // cleared when `terminate` is called. Thus, we also need to check the
        // `terminated` flag to make sure the thread doesn't immediately go back
        // to sleep.
        // 2. Worker thread went to sleep, then another thread posted a message.
        // Wake up to process the message.
        workerState->toWorkerCondition.wait(lock, [workerState] {
          return workerState->terminated || !workerState->toWorkerQueue.empty();
        });
        // If the Worker thread woke up because the Worker was terminated,
        // then break out of the event loop.
        if (workerState->terminated) {
          break;
        }

        // Otherwise, process the next message on the queue, which is guaranteed
        // to exist.
        Message message = std::move(workerState->toWorkerQueue.front());
        workerState->toWorkerQueue.pop_front();
        lock.unlock();

        // Everything from now on can be processed without the lock as it
        // doesn't rely on any shared resources between threads.
        auto workerGlobal = workerRuntime->global();
        jsi::Value onMessage = getHandler(
            *workerRuntime,
            workerGlobal,
            jsi::PropNameID::forAscii(*workerRuntime, "onmessage"));
        if (LLVM_LIKELY(!onMessage.isUndefined())) {
          auto onMessageFunc =
              onMessage.asObject(*workerRuntime).asFunction(*workerRuntime);
          processMessageWithHandler(
              *workerRuntime, std::move(message), onMessageFunc);
        }

        // Lock again for the next tick.
        lock.lock();
      }
    } catch (const jsi::JSIException &) {
      // Script failed. End worker thread.
      // This is different from HTML behavior, where Worker report these
      // error events, and an 'onerror' handler can be attached to the
      // Worker to handle the event. However, that event handling is not
      // implemented yet.
      setTerminationState(workerState, *workerRuntime, false);
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
  auto workerState = std::make_shared<WorkerState>();
  auto workerNativeState = std::make_shared<WorkerNativeState>(
      workerState, std::move(workerRuntime));
  self.setNativeState(rt, workerNativeState);

  // Start the worker thread
  std::string script = args[1].asString(rt).utf8(rt);
  workerNativeState->startWorkerThread(std::move(script));
  return jsi::Value::undefined();
}
/// This implements the `terminate` method of the Worker object, which takes in
/// no arguments. This method marks the Worker as terminated, requests the
/// Worker to stop execution, and clears all messages.
jsi::Value terminateWorker(
    jsi::Runtime &rt,
    const jsi::Value &self,
    const jsi::Value *args,
    size_t count) {
  if (LLVM_UNLIKELY(!isWorkerInstance(rt, self))) {
    throwTypeError(rt, "'this' object must be a Worker");
  }
  auto worker = self.asObject(rt).getNativeState<WorkerNativeState>(rt);
  setTerminationState(worker->workerState, *worker->workerRuntime, true);
  return jsi::Value::undefined();
}

/// This implements the `postMessage` method of the Worker object that sends a
/// message to the Worker. The arguments in \p args must be provided in the
/// following order:
/// 1. A serializable message
/// 2. [not implemented yet] An optional array of transferable arguments
jsi::Value postMessageToWorker(
    jsi::Runtime &rt,
    const jsi::Value &self,
    const jsi::Value *args,
    size_t count) {
  if (LLVM_UNLIKELY(!isWorkerInstance(rt, self))) {
    throwTypeError(rt, "'this' object should be a Worker.");
  }
  if (LLVM_UNLIKELY(count == 0)) {
    throwTypeError(rt, "Must provide a message to post to Worker.");
  }

  // This is safe because the isWorkerInstance check above checks that a
  // WorkerNativeState is attached.
  auto workerNs = self.asObject(rt).getNativeState<WorkerNativeState>(rt);
  auto workerState = workerNs->workerState;
  {
    std::lock_guard<std::mutex> lock(workerState->stateMutex);
    if (workerState->terminated) {
      // Worker is terminated, just return and don't try to serialize.
      return jsi::Value::undefined();
    }
  }
  // Serialize outside the lock because serialization can run some JS
  // and try to acquire the lock as a side effect.
  auto *serializationInterface = jsi::castInterface<jsi::ISerialization>(&rt);
  assert(serializationInterface && "ISerialization not supported");
  auto serialized = serializationInterface->serialize(args[0]);

  {
    // Accessing Worker's shared state. Acquire the lock.
    std::lock_guard<std::mutex> lock(workerState->stateMutex);
    // We need to perform the termination check again since there is a chance
    // the Worker was terminated while serialization.
    if (workerState->terminated) {
      return jsi::Value::undefined();
    }

    workerState->toWorkerQueue.push_back(std::move(serialized));
    // Notify the Worker to wake up and process this message.
    workerState->toWorkerCondition.notify_all();
  }
  return jsi::Value::undefined();
}

} // namespace

void installWorker(jsi::Runtime &rt, jsi::Object &extensions) {
  // Set up function specified in `11-Worker.js`.
  jsi::Function setup = extensions.getPropertyAsFunction(rt, "Worker");

  jsi::Function initWorker = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "initWorker"), 2, initializeWorker);

  jsi::Function terminateWorkerFunc = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "terminateWorker"), 0, terminateWorker);

  jsi::Function postMessageFunc = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "postMessage"), 1, postMessageToWorker);

  setup.call(rt, initWorker, terminateWorkerFunc, postMessageFunc);
}

} // namespace hermes
} // namespace facebook
#endif
