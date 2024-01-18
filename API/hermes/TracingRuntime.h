/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TRACINGRUNTIME_H
#define HERMES_TRACINGRUNTIME_H

#include "SynthTrace.h"

#include <hermes/hermes.h>
#include <jsi/decorator.h>
#include "llvh/Support/raw_ostream.h"

namespace facebook {
namespace hermes {
namespace tracing {

class TracingRuntime : public jsi::RuntimeDecorator<jsi::Runtime> {
 public:
  using RD = RuntimeDecorator<jsi::Runtime>;

  TracingRuntime(
      std::unique_ptr<jsi::Runtime> runtime,
      uint64_t globalID,
      const ::hermes::vm::RuntimeConfig &conf,
      std::unique_ptr<llvh::raw_ostream> traceStream);

  virtual SynthTrace::ObjectID getUniqueID(const jsi::Object &o) = 0;
  virtual SynthTrace::ObjectID getUniqueID(const jsi::BigInt &s) = 0;
  virtual SynthTrace::ObjectID getUniqueID(const jsi::String &s) = 0;
  virtual SynthTrace::ObjectID getUniqueID(const jsi::PropNameID &pni) = 0;
  virtual SynthTrace::ObjectID getUniqueID(const jsi::Symbol &sym) = 0;

  virtual void flushAndDisableTrace() = 0;

  /// @name jsi::Runtime methods.
  /// @{

  jsi::Value evaluateJavaScript(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::string &sourceURL) override;

  bool drainMicrotasks(int maxMicrotasksHint = -1) override;

  jsi::Object createObject() override;
  jsi::Object createObject(std::shared_ptr<jsi::HostObject> ho) override;

  // Note that the NativeState methods do not need to be traced since they
  // cannot be observed in JS.

  jsi::BigInt createBigIntFromInt64(int64_t value) override;
  jsi::BigInt createBigIntFromUint64(uint64_t value) override;
  jsi::String bigintToString(const jsi::BigInt &bigint, int radix) override;

  jsi::String createStringFromAscii(const char *str, size_t length) override;
  jsi::String createStringFromUtf8(const uint8_t *utf8, size_t length) override;

  jsi::PropNameID createPropNameIDFromAscii(const char *str, size_t length)
      override;
  jsi::PropNameID createPropNameIDFromUtf8(const uint8_t *utf8, size_t length)
      override;
  jsi::PropNameID createPropNameIDFromString(const jsi::String &str) override;
  jsi::PropNameID createPropNameIDFromSymbol(const jsi::Symbol &sym) override;

  jsi::Value getProperty(const jsi::Object &obj, const jsi::String &name)
      override;
  jsi::Value getProperty(const jsi::Object &obj, const jsi::PropNameID &name)
      override;

  bool hasProperty(const jsi::Object &obj, const jsi::String &name) override;
  bool hasProperty(const jsi::Object &obj, const jsi::PropNameID &name)
      override;

  void setPropertyValue(
      const jsi::Object &obj,
      const jsi::String &name,
      const jsi::Value &value) override;
  void setPropertyValue(
      const jsi::Object &obj,
      const jsi::PropNameID &name,
      const jsi::Value &value) override;

  jsi::Array getPropertyNames(const jsi::Object &o) override;

  jsi::WeakObject createWeakObject(const jsi::Object &o) override;

  jsi::Value lockWeakObject(const jsi::WeakObject &wo) override;

  jsi::Array createArray(size_t length) override;
  jsi::ArrayBuffer createArrayBuffer(
      std::shared_ptr<jsi::MutableBuffer> buffer) override;

  size_t size(const jsi::Array &arr) override;
  size_t size(const jsi::ArrayBuffer &buf) override;

  uint8_t *data(const jsi::ArrayBuffer &buf) override;

  jsi::Value getValueAtIndex(const jsi::Array &arr, size_t i) override;

  void setValueAtIndexImpl(
      const jsi::Array &arr,
      size_t i,
      const jsi::Value &value) override;

  jsi::Function createFunctionFromHostFunction(
      const jsi::PropNameID &name,
      unsigned int paramCount,
      jsi::HostFunctionType func) override;

  jsi::Value call(
      const jsi::Function &func,
      const jsi::Value &jsThis,
      const jsi::Value *args,
      size_t count) override;

  jsi::Value callAsConstructor(
      const jsi::Function &func,
      const jsi::Value *args,
      size_t count) override;

  void setExternalMemoryPressure(const jsi::Object &obj, size_t amount)
      override;

  /// @}

  void addMarker(const std::string &marker);

  SynthTrace &trace() {
    return trace_;
  }

  const SynthTrace &trace() const {
    return trace_;
  }

  void replaceNondeterministicFuncs();

  // This is the number of records recorded as part of the 'preamble' of a synth
  // trace. This means all the records after this amount are from the actual
  // execution of the trace.
  uint32_t getNumPreambleRecordsForTest() const {
    assert(
        numPreambleRecords_ > 0 &&
        "Only call this method if the preamble has been executed");
    return numPreambleRecords_;
  }

 private:
  SynthTrace::TraceValue toTraceValue(const jsi::Value &value);

  std::vector<SynthTrace::TraceValue> argStringifyer(
      const jsi::Value *args,
      size_t count);

  SynthTrace::TimeSinceStart getTimeSinceStart() const;

  std::unique_ptr<jsi::Runtime> runtime_;
  SynthTrace trace_;
  std::deque<jsi::Function> savedFunctions;
  const SynthTrace::TimePoint startTime_{std::chrono::steady_clock::now()};
  uint32_t numPreambleRecords_;
};

// TracingRuntime is *almost* vm independent.  This provides the
// vm-specific bits.  And, it's not a HermesRuntime, but it holds one.
class TracingHermesRuntime final : public TracingRuntime {
 public:
  /// This constructor is not intended to be invoked directly.
  /// Use makeTracingHermesRuntime instead.
  ///
  /// \p traceStream  the stream to write trace to.
  /// \p commitAction is invoked on completion of tracing.
  /// Completion can be triggered implicitly by crash (if crash manager is
  /// provided) or explicitly by invocation of flush. If the committed trace
  /// can be found in a file, the callback returns the file name. Otherwise,
  /// the callback returns empty.
  /// \p rollbackAction is invoked if the runtime is destructed prior to
  /// completion of tracing. It may or may not invoked if completion failed.
  TracingHermesRuntime(
      std::unique_ptr<HermesRuntime> runtime,
      const ::hermes::vm::RuntimeConfig &runtimeConfig,
      std::unique_ptr<llvh::raw_ostream> traceStream,
      std::function<std::string()> commitAction,
      std::function<void()> rollbackAction);

  ~TracingHermesRuntime() override;

  SynthTrace::ObjectID getUniqueID(const jsi::Object &o) override {
    return static_cast<SynthTrace::ObjectID>(hermesRuntime().getUniqueID(o));
  }
  SynthTrace::ObjectID getUniqueID(const jsi::BigInt &b) override {
    return static_cast<SynthTrace::ObjectID>(hermesRuntime().getUniqueID(b));
  }
  SynthTrace::ObjectID getUniqueID(const jsi::String &s) override {
    return static_cast<SynthTrace::ObjectID>(hermesRuntime().getUniqueID(s));
  }
  SynthTrace::ObjectID getUniqueID(const jsi::PropNameID &pni) override {
    return static_cast<SynthTrace::ObjectID>(hermesRuntime().getUniqueID(pni));
  }
  SynthTrace::ObjectID getUniqueID(const jsi::Symbol &sym) override {
    return static_cast<SynthTrace::ObjectID>(hermesRuntime().getUniqueID(sym));
  }

  void flushAndDisableTrace() override;

  std::string flushAndDisableBridgeTrafficTrace() override;

  jsi::Value evaluateJavaScript(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::string &sourceURL) override;

  HermesRuntime &hermesRuntime() {
    return static_cast<HermesRuntime &>(plain());
  }

  const HermesRuntime &hermesRuntime() const {
    return static_cast<const HermesRuntime &>(plain());
  }

 private:
  // Why do we have a private ctor executed from the public one,
  // instead of just having a single public ctor which calls
  // getUniqueID() to initialize the base class?  This one weird trick
  // is needed to avoid undefined behavior in that case.  Otherwise,
  // when calling the base class ctor, the order of evaluating the
  // globalID value and the side effect of moving the runtime would be
  // unspecified.
  TracingHermesRuntime(
      std::unique_ptr<HermesRuntime> &runtime,
      uint64_t globalID,
      const ::hermes::vm::RuntimeConfig &runtimeConfig,
      std::unique_ptr<llvh::raw_ostream> traceStream,
      std::function<std::string()> commitAction,
      std::function<void()> rollbackAction);

  void crashCallback(int fd);

  const ::hermes::vm::RuntimeConfig conf_;
  const std::function<std::string()> commitAction_;
  const std::function<void()> rollbackAction_;
  const llvh::Optional<::hermes::vm::CrashManager::CallbackKey>
      crashCallbackKey_;

  bool flushedAndDisabled_{false};
  std::string committedTraceFilename_;
};

/// Creates and returns a HermesRuntime that traces JSI interactions.
/// The trace will be written to \p traceScratchPath incrementally.
/// On completion, the file will be renamed to \p traceResultPath, and
/// \p traceCompletionCallback (for post-processing) will be invoked.
/// Completion can be triggered implicitly by crash (if crash manager is
/// provided) or explicitly by invocation of flush.
/// If the runtime is destructed without triggering trace completion,
/// the file at \p traceScratchPath will be deleted.
/// The return value of \p traceCompletionCallback indicates whether the
/// invocation completed successfully.
std::unique_ptr<TracingHermesRuntime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    const std::string &traceScratchPath,
    const std::string &traceResultPath,
    std::function<bool()> traceCompletionCallback);

/// Creates and returns a HermesRuntime that traces JSI interactions.
/// If \p traceStream is non-null, writes the trace to \p traceStream.
/// The \p forReplay parameter indicates whether the runtime is being used
/// in trace replay.  (Its behavior can differ slightly in that case.)
std::unique_ptr<TracingHermesRuntime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    std::unique_ptr<llvh::raw_ostream> traceStream,
    bool forReplay = false);

} // namespace tracing
} // namespace hermes
} // namespace facebook

#endif // HERMES_TRACINGRUNTIME_H
