/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_DENSEUINT64_H
#define HERMES_ADT_DENSEUINT64_H

#include "llvh/ADT/DenseMapInfo.h"

namespace hermes {

/// An adapter for inserting 64-bit values in DenseMap without information loss.
class DenseUInt64 {
 public:
  /// Enum to differentiate between LLVM's empty/tombstone and normal values.
  enum class MapKeyType {
    empty,
    tombstone,
    valid,
  };
  DenseUInt64(uint64_t cval) : keyType_(MapKeyType::valid), rawValue_(cval) {}

  DenseUInt64(void *addr)
      : keyType_(MapKeyType::valid), rawValue_((uint64_t)addr) {}

  DenseUInt64(double v) : DenseUInt64(llvh::DoubleToBits(v)) {}

  DenseUInt64(MapKeyType keyType) : keyType_(keyType), rawValue_(0) {
    assert(keyType_ != MapKeyType::valid && "valid entries must have a value");
  }

  bool operator==(const DenseUInt64 &other) const {
    return keyType_ == other.keyType_ && rawValue_ == other.rawValue_;
  }

  void *valueAsAddr() const {
    assert(
        keyType_ == MapKeyType::valid &&
        "attempting to get the value of tombstone/empty entry");
    return (void *)rawValue_;
  }

  double valueAsDouble() const {
    return llvh::BitsToDouble(rawValue_);
  }

  uint64_t rawValue() const {
    return rawValue_;
  }

 private:
  /// The type of value we have: empty/tombstone/normal.
  MapKeyType keyType_;
  /// A raw uint64_t.
  uint64_t rawValue_;
};

} // namespace hermes

namespace llvh {
/// Traits to enable using UInt64Constant with llvh::DenseSet/Map.
template <>
struct DenseMapInfo<hermes::DenseUInt64> {
  using UInt64Constant = hermes::DenseUInt64;
  using MapKeyType = UInt64Constant::MapKeyType;

  static UInt64Constant getEmptyKey() {
    return MapKeyType::empty;
  }
  static UInt64Constant getTombstoneKey() {
    return MapKeyType::tombstone;
  }
  static unsigned getHashValue(UInt64Constant x) {
    // Representation of small floating values may have many lower bits as
    // 0's. Hash functions that generate 64bits hashes identical to the
    // input integer values (e.g., std::hash) may cause a lot of collisions,
    // since the hash values are truncated to 32bits. The LLVM hash_value
    // function has the nice property that each input bit affects each
    // output bit with close probability, so the hash value after truncating
    // is still good. On a random number set of uniform distribution (with
    // 1/16 being random doubles and the rest integers) in the range of
    // [-1000000, 1000000], with size 80M, this performs pretty well. In
    // practice, most numbers in the heap would be small integers, so use
    // this for now until we see other extreme cases.
    return llvh::hash_value(x.rawValue());
  }
  static bool isEqual(UInt64Constant a, UInt64Constant b) {
    return a == b;
  }
};
} // namespace llvh

#endif // HERMES_ADT_DENSEUINT64_H
