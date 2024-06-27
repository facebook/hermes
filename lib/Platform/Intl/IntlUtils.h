/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_INTLUTILS_H
#define HERMES_PLATFORMINTL_INTLUTILS_H

#include <string>

namespace hermes {
namespace platform_intl {

class IntlUtils {
 public:
  /**
   * Converts UTF-16 ASCII-only string to UTF-8 string.
   * @param source UTF-16 ASCII-only string to convert
   * @return UTF-8 string converted from the given UTF-16 ASCII-only string
   */
  static std::string toUTF8ASCII(const std::u16string &utf16Source);

  /**
   * Converts UTF-8 ASCII-only string to UTF-16 string.
   * @param utf8Source UTF-8 ASCII-only string to convert
   * @return UTF-16 string converted from the given UTF-8 ASCII-only string
   */
  static std::u16string toUTF16ASCII(const std::string &utf8Source);

  /**
   * Converts UTF-8 string to bool.
   * @param source UTF-8 string to convert
   * @return a bool value.
   */
  static bool convertToBool(const std::string &str);

  /**
   * Converts UTF-16 string to bool.
   * @param source UTF-16 string to convert
   * @return a bool value.
   */
  static bool convertToBool(const std::u16string &str);

  /**
   * Converts all characters in given ASCII-only string to lowercase.
   * The method only lowercase 'A'-'Z' to 'a'-'z'.
   * @param s ASCII-only string to convert
   * @return a lowercased string.
   */
  static std::string toLowerASCII(std::string s);
};

} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_INTLUTILS_H
