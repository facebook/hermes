/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"

#include "hermes/Support/Compiler.h"

namespace hermes {
namespace vm {

::testing::AssertionResult isException(
    Runtime *runtime,
    ExecutionStatus status) {
  if (status == ExecutionStatus::EXCEPTION) {
    std::string s;
    llvh::raw_string_ostream os(s);
    runtime->printException(os, runtime->makeHandle(runtime->getThrownValue()));
    os.flush();
    return ::testing::AssertionSuccess() << "An exception occurred: " << s;
  }
  return ::testing::AssertionFailure();
}

DummyRuntime::DummyRuntime(
    const GCConfig &gcConfig,
    std::shared_ptr<StorageProvider> storageProvider,
    std::shared_ptr<CrashManager> crashMgr)
    : gcStorage_{
          this,
          this,
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
  DummyRuntime *rt = new DummyRuntime(gcConfig, provider, crashMgr);
  return std::shared_ptr<DummyRuntime>{rt};
}

std::shared_ptr<DummyRuntime> DummyRuntime::create(const GCConfig &gcConfig) {
  return create(gcConfig, defaultProvider());
}

std::unique_ptr<StorageProvider> DummyRuntime::defaultProvider() {
  return StorageProvider::mmapProvider();
}

void DummyRuntime::collect() {
  getHeap().collect("test");
}

void DummyRuntime::markRoots(RootAndSlotAcceptorWithNames &acceptor, bool) {
  // DummyRuntime doesn't care what root section it is, but it needs one for
  // snapshot tests.
  acceptor.beginRootSection(RootAcceptor::Section::Custom);
  markGCScopes(acceptor);
  for (GCCell **pp : pointerRoots)
    acceptor.acceptPtr(*pp);
  for (PinnedHermesValue *pp : valueRoots)
    acceptor.accept(*pp);
  acceptor.endRootSection();
}

void DummyRuntime::markWeakRoots(WeakRootAcceptor &acceptor, bool) {
  for (WeakRoot<GCCell> *ptr : weakRoots) {
    acceptor.acceptWeak(*ptr);
  }
  if (markExtraWeak)
    markExtraWeak(acceptor);
}

// Dummy runtime doesn't need to mark anything during complete marking.
void DummyRuntime::markRootsForCompleteMarking(RootAndSlotAcceptorWithNames &) {
}

std::string DummyRuntime::convertSymbolToUTF8(SymbolID) {
  return "";
}

} // namespace vm
} // namespace hermes
