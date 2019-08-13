/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifdef HERMESVM_MEMORY_PROFILER
#include "TestHelpers.h"
#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

/// MemoryEvent represents an event by the GC that we want to track using
/// the Memory profile (MemoryEventTracker). This allows us to construct
/// unittests and compare two lists of events.
/// MemoryEvents are printable, and can be compared.
class AllocEvent {
  uint32_t kind_;
  uint32_t size_;

 public:
  AllocEvent(uint32_t kind, uint32_t size) : kind_(kind), size_(size) {}
  bool operator==(const AllocEvent &) const;
  friend void PrintTo(const AllocEvent &ae, std::ostream *os);
};

bool AllocEvent::operator==(const AllocEvent &that) const {
  return this->kind_ == that.kind_ && this->size_ == that.size_;
}

void PrintTo(const AllocEvent &ae, std::ostream *os) {
  *os << "Alloc (" << cellKindStr(static_cast<CellKind>(ae.kind_)) << ", "
      << ae.size_ << ")";
}

/// Implementation of MemoryEventTracker interface for MemoryEvents
/// Used to unittest MemoryEventTracker
class DummyMemoryEventTracker : public MemoryEventTracker {
 public:
  const std::vector<AllocEvent> &getAllocEvents() const {
    return allocEvents_;
  }

  void emitAlloc(uint32_t kind, uint32_t size) override {
    allocEvents_.emplace_back(kind, size);
  }

 private:
  std::vector<AllocEvent> allocEvents_;
};

/// Object containing some dummy integer, used to test the MemEventTracker
/// allocations.
struct DummyObject final : public GCCell {
  static const VTable vt;

  DummyObject(GC *gc) : GCCell(gc, &vt) {}

  static DummyObject *create(DummyRuntime &runtime) {
    return new (runtime.alloc(sizeof(DummyObject)))
        DummyObject(&runtime.getHeap());
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::UninitializedKind;
  }
};

const VTable DummyObject::vt{CellKind::UninitializedKind, sizeof(DummyObject)};

static void DummyObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  const auto *self = static_cast<const DummyObject *>(cell);
}

static MetadataTableForTests getMetadataTable() {
  static const Metadata storage[] = {
      buildMetadata(CellKind::UninitializedKind, DummyObjectBuildMeta)};
  return MetadataTableForTests(storage);
}
} // namespace

namespace hermes {
namespace vm {
template <>
struct IsGCObject<DummyObject> : public std::true_type {};
} // namespace vm
} // namespace hermes

namespace {
/// Abstraction that allows us to store the MemoryEventTracker so that
/// we can compare its events with some correct sequence of events in
/// unittests.
class MemEventTrackerTest : public DummyRuntimeTestFixtureBase {
 protected:
  std::shared_ptr<DummyMemoryEventTracker> memEventTracker_;
  MemEventTrackerTest()
      : MemEventTrackerTest(std::make_shared<DummyMemoryEventTracker>()) {}
  MemEventTrackerTest(std::shared_ptr<DummyMemoryEventTracker> memEventTracker)
      : DummyRuntimeTestFixtureBase(
            getMetadataTable(),
            GCConfig::Builder()
                .withInitHeapSize(kInitHeapSize)
                .withMaxHeapSize(kMaxHeapSize)
                .withMemEventTracker(memEventTracker)
                .build()),
        memEventTracker_(std::move(memEventTracker)) {}
};

TEST_F(MemEventTrackerTest, AllocTest) {
  // Allocate some object, no need to keep it alive, we're only checking for
  // allocation events.
  DummyObject::create(*runtime);

  // Vector containing correct allocation events
  std::vector<AllocEvent> events{AllocEvent(
      static_cast<uint32_t>(CellKind::UninitializedKind), sizeof(DummyObject))};
  EXPECT_EQ(events, memEventTracker_->getAllocEvents());
}
} // namespace
#endif
