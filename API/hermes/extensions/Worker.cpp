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
#include "hermes/Support/Base64.h"
#include "hermes/hermes.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/Compiler.h"
#include "llvh/Support/ErrorHandling.h"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

#include "hermes/Platform/Logging.h"

namespace facebook {
namespace hermes {
namespace {

using Message = std::
    variant<std::shared_ptr<jsi::Serialized>, std::unique_ptr<jsi::Serialized>>;

/// The script source handed to the worker thread. Exactly one of the two forms
/// is active: eager bytes already materialized (source string, decoded data:
/// URL, or a copied buffer input), or a URL to resolve on the worker thread.
struct WorkerScriptSource {
  std::shared_ptr<const jsi::Buffer> eagerBuffer;
  std::string url;
  bool needsResolve{false};
};

/// Return the integrator's worker setup registered on \p rt, or
/// nullptr if none. Obtained via the runtime's ISetWorkerSetup interface.
IWorkerSetup *getWorkerSetup(jsi::Runtime &rt) {
  if (auto *setter = jsi::castInterface<ISetWorkerSetup>(&rt))
    if (jsi::ICast *provider = setter->getWorkerSetup())
      return jsi::castInterface<IWorkerSetup>(provider);
  return nullptr;
}
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

  /// Start the worker thread. \p source is eager bytes or a URL to resolve;
  /// \p provider (may be null) supplies resolveScript/initWorkerRuntime.
  void startWorkerThread(WorkerScriptSource source, IWorkerSetup *provider);

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
  } catch (const jsi::JSError &) {
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

void WorkerNativeState::startWorkerThread(
    WorkerScriptSource source,
    IWorkerSetup *provider) {
  workerThread_ = std::thread([source = std::move(source),
                               provider,
                               workerRuntime = workerRuntime.get(),
                               workerState = workerState]() {
    try {
      std::shared_ptr<const jsi::Buffer> buffer;
      if (source.needsResolve) {
        assert(provider && "URL input requires a provider");
        std::string error;
        buffer = provider->resolveScript(source.url, error);
        if (!buffer) {
          throw jsi::JSError(
              *workerRuntime,
              error.empty() ? std::string("Failed to load worker script")
                            : error);
        }
        if (buffer->size() == 0) {
          // An empty script is a TypeError per the design.
          throw jsi::JSError(
              *workerRuntime,
              workerRuntime->global()
                  .getPropertyAsFunction(*workerRuntime, "TypeError")
                  .callAsConstructor(
                      *workerRuntime,
                      jsi::String::createFromUtf8(
                          *workerRuntime,
                          "Cannot create Worker from empty worker script")));
        }
      } else {
        buffer = source.eagerBuffer;
      }

      if (provider) {
        provider->initWorkerRuntime(*workerRuntime);
      }

      // Use the resolved URL (if any) as the source identifier so worker
      // stack traces and debugger locations reference it. Empty for the eager
      // (source string / buffer / data:) paths.
      workerRuntime->evaluateJavaScript(buffer, source.url);
    } catch (const jsi::JSError &scriptError) {
      postError(*workerRuntime, scriptError.value(), workerState);
    } catch (const jsi::JSINativeException &) {
      ::hermes::hermesLog(
          "HermesWorker",
          "Encountered JSINativeException while running Worker script.");
      setTerminationState(workerState, *workerRuntime, false);
      return;
    } catch (const std::exception &e) {
      ::hermes::hermesLog(
          "HermesWorker",
          "Encountered C++ exception while starting Worker: %s",
          e.what());
      setTerminationState(workerState, *workerRuntime, false);
      return;
    } catch (...) {
      ::hermes::hermesLog(
          "HermesWorker",
          "Encountered unknown exception while starting Worker.");
      setTerminationState(workerState, *workerRuntime, false);
      return;
    }

    std::unique_lock<std::mutex> lock(workerState->stateMutex);
    while (!workerState->terminated) {
      workerState->toWorkerCondition.wait(lock, [workerState] {
        return workerState->terminated || !workerState->toWorkerQueue.empty();
      });
      if (workerState->terminated) {
        break;
      }
      Message message = std::move(workerState->toWorkerQueue.front());
      workerState->toWorkerQueue.pop_front();
      lock.unlock();

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
          postError(*workerRuntime, error.value(), workerState);
        } catch (const jsi::JSINativeException &) {
          ::hermes::hermesLog(
              "HermesWorker",
              "Encountered JSINativeException while processing Worker "
              "message.");
          setTerminationState(workerState, *workerRuntime, false);
          return;
        } catch (const std::exception &e) {
          ::hermes::hermesLog(
              "HermesWorker",
              "Encountered C++ exception while processing Worker message: %s",
              e.what());
          setTerminationState(workerState, *workerRuntime, false);
          return;
        } catch (...) {
          ::hermes::hermesLog(
              "HermesWorker",
              "Encountered unknown exception while processing Worker "
              "message.");
          setTerminationState(workerState, *workerRuntime, false);
          return;
        }
      }
      lock.lock();
    }
  });
}

/// Throw a TypeError if \p ab is detached. Call this before querying a
/// buffer's size or a view's byteOffset/byteLength, or copying its data: on a
/// detached buffer ArrayBuffer::size()/data() throw a non-TypeError exception,
/// and the DataView byteOffset/byteLength getters throw as well. Checking up
/// front yields a consistent "detached buffer" TypeError.
void checkBufferAttached(jsi::Runtime &rt, const jsi::ArrayBuffer &ab) {
  if (ab.detached(rt)) {
    throwTypeError(rt, "Cannot create Worker from a detached buffer");
  }
}

/// Copy \p length bytes starting at \p offset of \p ab into a std::string.
/// Throws TypeError if the range is empty. std::string carries arbitrary
/// binary faithfully (no UTF-8 re-encoding).
/// \pre \p ab is attached; callers must call checkBufferAttached first.
std::string copyBufferBytes(
    jsi::Runtime &rt,
    jsi::ArrayBuffer ab,
    size_t offset,
    size_t length) {
  if (length == 0) {
    throwTypeError(rt, "Cannot create Worker from empty binary input");
  }
  const uint8_t *data = ab.data(rt);
  return std::string(reinterpret_cast<const char *>(data + offset), length);
}

/// Create the Worker runtime/thread and attach state to \p self. \p source is
/// either eager bytes or a URL to resolve on the worker thread; \p provider
/// (may be null) supplies
/// resolveScript/initWorkerRuntime/configureWorkerRuntime.
void startWorker(
    jsi::Runtime &rt,
    jsi::Object self,
    WorkerScriptSource source,
    IWorkerSetup *provider) {
  auto *api = jsi::castInterface<IHermesRootAPI>(makeHermesRootAPI());
  // Seed a default config; let the integrator adjust it in place.
  ::hermes::vm::RuntimeConfig workerConfig;
  if (provider) {
    provider->configureWorkerRuntime(workerConfig);
  }
  auto workerRuntime = api->makeHermesRuntime(workerConfig);
  auto workerState = std::make_shared<WorkerState>(rt, self);

  // Propagate the provider to the worker runtime so a nested `new Worker`
  // created inside this worker inherits it. `provider` is an
  // IWorkerSetup*, i.e. a jsi::ICast*; re-casting it on the child
  // still reaches every interface the object implements.
  if (provider) {
    auto *childSetter =
        jsi::castInterface<ISetWorkerSetup>(workerRuntime.get());
    assert(childSetter && "ISetWorkerSetup is not supported");
    childSetter->setWorkerSetup(provider);
  }

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

  workerNativeState->startWorkerThread(std::move(source), provider);
}

/// Percent-decode \p in into \p out. Returns false on a malformed escape.
bool percentDecode(llvh::StringRef in, std::string &out) {
  out.clear();
  auto hex = [](char c) -> int {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    return -1;
  };
  for (size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 2 >= in.size())
        return false;
      int hi = hex(in[i + 1]), lo = hex(in[i + 2]);
      if (hi < 0 || lo < 0)
        return false;
      out.push_back(static_cast<char>((hi << 4) | lo));
      i += 2;
    } else {
      out.push_back(in[i]);
    }
  }
  return true;
}

/// If \p url is a data: URL, decode its payload into \p out and return true.
/// Return false if \p url is not a data: URL. Throw a TypeError (via
/// throwTypeError) for a malformed data: URL. Format:
/// data:[<mediatype>][;base64],<payload>
bool decodeDataUrl(jsi::Runtime &rt, const std::string &url, std::string &out) {
  llvh::StringRef ref(url);
  // Strip any URL fragment: everything from the first literal '#'. A literal
  // '#' in the body must be percent-encoded (%23), which is preserved because
  // it is not a literal '#'.
  size_t hash = ref.find('#');
  if (hash != llvh::StringRef::npos) {
    ref = ref.take_front(hash);
  }
  // The URL scheme is case-insensitive (RFC 3986); accept e.g. "DATA:".
  if (!ref.startswith_lower("data:"))
    return false;
  ref = ref.drop_front(5); // after "data:" (5 chars regardless of case)
  size_t comma = ref.find(',');
  if (comma == llvh::StringRef::npos) {
    throwTypeError(rt, "Malformed data: URL (missing comma)");
  }
  llvh::StringRef meta = ref.take_front(comma);
  llvh::StringRef payload = ref.drop_front(comma + 1);
  // Per the WHATWG data: URL processor, percent-decode the body first (for both
  // plain and base64 URLs).
  if (!percentDecode(payload, out)) {
    throwTypeError(rt, "Malformed data: URL payload");
  }
  // An empty body decodes to empty bytes; skip base64 (hermes::base64Decode
  // must not be called with empty input) and let the caller report the empty
  // worker TypeError.
  if (out.empty()) {
    return true;
  }
  // The ";base64" token is case-insensitive per RFC 2397. For base64 URLs,
  // base64-decode the (already percent-decoded) body with the vetted Support
  // decoder.
  if (meta.endswith_lower(";base64")) {
    llvh::Optional<std::string> b64 = ::hermes::base64Decode(out);
    if (!b64) {
      throwTypeError(rt, "Malformed data: URL payload");
    }
    out = std::move(*b64);
  }
  return true;
}

/// Build a WorkerScriptSource from a string argument \p str per the option
/// flags and whether a \p provider is present.
WorkerScriptSource sourceFromString(
    jsi::Runtime &rt,
    std::string str,
    bool inlineFlag,
    bool allowData,
    IWorkerSetup *provider) {
  WorkerScriptSource source;
  std::string decoded;
  if (!inlineFlag && allowData && decodeDataUrl(rt, str, decoded)) {
    if (decoded.empty()) {
      throwTypeError(rt, "Cannot create Worker from empty data: URL");
    }
    source.eagerBuffer =
        std::make_shared<jsi::StringBuffer>(std::move(decoded));
  } else if (provider && !inlineFlag) {
    source.url = std::move(str);
    source.needsResolve = true;
  } else {
    source.eagerBuffer = std::make_shared<jsi::StringBuffer>(std::move(str));
  }
  return source;
}

jsi::Value initializeWorker(
    jsi::Runtime &rt,
    const jsi::Value &,
    const jsi::Value *args,
    size_t count) {
  // Called only by 11-Worker.js: (self, script, inline, allowData).
  assert(count == 4);
  auto self = args[0].asObject(rt);
  const jsi::Value &input = args[1];
  bool inlineFlag = args[2].getBool();
  bool allowData = args[3].getBool();

  IWorkerSetup *provider = getWorkerSetup(rt);

  if (input.isString()) {
    WorkerScriptSource source = sourceFromString(
        rt, input.asString(rt).utf8(rt), inlineFlag, allowData, provider);
    startWorker(rt, std::move(self), std::move(source), provider);
    return jsi::Value::undefined();
  }

  if (input.isObject()) {
    jsi::Object obj = input.asObject(rt);
    std::string bytes;
    if (obj.isArrayBuffer(rt)) {
      jsi::ArrayBuffer ab = obj.getArrayBuffer(rt);
      checkBufferAttached(rt, ab);
      size_t size = ab.size(rt);
      bytes = copyBufferBytes(rt, std::move(ab), 0, size);
    } else if (obj.isTypedArray(rt)) {
      jsi::TypedArray ta = obj.getTypedArray(rt);
      jsi::ArrayBuffer ab = ta.buffer(rt);
      checkBufferAttached(rt, ab);
      bytes = copyBufferBytes(
          rt, std::move(ab), ta.byteOffset(rt), ta.byteLength(rt));
    } else if (isDataView(rt, obj)) {
      jsi::ArrayBuffer ab = dataViewBuffer(rt, obj);
      checkBufferAttached(rt, ab);
      bytes = copyBufferBytes(
          rt,
          std::move(ab),
          dataViewByteOffset(rt, obj),
          dataViewByteLength(rt, obj));
    } else {
      // Non-buffer object: coerce to string via ToString (invokes toString /
      // Symbol.toPrimitive), matching the web's USVString coercion, so an RN
      // URL is used as its href. Reclassify the result as a string.
      std::string str = input.toString(rt).utf8(rt);
      WorkerScriptSource source =
          sourceFromString(rt, std::move(str), inlineFlag, allowData, provider);
      startWorker(rt, std::move(self), std::move(source), provider);
      return jsi::Value::undefined();
    }
    WorkerScriptSource source;
    source.eagerBuffer = std::make_shared<jsi::StringBuffer>(std::move(bytes));
    startWorker(rt, std::move(self), std::move(source), provider);
    return jsi::Value::undefined();
  }

  throwTypeError(
      rt,
      "Worker script must be a string, ArrayBuffer, TypedArray, or DataView");
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
      rt, jsi::PropNameID::forAscii(rt, "initWorker"), 4, initializeWorker);

  jsi::Function terminateWorkerFunc = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "terminateWorker"), 0, terminateWorker);

  jsi::Function postMessageFunc = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "postMessage"), 1, postMessageToWorker);

  setup.call(rt, initWorker, terminateWorkerFunc, postMessageFunc);
}

} // namespace hermes
} // namespace facebook
#endif
