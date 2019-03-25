/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "HermesExecutorFactory.h"

#include <thread>

#include <cxxreact/MessageQueueThread.h>
#include <cxxreact/SystraceSection.h>
#include <hermes/hermes_tracing.h>
#include <jsi/decorator.h>

#ifdef HERMES_ENABLE_DEBUGGER
#include <hermes/inspector/RuntimeAdapter.h>
#include <hermes/inspector/chrome/Registration.h>
#endif

#include "JSITracing.h"

using namespace facebook::hermes;
using namespace facebook::jsi;

namespace facebook {
namespace react {

namespace {

std::unique_ptr<HermesRuntime> makeHermesRuntimeSystraced(
    const ::hermes::vm::RuntimeConfig &runtimeConfig) {
  SystraceSection s("HermesExecutorFactory::makeHermesRuntimeSystraced");
  return hermes::makeHermesRuntime(runtimeConfig);
}

#ifdef HERMES_ENABLE_DEBUGGER

class HermesExecutorRuntimeAdapter
    : public facebook::hermes::inspector::RuntimeAdapter {
 public:
  HermesExecutorRuntimeAdapter(
      HermesRuntime &runtime,
      std::shared_ptr<MessageQueueThread> thread)
      : runtime_(runtime), thread_(std::move(thread)) {}

  virtual ~HermesExecutorRuntimeAdapter() = default;

  HermesRuntime &getRuntime() override {
    return runtime_;
  }

  void tickleJs() override {
    // The queue will ensure that runtime_ is still valid when this
    // gets invoked.
    // clang-format off
    thread_->runOnQueue([&runtime = runtime_]() {
      // clang-format on
      auto func = runtime.global().getPropertyAsFunction(runtime, "__tickleJs");
      func.call(runtime);
    });
  }

 private:
  HermesRuntime &runtime_;
  std::shared_ptr<MessageQueueThread> thread_;
};

#endif

struct ThreadCheck {
  explicit ThreadCheck(const std::thread::id &id) {
    if (id != std::this_thread::get_id()) {
      __builtin_trap();
    }
  }
};

// This adds ThreadCheck and debugger enable/teardown to the given
// Runtime.
class DecoratedRuntime
    : public jsi::WithRuntimeDecorator<ThreadCheck, std::thread::id> {
 public:
  // The first argument may be a tracing runtime which itself
  // decorates the real HermesRuntime, depending on the build config.
  // The second argument is the the real HermesRuntime as well to
  // manage the debugger registration.
  DecoratedRuntime(
      std::unique_ptr<Runtime> runtime,
      HermesRuntime &hermesRuntime,
      std::shared_ptr<MessageQueueThread> jsQueue)
      : jsi::WithRuntimeDecorator<ThreadCheck, std::thread::id>(
            *runtime,
            threadId_),
        runtime_(std::move(runtime)),
        threadId_(std::this_thread::get_id()),
        hermesRuntime_(hermesRuntime) {
#ifdef HERMES_ENABLE_DEBUGGER
    auto adapter =
        std::make_unique<HermesExecutorRuntimeAdapter>(hermesRuntime_, jsQueue);
    facebook::hermes::inspector::chrome::enableDebugging(
        std::move(adapter), "Hermes React Native");
#else
    (void)hermesRuntime_;
#endif
  }

  ~DecoratedRuntime() {
#ifdef HERMES_ENABLE_DEBUGGER
    facebook::hermes::inspector::chrome::disableDebugging(hermesRuntime_);
#endif
  }

 private:
  // runtime_ is a TracingRuntime, but we don't need to worry about
  // the details.  hermesRuntime is a reference to the HermesRuntime
  // managed by the TracingRuntime.
  //
  // HermesExecutorRuntimeAdapter requirements are kept, because the
  // dtor will disable debugging on the HermesRuntime before the
  // member managing it is destroyed.

  std::unique_ptr<Runtime> runtime_;
  std::thread::id threadId_;
  HermesRuntime &hermesRuntime_;
};

} // namespace

std::unique_ptr<JSExecutor> HermesExecutorFactory::createJSExecutor(
    std::shared_ptr<ExecutorDelegate> delegate,
    std::shared_ptr<MessageQueueThread> jsQueue) {
  std::unique_ptr<HermesRuntime> hermesRuntime =
      makeHermesRuntimeSystraced(runtimeConfig_);
  HermesRuntime &hermesRuntimeRef = *hermesRuntime;
  auto decoratedRuntime = std::make_shared<DecoratedRuntime>(
      makeTracingHermesRuntime(std::move(hermesRuntime), runtimeConfig_),
      hermesRuntimeRef,
      jsQueue);

  // So what do we have now?
  // DecoratedRuntime -> TracingRuntime -> HermesRuntime
  //
  // DecoratedRuntime is held by JSIExecutor.  When it gets used, it
  // will check that it's on the right thread, do any necessary trace
  // logging, then call the real HermesRuntime.  When it is destroyed,
  // it will shut down the debugger before the HermesRuntime is.  In
  // the normal case where tracing and debugging are not compiled in,
  // all that's left is the thread checking.

  // Add js engine information to Error.prototype so in error reporting we
  // can send this information.
  auto errorPrototype =
      decoratedRuntime->global()
          .getPropertyAsObject(*decoratedRuntime, "Error")
          .getPropertyAsObject(*decoratedRuntime, "prototype");
  errorPrototype.setProperty(*decoratedRuntime, "jsEngine", "hermes");

  return std::make_unique<HermesExecutor>(
      decoratedRuntime, delegate, jsQueue, timeoutInvoker_, runtimeInstaller_);
}

HermesExecutor::HermesExecutor(
    std::shared_ptr<jsi::Runtime> runtime,
    std::shared_ptr<ExecutorDelegate> delegate,
    std::shared_ptr<MessageQueueThread> jsQueue,
    const JSIScopedTimeoutInvoker &timeoutInvoker,
    RuntimeInstaller runtimeInstaller)
    : JSIExecutor(runtime, delegate, timeoutInvoker, runtimeInstaller) {
  jsi::addNativeTracingHooks(*runtime);
}

} // namespace react
} // namespace facebook
