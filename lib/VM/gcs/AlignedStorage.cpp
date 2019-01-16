#include "hermes/VM/AlignedStorage.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/StorageProvider.h"

#include <cassert>
#include <utility>

namespace hermes {
namespace vm {

void swap(AlignedStorage &a, AlignedStorage &b) {
  using std::swap;

  swap(a.lowLim_, b.lowLim_);
  swap(a.provider_, b.provider_);
}

AlignedStorage::AlignedStorage(StorageProvider *provider)
    : AlignedStorage(provider, nullptr) {}

AlignedStorage::AlignedStorage(StorageProvider *provider, const char *name)
    : lowLim_(reinterpret_cast<char *>(provider->newStorage(name))),
      provider_(provider) {
  assert(
      AlignedStorage::start(lowLim_) == lowLim_ &&
      "The lower limit of this storage must be aligned");
}

AlignedStorage::AlignedStorage(AlignedStorage &&that) : AlignedStorage() {
  swap(*this, that);
  assert(
      AlignedStorage::start(lowLim_) == lowLim_ &&
      "The lower limit of this storage must be aligned");
}

AlignedStorage &AlignedStorage::operator=(AlignedStorage that) {
  swap(*this, that);
  assert(
      AlignedStorage::start(lowLim_) == lowLim_ &&
      "The lower limit of this storage must be aligned");
  return *this;
}

AlignedStorage::~AlignedStorage() {
  if (provider_) {
    provider_->deleteStorage(lowLim_);
  }
}

void AlignedStorage::markUnused(char *from, char *to) {
  assert(from <= to && "Unused region boundaries inverted");
  assert(lowLim() <= from && to <= hiLim() && "Unused region out-of-bounds");
  oscompat::vm_unused(from, to - from);
}

} // namespace vm
} // namespace hermes
