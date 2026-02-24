/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSLIB_RUNTIMEJSONSTRINGIFY_H
#define HERMES_VM_JSLIB_RUNTIMEJSONSTRINGIFY_H

#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

/// Returns a String in JSON format representing an ECMAScript value,
/// according to 15.12.3.
CallResult<HermesValue> runtimeJSONStringify(
    Runtime &runtime,
    Handle<> value,
    Handle<> replacer,
    Handle<> space);

} // namespace vm
} // namespace hermes

#endif
