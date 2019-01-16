#include "Array.h"

namespace hermes {
namespace unittest {

/// A virtual table for an array of HermesValue.
namespace {
void dummyFinalizer(GCCell *cell, GC *) {}
} // namespace

const VTable Array::vt{CellKind::FillerCellKind, 0, dummyFinalizer};

void ArrayBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const Array *>(cell);
  mb.addArray<Metadata::ArrayData::ArrayType::HermesValue>(
      self->values(), &self->length, sizeof(GCHermesValue));
}

} // namespace unittest
} // namespace hermes
