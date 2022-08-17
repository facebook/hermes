/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"
#include "hermes/Public/GCConfig.h"
#include "hermes/Support/OSCompat.h"

#include "gtest/gtest.h"

using namespace hermes;
using namespace hermes::vm;

// At present, this test only applies to the non-contig generational collector,
// and only in dbg, where we can set the test page size.

#ifndef NDEBUG

namespace {

static constexpr size_t kK = 1024;
static constexpr size_t kM = kK * kK;

// The arguments are <heap size, page size>
struct GCInitTests
    : public ::testing::TestWithParam<std::tuple<gcheapsize_t, gcheapsize_t>> {
};

TEST_P(GCInitTests, InitSizeTest) {
  oscompat::set_test_page_size(std::get<1>(GetParam()));

  GCConfig config = GCConfig::Builder()
                        .withInitHeapSize(std::get<0>(GetParam()))
                        .withMaxHeapSize(32 * kM)
                        .build();

  auto runtime = DummyRuntime::create(config);
  oscompat::reset_test_page_size();
}

INSTANTIATE_TEST_CASE_P(
    GCInitTests,
    GCInitTests,
    ::testing::Values(
        std::make_tuple(32 * kK, 4 * kK),
        std::make_tuple(128 * kK, 4 * kK),
        std::make_tuple(512 * kK, 4 * kK),
        std::make_tuple(1 * kM, 4 * kK),
        std::make_tuple(8 * kM, 4 * kK),
        std::make_tuple(32 * kM, 4 * kK),
        std::make_tuple(32 * kK, 16 * kK),
        std::make_tuple(128 * kK, 16 * kK),
        std::make_tuple(512 * kK, 16 * kK),
        std::make_tuple(1 * kM, 16 * kK),
        std::make_tuple(8 * kM, 16 * kK),
        std::make_tuple(32 * kM, 16 * kK)));

} // namespace

#endif
