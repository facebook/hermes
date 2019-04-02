/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifdef HERMESVM_MEMORY_PROFILER
#include "TestHelpers.h"
#include "gtest/gtest.h"

#include "hermes/VM/CellKind.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SymbolID.h"

#include "llvm/Support/raw_ostream.h"

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
};

bool AllocEvent::operator==(const AllocEvent &that) const {
  return this->kind_ == that.kind_ && this->size_ == that.size_;
}

/// Implementation of MemoryEventTracker interface for MemoryEvents
/// Used to unittest MemoryEventTracker
class DummyMemoryEventTracker : public MemoryEventTracker {
 public:
  const std::vector<AllocEvent> &getAllocEvents() const {
    return allocEvents_;
  }

  void emitAlloc(uint32_t kind, uint32_t size) override {
    allocEvents_.emplace_back(AllocEvent(kind, size));
  }

 private:
  std::vector<AllocEvent> allocEvents_;
};
} // namespace
#endif
