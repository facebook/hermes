/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#if defined(HERMESVM_GC_GENERATIONAL) && defined(NDEBUG)

#include "EmptyCell.h"
#include "Footprint.h"
#include "TestHelpers.h"

#include <gtest/gtest.h>
#include <hermes/Support/OSCompat.h>
#include <hermes/VM/GC.h>
#include <cstdint>

using namespace hermes::vm;
using namespace hermes::vm::detail;

namespace {

static constexpr size_t kHeapSize = 8 << 20;
static const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSize);
static const size_t kPageSize = hermes::oscompat::page_size();

const MetadataTableForTests getMetadataTable() {
  static const Metadata table[] = {Metadata()};
  return MetadataTableForTests(table);
}

TEST(GCReturnUnusedMemoryTest, InitialFootprint) {
  auto rt = DummyRuntime::create(getMetadataTable(), kGCConfig);
  auto &gc = rt->gc;

  size_t youngGenPages = (gc.youngGenSize(kHeapSize) - 1) / kPageSize + 1;
  size_t initialFootprint = regionFootprint(gc.lowLim(), gc.hiLim());

  EXPECT_LE(initialFootprint, youngGenPages);
}

TEST(GCReturnUnusedMemoryTest, CollectReturnsFreeMemory) {
  constexpr size_t FAILED = SIZE_MAX;

  auto runtime = DummyRuntime::create(getMetadataTable(), kGCConfig);
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

  using BigCell = EmptyCell<500 * 4096>;
  ASSERT_EQ(0, BigCell::size() % kPageSize);

  auto *cell1 = BigCell::create(rt);
  rt.pointerRoots.push_back(reinterpret_cast<GCCell **>(&cell1));

  auto *cell2 = BigCell::create(rt);
  rt.pointerRoots.push_back(reinterpret_cast<GCCell **>(&cell2));

  // Collect #1 moves cells to the old generation if it hasn't already.
  gc.collect();

  size_t before = regionFootprint(gc.lowLim(), gc.hiLim());
  ASSERT_NE(before, FAILED);

  // Make the pages dirty.
  auto touched1 = cell1->touch();
  cell2->touch();

  size_t touched = regionFootprint(gc.lowLim(), gc.hiLim());
  ASSERT_NE(touched, FAILED);

  // Free the first cell.
  rt.pointerRoots.erase(rt.pointerRoots.begin());

  // Collect #2 should return the unused memory back to the OS.
  gc.collect();

  size_t collected = regionFootprint(gc.lowLim(), gc.hiLim());
  ASSERT_NE(collected, FAILED);

  EXPECT_EQ(collected, touched - touched1);
}

} // anonymous namespace

#endif // HERMESVM_GC_GENERAIONAL, NDEBUG
