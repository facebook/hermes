/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/static_h.h"
#include "hermes/hermes.h"
#include "jsi/jsi.h"

#include "llvh/ADT/DenseSet.h"

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace {

/// A jsi::MutableBuffer backed by a std::vector<uint8_t>.
class VectorMutableBuffer : public facebook::jsi::MutableBuffer {
 public:
  explicit VectorMutableBuffer(std::vector<uint8_t> data)
      : data_(std::move(data)) {}

  size_t size() const override {
    return data_.size();
  }
  uint8_t *data() override {
    return data_.data();
  }

 private:
  std::vector<uint8_t> data_;
};

/// Throw a JS TypeError with the given message via JSI.
[[noreturn]] static void throwTypeError(
    facebook::jsi::Runtime &rt,
    const char *msg) {
  auto typeErrorCtor = rt.global().getPropertyAsFunction(rt, "TypeError");
  auto errObj = typeErrorCtor
                    .callAsConstructor(
                        rt, facebook::jsi::String::createFromAscii(rt, msg))
                    .asObject(rt);
  throw facebook::jsi::JSError(rt, std::move(errObj));
}

} // namespace

// Simple EventLoopControl that just adds the tasks to a queue when
// `scheduleTask` is called and tracks the number of sources. When the queue
// is empty, users can also wait for new tasks to be added.
class EventLoopControl final : public facebook::hermes::IEventLoopControl {
  void scheduleTask(const std::function<void()> &callback) override {
    std::lock_guard<std::mutex> lock(mutex_);
    tasks_.push(callback);
    cv_.notify_all();
  }

  uint64_t registerTaskQueueSource() override {
    std::lock_guard<std::mutex> lock(mutex_);
    sourceCount_++;
    sources_.insert(sourceCount_);
    return sourceCount_;
  }

  void unregisterTaskQueueSource(uint64_t sourceId) override {
    std::lock_guard<std::mutex> lock(mutex_);
    sources_.erase(sourceId);
    if (sources_.empty()) {
      cv_.notify_all();
    }
  }

 public:
  ~EventLoopControl() = default;
  /// Drain all tasks from the queue, blocking until there are no more tasks
  /// and no more active sources that could schedule new tasks.
  void drainTasks() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (true) {
      if (!tasks_.empty()) {
        // There are tasks to run. Grab the next one and run it.
        auto task = tasks_.front();
        tasks_.pop();
        lock.unlock();
        // Run the task outside the lock. Otherwise, the running task may
        // also try to queue more tasks, causing a deadlock.
        task();
        lock.lock();
      } else if (!sources_.empty()) {
        // No tasks to run, but there are still active sources that may
        // schedule tasks. Sleep until woken by `scheduleTask` or
        // `decrementTaskQueueSource`
        cv_.wait(lock);
      } else {
        // No tasks and no sources at this point, exit the event loop.
        break;
      }
    }
  }

 private:
  // Callbacks are added to the queue from the Worker thread and removed from
  // the main thread. Guard with a mutex.
  std::mutex mutex_{};
  // Used to wake up the event-loop if it is waiting for a new task or if all
  // task sources have disappeared.
  std::condition_variable cv_;
  // All schedules tasks.
  std::queue<std::function<void()>> tasks_{};
  // Counter of total number of sources. Incremented when register is called.
  uint64_t sourceCount_{0};
  // The set of active sources
  llvh::DenseSet<uint64_t> sources_{};
};

/// Object that contains console state that needs to be preserved between
/// init_console_bindings and run_event_loop.
struct SHConsoleContext {
  /// The JS object that contains the console helpers from the
  /// ConsoleBindings.js.inc file.
  facebook::jsi::Object helpers;

  /// Script arguments passed after "--" on the command line.
  int scriptArgc;
  const char *const *scriptArgv;

  EventLoopControl eventLoopControl{};
  facebook::hermes::HermesRuntime *hermesRuntime{nullptr};

  SHConsoleContext(
      facebook::jsi::Object &&helpers,
      int scriptArgc,
      const char *const *scriptArgv)
      : helpers(std::move(helpers)),
        scriptArgc(scriptArgc),
        scriptArgv(scriptArgv) {}

  ~SHConsoleContext() {
    if (hermesRuntime) {
      auto *iface =
          facebook::jsi::castInterface<facebook::hermes::ISetEventLoopControl>(
              hermesRuntime);
      iface->setEventLoopControl(nullptr);
    }
  }
};

/// Init harness symbols used in the test262 testsuite:
/// - $test262, with properties:
///   - global
///   - evalScript
///   - detachArrayBuffer
/// - alert
static void initTest262Bindings(facebook::hermes::HermesRuntime &hrt) {
  auto global = hrt.global();
  facebook::jsi::Object test262Obj{hrt};

  // Get Object.defineProperty(), use it to define property below.
  auto definePropertyFn = hrt.global()
                              .getProperty(hrt, "Object")
                              .asObject(hrt)
                              .getProperty(hrt, "defineProperty")
                              .asObject(hrt)
                              .asFunction(hrt);

  facebook::jsi::Object descriptor{hrt};
  descriptor.setProperty(hrt, "enumerable", false);
  descriptor.setProperty(hrt, "writable", true);
  descriptor.setProperty(hrt, "configurable", true);
  auto valueProp = facebook::jsi::PropNameID::forAscii(hrt, "value");

  // Define $262.global.
  descriptor.setProperty(hrt, valueProp, global);
  definePropertyFn.call(hrt, test262Obj, "global", descriptor);

  // Define $262.evalScript.
  auto evalFunc = global.getProperty(hrt, "eval");
  descriptor.setProperty(hrt, valueProp, evalFunc);
  definePropertyFn.call(hrt, test262Obj, "evalScript", descriptor);

  // Define $262.detachArrayBuffer.
  auto hermesInternalProp = global.getProperty(hrt, "HermesInternal");
  if (hermesInternalProp.isObject()) {
    auto hermesInternalObj = hermesInternalProp.asObject(hrt);
    auto detachArrayBufferFunc =
        hermesInternalObj.getProperty(hrt, "detachArrayBuffer");
    if (detachArrayBufferFunc.isObject()) {
      descriptor.setProperty(hrt, valueProp, detachArrayBufferFunc);
      definePropertyFn.call(hrt, test262Obj, "detachArrayBuffer", descriptor);
    }
  }

  // Define global object $262.
  descriptor.setProperty(hrt, valueProp, test262Obj);
  definePropertyFn.call(hrt, global, "$262", descriptor);

  // Define global function alert().
  auto printFunc = global.getProperty(hrt, "print");
  descriptor.setProperty(hrt, valueProp, printFunc);
  definePropertyFn.call(hrt, global, "alert", descriptor);

  // Define console.log.
  facebook::jsi::Object consoleObj{hrt};
  definePropertyFn.call(hrt, consoleObj, "log", descriptor);
  descriptor.setProperty(hrt, valueProp, consoleObj);
  definePropertyFn.call(hrt, global, "console", descriptor);
}

/// Register the hermescli global object with loadFile, loadHBC, and
/// getScriptArgs, gated on -Xhermes-internal-test-methods.
static void initHermesCLIBindings(
    facebook::hermes::HermesRuntime &hrt,
    SHConsoleContext *ctx) {
  using namespace facebook;

  // Check if -Xhermes-internal-test-methods is active by probing for
  // HermesInternal.detachArrayBuffer (only present with that flag).
  auto hi = hrt.global().getProperty(hrt, "HermesInternal");
  bool testMethods = hi.isObject() &&
      hi.asObject(hrt).getProperty(hrt, "detachArrayBuffer").isObject();
  if (!testMethods)
    return;

  jsi::Object hermescliObj{hrt};

  // hermescli.loadFile(path) -> Uint8Array
  hermescliObj.setProperty(
      hrt,
      "loadFile",
      jsi::Function::createFromHostFunction(
          hrt,
          jsi::PropNameID::forAscii(hrt, "loadFile"),
          1,
          [](jsi::Runtime &rt,
             const jsi::Value &,
             const jsi::Value *args,
             size_t count) -> jsi::Value {
            if (count < 1 || !args[0].isString())
              throwTypeError(rt, "loadFile requires a string path argument");

            std::string path = args[0].getString(rt).utf8(rt);
            FILE *f = std::fopen(path.c_str(), "rb");
            if (!f) {
              std::string msg = "Failed to open file: " + path;
              throwTypeError(rt, msg.c_str());
            }

            std::fseek(f, 0, SEEK_END);
            long len = std::ftell(f);
            std::fseek(f, 0, SEEK_SET);

            std::vector<uint8_t> data(len);
            std::fread(data.data(), 1, len, f);
            std::fclose(f);

            // Create an ArrayBuffer with the file contents.
            auto arrayBuf =
                std::make_shared<VectorMutableBuffer>(std::move(data));
            jsi::ArrayBuffer ab{rt, std::move(arrayBuf)};

            // Call new Uint8Array(arrayBuffer) to return a Uint8Array.
            auto uint8ArrayCtor =
                rt.global().getPropertyAsFunction(rt, "Uint8Array");
            return uint8ArrayCtor.callAsConstructor(rt, std::move(ab));
          }));

  // hermescli.loadHBC(buffer) -> result of evaluating the bytecode
  hermescliObj.setProperty(
      hrt,
      "loadHBC",
      jsi::Function::createFromHostFunction(
          hrt,
          jsi::PropNameID::forAscii(hrt, "loadHBC"),
          1,
          [](jsi::Runtime &rt,
             const jsi::Value &,
             const jsi::Value *args,
             size_t count) -> jsi::Value {
            if (count < 1 || !args[0].isObject())
              throwTypeError(
                  rt, "loadHBC requires a Uint8Array or ArrayBuffer argument");

            jsi::Object obj = args[0].getObject(rt);
            uint8_t *data = nullptr;
            size_t len = 0;

            if (obj.isArrayBuffer(rt)) {
              // Direct ArrayBuffer.
              jsi::ArrayBuffer ab = obj.getArrayBuffer(rt);
              data = ab.data(rt);
              len = ab.size(rt);
            } else {
              // Try TypedArray: access .buffer, .byteOffset, .byteLength.
              auto bufProp = obj.getProperty(rt, "buffer");
              if (!bufProp.isObject() ||
                  !bufProp.asObject(rt).isArrayBuffer(rt))
                throwTypeError(
                    rt,
                    "loadHBC requires a Uint8Array or ArrayBuffer argument");
              jsi::ArrayBuffer ab = bufProp.asObject(rt).getArrayBuffer(rt);
              auto byteOffset = obj.getProperty(rt, "byteOffset");
              auto byteLength = obj.getProperty(rt, "byteLength");
              size_t offset = byteOffset.isNumber()
                  ? static_cast<size_t>(byteOffset.getNumber())
                  : 0;
              len = byteLength.isNumber()
                  ? static_cast<size_t>(byteLength.getNumber())
                  : ab.size(rt);
              data = ab.data(rt) + offset;
            }

            // Copy the data and evaluate as bytecode/JS.
            auto strBuf = std::make_shared<jsi::StringBuffer>(
                std::string(reinterpret_cast<const char *>(data), len));
            try {
              return rt.evaluateJavaScript(strBuf, "loadHBC");
            } catch (jsi::JSIException &) {
              throwTypeError(rt, "Error deserializing bytecode");
            }
          }));

  // hermescli.getScriptArgs() -> Array of strings
  hermescliObj.setProperty(
      hrt,
      "getScriptArgs",
      jsi::Function::createFromHostFunction(
          hrt,
          jsi::PropNameID::forAscii(hrt, "getScriptArgs"),
          0,
          [ctx](
              jsi::Runtime &rt, const jsi::Value &, const jsi::Value *, size_t)
              -> jsi::Value {
            auto arr = jsi::Array(rt, ctx->scriptArgc);
            for (int i = 0; i < ctx->scriptArgc; ++i) {
              arr.setValueAtIndex(
                  rt, i, jsi::String::createFromAscii(rt, ctx->scriptArgv[i]));
            }
            return arr;
          }));

  hrt.global().setProperty(hrt, "hermescli", std::move(hermescliObj));
}

/// The JS library that implements the event loop.
static const char *s_jslib =
#include "ConsoleBindings.js.inc"
    ;

/// \return a SHConsoleContext initialized with the console bindings.
///   Must be freed by free_console_context.
extern "C" SHERMES_EXPORT SHConsoleContext *init_console_bindings(
    SHRuntime *shr,
    int scriptArgc,
    const char *const *scriptArgv) {
  using namespace facebook;
  auto &hrt = *_sh_get_hermes_runtime(shr);
  initTest262Bindings(hrt);

  auto consoleContext = std::make_unique<SHConsoleContext>(
      hrt.evaluateJavaScript(
             std::make_unique<jsi::StringBuffer>(s_jslib),
             "ConsoleBindings.js.inc")
          .asObject(hrt),
      scriptArgc,
      scriptArgv);

  initHermesCLIBindings(hrt, consoleContext.get());

  consoleContext->hermesRuntime = &hrt;
  auto *setEventLoopInterface =
      jsi::castInterface<facebook::hermes::ISetEventLoopControl>(&hrt);
  setEventLoopInterface->setEventLoopControl(&consoleContext->eventLoopControl);

  jsi::Object &helpers = consoleContext->helpers;

  jsi::Function runMacroTask = helpers.getPropertyAsFunction(hrt, "run");

  // There are no pending tasks, but we want to initialize the event loop
  // current time.
  {
    double curTimeMs =
        (double)std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count();
    runMacroTask.call(hrt, curTimeMs);
  }

  return consoleContext.release();
}

/// Free the \p consoleContext.
extern "C" SHERMES_EXPORT void free_console_context(
    SHConsoleContext *consoleContext) {
  delete consoleContext;
}

/// Run the event loop, using \p consoleContext to manage the event queue.
/// Prints uncaught errors to stderr.
/// \return true on success, false on error.
extern "C" SHERMES_EXPORT bool run_event_loop(
    SHRuntime *shr,
    SHConsoleContext *consoleContext) {
  using namespace facebook;
  auto &hrt = *_sh_get_hermes_runtime(shr);

  try {
    // Register event loop functions and obtain the runMicroTask() helper
    // function.
    jsi::Object &helpers = consoleContext->helpers;
    jsi::Function peekMacroTask = helpers.getPropertyAsFunction(hrt, "peek");
    jsi::Function runMacroTask = helpers.getPropertyAsFunction(hrt, "run");

    double nextTimeMs;

    hrt.drainMicrotasks();

    // This is the event loop. Loop while there are pending tasks.
    while ((nextTimeMs = peekMacroTask.call(hrt).getNumber()) >= 0) {
      auto now = std::chrono::steady_clock::now();
      double curTimeMs =
          (double)std::chrono::duration_cast<std::chrono::milliseconds>(
              now.time_since_epoch())
              .count();

      // If we have to, sleep until the next task is ready.
      if (nextTimeMs > curTimeMs) {
        std::this_thread::sleep_until(
            now +
            std::chrono::milliseconds((int_least64_t)(nextTimeMs - curTimeMs)));

        // Update the current time because we slept.
        curTimeMs =
            (double)std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch())
                .count();
      }

      // Run the next task.
      runMacroTask.call(hrt, curTimeMs);
      hrt.drainMicrotasks();
    }

    // Run all tasks queued in the event loop.
    consoleContext->eventLoopControl.drainTasks();
  } catch (jsi::JSError &e) {
    // Handle JS exceptions here.
    fprintf(stderr, "JS Exception: %s\n", e.what());
    return false;
  } catch (jsi::JSIException &e) {
    // Handle JSI exceptions here.
    fprintf(stderr, "JS Exception: %s\n", e.what());
    return false;
  }

  return true;
}
