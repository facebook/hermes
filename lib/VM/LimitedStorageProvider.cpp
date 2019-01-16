#include "hermes/VM/LimitedStorageProvider.h"

#include "hermes/VM/AlignedStorage.h"

namespace hermes {
namespace vm {

void *LimitedStorageProvider::newStorage(const char *name) {
  if (limit_ < AlignedStorage::size()) {
    return nullptr;
  }
  limit_ -= AlignedStorage::size();
  return delegate_->newStorage(name);
}

void LimitedStorageProvider::deleteStorage(void *storage) {
  if (!storage) {
    return;
  }
  delegate_->deleteStorage(storage);
  limit_ += AlignedStorage::size();
}

} // namespace vm
} // namespace hermes
