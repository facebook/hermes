/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AsyncHermesRuntime.h"

#include <functional>
#include <stdexcept>
#include <thread>

#include <glog/logging.h>

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {

AsyncHermesRuntime::AsyncHermesRuntime(bool veryLazy)
    : executor_(std::make_unique<::hermes::SerialExecutor>()) {
  using namespace std::placeholders;

  auto builder = ::hermes::vm::RuntimeConfig::Builder();
  if (veryLazy) {
    builder.withCompilationMode(::hermes::vm::ForceLazyCompilation);
  }
  runtime_ = facebook::hermes::makeHermesRuntime(builder.build());

  runtime_->global().setProperty(
      *runtime_,
      "shouldStop",
      jsi::Function::createFromHostFunction(
          *runtime_,
          jsi::PropNameID::forAscii(*runtime_, "shouldStop"),
          0,
          std::bind(&AsyncHermesRuntime::shouldStop, this, _1, _2, _3, _4)));

  runtime_->global().setProperty(
      *runtime_,
      "storeValue",
      jsi::Function::createFromHostFunction(
          *runtime_,
          jsi::PropNameID::forAscii(*runtime_, "storeValue"),
          0,
          std::bind(&AsyncHermesRuntime::storeValue, this, _1, _2, _3, _4)));
}

AsyncHermesRuntime::~AsyncHermesRuntime() {
  stop();
  wait();
}

void AsyncHermesRuntime::tickleJsAsync() {
  executor_->add([this]() {
    auto func =
        runtime_->global().getPropertyAsFunction(*runtime_, "__tickleJs");
    func.call(*runtime_);
  });
}

void AsyncHermesRuntime::executeScriptAsync(
    const std::string &script,
    const std::string &url,
    HermesRuntime::DebugFlags flags) {
  int scriptId = rand();
  LOG(INFO) << "AsyncHermesRuntime will execute script with id: " << scriptId
            << ", contents: " << script;

  executor_->add([this, script, url, flags, scriptId] {
    LOG(INFO) << "AsyncHermesRuntime executing script id " << scriptId
              << " in background";

    try {
      runtime_->debugJavaScript(script, url, flags);
    } catch (jsi::JSError &error) {
      LOG(INFO) << "AsyncHermesRuntime JSError " << error.getMessage();
      thrownExceptions_.push_back(error.getMessage());
    }

    LOG(INFO) << "AsyncHermesRuntime finished executing script id " << scriptId;
  });
}

void AsyncHermesRuntime::executeScriptSync(
    const std::string &script,
    const std::string &url,
    HermesRuntime::DebugFlags flags,
    std::chrono::milliseconds timeout) {
  executeScriptAsync(script, url, flags);
  wait(timeout);
}

void AsyncHermesRuntime::evaluateBytecodeAsync(
    const std::string &bytecode,
    const std::string &url) {
  executor_->add([this, bytecode, url] {
    try {
      runtime_->evaluateJavaScript(
          std::unique_ptr<jsi::StringBuffer>(new jsi::StringBuffer(bytecode)),
          url);
    } catch (jsi::JSError &error) {
      thrownExceptions_.push_back(error.getMessage());
    }
  });
}

void AsyncHermesRuntime::start() {
  LOG(INFO) << "AsyncHermesRuntime: set stop flag false";
  stopFlag_.store(false);
}

void AsyncHermesRuntime::stop() {
  LOG(INFO) << "AsyncHermesRuntime: set stop flag true";
  stopFlag_.store(true);
}

bool AsyncHermesRuntime::hasStoredValue() {
  return hasStoredValue_;
}

jsi::Value AsyncHermesRuntime::awaitStoredValue(
    std::chrono::milliseconds timeout) {
  auto future = storedValue_.get_future();
  auto status = future.wait_for(timeout);
  if (status == std::future_status::ready) {
    return future.get();
  }
  throw std::runtime_error("Timeout waiting for stored value");
}

void AsyncHermesRuntime::wait(std::chrono::milliseconds timeout) {
  LOG(INFO) << "AsyncHermesRuntime wait requested";
  auto promise = std::make_shared<std::promise<bool>>();
  auto future = promise->get_future();
  executor_->add([promise] {
    LOG(INFO) << "AsyncHermesRuntime wait resolved";
    promise->set_value(true);
  });
  auto status = future.wait_for(timeout);
  if (status != std::future_status::ready) {
    throw std::runtime_error("wait could not finish within timeout.");
  }
}

jsi::Value AsyncHermesRuntime::shouldStop(
    jsi::Runtime &runtime,
    const jsi::Value &thisVal,
    const jsi::Value *args,
    size_t count) {
  return stopFlag_.load() ? jsi::Value(true) : jsi::Value(false);
}

jsi::Value AsyncHermesRuntime::storeValue(
    jsi::Runtime &runtime,
    const jsi::Value &thisVal,
    const jsi::Value *args,
    size_t count) {
  if (count > 0) {
    storedValue_.set_value((jsi::Value(runtime, args[0])));
    hasStoredValue_ = true;
  }
  return jsi::Value();
}

size_t AsyncHermesRuntime::getNumberOfExceptions() {
  return thrownExceptions_.size();
}

std::string AsyncHermesRuntime::getLastThrownExceptionMessage() {
  return thrownExceptions_.back();
}

void AsyncHermesRuntime::registerForProfilingInExecutor() {
  // Sampling profiler registration needs to happen in the thread where JS runs.
  executor_->add([runtime = runtime_]() { runtime->registerForProfiling(); });

  // Wait until the executor is registered for profiling.
  wait();
}

void AsyncHermesRuntime::unregisterForProfilingInExecutor() {
  // Sampling profiler deregistration needs to happen in the thread where JS
  // runs.
  executor_->add([runtime = runtime_]() { runtime->unregisterForProfiling(); });

  // Wait until the executor is unregistered for profiling.
  wait();
}

} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook
