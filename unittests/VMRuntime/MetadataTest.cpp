/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Metadata.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/SymbolID.h"

#include "gtest/gtest.h"

namespace {

using namespace hermes::vm;

struct DummyCell final {
 public:
  static void buildMeta(const GCCell *cell, Metadata::Builder &mb);
  static const VTable vt;
  GCPointerBase x_;
  GCPointerBase y_;
  GCPointerBase z_;
  GCSymbolID sym_;
};

const VTable DummyCell::vt{CellKind::DummyObjectKind, sizeof(DummyCell)};

static_assert(
    std::is_standard_layout<DummyCell>::value,
    "DummyCell isn't a standard layout, offsetof won't work");

void DummyCell::buildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = reinterpret_cast<const DummyCell *>(cell);
  mb.setVTable(&DummyCell::vt);
  mb.addField("x", &self->x_);
  mb.addField("y", &self->y_);
  mb.addField("z", &self->z_);
  mb.addField("sym", &self->sym_);
}

struct DummyArrayCell {
 public:
  static const VTable vt;
  AtomicIfConcurrentGC<std::uint32_t> length_{3};
  GCPointerBase data_[3];

  static void buildMeta(const GCCell *cell, Metadata::Builder &mb);
};

const VTable DummyArrayCell::vt{
    CellKind::DummyObjectKind,
    sizeof(DummyArrayCell)};

static_assert(
    std::is_standard_layout<DummyArrayCell>::value,
    "DummyArrayCell isn't a standard layout, offsetof won't work");

void DummyArrayCell::buildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = reinterpret_cast<const DummyArrayCell *>(cell);
  mb.setVTable(&DummyArrayCell::vt);
  mb.addArray(
      "dummystorage", self->data_, &self->length_, sizeof(DummyArrayCell));
}

TEST(MetadataTest, TestNormalFields) {
  const auto meta =
      buildMetadata(CellKind::UninitializedKind, DummyCell::buildMeta);
  ASSERT_FALSE(meta.array_);

  auto &fields = meta.pointers_;
  ASSERT_EQ(fields.size(), 3u);
  EXPECT_STREQ(fields.names[0], "x");
  EXPECT_STREQ(fields.names[1], "y");
  EXPECT_STREQ(fields.names[2], "z");
  EXPECT_EQ(fields.offsets[0], offsetof(DummyCell, x_));
  EXPECT_EQ(fields.offsets[1], offsetof(DummyCell, y_));
  EXPECT_EQ(fields.offsets[2], offsetof(DummyCell, z_));

  EXPECT_EQ(meta.pointers_.size(), 3u);
  EXPECT_EQ(meta.values_.size(), 0u);

  EXPECT_EQ(meta.symbols_.size(), 1u);
  EXPECT_STREQ(meta.symbols_.names[0], "sym");
  EXPECT_EQ(meta.symbols_.offsets[0], offsetof(DummyCell, sym_));
}

TEST(MetadataTest, TestArray) {
  const auto meta =
      buildMetadata(CellKind::UninitializedKind, DummyArrayCell::buildMeta);
  ASSERT_TRUE(meta.array_);
  auto &array = *(meta.array_);
  EXPECT_EQ(array.type, Metadata::ArrayData::ArrayType::Pointer);

  EXPECT_EQ(array.lengthOffset, offsetof(DummyArrayCell, length_));
  EXPECT_EQ(array.startOffset, offsetof(DummyArrayCell, data_));
  EXPECT_EQ(array.stride, sizeof(DummyCell));
}

} // namespace
