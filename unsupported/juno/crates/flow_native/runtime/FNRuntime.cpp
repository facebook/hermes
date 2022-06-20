/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FNRuntime.h"

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
