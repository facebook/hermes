#include "LogSuccessStorageProvider.h"

#include <cassert>

namespace hermes {
namespace vm {

LogSuccessStorageProvider::LogSuccessStorageProvider(
    std::unique_ptr<StorageProvider> delegate)
    : delegate_(std::move(delegate)) {}

void *LogSuccessStorageProvider::newStorage(const char *name) {
  numAllocated_++;
  return delegate_->newStorage(name);
}

void LogSuccessStorageProvider::deleteStorage(void *storage) {
  delegate_->deleteStorage(storage);
  if (!storage) {
    return;
  }

  numDeleted_++;
  assert(numAllocated_ >= numDeleted_);
}

size_t LogSuccessStorageProvider::numAllocated() const {
  return numAllocated_;
}

size_t LogSuccessStorageProvider::numDeleted() const {
  return numDeleted_;
}

size_t LogSuccessStorageProvider::numLive() const {
  return numAllocated_ - numDeleted_;
}

} // namespace vm
} // namespace hermes
