/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
/// \file
/// ES6.0 25.3.1 The %GeneratorPrototype% Object
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/VM/JSGenerator.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

void populateGeneratorPrototype(Runtime *runtime) {
  auto proto = Handle<JSObject>::vmcast(&runtime->generatorPrototype);

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;

  // The initial value of GeneratorFunction.prototype.constructor is the
  // intrinsic object %Generator% (which is GeneratorFunction.prototype).
  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::constructor),
      Handle<>(&runtime->generatorFunctionPrototype),
      dpf);

  defineProperty(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolToStringTag),
      runtime->getPredefinedStringHandle(Predefined::Generator),
      dpf);
}

} // namespace vm
} // namespace hermes
