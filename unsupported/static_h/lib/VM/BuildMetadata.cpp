/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/CellKind.h"

#include "llvh/Support/Debug.h"

#include <cstdint>

#define DEBUG_TYPE "metadata"

namespace hermes {
namespace vm {

Metadata buildMetadata(CellKind kind, BuildMetadataCallback *builder) {
  const GCCell *base;
#ifdef HERMES_UBSAN
  // Using members on a nullptr is UB, but using a pointer to static memory is
  // not.
  static const int64_t buf[128]{};
  base = reinterpret_cast<const GCCell *>(&buf);
#else
  // Use nullptr when not building with UBSAN to ensure fast failures.
  base = nullptr;
#endif
  // This memory should not be read or written to
  Metadata::Builder mb(base);
  builder(base, mb);
  Metadata meta = mb.build();
  LLVM_DEBUG(
      llvh::dbgs() << "Initialized metadata for cell kind " << cellKindStr(kind)
                   << ": " << meta << "\n");
  return meta;
}

void buildMetadataTable() {
  // Once the storage is initialized, also initialize the global array of VTable
  // pointers using the new metadata.
  static std::once_flag flag;
  std::call_once(flag, [] {
    // For each class of object, initialize its metadata
    // Only run this once per class, not once per Runtime instantiation.
    Metadata::metadataTable = {
#define CELL_KIND(name) buildMetadata(CellKind::name##Kind, name##BuildMeta),
#include "hermes/VM/CellKinds.def"
#undef CELL_KIND
    };

    assert(
        !VTable::vtableArray[0] &&
        "VTable array should not be initialized before this point.");
    VTable::vtableArray = {
#define CELL_KIND(name) \
  Metadata::metadataTable[static_cast<uint8_t>(CellKind::name##Kind)].vtp,
#include "hermes/VM/CellKinds.def"
#undef CELL_KIND
    };
  });
}

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
