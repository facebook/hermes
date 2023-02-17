#ifndef HERMES_VM_JSLIB_RUNTIMEFASTJSONPARSER_H
#define HERMES_VM_JSLIB_RUNTIMEFASTJSONPARSER_H

#include "hermes/Support/UTF16Stream.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

CallResult<HermesValue> runtimeFastJSONParse(
    Runtime &runtime,
    Handle<StringPrimitive> jsonString,
    Handle<Callable> reviver);

} // namespace vm
} // namespace hermes

#endif
