/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TracingRuntime.h"

#include "llvm/Support/FileSystem.h"

namespace facebook {
namespace hermes {
namespace tracing {

TracingRuntime::TracingRuntime(
    std::unique_ptr<jsi::Runtime> runtime,
    uint64_t globalID)
    : RuntimeDecorator<jsi::Runtime>(*runtime),
      runtime_(std::move(runtime)),
      trace_(globalID) {}

jsi::Value TracingRuntime::evaluateJavaScript(
    const std::shared_ptr<const jsi::Buffer> &buffer,
    const std::string &sourceURL) {
  trace_.emplace_back<SynthTrace::BeginExecJSRecord>(getTimeSinceStart());
  auto res = RD::evaluateJavaScript(buffer, sourceURL);
  trace_.emplace_back<SynthTrace::EndExecJSRecord>(
      getTimeSinceStart(), toTraceValue(res));
  return res;
}

jsi::Object TracingRuntime::createObject() {
  auto obj = RD::createObject();
  trace_.emplace_back<SynthTrace::CreateObjectRecord>(
      getTimeSinceStart(), getUniqueID(obj));
  return obj;
}

jsi::Object TracingRuntime::createObject(std::shared_ptr<jsi::HostObject> ho) {
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

  auto tracer = std::make_shared<TracingHostObject>(*this, ho);
  auto obj = RD::createObject(tracer);
  tracer->setObjectID(getUniqueID(obj));
  trace_.emplace_back<SynthTrace::CreateHostObjectRecord>(
      getTimeSinceStart(), getUniqueID(obj));
  return obj;
}

jsi::Value TracingRuntime::getProperty(
    const jsi::Object &obj,
    const jsi::String &name) {
  auto value = RD::getProperty(obj, name);
  trace_.emplace_back<SynthTrace::GetPropertyRecord>(
      getTimeSinceStart(), getUniqueID(obj), utf8(name), toTraceValue(value));
  return value;
}

jsi::Value TracingRuntime::getProperty(
    const jsi::Object &obj,
    const jsi::PropNameID &name) {
  auto value = RD::getProperty(obj, name);
  trace_.emplace_back<SynthTrace::GetPropertyRecord>(
      getTimeSinceStart(), getUniqueID(obj), utf8(name), toTraceValue(value));
  return value;
}

bool TracingRuntime::hasProperty(
    const jsi::Object &obj,
    const jsi::String &name) {
  trace_.emplace_back<SynthTrace::HasPropertyRecord>(
      getTimeSinceStart(), getUniqueID(obj), utf8(name));
  return RD::hasProperty(obj, name);
}

bool TracingRuntime::hasProperty(
    const jsi::Object &obj,
    const jsi::PropNameID &name) {
  trace_.emplace_back<SynthTrace::HasPropertyRecord>(
      getTimeSinceStart(), getUniqueID(obj), utf8(name));
  return RD::hasProperty(obj, name);
}

void TracingRuntime::setPropertyValue(
    jsi::Object &obj,
    const jsi::String &name,
    const jsi::Value &value) {
  trace_.emplace_back<SynthTrace::SetPropertyRecord>(
      getTimeSinceStart(), getUniqueID(obj), utf8(name), toTraceValue(value));
  RD::setPropertyValue(obj, name, value);
}

void TracingRuntime::setPropertyValue(
    jsi::Object &obj,
    const jsi::PropNameID &name,
    const jsi::Value &value) {
  trace_.emplace_back<SynthTrace::SetPropertyRecord>(
      getTimeSinceStart(), getUniqueID(obj), utf8(name), toTraceValue(value));
  RD::setPropertyValue(obj, name, value);
}

jsi::Array TracingRuntime::getPropertyNames(const jsi::Object &o) {
  jsi::Array arr = RD::getPropertyNames(o);
  trace_.emplace_back<SynthTrace::GetPropertyNamesRecord>(
      getTimeSinceStart(), getUniqueID(o), getUniqueID(arr));
  return arr;
}

jsi::WeakObject TracingRuntime::createWeakObject(const jsi::Object &o) {
  auto wo = RD::createWeakObject(o);
  // TODO mhorowitz: add synthtrace support for WeakObject
  return wo;
}

jsi::Value TracingRuntime::lockWeakObject(const jsi::WeakObject &wo) {
  auto val = RD::lockWeakObject(wo);
  // TODO mhorowitz: add synthtrace support for WeakObject
  return val;
}

jsi::Array TracingRuntime::createArray(size_t length) {
  auto arr = RD::createArray(length);
  trace_.emplace_back<SynthTrace::CreateArrayRecord>(
      getTimeSinceStart(), getUniqueID(arr), length);
  return arr;
}

size_t TracingRuntime::size(const jsi::Array &arr) {
  // Array size inquiries read from the length property, which is
  // non-configurable and thus cannot have side effects.
  return RD::size(arr);
}

size_t TracingRuntime::size(const jsi::ArrayBuffer &buf) {
  // ArrayBuffer size inquiries read from the byteLength property, which is
  // non-configurable and thus cannot have side effects.
  return RD::size(buf);
}

uint8_t *TracingRuntime::data(const jsi::ArrayBuffer &buf) {
  throw std::logic_error(
      "Cannot write raw bytes into an ArrayBuffer in trace mode");
}

jsi::Value TracingRuntime::getValueAtIndex(const jsi::Array &arr, size_t i) {
  auto value = RD::getValueAtIndex(arr, i);
  trace_.emplace_back<SynthTrace::ArrayReadRecord>(
      getTimeSinceStart(), getUniqueID(arr), i, toTraceValue(value));
  return value;
}

void TracingRuntime::setValueAtIndexImpl(
    jsi::Array &arr,
    size_t i,
    const jsi::Value &value) {
  trace_.emplace_back<SynthTrace::ArrayWriteRecord>(
      getTimeSinceStart(), getUniqueID(arr), i, toTraceValue(value));
  return RD::setValueAtIndexImpl(arr, i, value);
}

jsi::Function TracingRuntime::createFunctionFromHostFunction(
    const jsi::PropNameID &name,
    unsigned int paramCount,
    jsi::HostFunctionType func) {
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

   private:
    /// The object id of the function that this is attached to.
    SynthTrace::ObjectID functionID_;
  };
  auto tracer = TracingHostFunction(*this, func);
  auto tfunc =
      RD::createFunctionFromHostFunction(name, paramCount, std::move(tracer));
  RD::getHostFunction(tfunc).target<TracingHostFunction>()->setFunctionID(
      getUniqueID(tfunc));
  trace_.emplace_back<SynthTrace::CreateHostFunctionRecord>(
      getTimeSinceStart(), getUniqueID(tfunc));
  return tfunc;
}

jsi::Value TracingRuntime::call(
    const jsi::Function &func,
    const jsi::Value &jsThis,
    const jsi::Value *args,
    size_t count) {
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

jsi::Value TracingRuntime::callAsConstructor(
    const jsi::Function &func,
    const jsi::Value *args,
    size_t count) {
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

void TracingRuntime::addMarker(const std::string &marker) {
  trace_.emplace_back<SynthTrace::MarkerRecord>(getTimeSinceStart(), marker);
}

std::vector<SynthTrace::TraceValue> TracingRuntime::argStringifyer(
    const jsi::Value *args,
    size_t count) {
  std::vector<SynthTrace::TraceValue> stringifiedArgs;
  stringifiedArgs.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    stringifiedArgs.emplace_back(toTraceValue(args[i]));
  }
  return stringifiedArgs;
}

SynthTrace::TraceValue TracingRuntime::toTraceValue(const jsi::Value &value) {
  if (value.isUndefined()) {
    return SynthTrace::encodeUndefined();
  } else if (value.isNull()) {
    return SynthTrace::encodeNull();
  } else if (value.isBool()) {
    return SynthTrace::encodeBool(value.getBool());
  } else if (value.isNumber()) {
    return SynthTrace::encodeNumber(value.getNumber());
  } else if (value.isString()) {
    return trace_.encodeString(value.getString(*this).utf8(*this));
  } else if (value.isObject()) {
    // Get a unique identifier from the object, and use that instead. This is
    // so that object identity is tracked.
    return SynthTrace::encodeObject(getUniqueID(value.getObject(*this)));
  } else {
    throw std::logic_error("Unsupported value reached");
  }
}

SynthTrace::TimeSinceStart TracingRuntime::getTimeSinceStart() const {
  return std::chrono::duration_cast<SynthTrace::TimeSinceStart>(
      std::chrono::steady_clock::now() - startTime_);
}

TracingHermesRuntime::TracingHermesRuntime(
    std::unique_ptr<HermesRuntime> runtime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig)
    : TracingHermesRuntime(
          runtime,
          runtime->getUniqueID(runtime->global()),
          runtimeConfig) {}

TracingHermesRuntime::TracingHermesRuntime(
    std::unique_ptr<HermesRuntime> &runtime,
    uint64_t globalID,
    const ::hermes::vm::RuntimeConfig &runtimeConfig)
    : TracingRuntime(std::move(runtime), globalID), conf_(runtimeConfig) {}

void TracingHermesRuntime::writeTrace(llvm::raw_ostream &os) const {
  os << SynthTrace::Printable(
      trace(), hermesRuntime().getMockedEnvironment(), conf_);
}

void TracingHermesRuntime::writeBridgeTrafficTraceToFile(
    const std::string &fileName) const {
  std::error_code ec;
  llvm::raw_fd_ostream fs{fileName.c_str(), ec, llvm::sys::fs::F_Text};
  if (ec) {
    throw std::system_error(ec);
  }

  writeTrace(fs);
}

jsi::Value TracingHermesRuntime::evaluateJavaScript(
    const std::shared_ptr<const jsi::Buffer> &buffer,
    const std::string &sourceURL) {
  if (HermesRuntime::isHermesBytecode(buffer->data(), buffer->size())) {
    trace().setSourceHash(
        ::hermes::hbc::BCProviderFromBuffer::getSourceHashFromBytecode(
            llvm::makeArrayRef(buffer->data(), buffer->size())));
  }
  return TracingRuntime::evaluateJavaScript(buffer, sourceURL);
}

namespace {

void addRecordMarker(TracingRuntime &tracingRuntime) {
  jsi::Runtime &rt = tracingRuntime.plain();
  const char *funcName = "__nativeRecordTraceMarker";
  if (rt.global().hasProperty(rt, funcName)) {
    // If this function is already defined, throw.
    throw jsi::JSINativeException(
        std::string("global.") + funcName +
        " already exists, won't overwrite it");
  }
  rt.global().setProperty(
      rt,
      funcName,
      jsi::Function::createFromHostFunction(
          rt,
          jsi::PropNameID::forAscii(rt, funcName),
          0,
          [funcName, &tracingRuntime](
              jsi::Runtime &rt,
              const jsi::Value &,
              const jsi::Value *args,
              size_t argc) -> jsi::Value {
            if (argc < 1) {
              throw jsi::JSINativeException(
                  std::string(funcName) + " requires a single string argument");
            }
            tracingRuntime.addMarker(args[0].asString(rt).utf8(rt));
            return jsi::Value::undefined();
          }));
}

} // namespace

std::unique_ptr<TracingHermesRuntime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig) {
  auto ret = std::make_unique<TracingHermesRuntime>(
      std::move(hermesRuntime), runtimeConfig);
  addRecordMarker(*ret);
  return ret;
}

} // namespace tracing
} // namespace hermes
} // namespace facebook
