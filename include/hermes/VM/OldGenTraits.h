#ifndef HERMES_VM_OLDGENTRAITS_H
#define HERMES_VM_OLDGENTRAITS_H

#include "hermes/VM/OldGenSegmentIterator.h"
#include "hermes/VM/SegTraits.h"

#include "llvm/ADT/iterator_range.h"

namespace hermes {
namespace vm {

class OldGen;

template <>
struct SegTraits<OldGen> {
  using It = OldGenSegmentIterator;
  using Range = llvm::iterator_range<It>;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_OLDGENTRAITS_H
