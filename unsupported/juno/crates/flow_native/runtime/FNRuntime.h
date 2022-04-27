/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>
#include <unordered_map>

struct FNValue;

struct FNString {
  std::string str;
};
struct FNObject {
  std::unordered_map<std::string, FNValue> props;
};
struct FNClosure {
  void (*func)(void);
  void *env;
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
struct FNValue {
  FNType tag;
  union {
    double num;
    bool b;
    FNString *str;
    FNObject *obj;
    FNClosure *closure;
  } value;

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
