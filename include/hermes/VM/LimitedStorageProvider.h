#ifndef HERMES_VM_LIMITEDSTORAGEPROVIDER_H
#define HERMES_VM_LIMITEDSTORAGEPROVIDER_H

#include "hermes/VM/StorageProvider.h"

#include <cstddef>
#include <memory>

namespace hermes {
namespace vm {

/// A LimitedStorageProvider is an adapter that imposes an extra limit on
/// requests for new storage.
class LimitedStorageProvider final : public StorageProvider {
  std::unique_ptr<StorageProvider> delegate_;
  size_t limit_;

 public:
  LimitedStorageProvider(
      std::unique_ptr<StorageProvider> &&provider,
      size_t limit)
      : delegate_(std::move(provider)), limit_(limit) {}

  void *newStorage(const char *name) override;

  void deleteStorage(void *storage) override;
};

} // namespace vm
} // namespace hermes

#endif
