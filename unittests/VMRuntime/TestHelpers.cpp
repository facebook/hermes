/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "TestHelpers.h"

#include "hermes/Support/Compiler.h"

namespace hermes {
namespace vm {

DummyRuntime::DummyRuntime(
    MetadataTableForTests metaTable,
    const GCConfig &gcConfig,
    std::shared_ptr<StorageProvider> storageProvider)
    : gc{metaTable,
         this,
         this,
         gcConfig,
         std::shared_ptr<CrashManager>(new NopCrashManager),
         storageProvider.get()} {}

std::shared_ptr<DummyRuntime> DummyRuntime::create(
    MetadataTableForTests metaTable,
    const GCConfig &gcConfig) {
  return create(metaTable, gcConfig, defaultProvider(gcConfig));
}

std::shared_ptr<DummyRuntime> DummyRuntime::create(
    MetadataTableForTests metaTable,
    const GCConfig &gcConfig,
    std::shared_ptr<StorageProvider> provider) {
  // This should mimic the structure of Runtime::create if compressed pointers
  // are active.
#ifdef HERMESVM_COMPRESSED_POINTERS
  auto storage = provider->newStorage();
  if (LLVM_UNLIKELY(!storage)) {
    hermes_fatal(
        (llvm::Twine("Could not allocate initial storage for Runtime: ") +
         convert_error_to_message(storage.getError()))
            .str());
  }
  DummyRuntime *rt =
      new (storage.get()) DummyRuntime(metaTable, gcConfig, provider);
  return std::shared_ptr<DummyRuntime>{rt, [provider](DummyRuntime *dr) {
                                         dr->~DummyRuntime();
                                         provider->deleteStorage(dr);
                                       }};
#else
  DummyRuntime *rt = new DummyRuntime(metaTable, gcConfig, provider);
  return std::shared_ptr<DummyRuntime>{
      rt, [provider](DummyRuntime *runtime) { delete runtime; }};
#endif
}

std::unique_ptr<StorageProvider> DummyRuntime::defaultProvider(
    const GCConfig &gcConfig) {
  const GC::Size gcSize{gcConfig.getMinHeapSize(), gcConfig.getMaxHeapSize()};
#ifdef HERMESVM_COMPRESSED_POINTERS
  auto result = StorageProvider::preAllocatedProvider(
      gcSize.storageFootprint(),
      gcSize.minStorageFootprint(),
      sizeof(DummyRuntime));
  if (!result) {
    hermes_fatal((llvm::Twine("Could not allocate provider: ") +
                  convert_error_to_message(result.getError()))
                     .str());
  }
  return std::move(result.get());
#else
  // Sizes aren't necessary without compressed pointers.
  (void)gcSize;
  return StorageProvider::mmapProvider();
#endif
}

void DummyRuntime::markRoots(SlotAcceptorWithNames &acceptor, bool) {
  markGCScopes(acceptor);
  for (GCCell **pp : pointerRoots)
    acceptor.acceptPtr(*pp);
  for (HermesValue *pp : valueRoots)
    acceptor.accept(*pp);

  if (markExtra)
    markExtra(&getHeap(), acceptor);
}

void DummyRuntime::markWeakRoots(SlotAcceptorWithNames &acceptor) {
  for (void **ptr : weakRoots) {
    acceptor.accept(*ptr);
  }
}

std::string DummyRuntime::convertSymbolToUTF8(SymbolID) {
  assert(false && "Should never attempt to resolve a symbol on a DummyRuntime");
  return "";
}

} // namespace vm
} // namespace hermes
