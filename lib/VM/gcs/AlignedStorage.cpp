/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

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

llvh::ErrorOr<AlignedStorage> AlignedStorage::create(
    StorageProvider *provider) {
  return create(provider, nullptr);
}

llvh::ErrorOr<AlignedStorage> AlignedStorage::create(
    StorageProvider *provider,
    const char *name) {
  auto result = provider->newStorage(name);
  if (!result) {
    return result.getError();
  }
  return AlignedStorage{provider, *result};
}

AlignedStorage::AlignedStorage(StorageProvider *provider, void *lowLim)
    : lowLim_(static_cast<char *>(lowLim)), provider_(provider) {
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
#ifndef HERMESVM_ALLOW_HUGE_PAGES
  oscompat::vm_unused(from, to - from);
#endif
}

} // namespace vm
} // namespace hermes
