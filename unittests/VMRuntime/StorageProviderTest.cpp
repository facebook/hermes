/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"

#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/LimitedStorageProvider.h"

using namespace hermes;
using namespace hermes::vm;

namespace {

/// Implementation of StorageProvider that always fails to allocate.
struct NullStorageProvider : public StorageProvider {
  static std::unique_ptr<NullStorageProvider> create();

 protected:
  llvh::ErrorOr<void *> newStorageImpl(size_t sz, const char *) override;
  void deleteStorageImpl(void *, size_t sz) override;
};

/* static */
std::unique_ptr<NullStorageProvider> NullStorageProvider::create() {
  return std::make_unique<NullStorageProvider>();
}

llvh::ErrorOr<void *> NullStorageProvider::newStorageImpl(
    size_t sz,
    const char *) {
  // Doesn't matter what code is returned here.
  return make_error_code(OOMError::TestVMLimitReached);
}

enum StorageProviderType {
  MmapProvider,
  ContiguousVAProvider,
};

struct StorageProviderParam {
  StorageProviderType providerType;
  size_t storageSize;
  size_t vaSize;
};

static std::unique_ptr<StorageProvider> GetStorageProvider(
    StorageProviderType type,
    size_t vaSize) {
  switch (type) {
    case MmapProvider:
      return StorageProvider::mmapProvider();
    case ContiguousVAProvider:
      return StorageProvider::contiguousVAProvider(vaSize);
    default:
      return nullptr;
  }
}

class StorageProviderTest
    : public ::testing::TestWithParam<StorageProviderParam> {};

void NullStorageProvider::deleteStorageImpl(void *, size_t sz) {}

/// Minimum segment storage size.
static constexpr size_t SIZE = FixedSizeHeapSegment::storageSize();

TEST_P(StorageProviderTest, StorageProviderSucceededAllocsLogCount) {
  auto &params = GetParam();
  auto provider{GetStorageProvider(params.providerType, params.vaSize)};

  ASSERT_EQ(0, provider->numSucceededAllocs());
  ASSERT_EQ(0, provider->numFailedAllocs());
  ASSERT_EQ(0, provider->numDeletedAllocs());
  ASSERT_EQ(0, provider->numLiveAllocs());

  auto result = provider->newStorage(params.storageSize, "Test");
  ASSERT_TRUE(result);
  void *s = result.get();

  EXPECT_EQ(1, provider->numSucceededAllocs());
  EXPECT_EQ(0, provider->numFailedAllocs());
  EXPECT_EQ(0, provider->numDeletedAllocs());
  EXPECT_EQ(1, provider->numLiveAllocs());

  provider->deleteStorage(s, params.storageSize);

  EXPECT_EQ(1, provider->numSucceededAllocs());
  EXPECT_EQ(0, provider->numFailedAllocs());
  EXPECT_EQ(1, provider->numDeletedAllocs());
  EXPECT_EQ(0, provider->numLiveAllocs());
}

TEST(StorageProviderTest, StorageProviderFailedAllocsLogCount) {
  auto provider{NullStorageProvider::create()};

  ASSERT_EQ(0, provider->numSucceededAllocs());
  ASSERT_EQ(0, provider->numFailedAllocs());
  ASSERT_EQ(0, provider->numDeletedAllocs());
  ASSERT_EQ(0, provider->numLiveAllocs());

  auto result = provider->newStorage(SIZE, "Test");
  ASSERT_FALSE(result);

  EXPECT_EQ(0, provider->numSucceededAllocs());
  EXPECT_EQ(1, provider->numFailedAllocs());
  EXPECT_EQ(0, provider->numDeletedAllocs());
  EXPECT_EQ(0, provider->numLiveAllocs());
}

TEST(StorageProviderTest, LimitedStorageProviderEnforce) {
  constexpr size_t LIM = 2;
  LimitedStorageProvider provider{
      StorageProvider::mmapProvider(),
      SIZE * LIM,
  };
  void *live[LIM];
  for (size_t i = 0; i < LIM; ++i) {
    auto result = provider.newStorage(SIZE, "Live");
    ASSERT_TRUE(result);
    live[i] = result.get();
  }

  EXPECT_FALSE(provider.newStorage(SIZE, "Dead"));

  // Clean-up
  for (auto s : live) {
    provider.deleteStorage(s, SIZE);
  }
}

TEST(StorageProviderTest, LimitedStorageProviderTrackDelete) {
  constexpr size_t LIM = 2;
  LimitedStorageProvider provider{
      StorageProvider::mmapProvider(),
      SIZE * LIM,
  };

  // If the storage gets deleted, we should be able to re-allocate it, even if
  // the total number of allocations exceeds the limit.
  for (size_t i = 0; i < LIM + 1; ++i) {
    auto result = provider.newStorage(SIZE, "Live");
    ASSERT_TRUE(result);
    auto *s = result.get();
    provider.deleteStorage(s, SIZE);
  }
}

TEST(StorageProviderTest, LimitedStorageProviderDeleteNull) {
  constexpr size_t LIM = 2;
  LimitedStorageProvider provider{
      StorageProvider::mmapProvider(),
      SIZE * LIM,
  };

  void *live[LIM];

  for (size_t i = 0; i < LIM; ++i) {
    auto result = provider.newStorage(SIZE, "Live");
    ASSERT_TRUE(result);
    live[i] = result.get();
  }

  // The allocations should fail because we have hit the limit, and the
  // deletions should not affect the limit, because they are of null storages.
  for (size_t i = 0; i < 2; ++i) {
    auto result = provider.newStorage(SIZE, "Live");
    EXPECT_FALSE(result);
  }

  // Clean-up
  for (auto s : live) {
    provider.deleteStorage(s, SIZE);
  }
}

TEST(StorageProviderTest, StorageProviderAllocsCount) {
  constexpr size_t LIM = 2;
  auto provider = std::unique_ptr<LimitedStorageProvider>{
      new LimitedStorageProvider{StorageProvider::mmapProvider(), SIZE * LIM}};

  constexpr size_t FAILS = 3;
  void *storages[LIM];
  for (size_t i = 0; i < LIM; ++i) {
    auto result = provider->newStorage(SIZE);
    ASSERT_TRUE(result);
    storages[i] = result.get();
  }

  EXPECT_EQ(LIM, provider->numSucceededAllocs());
  EXPECT_EQ(LIM, provider->numLiveAllocs());

  for (size_t i = 0; i < FAILS; ++i) {
    auto result = provider->newStorage(SIZE);
    ASSERT_FALSE(result);
  }

  EXPECT_EQ(FAILS, provider->numFailedAllocs());

  // Clean-up
  for (auto s : storages) {
    provider->deleteStorage(s, SIZE);
  }

  EXPECT_EQ(0, provider->numLiveAllocs());
  EXPECT_EQ(LIM, provider->numDeletedAllocs());
}

/// Testing that the ContiguousProvider allocates and deallocates as intended,
/// which is to always allocate at lowest-address free space and correctly free
/// the space when the storage is deleted.
TEST(StorageProviderTest, ContiguousProviderTest) {
  auto provider =
      GetStorageProvider(StorageProviderType::ContiguousVAProvider, SIZE * 10);

  size_t sz1 = SIZE * 5;
  auto result = provider->newStorage(sz1);
  ASSERT_TRUE(result);
  auto *s1 = *result;

  size_t sz2 = SIZE * 3;
  result = provider->newStorage(sz2);
  ASSERT_TRUE(result);
  auto *s2 = *result;

  size_t sz3 = SIZE * 3;
  result = provider->newStorage(sz3);
  ASSERT_FALSE(result);

  provider->deleteStorage(s1, sz1);

  result = provider->newStorage(sz3);
  ASSERT_TRUE(result);
  auto *s3 = *result;

  size_t sz4 = SIZE * 2;
  result = provider->newStorage(sz4);
  ASSERT_TRUE(result);
  auto *s4 = *result;

  result = provider->newStorage(sz4);
  ASSERT_TRUE(result);
  auto *s5 = *result;

  provider->deleteStorage(s2, sz2);
  provider->deleteStorage(s3, sz3);
  provider->deleteStorage(s4, sz4);
  provider->deleteStorage(s5, sz4);
}

/// StorageGuard will free storage on scope exit.
class StorageGuard final {
 public:
  StorageGuard(
      std::shared_ptr<StorageProvider> provider,
      void *storage,
      size_t sz)
      : provider_(std::move(provider)), storage_(storage), sz_(sz) {}

  ~StorageGuard() {
    provider_->deleteStorage(storage_, sz_);
  }

  void *raw() const {
    return storage_;
  }

 private:
  std::shared_ptr<StorageProvider> provider_;
  void *storage_;
  size_t sz_;
};

#ifndef NDEBUG

class SetVALimit final {
 public:
  explicit SetVALimit(size_t vaLimit) {
    oscompat::set_test_vm_allocate_limit(vaLimit);
  }

  ~SetVALimit() {
    oscompat::unset_test_vm_allocate_limit();
  }
};

static constexpr size_t KB = 1 << 10;
static constexpr size_t MB = KB * KB;

TEST(StorageProviderTest, SucceedsWithoutReducing) {
  // Should succeed without reducing the size at all.
  SetVALimit limit{16 * MB};
  auto result = vmAllocateAllowLess(8 * MB, 1 * MB, 1 * MB);
  ASSERT_TRUE(result);
  auto memAndSize = result.get();
  EXPECT_TRUE(memAndSize.first != nullptr);
  EXPECT_EQ(memAndSize.second, 8 * MB);
}

TEST(StorageProviderTest, SucceedsAfterReducing) {
  {
    // Should succeed after reducing the size to below the limit.
    SetVALimit limit{40 * MB};
    auto result = vmAllocateAllowLess(100 * MB, 25 * MB, 1 * MB);
    ASSERT_TRUE(result);
    auto memAndSize = result.get();
    EXPECT_TRUE(memAndSize.first != nullptr);
    EXPECT_GE(memAndSize.second, 25 * MB);
    EXPECT_LE(memAndSize.second, 40 * MB);
  }
  {
    // Test using the aligned storage alignment
    SetVALimit limit{50 * SIZE};
    auto result = vmAllocateAllowLess(100 * SIZE, 30 * SIZE, SIZE);
    ASSERT_TRUE(result);
    auto memAndSize = result.get();
    EXPECT_TRUE(memAndSize.first != nullptr);
    EXPECT_GE(memAndSize.second, 30 * SIZE);
    EXPECT_LE(memAndSize.second, 50 * SIZE);
  }
}

TEST(StorageProviderTest, FailsDueToLimitLowerThanMin) {
  // Should not succeed, limit below min amount.
  SetVALimit limit{5 * MB};
  auto result = vmAllocateAllowLess(100 * MB, 10 * MB, 1 * MB);
  ASSERT_FALSE(result);
}

TEST_P(StorageProviderTest, VirtualMemoryFreed) {
  SetVALimit limit{25 * MB};

  auto &params = GetParam();
  for (size_t i = 0; i < 20; i++) {
    std::shared_ptr<StorageProvider> sp =
        GetStorageProvider(params.providerType, params.vaSize);
    StorageGuard sg{
        sp, *sp->newStorage(params.storageSize), params.storageSize};
  }
}

#endif

INSTANTIATE_TEST_CASE_P(
    StorageProviderTests,
    StorageProviderTest,
    ::testing::Values(
        StorageProviderParam{
            MmapProvider,
            SIZE,
            0,
        },
        StorageProviderParam{
            ContiguousVAProvider,
            SIZE,
            SIZE,
        },
        StorageProviderParam{ContiguousVAProvider, SIZE * 5, SIZE * 5}));

} // namespace
