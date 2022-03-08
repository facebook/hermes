/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_JSLIB_OBJECT_H
#define HERMES_JSLIB_OBJECT_H

#include "hermes/VM/CallResult.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/NativeArgs.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

// This file declares some functions in Object which are also used by
// Reflect.

CallResult<bool>
defineProperty(Runtime &runtime, NativeArgs args, PropOpFlags opFlags);

CallResult<HermesValue> getOwnPropertyDescriptor(
    Runtime &runtime,
    Handle<JSObject> object,
    Handle<> key);

CallResult<HermesValue> getPrototypeOf(Runtime &runtime, Handle<JSObject> obj);

CallResult<HermesValue> getOwnPropertyKeysAsStrings(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    OwnKeysFlags okFlags);

/// "Kind" provided to enumerableOwnProperties to request different
/// representation of the properties in the object.
enum class EnumerableOwnPropertiesKind {
  Key,
  Value,
  KeyValue,
};

/// ES8.0 7.3.21.
/// EnumerableOwnProperties gets the requested properties based on \p kind.
CallResult<HermesValue> enumerableOwnProperties_RJS(
    Runtime &runtime,
    Handle<JSObject> objHandle,
    EnumerableOwnPropertiesKind kind);

} // namespace vm
} // namespace hermes

#endif
