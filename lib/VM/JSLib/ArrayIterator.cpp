//===----------------------------------------------------------------------===//
/// \file
/// ES6.0 22.1.5 Array Iterator Objects
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/VM/JSArray.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

/// ES6.0 22.1.5.2.1
static CallResult<HermesValue>
arrayIteratorPrototypeNext(void *, Runtime *runtime, NativeArgs args);

void populateArrayIteratorPrototype(Runtime *runtime) {
  auto proto = Handle<JSObject>::vmcast(&runtime->arrayIteratorPrototype);

  defineMethod(
      runtime,
      proto,
      runtime->getPredefinedSymbolID(Predefined::next),
      nullptr,
      arrayIteratorPrototypeNext,
      0);

  auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  defineProperty(
      runtime,
      proto,
      runtime->getPredefinedSymbolID(Predefined::SymbolToStringTag),
      runtime->getPredefinedStringHandle(Predefined::ArrayIterator),
      dpf);
}

static CallResult<HermesValue>
arrayIteratorPrototypeNext(void *, Runtime *runtime, NativeArgs args) {
  auto O = args.dyncastThis<JSArrayIterator>(runtime);
  if (LLVM_UNLIKELY(!O)) {
    return runtime->raiseTypeError(
        "ArrayIteratorPrototype.next requires that 'this' be an Array Iterator");
  }
  return JSArrayIterator::nextElement(O, runtime);
}

} // namespace vm
} // namespace hermes
