#ifndef HERMES_VM_FILLERCELL_H
#define HERMES_VM_FILLERCELL_H

#include "hermes/VM/Runtime.h"
#include "hermes/VM/VTable.h"

namespace hermes {
namespace vm {

/// This class exists for cases where the GC wants to fill some heap region
/// with a non-object, just to allow a contiguous heap to "parse" correctly.
class FillerCell final : public VariableSizeRuntimeCell {
  static const VTable vt;

 public:
  using size_type = uint32_t;

  static const VTable *vtable() {
    return &vt;
  };

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::FillerCellKind;
  }

  FillerCell(GC *gc, size_type size) : VariableSizeRuntimeCell(gc, &vt, size) {}
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_FILLERCELL_H
