/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TracingRuntime.h"

#include <hermes/BCGen/HBC/BytecodeDataProvider.h>
#include <hermes/Platform/Logging.h>
#include <hermes/Support/Algorithms.h>
#include <hermes/Support/JSONEmitter.h>

#include <llvh/Support/raw_ostream.h>
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/SHA1.h"

namespace facebook {
namespace hermes {
namespace tracing {

TracingRuntime::TracingRuntime(
    std::unique_ptr<jsi::Runtime> runtime,
    uint64_t globalID,
    const ::hermes::vm::RuntimeConfig &conf,
    std::unique_ptr<llvh::raw_ostream> traceStream)
    : RuntimeDecorator<jsi::Runtime>(*runtime),
      runtime_(std::move(runtime)),
      trace_(globalID, conf, std::move(traceStream)),
      numPreambleRecords_(0) {}

void TracingRuntime::replaceNondeterministicFuncs() {
  insertHostForwarder({"Math", "random"});
  setupDate();
  setUpWeakRef();

  numPreambleRecords_ = trace_.records().size();
}

void TracingRuntime::insertHostForwarder(
    const std::vector<const char *> &propertyPath) {
  auto lenProp = walkPropertyPath(*runtime_, propertyPath)
                     .getProperty(*runtime_, "length")
                     .asNumber();
  jsi::Function *funcPtr = saveFunction(propertyPath);

  jsi::Function funcReplacement = jsi::Function::createFromHostFunction(
      *this,
      jsi::PropNameID::forAscii(*this, propertyPath.back()),
      lenProp,
      [this, funcPtr](
          Runtime &rt,
          const jsi::Value &thisVal,
          const jsi::Value *args,
          size_t count) {
        return thisVal.isObject()
            ? funcPtr->callWithThis(
                  *runtime_, thisVal.asObject(*runtime_), args, count)
            : funcPtr->call(*runtime_, args, count);
      });

  walkPropertyPath(*this, propertyPath, 1)
      .setProperty(*this, propertyPath.back(), funcReplacement);
}

void TracingRuntime::setUpWeakRef() {
  // WeakRef is not always defined.
  if (runtime_->global().getProperty(*runtime_, "WeakRef").isUndefined())
    return;
  // The constructor, though deterministic, needs to be replaced as well. This
  // is because the object that is returned from deref needs to have appeared in
  // the synth trace before deref is called. Therefore, we simply insert a
  // 'no-op' with the object as the parameter, so that the object returned from
  // deref shows up in the trace.
  jsi::Function nativeNoOp = jsi::Function::createFromHostFunction(
      *this,
      jsi::PropNameID::forAscii(*this, "WeakRef"),
      0,
      [](Runtime &rt,
         const jsi::Value &thisVal,
         const jsi::Value *args,
         size_t count) { return jsi::Value::undefined(); });
  auto code = R"(
(function(nativeNoOp){
  var WeakRefReal = WeakRef;
  function WeakRefJSReplacement(arg){
    if (new.target){
      nativeNoOp(arg);
      return new WeakRefReal(arg);
    }
    return WeakRefReal(arg);
  }
  WeakRefJSReplacement.prototype = WeakRefReal.prototype;
  globalThis.WeakRef = WeakRefJSReplacement;
});
)";
  global()
      .getPropertyAsFunction(*this, "eval")
      .call(*this, code)
      .asObject(*this)
      .asFunction(*this)
      .call(*this, {std::move(nativeNoOp)});
  insertHostForwarder({"WeakRef", "prototype", "deref"});
}

void TracingRuntime::setupDate() {
  auto lenProp = walkPropertyPath(*runtime_, {"Date"})
                     .getProperty(*runtime_, "length")
                     .asNumber();
  jsi::Function *origDateFunc = saveFunction({"Date"});

  jsi::Function nativeDateCtor = jsi::Function::createFromHostFunction(
      *this,
      jsi::PropNameID::forAscii(*this, "Date"),
      lenProp,
      [this, origDateFunc](
          Runtime &rt,
          const jsi::Value &thisVal,
          const jsi::Value *args,
          size_t count) {
        auto ret = origDateFunc->callAsConstructor(*runtime_);
        // We cannot return this value here, because the trace would be
        // invalid. `new Date()` returns an object, so returning it would mean
        // returning an object that has never been defined. Therefore, we trace
        // reconstructing a new Date with the argument being the getTime() value
        // from the Date object created in the untraced runtime. Conceptually,
        // we are transforming calls to the no-arg Date constructor:
        // var myDate = new Date();
        // -->
        // var tmp = new Date();        <-- this is untraced
        // var arg = tmp.getTime();     <-- this is untraced
        // var myDate = new Date(arg);  <-- this is traced
        auto obj = ret.asObject(*runtime_);
        auto val = obj.getPropertyAsFunction(*runtime_, "getTime")
                       .callWithThis(*runtime_, obj);
        return this->global()
            .getPropertyAsFunction(*this, "Date")
            .callAsConstructor(*this, val);
      });

  jsi::Function nativeDateFunc = jsi::Function::createFromHostFunction(
      *this,
      jsi::PropNameID::forAscii(*this, "Date"),
      lenProp,
      [this, origDateFunc](
          Runtime &rt,
          const jsi::Value &thisVal,
          const jsi::Value *args,
          size_t count) {
        auto ret = origDateFunc->call(*runtime_, args, count);
        std::string retStr = ret.asString(*runtime_).utf8(*runtime_);
        // If we just returned the string directly from the above call, the
        // trace would not be valid because we would be using a string that has
        // never been defined before. Therefore, we must copy the string in the
        // tracing runtime to get this string to show up and be defined in the
        // trace.
        return jsi::String::createFromAscii(*this, retStr);
      });

  auto code = R"(
(function(nativeDateCtor, nativeDateFunc){
  var DateReal = Date;
  function DateJSReplacement(...args){
    if (new.target){
      if (arguments.length == 0){
        return nativeDateCtor();
      } else {
        // calling new Date with arguments is deterministic
        return new DateReal(...args);
      }
    } else {
      return nativeDateFunc(...args);
    }
  }
  // Cannot use Object.assign because Date methods are not enumerable
  for (p of Object.getOwnPropertyNames(DateReal)){
    DateJSReplacement[p] = DateReal[p];
  }
  globalThis.Date = DateJSReplacement;
});
)";
  global()
      .getPropertyAsFunction(*this, "eval")
      .call(*this, code)
      .asObject(*this)
      .asFunction(*this)
      .call(*this, {std::move(nativeDateCtor), std::move(nativeDateFunc)});
  insertHostForwarder({"Date", "now"});
}

jsi::Function *TracingRuntime::saveFunction(
    const std::vector<const char *> &propertyPath) {
  jsi::Function origFunc =
      walkPropertyPath(*runtime_, propertyPath).asFunction(*runtime_);
  savedFunctions.push_back(std::move(origFunc));
  return &savedFunctions.back();
}

jsi::Object TracingRuntime::walkPropertyPath(
    jsi::Runtime &runtime,
    const std::vector<const char *> &propertyPath,
    size_t skipLastAmt) {
  assert(
      skipLastAmt <= propertyPath.size() &&
      "skipLastAmt cannot be larger than length of property path");
  jsi::Object obj = runtime.global();
  for (auto e = propertyPath.begin(); e != propertyPath.end() - skipLastAmt;
       e++) {
    obj = obj.getPropertyAsObject(runtime, *e);
  }
  return obj;
}

jsi::Value TracingRuntime::evaluateJavaScript(
    const std::shared_ptr<const jsi::Buffer> &buffer,
    const std::string &sourceURL) {
  ::hermes::SHA1 sourceHash{};
  bool sourceIsBytecode = false;
  if (HermesRuntime::isHermesBytecode(buffer->data(), buffer->size())) {
    sourceHash = ::hermes::hbc::BCProviderFromBuffer::getSourceHashFromBytecode(
        llvh::makeArrayRef(buffer->data(), buffer->size()));
    sourceIsBytecode = true;
  } else {
    sourceHash =
        llvh::SHA1::hash(llvh::makeArrayRef(buffer->data(), buffer->size()));
  }
  trace_.emplace_back<SynthTrace::BeginExecJSRecord>(
      getTimeSinceStart(), sourceURL, sourceHash, sourceIsBytecode);
  auto res = RD::evaluateJavaScript(buffer, sourceURL);
  trace_.emplace_back<SynthTrace::EndExecJSRecord>(
      getTimeSinceStart(), toTraceValue(res));
  return res;
}

bool TracingRuntime::drainMicrotasks(int maxMicrotasksHint) {
  auto res = RD::drainMicrotasks(maxMicrotasksHint);
  trace_.emplace_back<SynthTrace::DrainMicrotasksRecord>(
      getTimeSinceStart(), maxMicrotasksHint);
  return res;
};

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
      TracingRuntime &trt = tracingRuntime();
      trt.trace_.emplace_back<SynthTrace::GetPropertyNativeRecord>(
          trt.getTimeSinceStart(),
          objID_,
          trt.getUniqueID(name),
          name.utf8(rt));

      try {
        // Note that this ignores the "rt" argument, passing the
        // DHO's cached decoratedRuntime() to the underlying HostObject's get.
        // In this case, that will be a TracingRuntime.
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
      TracingRuntime &trt = tracingRuntime();
      trt.trace_.emplace_back<SynthTrace::SetPropertyNativeRecord>(
          trt.getTimeSinceStart(),
          objID_,
          trt.getUniqueID(name),
          name.utf8(rt),
          trt.toTraceValue(value));

      try {
        // Note that this ignores the "rt" argument, passing the
        // DHO's cached decoratedRuntime() to the underlying HostObject's set.
        // In this case, that will be a TracingRuntime.
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

    std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime &rt) override {
      TracingRuntime &trt = tracingRuntime();
      trt.trace_.emplace_back<SynthTrace::GetNativePropertyNamesRecord>(
          trt.getTimeSinceStart(), objID_);
      std::vector<jsi::PropNameID> props;
      try {
        // Note that this ignores the "rt" argument, passing the
        // DHO's cached decoratedRuntime() to the underlying HostObject's
        // getPropertyNames.  In this case, that will be a TracingRuntime.
        props = DecoratedHostObject::getPropertyNames(rt);
      } catch (...) {
        // TODO(T28293178): The trace currently has no way to model
        // exceptions thrown from C++ code.
        ::hermes::hermes_fatal(
            "Exception happened in native code during trace");
      }
      std::vector<std::string> names;
      names.reserve(props.size());
      for (const jsi::PropNameID &prop : props) {
        names.emplace_back(prop.utf8(rt));
      }

      trt.trace_.emplace_back<SynthTrace::GetNativePropertyNamesReturnRecord>(
          trt.getTimeSinceStart(), names);
      return props;
    }

    void setObjectID(SynthTrace::ObjectID id) {
      objID_ = id;
    }

   private:
    /// The TracingRuntime used when the TracingHostObject was created.
    TracingRuntime &tracingRuntime() {
      // A TracingHostObject is always created with a TracingRuntime,
      // so this cast is safe.
      return static_cast<TracingRuntime &>(decoratedRuntime());
    }
    /// The object id of the host object that this is attached to.
    SynthTrace::ObjectID objID_;
  };

  // These next two lines are very similar to the body of
  // RD::createObject:
  // return plain_.createObject(
  //     std::make_shared<DecoratedHostObject>(*this, std::move(ho)));
  // but (a) using the TracingHostObject subtype of
  // DecoratedHostObject, and (b) using createFromHostObject, because
  // createObject is protected, and is thus inaccessible here.
  auto tracer = std::make_shared<TracingHostObject>(*this, ho);
  auto obj = jsi::Object::createFromHostObject(plain(), tracer);

  tracer->setObjectID(getUniqueID(obj));
  trace_.emplace_back<SynthTrace::CreateHostObjectRecord>(
      getTimeSinceStart(), getUniqueID(obj));
  return obj;
}

jsi::String TracingRuntime::createStringFromAscii(
    const char *str,
    size_t length) {
  jsi::String res = RD::createStringFromAscii(str, length);
  trace_.emplace_back<SynthTrace::CreateStringRecord>(
      getTimeSinceStart(), getUniqueID(res), str, length);
  return res;
};

jsi::String TracingRuntime::createStringFromUtf8(
    const uint8_t *utf8,
    size_t length) {
  jsi::String res = RD::createStringFromUtf8(utf8, length);
  trace_.emplace_back<SynthTrace::CreateStringRecord>(
      getTimeSinceStart(), getUniqueID(res), utf8, length);
  return res;
};

jsi::PropNameID TracingRuntime::createPropNameIDFromAscii(
    const char *str,
    size_t length) {
  jsi::PropNameID res = RD::createPropNameIDFromAscii(str, length);
  trace_.emplace_back<SynthTrace::CreatePropNameIDRecord>(
      getTimeSinceStart(), getUniqueID(res), str, length);
  return res;
}

jsi::PropNameID TracingRuntime::createPropNameIDFromUtf8(
    const uint8_t *utf8,
    size_t length) {
  jsi::PropNameID res = RD::createPropNameIDFromUtf8(utf8, length);
  trace_.emplace_back<SynthTrace::CreatePropNameIDRecord>(
      getTimeSinceStart(), getUniqueID(res), utf8, length);
  return res;
}

jsi::PropNameID TracingRuntime::createPropNameIDFromString(
    const jsi::String &str) {
  jsi::PropNameID res = RD::createPropNameIDFromString(str);
  trace_.emplace_back<SynthTrace::CreatePropNameIDRecord>(
      getTimeSinceStart(),
      getUniqueID(res),
      SynthTrace::encodeString(getUniqueID(str)));
  return res;
}

jsi::PropNameID TracingRuntime::createPropNameIDFromSymbol(
    const jsi::Symbol &sym) {
  jsi::PropNameID res = RD::createPropNameIDFromSymbol(sym);
  trace_.emplace_back<SynthTrace::CreatePropNameIDRecord>(
      getTimeSinceStart(),
      getUniqueID(res),
      SynthTrace::encodeSymbol(getUniqueID(sym)));
  return res;
}

jsi::Value TracingRuntime::getProperty(
    const jsi::Object &obj,
    const jsi::String &name) {
  auto value = RD::getProperty(obj, name);
  trace_.emplace_back<SynthTrace::GetPropertyRecord>(
      getTimeSinceStart(),
      getUniqueID(obj),
      getUniqueID(name),
#ifdef HERMESVM_API_TRACE_DEBUG
      name.utf8(*this),
#endif
      toTraceValue(value));
  return value;
}

jsi::Value TracingRuntime::getProperty(
    const jsi::Object &obj,
    const jsi::PropNameID &name) {
  auto value = RD::getProperty(obj, name);
  trace_.emplace_back<SynthTrace::GetPropertyRecord>(
      getTimeSinceStart(),
      getUniqueID(obj),
      getUniqueID(name),
#ifdef HERMESVM_API_TRACE_DEBUG
      name.utf8(*this),
#endif
      toTraceValue(value));
  return value;
}

bool TracingRuntime::hasProperty(
    const jsi::Object &obj,
    const jsi::String &name) {
  trace_.emplace_back<SynthTrace::HasPropertyRecord>(
      getTimeSinceStart(),
      getUniqueID(obj),
      getUniqueID(name)
#ifdef HERMESVM_API_TRACE_DEBUG
          ,
      name.utf8(*this)
#endif
  );
  return RD::hasProperty(obj, name);
}

bool TracingRuntime::hasProperty(
    const jsi::Object &obj,
    const jsi::PropNameID &name) {
  trace_.emplace_back<SynthTrace::HasPropertyRecord>(
      getTimeSinceStart(),
      getUniqueID(obj),
      getUniqueID(name)
#ifdef HERMESVM_API_TRACE_DEBUG
          ,
      name.utf8(*this)
#endif
  );
  return RD::hasProperty(obj, name);
}

void TracingRuntime::setPropertyValue(
    jsi::Object &obj,
    const jsi::String &name,
    const jsi::Value &value) {
  trace_.emplace_back<SynthTrace::SetPropertyRecord>(
      getTimeSinceStart(),
      getUniqueID(obj),
      getUniqueID(name),
#ifdef HERMESVM_API_TRACE_DEBUG
      name.utf8(*this),
#endif
      toTraceValue(value));
  RD::setPropertyValue(obj, name, value);
}

void TracingRuntime::setPropertyValue(
    jsi::Object &obj,
    const jsi::PropNameID &name,
    const jsi::Value &value) {
  trace_.emplace_back<SynthTrace::SetPropertyRecord>(
      getTimeSinceStart(),
      getUniqueID(obj),
      getUniqueID(name),
#ifdef HERMESVM_API_TRACE_DEBUG
      name.utf8(*this),
#endif
      toTraceValue(value));
  RD::setPropertyValue(obj, name, value);
}

jsi::Array TracingRuntime::getPropertyNames(const jsi::Object &o) {
  jsi::Array arr = RD::getPropertyNames(o);
  trace_.emplace_back<SynthTrace::GetPropertyNamesRecord>(
      getTimeSinceStart(), getUniqueID(o), getUniqueID(arr));
  return arr;
}

jsi::WeakObject TracingRuntime::createWeakObject(const jsi::Object &o) {
  // WeakObject is not traced for two reasons:
  // It has no effect on the correctness of replay:
  //  Say an object that is created, then a WeakObject created for
  //  that object. At some point in the future, lockWeakObject is called. At
  //  that point, either the original object was dead, and lockWeakObject
  //  returns an undefined value; else, the original object is still alive, and
  //  it returns the object reference. For an undefined return, there will be no
  //  further operations on the object, and the replay will delete it. If that
  //  returned object is then used for some operation such as getProperty, the
  //  trace will see that and record that the object was alive for at least that
  //  long. So it doesn't matter that the WeakObject was created at all, the
  //  lifetime is unaffected.
  // lockWeakObject can have a non-deterministic return value:
  //  Because the return value of lockWeakObject is non-deterministic, there's
  //  no guarantee that replaying lockWeakObject will have the same return
  //  value. The GC may have run at different times on replay then it originally
  //  did. Making this deterministic would require adding the GC schedule to
  //  synth traces, which might not even be possible for a concurrent GC. So
  //  tracing lockWeakObject would not guarantee correct replay of WeakObject
  //  operations.
  return RD::createWeakObject(o);
}

jsi::Value TracingRuntime::lockWeakObject(jsi::WeakObject &wo) {
  // See comment in TracingRuntime::createWeakObject for why this function isn't
  // traced.
  return RD::lockWeakObject(wo);
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
      TracingRuntime &trt = static_cast<TracingRuntime &>(decoratedRuntime());

      trt.trace_.emplace_back<SynthTrace::CallToNativeRecord>(
          trt.getTimeSinceStart(),
          functionID_,
          trt.toTraceValue(thisVal),
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
  auto tfunc = jsi::Function::createFromHostFunction(
      plain(), name, paramCount, std::move(tracer));
  const auto funcID = getUniqueID(tfunc);
  tfunc.getHostFunction(plain()).target<TracingHostFunction>()->setFunctionID(
      funcID);
  trace_.emplace_back<SynthTrace::CreateHostFunctionRecord>(
      getTimeSinceStart(),
      funcID,
      getUniqueID(name),
#ifdef HERMESVM_API_TRACE_DEBUG
      name.utf8(*this),
#endif
      paramCount);
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
    return trace_.encodeString(getUniqueID(value.getString(*this)));
  } else if (value.isObject()) {
    // Get a unique identifier from the object, and use that instead. This is
    // so that object identity is tracked.
    return SynthTrace::encodeObject(getUniqueID(value.getObject(*this)));
  } else if (value.isSymbol()) {
    return SynthTrace::encodeSymbol(getUniqueID(value.getSymbol(*this)));
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
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    std::unique_ptr<llvh::raw_ostream> traceStream,
    std::function<std::string()> commitAction,
    std::function<void()> rollbackAction)
    : TracingHermesRuntime(
          runtime,
          runtime->getUniqueID(runtime->global()),
          runtimeConfig,
          std::move(traceStream),
          std::move(commitAction),
          std::move(rollbackAction)) {}

TracingHermesRuntime::~TracingHermesRuntime() {
  if (crashCallbackKey_) {
    conf_.getCrashMgr()->unregisterCallback(*crashCallbackKey_);
  }
  if (!flushedAndDisabled_) {
    // If the trace is not flushed and disabled, flush the trace and
    // run rollback action (e.g. delete the in-progress trace)
    flushAndDisableTrace();
    rollbackAction_();
  }
}

TracingHermesRuntime::TracingHermesRuntime(
    std::unique_ptr<HermesRuntime> &runtime,
    uint64_t globalID,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    std::unique_ptr<llvh::raw_ostream> traceStream,
    std::function<std::string()> commitAction,
    std::function<void()> rollbackAction)
    : TracingRuntime(
          std::move(runtime),
          globalID,
          runtimeConfig,
          std::move(traceStream)),
      conf_(runtimeConfig),
      commitAction_(std::move(commitAction)),
      rollbackAction_(std::move(rollbackAction)),
      crashCallbackKey_(
          conf_.getCrashMgr()
              ? llvh::Optional<::hermes::vm::CrashManager::CallbackKey>(
                    conf_.getCrashMgr()->registerCallback(
                        [this](int fd) { crashCallback(fd); }))
              : llvh::None) {}

void TracingHermesRuntime::flushAndDisableTrace() {
  (void)flushAndDisableBridgeTrafficTrace();
}

std::string TracingHermesRuntime::flushAndDisableBridgeTrafficTrace() {
  if (flushedAndDisabled_) {
    return committedTraceFilename_;
  }
  trace().flushAndDisable(
      hermesRuntime().getMockedEnvironment(), hermesRuntime().getGCExecTrace());
  flushedAndDisabled_ = true;
  committedTraceFilename_ = commitAction_();
  return committedTraceFilename_;
}

jsi::Value TracingHermesRuntime::evaluateJavaScript(
    const std::shared_ptr<const jsi::Buffer> &buffer,
    const std::string &sourceURL) {
  return TracingRuntime::evaluateJavaScript(buffer, sourceURL);
}

void TracingHermesRuntime::crashCallback(int fd) {
  if (flushedAndDisabled_) {
    // The trace is disabled prior to the crash.
    // As a result, this trace will likely not re-produce the crash.
    return;
  }
  llvh::raw_fd_ostream jsonStream(fd, false);
  ::hermes::JSONEmitter json(jsonStream);
  json.openDict();
  json.emitKeyValue("type", "tracing");
  std::string status = "Failed to flush";
  try {
    flushAndDisableBridgeTrafficTrace();
    status = "Completed";
  } catch (std::exception &ex) {
    ::hermes::hermesLog("Hermes", "Failed to flush trace: %s", ex.what());
    // suppress; we're in a crash handler
  } catch (...) {
    // suppress; we're in a crash handler
  }
  json.emitKeyValue("status", status);
  json.emitKeyValue("fileName", committedTraceFilename_);
  json.closeDict();
}

namespace {

void addRecordMarker(TracingRuntime &tracingRuntime) {
  jsi::Runtime &rt = tracingRuntime.plain();
  const char *funcName = "__nativeRecordTraceMarker";
  const auto funcProp = jsi::PropNameID::forAscii(tracingRuntime, funcName);
  if (tracingRuntime.global().hasProperty(tracingRuntime, funcProp)) {
    // If this function is already defined, throw.
    throw jsi::JSINativeException(
        std::string("global.") + funcName +
        " already exists, won't overwrite it");
  }
  rt.global().setProperty(
      tracingRuntime,
      funcProp,
      jsi::Function::createFromHostFunction(
          tracingRuntime,
          funcProp,
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

static std::unique_ptr<TracingHermesRuntime> makeTracingHermesRuntimeImpl(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    std::unique_ptr<llvh::raw_ostream> traceStream,
    std::function<std::string()> commitAction,
    std::function<void()> rollbackAction,
    bool forReplay) {
  ::hermes::hermesLog("Hermes", "Creating TracingHermesRuntime.");

  auto ret = std::make_unique<TracingHermesRuntime>(
      std::move(hermesRuntime),
      runtimeConfig,
      std::move(traceStream),
      commitAction,
      rollbackAction);
  if (!forReplay) {
    // In non-replay executions, add the __nativeRecordTraceMarker function.
    // In replay executions, this will be simulated from the trace.
    addRecordMarker(*ret);
    // In replay executions, the trace will contain the instructions for
    // replacing the nondeterministic functions.
    ret->replaceNondeterministicFuncs();
  }
  return ret;
}

std::unique_ptr<TracingHermesRuntime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    const std::string &traceScratchPath,
    const std::string &traceResultPath,
    std::function<bool()> traceCompletionCallback) {
  std::error_code ec;
  std::unique_ptr<llvh::raw_ostream> traceStream =
      std::make_unique<llvh::raw_fd_ostream>(
          traceScratchPath, ec, llvh::sys::fs::F_Text);
  if (ec) {
    ::hermes::hermesLog(
        "Hermes",
        "Failed to open file %s for tracing: %s",
        traceScratchPath.c_str(),
        ec.message().c_str());
    return makeTracingHermesRuntime(
        std::move(hermesRuntime), runtimeConfig, nullptr, true);
  }

  return makeTracingHermesRuntimeImpl(
      std::move(hermesRuntime),
      runtimeConfig,
      std::move(traceStream),
      [traceCompletionCallback, traceScratchPath, traceResultPath]() {
        if (traceScratchPath != traceResultPath) {
          std::error_code ec =
              llvh::sys::fs::rename(traceScratchPath, traceResultPath);
          if (ec) {
            ::hermes::hermesLog(
                "Hermes",
                "Failed to rename tracing file from %s to %s: %s",
                traceScratchPath.c_str(),
                traceResultPath.c_str(),
                ec.message().c_str());
            return std::string();
          }
        }
        bool success = traceCompletionCallback();
        if (!success) {
          ::hermes::hermesLog(
              "Hermes",
              "Failed to invoke completion callback for tracing file %s",
              traceResultPath.c_str());
          return std::string();
        }
        ::hermes::hermesLog(
            "Hermes", "Completed tracing file at %s", traceResultPath.c_str());
        return traceResultPath;
      },
      [traceScratchPath]() {
        // Delete the in-progress trace
        llvh::sys::fs::remove(traceScratchPath);
      },
      false);
}

std::unique_ptr<TracingHermesRuntime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    std::unique_ptr<llvh::raw_ostream> traceStream,
    bool forReplay) {
  return makeTracingHermesRuntimeImpl(
      std::move(hermesRuntime),
      runtimeConfig,
      std::move(traceStream),
      []() { return std::string(); },
      []() {},
      forReplay);
}

} // namespace tracing
} // namespace hermes
} // namespace facebook
