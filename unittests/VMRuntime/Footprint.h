/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_VMRUNTIME_FOOTPRINT_H
#define HERMES_UNITTESTS_VMRUNTIME_FOOTPRINT_H

#include <cstddef>
#include <cstdint>

namespace hermes {
namespace vm {
namespace detail {

/// Return the footprint of the memory-mapping starting at \p start (inclusive)
/// and ending at \p end (exclusive). The notions of "footprint" and "mapping"
/// are platform-specific, conforming to the following specification:
///
///  - "Mapping" refers to the abstraction from the kernel for contiguous chunks
///    of virtual memory (i.e. from `mmap` or `vm_allocate`).
///  - "Footprint" refers to the metric by which the platform measures the
///    impact of a region on memory pressure. I.e. if this metric goes up, the
///    likelihood that the process is killed due to memory pressure increases.
///
/// \return the footprint as a number of pages on success, and \p SIZE_MAX on
///     failure.
size_t regionFootprint(char *start, char *end);

} // namespace detail
} // namespace vm
} // namespace hermes

#endif
