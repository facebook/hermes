/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Locale.h"
#include "hermes/Platform/Unicode/icu.h"

#include <vector>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

std::string Locale::convertBCP47toICULocale(const std::string &localeBCP47) {
  std::vector<char> localeICU(16);
  UErrorCode errForLanguageTag{U_ZERO_ERROR};
  size_t outputLength = uloc_forLanguageTag(
      localeBCP47.c_str(),
      localeICU.data(),
      localeICU.size(),
      nullptr,
      &errForLanguageTag);
  if (outputLength > localeICU.size()) {
    localeICU.resize(outputLength);
    errForLanguageTag = U_ZERO_ERROR;
    outputLength = uloc_forLanguageTag(
        localeBCP47.c_str(),
        localeICU.data(),
        localeICU.size(),
        nullptr,
        &errForLanguageTag);
  }
  if (U_FAILURE(errForLanguageTag)) {
    // ICU can kind of deal with BCP-47 Language Tags, though not all cases are
    // covered if it's not first converted to ICU locale using forLanguageTag().
    // Return original input as fallback.
    return localeBCP47;
  }
  std::string localeICUStr(localeICU.data(), outputLength);
  return localeICUStr;
}

std::string Locale::convertICUtoBCP47Locale(const std::string &localeICU) {
  std::vector<char> localeBCP47(16);
  UErrorCode errToLanguageTag{U_ZERO_ERROR};
  size_t outputLength = uloc_toLanguageTag(
      localeICU.c_str(),
      localeBCP47.data(),
      localeBCP47.size(),
      true,
      &errToLanguageTag);
  if (outputLength > localeBCP47.size()) {
    localeBCP47.resize(outputLength);
    errToLanguageTag = U_ZERO_ERROR;
    outputLength = uloc_toLanguageTag(
        localeICU.c_str(),
        localeBCP47.data(),
        localeBCP47.size(),
        true,
        &errToLanguageTag);
  }
  if (U_FAILURE(errToLanguageTag)) {
    return std::string();
  }
  std::string localeBCP47Str(localeBCP47.data(), outputLength);
  return localeBCP47Str;
}

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes
