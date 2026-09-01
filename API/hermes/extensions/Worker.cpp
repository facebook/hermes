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
#include "llvh/Support/ErrorHandling.h"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include "hermes/Platform/Logging.h"

namespace facebook {
namespace hermes {
namespace {

using Message = std::
    variant<std::shared_ptr<jsi::Serialized>, std::unique_ptr<jsi::Serialized>>;
/// Stores some resources shared between a specific Worker and the main
/// thread/event loop thread. Everything in this struct is guarded by the state
/// mutex;
struct WorkerState {
  WorkerState(jsi::Runtime &parentRuntime, const jsi::Object &workerObject)
      : weakWorker(
            std::make_unique<jsi::WeakObject>(parentRuntime, workerObject)),
        parentRuntime(parentRuntime) {}
  /// Mutex to guard all the resources in this WorkerState
  std::mutex stateMutex{};
  /// Used to put the Worker thread to sleep when there's nothing to do in the
  /// Worker event loop, and signals the thread should wake up for processing.
  std::condition_variable toWorkerCondition{};
  /// Once true, worker will be terminated at the earliest convenience.
  bool terminated{false};
  /// Messages to Worker from the parent runtime.
  std::deque<Message> toWorkerQueue;
  /// Messages from the Worker to the parent runtime.
  std::deque<Message> fromWorkerQueue;
  /// Errors encountered by the Worker thread
  std::deque<Message> workerErrorQueue;
  /// A WeakRef of the JS Worker Object. This is used by the event loop to
  /// process messages from the Worker. We use a WeakRef here because a Worker
  /// can be terminated but previous tasks may still be scheduled, which
  /// shouldn't prevent clean up of the Worker if it is ready for GC. Upon
  /// termination, this will be reset.
  std::unique_ptr<jsi::WeakObject> weakWorker;
  /// Parent runtime that created this Worker. Used to register Workers
  /// for the event loop.
  jsi::Runtime &parentRuntime;
  /// The ID assigned when the Worker was registered with the integrator
  /// event-loop.
  uint64_t id;
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
  if (auto serializedNoTransfer =
          std::get_if<std::shared_ptr<jsi::Serialized>>(&message)) {
    jsi::Value deserialized =
        serializationInterface->deserialize(*serializedNoTransfer);
    handler.call(rt, deserialized);
  } else if (
      auto serializedWithTransfer =
          std::get_if<std::unique_ptr<jsi::Serialized>>(&message)) {
    jsi::Array deserialized = serializationInterface->deserializeWithTransfer(
        *serializedWithTransfer);

    // 'deserializeWithTransfer' must return the deserialized message at the
    // index 0 of the return JS Array. Thus, the size must not be 0.
    assert(
        deserialized.size(rt) != 0 &&
        "deserializeWithTransfer must contain the message in the array");
    handler.call(rt, deserialized.getValueAtIndex(rt, 0));
  } else {
    llvm_unreachable("Unknown serialization type encountered");
  }
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
  workerState->fromWorkerQueue.clear();
  workerState->workerErrorQueue.clear();

  workerState->weakWorker.reset();

  auto *setEventLoopControlInterface =
      jsi::castInterface<ISetEventLoopControl>(&workerState->parentRuntime);
  assert(
      setEventLoopControlInterface && "ISetEventLoopControl is not supported");
  auto *eventLoopControl = setEventLoopControlInterface->getEventLoopControl();
  if (LLVM_LIKELY(eventLoopControl)) {
    eventLoopControl->unregisterTaskQueueSource(workerState->id);
  }

  if (notifyWorker) {
    workerState->toWorkerCondition.notify_all();
  }
}

/// Scheduled as a task on the integrator's event loop to process a single
/// message sent from the Worker via `postMessage`. Dequeues the message from
/// the shared `fromWorkerQueue`, resolves the Worker's `onmessage` handler,
/// and process the message using the handler.
/// This may be called while the Worker is terminating itself in the Worker
/// thread. In this case, there is no guarantee that the next message will be
/// processed by this function call.
void processMessageFromWorker(const std::shared_ptr<WorkerState> &workerState) {
  std::unique_lock<std::mutex> stateLock(workerState->stateMutex);
  if (workerState->terminated) {
    // By the time this task is running, the Worker has been terminated, so
    // no message should be processed.
    return;
  }
  // There must be a message to be processed, otherwise this task wouldn't
  // have been scheduled.
  assert(
      !workerState->fromWorkerQueue.empty() &&
      "Processing non-existent message");
  Message serialized = std::move(workerState->fromWorkerQueue.front());
  workerState->fromWorkerQueue.pop_front();

  // weakWorker is safe to dereference because it is only reset when the Worker
  // is terminated. However, we currently hold the worker state lock and checked
  // the worker is not terminated.
  auto worker = workerState->weakWorker->lock(workerState->parentRuntime);
  if (LLVM_UNLIKELY(worker.isUndefined())) {
    /// The Worker object is not valid anymore, no message processing will be
    /// done.
    return;
  }
  auto workerObj = worker.asObject(workerState->parentRuntime);

  // At this point, we've checked for termination and obtained the message.
  // We don't care if the Worker is terminated from the Worker thread while
  // the event is being processed. The actual event processing needs to
  // happen outside the lock because it can run JS.
  stateLock.unlock();

  auto onMessageRes = getHandler(
      workerState->parentRuntime,
      workerObj,
      jsi::PropNameID::forAscii(workerState->parentRuntime, "onmessage"));
  if (LLVM_UNLIKELY(onMessageRes.isUndefined())) {
    return;
  }
  auto onMessageHandler = onMessageRes.asObject(workerState->parentRuntime)
                              .asFunction(workerState->parentRuntime);
  processMessageWithHandler(
      workerState->parentRuntime, std::move(serialized), onMessageHandler);
}

/// Install the 'postMessage` global function on the \p workerRuntime. The
/// 'postMessage' function will serialize the provided message and transfer
/// values, and queue the message in \p workerState.fromWorkerQueue. It will
/// also schedule a task using the ISetEventLoop functionality of \p
/// workerState.parentRuntime
void installPostMessageFromWorker(
    jsi::Runtime &workerRuntime,
    const std::shared_ptr<WorkerState> &workerState) {
  // Native Function that handles the `postMessage` calls from Worker to send
  // message to the event loop.
  auto postMessageFromWorker = [workerState = workerState](
                                   jsi::Runtime &runtime,
                                   const jsi::Value &,
                                   const jsi::Value *args,
                                   size_t count) {
    if (LLVM_UNLIKELY(count == 0)) {
      throwTypeError(runtime, "Must provide a message to postMessage");
    }
    auto *setEventLoopControlInterface =
        jsi::castInterface<ISetEventLoopControl>(&workerState->parentRuntime);
    assert(
        setEventLoopControlInterface &&
        "ISetEventLoopControl is not supported");
    auto *eventLoopControl =
        setEventLoopControlInterface->getEventLoopControl();

    // No integrator-provided way for the Worker to schedule a task for message
    // processing, so this message will get lost anyway. Just return.
    if (LLVM_UNLIKELY(!eventLoopControl)) {
      return jsi::Value::undefined();
    }

    {
      std::lock_guard<std::mutex> lock(workerState->stateMutex);
      if (workerState->terminated) {
        // Worker is terminated, just return and don't try to serialize.
        return jsi::Value::undefined();
      }
    }
    // Serialization happens outside the lock because  serialization can run
    // some JS and try to acquire the lock as a side effect.

    auto serializationInterface =
        jsi::castInterface<jsi::ISerialization>(&runtime);
    assert(serializationInterface && "ISerialization is not supported");

    Message serialized;
    const jsi::Value &message = args[0];
    if (count == 1) {
      serialized = serializationInterface->serialize(message);
    } else {
      // Check the 'transfers' argument is an Array
      const jsi::Value &transfers = args[1];
      if (LLVM_UNLIKELY(
              !transfers.isObject() ||
              !transfers.asObject(runtime).isArray(runtime))) {
        throwTypeError(
            runtime, "Must provide an Array of transferable arguments");
      }
      serialized = serializationInterface->serializeWithTransfer(
          message, transfers.asObject(runtime).asArray(runtime));
    }

    {
      // Lock again to access the shared worker state.
      std::lock_guard<std::mutex> lock(workerState->stateMutex);
      // We need to perform the termination check again since there is a chance
      // the Worker was terminated while serialization.
      if (workerState->terminated) {
        return jsi::Value::undefined();
      }

      workerState->fromWorkerQueue.push_back(std::move(serialized));
    }

    // Schedule a task for the event-loop to check the message we just queued
    eventLoopControl->scheduleTask([workerState = workerState]() {
      processMessageFromWorker(workerState);
    });
    return jsi::Value::undefined();
  };

  jsi::Function onMessage = jsi::Function::createFromHostFunction(
      workerRuntime,
      jsi::PropNameID::forAscii(workerRuntime, "postMessage"),
      1,
      postMessageFromWorker);
  workerRuntime.global().setProperty(workerRuntime, "postMessage", onMessage);
}

/// Install the 'close` global function on the \p workerRuntime. The close
/// function will acquire the mutex \p workerState.stateMutex, set the
/// termination flag, and clear all messages.
void installCloseFromWorker(
    jsi::Runtime &workerRuntime,
    const std::shared_ptr<WorkerState> &workerState) {
  // Native Function that handles the `close` calls from inside the Worker to
  // terminate the Worker.
  auto closeFromWorker = [workerState = workerState](
                             jsi::Runtime &runtime,
                             const jsi::Value &,
                             const jsi::Value *args,
                             size_t) {
    setTerminationState(workerState, runtime, false);
    return jsi::Value::undefined();
  };
  auto closePropId = jsi::PropNameID::forAscii(workerRuntime, "close");
  jsi::Function close = jsi::Function::createFromHostFunction(
      workerRuntime, closePropId, 0, closeFromWorker);
  workerRuntime.global().setProperty(workerRuntime, closePropId, close);
}

/// A helper function called from the Worker thread to post \p error encountered
/// by the Worker runtime, and schedule an error processign task. This will
/// acquire the \p workerState.mutex to add the message.
void postError(
    jsi::Runtime &runtime,
    const jsi::Value &error,
    const std::shared_ptr<WorkerState> &workerState) {
  auto errorHandlingTask = [workerState = workerState]() {
    std::unique_lock<std::mutex> stateLock(workerState->stateMutex);
    if (workerState->terminated) {
      // By the time this task is running, the Worker has been terminated, so
      // error handling shouldn't matter
      return;
    }
    // There must be an error to be processed, otherwise this task wouldn't
    // have been scheduled.
    assert(
        !workerState->workerErrorQueue.empty() &&
        "Processing non-existent error");
    Message serialized = std::move(workerState->workerErrorQueue.front());
    workerState->workerErrorQueue.pop_front();
    auto worker = workerState->weakWorker->lock(workerState->parentRuntime);
    if (LLVM_UNLIKELY(worker.isUndefined())) {
      /// The Worker object is not valid anymore, no message processing will be
      /// done.
      return;
    }
    auto workerObj = worker.asObject(workerState->parentRuntime);
    stateLock.unlock();

    auto onErrorRes = getHandler(
        workerState->parentRuntime,
        workerObj,
        jsi::PropNameID::forAscii(workerState->parentRuntime, "onerror"));
    if (LLVM_UNLIKELY(onErrorRes.isUndefined())) {
      return;
    }
    auto onErrorHandler = onErrorRes.asObject(workerState->parentRuntime)
                              .asFunction(workerState->parentRuntime);
    processMessageWithHandler(
        workerState->parentRuntime, std::move(serialized), onErrorHandler);
  };

  auto *setEventLoopControlInterface =
      jsi::castInterface<ISetEventLoopControl>(&workerState->parentRuntime);
  assert(
      setEventLoopControlInterface && "ISetEventLoopControl is not supported");
  auto *eventLoopControl = setEventLoopControlInterface->getEventLoopControl();

  // No integrator-provided way for the Worker to schedule a task to process the
  // error, so this message will get lost anyway. Just return.
  if (LLVM_UNLIKELY(!eventLoopControl)) {
    return;
  }

  auto serializationInterface =
      jsi::castInterface<jsi::ISerialization>(&runtime);
  assert(serializationInterface && "ISerialization is not supported");
  std::shared_ptr<jsi::Serialized> serialized;

  try {
    serialized = serializationInterface->serialize(error);
  } catch (const jsi::JSError &error) {
    /// If we encounter an error while serializing the original JSError, then
    /// give up
    ::hermes::hermesLog("HermesWorker", "Failed to serialize Worker error.");
    return;
  }

  {
    std::lock_guard<std::mutex> lock(workerState->stateMutex);
    if (workerState->terminated) {
      // Worker was terminated after the Error has been serialized, don't post
      // the message.
      return;
    }
    workerState->workerErrorQueue.push_back(std::move(serialized));
  }

  eventLoopControl->scheduleTask(errorHandlingTask);
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
    } catch (const jsi::JSError &scriptError) {
      postError(*workerRuntime, scriptError.value(), workerState);
    } catch (const jsi::JSINativeException &) {
      /// evaluateJavaScript can also throw JSINativeException, which isn't
      /// serializable. In this case, just terminate the Worker.
      ::hermes::hermesLog(
          "HermesWorker",
          "Encountered JSINativeException while running Worker script.");
      setTerminationState(workerState, *workerRuntime, false);
      return;
    }

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

      // Everything from now on can be processed without the lock as it doesn't
      // rely on any shared resources between threads.
      auto workerGlobal = workerRuntime->global();
      jsi::Value onMessage = getHandler(
          *workerRuntime,
          workerGlobal,
          jsi::PropNameID::forAscii(*workerRuntime, "onmessage"));
      if (LLVM_LIKELY(!onMessage.isUndefined())) {
        try {
          auto onMessageFunc =
              onMessage.asObject(*workerRuntime).asFunction(*workerRuntime);
          processMessageWithHandler(
              *workerRuntime, std::move(message), onMessageFunc);
        } catch (const jsi::JSError &error) {
          // Error processing the message, post the error for the parent to
          // handle.
          postError(*workerRuntime, error.value(), workerState);
        }
      }
      // Lock again for the next tick.
      lock.lock();
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
  auto workerState = std::make_shared<WorkerState>(rt, self);

  // Install the event-processing handlers onto the Worker runtime
  installPostMessageFromWorker(*workerRuntime, workerState);
  installCloseFromWorker(*workerRuntime, workerState);

  auto workerNativeState = std::make_shared<WorkerNativeState>(
      workerState, std::move(workerRuntime));
  self.setNativeState(rt, workerNativeState);

  auto *setEventLoopControlInterface =
      jsi::castInterface<ISetEventLoopControl>(&workerState->parentRuntime);
  assert(
      setEventLoopControlInterface && "ISetEventLoopControl is not supported");
  auto *eventLoopControl = setEventLoopControlInterface->getEventLoopControl();
  if (LLVM_LIKELY(eventLoopControl)) {
    workerState->id = eventLoopControl->registerTaskQueueSource();
  }

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
/// 2. An optional array of transferable arguments
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
  Message serialized;
  const jsi::Value &message = args[0];
  if (count == 1) {
    serialized = serializationInterface->serialize(message);
  } else {
    // Check the 'transfers' argument is an Array
    const jsi::Value &transfers = args[1];
    if (LLVM_UNLIKELY(
            !transfers.isObject() || !transfers.asObject(rt).isArray(rt))) {
      throw jsi::JSError(rt, "Must provide an Array of transferable arguments");
    }
    serialized = serializationInterface->serializeWithTransfer(
        message, transfers.asObject(rt).asArray(rt));
  }

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
