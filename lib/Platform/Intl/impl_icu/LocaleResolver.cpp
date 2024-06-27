/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LocaleResolver.h"
#include "../Constants.h"
#include "../IntlUtils.h"
#include "../LocaleBCP47Object.h"
#include "../OptionHelpers.h"
#include "Locale.h"
#include "hermes/Platform/Unicode/icu.h"

namespace hermes {
namespace platform_intl {
namespace impl_icu {

const LocaleResolver::AvailableLocale LocaleResolver::availableLocales_ =
    LocaleResolver::getAvailableLocales();

// https://tc39.es/ecma402/#sec-resolvelocale
LocaleResolver::ResolvedResult LocaleResolver::resolveLocale(
    const std::vector<LocaleBCP47Object> &requestedLocales,
    const Options &opt,
    const std::unordered_set<std::u16string> &relevantExtensionKeys,
    const std::function<bool(
        const std::u16string &, /* extension key */
        const std::u16string &, /* extension type */
        const LocaleBCP47Object &)> &isExtensionTypeSupported) {
  Option matcher = opt.at(Constants::optName.matcher_);
  LocaleBCP47Object matchedLocaleObj;
  if (matcher.getString() == Constants::optValue.matcher.lookup_) {
    matchedLocaleObj = lookupMatcher(requestedLocales);
  } else {
    matchedLocaleObj = bestFitMatcher(requestedLocales);
  }

  std::unordered_map<std::u16string, std::u16string> extensions =
      getSupportedExtensions(
          matchedLocaleObj, relevantExtensionKeys, isExtensionTypeSupported);

  Options resolvedOptions;
  for (const auto &ext : relevantExtensionKeys) {
    auto extIter = extensions.find(ext);
    if (extIter != extensions.end()) {
      resolvedOptions.emplace(ext, extIter->second);
    }
    auto optIter = opt.find(ext);
    if (optIter != opt.end()) {
      Option option = optIter->second;
      std::u16string optExtType;
      if (option.isNumber()) {
        optExtType =
            IntlUtils::toUTF16ASCII(std::to_string(option.getNumber()));
      } else if (option.isBool()) {
        optExtType = option.getBool() ? Constants::boolStr.true_
                                      : Constants::boolStr.false_;
      } else {
        optExtType = option.getString();
      }
      // The following takes care of the behavior from
      // https://tc39.es/ecma402/#sec-resolvelocale, step 9 > i > iv:
      // iv. If SameValue(optionsValue, value) is false and keyLocaleData
      // contains optionsValue, then
      //     1. Let value be optionsValue.
      //     2. Let supportedExtensionAddition be "".
      if (isExtensionTypeSupported(ext, optExtType, matchedLocaleObj)) {
        // Extension type specified in option is supported. It takes precedence
        // over locale extension subtag if any.
        resolvedOptions.erase(ext);
        resolvedOptions.emplace(ext, option);
        if (extIter != extensions.end() && extIter->second != optExtType) {
          // The extension type is specified in both option and locale
          // extension subtag, but they don't match. Remove the locale
          // extension subtag.
          extensions.erase(extIter);
        }
      }
    }
  }

  matchedLocaleObj.updateExtensionMap(extensions);
  LocaleResolver::ResolvedResult result;
  result.resolvedOpts = resolvedOptions;
  result.localeBcp47Object = matchedLocaleObj;
  return result;
}

LocaleResolver::AvailableLocale LocaleResolver::getAvailableLocales() {
  LocaleResolver::AvailableLocale availableLocale;
  const int32_t countAvailable = uloc_countAvailable();
  for (int32_t i = 0; i < countAvailable; ++i) {
    const char *locale = uloc_getAvailable(i);
    availableLocale.icu.emplace_back(locale);
    std::string bcp47Locale = Locale::convertICUtoBCP47Locale(locale);
    availableLocale.bcp47Lowercase.emplace(
        IntlUtils::toLowerASCII(bcp47Locale));
  }
  return availableLocale;
}

LocaleBCP47Object LocaleResolver::getDefaultLocale() {
  return LocaleBCP47Object::forLanguageTag(
             IntlUtils::toUTF16ASCII(
                 Locale::convertICUtoBCP47Locale(uloc_getDefault())))
      .value_or(LocaleBCP47Object());
}

LocaleBCP47Object LocaleResolver::toLocaleBCP47ObjectWithExtensions(
    const std::string &locale,
    const std::unordered_map<std::u16string, std::u16string> &extensions) {
  LocaleBCP47Object result =
      LocaleBCP47Object::forLanguageTag(IntlUtils::toUTF16ASCII(locale))
          .value_or(LocaleBCP47Object());
  result.updateExtensionMap(extensions);
  return result;
}

// https://tc39.es/ecma402/#sec-lookupmatcher
LocaleBCP47Object LocaleResolver::lookupMatcher(
    const std::vector<LocaleBCP47Object> &requestedLocales) {
  // 1. Let result be a new Record.
  // 2. For each element locale of requestedLocales in List order, do
  for (const auto &localeBcp47Object : requestedLocales) {
    // 2. a. Let noExtensionsLocale be the String value that is locale
    //       with all Unicode locale extension sequences removed.
    // 2. b. Let availableLocale be
    //       BestAvailableLocale(availableLocales, noExtensionsLocale).
    std::string availableLocale = LocaleResolver::bestAvailableLocale(
        IntlUtils::toUTF8ASCII(localeBcp47Object.getLocaleNoExt()));

    // 2. c. If availableLocale is not undefined, append locale to the
    //       end of subset.
    if (!availableLocale.empty()) {
      // 2. c. i. Set result.[[locale]] to availableLocale.
      // 2. c. ii. If locale and noExtensionsLocale are not the same
      // String value, then
      // 2. c. ii. 1. Let extension be the String value consisting of
      // the first substring of locale that is a Unicode locale
      // extension sequence.
      // 2. c. ii. 2. Set result.[[extension]] to extension.
      // 2. c. iii. Return result.
      return toLocaleBCP47ObjectWithExtensions(
          availableLocale, localeBcp47Object.getExtensionMap());
    }
  }
  // 3. Let defLocale be DefaultLocale();
  // 4. Set result.[[locale]] to defLocale.
  // 5. Return result.
  return getDefaultLocale();
}

// https://tc39.es/ecma402/#sec-bestavailablelocale
std::string LocaleResolver::bestAvailableLocale(const std::string &locale) {
  // 1. Let candidate be locale.
  // Attention: The locale suppose to be lower case for comparison
  std::string candidate = locale;
  // 2. Repeat,
  while (true) {
    // 2.a. If availableLocales contains an element equal to candidate, return
    //      candidate.
    if (availableLocales_.bcp47Lowercase.find(candidate) !=
        availableLocales_.bcp47Lowercase.end()) {
      return candidate;
    }

    // 2.b. Let pos be the character index of the last occurrence of "-"
    //      (U+002D) within candidate. If that character does not occur, return
    //      undefined.
    size_t pos = candidate.rfind('-');
    if (pos == std::string::npos) {
      return std::string();
    }

    // 2.c. If pos ≥ 2 and the character "-" occurs at index pos-2 of candidate,
    //      decrease pos by 2.
    if (pos >= 2 && candidate[pos - 2] == '-') {
      pos -= 2;
    }

    // 2.d. Let candidate be the substring of candidate from position 0,
    //      inclusive, to position pos, exclusive.
    candidate.resize(pos);
  }
}

// https://tc39.es/ecma402/#sec-bestfitmatcher
LocaleBCP47Object LocaleResolver::bestFitMatcher(
    const std::vector<LocaleBCP47Object> &requestedLocales) {
  for (const auto &localeBcp47Object : requestedLocales) {
    // 2. b. Let availableLocale be
    //       BestAvailableLocale(availableLocales, noExtensionsLocale).
    std::string availableLocale =
        LocaleResolver::bestFitBestAvailableLocale(localeBcp47Object);

    if (!availableLocale.empty()) {
      return toLocaleBCP47ObjectWithExtensions(
          availableLocale, localeBcp47Object.getExtensionMap());
    }
  }
  return getDefaultLocale();
}

std::string LocaleResolver::bestFitBestAvailableLocale(
    const LocaleBCP47Object &localeBCP47Object) {
  UErrorCode status = U_ZERO_ERROR;
  char result[ULOC_FULLNAME_CAPACITY];
  UAcceptResult outResult = ULOC_ACCEPT_VALID;

  std::string localeNoExt =
      IntlUtils::toUTF8ASCII(localeBCP47Object.getLocaleNoExt());
  std::string localeICU = Locale::convertBCP47toICULocale(localeNoExt);
  const char *acceptList[1]{localeICU.c_str()};

  UEnumeration *availableLocales = uenum_openCharStringsEnumeration(
      availableLocales_.icu.data(), availableLocales_.icu.size(), &status);

  int32_t localeLen = uloc_acceptLanguage(
      result,
      ULOC_FULLNAME_CAPACITY,
      &outResult,
      acceptList,
      1,
      availableLocales,
      &status);

  uenum_close(availableLocales);

  // Process if there is a match without fallback to ROOT
  assert(
      localeLen <= ULOC_FULLNAME_CAPACITY &&
      "locale string length is longer than expected");
  if (U_SUCCESS(status) && outResult != ULOC_ACCEPT_FAILED) {
    return Locale::convertICUtoBCP47Locale(std::string(result, localeLen));
  }

  return std::string();
}

std::unordered_map<std::u16string, std::u16string>
LocaleResolver::getSupportedExtensions(
    const LocaleBCP47Object &localeBCP47Object,
    const std::unordered_set<std::u16string> &relevantExtensionKeys,
    const std::function<bool(
        const std::u16string &, /* extension key */
        const std::u16string &, /* extension type */
        const LocaleBCP47Object &)> &isExtensionTypeSupported) {
  std::unordered_map<std::u16string, std::u16string> exts;

  std::unordered_map<std::u16string, std::u16string> extensionsMap =
      localeBCP47Object.getExtensionMap();

  for (auto const &extension : extensionsMap) {
    std::u16string extType =
        extension.second.empty() ? Constants::boolStr.true_ : extension.second;
    if (relevantExtensionKeys.find(extension.first) !=
            relevantExtensionKeys.end() &&
        isExtensionTypeSupported(extension.first, extType, localeBCP47Object)) {
      exts.emplace(extension.first, extType);
    }
  }
  return exts;
}

vm::CallResult<std::vector<std::u16string>> LocaleResolver::supportedLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  auto localeBcp47ObjectsRes =
      LocaleBCP47Object::canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(localeBcp47ObjectsRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  auto matcherRes = OptionHelpers::getStringOption(
      runtime,
      options,
      Constants::optName.matcher_,
      Constants::validMatchers,
      Constants::optValue.matcher.best_fit_);
  if (LLVM_UNLIKELY(matcherRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  std::vector<std::u16string> subset;
  for (auto const &localeBcp47Object : *localeBcp47ObjectsRes) {
    std::string locale;
    if (**matcherRes == Constants::optValue.matcher.lookup_) {
      locale = LocaleResolver::bestAvailableLocale(
          IntlUtils::toUTF8ASCII(localeBcp47Object.getLocaleNoExt()));
    } else {
      locale = LocaleResolver::bestFitBestAvailableLocale(localeBcp47Object);
    }
    if (!locale.empty()) {
      subset.emplace_back(localeBcp47Object.getCanonicalizedLocaleId());
    }
  }
  return subset;
}

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes
