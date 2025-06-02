/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_INTLUTILS_H
#define HERMES_PLATFORMINTL_IMPLICU_INTLUTILS_H

#include <string>
#include <string_view>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

/**
 * Converts UTF-16 ASCII-only string to UTF-8 string.
 * @param utf16Source UTF-16 ASCII-only string to convert
 * @return UTF-8 string converted from the given UTF-16 ASCII-only string
 */
std::string toUTF8ASCII(std::u16string_view utf16Source);

/**
 * Converts UTF-8 ASCII-only string to UTF-16 string.
 * @param utf8Source UTF-8 ASCII-only string to convert
 * @return UTF-16 string converted from the given UTF-8 ASCII-only string
 */
std::u16string toUTF16ASCII(std::string_view utf8Source);

/**
 * Converts UTF-8 string to bool.
 * @param str UTF-8 string to convert
 * @return a bool value.
 */
bool convertToBool(const std::string &str);

/**
 * Converts UTF-16 string to bool.
 * @param str UTF-16 string to convert
 * @return a bool value.
 */
bool convertToBool(std::u16string_view str);

/**
 * Converts ASCII 'A' - 'Z' characters in given string to lowercase.
 * The function only lowercase 'A'-'Z' to 'a'-'z'.
 * @param str string to convert
 * @return a lowercased string.
 */
std::string toLowerASCII(std::string str);

/**
 * Converts ASCII 'a' - 'z' characters in given string to uppercase.
 * The function only uppercase 'a'-'z' to 'A'-'Z'.
 * @param str string to convert
 * @return an uppercased string.
 */
std::u16string toUpperASCII(std::u16string str);

/**
 * Checks if given string starts with given prefix.
 * @param str string to check
 * @param prefix prefix to check
 * @return true if \p str starts with \p prefix, false otherwise.
 */
bool startsWith(std::u16string_view str, std::u16string_view prefix);

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_INTLUTILS_H
