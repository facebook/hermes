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
