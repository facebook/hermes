/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifdef HERMESVM_GC_GENERATIONAL

#include "gtest/gtest.h"

#include "Footprint.h"

#include <hermes/Support/OSCompat.h>
#include <hermes/VM/DependentMemoryRegion.h>
#include <hermes/VM/HeapAlign.h>

using namespace hermes::vm;
using namespace hermes::vm::detail;

namespace {

TEST(DependentMemoryRegionTest, ParentUnused) {
  // TODO(T40416012) Re-enable this test when vm_unused is fixed.
  // Skip this test in Windows because vm_unused has a no-op implementation.
#ifndef _WINDOWS
  const size_t FAIL = SIZE_MAX;
  const size_t PS = hermes::oscompat::page_size();

  const size_t kLogRatio = 2;
  const size_t kParentSize = 17 * PS;
  const size_t kDMRPages = 5;

  DependentMemoryRegion dmr(
      "test-dmr",
      reinterpret_cast<char *>(0),
      reinterpret_cast<char *>(0),
      reinterpret_cast<char *>(kParentSize),
      reinterpret_cast<char *>(kParentSize),
      kLogRatio);

  size_t init = regionFootprint(dmr.lowLim(), dmr.hiLim());
  ASSERT_NE(FAIL, init);

  // Touch the pages so we have something to mark as unused.
  for (volatile char *p = dmr.lowLim(); p < dmr.hiLim(); p += PS)
    *p = 1;

  {
    size_t touched = regionFootprint(dmr.lowLim(), dmr.hiLim());
    ASSERT_NE(FAIL, touched);
    ASSERT_EQ(kDMRPages, touched - init);
  }

  /**
   * # Test Setup
   *
   *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *     |  /  /  /  |  /  /  /  |  /  /  /  |  /  /  /  |  /        |
   *     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
   *     ^           ^           ^                 ^     ^  ^
   *    (5)         (4)         (3)               (2)   (1)(0)
   *
   * In the above diagram, each parent page is delimited by /, each dependent
   * page is delimited by |.  As the log-ratio is 2, each dependent page has 4
   * parent pages in its pre-image. We have organised for there to be one
   * overhanging parent page, causing us to allocate a mostly empty final
   * dependent page.
   *
   * There are also numbered labels pointing to various parts of the
   * diagram. The test proceeds by marking the region between point `N` and
   * point `N+1` as unused in the parent region, and verifying that the correct
   * pages in the dependent region are marked as unused.
   */

  struct Label {
    char *ptr;
    size_t unused;
  };

  std::vector<Label> labels{
      Label{reinterpret_cast<char *>(kParentSize), 0},
      Label{reinterpret_cast<char *>(kParentSize - 1 * PS), 1},
      Label{reinterpret_cast<char *>(kParentSize - 3 * PS), 1},
      Label{reinterpret_cast<char *>(kParentSize - 9 * PS), 3},
      Label{reinterpret_cast<char *>(kParentSize - 13 * PS), 4},
      Label{reinterpret_cast<char *>(kParentSize - 17 * PS), 5},
  };

  for (size_t i = 1; i < labels.size(); ++i) {
    dmr.parentDidAdviseUnused(labels[i].ptr, labels[i - 1].ptr);

    size_t footprint = regionFootprint(dmr.lowLim(), dmr.hiLim());
    ASSERT_NE(FAIL, footprint);
    EXPECT_EQ(labels[i].unused, kDMRPages + init - footprint);
  }
#endif
}

} // anonymous namespace

#endif // HERMESVM_GC_GENERATIONAL
