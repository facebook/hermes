/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSLIB_RUNTIMEJSONPARSE_H
#define HERMES_VM_JSLIB_RUNTIMEJSONPARSE_H

#include "hermes/VM/Runtime.h"

namespace hermes {

class UTF16Stream;

namespace vm {

/// Parse JSON string \p jsonString according to ES5.1 15.12.1 and 15.12.2.
CallResult<HermesValue> runtimeJSONParse(
    Runtime &runtime,
    Handle<StringPrimitive> jsonString,
    Handle<Callable> reviver);

/// Alternative interface to runtimeJSONParse for strings outside the JS heap.
CallResult<HermesValue> runtimeJSONParseRef(Runtime &runtime, UTF16Stream &&s);

} // namespace vm
} // namespace hermes

#endif
