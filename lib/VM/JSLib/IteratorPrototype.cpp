/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// ES6.0 25.1.2 The %IteratorPrototype% Object
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

void populateIteratorPrototype(Runtime &runtime) {
  auto proto = Handle<JSObject>::vmcast(&runtime.iteratorPrototype);

  auto iteratorFunc = NativeFunction::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime.functionPrototype),
      nullptr,
      iteratorPrototypeIterator,
      Predefined::getSymbolID(Predefined::squareSymbolIterator),
      0,
      Runtime::makeNullHandle<JSObject>());

  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolIterator),
      runtime.makeHandle<NativeFunction>(*iteratorFunc));
}

CallResult<HermesValue>
iteratorPrototypeIterator(void *, Runtime &runtime, NativeArgs args) {
  return args.getThisArg();
}

} // namespace vm
} // namespace hermes
