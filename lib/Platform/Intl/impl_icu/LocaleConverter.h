/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_LOCALECONVERTER_H
#define HERMES_PLATFORMINTL_IMPLICU_LOCALECONVERTER_H

#include <string>
#include <string_view>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

/**
 * Converts BCP47 locale to ICU locale.
 * @param localeBCP47 BCP47 locale to convert
 * @return a ICU locale
 */
std::string convertBCP47toICULocale(std::u16string_view localeBCP47);

/**
 * Converts ICU locale to BCP47 locale.
 * @param localeBCP47 ICU locale to convert
 * @return a BCP47 locale
 */
std::string convertICUtoBCP47Locale(const char *localeICU);

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_LOCALECONVERTER_H
