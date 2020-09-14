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
    MetadataTableForTests metaTable,
    const GCConfig &gcConfig,
    std::shared_ptr<StorageProvider> storageProvider,
    std::shared_ptr<CrashManager> crashMgr)
    : gc{metaTable,
         this,
         this,
         gcConfig,
         crashMgr,
         std::move(storageProvider)} {}

std::shared_ptr<DummyRuntime> DummyRuntime::create(
    MetadataTableForTests metaTable,
    const GCConfig &gcConfig,
    std::shared_ptr<StorageProvider> provider,
    std::shared_ptr<CrashManager> crashMgr) {
  DummyRuntime *rt = new DummyRuntime(metaTable, gcConfig, provider, crashMgr);
  return std::shared_ptr<DummyRuntime>{rt};
}

std::shared_ptr<DummyRuntime> DummyRuntime::create(
    MetadataTableForTests metaTable,
    const GCConfig &gcConfig) {
  return create(metaTable, gcConfig, defaultProvider());
}

std::unique_ptr<StorageProvider> DummyRuntime::defaultProvider() {
  return StorageProvider::mmapProvider();
}

void DummyRuntime::markRoots(RootAcceptor &acceptor, bool) {
  // DummyRuntime doesn't care what root section it is, but it needs one for
  // snapshot tests.
  acceptor.beginRootSection(RootAcceptor::Section::Custom);
  markGCScopes(acceptor);
  for (GCCell **pp : pointerRoots)
    acceptor.acceptPtr(*pp);
  for (HermesValue *pp : valueRoots)
    acceptor.accept(*pp);
  acceptor.endRootSection();
}

void DummyRuntime::markWeakRoots(WeakRootAcceptor &acceptor) {
  for (WeakRoot<void> *ptr : weakRoots) {
    acceptor.acceptWeak(*ptr);
  }
  if (markExtraWeak)
    markExtraWeak(acceptor);
}

std::string DummyRuntime::convertSymbolToUTF8(SymbolID) {
  assert(false && "Should never attempt to resolve a symbol on a DummyRuntime");
  return "";
}

} // namespace vm
} // namespace hermes
