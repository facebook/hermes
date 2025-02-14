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
 * Converts all characters in given ASCII-only string to lowercase.
 * The method only lowercase 'A'-'Z' to 'a'-'z'.
 * @param str ASCII-only string to convert
 * @return a lowercased string.
 */
std::string toLowerASCII(std::string str);

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_INTLUTILS_H
