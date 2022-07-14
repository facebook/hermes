/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.8 Populate the Math object.
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/VM/JSLib/RuntimeCommonStorage.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/SingleObject.h"
#include "hermes/VM/StringPrimitive.h"

#define _USE_MATH_DEFINES
#include <float.h>
#include <math.h>
#include <random>
#include "hermes/Support/Math.h"
#include "hermes/Support/OSCompat.h"

#include "llvh/Support/MathExtras.h"

namespace hermes {
namespace vm {

/// @name Math
/// @{

/// @}

//===----------------------------------------------------------------------===//
/// Math.
// Implementation of Math.round(), following ES 5.1 15.8.2.15
// This cannot be a simple call to std::round() because std::round() rounds
// halfways away from zero, while Math.round must round towards positive
// infinity.
// The essential algorithm is floor(x + 0.5). However this has three
// complications:
//  1. The range [-.5, -0] must round to -0, not +0
//  2. The largest value less than 0.5, when added to 0.5, becomes 1.0
//  (precision loss), causing us to round to 1 and not 0.
//  3. Above a certain threshold (shown below), x + 0.5 is the same as x + 1.0
//  (precision loss), causing us to round too high.
// We handle this by checking explicitly for the problematic ranges.
static double roundHalfwaysTowardsInfinity(double x) {
  // The first integer where all larger values are also integral
  // The -1 is to account for the implicit (hidden) bit in the mantissa
  static constexpr double integer_threshold = 1LLU << (DBL_MANT_DIG - 1);
  double absf = std::fabs(x);
  if (absf >= integer_threshold) {
    // x is necessarily already integral.
    return x;
  } else if (absf < 0.5) {
    // x may have too much precision to add 0.5. Just round to +/- 0.
    return std::copysign(0, x);
  } else {
    // Here we can apply the normal rounding algorithm, but we need to be
    // careful about -0.5, which must round to -0.
    return std::copysign(std::floor(x + 0.5), x);
  }
}

/// The Math object has functions like sin, cos, exp, etc. Most take one
/// argument, a few take two arguments, min() and max() may take any number
/// of arguments, and random() takes none. Use context as a index to switch to
/// the corresponding c function.
enum class MathKind {
#define MATHFUNC_1ARG(name, func) name,
#include "MathStdFunctions.def"
#undef MATHFUNC_1ARG
  Num1ArgKinds,
#define MATHFUNC_2ARG(name, func) name,
#include "MathStdFunctions.def"
#undef MATHFUNC_2ARG
  Num2ArgKinds
};
// Implementation of 1-arg Math functions like sin or exp
// Interprets the ctx pointer as an enum to invoke the
// corresponding function with the first argument

CallResult<HermesValue>
runContextFunc1Arg(void *ctx, Runtime &runtime, NativeArgs args) {
  typedef double (*Math1ArgFuncPtr)(double);
  static Math1ArgFuncPtr math1ArgFuncs[] = {
#define MATHFUNC_1ARG(name, func) func,
#include "MathStdFunctions.def"
#undef MATHFUNC_1ARG
  };
  assert(
      (uint64_t)ctx < (uint64_t)MathKind::Num1ArgKinds &&
      "runContextFunc1Arg with wrong kind");
  Math1ArgFuncPtr func = math1ArgFuncs[(uint64_t)ctx];
  auto res = toNumber_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double arg = res->getNumber();
  return HermesValue::encodeDoubleValue(func(arg));
}

// Implementation of 2-arg Math functions like pow and atan2
// Interprets the ctx pointer as an enum and invoke corresponding
// function with the first two arguments
CallResult<HermesValue>
runContextFunc2Arg(void *ctx, Runtime &runtime, NativeArgs args) {
  typedef double (*Math2ArgFuncPtr)(double, double);
  static Math2ArgFuncPtr math2ArgFuncs[] = {
#define MATHFUNC_2ARG(name, func) func,
#include "MathStdFunctions.def"
#undef MATHFUNC_2ARG
  };
  assert(
      (uint64_t)ctx > (uint64_t)MathKind::Num1ArgKinds &&
      (uint64_t)ctx < (uint64_t)MathKind::Num2ArgKinds &&
      "runContextFunc1Arg with wrong kind");
  Math2ArgFuncPtr func =
      math2ArgFuncs[(uint64_t)ctx - (uint64_t)MathKind::Num1ArgKinds - 1];
  auto res = toNumber_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double arg0 = res->getNumber();

  res = toNumber_RJS(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double arg1 = res->getNumber();

  return HermesValue::encodeDoubleValue(func(arg0, arg1));
}

// ES5.1 15.8.2.11
CallResult<HermesValue> mathMax(void *, Runtime &runtime, NativeArgs args) {
  double result = -std::numeric_limits<double>::infinity();
  GCScopeMarkerRAII marker{runtime};
  for (const Handle<> sarg : args.handles()) {
    marker.flush();
    auto res = toNumber_RJS(runtime, sarg);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    double arg = res->getNumber();
    if (std::isnan(result)) {
      continue;
    } else if (std::isnan(arg)) {
      result = std::numeric_limits<double>::quiet_NaN();
    } else if (arg > result || std::signbit(arg) < std::signbit(result)) {
      // signbit(arg) < signbit(result) => arg is at least +0, result at most -0
      result = arg;
    }
  }
  return HermesValue::encodeDoubleValue(result);
}

// ES5.1 15.8.2.12
CallResult<HermesValue> mathMin(void *, Runtime &runtime, NativeArgs args) {
  double result = std::numeric_limits<double>::infinity();
  GCScopeMarkerRAII marker{runtime};
  for (const Handle<> sarg : args.handles()) {
    marker.flush();
    auto res = toNumber_RJS(runtime, sarg);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    double arg = res->getNumber();
    if (std::isnan(result)) {
      continue;
    } else if (std::isnan(arg)) {
      result = std::numeric_limits<double>::quiet_NaN();
    } else if (arg < result || std::signbit(arg) > std::signbit(result)) {
      // signbit(arg) > signbit(result) => arg is at most -0, result at least +0
      result = arg;
    }
  }
  return HermesValue::encodeDoubleValue(result);
}

// ES9.0 20.2.2.26
CallResult<HermesValue> mathPow(void *, Runtime &runtime, NativeArgs args) {
  auto res = toNumber_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  const double x = res->getNumber();

  res = toNumber_RJS(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  const double y = res->getNumber();

  return HermesValue::encodeNumberValue(expOp(x, y));
}

// ES5.1 15.8.2.14
// Returns a Hermes-encoded pseudo-random number uniformly chosen from [0, 1)
CallResult<HermesValue> mathRandom(void *, Runtime &runtime, NativeArgs) {
  RuntimeCommonStorage *storage = runtime.getCommonStorage();
  if (!storage->randomEngineSeeded_) {
    std::minstd_rand::result_type seed;
    seed = std::random_device()();
    storage->randomEngine_.seed(seed);
    storage->randomEngineSeeded_ = true;
  }
  std::uniform_real_distribution<> dist(0.0, 1.0);
  return HermesValue::encodeDoubleValue(dist(storage->randomEngine_));
}

CallResult<HermesValue> mathFround(void *, Runtime &runtime, NativeArgs args)
    LLVM_NO_SANITIZE("float-cast-overflow");

CallResult<HermesValue> mathFround(void *, Runtime &runtime, NativeArgs args) {
  auto res = toNumber_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double x = res->getNumber();

  // Make the double x into a 32-bit float,
  // and then recast it back to a 64-bit float to return it.
  // This is UB for values outside of the range of a float, but this works on
  // our current compilers.
  // TODO(T43892577): Find an alternative that doesn't use UB (or validate that
  // the UB is ok).
  return HermesValue::encodeNumberValue(
      static_cast<double>(static_cast<float>(x)));
}

// ES2022 21.3.2.18
CallResult<HermesValue> mathHypot(void *, Runtime &runtime, NativeArgs args) {
  GCScope gcScope{runtime};
  // 1. Let coerced be a new empty List.
  llvh::SmallVector<double, 4> coerced{};
  coerced.reserve(args.getArgCount());

  // Store the max abs(arg), since every argument will be squared anyway.
  // We scale down every argument by max while doing addition and sqrt,
  // and then multiply by max at the end.
  double max = 0;

  bool hasNaN = false, hasInf = false;
  auto marker = gcScope.createMarker();
  // 2. For each element arg of args, do
  for (const Handle<> arg : args.handles()) {
    gcScope.flushToMarker(marker);
    // a. Let n be ? ToNumber(arg).
    auto res = toNumber_RJS(runtime, arg);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    double value = res->getNumber();
    hasInf = std::isinf(value) || hasInf;
    hasNaN = std::isnan(value) || hasNaN;
    // b. Append n to coerced.
    coerced.push_back(value);
    max = std::max(std::fabs(value), max);
  }
  // 3. For each element number of coerced, do
  //   a. If number is +∞𝔽 or number is -∞𝔽, return +∞𝔽.
  if (hasInf)
    return HermesValue::encodeNumberValue(
        std::numeric_limits<double>::infinity());
  // 5. For each element number of coerced, do
  //   a. If number is NaN, return NaN.
  if (hasNaN)
    return HermesValue::encodeNaNValue();

  assert(!(max < 0) && "max must not be negative (max(abs(value))");
  // 6. If onlyZero is true, return +0𝔽.
  if (max == 0) {
    return HermesValue::encodeNumberValue(+0);
  }

  // 7. Return an implementation-approximated Number value representing the
  // square root of the sum of squares of the mathematical values of the
  // elements of coerced.

  // We use the Kahan summation algorithm, since we are supposed to
  // "take care to avoid the loss of precision from overflows and underflows".
  // We add (value / max)**2 each iteration through the loop,
  // so that multiplying by max following the sqrt will negate
  // its effects. This normalizes the values to allow more accurate summation.
  double sum = 0;
  double c = 0;
  for (const double value : coerced) {
    double addend = (value / max) * (value / max);
    // Perform Kahan summation and put the result and compensation in sum and c.
    double y = addend - c;
    double t = sum + y;
    c = (t - sum) - y;
    sum = t;
  }
  double result = std::sqrt(sum) * max;

  return HermesValue::encodeNumberValue(result);
}

// ES6.0 20.2.2.19
// Integer multiplication.
CallResult<HermesValue> mathImul(void *, Runtime &runtime, NativeArgs args) {
  auto res = toUInt32_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t a = res->getNumber();
  res = toUInt32_RJS(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t b = res->getNumber();

  // Compute a * b mod 2^32.
  uint32_t product = a * b;

  // If product >= 2^31, return product - 2^32, else return product.
  return HermesValue::encodeNumberValue(static_cast<int32_t>(product));
}

// ES6.0 20.2.2.11
// Count leading zeros on the 32-bit number.
CallResult<HermesValue> mathClz32(void *, Runtime &runtime, NativeArgs args) {
  auto res = toUInt32_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t n = res->getNumberAs<uint32_t>();
  uint32_t p = llvh::countLeadingZeros(n);
  return HermesValue::encodeNumberValue(p);
}

// ES6.0 20.2.2.29
// Get the sign of the input.
CallResult<HermesValue> mathSign(void *, Runtime &runtime, NativeArgs args) {
  auto res = toNumber_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double x = res->getNumber();

  if (std::isnan(x)) {
    return HermesValue::encodeNaNValue();
  }
  if (x == 0) {
    // Preserve sign bit: return -0 for x == -0 and +0 for x == +0.
    return HermesValue::encodeNumberValue(x);
  }

  return HermesValue::encodeNumberValue(std::signbit(x) ? -1 : +1);
}

Handle<JSObject> createMathObject(Runtime &runtime) {
  auto objRes = JSMath::create(
      runtime, Handle<JSObject>::vmcast(&runtime.objectPrototype));
  assert(objRes != ExecutionStatus::EXCEPTION && "unable to define Math");
  auto math = runtime.makeHandle<JSMath>(*objRes);

  DefinePropertyFlags constantDPF =
      DefinePropertyFlags::getDefaultNewPropertyFlags();
  constantDPF.enumerable = 0;
  constantDPF.writable = 0;
  constantDPF.configurable = 0;

  MutableHandle<> numberHandle{runtime};

  // ES5.1 15.8.1, Math value properties
  auto setMathValueProperty = [&](SymbolID name, double value) {
    numberHandle = HermesValue::encodeNumberValue(value);
    auto result = JSObject::defineOwnProperty(
        math, runtime, name, constantDPF, numberHandle);
    assert(
        result != ExecutionStatus::EXCEPTION &&
        "defineOwnProperty() failed on a new object");
    (void)result;
  };
  setMathValueProperty(Predefined::getSymbolID(Predefined::E), M_E);
  setMathValueProperty(Predefined::getSymbolID(Predefined::LN10), M_LN10);
  setMathValueProperty(Predefined::getSymbolID(Predefined::LN2), M_LN2);
  setMathValueProperty(Predefined::getSymbolID(Predefined::LOG2E), M_LOG2E);
  setMathValueProperty(Predefined::getSymbolID(Predefined::LOG10E), M_LOG10E);
  setMathValueProperty(Predefined::getSymbolID(Predefined::PI), M_PI);
  setMathValueProperty(Predefined::getSymbolID(Predefined::SQRT1_2), M_SQRT1_2);
  setMathValueProperty(Predefined::getSymbolID(Predefined::SQRT2), M_SQRT2);

  // ES5.1 15.8.2, Math function properties
  auto setMathFunctionProperty1Arg = [&runtime, math](
                                         SymbolID name, MathKind kind) {
    defineMethod(runtime, math, name, (void *)kind, runContextFunc1Arg, 1);
  };

  auto setMathFunctionProperty2Arg = [&runtime, math](
                                         SymbolID name, MathKind kind) {
    defineMethod(runtime, math, name, (void *)kind, runContextFunc2Arg, 2);
  };

  // We use the C versions of some of these functions from <math.h>
  // because on Android, the C++ <cmath> library doesn't have them.
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::abs), MathKind::abs);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::acos), MathKind::acos);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::acosh), MathKind::acosh);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::asin), MathKind::asin);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::asinh), MathKind::asinh);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::atan), MathKind::atan);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::atanh), MathKind::atanh);
  setMathFunctionProperty2Arg(
      Predefined::getSymbolID(Predefined::atan2), MathKind::atan2);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::cbrt), MathKind::cbrt);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::ceil), MathKind::ceil);
  defineMethod(
      runtime,
      math,
      Predefined::getSymbolID(Predefined::clz32),
      nullptr,
      mathClz32,
      1);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::cos), MathKind::cos);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::cosh), MathKind::cosh);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::exp), MathKind::exp);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::expm1), MathKind::expm1);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::floor), MathKind::floor);
  defineMethod(
      runtime,
      math,
      Predefined::getSymbolID(Predefined::fround),
      nullptr,
      mathFround,
      1);
  defineMethod(
      runtime,
      math,
      Predefined::getSymbolID(Predefined::hypot),
      nullptr,
      mathHypot,
      2);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::log), MathKind::log);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::log10), MathKind::log10);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::log1p), MathKind::log1p);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::log2), MathKind::log2);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::trunc), MathKind::trunc);
  defineMethod(
      runtime,
      math,
      Predefined::getSymbolID(Predefined::max),
      nullptr,
      mathMax,
      2);
  defineMethod(
      runtime,
      math,
      Predefined::getSymbolID(Predefined::min),
      nullptr,
      mathMin,
      2);
  defineMethod(
      runtime,
      math,
      Predefined::getSymbolID(Predefined::imul),
      nullptr,
      mathImul,
      2);
  defineMethod(
      runtime,
      math,
      Predefined::getSymbolID(Predefined::pow),
      nullptr,
      mathPow,
      2);
  defineMethod(
      runtime,
      math,
      Predefined::getSymbolID(Predefined::random),
      nullptr,
      mathRandom,
      0);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::round), MathKind::round);
  defineMethod(
      runtime,
      math,
      Predefined::getSymbolID(Predefined::sign),
      nullptr,
      mathSign,
      1);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::sin), MathKind::sin);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::sinh), MathKind::sinh);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::sqrt), MathKind::sqrt);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::tan), MathKind::tan);
  setMathFunctionProperty1Arg(
      Predefined::getSymbolID(Predefined::tanh), MathKind::tanh);

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      math,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime.getPredefinedStringHandle(Predefined::Math),
      dpf);

  return math;
}

} // namespace vm
} // namespace hermes
