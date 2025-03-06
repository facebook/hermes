/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LocaleConverter.h"

#include "IntlUtils.h"
#include "hermes/Platform/Unicode/icu.h"

#include <vector>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

std::string convertBCP47toICULocale(std::u16string_view localeBCP47) {
  std::string localeBCP47Str = toUTF8ASCII(localeBCP47);
  UErrorCode status = U_ZERO_ERROR;
  std::string localeICU(16, char());
  int32_t bufferSize = localeICU.size();
  int32_t resultLength = 0;
  while ((resultLength = uloc_forLanguageTag(
              localeBCP47Str.c_str(),
              localeICU.data(),
              localeICU.size(),
              nullptr,
              &status)) > bufferSize) {
    status = U_ZERO_ERROR;
    localeICU.resize(resultLength);
    bufferSize = localeICU.size();
  }
  localeICU.resize(resultLength);
  if (U_FAILURE(status)) {
    // ICU can kind of deal with BCP-47 Language Tags, though not all cases are
    // covered if it's not first converted to ICU locale using forLanguageTag().
    // Return original input as fallback.
    return localeBCP47Str;
  }
  return localeICU;
}

std::string convertICUtoBCP47Locale(const char *localeICU) {
  UErrorCode status = U_ZERO_ERROR;
  std::string localeBCP47(16, char());
  int32_t bufferSize = localeBCP47.size();
  int32_t resultLength = 0;
  while (
      (resultLength = uloc_toLanguageTag(
           localeICU, localeBCP47.data(), localeBCP47.size(), true, &status)) >
      bufferSize) {
    status = U_ZERO_ERROR;
    localeBCP47.resize(resultLength);
    bufferSize = localeBCP47.size();
  }
  localeBCP47.resize(resultLength);
  if (U_FAILURE(status)) {
    return "und";
  }
  return localeBCP47;
}

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes
