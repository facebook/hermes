#ifndef HERMES_VM_GENTRAITS_H
#define HERMES_VM_GENTRAITS_H

#include "llvm/ADT/iterator_range.h"

namespace hermes {
namespace vm {

/// This traits class holds types associated with subclasses of GCGeneration,
/// defining iterators over segments managed by that generation.
///
/// Type of iterator over segments in this generation.  We expect that it
/// iterates over instances of AlignedHeapSegment, that is to say:
///
///     std::iterator_traits<It>::value_type == AlignedHeapSegment
///
///   type It;
///
/// A representation of a sequence of segments, as a pair of begin and end
/// iterators.
///   type Range {
///     It begin();
///     It end();
///   };
template <typename Gen>
struct SegTraits {};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GENTRAITS_H
