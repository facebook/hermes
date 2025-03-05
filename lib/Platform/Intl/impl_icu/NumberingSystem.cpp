/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "NumberingSystem.h"

#include "IntlUtils.h"
#include "LocaleConverter.h"

#include <unicode/unumsys.h>

#include <algorithm>
#include <memory>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

std::u16string getDefaultNumberingSystem(const std::string &localeICU) {
  UErrorCode status = U_ZERO_ERROR;
  std::unique_ptr<UNumberingSystem, decltype(&unumsys_close)> numberingSystem(
      unumsys_open(localeICU.c_str(), &status), &unumsys_close);
  if (U_SUCCESS(status) && numberingSystem) {
    auto name = unumsys_getName(numberingSystem.get());
    if (name != nullptr) {
      return toUTF16ASCII(name);
    }
  }
  return u"latn";
}

const std::unordered_set<std::u16string> &getAvailableNumberingSystems() {
  // Intentionally leaked to avoid destruction order problems.
  static const auto *availableNumberingSystems = [] {
    auto *numberingSystems = new std::unordered_set<std::u16string>();
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UEnumeration, decltype(&uenum_close)> names(
        unumsys_openAvailableNames(&status), &uenum_close);
    if (U_FAILURE(status) || !names) {
      return numberingSystems;
    }
    // Excludes "native", "traditio", and "finance" per
    // https://tc39.es/ecma402/#sec-intl.numberformat-internal-slots
    constexpr std::u16string_view namesToExclude[] = {
        u"native", u"traditio", u"traditional", u"finance"};
    const auto *namesToExcludeBegin = std::begin(namesToExclude);
    const auto *namesToExcludeEnd = std::end(namesToExclude);
    int32_t length;
    const UChar *name;
    while ((name = uenum_unext(names.get(), &length, &status)) != nullptr &&
           U_SUCCESS(status)) {
      std::u16string_view nameStr(name, length);
      if (std::find(namesToExcludeBegin, namesToExcludeEnd, nameStr) ==
          namesToExcludeEnd) {
        numberingSystems->emplace(nameStr);
      }
    }
    return numberingSystems;
  }();
  return *availableNumberingSystems;
}

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes
