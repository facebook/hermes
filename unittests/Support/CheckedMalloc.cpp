/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/CheckedMalloc.h"

#include "gtest/gtest.h"

#include <climits>

using namespace hermes;

namespace {

TEST(CheckedMalloc, Death) {
  size_t size_max = std::numeric_limits<size_t>::max();
  // These products are too big to fit into a size_t.
  EXPECT_DEATH_IF_SUPPORTED({ checkedMalloc2(size_max, 2); }, "malloc");
  EXPECT_DEATH_IF_SUPPORTED({ checkedMalloc2(2, size_max - 1); }, "malloc");
  // Allocating size 0 should not crash.
  free(checkedMalloc2(0, size_max));
  free(checkedMalloc2(0, size_max));
  free(checkedCalloc(0, size_max));
  free(checkedCalloc(0, size_max));
  free(checkedMalloc(0));
}
} // namespace
