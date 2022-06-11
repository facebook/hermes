/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FNRuntime.h"

FNValue FNObject::getByVal(FNValue key) {
  if (key.isString()) {
    auto it = props.find(key.getString()->str);
    if (it == props.end())
      return FNValue::encodeUndefined();
    return it->second;
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

static FNValue print(void *, FNValue arg) {
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

static FNObject *createGlobalObject() {
  auto *global = new FNObject();
  auto *printClosure = new FNClosure((void (*)(void))print, nullptr);
  global->props["print"] = FNValue::encodeClosure(printClosure);
  global->props["undefined"] = FNValue::encodeUndefined();
  return global;
}

// Use a per-process global object for now, but we will need to support multiple
// instances in the same process eventually.
FNObject *global() {
  static FNObject *global = createGlobalObject();
  return global;
}
