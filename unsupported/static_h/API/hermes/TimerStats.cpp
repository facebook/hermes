/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TimerStats.h"

#include <hermes/Support/OSCompat.h>
#include <hermes/Support/PerfSection.h>
#include <jsi/decorator.h>

namespace facebook {
namespace hermes {
namespace {

/// RuntimeStats contains statistics which may be manipulated by users of
/// Runtime.
class RuntimeStats {
  /// A Statistic tracks duration in wall and CPU time and a count.  All times
  /// are in seconds.
  struct Statistic {
    double wallDuration{0};
    double cpuDuration{0};
    uint64_t count{0};
  };

  /// An RAII-style class for updating a Statistic.
  class RAIITimer {
    friend RuntimeStats;
    /// RAII class for delimiting the code region tracked by this timer for the
    /// purpose of capturing tracing profiles.
    ::hermes::PerfSection perfSection_;

    /// The RuntimeStats we are updating. This is stored so we can manipulate
    /// its timerStack and statistics.
    RuntimeStats &runtimeStats_;

    /// The particular statistic we are updating.
    Statistic &stat_;

    /// The parent timer. This link forms a stack. At the point the stats are
    /// collected, all existing RAIITimers are flushed so that pending data can
    /// be collected.
    RAIITimer *const parent_;

    /// The initial value of the wall time.
    std::chrono::steady_clock::time_point wallTimeStart_;

    /// The initial value of the CPU time.
    std::chrono::microseconds cpuTimeStart_;

    explicit RAIITimer(
        const char *name,
        RuntimeStats &runtimeStats,
        Statistic &stat)
        : perfSection_(name),
          runtimeStats_(runtimeStats),
          stat_(stat),
          parent_(runtimeStats.timerStack_),
          wallTimeStart_(std::chrono::steady_clock::now()),
          cpuTimeStart_(::hermes::oscompat::thread_cpu_time()) {
      runtimeStats.timerStack_ = this;
      stat_.count += 1;
      if (!parent_)
        runtimeStats_.total_.count += 1;
    }

    /// Flush the timer to the referenced statistic, resetting the start times.
    /// Note that 'count' is incremented when the RAIITimer is created and so is
    /// unaffected. This is invoked when the timer is destroyed, but also
    /// invoked when data is collected to include any aggregate data.
    void flush() {
      auto currentCPUTime = ::hermes::oscompat::thread_cpu_time();
      auto currentWallTime = std::chrono::steady_clock::now();
      auto wallDiff =
          std::chrono::duration<double>(currentWallTime - wallTimeStart_)
              .count();
      auto cpuDiff =
          std::chrono::duration<double>(currentCPUTime - cpuTimeStart_).count();
      auto update = [wallDiff, cpuDiff](Statistic &stat) {
        stat.wallDuration += wallDiff;
        stat.cpuDuration += cpuDiff;
      };
      update(stat_);
      // If this is the initial entrypoint to the runtime, also increase the
      // total stats.
      if (!parent_)
        update(runtimeStats_.total_);
      wallTimeStart_ = currentWallTime;
      cpuTimeStart_ = currentCPUTime;
    }

   public:
    ~RAIITimer() {
      flush();
      assert(
          runtimeStats_.timerStack_ == this &&
          "Destroyed RAIITimer is not at top of stack");
      runtimeStats_.timerStack_ = parent_;
    }
  };

  /// Measure of outgoing calls through HostFunctions and HostObjects.
  Statistic outgoing_;

  /// Measure of incoming calls through JSI.
  Statistic incoming_;

  /// Measure of total time spent in Hermes.
  Statistic total_;

  /// The topmost RAIITimer in the stack.
  RAIITimer *timerStack_{nullptr};

 public:
  RAIITimer incomingTimer(const char *name) {
    return RAIITimer(name, *this, incoming_);
  }
  RAIITimer outgoingTimer(const char *name) {
    return RAIITimer(name, *this, outgoing_);
  }

  double getRuntimeDuration() const {
    return incoming_.wallDuration - outgoing_.wallDuration;
  }
  double getRuntimeCPUDuration() const {
    return incoming_.cpuDuration - outgoing_.cpuDuration;
  }
  double getTotalDuration() const {
    return total_.wallDuration;
  }
  double getTotalCPUDuration() const {
    return total_.cpuDuration;
  }

  /// Flush all timers pending in our timer stack.
  void flushPendingTimers() {
    for (auto cursor = timerStack_; cursor != nullptr;
         cursor = cursor->parent_) {
      cursor->flush();
    }
  }
};

class TimedHostObject final : public jsi::DecoratedHostObject {
 public:
  using DHO = jsi::DecoratedHostObject;

  TimedHostObject(
      jsi::Runtime &rt,
      std::shared_ptr<HostObject> plainHO,
      RuntimeStats &rts)
      : DHO(rt, std::move(plainHO)), rts_(rts) {}

  /// @name jsi::DecoratedHostObject methods.
  /// @{
  jsi::Value get(jsi::Runtime &rt, const jsi::PropNameID &name) override {
    auto timer = rts_.outgoingTimer("HostObject.get");
    return DHO::get(rt, name);
  }

  void set(
      jsi::Runtime &rt,
      const jsi::PropNameID &name,
      const jsi::Value &value) override {
    auto timer = rts_.outgoingTimer("HostObject.set");
    return DHO::set(rt, name, value);
  }

  std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime &rt) override {
    auto timer = rts_.outgoingTimer("HostObject.getHostPropertyNames");
    return DHO::getPropertyNames(rt);
  }
  /// @}

 private:
  RuntimeStats &rts_;
};

class TimedHostFunction final : public jsi::DecoratedHostFunction {
 public:
  using DHF = jsi::DecoratedHostFunction;

  TimedHostFunction(
      jsi::Runtime &rt,
      jsi::HostFunctionType plainHF,
      RuntimeStats &rts)
      : DHF(rt, std::move(plainHF)), rts_(rts) {}
  jsi::Value operator()(
      jsi::Runtime &rt,
      const jsi::Value &thisVal,
      const jsi::Value *args,
      size_t count) {
    auto timer = rts_.outgoingTimer("HostFunction");
    return DHF::operator()(rt, thisVal, args, count);
  }

 private:
  RuntimeStats &rts_;
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
    // We cannot invoke RD::createObject here since that wraps its argument in
    // another DecoratedHostObject. We also can't directly call createObject on
    // the plain runtime because createObject is a protected method.
    return jsi::Object::createFromHostObject(
        plain(), std::make_shared<TimedHostObject>(*this, std::move(ho), rts_));
  }

  jsi::Function createFunctionFromHostFunction(
      const jsi::PropNameID &name,
      unsigned int paramCount,
      jsi::HostFunctionType func) override {
    // See the comment on createObject above for why we cannot call
    // RD::createFunctionFromHostFunction.
    return jsi::Function::createFromHostFunction(
        plain(),
        name,
        paramCount,
        TimedHostFunction{*this, std::move(func), rts_});
  }

  jsi::Value evaluateJavaScript(
      const std::shared_ptr<const jsi::Buffer> &buffer,
      const std::string &sourceURL) override {
    auto timer = rts_.incomingTimer("evaluateJavaScript");
    return RD::evaluateJavaScript(buffer, sourceURL);
  }

  jsi::Value evaluatePreparedJavaScript(
      const std::shared_ptr<const jsi::PreparedJavaScript> &js) override {
    auto timer = rts_.incomingTimer("evaluatePreparedJavaScript");
    return RD::evaluatePreparedJavaScript(js);
  }

  jsi::Value call(
      const jsi::Function &func,
      const jsi::Value &jsThis,
      const jsi::Value *args,
      size_t count) override {
    auto timer = rts_.incomingTimer("call");
    return RD::call(func, jsThis, args, count);
  }

  jsi::Value callAsConstructor(
      const jsi::Function &func,
      const jsi::Value *args,
      size_t count) override {
    auto timer = rts_.incomingTimer("callAsConstructor");
    return RD::callAsConstructor(func, args, count);
  }

  bool drainMicrotasks(int maxMicrotasksHint) override {
    auto timer = rts_.incomingTimer("drainMicrotasks");
    return RD::drainMicrotasks(maxMicrotasksHint);
  }
  /// @}

 private:
  RuntimeStats rts_{};
  std::unique_ptr<jsi::Runtime> runtime_;

  // Creates the C++ handler for JSITimerInternal.getTimes().
  jsi::HostFunctionType getInternalTimerInternalGetTimesHandler() {
    return [this](Runtime &rt, const jsi::Value &, const jsi::Value *, size_t) {
      assert(&rt == this);

      jsi::Object ret(rt);

      // Ensure that the timers measuring the current execution are up to
      // date.
      rts_.flushPendingTimers();

      ret.setProperty(*this, "jsi_runtimeDuration", rts_.getRuntimeDuration());
      ret.setProperty(*this, "jsi_totalDuration", rts_.getTotalDuration());
      ret.setProperty(
          *this, "jsi_runtimeCPUDuration", rts_.getRuntimeCPUDuration());
      ret.setProperty(
          *this, "jsi_totalCPUDuration", rts_.getTotalCPUDuration());

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

} // namespace

std::unique_ptr<jsi::Runtime> makeTimedRuntime(
    std::unique_ptr<jsi::Runtime> hermesRuntime) {
  return std::make_unique<TimedRuntime>(std::move(hermesRuntime));
}

} // namespace hermes
} // namespace facebook
