/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PUBLIC_GCCONFIG_H
#define HERMES_PUBLIC_GCCONFIG_H

#include "hermes/Public/CtorConfig.h"
#include "hermes/Public/GCTripwireContext.h"
#include "hermes/Public/MemoryEventTracker.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>

namespace hermes {
namespace vm {

/// A type big enough to accomodate the entire allocated address space.
/// Individual allocations are always 'uint32_t', but on a 64-bit machine we
/// might want to accommodate a larger total heap (or not, in which case we keep
/// it 32-bit).
using gcheapsize_t = uint32_t;

/// Parameters to control a tripwire function called when the live set size
/// surpasses a given threshold after collections.  Check documentation in
/// README.md
#define GC_TRIPWIRE_FIELDS(F)                                                  \
  /* If the heap size is above this threshold after a collection, the tripwire \
   * is triggered. */                                                          \
  F(constexpr, gcheapsize_t, Limit, std::numeric_limits<gcheapsize_t>::max())  \
                                                                               \
  /* The callback to call when the tripwire is considered triggered. */        \
  F(HERMES_NON_CONSTEXPR,                                                      \
    std::function<void(GCTripwireContext &)>,                                  \
    Callback,                                                                  \
    nullptr)                                                                   \
  /* GC_TRIPWIRE_FIELDS END */

_HERMES_CTORCONFIG_STRUCT(GCTripwireConfig, GC_TRIPWIRE_FIELDS, {});

#undef HEAP_TRIPWIRE_FIELDS

#define GC_HANDLESAN_FIELDS(F)                                        \
  /* The probability with which the GC should keep moving the heap */ \
  /* to detect stale GC handles. */                                   \
  F(constexpr, double, SanitizeRate, 0.0)                             \
  /* Random seed to use for basis of decisions whether or not to */   \
  /* sanitize. A negative value will mean a seed will be chosen at */ \
  /* random. */                                                       \
  F(constexpr, int64_t, RandomSeed, -1)                               \
  /* GC_HANDLESAN_FIELDS END */

_HERMES_CTORCONFIG_STRUCT(GCSanitizeConfig, GC_HANDLESAN_FIELDS, {});

#undef GC_HANDLESAN_FIELDS

/// How aggressively to return unused memory to the OS.
enum ReleaseUnused {
  kReleaseUnusedNone = 0, /// Don't try to release unused memory.
  kReleaseUnusedOld, /// Only old gen, on full collections.
  kReleaseUnusedYoungOnFull, /// Also young gen, but only on full collections.
  kReleaseUnusedYoungAlways /// Also young gen, also on young gen collections.
};

enum class GCEventKind {
  CollectionStart,
  CollectionEnd,
};

/// Parameters for GC Initialisation.  Check documentation in README.md
/// constexpr indicates that the default value is constexpr.
#define GC_FIELDS(F)                                                      \
  /* Minimum heap size hint. */                                           \
  F(constexpr, gcheapsize_t, MinHeapSize, 0)                              \
                                                                          \
  /* Initial heap size hint. */                                           \
  F(constexpr, gcheapsize_t, InitHeapSize, 32 << 20)                      \
                                                                          \
  /* Maximum heap size hint. */                                           \
  F(constexpr, gcheapsize_t, MaxHeapSize, 512 << 20)                      \
                                                                          \
  /* Sizing heuristic: fraction of heap to be occupied by live data. */   \
  F(constexpr, double, OccupancyTarget, 0.5)                              \
                                                                          \
  /* Number of consecutive full collections considered to be an OOM. */   \
  F(constexpr,                                                            \
    unsigned,                                                             \
    EffectiveOOMThreshold,                                                \
    std::numeric_limits<unsigned>::max())                                 \
                                                                          \
  /* Sanitizer configuration for the GC. */                               \
  F(constexpr, GCSanitizeConfig, SanitizeConfig)                          \
                                                                          \
  /* Whether the GC should spread allocations across all its "spaces". */ \
  F(constexpr, bool, ShouldRandomizeAllocSpace, false)                    \
                                                                          \
  /* Whether to Keep track of GC Statistics. */                           \
  F(constexpr, bool, ShouldRecordStats, false)                            \
                                                                          \
  /* How aggressively to return unused memory to the OS. */               \
  F(constexpr, ReleaseUnused, ShouldReleaseUnused, kReleaseUnusedOld)     \
                                                                          \
  /* Name for this heap in logs. */                                       \
  F(HERMES_NON_CONSTEXPR, std::string, Name, "HermesRuntime")             \
                                                                          \
  /* Configuration for the Heap Tripwire. */                              \
  F(HERMES_NON_CONSTEXPR, GCTripwireConfig, TripwireConfig)               \
                                                                          \
  /* Whether to (initially) allocate from the young gen (true) or the */  \
  /* old gen (false). */                                                  \
  F(constexpr, bool, AllocInYoung, true)                                  \
                                                                          \
  /* Whether to revert, if necessary, to young-gen allocation at TTI. */  \
  F(constexpr, bool, RevertToYGAtTTI, false)                              \
                                                                          \
  /* Whether to use mprotect on GC metadata between GCs. */               \
  F(constexpr, bool, ProtectMetadata, false)                              \
                                                                          \
  /* Whether to enable "proper" (spec-compliant) WeakMap marking.         \
     (There have been bugs, and perf issues, so we want to be able        \
     to revert to the previous, non-spec-compliant, behavior.) */         \
  F(constexpr, bool, ProperWeakMapMarking, false)                         \
                                                                          \
  /* Pointer to the memory profiler (Memory Event Tracker). */            \
  F(HERMES_NON_CONSTEXPR,                                                 \
    std::shared_ptr<MemoryEventTracker>,                                  \
    MemEventTracker,                                                      \
    nullptr)                                                              \
                                                                          \
  /* Called at GC events (see GCEventKind enum for the list). The */      \
  /* second argument contains human-readable details about the event. */  \
  /* NOTE: The function MUST NOT invoke any methods on the Runtime. */    \
  F(HERMES_NON_CONSTEXPR,                                                 \
    std::function<void(GCEventKind, const char *)>,                       \
    Callback,                                                             \
    nullptr)                                                              \
  /* GC_FIELDS END */

_HERMES_CTORCONFIG_STRUCT(GCConfig, GC_FIELDS, {
  if (builder.hasMinHeapSize()) {
    if (builder.hasInitHeapSize()) {
      // If both are specified, normalize the initial size up to the minimum,
      // if necessary.
      InitHeapSize_ = std::max(MinHeapSize_, InitHeapSize_);
    } else {
      // If the minimum is set explicitly, but the initial heap size is not,
      // use the minimum as the initial size.
      InitHeapSize_ = MinHeapSize_;
    }
  }
  assert(InitHeapSize_ >= MinHeapSize_);

  // Make sure the max is at least the Init.
  MaxHeapSize_ = std::max(InitHeapSize_, MaxHeapSize_);
});

#undef GC_FIELDS

} // namespace vm
} // namespace hermes

#endif // HERMES_PUBLIC_GCCONFIG_H
