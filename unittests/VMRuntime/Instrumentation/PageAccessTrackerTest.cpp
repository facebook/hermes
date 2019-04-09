/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef _WINDOWS
#include "hermes/VM/instrumentation/PageAccessTracker.h"

#include "hermes/Support/OSCompat.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

TEST(PageAccessTrackerTest, Order) {
  const size_t PS = hermes::oscompat::page_size();
  const int numPages = 3;
  auto buf = hermes::oscompat::vm_allocate(PS * numPages);
  std::unique_ptr<volatile PageAccessTracker> tracker =
      PageAccessTracker::create(buf, PS * numPages);
  auto p = reinterpret_cast<volatile char *>(buf);

  p[1 * PS];
  p[2 * PS];
  p[0 * PS];

  std::string out;
  {
    llvm::raw_string_ostream OS(out);
    tracker->printStats(OS, true);
  }
  std::string expected = "\"page_ids\":[1,2,0],";
  EXPECT_TRUE(out.find(expected) != std::string::npos);
}

} // namespace
#endif // not _WINDOWS
