/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/static_h.h"
#include "hermes/hermes.h"
#include "jsi/jsi.h"

#include <chrono>
#include <cstdio>
#include <thread>

/// Object that contains console state that needs to be preserved between
/// init_console_bindings and run_event_loop.
struct SHConsoleContext {
  /// The JS object that contains the console helpers from the
  /// ConsoleBindings.js.inc file.
  facebook::jsi::Object helpers;

  SHConsoleContext(facebook::jsi::Object &&helpers)
      : helpers(std::move(helpers)) {}
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

/// The JS library that implements the event loop.
static const char *s_jslib =
#include "ConsoleBindings.js.inc"
    ;

/// \return a SHConsoleContext initialized with the console bindings.
///   Must be freed by free_console_context.
extern "C" SHERMES_EXPORT SHConsoleContext *init_console_bindings(
    SHRuntime *shr) {
  using namespace facebook;
  auto &hrt = *_sh_get_hermes_runtime(shr);
  initTest262Bindings(hrt);

  auto consoleContext = std::make_unique<SHConsoleContext>(
      hrt.evaluateJavaScript(
             std::make_unique<jsi::StringBuffer>(s_jslib),
             "ConsoleBindings.js.inc")
          .asObject(hrt));
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
