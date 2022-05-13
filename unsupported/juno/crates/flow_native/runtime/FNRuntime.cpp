/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FNRuntime.h"

// This is a TEMPORARY implementation that will insert elements if they don't
// already exist. The behaviour will change once the compiler distinguishes
// lvalues.
FNValue &FNObject::getByVal(FNValue key) {
  if (key.isString())
    return props[key.getString()->str];
  auto &arr = static_cast<FNArray *>(this)->arr;
  double n = key.getNumber();
  if (arr.size() <= n)
    arr.resize(n + 1, FNValue::encodeUndefined());
  return arr[n];
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
  return global;
}

// Use a per-process global object for now, but we will need to support multiple
// instances in the same process eventually.
FNObject *global() {
  static FNObject *global = createGlobalObject();
  return global;
}
