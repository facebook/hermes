//===----------------------------------------------------------------------===//
/// \file
/// ES6.0 25.1.2 The %IteratorPrototype% Object
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

/// ES6.0 25.1.2.1.
static CallResult<HermesValue>
iteratorPrototypeIterator(void *, Runtime *runtime, NativeArgs args);

void populateIteratorPrototype(Runtime *runtime) {
  auto proto = Handle<JSObject>::vmcast(&runtime->iteratorPrototype);

  auto iteratorFunc = NativeFunction::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime->functionPrototype),
      nullptr,
      iteratorPrototypeIterator,
      runtime->getPredefinedSymbolID(Predefined::squareSymbolIterator),
      0,
      runtime->makeNullHandle<JSObject>());

  defineProperty(
      runtime,
      proto,
      runtime->getPredefinedSymbolID(Predefined::SymbolIterator),
      runtime->makeHandle<NativeFunction>(*iteratorFunc));
}

static CallResult<HermesValue>
iteratorPrototypeIterator(void *, Runtime *runtime, NativeArgs args) {
  return args.getThisArg();
}

} // namespace vm
} // namespace hermes
