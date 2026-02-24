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
  struct : public Locals {
    PinnedValue<NativeFunction> iteratorFunc;
  } lv;
  LocalsRAII lraii(runtime, &lv);

  auto proto = Handle<JSObject>::vmcast(&runtime.iteratorPrototype);

  lv.iteratorFunc.castAndSetHermesValue<NativeFunction>(
      NativeFunction::create(
          runtime,
          Handle<JSObject>::vmcast(&runtime.functionPrototype),
          Runtime::makeNullHandle<Environment>(),
          nullptr,
          iteratorPrototypeIterator,
          Predefined::getSymbolID(Predefined::squareSymbolIterator),
          0,
          Runtime::makeNullHandle<JSObject>())
          .getHermesValue());

  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolIterator),
      lv.iteratorFunc);
}

CallResult<HermesValue> iteratorPrototypeIterator(void *, Runtime &runtime) {
  NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
  return args.getThisArg();
}

} // namespace vm
} // namespace hermes
