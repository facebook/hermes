#include "hermes/VM/FillerCell.h"

#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {

const VTable FillerCell::vt{CellKind::FillerCellKind, 0};

// Empty to prevent linker errors, doesn't need to do anything.
void UninitializedBuildMeta(const GCCell *, Metadata::Builder &) {}
void FillerCellBuildMeta(const GCCell *, Metadata::Builder &) {}

} // namespace vm
} // namespace hermes
