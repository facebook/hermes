/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/DummyObject.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCCell.h"

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
#include "hermes/VM/GenGCHeapSegment.h"
#endif

#include <deque>

using namespace hermes::vm;

namespace {

TEST(GCFragmentationTest, TestCoalescing) {
  // Fill the heap with increasingly larger cells, in order to test
  // defragmentation code.
  static const size_t kNumSegments = 4;
  static const size_t kNumOGSegments = kNumSegments - 1;
  static const size_t kHeapSize = AlignedHeapSegment::maxSize() * kNumSegments;
  static const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSize);

  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;

  using SixteenthCell = EmptyCell<AlignedHeapSegment::maxSize() / 16>;
  using EighthCell = EmptyCell<AlignedHeapSegment::maxSize() / 8>;
  using QuarterCell = EmptyCell<AlignedHeapSegment::maxSize() / 4>;

  {
    GCScope scope(&rt);
    for (size_t i = 0; i < 16 * kNumOGSegments; i++)
      rt.makeHandle(SixteenthCell::create(rt));
  }

  // Hades needs a manually triggered full collection, since full collections
  // are started at the end of a YG GC.
#if defined(HERMESVM_GC_HADES) || defined(HERMESVM_GC_RUNTIME)
  rt.collect();
#endif

  {
    GCScope scope(&rt);
    for (size_t i = 0; i < 8 * kNumOGSegments; i++)
      rt.makeHandle(EighthCell::create(rt));
  }

#if defined(HERMESVM_GC_HADES) || defined(HERMESVM_GC_RUNTIME)
  rt.collect();
#endif

  {
    GCScope scope(&rt);
    for (size_t i = 0; i < 4 * kNumOGSegments; i++)
      rt.makeHandle(QuarterCell::create(rt));
  }
}

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
TEST(GCFragmentationTest, Test) {
  // Ensure that the heap will be big enough for the YoungGen to accommodate its
  // full size and add some overhang on the end to make the size of OldGen's
  // last segment occupy less than a full segment, if the heap was exactly the
  // size given in the hint.
  static constexpr gcheapsize_t kHeapSizeHint =
      GenGCHeapSegment::maxSize() * GenGC::kYoungGenFractionDenom +
      GenGCHeapSegment::maxSize() / 2;

  static const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSizeHint);

  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();
  GCScope scope{&rt};

  // Number of bytes allocatable in the old generation, assuming the heap
  // contains \c kHeapSizeHint bytes in total.
  const gcheapsize_t kOGSizeHint =
      kHeapSizeHint - gc.youngGenSize(kHeapSizeHint);

  // A cell the size of a segment's allocation region.
  using SegmentCell = EmptyCell<GenGCHeapSegment::maxSize()>;
  // A cell whose size makes it awkward to pack into an allocation region.
  using AwkwardCell = EmptyCell<GenGCHeapSegment::maxSize() / 2 + 1>;

  { // (1) Allocate enough segment cells to fill every segment in the old gen
    //     except the last two (the full one and the overhang).
    const int cells = kOGSizeHint / SegmentCell::size() - 1;
    for (int i = 0; i < cells; ++i) {
      rt.makeHandle(SegmentCell::create(rt));
    }
  }

  { // (2) Then allocate an awkward cell.
    rt.makeHandle(AwkwardCell::create(rt));
  }

  { // (3) Force a full collection to make sure all the allocated cells so far
    //     end up in the old generation.
    rt.collect();
  }

  // The end of the old generation now looks like this:
  //
  //     ----+-------+---|---+
  //     ... |XXXX|  |   |   |
  //     ----+-------+---|---+
  //              ^      ^
  //              L      E
  //
  //  - `L` Is the level, and is just over the midpoint of the penultimate
  //    segment.
  //  - `E` Is the end of the generation, and it is in the center of the last
  //    segment.
  //

  { // (4) Without increasing `E`, it is impossible to fit another awkward cell
    //     into the old generation, so let's see what happens when we try.

    rt.makeHandle(AwkwardCell::create(rt));

    // This should cause a young generation collection, evacuating the rooted
    // awkward cell above to the old generation.
    SegmentCell::create(rt);
  }
}

TEST(GCFragmentationTest, ExternalMemoryTest) {
  // This is another version of GCFragmentationTest.Test.  It illustrates a
  // problem with an early version of external memory accounting in GenGCNC.cpp:
  // the external memory charge can make use part of the last segment, thus
  // allowing fragmentation-based allocation failure.

  // Allocate a heap whose young-gen is a full segment, and whose old gen size
  // is rounded up to a multiple of the segment size.
  static constexpr size_t kHeapSize =
      GenGCHeapSegment::maxSize() * GenGC::kYoungGenFractionDenom;
  static const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSize);

  // Number of bytes allocatable in each generation, assuming the heap contains
  // \c kHeapSize bytes in total.
  static constexpr size_t kOGSize = kHeapSize - GenGCHeapSegment::maxSize();

  auto runtime = DummyRuntime::create(kGCConfig);
  DummyRuntime &rt = *runtime;
  GenGC &gc = rt.getHeap();

  // A cell the size of a segment's allocation region.
  using SegmentCell = EmptyCell<GenGCHeapSegment::maxSize()>;
  // A cell whose size makes it awkward to pack into an allocation region.
  using AwkwardCell = EmptyCell<GenGCHeapSegment::maxSize() / 2 + 1>;

  GCScope scope{&rt};

  { // (1) Allocate enough segment cells to fill every segment in the old gen
    //     except the last two.
    const int cells = kOGSize / SegmentCell::size() - 2;
    for (int i = 0; i < cells; ++i) {
      rt.makeHandle(SegmentCell::create(rt));
    }
  }

  { // (2) Then allocate an awkward cell.
    rt.makeHandle(AwkwardCell::create(rt));
  }

  // Now allocate external memory half as big as a segment.
  auto extObj = rt.makeHandle(testhelpers::DummyObject::create(&gc));
  extObj->acquireExtMem(&gc, GenGCHeapSegment::maxSize() / 2);

  { // (3) Force a full collection to make sure all the allocated cells so far
    //     end up in the old generation.  The external memory charge should also
    //     be transfered.
    rt.collect();
    EXPECT_EQ(1, gc.numFullGCs());
  }

  // The end of the old generation now looks like this:
  //
  //     ----+-------+---|---+
  //     ... |XXXX|  |   |   |
  //     ----+-------+---|---+
  //              ^      ^
  //              L      E
  //
  //  - `L` Is the level, and is just over the midpoint of the penultimate
  //    segment.
  //  - `E` Is the effective end of the generation, and it is in the center of
  //    the last segment.
  //

  { // (4) Without increasing `E`, it is impossible to fit another awkward cell
    //     into the old generation, so let's see what happens when we try.

    rt.makeHandle(AwkwardCell::create(rt));

    // This should cause a young generation collection, evacuating the rooted
    // awkward cell above to the old generation.

    size_t ygGCsBefore = gc.numYoungGCs();
    SegmentCell::create(rt);
    EXPECT_EQ(ygGCsBefore + 1, gc.numYoungGCs());
    EXPECT_EQ(1, gc.numFullGCs());
  }
}
#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL
} // namespace
