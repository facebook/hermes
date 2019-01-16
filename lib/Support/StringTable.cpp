#include "hermes/Support/StringTable.h"
#include "llvm/Support/raw_ostream.h"

namespace hermes {

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, Identifier id) {
  return os << id.str();
}

} // namespace hermes
