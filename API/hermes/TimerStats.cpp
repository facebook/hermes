/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TimerStats.h"

#include <hermes/Platform/Logging.h>
#include <hermes/VM/Runtime.h>
#include <hermes/VM/RuntimeStats.h>
#include <jsi/decorator.h>

#include "hermes/hermes.h"

namespace vm = hermes::vm;

namespace facebook {
namespace hermes {

class TimedHostObject final : public jsi::DecoratedHostObject {
 public:
  using DHO = jsi::DecoratedHostObject;

  TimedHostObject(
      jsi::Runtime &rt,
      std::shared_ptr<HostObject> plainHO,
      ::hermes::vm::instrumentation::RuntimeStats &rts)
      : DHO(rt, std::move(plainHO)), rts_(rts) {}

  /// @name jsi::DecoratedHostObject methods.
  /// @{
  jsi::Value get(jsi::Runtime &rt, const jsi::PropNameID &name) override {
    const vm::instrumentation::RAIITimer timer{
        "HostObject.get", rts_, rts_.hostFunction};
    return DHO::get(rt, name);
  }

  void set(
      jsi::Runtime &rt,
      const jsi::PropNameID &name,
      const jsi::Value &value) override {
    const vm::instrumentation::RAIITimer timer{
        "HostObject.set", rts_, rts_.hostFunction};
    return DHO::set(rt, name, value);
  }

  std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime &rt) override {
    const vm::instrumentation::RAIITimer timer{
        "HostObject.getHostPropertyNames", rts_, rts_.hostFunction};
    return DHO::getPropertyNames(rt);
  }
  /// @}

 private:
  ::hermes::vm::instrumentation::RuntimeStats &rts_;
};

class TimedHostFunction final : public jsi::DecoratedHostFunction {
 public:
  using DHF = jsi::DecoratedHostFunction;

  TimedHostFunction(
      jsi::Runtime &rt,
      jsi::HostFunctionType plainHF,
      ::hermes::vm::instrumentation::RuntimeStats &rts)
      : DHF(rt, std::move(plainHF)), rts_(rts) {}
  jsi::Value operator()(
      jsi::Runtime &rt,
      const jsi::Value &thisVal,
      const jsi::Value *args,
      size_t count) {
    const vm::instrumentation::RAIITimer timer{
        "Host Function", rts_, rts_.hostFunction};
    return DHF::operator()(rt, thisVal, args, count);
  }

 private:
  ::hermes::vm::instrumentation::RuntimeStats &rts_;
};

class TimedRuntime final : public jsi::RuntimeDecorator<jsi::Runtime> {
 public:
  using RD = RuntimeDecorator<jsi::Runtime>;

  TimedRuntime(std::unique_ptr<jsi::Runtime> runtime)
      : RD(*runtime), runtime_(std::move(runtime)) {
    addTimerStatsInternalObject();
  }

  /// @name HermesRuntime methods.
  /// @{
  jsi::Object createObject(std::shared_ptr<jsi::HostObject> ho) override {
    return RD::createObject(
        std::make_shared<TimedHostObject>(*this, std::move(ho), rts_));
  }

  jsi::Function createFunctionFromHostFunction(
      const jsi::PropNameID &name,
      unsigned int paramCount,
      jsi::HostFunctionType func) override {
    return RD::createFunctionFromHostFunction(
        name, paramCount, TimedHostFunction{*this, std::move(func), rts_});
  }

  jsi::Value evaluatePreparedJavaScript(
      const std::shared_ptr<const jsi::PreparedJavaScript> &js) override {
    const vm::instrumentation::RAIITimer timer{
        "Evaluate JS", rts_, rts_.evaluateJS};
    return RD::evaluatePreparedJavaScript(js);
  }

  jsi::Value call(
      const jsi::Function &func,
      const jsi::Value &jsThis,
      const jsi::Value *args,
      size_t count) override {
    const vm::instrumentation::RAIITimer timer{
        "Incoming Function", rts_, rts_.incomingFunction};
    return RD::call(func, jsThis, args, count);
  }

  jsi::Value callAsConstructor(
      const jsi::Function &func,
      const jsi::Value *args,
      size_t count) override {
    const vm::instrumentation::RAIITimer timer{
        "Incoming Function: Call As Constructor", rts_, rts_.incomingFunction};
    return RD::callAsConstructor(func, args, count);
  }
  /// @}

 private:
  ::hermes::vm::instrumentation::RuntimeStats rts_{false};
  std::unique_ptr<jsi::Runtime> runtime_;

  template <double ::vm::instrumentation::RuntimeStats::Statistic::*FieldPtr>
  double computeRuntimeWith() const {
    return rts_.evaluateJS.*FieldPtr - rts_.hostFunction.*FieldPtr +
        rts_.incomingFunction.*FieldPtr;
  };

  // Creates the C++ handler for JSITimerInternal.getTimes().
  jsi::HostFunctionType getInternalTimerInternalGetTimesHandler() {
    return [this](Runtime &rt, const jsi::Value &, const jsi::Value *, size_t) {
      assert(&rt == this);

      jsi::Object ret(rt);

      // Ensure that the timers measuring the current execution are up to
      // date.
      rts_.flushPendingTimers();

      ret.setProperty(
          *this,
          "jsi_runtimeDuration",
          computeRuntimeWith<
              &::vm::instrumentation::RuntimeStats::Statistic::wallDuration>());

      ret.setProperty(
          *this,
          "jsi_runtimeCPUDuration",
          computeRuntimeWith<
              &::vm::instrumentation::RuntimeStats::Statistic::cpuDuration>());

      return ret;
    };
  }

  // Seals and freezes \p obj so client code cannot change it.
  void sealAndFreeze(jsi::Object &obj) {
    auto objectClass = global().getPropertyAsObject(*this, "Object");

    auto invoke = [&](const char *name) {
      objectClass.getPropertyAsFunction(*this, name).call(*this, obj);
    };

    invoke("preventExtensions");
    invoke("seal");
    invoke("freeze");
  }

  // Adds an immutable global variable \p name with \p value.
  template <typename T>
  void addImmutableGlobal(const char *name, T &&value) {
    jsi::Object desc(*this);
    desc.setProperty(*this, "value", std::forward<T>(value));
    desc.setProperty(*this, "writeable", false);
    desc.setProperty(*this, "configurable", false);
    desc.setProperty(*this, "enumerable", false);

    global()
        .getPropertyAsObject(*this, "Object")
        .getPropertyAsFunction(*this, "defineProperty")
        .call(*this, global(), name, desc);
  }

  // Creates the JSITimerInternal object that clients can query for timing
  // related information.
  void addTimerStatsInternalObject() {
    static constexpr size_t kNoParams = 0;

    // Handler for JSITimerInternal.getTimes
    static constexpr auto getTimesName = "getTimes";
    auto getTimesID = jsi::PropNameID::forAscii(*this, getTimesName);
    auto getTimesHandler = getInternalTimerInternalGetTimesHandler();
    auto getTimes = jsi::Function::createFromHostFunction(
        *this, getTimesID, kNoParams, getTimesHandler);

    // Builds the JSITimerInternal object. Make sure it is sealed and frozen.
    static constexpr auto jsiTimerInternalName = "JSITimerInternal";
    jsi::Object jsiTimerInternalObject(*this);
    jsiTimerInternalObject.setProperty(*this, getTimesName, getTimes);
    sealAndFreeze(jsiTimerInternalObject);

    // Adds the JSITimerInternal global variable. It is also immutable.
    addImmutableGlobal(jsiTimerInternalName, jsiTimerInternalObject);
  }
};

std::unique_ptr<jsi::Runtime> makeTimedRuntime(
    std::unique_ptr<jsi::Runtime> hermesRuntime) {
  return std::make_unique<TimedRuntime>(std::move(hermesRuntime));
}

} // namespace hermes
} // namespace facebook
