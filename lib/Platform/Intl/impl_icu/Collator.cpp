/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Collator.h"

#include "Constants.h"
#include "IntlUtils.h"
#include "LocaleBCP47Object.h"
#include "LocaleConverter.h"
#include "LocaleResolver.h"
#include "OptionHelpers.h"
#include "hermes/Platform/Intl/BCP47Parser.h"

#include <algorithm>
#include <optional>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

constexpr char16_t collationTypeDefault[] = u"default";
constexpr char16_t collationTypeSearch[] = u"search";
constexpr char16_t collationTypeStandard[] = u"standard";

vm::ExecutionStatus Collator::initialize(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  if (LLVM_UNLIKELY(
          initializeCollator(runtime, locales, options) ==
          vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  };

  std::string localeICU = convertBCP47toICULocale(resolvedInternalLocale_);

  UErrorCode err{U_ZERO_ERROR};
  coll_ = ucol_open(localeICU.c_str(), &err);

  if (U_FAILURE(err)) {
    // Failover to root locale if we're unable to open in resolved locale.
    err = U_ZERO_ERROR;
    coll_ = ucol_open("", &err);
  }
  assert(U_SUCCESS(err) && "failed to open collator");

  // Spec requires normalization to be always on.
  // ICU Collator default nomalization mode is locale dependent,
  // with most locale default to off.
  // Set collator normalization mode to on.
  ucol_setAttribute(coll_, UCOL_NORMALIZATION_MODE, UCOL_ON, &err);

  setAttributes();

  return vm::ExecutionStatus::RETURNED;
}

Collator::~Collator() {
  close();
}

void Collator::close() {
  if (coll_ != nullptr) {
    ucol_close(coll_);
  }
}

/**
 * For Intl options, `sensitivity`, `ignorePunctuation`, `numeric`, `caseFirst`,
 * there are corresponding attributes for ICU collator. Setting those attributes
 * here.
 * https://unicode-org.github.io/icu-docs/apidoc/dev/icu4c/ucol_8h.html#a583fbe7fc4a850e2fcc692e766d2826c
 * For the other options `localeMatcher`, `usage`, `collation`,
 * they are used to resolve locale.
 */
void Collator::setAttributes() {
  // - numeric - UCOL_NUMERIC_COLLATION
  // Whether numeric collation should be used, such that "1" < "2" < "10".
  // Possible values are true and false; the default is false.
  // Note: This option can also be set through the 'kn' Unicode extension key;
  // if both are provided, this options property takes precedence.
  UErrorCode errNumeric{U_ZERO_ERROR};
  if (resolvedNumericValue_ == true) {
    ucol_setAttribute(coll_, UCOL_NUMERIC_COLLATION, UCOL_ON, &errNumeric);
  } else {
    ucol_setAttribute(coll_, UCOL_NUMERIC_COLLATION, UCOL_OFF, &errNumeric);
  }
  assert(
      U_SUCCESS(errNumeric) &&
      "failed to set collator numeric-collation attribute");

  // - caseFirst - UCOL_CASE_FIRST
  // Whether upper case or lower case should sort first. Possible values are
  // "upper", "lower", or "false" (use the locale's default). This option can be
  // set through an options property or through a Unicode extension key; if both
  // are provided, the options property takes precedence.
  // Note: This option can also be set through the 'kf' Unicode extension key;
  // if both are provided, this options property takes precedence.
  UErrorCode errCaseFirst{U_ZERO_ERROR};
  if (!resolvedCaseFirstValue_.empty()) {
    if (resolvedCaseFirstValue_ == constants::opt_value::case_first::upper) {
      ucol_setAttribute(
          coll_, UCOL_CASE_FIRST, UCOL_UPPER_FIRST, &errCaseFirst);
    } else if (
        resolvedCaseFirstValue_ == constants::opt_value::case_first::lower) {
      ucol_setAttribute(
          coll_, UCOL_CASE_FIRST, UCOL_LOWER_FIRST, &errCaseFirst);
    } else {
      ucol_setAttribute(coll_, UCOL_CASE_FIRST, UCOL_OFF, &errCaseFirst);
    }
    assert(
        U_SUCCESS(errCaseFirst) &&
        "failed to set collator case-first attribute");
  } else {
    auto caseFirstAttribute =
        ucol_getAttribute(coll_, UCOL_CASE_FIRST, &errCaseFirst);
    assert(
        U_SUCCESS(errCaseFirst) &&
        "failed to get collator attribute : UCOL_CASE_FIRST");
    if (caseFirstAttribute == UCOL_UPPER_FIRST) {
      resolvedCaseFirstValue_ = constants::opt_value::case_first::upper;
    } else if (caseFirstAttribute == UCOL_LOWER_FIRST) {
      resolvedCaseFirstValue_ = constants::opt_value::case_first::lower;
    } else {
      resolvedCaseFirstValue_ = constants::opt_value::falseStr;
    }
  }

  // sensitivity - UCOL_STRENGTH
  // Which differences in the strings should lead to non-zero result values.
  // Possible values are:

  // - "base": Only strings that differ in base letters compare as unequal.
  // Examples: a ≠ b, a = á, a = A.
  // - "accent": Only strings that differ in base
  // letters or accents and other diacritic marks compare as unequal.
  // Examples: a ≠ b, a ≠ á, a = A.
  // - "case": Only strings that differ in base letters or case compare as
  // unequal.
  // Examples: a ≠ b, a = á, a ≠ A.
  // - "variant": Strings that differ in base letters, accents and other
  // diacritic marks, or case compare as unequal. Other differences may also be
  // taken into consideration.
  // Examples: a ≠ b, a ≠ á, a ≠ A.
  // - The default is "variant" for usage "sort"; it's locale dependent for
  // usage "search".

  // Can be either UCOL_PRIMARY, UCOL_SECONDARY, UCOL_TERTIARY, UCOL_QUATERNARY
  // or UCOL_IDENTICAL. The usual strength for most locales (except Japanese) is
  // tertiary.
  UErrorCode errSensitivity{U_ZERO_ERROR};
  if (!resolvedSensitivityValue_.empty()) {
    if (resolvedSensitivityValue_ == constants::opt_value::sensitivity::base) {
      ucol_setAttribute(coll_, UCOL_STRENGTH, UCOL_PRIMARY, &errSensitivity);
    } else if (
        resolvedSensitivityValue_ ==
        constants::opt_value::sensitivity::accent) {
      ucol_setAttribute(coll_, UCOL_STRENGTH, UCOL_SECONDARY, &errSensitivity);
    } else if (
        resolvedSensitivityValue_ ==
        constants::opt_value::sensitivity::caseStr) {
      ucol_setAttribute(coll_, UCOL_STRENGTH, UCOL_PRIMARY, &errSensitivity);
      ucol_setAttribute(coll_, UCOL_CASE_LEVEL, UCOL_ON, &errSensitivity);
    } else if (
        resolvedSensitivityValue_ ==
        constants::opt_value::sensitivity::variant) {
      ucol_setAttribute(coll_, UCOL_STRENGTH, UCOL_TERTIARY, &errSensitivity);
    }
    assert(
        U_SUCCESS(errSensitivity) &&
        "failed to set collator strength attribute");
  } else {
    // update resolvedSensitivityValue_ with ICU collator sensitivity
    auto strengthAttribute =
        ucol_getAttribute(coll_, UCOL_STRENGTH, &errSensitivity);
    assert(
        U_SUCCESS(errSensitivity) &&
        "failed to get collator attribute : UCOL_STRENGTH");
    if (strengthAttribute == UCOL_PRIMARY) {
      UErrorCode errCaseLevel{U_ZERO_ERROR};
      auto caseLevelAttribute =
          ucol_getAttribute(coll_, UCOL_CASE_LEVEL, &errCaseLevel);
      assert(
          U_SUCCESS(errCaseLevel) &&
          "failed to get collator attribute: UCOL_CASE_LEVEL");
      if (caseLevelAttribute == UCOL_ON) {
        resolvedSensitivityValue_ = constants::opt_value::sensitivity::caseStr;
      } else {
        resolvedSensitivityValue_ = constants::opt_value::sensitivity::base;
      }
    } else if (strengthAttribute == UCOL_SECONDARY) {
      resolvedSensitivityValue_ = constants::opt_value::sensitivity::accent;
    } else {
      resolvedSensitivityValue_ = constants::opt_value::sensitivity::variant;
    }
  }

  // ignorePunctuation - UCOL_ALTERNATE_HANDLING
  // Whether punctuation should be ignored. Possible values are true and false;
  // the default is false.
  // https://unicode-org.github.io/icu/userguide/collation/customization/ignorepunct.html
  // Setting alternate-handling to shifted when strength is
  // primary/secondary/tertiary, all punctuation are ignored.
  UErrorCode errIgnorePunctuation{U_ZERO_ERROR};
  if (resolvedIgnorePunctuationValue_ == true) {
    ucol_setAttribute(
        coll_, UCOL_ALTERNATE_HANDLING, UCOL_SHIFTED, &errIgnorePunctuation);
  } else {
    ucol_setAttribute(
        coll_,
        UCOL_ALTERNATE_HANDLING,
        UCOL_NON_IGNORABLE,
        &errIgnorePunctuation);
  }
  assert(
      U_SUCCESS(errIgnorePunctuation) &&
      "failed to set collator alternate-handling attribute");
}

vm::ExecutionStatus Collator::initializeCollator(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  auto requestedLocalesRes =
      LocaleBCP47Object::canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocalesRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }

  // 2. Set options to ? CoerceOptionsToObject(options).
  // 3. Let usage be ? GetOption(options, "usage", "string", « "sort", "search"
  // », "sort").
  // - usage
  // Whether the comparison is for sorting or for searching for matching
  // strings. Possible values are "sort" and "search"; the default is "sort".
  // 4. Set collator.[[Usage]] to usage.
  auto usageRes = getStringOption(
      runtime,
      options,
      constants::opt_name::usage,
      constants::opt_value::usage::validUsages,
      constants::opt_value::usage::sort);
  if (LLVM_UNLIKELY(usageRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  resolvedUsageValue_ = **usageRes;

  // 5. If usage is "sort", then
  //    a. Let localeData be %Collator%.[[SortLocaleData]].
  // 6. Else,
  //    a. Let localeData be %Collator%.[[SearchLocaleData]].
  //
  // For "search", the effect of this will be achieved by specifying
  // "co-search" Unicode extension key and type in the internal resolved
  // locale used for opening ICU4C collator. See relevant code after
  // resolveLocale(). "sort" is default for ICU4C collator, so don't
  // need to specify extension key and type for "sort".
  //
  // Note that the only way to set collation type with ICU4C collator is
  // through the 'co' extension key in the locale parameter when opening
  // ICU4C collator.

  // 7. Let opt be a new Record.
  // 'opt' will contain the options that can also be specified in a BCP-47
  // locale string with extensions, i.e. collation ("co" extension), numeric
  // ("kn" extension), and case-first ("kf" extension) along with the
  // locale-matcher option. 'opt' and the input locale list will then be input
  // to locale resolution that will output the resolved values of those options
  // and the resolved locale.
  Options opt;
  // 8. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  // "lookup", "best fit" », "best fit").
  // - localeMatcher
  // The locale matching algorithm to use. Possible values are "lookup" and
  // "best fit"; the default is "best fit". For information about this option,
  // see the Intl page.
  auto matcherRes = getStringOption(
      runtime,
      options,
      constants::opt_name::localeMatcher,
      constants::opt_value::locale_matcher::validLocaleMatchers,
      constants::opt_value::locale_matcher::best_fit);
  if (LLVM_UNLIKELY(matcherRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  // 9. Set opt.[[localeMatcher]] to matcher.
  opt.emplace(constants::opt_name::localeMatcher, **matcherRes);

  // 10. Let collation be ? GetOption(options, "collation", "string", empty,
  // undefined).
  auto collationValueRes = getStringOption(
      runtime, options, constants::opt_name::collation, {}, std::nullopt);
  if (LLVM_UNLIKELY(collationValueRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  // 11. If collation is not undefined, then
  // a. If collation does not match the Unicode Locale Identifier type
  // nonterminal, throw a RangeError exception.
  if (collationValueRes->has_value()) {
    if (!isUnicodeExtensionType(**collationValueRes)) {
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid collation: ") +
          vm::TwineChar16((*collationValueRes)->c_str()));
    }
    // 12. Set opt.[[co]] to collation.
    opt.emplace(constants::extension_key::co, **collationValueRes);
  }

  // 13. Let numeric be ? GetOption(options, "numeric", "boolean", empty,
  // undefined).
  auto numericValue =
      getBoolOption(options, constants::opt_name::numeric, std::nullopt);
  // 14. If numeric is not undefined, then
  // a. Let numeric be ! ToString(numeric).
  // Note: We omit the ToString(numeric) operation as it's not observable.
  // GetBoolOption returns a Boolean and ToString(Boolean) does not
  // have side effects.
  // 15. Set opt.[[kn]] to numeric.
  if (numericValue.has_value()) {
    opt.emplace(constants::extension_key::kn, *numericValue);
  }

  // 16. Let caseFirst be ? GetOption(options, "caseFirst", "string", « "upper",
  // "lower", "false" », undefined).
  auto caseFirstValueRes = getStringOption(
      runtime,
      options,
      constants::opt_name::caseFirst,
      constants::opt_value::case_first::validCaseFirsts,
      std::nullopt);
  if (LLVM_UNLIKELY(caseFirstValueRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  // 17. Set opt.[[kf]] to caseFirst.
  if (caseFirstValueRes->has_value()) {
    opt.emplace(constants::extension_key::kf, **caseFirstValueRes);
  }

  // The relevant unicode extensions accepted by Collator as specified here:
  // https://tc39.github.io/ecma402/#sec-intl-collator-internal-slots
  // 18. Let relevantExtensionKeys be %Collator%.[[RelevantExtensionKeys]].
  static constexpr std::u16string_view relevantExtensionKeys[] = {
      constants::extension_key::co,
      constants::extension_key::kn,
      constants::extension_key::kf};
  // 19. Let r be ResolveLocale(%Collator%.[[AvailableLocales]],
  // requestedLocales, opt, relevantExtensionKeys, localeData).
  // Implementation pass in a local function isExtensionTypeSupported instead
  // of localeData to check whether collation, case-first, and numeric types
  // specified through locale extension subtag and options are supported.
  // In particular, the collation type check is done by querying ICU
  // for collation types that are supported for the resolved locale.
  ResolvedResult result = resolveLocale(
      *requestedLocalesRes,
      opt,
      relevantExtensionKeys,
      isExtensionTypeSupported);

  // 20. Set collator.[[Locale]] to r.[[locale]].
  LocaleBCP47Object resolvedBCP47Locale = result.localeBcp47Object;
  Options resolvedOpt = result.resolvedOpts;
  resolvedLocale_ = resolvedBCP47Locale.getCanonicalizedLocaleId();

  // 21. Let collation be r.[[co]].
  // 22. If collation is null, let collation be "default".
  // 23. Set collator.[[Collation]] to collation.
  //
  // The only way to set collation type with ICU4C collator is
  // through the 'co' extension key in the locale parameter when opening
  // ICU4C collator. So if resolved options contains collation option,
  // add collation extension key and resolved type to resolvedInternalLocale_,
  // which will be used when opening ICU4C collaor.
  //
  // Special case: Usage option "search" takes precedence over collation
  // resolved option. Hence, if usage option is "search", the collation
  // extension type is set to "search" for adding to resolvedInternalLocale_.
  // The resolved collation value is still "default" though because "search"
  // is not allowed as collation resolved value per spec:
  // https://tc39.github.io/ecma402/#sec-intl-collator-internal-slots
  resolvedCollationValue_ = collationTypeDefault;
  std::map<std::u16string, std::u16string> resolvedExtMap =
      resolvedBCP47Locale.getExtensionMap();
  if (resolvedUsageValue_ == constants::opt_value::usage::search) {
    // If resolvedLocale_ has a collation unicode extension, remove it.
    auto nodeHandle = resolvedExtMap.extract(constants::extension_key::co);
    if (!nodeHandle.empty()) {
      resolvedBCP47Locale.updateExtensionMap(resolvedExtMap);
      resolvedLocale_ = resolvedBCP47Locale.getCanonicalizedLocaleId();
    }
    resolvedExtMap[constants::extension_key::co] = collationTypeSearch;
  } else {
    auto collationEntry = resolvedOpt.find(constants::extension_key::co);
    if (collationEntry != resolvedOpt.end()) {
      resolvedCollationValue_ = collationEntry->second.getString();
      resolvedExtMap[constants::extension_key::co] = resolvedCollationValue_;
    }
  }
  resolvedBCP47Locale.updateExtensionMap(resolvedExtMap);
  resolvedInternalLocale_ = resolvedBCP47Locale.getCanonicalizedLocaleId();

  // 24. If relevantExtensionKeys contains "kn", then
  // a. Set collator.[[Numeric]] to ! SameValue(r.[[kn]], "true").
  resolvedNumericValue_ = false;
  auto numericEntry = resolvedOpt.find(constants::extension_key::kn);
  if (numericEntry != resolvedOpt.end()) {
    if (numericEntry->second.isBool()) {
      resolvedNumericValue_ = numericEntry->second.getBool();
    } else {
      resolvedNumericValue_ = convertToBool(numericEntry->second.getString());
    }
  }

  // 25. If relevantExtensionKeys contains "kf", then
  // a. Set collator.[[CaseFirst]] to r.[[kf]].
  auto caseFirstEntry = resolvedOpt.find(constants::extension_key::kf);
  if (caseFirstEntry != resolvedOpt.end()) {
    resolvedCaseFirstValue_ = caseFirstEntry->second.getString();
  }

  // 26. Let sensitivity be ? GetOption(options, "sensitivity", "string", «
  // "base", "accent", "case", "variant" », undefined).
  // 27. If sensitivity is undefined, then
  // a. If usage is "sort", then
  // i. Let sensitivity be "variant".
  // b. Else,
  // i. Let dataLocale be r.[[dataLocale]].
  // ii. Let dataLocaleData be localeData.[[<dataLocale>]].
  // iii. Let sensitivity be dataLocaleData.[[sensitivity]].
  // 28. Set collator.[[Sensitivity]] to sensitivity.
  auto sensitivitiesValueRes = getStringOption(
      runtime,
      options,
      constants::opt_name::sensitivity,
      constants::opt_value::sensitivity::validSensitivities,
      std::nullopt);
  if (LLVM_UNLIKELY(sensitivitiesValueRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  if (resolvedUsageValue_ == constants::opt_value::usage::sort &&
      !(sensitivitiesValueRes->has_value())) {
    resolvedSensitivityValue_ = constants::opt_value::sensitivity::variant;
  } else if (sensitivitiesValueRes->has_value()) {
    resolvedSensitivityValue_ = **sensitivitiesValueRes;
  } // Sensitivity value not specified and usage is 'search', need to default to
    // ICU Collator's default for the locale. resolvedSensitivityValue_ remains
    // an empty string here to indicate that and then in setAttributes(), it
    // would be set based on looking up ICU Collator's default for the locale.

  // 29. Let ignorePunctuation be ? GetOption(options, "ignorePunctuation",
  // "boolean", empty, false).
  // 30. Set collator.[[IgnorePunctuation]] to ignorePunctuation.
  // 31. Return collator.
  auto ignorePunctuationValue =
      getBoolOption(options, constants::opt_name::ignorePunctuation, false);

  resolvedIgnorePunctuationValue_ = *ignorePunctuationValue;

  return vm::ExecutionStatus::RETURNED;
}

// https://tc39.es/ecma402/#sec-collator-comparestrings
double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  // The C API used for comparing two strings is ucol_strcoll. It requires two
  // UChar * strings and their lengths as parameters, as well as a pointer to a
  // valid UCollator instance. The result is a UCollationResult constant, which
  // can be one of UCOL_LESS, UCOL_EQUAL or UCOL_GREATER.
  auto result = ucol_strcoll(
      coll_,
      (const UChar *)x.data(),
      x.size(),
      (const UChar *)y.data(),
      y.size());

  switch (result) {
    case UCOL_LESS:
      return -1;
    case UCOL_EQUAL:
      return 0;
    case UCOL_GREATER:
      return 1;
  }
  llvm_unreachable("Invalid result from ucol_strcoll");
}

// https://tc39.es/ecma402/#sec-intl.collator.prototype.resolvedoptions
Options Collator::resolvedOptions() noexcept {
  Options finalResolvedOptions;
  finalResolvedOptions.emplace(
      constants::opt_name::locale, Option(resolvedLocale_));
  finalResolvedOptions.emplace(
      constants::opt_name::usage, Option(resolvedUsageValue_));
  finalResolvedOptions.emplace(
      constants::opt_name::sensitivity, Option(resolvedSensitivityValue_));
  finalResolvedOptions.emplace(
      constants::opt_name::ignorePunctuation,
      Option(resolvedIgnorePunctuationValue_));
  finalResolvedOptions.emplace(
      constants::opt_name::collation, Option(resolvedCollationValue_));
  finalResolvedOptions.emplace(
      constants::opt_name::numeric, Option(resolvedNumericValue_));
  finalResolvedOptions.emplace(
      constants::opt_name::caseFirst, Option(resolvedCaseFirstValue_));

  return finalResolvedOptions;
}

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return supportedLocales(runtime, locales, options);
}

bool Collator::isExtensionTypeSupported(
    std::u16string_view extensionKey,
    std::u16string_view extensionType,
    const LocaleBCP47Object &localeBCP47Object) {
  if (extensionKey == constants::extension_key::kf) {
    return std::find(
               std::begin(constants::opt_value::case_first::validCaseFirsts),
               std::end(constants::opt_value::case_first::validCaseFirsts),
               extensionType) !=
        std::end(constants::opt_value::case_first::validCaseFirsts);
  }
  if (extensionKey == constants::extension_key::kn) {
    return std::find(
               std::begin(constants::opt_value::numeric::validNumerics),
               std::end(constants::opt_value::numeric::validNumerics),
               extensionType) !=
        std::end(constants::opt_value::numeric::validNumerics);
  }
  if (extensionKey == constants::extension_key::co) {
    // The Intl.Collator spec disallows "standard" and "search" as an
    // extension type per:
    // https://tc39.github.io/ecma402/#sec-intl-collator-internal-slots
    if (extensionType == collationTypeStandard ||
        extensionType == collationTypeSearch) {
      return false;
    }
    // ICU4C returns alias / legacy type ids for types that were available
    // before Unicode extension type defined its ids to be
    // 3 - 8 alphanumeric characters. Map these alias / legacy type ids to
    // Unicode extension type ids.
    // https://github.com/unicode-org/cldr/blob/main/common/bcp47/collation.xml
    static constexpr std::pair<std::u16string_view, std::u16string_view>
        aliasMap[] = {
            {u"dictionary", u"dict"},
            {u"gb2312han", u"gb2312"},
            {u"phonebook", u"phonebk"},
            {u"traditional", u"trad"}};
    auto icuLocale =
        convertBCP47toICULocale(localeBCP47Object.getLocaleNoExt());
    UErrorCode status = U_ZERO_ERROR;
    UEnumeration *supportedTypes = ucol_getKeywordValuesForLocale(
        "collation",
        icuLocale.c_str(),
        // false means to include all available values for the locale, not just
        // common ones
        false,
        &status);
    if (U_FAILURE(status)) {
      return false;
    }
    bool supported = false;
    int32_t length;
    const UChar *type = uenum_unext(supportedTypes, &length, &status);
    while (type != nullptr && U_SUCCESS(status)) {
      std::u16string_view supportedType(
          reinterpret_cast<const char16_t *>(type), length);
      for (const auto &aliasPair : aliasMap) {
        if (supportedType == aliasPair.first) {
          supportedType = aliasPair.second;
          break;
        }
      }
      if (extensionType == supportedType) {
        supported = true;
        break;
      }
      type = uenum_unext(supportedTypes, &length, &status);
    }
    uenum_close(supportedTypes);
    return supported;
  }

  return false;
}

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes
