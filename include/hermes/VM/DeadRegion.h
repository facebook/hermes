/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_DEADREGION_H
#define HERMES_VM_DEADREGION_H

namespace hermes {
namespace vm {

// TODO: should this become a CellKind?

/// Used to indicate the start region of consecutive dead objects, of
/// the given total size.
struct DeadRegion final {
#ifndef NDEBUG
  static constexpr gcheapsize_t kMagic{0xdeadbeef};
  const gcheapsize_t magic_{kMagic};
#endif
  const gcheapsize_t size_;

  DeadRegion(gcheapsize_t size) : size_(size) {
    assert(size_ && "Cannot construct a zero sized DeadRegion");
  }

  gcheapsize_t size() const {
    assert(isValid() && "Invalid DeadRegion");
    return size_;
  }

#ifndef NDEBUG
  bool isValid() const {
    return magic_ == kMagic;
  }
#endif
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_DEADREGION_H
