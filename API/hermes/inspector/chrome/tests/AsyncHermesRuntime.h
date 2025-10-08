/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>

#include <hermes/Support/SerialExecutor.h>

#include <hermes/hermes.h>
#include <jsi/jsi.h>

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {

/// URL assigned to scripts being executed in the absense of a caller-specified
/// URL.
constexpr auto kDefaultUrl = "url";

/**
 * AsyncHermesRuntime is a helper class that runs JS scripts in a Hermes VM on
 * a separate thread. This is useful for tests that want to test running JS
 * in a multithreaded environment.
 */
class AsyncHermesRuntime {
 public:
  // Create a runtime. If veryLazy, configure the runtime to use completely
  // lazy compilation.
  AsyncHermesRuntime(bool veryLazy = false);
  ~AsyncHermesRuntime();

  std::shared_ptr<HermesRuntime> runtime() {
    return runtime_;
  }

  /**
   * stop sets the stop flag on this instance. JS scripts can get the current
   * value of the stop flag by calling the global shouldStop() function.
   */
  void stop();

  /**
   * start unsets the stop flag on this instance. JS scripts can get the current
   * value of the stop flag by calling the global shouldStop() function.
   */
  void start();

  /**
   * hasStoredValue returns whether or not a value has been stored yet
   */
  bool hasStoredValue();

  /**
   * awaitStoredValue is a helper for getStoredValue that returns the value
   * synchronously rather than in a future.
   */
  jsi::Value awaitStoredValue(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(2500));

  /**
   * tickleJsAsync evaluates '__tickleJs()' in the underlying Hermes runtime on
   * a separate thread.
   */
  void tickleJsAsync();

  /**
   * executeScriptAsync evaluates JS in the underlying Hermes runtime on a
   * separate thread.
   *
   * This method should be called at most once during the lifetime of an
   * AsyncHermesRuntime instance.
   */
  void executeScriptAsync(
      const std::string &str,
      const std::string &url = kDefaultUrl,
      facebook::hermes::HermesRuntime::DebugFlags flags =
          facebook::hermes::HermesRuntime::DebugFlags{});

  /**
   * executeScriptSync evaluates JS in the underlying Hermes runtime on a
   * separate thread. It will block the caller until execution completes. If
   * this takes longer than \p timeout, an exception will be thrown.
   */
  void executeScriptSync(
      const std::string &script,
      const std::string &url = kDefaultUrl,
      facebook::hermes::HermesRuntime::DebugFlags flags =
          facebook::hermes::HermesRuntime::DebugFlags{},
      std::chrono::milliseconds timeout = std::chrono::milliseconds(2500));

  /// Evaluates the given bytecode in the underlying Hermes runtime on a
  /// separate thread.
  /// \param bytecode Bytecode compiled with compileJS() API
  /// \param url Corresponding source URL
  void evaluateBytecodeAsync(
      const std::string &bytecode,
      const std::string &url = "url");

  /**
   * wait blocks until all previous executeScriptAsync calls finish.
   */
  void wait(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(2500));

  /**
   * returns the number of thrown exceptions.
   */
  size_t getNumberOfExceptions();

  /**
   * returns the message of the last thrown exception.
   */
  std::string getLastThrownExceptionMessage();

  /**
   * registers the runtime for profiling in the executor thread.
   */
  void registerForProfilingInExecutor();

  /**
   * unregisters the runtime for profiling in the executor thread.
   */
  void unregisterForProfilingInExecutor();

 private:
  jsi::Value shouldStop(
      jsi::Runtime &runtime,
      const jsi::Value &thisVal,
      const jsi::Value *args,
      size_t count);

  jsi::Value storeValue(
      jsi::Runtime &runtime,
      const jsi::Value &thisVal,
      const jsi::Value *args,
      size_t count);

  std::shared_ptr<HermesRuntime> runtime_;
  std::unique_ptr<::hermes::SerialExecutor> executor_;
  std::atomic<bool> stopFlag_{};
  std::promise<jsi::Value> storedValue_;
  bool hasStoredValue_{false};
  std::vector<std::string> thrownExceptions_;
};

/// RAII-style class dealing with sampling profiler registration in tests. This
/// is especially important in tests -- if any test failure is caused by an
/// uncaught exception, stack unwinding will destroy a VM registered for
/// profiling in a thread that's not the one where registration happened, which
/// will lead to a hermes fatal error. Using this RAII class ensure that the
/// proper test failure cause is reported.
struct SamplingProfilerRAII {
  explicit SamplingProfilerRAII(AsyncHermesRuntime &rt) : runtime_(rt) {
    runtime_.registerForProfilingInExecutor();
  }

  ~SamplingProfilerRAII() {
    runtime_.unregisterForProfilingInExecutor();
  }

  AsyncHermesRuntime &runtime_;
};
} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook
