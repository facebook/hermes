/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HermesValue.h"

#include "hermes/VM/GC.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"

#include "hermes/Support/UTF8.h"

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

namespace hermes {
namespace vm {

void HermesValue::dump(llvm::raw_ostream &os) const {
  os << *this << "\n";
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, HermesValue hv) {
  switch (hv.getTag()) {
    case ObjectTag: {
      auto *cell = static_cast<GCCell *>(hv.getObject());
      return OS << "[Object " << (cell ? cellKindStr(cell->getKind()) : "")
                << ":" << (cell ? cell->getDebugAllocationId() : 0) << " "
                << llvm::format_hex((uintptr_t)cell, 10) << "]";
    }
    case StrTag: {
      auto *cell = static_cast<GCCell *>(hv.getPointer());
      OS << "[String "
         << ":" << (cell ? cell->getDebugAllocationId() : 0) << " "
         << llvm::format_hex((uintptr_t)cell, 10);
      // Note contained StringPrimitive may be NULL.
      if (hv.getString()) {
        llvm::SmallVector<char16_t, 16> storage;
        hv.getString()->copyUTF16String(storage);
        std::string narrowStr;
        convertUTF16ToUTF8WithReplacements(narrowStr, storage);
        OS << " '";
        OS.write_escaped(narrowStr);
        OS << "'";
      }
      return OS << ']';
    }
    case NativeValueTag:
      return OS << "[NativeValue " << hv.getNativeValue() << "]";
    case SymbolTag:
      return OS << "[Symbol "
                << (hv.getSymbol().isNotUniqued() ? "(External)" : "(Internal)")
                << ' ' << hv.getSymbol().unsafeGetIndex() << "]";
    case BoolTag:
      return OS << (hv.getBool() ? "true" : "false");
    case NullTag:
      return OS << "null";
    case UndefinedTag:
      return OS << "undefined";
    case EmptyTag:
      return OS << "empty";
    default:
      double num = hv.getDouble();
      // Is it representable as int64_t?
      if (num >= std::numeric_limits<int64_t>::lowest() &&
          num <= std::numeric_limits<int64_t>::max() && (int64_t)num == num) {
        return OS << "[double " << (int64_t)num << "]";
      } else {
        return OS << "[double " << num << "]";
      }
  }
}

} // namespace vm
} // namespace hermes
