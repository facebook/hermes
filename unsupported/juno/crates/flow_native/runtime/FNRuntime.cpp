/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FNRuntime.h"

#include <limits>

FNStringTable g_fnStringTable{};
std::vector<FNUniqueString> g_fnCompilerStrings{};

FNValue FNObject::getByName(FNUniqueString key) {
  auto *cur = this;
  do {
    auto it = cur->props.find(key);
    if (it != cur->props.end())
      return it->second;
    cur = cur->parent;
  } while (cur);
  return FNValue::encodeUndefined();
}

void FNObject::putByName(FNUniqueString key, FNValue val) {
  props[key] = val;
}

FNValue FNObject::getByVal(FNValue key) {
  if (key.isString()) {
    return getByName(g_fnStringTable.uniqueString(key.getString()->str));
  } else {
    auto &arr = static_cast<FNArray *>(this)->arr;
    double n = key.getNumber();
    if (arr.size() <= n)
      return FNValue::encodeUndefined();
    return arr[n];
  }
}

void FNObject::putByVal(FNValue key, FNValue val) {
  if (key.isString()) {
    putByName(g_fnStringTable.uniqueString(key.getString()->str), val);
  } else {
    auto &arr = static_cast<FNArray *>(this)->arr;
    double n = key.getNumber();
    if (arr.size() <= n)
      arr.resize(n + 1, FNValue::encodeUndefined());
    arr[n] = val;
  }
}

const FNString *FNValue::typeOf(FNValue v) {
  switch (v.tag) {
    case FNType::Undefined:
      return g_fnStringTable.fnString(FNPredefined::undefined);
    case FNType::Null:
    case FNType::Object:
      return g_fnStringTable.fnString(FNPredefined::object);
    case FNType::Bool:
      return g_fnStringTable.fnString(FNPredefined::boolean);
    case FNType::Closure:
      return g_fnStringTable.fnString(FNPredefined::function);
    case FNType::String:
      return g_fnStringTable.fnString(FNPredefined::string);
    case FNType::Number:
      return g_fnStringTable.fnString(FNPredefined::number);
    case FNType::Symbol:
      return g_fnStringTable.fnString(FNPredefined::symbol);
    default:
      abort();
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
  global->props[g_fnStringTable.uniqueString("print")] =
      FNValue::encodeClosure(printClosure);
  auto *arrayConstructorClosure =
      new FNClosure((void (*)(void))arrayConstructor, nullptr);
  global->props[g_fnStringTable.uniqueString("Array")] =
      FNValue::encodeClosure(arrayConstructorClosure);
  global->props[FNPredefined::undefined] = FNValue::encodeUndefined();
  global->props[g_fnStringTable.uniqueString("Infinity")] =
      FNValue::encodeNumber(std::numeric_limits<double>::infinity());
  global->props[g_fnStringTable.uniqueString("NaN")] =
      FNValue::encodeNumber(std::numeric_limits<double>::quiet_NaN());
  return global;
}

// Use a per-process global object for now, but we will need to support
// multiple instances in the same process eventually.
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
    // and m << exp is exactly INT32_MIN, since a 32-bit signed int cannot
    // hold the resulting INT32_MAX + 1. When it is returned, it will be
    // correctly set to INT32_MIN.
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

FNStringTable::FNStringTable() {
#define FN_PREDEFINED(n)                 \
  do {                                   \
    FNUniqueString n = uniqueString(#n); \
    (void)n;                             \
    assert(n == FNPredefined::n);        \
  } while (0);
#include "predefined.def"
}

FNStringTable::~FNStringTable() noexcept = default;

FNUniqueString FNStringTable::uniqueString(std::string_view s) {
  auto it = map_.find(s);
  if (it != map_.end())
    return it->second;

  auto *newStr = new FNString{std::string(s)};
  strings_.push_back(newStr);
  FNUniqueString res = strings_.size() - 1;
  map_.emplace(newStr->str, res);
  return res;
}
