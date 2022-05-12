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

struct FNValue;

struct FNString {
  std::string str;
};
struct FNObject {
  std::unordered_map<std::string, FNValue> props;

  FNValue &getByVal(FNValue key);
};
struct FNClosure : public FNObject {
  explicit FNClosure(void (*func)(void), void *env) : func(func), env(env) {}

  void (*func)(void);
  void *env;
};
struct FNArray : public FNObject {
  explicit FNArray(std::vector<FNValue> arr) : arr(arr) {}

  std::vector<FNValue> arr;
};

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
  union {
    double num;
    bool b;
    FNString *str;
    FNObject *obj;
    FNClosure *closure;
  } value;

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

  double &getNumberRef() {
    assert(isNumber());
    return value.num;
  }

  double getNumber() const {
    assert(isNumber());
    return value.num;
  }
  bool getBool() const {
    assert(isBool());
    return value.b;
  }
  FNString *getString() const {
    assert(isString());
    return value.str;
  }
  FNObject *getObject() const {
    assert(isObject());
    return value.obj;
  }
  FNClosure *getClosure() const {
    assert(isClosure());
    return value.closure;
  }

  static FNValue encodeUndefined() {
    FNValue ret;
    ret.tag = FNType::Undefined;
    return ret;
  }
  static FNValue encodeNull() {
    FNValue ret;
    ret.tag = FNType::Null;
    return ret;
  }
  static FNValue encodeNumber(double num) {
    FNValue ret;
    ret.tag = FNType::Number;
    ret.value.num = num;
    return ret;
  }
  static FNValue encodeBool(bool b) {
    FNValue ret;
    ret.tag = FNType::Bool;
    ret.value.b = b;
    return ret;
  }
  static FNValue encodeString(FNString *str) {
    FNValue ret;
    ret.tag = FNType::String;
    ret.value.str = str;
    return ret;
  }
  static FNValue encodeObject(FNObject *obj) {
    FNValue ret;
    ret.tag = FNType::Object;
    ret.value.obj = obj;
    return ret;
  }
  static FNValue encodeClosure(FNClosure *closure) {
    FNValue ret;
    ret.tag = FNType::Closure;
    ret.value.closure = closure;
    return ret;
  }
};

#endif // FNRUNTIME_H
