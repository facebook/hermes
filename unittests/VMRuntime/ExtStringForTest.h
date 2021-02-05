/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_VMRUNTIME_EXTSTRINGFORTEST_H
#define HERMES_UNITTESTS_VMRUNTIME_EXTSTRINGFORTEST_H

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GCCell.h"

namespace hermes {
namespace vm {

class DummyRuntime;

/// This is a testing version of ExternalStringPrimitive<char>.
/// The GC will test whether these cells have this type, so this must have the
/// same cell kind, and the MetadataTable must cover that kind.
class ExtStringForTest final : public VariableSizeRuntimeCell {
 public:
  static const VTable vt;

  unsigned length;

  ExtStringForTest(GC *gc, unsigned const length)
      : VariableSizeRuntimeCell(gc, &vt, sizeof(ExtStringForTest)),
        length(length) {}

  ~ExtStringForTest() = default;

  // For some testing purposes, we want to be able to give back the memory
  // before finalization.
  void releaseMem(GC *gc);

  static bool classof(const GCCell *cell) {
    return cell->getVT() == &vt;
  }

  static ExtStringForTest *create(DummyRuntime &runtime, unsigned length);

  static ExtStringForTest *createLongLived(
      DummyRuntime &runtime,
      unsigned length);

 private:
  static void _finalizeImpl(GCCell *cell, GC *);
  static gcheapsize_t _externalMemorySizeImpl(const GCCell *cell);
};

void ExtStringForTestBuildMeta(const GCCell *cell, Metadata::Builder &mb);

} // namespace vm
} // namespace hermes

#endif // HERMES_UNITTESTS_VMRUNTIME_EXTSTRINGFORTEST_H
