/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LocaleResolver.h"

#include "Constants.h"
#include "IntlUtils.h"
#include "Locale.h"
#include "LocaleBCP47Object.h"
#include "OptionHelpers.h"
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
        std::u16string_view, /* extension key */
        std::u16string_view, /* extension type */
        const LocaleBCP47Object &)> &isExtensionTypeSupported) {
  Option matcher = opt.at(constants::opt_name::localeMatcher);
  LocaleBCP47Object matchedLocaleObj;
  if (matcher.getString() == constants::opt_value::locale_matcher::lookup) {
    matchedLocaleObj = lookupMatcher(requestedLocales);
  } else {
    matchedLocaleObj = bestFitMatcher(requestedLocales);
  }

  std::map<std::u16string, std::u16string> extensions = getSupportedExtensions(
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
      std::u16string_view optExtType;
      if (option.isNumber()) {
        // No known relevant extension key use numbers for extension type
        // strings, so this should not happen.
        assert(
            false &&
            "unexpected number-type option for a relevant extension key");
      } else if (option.isBool()) {
        optExtType = option.getBool() ? constants::opt_value::trueStr
                                      : constants::opt_value::falseStr;
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
        resolvedOptions.insert_or_assign(ext, option);
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
    std::string_view locale,
    const std::map<std::u16string, std::u16string> &extensions) {
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
    std::string availableLocale =
        LocaleResolver::bestAvailableLocale(localeBcp47Object.getLocaleNoExt());

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
std::string LocaleResolver::bestAvailableLocale(std::u16string_view locale) {
  // 1. Let candidate be locale.
  // Attention: The locale suppose to be lower case for comparison
  std::string candidate = IntlUtils::toUTF8ASCII(locale);
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
  char result[ULOC_FULLNAME_CAPACITY + 1]; // +1 for null terminator
  UAcceptResult outResult = ULOC_ACCEPT_VALID;

  std::string localeICU =
      Locale::convertBCP47toICULocale(localeBCP47Object.getLocaleNoExt());
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
  if (U_SUCCESS(status) && outResult != ULOC_ACCEPT_FAILED &&
      localeLen <= ULOC_FULLNAME_CAPACITY) {
    return Locale::convertICUtoBCP47Locale(result);
  }

  return std::string();
}

std::map<std::u16string, std::u16string> LocaleResolver::getSupportedExtensions(
    const LocaleBCP47Object &localeBCP47Object,
    const std::unordered_set<std::u16string> &relevantExtensionKeys,
    const std::function<bool(
        std::u16string_view, /* extension key */
        std::u16string_view, /* extension type */
        const LocaleBCP47Object &)> &isExtensionTypeSupported) {
  std::map<std::u16string, std::u16string> exts;

  const std::map<std::u16string, std::u16string> &extensionsMap =
      localeBCP47Object.getExtensionMap();

  for (auto const &extension : extensionsMap) {
    std::u16string_view extType{extension.second};
    if (extType.empty()) {
      extType = constants::opt_value::trueStr;
    }
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
      constants::opt_name::localeMatcher,
      constants::opt_value::locale_matcher::validLocaleMatchers,
      constants::opt_value::locale_matcher::best_fit);
  if (LLVM_UNLIKELY(matcherRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  std::vector<std::u16string> subset;
  for (auto const &localeBcp47Object : *localeBcp47ObjectsRes) {
    std::string locale;
    if (**matcherRes == constants::opt_value::locale_matcher::lookup) {
      locale = LocaleResolver::bestAvailableLocale(
          localeBcp47Object.getLocaleNoExt());
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
