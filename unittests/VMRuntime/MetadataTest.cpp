/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Metadata.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/GCCell.h"

#include "gtest/gtest.h"
#include "llvm/Support/TrailingObjects.h"

namespace {

using namespace hermes::vm;

struct DummyCell final : public GCCell {
  static void buildMeta(const GCCell *cell, Metadata::Builder &mb);

  std::uint32_t x;
  std::uint32_t y;
  std::uint64_t z;
};

void DummyCell::buildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const DummyCell *>(cell);
  mb.addNonPointerField("@x", &self->x);
  mb.addNonPointerField("@y", &self->y);
  mb.addNonPointerField("@z", &self->z);
}

struct DummyArrayCell final
    : public VariableSizeRuntimeCell,
      private llvm::TrailingObjects<DummyArrayCell, GCPointer<int>> {
  friend TrailingObjects;

  static void buildMeta(const GCCell *cell, Metadata::Builder &mb);

  DummyArrayCell(GC *gc, int length)
      : VariableSizeRuntimeCell(
            gc,
            nullptr,
            totalSizeToAlloc<GCPointer<int>>(length)),
        length(length) {}

  int length;
};

void DummyArrayCell::buildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const DummyArrayCell *>(cell);
  mb.addArray<Metadata::ArrayData::ArrayType::Pointer>(
      "@dummystorage",
      self->getTrailingObjects<GCPointer<int>>(),
      &self->length,
      sizeof(void *));
}

TEST(MetadataTest, TestNormalFields) {
  const auto meta =
      buildMetadata(CellKind::UninitializedKind, DummyCell::buildMeta);
  ASSERT_FALSE(meta.array_);
  auto &fields = meta.nonPointerFields_;
  ASSERT_EQ(fields.size(), 3u);
  EXPECT_EQ(fields.names[0], "@x");
  EXPECT_EQ(fields.names[1], "@y");
  EXPECT_EQ(fields.names[2], "@z");
#ifdef HERMESVM_GCCELL_ID
#ifndef NDEBUG
  size_t offsetBase = 24;
#else
  size_t offsetBase = 16;
#endif
#else
  size_t offsetBase = 8;
#endif
  EXPECT_EQ(fields.offsets[0], offsetBase);
  EXPECT_EQ(fields.offsets[1], offsetBase + sizeof(uint32_t));
  EXPECT_EQ(fields.offsets[2], offsetBase + sizeof(uint32_t) * 2);
  EXPECT_EQ(fields.sizes[0], sizeof(uint32_t));
  EXPECT_EQ(fields.sizes[1], sizeof(uint32_t));
  EXPECT_EQ(fields.sizes[2], sizeof(uint64_t));

  EXPECT_EQ(meta.pointers_.size(), 0u);
  EXPECT_EQ(meta.values_.size(), 0u);
  EXPECT_EQ(meta.symbols_.size(), 0u);
}

TEST(MetadataTest, TestArray) {
  const auto meta =
      buildMetadata(CellKind::UninitializedKind, DummyArrayCell::buildMeta);
  ASSERT_TRUE(meta.array_);
  auto &array = *(meta.array_);
  EXPECT_EQ(array.type, Metadata::ArrayData::ArrayType::Pointer);
#ifdef HERMESVM_GCCELL_ID
#ifndef NDEBUG
  size_t offsetBase = 28;
#else
  size_t offsetBase = 20;
#endif
#else
  size_t offsetBase = 12;
#endif
  EXPECT_EQ(array.lengthOffset, offsetBase);
  EXPECT_EQ(array.startOffset, offsetBase + sizeof(uint32_t));
  EXPECT_EQ(array.stride, sizeof(GCPointer<int>));
}

} // namespace
