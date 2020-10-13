/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Simplified fork of folly::FixedString to avoid folly dependency.

#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>

#if defined(__has_feature)
#define FBJNI_HAS_FEATURE(...) __has_feature(__VA_ARGS__)
#else
#define FBJNI_HAS_FEATURE(...) 0
#endif

namespace facebook {
namespace jni {

namespace detail {

template <std::size_t N>
class SimpleFixedString;

constexpr size_t constexpr_strlen_internal(const char* s, size_t len) {
  // clang-format off
  return
      *(s + 0) == char(0) ? len + 0 :
      *(s + 1) == char(0) ? len + 1 :
      *(s + 2) == char(0) ? len + 2 :
      *(s + 3) == char(0) ? len + 3 :
      *(s + 4) == char(0) ? len + 4 :
      *(s + 5) == char(0) ? len + 5 :
      *(s + 6) == char(0) ? len + 6 :
      *(s + 7) == char(0) ? len + 7 :
      constexpr_strlen_internal(s + 8, len + 8);
  // clang-format on
}
static_assert(
    constexpr_strlen_internal("123456789", 0) == 9,
    "Someone appears to have broken constexpr_strlen...");

constexpr size_t constexpr_strlen(const char* s) {
#if FBJNI_HAS_FEATURE(cxx_constexpr_string_builtins)
  // clang provides a constexpr builtin
  return __builtin_strlen(s);
#elif defined(__GNUC__) && !defined(__clang__)
  // strlen() happens to already be constexpr under gcc
  return std::strlen(s);
#else
  return detail::constexpr_strlen_internal(s, 0);
#endif
}

namespace fixedstring {

// Intentionally NOT constexpr. By making this not constexpr, we make
// checkOverflow below ill-formed in a constexpr context when the condition
// it's testing for fails. In this way, precondition violations are reported
// at compile-time instead of at runtime.
[[noreturn]] inline void assertOutOfBounds() {
  assert(!"Array index out of bounds in SimpleFixedString");
  throw std::out_of_range(
      "Array index out of bounds in SimpleFixedString");
}

constexpr std::size_t checkOverflow(std::size_t i, std::size_t max) {
  return i <= max ? i : (void(assertOutOfBounds()), max);
}

constexpr std::size_t checkOverflowOrNpos(std::size_t i, std::size_t max) {
  return i == static_cast<std::size_t>(-1)
      ? max
      : (i <= max ? i : (void(assertOutOfBounds()), max));
}

// Intentionally NOT constexpr. See note above for assertOutOfBounds
[[noreturn]] inline void assertNotNullTerminated() noexcept {
  assert(!"Non-null terminated string used to initialize a SimpleFixedString");
  std::terminate(); // Fail hard, fail fast.
}

// Parsing help for human readers: the following is a constexpr noexcept
// function that accepts a reference to an array as a parameter and returns
// a reference to the same array.
template <std::size_t N>
constexpr const char (&checkNullTerminated(const char (&a)[N]) noexcept)[N] {
  // Strange decltype(a)(a) used to make MSVC happy.
  if (a[N - 1u] == '\0'
#ifndef NDEBUG
          // In Debug mode, guard against embedded nulls:
          && N - 1u == detail::constexpr_strlen_internal(a, 0u)
#endif
      ) {
    return decltype(a)(a);
  } else {
    assertNotNullTerminated();
    return decltype(a)(a);
  }
}

struct Helper {
  template <class Left, class Right, std::size_t... Is>
  static constexpr SimpleFixedString<sizeof...(Is)> concat_(
      const Left& left,
      std::size_t left_count,
      const Right& right,
      std::size_t right_count,
      std::index_sequence<Is...> is) noexcept {
    return {left, left_count, right, right_count, is};
  }

  template <std::size_t N>
  static constexpr const char (
      &data_(const SimpleFixedString<N>& that) noexcept)[N + 1u] {
    return that.data_;
  }
};

template <class Left, class Right>
constexpr int compare_(
    const Left& left,
    std::size_t left_pos,
    std::size_t left_size,
    const Right& right,
    std::size_t right_pos,
    std::size_t right_size) noexcept {
  return left_pos == left_size
      ? (right_pos == right_size ? 0 : -1)
      : (right_pos == right_size ? 1
                                 : (left[left_pos] < right[right_pos]
                                        ? -1
                                        : (left[left_pos] > right[right_pos]
                                               ? 1
                                               : fixedstring::compare_(
                                                     left,
                                                     left_pos + 1u,
                                                     left_size,
                                                     right,
                                                     right_pos + 1u,
                                                     right_size))));
}

template <class Left, class Right>
constexpr bool equal_(
    const Left& left,
    std::size_t left_size,
    const Right& right,
    std::size_t right_size) noexcept {
  return left_size == right_size &&
      0 == compare_(left, 0u, left_size, right, 0u, right_size);
}

template <class Left, class Right>
constexpr char char_at_(
    const Left& left,
    std::size_t left_count,
    const Right& right,
    std::size_t right_count,
    std::size_t i) noexcept {
  return i < left_count
      ? left[i]
      : i < (left_count + right_count) ? right[i - left_count] : '\0';
}

} // namespace fixedstring

template <std::size_t N>
class SimpleFixedString {
 private:
  template <std::size_t>
  friend class SimpleFixedString;
  friend struct detail::fixedstring::Helper;

  char data_[N + 1u]; // +1 for the null terminator
  std::size_t size_; // Nbr of chars, not incl. null teminator. size_ <= N

  using Indices = std::make_index_sequence<N>;

  template <class That, std::size_t... Is>
  constexpr SimpleFixedString(
    const That& that,
    std::size_t size,
    std::index_sequence<Is...>,
    std::size_t pos = 0,
    std::size_t count = static_cast<std::size_t>(-1)) noexcept
      : data_{(Is < (size - pos) && Is < count ? that[Is + pos] : '\0')..., '\0'},
        size_{(size - pos) < count ? (size - pos) : count} {}

  // Concatenation constructor
  template <class Left, class Right, std::size_t... Is>
  constexpr SimpleFixedString(
      const Left& left,
      std::size_t left_size,
      const Right& right,
      std::size_t right_size,
      std::index_sequence<Is...>) noexcept
      : data_{detail::fixedstring::char_at_(
                  left,
                  left_size,
                  right,
                  right_size,
                  Is)...,
              '\0'},
        size_{left_size + right_size} {}

 public:
  constexpr SimpleFixedString() : data_{}, size_{} {}

  constexpr SimpleFixedString(const SimpleFixedString&) = default;

  template <std::size_t M>
  constexpr SimpleFixedString(const SimpleFixedString<M>& that) noexcept(M <= N)
      : SimpleFixedString{that, 0u, that.size_} {}

  // Support substr.
  template <std::size_t M>
  constexpr SimpleFixedString(const SimpleFixedString<M>& that, std::size_t pos, std::size_t count) noexcept(false)
      : SimpleFixedString{
            that.data_,
            that.size_,
            std::make_index_sequence<(M < N ? M : N)>{},
            pos,
            detail::fixedstring::checkOverflow(
                detail::fixedstring::checkOverflowOrNpos(
                    count,
                    that.size_ -
                        detail::fixedstring::checkOverflow(pos, that.size_)),
                N)} {}

  // Construct from literal.
  template <std::size_t M, class = typename std::enable_if<(M - 1u <= N)>::type>
  constexpr SimpleFixedString(const char (&literal)[M]) noexcept
      : SimpleFixedString{detail::fixedstring::checkNullTerminated(literal),
                          M - 1u,
                          std::make_index_sequence<M - 1u>{}} {}

  constexpr SimpleFixedString(const char *str, std::size_t count) noexcept(false)
      : SimpleFixedString{str, detail::fixedstring::checkOverflow(count, N),
                          Indices{}} {}

  constexpr const char* c_str() const noexcept {
    return data_;
  }

  constexpr std::size_t size() const noexcept {
    return size_;
  }

  constexpr SimpleFixedString substr(std::size_t pos, std::size_t count) const {
    return {*this, pos, count};
  }

  /** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **
   * Asymmetric relational operators
   */
  friend constexpr bool operator==(
      const char* a,
      const SimpleFixedString& b) noexcept {
    return detail::fixedstring::equal_(
        a, constexpr_strlen(a), b.data_, b.size_);
  }

  /**
   * \overload
   */
  friend constexpr bool operator==(
      const SimpleFixedString& a,
      const char* b) noexcept {
    return b == a;
  }

  /** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **
   * Asymmetric concatenation
   */
  template <std::size_t M>
  friend constexpr SimpleFixedString<N + M - 1u> operator+(
      const char (&a)[M],
      const SimpleFixedString& b) noexcept {
    return detail::fixedstring::Helper::concat_(
      detail::fixedstring::checkNullTerminated(a),
      M-1u,
      b.data_,
      b.size_,
      std::make_index_sequence<N + M - 1u>{});
  }

  template <std::size_t M>
  friend constexpr SimpleFixedString<N + M - 1u> operator+(
      const SimpleFixedString& a,
      const char (&b)[M]) noexcept {
    return detail::fixedstring::Helper::concat_(
      a.data_,
      a.size_,
      detail::fixedstring::checkNullTerminated(b),
      M - 1u,
      std::make_index_sequence<N + M - 1u>{});
  }

  constexpr const char* begin() const noexcept {
    return data_;
  }

  constexpr const char* end() const noexcept {
    return data_ + size_;
  }

  operator std::string() const noexcept(false) {
    return std::string(begin(), end());
  }
};

template <std::size_t N, std::size_t M>
constexpr SimpleFixedString<N + M> operator+(
    const SimpleFixedString<N>& a,
    const SimpleFixedString<M>& b) noexcept {
  return detail::fixedstring::Helper::concat_(
      detail::fixedstring::Helper::data_(a),
      a.size(),
      detail::fixedstring::Helper::data_(b),
      b.size(),
      std::make_index_sequence<N + M>{});
}

template <std::size_t N>
constexpr SimpleFixedString<N - 1u> makeSimpleFixedString(
    const char (&a)[N]) noexcept {
  return {a};
}

} // namespace detail
} // namespace jni
} // namespace facebook
