/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_DENSEMAPINFO_H
#define HERMES_SUPPORT_DENSEMAPINFO_H

#include "llvh/ADT/DenseMapInfo.h"

namespace llvh {

/// Provide a DenseMapInfo specialization for 3-tuples. All tuple members must
/// have DenseMapInfo.
template <typename T, typename U, typename V>
struct DenseMapInfo<std::tuple<T, U, V>> {
  using Tuple = std::tuple<T, U, V>;
  using FirstInfo = DenseMapInfo<T>;
  using SecondInfo = DenseMapInfo<U>;
  using ThirdInfo = DenseMapInfo<V>;

  static inline Tuple getEmptyKey() {
    return std::make_tuple(
        FirstInfo::getEmptyKey(),
        SecondInfo::getEmptyKey(),
        ThirdInfo::getEmptyKey());
  }

  static inline Tuple getTombstoneKey() {
    return std::make_tuple(
        FirstInfo::getTombstoneKey(),
        SecondInfo::getTombstoneKey(),
        ThirdInfo::getTombstoneKey());
  }

  static unsigned getHashValue(const Tuple &tup) {
    return hash_combine(
        FirstInfo::getHashValue(std::get<0>(tup)),
        SecondInfo::getHashValue(std::get<1>(tup)),
        ThirdInfo::getHashValue(std::get<2>(tup)));
  }

  static bool isEqual(const Tuple &LHS, const Tuple &RHS) {
    return FirstInfo::isEqual(std::get<0>(LHS), std::get<0>(RHS)) &&
        SecondInfo::isEqual(std::get<1>(LHS), std::get<1>(RHS)) &&
        ThirdInfo::isEqual(std::get<2>(LHS), std::get<2>(RHS));
  }
};

} // namespace llvh

#endif
