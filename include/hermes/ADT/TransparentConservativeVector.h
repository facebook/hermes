/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "llvh/ADT/SmallVector.h"

#include <cstdint>

namespace hermes {

/// A vector that expands its capacity less aggressively.
/// The pointer is stored at offset 0, so it is accessible directly with `&`.
template <typename T>
class TransparentConservativeVector : public llvh::SmallVector<T, 0> {
  using Base = llvh::SmallVector<T, 0>;

 public:
  using Base::Base;
  using Base::capacity;
  using Base::reserve;
  using Base::size;
  using Base::operator[];
  using Base::begin;
  using Base::end;
  using Base::resize;

  /// \return the offset of the data pointer in this class.
  /// Will always return 0.
  /// NOTE: We put this in a function because we can't use `offsetof` of a
  /// protected member of the base class in a constexpr class field initializer,
  /// and we can't name TransparentConservativeVector<T> because it's
  /// incomplete within a field initializer.
  static constexpr size_t dataPointerOffset() {
    static_assert(
        offsetof(TransparentConservativeVector<T>, BeginX) == 0,
        "TransparentConservativeVector must be transparent");
    return offsetof(TransparentConservativeVector<T>, BeginX);
  }

  template <typename... Args>
  void emplace_back(Args &&...args) {
    auto cap = capacity();
    if (size() == cap) {
      reserve(cap + cap / 4);
    }
    Base::emplace_back(std::forward<Args>(args)...);
  }
};

} // namespace hermes
