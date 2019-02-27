/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_TRACINGRUNTIME_H
#define HERMES_TRACINGRUNTIME_H

#include "SynthTrace.h"

#include <hermes/hermes.h>
#include <jsi/decorator.h>

namespace facebook {
namespace hermes {
namespace tracing {

class TracingRuntime : public jsi::RuntimeDecorator<jsi::Runtime> {
  std::vector<SynthTrace::TraceValue> argStringifyer(
      const jsi::Value *args,
      size_t count) {
    std::vector<SynthTrace::TraceValue> stringifiedArgs;
    stringifiedArgs.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      stringifiedArgs.emplace_back(toTraceValue(args[i]));
    }
    return stringifiedArgs;
  }

 public:
  using RD = RuntimeDecorator<jsi::Runtime>;

  TracingRuntime(std::unique_ptr<jsi::Runtime> runtime, uint64_t globalID)
      : RuntimeDecorator<jsi::Runtime>(*runtime),
        runtime_(std::move(runtime)),
        trace_(globalID) {}

  virtual SynthTrace::ObjectID getUniqueID(const jsi::Object &o) = 0;

  virtual void writeTrace(llvm::raw_ostream &os) const = 0;

  /// @name jsi::Runtime methods.
  /// @{

  void evaluateJavaScript(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::string &sourceURL) override {
    trace_.emplace_back<SynthTrace::BeginExecJSRecord>(getTimeSinceStart());
    RD::evaluateJavaScript(buffer, sourceURL);
    trace_.emplace_back<SynthTrace::EndExecJSRecord>(getTimeSinceStart());
  }

  jsi::Object createObject() override {
    auto obj = RD::createObject();
    trace_.emplace_back<SynthTrace::CreateObjectRecord>(
        getTimeSinceStart(), getUniqueID(obj));
    return obj;
  }

  class TracingHostObject : public jsi::DecoratedHostObject {
   public:
    using jsi::DecoratedHostObject::DecoratedHostObject;

    jsi::Value get(jsi::Runtime &rt, const jsi::PropNameID &name) override {
      TracingRuntime &trt = static_cast<TracingRuntime &>(rt);

      trt.trace_.emplace_back<SynthTrace::GetPropertyNativeRecord>(
          trt.getTimeSinceStart(), objID_, name.utf8(rt));

      try {
        auto ret = DecoratedHostObject::get(rt, name);

        trt.trace_.emplace_back<SynthTrace::GetPropertyNativeReturnRecord>(
            trt.getTimeSinceStart(), trt.toTraceValue(ret));

        return ret;
      } catch (...) {
        // TODO(T28293178): The trace currently has no way to model
        // exceptions thrown from C++ code.
        ::hermes::hermes_fatal(
            "Exception happened in native code during trace");
      }
    }

    void set(
        jsi::Runtime &rt,
        const jsi::PropNameID &name,
        const jsi::Value &value) override {
      TracingRuntime &trt = static_cast<TracingRuntime &>(rt);

      trt.trace_.emplace_back<SynthTrace::SetPropertyNativeRecord>(
          trt.getTimeSinceStart(),
          objID_,
          name.utf8(rt),
          trt.toTraceValue(value));

      try {
        DecoratedHostObject::set(rt, name, value);
      } catch (...) {
        // TODO(T28293178): The trace currently has no way to model
        // exceptions thrown from C++ code.
        ::hermes::hermes_fatal(
            "Exception happened in native code during trace");
      }

      trt.trace_.emplace_back<SynthTrace::SetPropertyNativeReturnRecord>(
          trt.getTimeSinceStart());
    }

    // TODO(T31386973): getPropertyNames() is not implemented.

    void setObjectID(SynthTrace::ObjectID id) {
      objID_ = id;
    }

   private:
    /// The object id of the host object that this is attached to.
    SynthTrace::ObjectID objID_;
  };

  jsi::Object createObject(std::shared_ptr<jsi::HostObject> ho) override {
    auto tracer = std::make_shared<TracingHostObject>(*this, ho);
    auto obj = RD::createObject(tracer);
    tracer->setObjectID(getUniqueID(obj));
    trace_.emplace_back<SynthTrace::CreateHostObjectRecord>(
        getTimeSinceStart(), getUniqueID(obj));
    return obj;
  }

  jsi::Value getProperty(const jsi::Object &obj, const jsi::String &name)
      override {
    auto value = RD::getProperty(obj, name);
    trace_.emplace_back<SynthTrace::GetPropertyRecord>(
        getTimeSinceStart(), getUniqueID(obj), utf8(name), toTraceValue(value));
    return value;
  }

  jsi::Value getProperty(const jsi::Object &obj, const jsi::PropNameID &name)
      override {
    auto value = RD::getProperty(obj, name);
    trace_.emplace_back<SynthTrace::GetPropertyRecord>(
        getTimeSinceStart(), getUniqueID(obj), utf8(name), toTraceValue(value));
    return value;
  }

  bool hasProperty(const jsi::Object &obj, const jsi::String &name) override {
    trace_.emplace_back<SynthTrace::HasPropertyRecord>(
        getTimeSinceStart(), getUniqueID(obj), utf8(name));
    return RD::hasProperty(obj, name);
  }

  bool hasProperty(const jsi::Object &obj, const jsi::PropNameID &name)
      override {
    trace_.emplace_back<SynthTrace::HasPropertyRecord>(
        getTimeSinceStart(), getUniqueID(obj), utf8(name));
    return RD::hasProperty(obj, name);
  }

  void setPropertyValue(
      jsi::Object &obj,
      const jsi::String &name,
      const jsi::Value &value) override {
    trace_.emplace_back<SynthTrace::SetPropertyRecord>(
        getTimeSinceStart(), getUniqueID(obj), utf8(name), toTraceValue(value));
    RD::setPropertyValue(obj, name, value);
  }

  void setPropertyValue(
      jsi::Object &obj,
      const jsi::PropNameID &name,
      const jsi::Value &value) override {
    trace_.emplace_back<SynthTrace::SetPropertyRecord>(
        getTimeSinceStart(), getUniqueID(obj), utf8(name), toTraceValue(value));
    RD::setPropertyValue(obj, name, value);
  }

  jsi::WeakObject createWeakObject(const jsi::Object &o) override {
    auto wo = RD::createWeakObject(o);
    // TODO mhorowitz: add synthtrace support for WeakObject
    return wo;
  }

  jsi::Value lockWeakObject(const jsi::WeakObject &wo) override {
    auto val = RD::lockWeakObject(wo);
    // TODO mhorowitz: add synthtrace support for WeakObject
    return val;
  }

  jsi::Array createArray(size_t length) override {
    auto arr = RD::createArray(length);
    trace_.emplace_back<SynthTrace::CreateArrayRecord>(
        getTimeSinceStart(), getUniqueID(arr), length);
    return arr;
  }

  size_t size(const jsi::Array &arr) override {
    // Array size inquiries read from the length property, which is
    // non-configurable and thus cannot have side effects.
    return RD::size(arr);
  }

  size_t size(const jsi::ArrayBuffer &buf) override {
    // ArrayBuffer size inquiries read from the byteLength property, which is
    // non-configurable and thus cannot have side effects.
    return RD::size(buf);
  }

  uint8_t *data(const jsi::ArrayBuffer &buf) override {
    throw std::logic_error(
        "Cannot write raw bytes into an ArrayBuffer in trace mode");
  }

  jsi::Value getValueAtIndex(const jsi::Array &arr, size_t i) override {
    auto value = RD::getValueAtIndex(arr, i);
    trace_.emplace_back<SynthTrace::ArrayReadRecord>(
        getTimeSinceStart(), getUniqueID(arr), i, toTraceValue(value));
    return value;
  }

  void setValueAtIndexImpl(jsi::Array &arr, size_t i, const jsi::Value &value)
      override {
    trace_.emplace_back<SynthTrace::ArrayWriteRecord>(
        getTimeSinceStart(), getUniqueID(arr), i, toTraceValue(value));
    return RD::setValueAtIndexImpl(arr, i, value);
  }

  class TracingHostFunction : public jsi::DecoratedHostFunction {
   public:
    using jsi::DecoratedHostFunction::DecoratedHostFunction;

    jsi::Value operator()(
        jsi::Runtime &rt,
        const jsi::Value &thisVal,
        const jsi::Value *args,
        size_t count) {
      TracingRuntime &trt = static_cast<TracingRuntime &>(rt);

      trt.trace_.emplace_back<SynthTrace::CallToNativeRecord>(
          trt.getTimeSinceStart(),
          functionID_,
          // A host function does not have a this.
          SynthTrace::encodeUndefined(),
          trt.argStringifyer(args, count));

      try {
        auto ret =
            jsi::DecoratedHostFunction::operator()(rt, thisVal, args, count);

        trt.trace_.emplace_back<SynthTrace::ReturnFromNativeRecord>(
            trt.getTimeSinceStart(), trt.toTraceValue(ret));
        return ret;
      } catch (...) {
        // TODO(T28293178): The trace currently has no way to model
        // exceptions thrown from C++ code.
        ::hermes::hermes_fatal(
            "Exception happened in native code during trace");
      }
    }

    void setFunctionID(SynthTrace::ObjectID functionID) {
      functionID_ = functionID;
    }

    /// The object id of the function that this is attached to.
    SynthTrace::ObjectID functionID_;
  };

  jsi::Function createFunctionFromHostFunction(
      const jsi::PropNameID &name,
      unsigned int paramCount,
      jsi::HostFunctionType func) override {
    auto tracer = TracingHostFunction(*this, func);
    auto tfunc =
        RD::createFunctionFromHostFunction(name, paramCount, std::move(tracer));
    RD::getHostFunction(tfunc).target<TracingHostFunction>()->setFunctionID(
        getUniqueID(tfunc));
    trace_.emplace_back<SynthTrace::CreateHostFunctionRecord>(
        getTimeSinceStart(), getUniqueID(tfunc));
    return tfunc;
  }

  jsi::Value call(
      const jsi::Function &func,
      const jsi::Value &jsThis,
      const jsi::Value *args,
      size_t count) override {
    trace_.emplace_back<SynthTrace::CallFromNativeRecord>(
        getTimeSinceStart(),
        getUniqueID(func),
        toTraceValue(jsThis),
        argStringifyer(args, count));
    auto retval = RD::call(func, jsThis, args, count);
    trace_.emplace_back<SynthTrace::ReturnToNativeRecord>(
        getTimeSinceStart(), toTraceValue(retval));
    return retval;
  }

  jsi::Value callAsConstructor(
      const jsi::Function &func,
      const jsi::Value *args,
      size_t count) override {
    trace_.emplace_back<SynthTrace::ConstructFromNativeRecord>(
        getTimeSinceStart(),
        getUniqueID(func),
        // A construct call always has an undefined this.
        // The ReturnToNativeRecord will contain the object that was either
        // created by the new keyword, or the objec that's returned from the
        // function.
        SynthTrace::encodeUndefined(),
        argStringifyer(args, count));
    auto retval = RD::callAsConstructor(func, args, count);
    trace_.emplace_back<SynthTrace::ReturnToNativeRecord>(
        getTimeSinceStart(), toTraceValue(retval));
    return retval;
  }

  /// @}

  void addTTIMarker() {
    trace_.emplace_back<SynthTrace::MarkerRecord>(getTimeSinceStart(), "tti");
  }

  SynthTrace &trace() {
    return trace_;
  }

  const SynthTrace &trace() const {
    return trace_;
  }

 private:
  SynthTrace::TraceValue toTraceValue(const jsi::Value &value);

  SynthTrace::TimeSinceStart getTimeSinceStart() const {
    return std::chrono::duration_cast<SynthTrace::TimeSinceStart>(
        std::chrono::steady_clock::now() - startTime_);
  }

  std::unique_ptr<jsi::Runtime> runtime_;
  SynthTrace trace_;
  const SynthTrace::TimePoint startTime_{std::chrono::steady_clock::now()};
};

// TracingRuntime is *almost* vm independent.  This provides the
// vm-specific bits.  And, it's not a HermesRuntime, but it holds one.
class TracingHermesRuntime final : public TracingRuntime {
 public:
  TracingHermesRuntime(
      std::unique_ptr<HermesRuntime> runtime,
      const ::hermes::vm::RuntimeConfig &runtimeConfig)
      : TracingHermesRuntime(
            runtime,
            runtime->getUniqueID(runtime->global()),
            runtimeConfig) {}

  SynthTrace::ObjectID getUniqueID(const jsi::Object &o) override {
    return static_cast<SynthTrace::ObjectID>(hermesRuntime().getUniqueID(o));
  }

  void writeTrace(llvm::raw_ostream &os) const override;

  void writeBridgeTrafficTraceToFile(
      const std::string &fileName) const override;

  void evaluateJavaScript(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::string &sourceURL) override {
    if (HermesRuntime::isHermesBytecode(buffer->data(), buffer->size())) {
      trace().setSourceHash(
          ::hermes::hbc::BCProviderFromBuffer::getSourceHashFromBytecode(
              llvm::makeArrayRef(buffer->data(), buffer->size())));
    }
    TracingRuntime::evaluateJavaScript(buffer, sourceURL);
  }

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
      const ::hermes::vm::RuntimeConfig &runtimeConfig)
      : TracingRuntime(std::move(runtime), globalID), conf_(runtimeConfig) {}

  const ::hermes::vm::RuntimeConfig conf_;
};

std::unique_ptr<TracingHermesRuntime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    bool shouldExposeTraceFunctions = true);

} // namespace tracing
} // namespace hermes
} // namespace facebook

#endif
