/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/FrontEndDefs/Builtins.h"

#include <cassert>

namespace hermes {

static const char *builtinName[] = {
#define BUILTIN_METHOD(object, name) #object "." #name,
#define PRIVATE_BUILTIN(name) BUILTIN_METHOD(HermesBuiltin, name)
#define JS_BUILTIN(name) BUILTIN_METHOD(HermesBuiltin, name)
#include "hermes/FrontEndDefs/Builtins.def"
};

const char *getBuiltinMethodName(int method) {
  assert(
      method >= 0 && method < BuiltinMethod::_count &&
      "invalid builtin method index");
  return builtinName[method];
}

} // namespace hermes
