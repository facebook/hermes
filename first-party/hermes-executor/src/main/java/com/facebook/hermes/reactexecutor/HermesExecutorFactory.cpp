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
#include <hermes/hermes.h>
#include <jsi/threadsafe.h>

#ifdef HERMES_ENABLE_DEBUGGER
#include <hermes/inspector/RuntimeAdapter.h>
#include <hermes/inspector/chrome/Registration.h>
#endif

using namespace facebook::hermes;

namespace facebook {
namespace react {

namespace {

std::unique_ptr<HermesRuntime> makeHermesRuntimeSystraced(
    const ::hermes::vm::RuntimeConfig &runtimeConfig) {
  SystraceSection s("HermesExecutorFactory::makeHermesRuntimeSystraced");
  return hermes::makeHermesRuntime(runtimeConfig);
}

struct ThreadCheck {
  ThreadCheck() : id_(std::this_thread::get_id()) {}

  void lock(const jsi::Runtime &) const {
    if (id_ != std::this_thread::get_id()) {
      __builtin_trap();
    }
  }

  void unlock(const jsi::Runtime &) const {}

 private:
  std::thread::id id_;
};

class ThreadCheckedHermesRuntime
    : public jsi::detail::ThreadSafeRuntimeImpl<jsi::Runtime &, ThreadCheck> {
 public:
  ThreadCheckedHermesRuntime(std::unique_ptr<HermesRuntime> runtime)
      : ThreadSafeRuntimeImpl(*runtime), runtime_(std::move(runtime)) {}

  HermesRuntime &getHermesRuntime() {
    return *runtime_;
  }

 private:
  std::unique_ptr<HermesRuntime> runtime_;
};

std::shared_ptr<HermesRuntime> makeThreadCheckedHermesRuntime(
    const ::hermes::vm::RuntimeConfig &runtimeConfig) {
  auto tchr = std::make_shared<ThreadCheckedHermesRuntime>(
      makeHermesRuntimeSystraced(runtimeConfig));
  return std::shared_ptr<HermesRuntime>(tchr, &(tchr->getHermesRuntime()));
}

#ifdef HERMES_ENABLE_DEBUGGER

class HermesExecutorRuntimeAdapter
    : public facebook::hermes::inspector::RuntimeAdapter {
 public:
  HermesExecutorRuntimeAdapter(
      std::shared_ptr<HermesRuntime> runtime,
      std::shared_ptr<MessageQueueThread> thread)
      : runtime_(std::move(runtime)), thread_(std::move(thread)) {}

  virtual ~HermesExecutorRuntimeAdapter() = default;

  HermesRuntime &getRuntime() override {
    return *runtime_;
  }

  void tickleJs() override {
    auto runtime = runtime_;
    thread_->runOnQueue([runtime]() {
      auto func =
          runtime->global().getPropertyAsFunction(*runtime, "__tickleJs");
      func.call(*runtime);
    });
  }

 private:
  std::shared_ptr<HermesRuntime> runtime_;
  std::shared_ptr<MessageQueueThread> thread_;
};

#endif

} // namespace

std::unique_ptr<JSExecutor> HermesExecutorFactory::createJSExecutor(
    std::shared_ptr<ExecutorDelegate> delegate,
    std::shared_ptr<MessageQueueThread> jsQueue) {
  auto runtime = makeThreadCheckedHermesRuntime(runtimeConfig_);
  // Add js engine information to Error.prototype so in error reporting we
  // can send this information.
  auto errorPrototype = runtime->global()
                            .getPropertyAsObject(*runtime.get(), "Error")
                            .getPropertyAsObject(*runtime.get(), "prototype");
  errorPrototype.setProperty(*runtime.get(), "jsEngine", "hermes");

  return std::make_unique<HermesExecutor>(
      std::move(runtime),
      delegate,
      jsQueue,
      logger_,
      timeoutInvoker_,
      runtimeInstaller_);
}

HermesExecutor::HermesExecutor(
    std::shared_ptr<HermesRuntime> runtime,
    std::shared_ptr<ExecutorDelegate> delegate,
    std::shared_ptr<MessageQueueThread> jsQueue,
    Logger logger,
    const JSIScopedTimeoutInvoker &timeoutInvoker,
    RuntimeInstaller runtimeInstaller)
    : JSIExecutor(runtime, delegate, logger, timeoutInvoker, runtimeInstaller),
      hermesRuntime_(runtime) {
#ifdef HERMES_ENABLE_DEBUGGER
  auto adapter =
      std::make_unique<HermesExecutorRuntimeAdapter>(runtime, jsQueue);
  facebook::hermes::inspector::chrome::enableDebugging(
      std::move(adapter), "Hermes React Native");
#endif
}

void HermesExecutor::destroy() {
#ifdef HERMES_ENABLE_DEBUGGER
  facebook::hermes::inspector::chrome::disableDebugging(*hermesRuntime_);
#endif

  JSIExecutor::destroy();
}

} // namespace react
} // namespace facebook
