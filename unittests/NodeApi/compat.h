// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#ifndef SRC_PUBLIC_COMPAT_H_
#define SRC_PUBLIC_COMPAT_H_

// This file contains some useful datatypes recently introduced in C++17 and
// C++20. They must be removed after we switch the toolset to the newer C++
// language version.

#include <string>
#ifdef __cpp_lib_span
#include <span>
#endif

namespace node_api_tests {

#ifdef __cpp_lib_span
using std::span;
#else
/**
 * @brief A span of values that can be used to pass arguments to function.
 *
 * For C++20 we should consider to replace it with std::span.
 */
template <typename T>
struct span {
  constexpr span(std::initializer_list<T> il) noexcept
      : data_{const_cast<T*>(il.begin())}, size_{il.size()} {}
  constexpr span(T* data, size_t size) noexcept : data_{data}, size_{size} {}

  [[nodiscard]] constexpr T* data() const noexcept { return data_; }

  [[nodiscard]] constexpr size_t size() const noexcept { return size_; }

  [[nodiscard]] constexpr T* begin() const noexcept { return data_; }

  [[nodiscard]] constexpr T* end() const noexcept { return data_ + size_; }

  const T& operator[](size_t index) const noexcept { return *(data_ + index); }

 private:
  T* data_;
  size_t size_;
};
#endif  // __cpp_lib_span

}  // namespace node_api_tests

#endif  // SRC_PUBLIC_COMPAT_H_
