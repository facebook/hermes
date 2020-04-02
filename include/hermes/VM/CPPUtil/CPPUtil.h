/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CPPUTIL_CPPUTIL_H
#define HERMES_VM_CPPUTIL_CPPUTIL_H

#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/IdentifierTable.h"
#include "hermes/VM/Interpreter.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

double makeDouble(uint64_t n);

ExecutionStatus putGetterSetter(
    Runtime *runtime,
    Handle<JSObject> object,
    SymbolID name,
    Handle<> getterHandle,
    Handle<> setterHandle);

CallResult<HermesValue>
getIsIn(Runtime *runtime, Handle<> name, Handle<> object);

} // namespace vm
} // namespace hermes

#endif
