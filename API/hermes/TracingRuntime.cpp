/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "TracingRuntime.h"

#include "llvm/Support/FileSystem.h"

namespace facebook {
namespace hermes {
namespace tracing {

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

namespace {

void addRecordTTI(TracingRuntime &tracingRuntime) {
  // I'd rather use Optional here, but the thing bound into the lambda
  // must be copyable.
  jsi::Runtime &rt = tracingRuntime.plain();
  const char funcName[] = "__nativeRecordTTI";
  std::shared_ptr<jsi::Function> prev;
  if (rt.global().hasProperty(rt, funcName)) {
    prev = std::make_shared<jsi::Function>(
        rt.global().getPropertyAsFunction(rt, funcName));
  }
  rt.global().setProperty(
      rt,
      funcName,
      jsi::Function::createFromHostFunction(
          rt,
          jsi::PropNameID::forAscii(rt, funcName),
          0,
          [prev, &tracingRuntime](
              jsi::Runtime &rt, const jsi::Value &, const jsi::Value *, size_t)
              -> jsi::Value {
            tracingRuntime.addTTIMarker();
            if (prev) {
              return prev->call(rt);
            } else {
              return jsi::Value::undefined();
            }
          }));
}

} // namespace

std::unique_ptr<TracingHermesRuntime> makeTracingHermesRuntime(
    std::unique_ptr<HermesRuntime> hermesRuntime,
    const ::hermes::vm::RuntimeConfig &runtimeConfig,
    bool shouldExposeTraceFunctions) {
  auto ret = std::make_unique<TracingHermesRuntime>(
      std::move(hermesRuntime), runtimeConfig);
  if (shouldExposeTraceFunctions) {
    // Wrap __nativeRecordTTI using the plain runtime, so it doesn't
    // get traced itself, as it's not part of the app.
    addRecordTTI(*ret);
  }
  return ret;
}

} // namespace tracing
} // namespace hermes
} // namespace facebook
