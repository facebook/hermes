/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_BUILDMETADATA_H
#define HERMES_VM_BUILDMETADATA_H

#include "hermes/Support/Compiler.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/Metadata.h"

#include "llvh/ADT/ArrayRef.h"

namespace hermes {
namespace vm {

using BuildMetadataCallback = void(const GCCell *, Metadata::Builder &);

/// Forward declare all \c BuildMeta functions.
/// These must be defined in other files.
#define CELL_KIND(name) \
  void name##BuildMeta(const GCCell *, Metadata::Builder &);
#include "hermes/VM/CellKinds.def"
#undef CELL_KIND

/// A MetadataTable is an association between CellKind and Metadata.
///
/// This is a reference to some static storage that holds the mapping.
/// It should be passed by value since it is a trivially copyable reference,
/// not the actual data.
using MetadataTable = llvh::ArrayRef<Metadata>;

static_assert(
    std::is_trivially_copyable<MetadataTable>::value,
    "MetadataTable should be trivially copyable to keep it cheap to copy");

Metadata buildMetadata(CellKind kind, BuildMetadataCallback *builder);

void buildMetadataTable();

} // namespace vm
} // namespace hermes

#endif
