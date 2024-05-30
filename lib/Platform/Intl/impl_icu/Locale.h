/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_LOCALE_H
#define HERMES_PLATFORMINTL_IMPLICU_LOCALE_H

#include <string>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

class Locale {
 public:
  /**
   * Converts BCP47 locale to ICU locale.
   * @param localeBCP47 BCP47 locale to convert
   * @return a ICU locale
   */
  static std::string convertBCP47toICULocale(const std::string &localeBCP47);

  /**
   * Converts ICU locale to BCP47 locale.
   * @param localeBCP47 ICU locale to convert
   * @return a BCP47 locale
   */
  static std::string convertICUtoBCP47Locale(const std::string &localeICU);
};

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_LOCALE_H
