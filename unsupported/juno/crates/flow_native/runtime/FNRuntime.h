/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef FNRUNTIME_H
#define FNRUNTIME_H

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

struct FNString;
struct FNObject;
struct FNClosure;

enum class FNType {
  Undefined,
  Null,
  Number,
  Bool,
  String,
  Symbol,
  Object,
  Closure
};

// WARNING: This implementation is TEMPORARY and purely for development
// purposes. It will mostly be deleted once we have real type checking.
class FNValue {
  FNType tag;
  uint64_t value;

  static_assert(
      sizeof(value) >= sizeof(uintptr_t),
      "Value must be able to fit a pointer.");

  void *getPointer() const {
    return reinterpret_cast<FNString *>(static_cast<uintptr_t>(value));
  }

 public:
  bool isUndefined() const {
    return tag == FNType::Undefined;
  }
  bool isNull() const {
    return tag == FNType::Null;
  }
  bool isNumber() const {
    return tag == FNType::Number;
  }
  bool isBool() const {
    return tag == FNType::Bool;
  }
  bool isString() const {
    return tag == FNType::String;
  }
  bool isSymbol() const {
    return tag == FNType::Symbol;
  }
  bool isObject() const {
    return tag == FNType::Object;
  }
  bool isClosure() const {
    return tag == FNType::Closure;
  }

  double getNumber() const {
    assert(isNumber());
    double num;
    memcpy(&num, &value, sizeof(double));
    return num;
  }
  bool getBool() const {
    assert(isBool() || isNumber() || isUndefined() || isNull());
    return value;
  }
  const FNString *getString() const {
    assert(isString());
    return reinterpret_cast<const FNString *>(value);
  }
  FNObject *getObject() const {
    assert(isObject() || isClosure());
    return reinterpret_cast<FNObject *>(value);
  }
  FNClosure *getClosure() const {
    assert(isClosure());
    return reinterpret_cast<FNClosure *>(value);
  }

  static FNValue encodeUndefined() {
    FNValue ret;
    ret.tag = FNType::Undefined;
    // Explicitly initialize value so we can reliably test for equality.
    ret.value = 0;
    return ret;
  }
  static FNValue encodeNull() {
    FNValue ret;
    ret.tag = FNType::Null;
    // Explicitly initialize value so we can reliably test for equality.
    ret.value = 0;
    return ret;
  }
  static FNValue encodeNumber(double num) {
    FNValue ret;
    ret.tag = FNType::Number;
    uint64_t bits;
    memcpy(&bits, &num, sizeof(double));
    ret.value = bits;
    return ret;
  }
  static FNValue encodeBool(bool b) {
    FNValue ret;
    ret.tag = FNType::Bool;
    ret.value = b;
    return ret;
  }
  static FNValue encodeString(const FNString *str) {
    FNValue ret;
    ret.tag = FNType::String;
    ret.value = reinterpret_cast<uint64_t>(str);
    return ret;
  }
  static FNValue encodeObject(FNObject *obj) {
    FNValue ret;
    ret.tag = FNType::Object;
    ret.value = reinterpret_cast<uint64_t>(obj);
    return ret;
  }
  static FNValue encodeClosure(FNClosure *closure) {
    FNValue ret;
    ret.tag = FNType::Closure;
    ret.value = reinterpret_cast<uint64_t>(closure);
    return ret;
  }

  static bool isEqual(FNValue a, FNValue b) {
    return a.tag == b.tag && a.value == b.value;
  }

  static const FNString *typeOf(FNValue v);
};

void *fnMalloc(size_t);

struct FNString {
  std::string str;

  void *operator new(size_t sz) {
    return fnMalloc(sz);
  }
};
struct FNObject {
  std::unordered_map<std::string, FNValue> props;
  FNObject *parent{};

  FNValue getByVal(FNValue key);
  void putByVal(FNValue key, FNValue val);

  void *operator new(size_t sz) {
    return fnMalloc(sz);
  }
};
struct FNClosure : public FNObject {
  explicit FNClosure(void (*func)(void), void *env) : func(func), env(env) {
    props["prototype"] = FNValue::encodeObject(new FNObject());
  }
  void *operator new(size_t sz) {
    return fnMalloc(sz);
  }

  void (*func)(void);
  void *env;
};
struct FNArray : public FNObject {
  explicit FNArray(std::vector<FNValue> arr) : arr(std::move(arr)) {}
  void *operator new(size_t sz) {
    return fnMalloc(sz);
  }

  std::vector<FNValue> arr;
};

FNObject *global();

int32_t truncateToInt32SlowPath(double d);

/// Convert a double to a 32-bit integer according to ES5.1 section 9.5.
/// It can also be used for converting to an unsigned integer, which has the
/// same bit pattern.
/// NaN and Infinity are always converted to 0. The rest of the numbers are
/// converted to a (conceptually) infinite-width integer and the low 32 bits of
/// the integer are then returned.
int32_t truncateToInt32(double d);
inline int32_t truncateToInt32(double d) {
  // Check of the value can be converted to integer without loss. We want to
  // use the widest available integer because this conversion will be much
  // faster than the bit-twiddling slow path.
  intmax_t fast = (intmax_t)d;
  if (fast == d)
    return (int32_t)fast;
  return truncateToInt32SlowPath(d);
}

#endif // FNRUNTIME_H
