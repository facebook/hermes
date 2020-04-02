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

#include <gtest/gtest.h>

#include <fbjni/detail/SimpleFixedString.h>

using facebook::jni::detail::SimpleFixedString;
using facebook::jni::detail::makeSimpleFixedString;

// Prevent unused function warnings portably.
#if (!defined(__has_feature) || !__has_feature(cxx_constexpr_string_builtins)) && (!defined(__GNUC__) || defined(__clang__))
constexpr int constexpr_strcmp_internal(const char* s1, const char* s2) {
  return (*s1 == '\0' || *s1 != *s2)
      ? (static_cast<int>(*s1 - *s2))
      : constexpr_strcmp_internal(s1 + 1, s2 + 1);
}
#endif

constexpr int constexpr_strcmp(const char* s1, const char* s2) {
#if defined(__has_feature) && __has_feature(cxx_constexpr_string_builtins)
  // clang provides a constexpr builtin
  return __builtin_strcmp(s1, s2);
#elif defined(__GNUC__) && !defined(__clang__)
  // strcmp() happens to already be constexpr under gcc
  return std::strcmp(s1, s2);
#else
  return constexpr_strcmp_internal(s1, s2);
#endif
}

TEST(SimpleFixedString_test, Tests) {
  static constexpr SimpleFixedString<0> empty = "";
  static_assert(empty.size() == 0, "empty not empty!");

  static constexpr auto str = makeSimpleFixedString("hello");
  static_assert(str.size() == 5, "wrong length for hello!");
  static_assert(constexpr_strcmp(str.c_str(), "hello") == 0, "bad fixedstring contents!");

  static constexpr auto substr = str.substr(1, str.size() - 2);
  static_assert(substr.size() == 3, "substr has wrong size");
  static_assert(constexpr_strcmp(substr.c_str(), "ell") == 0, "substr is broken!");

  static constexpr auto concatLeft = "Why " + str;
  static_assert(concatLeft.size() == 9, "wrong length for Why hello");
  static_assert(constexpr_strcmp(concatLeft.c_str(), "Why hello") == 0, "left concat is broken!");

  static constexpr auto concatRight = str + " there";
  static_assert(concatRight.size() == 11, "wrong length for hello there");
  static_assert(constexpr_strcmp(concatRight.c_str(), "hello there") == 0, "right concat is broken!");

  static constexpr auto bigConcat = makeSimpleFixedString("Let's ") + makeSimpleFixedString("make ") + makeSimpleFixedString("a ") + makeSimpleFixedString("long ") + makeSimpleFixedString("string!");
  static_assert(bigConcat.size() == 25, "big concat has wrong size");
  static_assert(constexpr_strcmp(bigConcat.c_str(), "Let's make a long string!") == 0, "bigConcat is broken!");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
