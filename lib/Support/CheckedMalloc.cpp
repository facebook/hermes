#include "hermes/Support/CheckedMalloc.h"

#include "hermes/Support/ErrorHandling.h"

#include "llvm/Support/Compiler.h"

namespace hermes {

void *checkedMalloc(size_t sz) {
  void *res = ::malloc(sz);
  if (LLVM_UNLIKELY(!res)) {
    hermes_fatal("malloc failure");
  }
  return res;
}

void *checkedCalloc(size_t elemSize, size_t numElems) {
  void *res = calloc(elemSize, numElems);
  if (LLVM_UNLIKELY(!res)) {
    hermes_fatal("calloc failure");
  }
  return res;
}

} // namespace hermes
