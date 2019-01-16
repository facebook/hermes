#include "hermes/VM/OldGenSegmentIterator.h"

#include "hermes/VM/GC.h"
#include "hermes/VM/OldGenNC.h"

#include <cassert>

namespace hermes {
namespace vm {

OldGenSegmentIterator::reference OldGenSegmentIterator::operator*() const {
  if (ix_ < oldGen_->filledSegments_.size()) {
    return oldGen_->filledSegments_[ix_];
  }

  while (LLVM_UNLIKELY(ix_ > oldGen_->filledSegments_.size())) {
    if (LLVM_UNLIKELY(!oldGen_->materializeNextSegment())) {
      oldGen_->gc_->oom();
    }
  }

  return oldGen_->activeSegment();
}

} // namespace vm
} // namespace hermes
