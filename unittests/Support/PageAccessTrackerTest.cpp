/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if defined(__linux__)
#include "hermes/Support/PageAccessTracker.h"

#include "hermes/Support/OSCompat.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

#ifdef HERMES_HAS_REAL_PAGE_TRACKER
TEST(PageAccessTrackerTest, Order) {
  const size_t PS = hermes::oscompat::page_size();
  const int numPages = 3;
  auto result = hermes::oscompat::vm_allocate(PS * numPages);
  ASSERT_TRUE(result);
  std::unique_ptr<volatile PageAccessTracker> tracker =
      PageAccessTracker::create(result.get(), PS * numPages);
  auto p = reinterpret_cast<volatile char *>(result.get());

  p[1 * PS];
  p[2 * PS];
  p[0 * PS];

  std::string out;
  {
    llvh::raw_string_ostream OS(out);
    tracker->printStats(OS, true);
  }
  std::string expected = "\"page_ids\":[1,2,0],";
  EXPECT_TRUE(out.find(expected) != std::string::npos);
}
#endif

} // namespace
#endif // __linux__
