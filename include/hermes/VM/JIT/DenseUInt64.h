/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_JIT_DENSEUINT64_H
#define HERMES_VM_JIT_DENSEUINT64_H

#include "hermes/VM/HermesValue.h"

#include "llvm/ADT/DenseMapInfo.h"

namespace hermes {
namespace vm {

/// A wrapper for using uint64_t with llvm::DenseMap/Set.
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

  DenseUInt64(HermesValue hv)
      : keyType_(MapKeyType::valid), rawValue_(hv.getRaw()) {}

  DenseUInt64(double v) : DenseUInt64(HermesValue::encodeDoubleValue(v)) {}

  DenseUInt64(MapKeyType keyType) : keyType_(keyType), rawValue_(0) {
    assert(keyType_ != MapKeyType::valid && "valid entries must have a value");
  }

  bool operator==(const DenseUInt64 &other) const {
    return keyType_ == other.keyType_ && rawValue_ == other.rawValue_;
  }

  HermesValue valueAsHV() const {
    assert(
        keyType_ == MapKeyType::valid &&
        "attempting to get the value of tombstone/empty entry");
    return HermesValue::fromRaw(rawValue_);
  }
  void *valueAsAddr() const {
    assert(
        keyType_ == MapKeyType::valid &&
        "attempting to get the value of tombstone/empty entry");
    return (void *)rawValue_;
  }

  uint64_t rawValue() const {
    return rawValue_;
  }

 private:
  /// The type of value we have: empty/tombstone/normal.
  MapKeyType keyType_;
  /// A raw uint64_t: it could be a HermesValue or an address
  uint64_t rawValue_;
};

} // namespace vm
} // namespace hermes

namespace llvm {
/// Traits to enable using UInt64Constant with llvm::DenseSet/Map.
template <>
struct DenseMapInfo<hermes::vm::DenseUInt64> {
  using UInt64Constant = hermes::vm::DenseUInt64;
  using MapKeyType = UInt64Constant::MapKeyType;

  static inline UInt64Constant getEmptyKey() {
    return MapKeyType::empty;
  }
  static inline UInt64Constant getTombstoneKey() {
    return MapKeyType::tombstone;
  }
  static inline unsigned getHashValue(UInt64Constant x) {
    return DenseMapInfo<uint64_t>::getHashValue(x.rawValue());
  }
  static inline bool isEqual(UInt64Constant a, UInt64Constant b) {
    return a == b;
  }
};
} // namespace llvm

#endif // HERMES_VM_JIT_DENSEUINT64_H
