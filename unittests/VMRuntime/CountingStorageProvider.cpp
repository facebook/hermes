#include "CountingStorageProvider.h"

#include <cassert>

namespace hermes {
namespace vm {

CountingStorageProvider::CountingStorageProvider(
    std::unique_ptr<StorageProvider> delegate)
    : delegate_(std::move(delegate)) {}

void *CountingStorageProvider::newStorage(const char *name) {
  numAllocated_++;
  return delegate_->newStorage(name);
}

void CountingStorageProvider::deleteStorage(void *storage) {
  delegate_->deleteStorage(storage);
  if (!storage) {
    return;
  }

  numDeleted_++;
  assert(numAllocated_ >= numDeleted_);
}

size_t CountingStorageProvider::numAllocated() const {
  return numAllocated_;
}

size_t CountingStorageProvider::numDeleted() const {
  return numDeleted_;
}

size_t CountingStorageProvider::numLive() const {
  return numAllocated_ - numDeleted_;
}

} // namespace vm
} // namespace hermes
