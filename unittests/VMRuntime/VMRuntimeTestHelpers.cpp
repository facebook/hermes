/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "VMRuntimeTestHelpers.h"

#include "hermes/Support/Compiler.h"
#include "hermes/VM/HeapRuntime.h"

namespace hermes {
namespace vm {

::testing::AssertionResult isException(
    Runtime &runtime,
    ExecutionStatus status) {
  if (status == ExecutionStatus::EXCEPTION) {
    std::string s;
    llvh::raw_string_ostream os(s);
    runtime.printException(os, runtime.makeHandle(runtime.getThrownValue()));
    os.flush();
    return ::testing::AssertionSuccess() << "An exception occurred: " << s;
  }
  return ::testing::AssertionFailure();
}

DummyRuntime::DummyRuntime(
    const GCConfig &gcConfig,
    std::shared_ptr<StorageProvider> storageProvider,
    std::shared_ptr<CrashManager> crashMgr)
    : gcCallbacksWrapper_(*this),
      heap_{
          gcCallbacksWrapper_,
          *this,
          gcConfig,
          crashMgr,
          std::move(storageProvider),
          /* experiments */ 0} {}

DummyRuntime::~DummyRuntime() {
  EXPECT_FALSE(getHeap().getIDTracker().hasNativeIDs())
      << "A pointer is left in the ID tracker that is from non-JS memory. "
         "Was untrackNative called?";
  getHeap().finalizeAll();
}

std::shared_ptr<DummyRuntime> DummyRuntime::create(
    const GCConfig &gcConfig,
    std::shared_ptr<StorageProvider> provider,
    std::shared_ptr<CrashManager> crashMgr) {
  auto rt = HeapRuntime<DummyRuntime>::create(provider);
  new (rt.get()) DummyRuntime(gcConfig, provider, crashMgr);
  return rt;
}

std::shared_ptr<DummyRuntime> DummyRuntime::create(const GCConfig &gcConfig) {
#ifdef HERMESVM_CONTIGUOUS_HEAP
  // Allow some extra segments for the runtime, and as a buffer for the GC.
  // This is consistent to VM::Runtime.
  uint64_t providerSize = std::min<uint64_t>(
      1ULL << 32,
      (uint64_t)gcConfig.getMaxHeapSize() +
          FixedSizeHeapSegment::storageSize() * 4);
  return create(gcConfig, defaultProvider(providerSize));
#else
  return create(gcConfig, defaultProvider());
#endif
}

std::unique_ptr<StorageProvider> DummyRuntime::defaultProvider(
    uint64_t providerSize) {
#ifdef HERMESVM_CONTIGUOUS_HEAP
  return StorageProvider::contiguousVAProvider(providerSize);
#else
  (void)providerSize;
  return StorageProvider::mmapProvider();
#endif
}

void DummyRuntime::collect() {
  getHeap().collect("test");
}

void DummyRuntime::markRoots(RootAcceptorWithNames &acceptor, bool) {
  // DummyRuntime doesn't care what root section it is, but it needs one for
  // snapshot tests.
  acceptor.beginRootSection(RootAcceptor::Section::Custom);
  markGCScopes(acceptor);
  acceptor.endRootSection();
}

void DummyRuntime::markWeakRoots(WeakRootAcceptor &acceptor, bool) {
  for (WeakRoot<GCCell> *ptr : weakRoots) {
    acceptor.acceptWeak(*ptr);
  }
}

// Dummy runtime doesn't need to mark anything during complete marking.
void DummyRuntime::markRootsForCompleteMarking(RootAcceptorWithNames &) {}

std::string DummyRuntime::convertSymbolToUTF8(SymbolID) {
  return "";
}

} // namespace vm
} // namespace hermes
