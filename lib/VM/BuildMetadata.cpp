/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

static size_t getNumCellKinds() {
  // This embeds the same value as the CellKind enum, but adds one more at the
  // end to know how many values it contains.
  enum CellKinds {
#define CELL_KIND(name) name,
#include "hermes/VM/CellKinds.def"
#undef CELL_KIND
    numKinds,
  };
  return numKinds;
}

/// Creates and populates the storage for the metadata of the classes.
/// The caller takes ownership of the memory.
static const Metadata *buildStorage() {
  // For each class of object, initialize its metadata
  // Only run this once per class, not once per Runtime instantiation.
  Metadata *storage = new Metadata[getNumCellKinds()];
  size_t i = 0;
#define CELL_KIND(name) \
  storage[i++] = buildMetadata(CellKind::name##Kind, name##BuildMeta);
#include "hermes/VM/CellKinds.def"
#undef CELL_KIND
  assert(i == getNumCellKinds() && "Incorrect number of metadata populated");
  return storage;
}

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

MetadataTable getMetadataTable() {
  // We intentionally leak memory here in order to avoid any static destructors
  // running at exit time.
  static const Metadata *storage = buildStorage();
  return MetadataTable(storage, getNumCellKinds());
}

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
