#include "hermes/VM/VTable.h"

namespace hermes {
namespace vm {

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const VTable &vt) {
  return os << "VTable: {\n\tsize: " << vt.size
            << ", finalize: " << reinterpret_cast<void *>(vt.finalize_)
            << ", markWeak: " << reinterpret_cast<void *>(vt.markWeak_) << "}";
}
} // namespace vm
} // namespace hermes
