/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FNRuntime.h"

#include <limits>

FNValue FNObject::getByVal(FNValue key) {
  if (key.isString()) {
    auto *cur = this;
    do {
      auto it = cur->props.find(key.getString()->str);
      if (it != cur->props.end())
        return it->second;
      cur = cur->parent;
    } while (cur);
    return FNValue::encodeUndefined();
  } else {
    auto &arr = static_cast<FNArray *>(this)->arr;
    double n = key.getNumber();
    if (arr.size() <= n)
      return FNValue::encodeUndefined();
    return arr[n];
  }
}

void FNObject::putByVal(FNValue key, FNValue val) {
  if (key.isString())
    props[key.getString()->str] = val;
  else {
    auto &arr = static_cast<FNArray *>(this)->arr;
    double n = key.getNumber();
    if (arr.size() <= n)
      arr.resize(n + 1, FNValue::encodeUndefined());
    arr[n] = val;
  }
}

static const FNString kUndefinedStr{"undefined"};
static const FNString kObjectStr{"object"};
static const FNString kBooleanStr{"boolean"};
static const FNString kFunctionStr{"function"};
static const FNString kStringStr{"string"};
static const FNString kNumberStr{"number"};
static const FNString kSymbolStr{"symbol"};

const FNString *FNValue::typeOf(FNValue v) {
  switch (v.tag) {
    case FNType::Undefined:
      return &kUndefinedStr;
    case FNType::Null:
      return &kObjectStr;
    case FNType::Object:
      return &kObjectStr;
    case FNType::Bool:
      return &kBooleanStr;
    case FNType::Closure:
      return &kFunctionStr;
    case FNType::String:
      return &kStringStr;
    case FNType::Number:
      return &kNumberStr;
    case FNType::Symbol:
      return &kSymbolStr;
  }
}

static FNValue print(void *, FNValue, FNValue arg) {
  if (arg.isUndefined())
    printf("undefined");
  else if (arg.isNull())
    printf("null");
  else if (arg.isNumber())
    printf("%f", arg.getNumber());
  else if (arg.isBool())
    printf("%s", arg.getBool() ? "true" : "false");
  else if (arg.isString())
    printf("%s", arg.getString()->str.c_str());
  else if (arg.isObject())
    printf("[Object]");
  else if (arg.isClosure())
    printf("[Closure]");
  return FNValue::encodeUndefined();
}

static FNValue arrayConstructor(void *, FNValue, FNValue size) {
  auto *arr = new FNArray({});
  arr->arr.resize(size.getNumber(), FNValue::encodeUndefined());
  return FNValue::encodeObject(arr);
}

static FNObject *createGlobalObject() {
  auto *global = new FNObject();
  auto *printClosure = new FNClosure((void (*)(void))print, nullptr);
  global->props["print"] = FNValue::encodeClosure(printClosure);
  auto *arrayConstructorClosure =
      new FNClosure((void (*)(void))arrayConstructor, nullptr);
  global->props["Array"] = FNValue::encodeClosure(arrayConstructorClosure);
  global->props["undefined"] = FNValue::encodeUndefined();
  global->props["Infinity"] =
      FNValue::encodeNumber(std::numeric_limits<double>::infinity());
  global->props["NaN"] =
      FNValue::encodeNumber(std::numeric_limits<double>::quiet_NaN());
  return global;
}

// Use a per-process global object for now, but we will need to support multiple
// instances in the same process eventually.
FNObject *global() {
  static FNObject *global = createGlobalObject();
  return global;
}

int32_t truncateToInt32SlowPath(double d) {
  uint64_t bits;
  memcpy(&bits, &d, sizeof(double));
  int exp = (int)(bits >> 52) & 0x7FF;
  // A negative sign is turned into 2, a positive into 0. Subtracting from 1
  // gives us what we need.
  int sign = 1 - ((int)((int64_t)bits >> 62) & 2);
  uint64_t m = bits & 0xFFFFFFFFFFFFFul;

  // Check for a denormalized exponent. We can bail early in that case.
  if (!exp)
    return 0;

  // Subtract the IEEE bias (1023). Additionally, move the decimal point to
  // the right of the mantissa by further decreasing the exponent by 52.
  exp -= 1023 + 52;
  // Add the implied leading 1 bit.
  m |= 1ull << 52;

  // The sign of the exponent tells us which way to shift.
  if (exp >= 0) {
    // Check if the shift would push all bits out. Additionally this catches
    // Infinity and NaN.
    // Cast to int64 here to avoid UB for the case where sign is negative one
    // and m << exp is exactly INT32_MIN, since a 32-bit signed int cannot hold
    // the resulting INT32_MAX + 1. When it is returned, it will be correctly
    // set to INT32_MIN.
    return exp <= 31 ? sign * (int64_t)(m << exp) : 0;
  } else {
    // Check if the shift would push out the entire mantissa.
    // We need to use int64_t here in case we are multiplying
    // -1 and 2147483648.
    return exp > -53 ? sign * (int64_t)(m >> -exp) : 0;
  }
}

void *fnMalloc(size_t sz) {
  static constexpr size_t blockSize = 10 * 1024;

  static char *level = nullptr;
  static char *end = nullptr;

  while (true) {
    if (end - level > sz)
      return std::exchange(level, level + sz);

    if (sz > blockSize)
      return malloc(sz);

    level = (char *)malloc(blockSize);
    end = level + blockSize;
  }
}
