/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "TestHelpers.h"

namespace hermes {
namespace vm {

std::shared_ptr<DummyRuntime> DummyRuntime::create(
    MetadataTableForTests metaTable,
    const GCConfig &gcConfig) {
  return create(
      metaTable, gcConfig, StorageProvider::defaultProvider(gcConfig));
}

std::shared_ptr<DummyRuntime> DummyRuntime::create(
    MetadataTableForTests metaTable,
    const GCConfig &gcConfig,
    std::unique_ptr<StorageProvider> provider) {
  std::shared_ptr<StorageProvider> shareableProvider{std::move(provider)};
  DummyRuntime *rt = new DummyRuntime(metaTable, gcConfig, shareableProvider);
  return std::shared_ptr<DummyRuntime>{
      rt, [shareableProvider](DummyRuntime *runtime) { delete runtime; }};
}

} // namespace vm
} // namespace hermes
