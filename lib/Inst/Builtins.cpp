#include "hermes/Inst/Builtins.h"

#include <cassert>

namespace hermes {
namespace inst {

static const char *builtinName[] = {
#define BUILTIN_METHOD(object, name) #object "." #name,
#include "hermes/Inst/Builtins.def"
};

const char *getBuiltinMethodName(int method) {
  assert(
      method >= 0 && method < BuiltinMethod::_count &&
      "invalid builtin method index");
  return builtinName[method];
}

} // namespace inst
} // namespace hermes
