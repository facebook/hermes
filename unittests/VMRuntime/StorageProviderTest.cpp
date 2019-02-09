/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "gtest/gtest.h"

#include "LogSuccessStorageProvider.h"
#include "hermes/VM/AlignedStorage.h"
#include "hermes/VM/LimitedStorageProvider.h"
#include "hermes/VM/LogFailStorageProvider.h"

#include "llvm/ADT/STLExtras.h"

using namespace hermes;
using namespace hermes::vm;

namespace {

/// Implementation of StorageProvider that always fails to allocate.
struct NullStorageProvider : public StorageProvider {
  static std::unique_ptr<NullStorageProvider> create();

  void *newStorage(const char *) override;
  void deleteStorage(void *) override;
};

/* static */
std::unique_ptr<NullStorageProvider> NullStorageProvider::create() {
  return llvm::make_unique<NullStorageProvider>();
}

void *NullStorageProvider::newStorage(const char *) {
  return nullptr;
}

void NullStorageProvider::deleteStorage(void *) {}

TEST(StorageProviderTest, LogSuccessStorageProviderSuccess) {
  LogSuccessStorageProvider provider{StorageProvider::mmapProvider()};

  ASSERT_EQ(0, provider.numAllocated());
  ASSERT_EQ(0, provider.numDeleted());
  ASSERT_EQ(0, provider.numLive());

  auto *s = provider.newStorage("Test");
  ASSERT_NE(nullptr, s);

  EXPECT_EQ(1, provider.numAllocated());
  EXPECT_EQ(0, provider.numDeleted());
  EXPECT_EQ(1, provider.numLive());

  provider.deleteStorage(s);

  EXPECT_EQ(1, provider.numAllocated());
  EXPECT_EQ(1, provider.numDeleted());
  EXPECT_EQ(0, provider.numLive());
}

TEST(StorageProviderTest, LogSuccessStorageProviderFail) {
  LogSuccessStorageProvider provider{NullStorageProvider::create()};

  ASSERT_EQ(0, provider.numAllocated());
  ASSERT_EQ(0, provider.numDeleted());
  ASSERT_EQ(0, provider.numLive());

  auto *s = provider.newStorage("Test");
  ASSERT_EQ(nullptr, s);

  EXPECT_EQ(0, provider.numAllocated());
  EXPECT_EQ(0, provider.numDeleted());
  EXPECT_EQ(0, provider.numLive());

  provider.deleteStorage(s);

  EXPECT_EQ(0, provider.numAllocated());
  EXPECT_EQ(0, provider.numDeleted());
  EXPECT_EQ(0, provider.numLive());
}

TEST(StorageProviderTest, LimitedStorageProviderEnforce) {
  constexpr size_t LIM = 2;
  LimitedStorageProvider provider{
      StorageProvider::mmapProvider(),
      AlignedStorage::size() * LIM,
  };
  void *live[LIM];
  for (size_t i = 0; i < LIM; ++i) {
    live[i] = provider.newStorage("Live");
    ASSERT_NE(nullptr, live[i]);
  }

  EXPECT_EQ(nullptr, provider.newStorage("Dead"));

  // Clean-up
  for (auto s : live) {
    provider.deleteStorage(s);
  }
}

TEST(StorageProviderTest, LimitedStorageProviderTrackDelete) {
  constexpr size_t LIM = 2;
  LimitedStorageProvider provider{
      StorageProvider::mmapProvider(),
      AlignedStorage::size() * LIM,
  };

  // If the storage gets deleted, we should be able to re-allocate it, even if
  // the total number of allocations exceeds the limit.
  for (size_t i = 0; i < LIM + 1; ++i) {
    auto *s = provider.newStorage("Live");
    EXPECT_NE(nullptr, s);
    provider.deleteStorage(s);
  }
}

TEST(StorageProviderTest, LimitedStorageProviderDeleteNull) {
  constexpr size_t LIM = 2;
  LimitedStorageProvider provider{
      StorageProvider::mmapProvider(),
      AlignedStorage::size() * LIM,
  };

  void *live[LIM];

  for (size_t i = 0; i < LIM; ++i) {
    live[i] = provider.newStorage("Live");
    ASSERT_NE(nullptr, live[i]);
  }

  // The allocations should fail because we have hit the limit, and the
  // deletions should not affect the limit, because they are of null storages.
  for (size_t i = 0; i < 2; ++i) {
    void *s = provider.newStorage("Dead");
    EXPECT_EQ(nullptr, s);
    provider.deleteStorage(s);
  }

  // Clean-up
  for (auto s : live) {
    provider.deleteStorage(s);
  }
}

TEST(StorageProviderTest, LogFailStorageProvider) {
  constexpr size_t LIM = 2;
  LimitedStorageProvider delegate{StorageProvider::mmapProvider(),
                                  AlignedStorage::size() * LIM};

  LogFailStorageProvider provider{&delegate};

  constexpr size_t FAILS = 3;
  void *storages[LIM + FAILS];
  for (size_t i = 0; i < LIM + FAILS; ++i) {
    storages[i] = provider.newStorage("");
  }

  EXPECT_EQ(FAILS, provider.numFailedAllocs());

  // Clean-up
  for (auto s : storages) {
    provider.deleteStorage(s);
  }
}

/// StorageGuard will free storage on scope exit.
class StorageGuard final {
 public:
  StorageGuard(std::shared_ptr<StorageProvider> provider, void *storage)
      : provider_(std::move(provider)), storage_(storage) {}

  ~StorageGuard() {
    provider_->deleteStorage(storage_);
  }

  void *raw() const {
    return storage_;
  }

 private:
  std::shared_ptr<StorageProvider> provider_;
  void *storage_;
};

TEST(StorageProviderTest, WithExcess) {
  std::shared_ptr<StorageProvider> provider{
      StorageProvider::defaultProviderWithExcess(0, 100)};
  // This should succeed even though the maxAmount is 0.
  // The excess bytes requested should be rounded up to give an extra storage
  // allocation.
  StorageGuard storage{provider, provider->newStorage()};
  EXPECT_TRUE(storage.raw());
  // A request for a second storage *can* fail, but is not required to.
}

} // namespace
