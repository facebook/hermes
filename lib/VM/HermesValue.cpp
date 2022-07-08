/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HermesValue.h"

#include "hermes/VM/BigIntPrimitive.h"
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
  switch (hv.getETag()) {
    case HermesValue::ETag::Object1:
    case HermesValue::ETag::Object2: {
      auto *cell = static_cast<GCCell *>(hv.getObject());
      return OS << "[Object " << (cell ? cellKindStr(cell->getKind()) : "")
                << ":" << (cell ? cell->getDebugAllocationId() : 0) << " "
                << llvh::format_hex((uintptr_t)cell, 10) << "]";
    }
    case HermesValue::ETag::Str1:
    case HermesValue::ETag::Str2: {
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
    case HermesValue::ETag::BigInt1:
    case HermesValue::ETag::BigInt2: {
      auto *cell = static_cast<GCCell *>(hv.getPointer());
      OS << "[BigInt "
         << ":" << (cell ? cell->getDebugAllocationId() : 0) << " "
         << llvh::format_hex((uintptr_t)cell, 10);
      // Note contained BigIntPrimitive may be NULL.
      if (hv.getBigInt()) {
        llvh::ArrayRef<uint8_t> storage = hv.getBigInt()->getRawDataCompact();
        for (auto it = storage.rbegin(); it != storage.rend(); ++it) {
          OS << " " << llvh::format_hex(*it, 2);
        }
      }
      return OS << ']';
    }
    case HermesValue::ETag::Native1:
    case HermesValue::ETag::Native2:
      return OS << "[NativeValue " << hv.getNativeUInt32() << "]";
    case HermesValue::ETag::Symbol:
      return OS << "[Symbol "
                << (hv.getSymbol().isNotUniqued() ? "(External)" : "(Internal)")
                << ' ' << hv.getSymbol().unsafeGetIndex() << "]";
    case HermesValue::ETag::Bool:
      return OS << (hv.getBool() ? "true" : "false");
    case HermesValue::ETag::Undefined:
      return OS << "undefined";
    case HermesValue::ETag::Null:
      return OS << "null";
    case HermesValue::ETag::Empty:
      return OS << "empty";
#ifdef HERMES_SLOW_DEBUG
    case HermesValue::ETag::Invalid:
      return OS << "invalid";
#endif
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
