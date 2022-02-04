/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HermesValue.h"

#include "hermes/VM/GC.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"

#include "hermes/Support/UTF8.h"

#include "llvh/Support/ErrorHandling.h"
#include "llvh/Support/Format.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {
namespace vm {

void HermesValue::dump(llvh::raw_ostream &os) const {
  os << *this << "\n";
}

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, HermesValue hv) {
  switch (hv.getTag()) {
    case ObjectTag: {
      auto *cell = static_cast<GCCell *>(hv.getObject());
      return OS << "[Object " << (cell ? cellKindStr(cell->getKind()) : "")
                << ":" << (cell ? cell->getDebugAllocationId() : 0) << " "
                << llvh::format_hex((uintptr_t)cell, 10) << "]";
    }
    case StrTag: {
      auto *cell = static_cast<GCCell *>(hv.getPointer());
      OS << "[String "
         << ":" << (cell ? cell->getDebugAllocationId() : 0) << " "
         << llvh::format_hex((uintptr_t)cell, 10);
      // Note contained StringPrimitive may be NULL.
      if (hv.getString()) {
        llvh::SmallVector<char16_t, 16> storage;
        hv.getString()->appendUTF16String(storage);
        std::string narrowStr;
        convertUTF16ToUTF8WithReplacements(narrowStr, storage);
        OS << " '";
        OS.write_escaped(narrowStr);
        OS << "'";
      }
      return OS << ']';
    }
    case NativeValueTag:
      return OS << "[NativeValue " << hv.getNativeUInt32() << "]";
    case SymbolTag:
      return OS << "[Symbol "
                << (hv.getSymbol().isNotUniqued() ? "(External)" : "(Internal)")
                << ' ' << hv.getSymbol().unsafeGetIndex() << "]";
    case BoolTag:
      return OS << (hv.getBool() ? "true" : "false");
    case UndefinedNullTag:
      return OS << (hv.isNull() ? "null" : "undefined");
    case EmptyInvalidTag:
#ifdef HERMES_SLOW_DEBUG
      if (hv.isInvalid())
        return OS << "invalid";
#endif
      return OS << "empty";
    default:
      double num = hv.getDouble();
      // Is it representable as int64_t?
      if (num >= std::numeric_limits<int64_t>::lowest() &&
          // The cast is to ignore the following warning:
          // implicit conversion from 'int64_t' to 'double' changes value.
          num <= (double)std::numeric_limits<int64_t>::max() &&
          (int64_t)num == num) {
        return OS << "[double " << (int64_t)num << "]";
      } else {
        return OS << "[double " << num << "]";
      }
  }
}

} // namespace vm
} // namespace hermes
