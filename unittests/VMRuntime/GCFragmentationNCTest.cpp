/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "ExtStringForTest.h"
#include "TestHelpers.h"
#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCCell.h"

#include <deque>

using namespace hermes::vm;

namespace {

MetadataTableForTests getMetadataTable() {
  static const Metadata storage[] = {
      Metadata(), // Uninitialized
      Metadata(), // FillerCell
      Metadata(), // DynamicUTF16StringPrimitive
      Metadata(), // DynamicASCIIStringPrimitive
      Metadata(), // DynamicUniquedUTF16StringPrimitive
      Metadata(), // DynamicUniquedASCIIStringPrimitive
      Metadata(), // ExternalUTF16StringPrimitive
      Metadata(), // ExternalASCIIStringPrimitive
  };
  return MetadataTableForTests(storage);
}

TEST(GCFragmentationNCTest, Test) {
  // Ensure that the heap will be big enough for the YoungGen to accommodate its
  // full size and add some overhang on the end to make the size of OldGen's
  // last segment occupy less than a full segment, if the heap was exactly the
  // size given in the hint.
  static constexpr gcheapsize_t kHeapSizeHint =
      AlignedHeapSegment::maxSize() * GC::kYoungGenFractionDenom +
      AlignedHeapSegment::maxSize() / 2;

  static const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSizeHint);

  auto runtime = DummyRuntime::create(getMetadataTable(), kGCConfig);
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

  // Number of bytes allocatable in the old generation, assuming the heap
  // contains \c kHeapSizeHint bytes in total.
  const gcheapsize_t kOGSizeHint =
      kHeapSizeHint - gc.youngGenSize(kHeapSizeHint);

  // A cell the size of a segment's allocation region.
  using SegmentCell = EmptyCell<AlignedHeapSegment::maxSize()>;
  // A cell whose size makes it awkward to pack into an allocation region.
  using AwkwardCell = EmptyCell<AlignedHeapSegment::maxSize() / 2 + 1>;

  std::deque<GCCell *> roots;

  { // (1) Allocate enough segment cells to fill every segment in the old gen
    //     except the last two (the full one and the overhang).
    const int cells = kOGSizeHint / SegmentCell::size() - 1;
    for (int i = 0; i < cells; ++i) {
      roots.push_back(SegmentCell::create(rt));
      rt.pointerRoots.push_back(&roots.back());
    }
  }

  { // (2) Then allocate an awkward cell.
    roots.push_back(AwkwardCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }

  { // (3) Force a full collection to make sure all the allocated cells so far
    //     end up in the old generation.
    gc.collect();
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

    roots.push_back(AwkwardCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());

    // This should cause a young generation collection, evacuating the rooted
    // awkward cell above to the old generation.
    SegmentCell::create(rt);
  }
}

TEST(GCFragmentationNCTest, ExternalMemoryTest) {
  // This is another version of GCFragmentationNCTest.Test.  It illustrates a
  // problem with an early version of external memory accounting in GenGCNC.cpp:
  // the external memory charge can make use part of the last segment, thus
  // allowing fragmentation-based allocation failure.

  // Allocate a heap whose young-gen is a full segment, and whose old gen size
  // is rounded up to a multiple of the segment size.
  static constexpr size_t kHeapSize =
      AlignedHeapSegment::maxSize() * GC::kYoungGenFractionDenom;
  static const GCConfig kGCConfig = TestGCConfigFixedSize(kHeapSize);

  // Number of bytes allocatable in each generation, assuming the heap contains
  // \c kHeapSize bytes in total.
  static constexpr size_t kOGSize = kHeapSize - AlignedHeapSegment::maxSize();

  auto runtime = DummyRuntime::create(getMetadataTable(), kGCConfig);
  DummyRuntime &rt = *runtime;
  auto &gc = rt.gc;

  // A cell the size of a segment's allocation region.
  using SegmentCell = EmptyCell<AlignedHeapSegment::maxSize()>;
  // A cell whose size makes it awkward to pack into an allocation region.
  using AwkwardCell = EmptyCell<AlignedHeapSegment::maxSize() / 2 + 1>;

  std::deque<GCCell *> roots;

  { // (1) Allocate enough segment cells to fill every segment in the old gen
    //     except the last two.
    const int cells = kOGSize / SegmentCell::size() - 2;
    for (int i = 0; i < cells; ++i) {
      roots.push_back(SegmentCell::create(rt));
      rt.pointerRoots.push_back(&roots.back());
    }
  }

  { // (2) Then allocate an awkward cell.
    roots.push_back(AwkwardCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }

  // Now allocate an external string half as big as a segment.
  roots.push_back(
      ExtStringForTest::create(rt, AlignedHeapSegment::maxSize() / 2));
  rt.pointerRoots.push_back(&roots.back());

  { // (3) Force a full collection to make sure all the allocated cells so far
    //     end up in the old generation.  The external memory charge should also
    //     be transfered.
    gc.collect();
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

    roots.push_back(AwkwardCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());

    // This should cause a young generation collection, evacuating the rooted
    // awkward cell above to the old generation.

    size_t ygGCsBefore = gc.numYoungGCs();
    SegmentCell::create(rt);
    EXPECT_EQ(ygGCsBefore + 1, gc.numYoungGCs());
    EXPECT_EQ(1, gc.numFullGCs());
  }
}

} // namespace

#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL
