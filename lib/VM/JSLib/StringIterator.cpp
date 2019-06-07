/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
/// \file
/// ES6.0 21.1.5 String Iterator Objects
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

/// ES6.0 21.1.5.2.1
static CallResult<HermesValue>
stringIteratorPrototypeNext(void *, Runtime *runtime, NativeArgs args);

void populateStringIteratorPrototype(Runtime *runtime) {
  auto proto = Handle<JSObject>::vmcast(&runtime->stringIteratorPrototype);

  defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::next),
      nullptr,
      stringIteratorPrototypeNext,
      0);

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime->getPredefinedStringHandle(Predefined::StringIterator),
      dpf);
}

static CallResult<HermesValue>
stringIteratorPrototypeNext(void *, Runtime *runtime, NativeArgs args) {
  auto O = args.dyncastThis<JSStringIterator>(runtime);
  if (LLVM_UNLIKELY(!O)) {
    return runtime->raiseTypeError(
        "StringIteratorPrototype.next requires 'this' is a String Iterator");
  }
  return JSStringIterator::nextElement(O, runtime);
}

} // namespace vm
} // namespace hermes
