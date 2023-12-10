/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <chrono>
#include <iostream>
#include "hermes/CompileJS.h"
#include "hermes/hermes.h"
#include "hermes_abi/HermesABIRuntime.h"
#include "hermes_abi/hermes_vtable.h"

using namespace facebook::jsi;
using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Duration = Clock::duration;

template <typename T>
void doNotOptimizeAway(const T &t) {
  // asm volatile("" : : "r"(t));
}

struct BenchmarkSuspender {
  struct DismissedTag {};
  static inline constexpr DismissedTag Dismissed{};

  BenchmarkSuspender() : start(Clock::now()) {}

  BenchmarkSuspender(const BenchmarkSuspender &) = delete;
  BenchmarkSuspender(BenchmarkSuspender &&rhs) noexcept {
    start = rhs.start;
    rhs.start = {};
  }

  BenchmarkSuspender &operator=(const BenchmarkSuspender &) = delete;
  BenchmarkSuspender &operator=(BenchmarkSuspender &&rhs) = delete;

  ~BenchmarkSuspender() {
    if (start != TimePoint{}) {
      tally();
    }
  }

  void dismiss() {
    assert(start != TimePoint{});
    tally();
    start = {};
  }

  void rehire() {
    assert(start == TimePoint{});
    start = Clock::now();
  }

  static Duration timeSpent;

 private:
  void tally() {
    auto end = Clock::now();
    timeSpent += end - start;
    start = end;
  }

  TimePoint start;
};

Duration BenchmarkSuspender::timeSpent{};

#define BENCHMARK(name, iters)                      \
  static void BM_##name(int iters);                 \
  static bool foo_##name = runBM(BM_##name, #name); \
  void BM_##name(int iters)
#define BENCHMARK_RELATIVE(name, iters) BENCHMARK(name, iters)

static bool runBM(void (*func)(int), const char *name) {
  int iters = 1;
  do {
    BenchmarkSuspender::timeSpent = Duration{};
    auto start = Clock::now();
    func(iters);
    auto totalTime = Clock::now() - start;

    // Spend at least one second on each benchmark.
    if (totalTime < std::chrono::milliseconds(100)) {
      iters *= (std::chrono::milliseconds(200) / totalTime);
      continue;
    }

    auto netTime = totalTime - BenchmarkSuspender::timeSpent;
    auto perIter = netTime / iters;
    auto perIterCount =
        std::chrono::duration_cast<std::chrono::nanoseconds>(perIter).count();
    std::cout << name << " took " << perIterCount << "ns/iter" << std::endl;
    return true;
  } while (true);
}

static Function function(Runtime &rt, const char *code) {
  std::string bytecode;
  hermes::compileJS(code, bytecode, /* optimize = */ true);
  Value val = rt.evaluateJavaScript(
      std::unique_ptr<StringBuffer>(new StringBuffer(bytecode)), "");
  return val.getObject(rt).getFunction(rt);
}

static std::unique_ptr<Runtime> makeRuntime() {
  return facebook::hermes::makeHermesABIRuntime(get_hermes_abi_vtable());
}

BENCHMARK(CallJSFunc1, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  auto f = function(*rt, "(function (n) { return ++n; })");
  double result = 0.0;
  // End of setup.
  braces.dismiss();

  for (int i = 0; i < n; ++i) {
    result = f.call(*rt, result).getNumber();
    doNotOptimizeAway(result);
  }

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(CallJSFunc4, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  auto f = function(*rt, "(function (a, b, c, d) { return a + b + c + d; })");
  double result = 0.0;
  // End of setup.
  braces.dismiss();

  for (int i = 0; i < n; ++i) {
    result = f.call(*rt, result, 1, 2, 3).getNumber();
    doNotOptimizeAway(result);
  }

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK_RELATIVE(CallJSFunc4Fixed, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  auto f = function(*rt, "(function (a, b, c, d) { return a + b + c + d; })");
  double result = 0.0;
  Value args[4] = {Value(0), Value(1), Value(2), Value(3)};
  // End of setup.
  braces.dismiss();

  for (int i = 0; i < n; ++i) {
    result = f.call(*rt, (const Value *)args, (size_t)4).getNumber();
    doNotOptimizeAway(result);
  }

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(CallJSFunc1Object, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  auto obj = Object(*rt);
  obj.setProperty(*rt, "n", 1);
  Function f = function(*rt, "(function (obj) { return obj.n; })");
  double result = 0.0;
  // End of setup.
  braces.dismiss();

  for (int i = 0; i < n; ++i) {
    Value args[] = {Value(*rt, obj)};
    double result =
        f.call(*rt, static_cast<const Value *>(args), static_cast<size_t>(1))
            .getNumber();
    doNotOptimizeAway(result);
  }

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(MakeValueFromObject, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  auto obj = facebook::jsi::Object(*rt);
  obj.setProperty(*rt, "n", 1);
  // End of setup.
  braces.dismiss();

  for (int i = 0; i < n; ++i) {
    auto val = Value(*rt, obj);
    doNotOptimizeAway(val);
  }

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(CallHostFunc1, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  double result = 0.0;
  auto hf = Function::createFromHostFunction(
      *rt,
      PropNameID::forAscii(*rt, "hf"),
      0,
      [&result](Runtime &, const Value &, const Value *args, size_t) {
        result += args[0].getNumber();
        return Value(result);
      });
  auto f = function(*rt, "(function (hf, n) { while (n--) hf(n); })");

  // End of setup.
  braces.dismiss();

  f.call(*rt, hf, (double)n);
  doNotOptimizeAway(result);

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(CallHostFunc4, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  double result = 0.0;
  auto hf = Function::createFromHostFunction(
      *rt,
      PropNameID::forAscii(*rt, "hf"),
      0,
      [&result](Runtime &, const Value &, const Value *args, size_t) {
        result += args[0].getNumber() + args[1].getNumber() +
            args[2].getNumber() + args[3].getNumber();
        return Value(result);
      });
  auto f = function(*rt, "(function (hf, n) { while (n--) hf(n, 1, 2, 3); })");

  // End of setup.
  braces.dismiss();

  f.call(*rt, hf, (double)n);
  doNotOptimizeAway(result);

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(CreateHostObj, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  class EmptyHostObject : public HostObject {};
  auto ptr = std::make_shared<EmptyHostObject>();

  // End of setup.
  braces.dismiss();

  for (int i = 0; i < n; ++i) {
    auto obj = Object::createFromHostObject(*rt, ptr);
    doNotOptimizeAway(obj);
  }

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(GetJSProp, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  auto f = function(*rt, "(function () { return {foo: 42, bar: 87}; })");
  Object obj = f.call(*rt).getObject(*rt);
  auto foo = PropNameID::forAscii(*rt, "foo");

  // End of setup.
  braces.dismiss();

  for (int i = 0; i < n; ++i) {
    auto result = obj.getProperty(*rt, foo);
    doNotOptimizeAway(result);
  }

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(GetJSPointerProp, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  auto f = function(*rt, "(function () { return {foo: {}, bar: 87}; })");
  Object obj = f.call(*rt).getObject(*rt);
  auto foo = PropNameID::forAscii(*rt, "foo");

  // End of setup.
  braces.dismiss();

  for (int i = 0; i < n; ++i) {
    auto result = obj.getProperty(*rt, foo);
    doNotOptimizeAway(result);
  }

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(GetHostProp, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  class SimpleHostObject : public HostObject {
    Value get(Runtime &, const PropNameID &) override {
      return 42;
    }
  };
  auto ptr = std::make_shared<SimpleHostObject>();
  auto obj = Object::createFromHostObject(*rt, ptr);
  auto f = function(*rt, "(function (obj, n) { while (n--) obj.blah; })");

  // End of setup.
  braces.dismiss();

  f.call(*rt, obj, (double)n);

  // Exclude teardown.
  braces.rehire();
}

/// Access properties on a HostObject that keeps them in a JS object.
BENCHMARK(AccessHostObjectStateInJS, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  struct SimpleHostObject : public HostObject {
    Object jsObj;
    void *otherStuff;
    explicit SimpleHostObject(Object jsObj) : jsObj(std::move(jsObj)) {}
    Value get(Runtime &runtime, const PropNameID &propName) override {
      return jsObj.getProperty(runtime, propName);
    }
  };
  auto factory =
      function(*rt, "(function () { return {one: 1, two: 2, three: 3}; })");
  auto ptr =
      std::make_shared<SimpleHostObject>(factory.call(*rt).getObject(*rt));
  auto obj = Object::createFromHostObject(*rt, ptr);
  auto f = function(
      *rt,
      "(function (obj, n) { while (n--) obj.one + obj.two + obj.three; })");

  // End of setup.
  braces.dismiss();

  f.call(*rt, obj, (double)n);

  // Exclude teardown.
  braces.rehire();
}

/// Access properties on a HostObject that keeps them in C++ fields.
BENCHMARK(AccessHostObjectStateInNative, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  struct SimpleHostObject : public HostObject {
    int one{1};
    int two{2};
    int three{3};
    void *otherStuff;
    Value get(Runtime &runtime, const PropNameID &propName) override {
      auto name = propName.utf8(runtime);
      if (name == "one") {
        return one;
      } else if (name == "two") {
        return two;
      } else if (name == "three") {
        return three;
      }
      return Value::undefined();
    }
  };
  auto ptr = std::make_shared<SimpleHostObject>();
  auto obj = Object::createFromHostObject(*rt, ptr);
  auto f = function(
      *rt,
      "(function (obj, n) { while (n--) obj.one + obj.two + obj.three; })");

  // End of setup.
  braces.dismiss();

  f.call(*rt, obj, (double)n);

  // Exclude teardown.
  braces.rehire();
}

/// For comparison purposes, perform the same property accesses as in
/// AccessHostObjectStateIn{JS,Native} on a JS object with attached NativeState.
BENCHMARK(AccessNativeStateObj, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  struct SimpleNativeState : public NativeState {
    void *otherStuff;
  };
  auto factory =
      function(*rt, "(function () { return {one: 1, two: 2, three: 3}; })");
  auto obj = factory.call(*rt).getObject(*rt);
  obj.setNativeState(*rt, std::make_shared<SimpleNativeState>());

  auto f = function(
      *rt,
      "(function (obj, n) { while (n--) obj.one + obj.two + obj.three; })");

  // End of setup.
  braces.dismiss();

  f.call(*rt, obj, (double)n);

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(SetNativeState, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  struct SimpleNativeState : public NativeState {
    void *otherStuff;
  };
  auto ns = std::make_shared<SimpleNativeState>();
  auto factory =
      function(*rt, "(function () { return {one: 1, two: 2, three: 3}; })");
  auto obj = factory.call(*rt).getObject(*rt);

  // End of setup.
  braces.dismiss();

  for (int i = 0; i < n; ++i) {
    // This will always allocate a new NativeState cell.
    obj.setNativeState(*rt, ns);
  }

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(GetNativeState, n) {
  // Must be first, to properly exclude setup and teardown.
  BenchmarkSuspender braces;
  auto rt = makeRuntime();
  struct SimpleNativeState : public NativeState {
    void *otherStuff;
  };
  auto ns = std::make_shared<SimpleNativeState>();
  auto factory =
      function(*rt, "(function () { return {one: 1, two: 2, three: 3}; })");
  auto obj = factory.call(*rt).getObject(*rt);
  obj.setNativeState(*rt, ns);

  // End of setup.
  braces.dismiss();

  for (int i = 0; i < n; ++i) {
    auto result = obj.getNativeState(*rt);
    doNotOptimizeAway(result);
  }

  // Exclude teardown.
  braces.rehire();
}

BENCHMARK(ConstructAndDestructRuntime, n) {
  for (int i = 0; i < n; ++i) {
    auto rt = makeRuntime();
    doNotOptimizeAway(rt);
  }
}

int main(int argc, char **argv) {
  return 0;
}
