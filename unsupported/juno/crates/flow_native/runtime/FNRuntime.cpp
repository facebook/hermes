/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FNRuntime.h"

#include <limits>

FNStringTable g_fnStringTable{};
fn_vector<FNUniqueString> g_fnCompilerStrings{};

FNPropMap::FNPropMap() : data_((value_type *)small_) {
  static_assert(FNPredefined::_EMPTY == 0, "_EMPTY state must be 0");
  memset(small_, 0, sizeof(small_));
}

// NOTE: we are leaking the table!
FNPropMap::~FNPropMap() noexcept = default;

void FNPropMap::erase(value_type *pos) {
  if (!pos)
    return;
  assert(FNPredefined::isValid(pos->first) && "erasing an invalid entry");
  pos->~value_type();
  pos->first = FNPredefined::_DELETED;
}

void FNPropMap::assign(FNUniqueString key, FNValue value) {
  {
    // Ensure that we can always find at least one empty slot.
    assert(limit_ < capacity_);
    assert(occupiedSlots_ <= limit_);

    auto [found, it] = lookup(key);
    if (found) {
      new (&it->second) FNValue(std::move(value));
      return;
    }

    // Increase the occupied slots, unless we are overwriting a deleted slot.
    if (it->first != FNPredefined::_DELETED)
      ++occupiedSlots_;
    // Write the new pair.
    new (it) value_type(key, std::move(value));

    // If we don't have to grow, we are done.
    if (occupiedSlots_ <= limit_)
      return;
  }

  // We need to grow the table. Allocate double the capacity.
  size_type newCapa = capacity_ * 2;
  if (newCapa <= capacity_)
    abort(); // Capacity overflow.
  value_type *newData = (value_type *)fnMalloc(sizeof(value_type) * newCapa);
  if (!newData)
    abort(); // OOM.
  memset(newData, 0, sizeof(value_type) * newCapa);

  // Save the current data and install the new table as the current one.
  value_type *oldData = data_;
  value_type *oldEnd = data_ + capacity_;
  data_ = newData;
  capacity_ = newCapa;
  occupiedSlots_ = 0;
  limit_ = capacity_ / 4 * 3;

  // Copy all valid entries.
  for (auto *cur = oldData; cur != oldEnd; ++cur) {
    if (!FNPredefined::isValid(cur->first))
      continue;
    auto res = lookup(cur->first);
    assert(
        !res.first && "lookup when copying new hash table must never succeed");
    new (res.second) value_type(cur->first, std::move(cur->second));
    ++occupiedSlots_;
  }
  // Note that we are not freeing the old data.
}

std::pair<bool, FNPropMap::value_type *> FNPropMap::lookup(FNUniqueString key) {
  size_type const mask = capacity_ - 1;
  size_type index = key & mask;

  // Probing step.
  size_type step = 1;
  // Save the address of the start of the table to avoid recalculating it.
  value_type *const tableStart = data_;
  // The first deleted entry we found.
  value_type *deleted = nullptr;

  for (;;) {
    value_type *curEntry = tableStart + index;

    if (curEntry->first == key) {
      return {true, curEntry};
    } else if (curEntry->first == FNPredefined::_EMPTY) {
      // If we encountered an empty pair, the search is over - we failed.
      // Return either this entry or a deleted one, if we encountered one.
      return {false, deleted ? deleted : curEntry};
    } else if (curEntry->first == FNPredefined::_DELETED) {
      // The first time we encounter a deleted entry, record it so we can
      // potentially reuse it for insertion.
      if (!deleted)
        deleted = curEntry;
    }

    index = (index + step) & mask;
    ++step;
  }
}

FNValue FNObject::getByName(FNUniqueString key) {
  auto *cur = this;
  do {
    if (auto it = cur->props.findOrNull(key))
      return it->second;
    cur = cur->parent;
  } while (cur);
  return FNValue::encodeUndefined();
}

void FNObject::putByName(FNUniqueString key, FNValue val) {
  props.assign(key, val);
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
  global->props.assign(
      g_fnStringTable.uniqueString("print"),
      FNValue::encodeClosure(printClosure));
  auto *arrayConstructorClosure =
      new FNClosure((void (*)(void))arrayConstructor, nullptr);
  global->props.assign(
      g_fnStringTable.uniqueString("Array"),
      FNValue::encodeClosure(arrayConstructorClosure));
  global->props.assign(FNPredefined::undefined, FNValue::encodeUndefined());
  global->props.assign(
      g_fnStringTable.uniqueString("Infinity"),
      FNValue::encodeNumber(std::numeric_limits<double>::infinity()));
  global->props.assign(
      g_fnStringTable.uniqueString("NaN"),
      FNValue::encodeNumber(std::numeric_limits<double>::quiet_NaN()));
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
  // Skip the empty and deleted strings.
  strings_.push_back(nullptr);
  strings_.push_back(nullptr);
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

  auto *newStr = new FNString{fn_string(s)};
  strings_.push_back(newStr);
  FNUniqueString res = strings_.size() - 1;
  map_.emplace(newStr->str, res);
  return res;
}
