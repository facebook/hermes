#include "hermes/VM/GCCell.h"
#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

#ifdef HERMESVM_GCCELL_ID

GCCell::GCCell(GC *gc, const VTable *vtp)
    : vtp_(vtp), _debugAllocationId_(gc->nextObjectID()) {}

#endif

} // namespace vm
} // namespace hermes
