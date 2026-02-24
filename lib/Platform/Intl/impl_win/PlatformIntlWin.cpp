/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * Windows Intl implementation using hermes_icu_vtable dispatch.
 *
 * This file implements the platform_intl interface (Collator, DateTimeFormat,
 * NumberFormat, getCanonicalLocales, toLocaleLowerCase, toLocaleUpperCase)
 * by dispatching ICU calls through the hermes_icu_vtable.
 *
 * When no ICU provider is available (vtable is nullptr), falls back to
 * WinGlob NLS-based stubs.
 *
 * Phase 2: Full Collator, NumberFormat, locale resolution implementations.
 */

#include "IcuProviderRegistry.h"
#include "PlatformIntlWinGlob.h"
#include "WinIntlUtils.h"

#include "hermes/Platform/Intl/BCP47Parser.h"
#include "hermes/Platform/Intl/PlatformIntl.h"
#include "hermes/Platform/Intl/hermes_icu.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace ::hermes;

namespace hermes {
namespace platform_intl {

// ============================================================================
// Locale resolution infrastructure (ported from impl_icu/LocaleResolver.cpp)
// ============================================================================

namespace {

/// Get all available ICU locales as BCP47, cached per vtable.
/// Returns a map from lowercased BCP47 key to properly-cased BCP47 value.
/// Different ICU providers may support different locale sets.
const std::unordered_map<std::string, std::string> &getAvailableLocales(
    const hermes_icu_vtable *icu) {
  // Cache keyed by vtable pointer. Entries are intentionally leaked
  // to avoid destruction order problems. Typically 1-2 entries.
  static std::mutex cacheMutex;
  static std::unordered_map<
      const hermes_icu_vtable *,
      std::unordered_map<std::string, std::string> *>
      cache;

  std::lock_guard<std::mutex> lock(cacheMutex);
  auto it = cache.find(icu);
  if (it != cache.end())
    return *it->second;

  auto *s = new std::unordered_map<std::string, std::string>();
  int32_t count = icu->uloc_countAvailable();
  for (int32_t i = 0; i < count; i++) {
    const char *loc = icu->uloc_getAvailable(i);
    if (loc) {
      std::string bcp47 = convertICUtoBCP47Locale(icu, loc);
      s->emplace(toLowerASCII(bcp47), bcp47);
    }
  }
  cache[icu] = s;
  return *s;
}

/// Get available locales as a u16string vector (for supportedLocalesOf).
/// Converts ICU locale IDs to BCP47 format so they match canonicalized
/// requested locales in lookupSupportedLocales.
std::vector<std::u16string> getAvailableLocalesU16(
    vm::Runtime &runtime,
    const hermes_icu_vtable *icu) {
  std::vector<std::u16string> result;
  int32_t count = icu->uloc_countAvailable();
  for (int32_t i = 0; i < count; i++) {
    const char *loc = icu->uloc_getAvailable(i);
    if (loc) {
      std::string bcp47 = convertICUtoBCP47Locale(icu, loc);
      auto conv = UTF8toUTF16(runtime, bcp47);
      if (conv != vm::ExecutionStatus::EXCEPTION)
        result.push_back(std::move(conv.getValue()));
    }
  }
  return result;
}

/// ECMA-402 BestAvailableLocale (lookup algorithm).
/// Returns properly-cased BCP47 locale string from the available set.
std::string bestAvailableLocaleICU(
    const hermes_icu_vtable *icu,
    std::u16string_view locale) {
  const auto &avail = getAvailableLocales(icu);
  std::string candidate = toLowerASCII(toUTF8ASCII(std::u16string(locale)));
  while (true) {
    auto it = avail.find(candidate);
    if (it != avail.end())
      return it->second; // Return properly-cased BCP47
    size_t pos = candidate.rfind('-');
    if (pos == std::string::npos)
      return std::string();
    if (pos >= 2 && candidate[pos - 2] == '-')
      pos -= 2;
    candidate.resize(pos);
  }
}

/// Get default locale as a ParsedLocaleIdentifier.
std::optional<ParsedLocaleIdentifier> getDefaultLocale(
    const hermes_icu_vtable *icu) {
  const char *defLoc = icu->uloc_getDefault();
  if (!defLoc)
    return std::nullopt;
  std::string bcp47 = convertICUtoBCP47Locale(icu, defLoc);
  return ParsedLocaleIdentifier::parse(toUTF16ASCII(bcp47));
}

/// Best-fit locale matching via uloc_acceptLanguage.
/// Falls back to bestAvailableLocaleICU when optional ICU functions are
/// not available (e.g. system icu.dll with limited exports).
std::string bestFitBestAvailableLocale(
    const hermes_icu_vtable *icu,
    const std::u16string &localeNoExt) {
  // uloc_openAvailableByType and uloc_acceptLanguage are optional vtable
  // entries — they may be nullptr when using a limited ICU provider
  // (e.g. the Windows system icu.dll).
  if (!icu->uloc_openAvailableByType || !icu->uloc_acceptLanguage) {
    return bestAvailableLocaleICU(icu, localeNoExt);
  }

  std::string localeICU = convertBCP47toICULocale(icu, localeNoExt);
  const char *acceptList[1]{localeICU.c_str()};

  UErrorCode err = U_ZERO_ERROR;
  UEnumeration *availEnum =
      icu->uloc_openAvailableByType(ULOC_AVAILABLE_DEFAULT, &err);
  if (U_FAILURE(err) || !availEnum)
    return std::string();

  char result[kULOC_FULLNAME_CAPACITY + 1] = {0};
  int32_t outResult = ULOC_ACCEPT_FAILED;
  err = U_ZERO_ERROR;
  int32_t resultLength = icu->uloc_acceptLanguage(
      result,
      kULOC_FULLNAME_CAPACITY,
      &outResult,
      acceptList,
      1,
      availEnum,
      &err);

  icu->uenum_close(availEnum);

  if (U_SUCCESS(err) && outResult != ULOC_ACCEPT_FAILED &&
      resultLength > 0) {
    return convertICUtoBCP47Locale(icu, result);
  }
  return std::string();
}

/// Get the locale string without Unicode extensions, with canonical BCP47
/// casing: language=lowercase, script=Titlecase, region=UPPERCASE.
/// ParsedLocaleIdentifier stores all subtags in lowercase, so we must
/// re-apply casing here.
std::u16string getLocaleNoExt(const ParsedLocaleIdentifier &parsed) {
  auto toUpper = [](char16_t c) -> char16_t {
    return (c >= u'a' && c <= u'z') ? (c - u'a' + u'A') : c;
  };

  // Language subtag: lowercase (already is).
  std::u16string result = parsed.languageIdentifier.languageSubtag;

  // Script subtag: Titlecase (first char uppercase, rest lowercase).
  if (!parsed.languageIdentifier.scriptSubtag.empty()) {
    const auto &script = parsed.languageIdentifier.scriptSubtag;
    result += u'-';
    result += toUpper(script[0]);
    for (size_t i = 1; i < script.size(); i++)
      result += script[i];
  }

  // Region subtag: UPPERCASE.
  if (!parsed.languageIdentifier.regionSubtag.empty()) {
    result += u'-';
    for (char16_t c : parsed.languageIdentifier.regionSubtag)
      result += toUpper(c);
  }

  // Variant subtags: lowercase (already is).
  for (const auto &variant : parsed.languageIdentifier.variantSubtagList) {
    result += u'-';
    result += variant;
  }
  return result;
}

/// Construct a canonicalized BCP47 locale with given extensions.
std::u16string buildLocaleWithExtensions(
    const std::u16string &localeNoExt,
    const std::map<std::u16string, std::u16string> &extMap) {
  if (extMap.empty())
    return localeNoExt;
  std::u16string result = localeNoExt;
  result += u"-u";
  for (const auto &[key, value] : extMap) {
    result += u'-';
    result += key;
    if (!value.empty() && value != u"true") {
      result += u'-';
      result += value;
    }
  }
  return result;
}

/// Resolved locale result from resolveLocale.
struct ResolvedLocaleResult {
  std::u16string localeNoExt;
  std::map<std::u16string, std::u16string> extensions;
  Options resolvedOpts;

  std::u16string getCanonicalizedLocaleId() const {
    return buildLocaleWithExtensions(localeNoExt, extensions);
  }
};

using ParsedLocaleList = std::vector<
    std::pair<std::u16string, std::map<std::u16string, std::u16string>>>;

/// Lookup matcher: find best locale from requested list.
ResolvedLocaleResult lookupMatcher(
    const hermes_icu_vtable *icu,
    const ParsedLocaleList &requestedLocales) {
  for (const auto &[localeNoExt, extMap] : requestedLocales) {
    std::string avail = bestAvailableLocaleICU(icu, localeNoExt);
    if (!avail.empty()) {
      return {toUTF16ASCII(avail), extMap, {}};
    }
  }
  auto defLocale = getDefaultLocale(icu);
  if (defLocale) {
    return {getLocaleNoExt(*defLocale), {}, {}};
  }
  return {u"en", {}, {}};
}

/// Best-fit matcher: use ICU's uloc_acceptLanguage.
ResolvedLocaleResult bestFitMatcher(
    const hermes_icu_vtable *icu,
    const ParsedLocaleList &requestedLocales) {
  for (const auto &[localeNoExt, extMap] : requestedLocales) {
    std::string avail = bestFitBestAvailableLocale(icu, localeNoExt);
    if (!avail.empty()) {
      return {toUTF16ASCII(avail), extMap, {}};
    }
  }
  auto defLocale = getDefaultLocale(icu);
  if (defLocale) {
    return {getLocaleNoExt(*defLocale), {}, {}};
  }
  return {u"en", {}, {}};
}

/// ECMA-402 Table 2: Sanctioned simple unit identifiers.
bool isSanctionedSimpleUnitIdentifier(std::u16string_view unit) {
  // Keep sorted for binary search.
  static constexpr std::u16string_view sanctioned[] = {
      u"acre",
      u"bit",
      u"byte",
      u"celsius",
      u"centimeter",
      u"day",
      u"degree",
      u"fahrenheit",
      u"fluid-ounce",
      u"foot",
      u"gallon",
      u"gigabit",
      u"gigabyte",
      u"gram",
      u"hectare",
      u"hour",
      u"inch",
      u"kilobit",
      u"kilobyte",
      u"kilogram",
      u"kilometer",
      u"liter",
      u"megabit",
      u"megabyte",
      u"meter",
      u"microsecond",
      u"mile",
      u"mile-scandinavian",
      u"milliliter",
      u"millimeter",
      u"millisecond",
      u"minute",
      u"month",
      u"nanosecond",
      u"ounce",
      u"percent",
      u"petabyte",
      u"pound",
      u"second",
      u"stone",
      u"terabit",
      u"terabyte",
      u"week",
      u"yard",
      u"year",
  };
  return std::binary_search(
      std::begin(sanctioned), std::end(sanctioned), unit);
}

/// ECMA-402 §6.5 IsWellFormedUnitIdentifier.
bool isWellFormedUnitIdentifier(const std::u16string &unitIdentifier) {
  if (isSanctionedSimpleUnitIdentifier(unitIdentifier))
    return true;
  // Check for compound unit: numerator-per-denominator
  const std::u16string per = u"-per-";
  auto pos = unitIdentifier.find(per);
  if (pos == std::u16string::npos || pos == 0)
    return false;
  std::u16string_view numerator(unitIdentifier.data(), pos);
  std::u16string_view denominator(
      unitIdentifier.data() + pos + per.size(),
      unitIdentifier.size() - pos - per.size());
  return isSanctionedSimpleUnitIdentifier(numerator) &&
      isSanctionedSimpleUnitIdentifier(denominator);
}

/// Collator extension type validation callback.
bool isCollatorExtensionTypeSupported(
    const hermes_icu_vtable *icu,
    std::u16string_view extensionKey,
    std::u16string_view extensionType,
    const std::u16string &localeNoExt) {
  static constexpr std::u16string_view validCaseFirsts[] = {
      u"upper", u"lower", u"false"};
  static constexpr std::u16string_view validNumerics[] = {u"true", u"false"};

  if (extensionKey == u"kf") {
    return std::find(
               std::begin(validCaseFirsts),
               std::end(validCaseFirsts),
               extensionType) != std::end(validCaseFirsts);
  }
  if (extensionKey == u"kn") {
    return std::find(
               std::begin(validNumerics),
               std::end(validNumerics),
               extensionType) != std::end(validNumerics);
  }
  if (extensionKey == u"co") {
    if (extensionType == u"standard" || extensionType == u"search")
      return false;

    static constexpr std::pair<std::u16string_view, std::u16string_view>
        aliasMap[] = {
            {u"dictionary", u"dict"},
            {u"gb2312han", u"gb2312"},
            {u"phonebook", u"phonebk"},
            {u"traditional", u"trad"}};

    std::string icuLocale = convertBCP47toICULocale(icu, localeNoExt);
    UErrorCode err = U_ZERO_ERROR;
    UEnumeration *supportedTypes = icu->ucol_getKeywordValuesForLocale(
        "collation", icuLocale.c_str(), 0, &err);
    if (U_FAILURE(err) || !supportedTypes)
      return false;

    bool supported = false;
    // ucol_getKeywordValuesForLocale returns UTF-8 strings,
    // so use uenum_next (UTF-8) not uenum_unext (UTF-16).
    int32_t length = 0;
    err = U_ZERO_ERROR;
    const char *typeStr = icu->uenum_next(supportedTypes, &length, &err);
    while (typeStr != nullptr && U_SUCCESS(err)) {
      std::u16string supportedTypeStr = toUTF16ASCII(
          std::string(typeStr, length));
      std::u16string_view supportedType{supportedTypeStr};
      for (const auto &[legacy, modern] : aliasMap) {
        if (supportedType == legacy) {
          supportedType = modern;
          break;
        }
      }
      if (extensionType == supportedType) {
        supported = true;
        break;
      }
      err = U_ZERO_ERROR;
      typeStr = icu->uenum_next(supportedTypes, &length, &err);
    }
    icu->uenum_close(supportedTypes);
    return supported;
  }
  return false;
}

using ExtTypeValidator = std::function<bool(
    std::u16string_view, /* extension key */
    std::u16string_view, /* extension type */
    const std::u16string & /* localeNoExt */)>;

/// ECMA-402 ResolveLocale algorithm.
ResolvedLocaleResult resolveLocale(
    const hermes_icu_vtable *icu,
    const ParsedLocaleList &requestedLocales,
    const Options &opt,
    llvh::ArrayRef<std::u16string_view> relevantExtensionKeys,
    const ExtTypeValidator &isExtensionTypeSupported) {
  auto matcherIt = opt.find(u"localeMatcher");
  bool useLookup = matcherIt != opt.end() &&
      matcherIt->second.getString() == u"lookup";

  ResolvedLocaleResult matched = useLookup
      ? lookupMatcher(icu, requestedLocales)
      : bestFitMatcher(icu, requestedLocales);

  std::map<std::u16string, std::u16string> supportedExts;
  for (const auto &[key, val] : matched.extensions) {
    std::u16string_view extType{val};
    if (extType.empty())
      extType = u"true";
    if (std::find(relevantExtensionKeys.begin(),
                  relevantExtensionKeys.end(),
                  key) != relevantExtensionKeys.end() &&
        isExtensionTypeSupported(key, extType, matched.localeNoExt)) {
      supportedExts.emplace(key, extType);
    }
  }

  Options resolvedOpts;
  for (const auto &extView : relevantExtensionKeys) {
    std::u16string ext{extView};
    auto extIter = supportedExts.find(ext);
    if (extIter != supportedExts.end()) {
      resolvedOpts.emplace(ext, extIter->second);
    }
    auto optIter = opt.find(ext);
    if (optIter != opt.end()) {
      std::u16string_view optExtType;
      const Option &option = optIter->second;
      if (option.isBool()) {
        optExtType = option.getBool() ? u"true" : u"false";
      } else {
        optExtType = option.getString();
      }
      if (isExtensionTypeSupported(ext, optExtType, matched.localeNoExt)) {
        resolvedOpts.insert_or_assign(ext, optIter->second);
        if (extIter != supportedExts.end() &&
            extIter->second != optExtType) {
          supportedExts.erase(extIter);
        }
      }
    }
  }

  matched.extensions = supportedExts;
  matched.resolvedOpts = resolvedOpts;
  return matched;
}

/// Parse locale list into (localeNoExt, extensionMap) pairs.
vm::CallResult<ParsedLocaleList> parseLocaleList(
    vm::Runtime &runtime,
    const hermes_icu_vtable *icu,
    const std::vector<std::u16string> &locales) {
  ParsedLocaleList result;
  for (const auto &locale : locales) {
    auto normalized = normalizeLanguageTag(runtime, icu, locale);
    if (LLVM_UNLIKELY(normalized == vm::ExecutionStatus::EXCEPTION))
      return vm::ExecutionStatus::EXCEPTION;
    auto parsed = ParsedLocaleIdentifier::parse(normalized.getValue());
    if (parsed) {
      std::u16string noExt = getLocaleNoExt(*parsed);
      result.push_back({noExt, parsed->unicodeExtensionKeywords});
    }
  }
  return result;
}

} // anonymous namespace

// ============================================================================
// getCanonicalLocales
// ============================================================================

vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales) {
  const hermes_icu_vtable *icu = getIcuVtableForRuntime(runtime);
  if (!icu)
    return winglob::getCanonicalLocales(runtime, locales);
  return canonicalizeLocaleList(runtime, icu, locales);
}

// ============================================================================
// toLocaleLowerCase / toLocaleUpperCase
// ============================================================================

vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  const hermes_icu_vtable *icu = getIcuVtableForRuntime(runtime);
  if (!icu)
    return winglob::toLocaleLowerCase(runtime, locales, str);

  std::string locale8;
  if (!locales.empty()) {
    auto conv = UTF16toUTF8(runtime, locales.front());
    if (LLVM_UNLIKELY(conv == vm::ExecutionStatus::EXCEPTION))
      return vm::ExecutionStatus::EXCEPTION;
    locale8 = std::move(conv.getValue());
  }

  UErrorCode err = U_ZERO_ERROR;
  int32_t resultLength = icu->u_strToLower(
      nullptr,
      0,
      reinterpret_cast<const UChar *>(str.data()),
      static_cast<int32_t>(str.length()),
      locale8.empty() ? nullptr : locale8.c_str(),
      &err);
  if (U_FAILURE(err) && err != U_BUFFER_OVERFLOW_ERROR) {
    return runtime.raiseRangeError("toLocaleLowerCase failed");
  }

  std::u16string result(resultLength, u' ');
  err = U_ZERO_ERROR;
  resultLength = icu->u_strToLower(
      reinterpret_cast<UChar *>(&result[0]),
      resultLength,
      reinterpret_cast<const UChar *>(str.data()),
      static_cast<int32_t>(str.length()),
      locale8.empty() ? nullptr : locale8.c_str(),
      &err);
  if (U_FAILURE(err)) {
    return runtime.raiseRangeError("toLocaleLowerCase failed");
  }
  result.resize(resultLength);
  return result;
}

vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  const hermes_icu_vtable *icu = getIcuVtableForRuntime(runtime);
  if (!icu)
    return winglob::toLocaleUpperCase(runtime, locales, str);

  std::string locale8;
  if (!locales.empty()) {
    auto conv = UTF16toUTF8(runtime, locales.front());
    if (LLVM_UNLIKELY(conv == vm::ExecutionStatus::EXCEPTION))
      return vm::ExecutionStatus::EXCEPTION;
    locale8 = std::move(conv.getValue());
  }

  UErrorCode err = U_ZERO_ERROR;
  int32_t resultLength = icu->u_strToUpper(
      nullptr,
      0,
      reinterpret_cast<const UChar *>(str.data()),
      static_cast<int32_t>(str.length()),
      locale8.empty() ? nullptr : locale8.c_str(),
      &err);
  if (U_FAILURE(err) && err != U_BUFFER_OVERFLOW_ERROR) {
    return runtime.raiseRangeError("toLocaleUpperCase failed");
  }

  std::u16string result(resultLength, u' ');
  err = U_ZERO_ERROR;
  resultLength = icu->u_strToUpper(
      reinterpret_cast<UChar *>(&result[0]),
      resultLength,
      reinterpret_cast<const UChar *>(str.data()),
      static_cast<int32_t>(str.length()),
      locale8.empty() ? nullptr : locale8.c_str(),
      &err);
  if (U_FAILURE(err)) {
    return runtime.raiseRangeError("toLocaleUpperCase failed");
  }
  result.resize(resultLength);
  return result;
}

// ============================================================================
// Collator (Phase 2: full ICU implementation)
// ============================================================================

namespace {

class CollatorWindows : public Collator {
 public:
  CollatorWindows() = default;
  ~CollatorWindows() override;

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const hermes_icu_vtable *icu,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

  Options resolvedOptions() noexcept;
  double compare(const std::u16string &x, const std::u16string &y) noexcept;

 private:
  void setAttributes();

  const hermes_icu_vtable *icu_ = nullptr;
  UCollator *coll_ = nullptr;

  std::u16string resolvedLocale_;
  std::u16string resolvedInternalLocale_;
  std::u16string resolvedUsageValue_;
  std::u16string resolvedSensitivityValue_;
  bool resolvedIgnorePunctuationValue_ = false;
  std::u16string resolvedCollationValue_;
  bool resolvedNumericValue_ = false;
  std::u16string resolvedCaseFirstValue_;
};

CollatorWindows::~CollatorWindows() {
  if (coll_ && icu_) {
    icu_->ucol_close(coll_);
  }
}

void CollatorWindows::setAttributes() {
  if (!coll_ || !icu_)
    return;

  UErrorCode err = U_ZERO_ERROR;

  // numeric
  err = U_ZERO_ERROR;
  icu_->ucol_setAttribute(
      coll_,
      UCOL_NUMERIC_COLLATION,
      resolvedNumericValue_ ? UCOL_ON : UCOL_OFF,
      &err);

  // caseFirst
  if (!resolvedCaseFirstValue_.empty()) {
    err = U_ZERO_ERROR;
    if (resolvedCaseFirstValue_ == u"upper") {
      icu_->ucol_setAttribute(coll_, UCOL_CASE_FIRST, UCOL_UPPER_FIRST, &err);
    } else if (resolvedCaseFirstValue_ == u"lower") {
      icu_->ucol_setAttribute(coll_, UCOL_CASE_FIRST, UCOL_LOWER_FIRST, &err);
    } else {
      icu_->ucol_setAttribute(coll_, UCOL_CASE_FIRST, UCOL_OFF, &err);
    }
  } else if (icu_->ucol_getAttribute) {
    err = U_ZERO_ERROR;
    UColAttributeValue val = icu_->ucol_getAttribute(coll_, UCOL_CASE_FIRST, &err);
    if (val == UCOL_UPPER_FIRST)
      resolvedCaseFirstValue_ = u"upper";
    else if (val == UCOL_LOWER_FIRST)
      resolvedCaseFirstValue_ = u"lower";
    else
      resolvedCaseFirstValue_ = u"false";
  } else {
    resolvedCaseFirstValue_ = u"false";
  }

  // sensitivity
  if (!resolvedSensitivityValue_.empty()) {
    err = U_ZERO_ERROR;
    if (resolvedSensitivityValue_ == u"base") {
      icu_->ucol_setAttribute(coll_, UCOL_STRENGTH, UCOL_PRIMARY, &err);
    } else if (resolvedSensitivityValue_ == u"accent") {
      icu_->ucol_setAttribute(coll_, UCOL_STRENGTH, UCOL_SECONDARY, &err);
    } else if (resolvedSensitivityValue_ == u"case") {
      icu_->ucol_setAttribute(coll_, UCOL_STRENGTH, UCOL_PRIMARY, &err);
      err = U_ZERO_ERROR;
      icu_->ucol_setAttribute(coll_, UCOL_CASE_LEVEL, UCOL_ON, &err);
    } else if (resolvedSensitivityValue_ == u"variant") {
      icu_->ucol_setAttribute(coll_, UCOL_STRENGTH, UCOL_TERTIARY, &err);
    }
  } else if (icu_->ucol_getAttribute) {
    err = U_ZERO_ERROR;
    UColAttributeValue strengthVal =
        icu_->ucol_getAttribute(coll_, UCOL_STRENGTH, &err);
    if (strengthVal == UCOL_PRIMARY) {
      err = U_ZERO_ERROR;
      UColAttributeValue caseLevelVal =
          icu_->ucol_getAttribute(coll_, UCOL_CASE_LEVEL, &err);
      resolvedSensitivityValue_ =
          (caseLevelVal == UCOL_ON) ? u"case" : u"base";
    } else if (strengthVal == UCOL_SECONDARY) {
      resolvedSensitivityValue_ = u"accent";
    } else {
      resolvedSensitivityValue_ = u"variant";
    }
  } else {
    resolvedSensitivityValue_ = u"variant";
  }

  // ignorePunctuation
  err = U_ZERO_ERROR;
  icu_->ucol_setAttribute(
      coll_,
      UCOL_ALTERNATE_HANDLING,
      resolvedIgnorePunctuationValue_ ? UCOL_SHIFTED : UCOL_NON_IGNORABLE,
      &err);
}

vm::ExecutionStatus CollatorWindows::initialize(
    vm::Runtime &runtime,
    const hermes_icu_vtable *icu,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  icu_ = icu;
  if (!icu_) {
    (void)runtime.raiseRangeError("Collator requires ICU support");
    return vm::ExecutionStatus::EXCEPTION;
  }

  // 1. CanonicalizeLocaleList
  auto requestedLocalesRes = parseLocaleList(runtime, icu_, locales);
  if (LLVM_UNLIKELY(requestedLocalesRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;

  // 3. usage
  static const std::vector<std::u16string> validUsages = {u"sort", u"search"};
  auto usageRes =
      getOptionString(runtime, options, u"usage", validUsages, u"sort");
  if (LLVM_UNLIKELY(usageRes == vm::ExecutionStatus::EXCEPTION))
    return usageRes.getStatus();
  resolvedUsageValue_ = usageRes.getValue();

  // 8. localeMatcher
  static const std::vector<std::u16string> validMatchers = {
      u"lookup", u"best fit"};
  auto matcherRes = getOptionString(
      runtime, options, u"localeMatcher", validMatchers, u"best fit");
  if (LLVM_UNLIKELY(matcherRes == vm::ExecutionStatus::EXCEPTION))
    return matcherRes.getStatus();
  Options opt;
  opt.emplace(u"localeMatcher", matcherRes.getValue());

  // 10. collation
  auto collationValueRes =
      getOptionString(runtime, options, u"collation", {}, {});
  if (LLVM_UNLIKELY(collationValueRes == vm::ExecutionStatus::EXCEPTION))
    return collationValueRes.getStatus();
  if (!collationValueRes.getValue().empty()) {
    if (!isUnicodeExtensionType(collationValueRes.getValue())) {
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid collation: ") +
          vm::TwineChar16(collationValueRes.getValue().c_str()));
    }
    opt.emplace(u"co", collationValueRes.getValue());
  }

  // 13. numeric
  auto numericValue = getOptionBool(runtime, options, u"numeric", {});
  if (numericValue.has_value()) {
    opt.emplace(u"kn", Option(*numericValue));
  }

  // 16. caseFirst
  static const std::vector<std::u16string> validCaseFirsts = {
      u"upper", u"lower", u"false"};
  auto caseFirstRes =
      getOptionString(runtime, options, u"caseFirst", validCaseFirsts, {});
  if (LLVM_UNLIKELY(caseFirstRes == vm::ExecutionStatus::EXCEPTION))
    return caseFirstRes.getStatus();
  if (!caseFirstRes.getValue().empty()) {
    opt.emplace(u"kf", caseFirstRes.getValue());
  }

  // 18. relevantExtensionKeys
  static constexpr std::u16string_view relevantExtensionKeys[] = {
      u"co", u"kn", u"kf"};

  // 19. ResolveLocale
  auto validator = [this](
                       std::u16string_view key,
                       std::u16string_view type,
                       const std::u16string &localeNoExt) {
    return isCollatorExtensionTypeSupported(icu_, key, type, localeNoExt);
  };
  ResolvedLocaleResult result = resolveLocale(
      icu_, requestedLocalesRes.getValue(), opt, relevantExtensionKeys,
      validator);

  // 20. Set locale
  resolvedLocale_ = result.getCanonicalizedLocaleId();

  // 21-23. collation
  resolvedCollationValue_ = u"default";
  auto &resolvedExt = result.extensions;
  if (resolvedUsageValue_ == u"search") {
    resolvedExt.erase(u"co");
    resolvedExt[u"co"] = u"search";
  } else {
    auto collationEntry = result.resolvedOpts.find(u"co");
    if (collationEntry != result.resolvedOpts.end()) {
      resolvedCollationValue_ = collationEntry->second.getString();
      resolvedExt[u"co"] = resolvedCollationValue_;
    }
  }

  resolvedInternalLocale_ =
      buildLocaleWithExtensions(result.localeNoExt, resolvedExt);

  // 24. numeric
  resolvedNumericValue_ = false;
  auto numericEntry = result.resolvedOpts.find(u"kn");
  if (numericEntry != result.resolvedOpts.end()) {
    if (numericEntry->second.isBool()) {
      resolvedNumericValue_ = numericEntry->second.getBool();
    } else {
      resolvedNumericValue_ = convertToBool(numericEntry->second.getString());
    }
  }

  // 25. caseFirst
  auto caseFirstEntry = result.resolvedOpts.find(u"kf");
  if (caseFirstEntry != result.resolvedOpts.end()) {
    resolvedCaseFirstValue_ = caseFirstEntry->second.getString();
  }

  // 26-28. sensitivity
  static const std::vector<std::u16string> validSensitivities = {
      u"base", u"accent", u"case", u"variant"};
  auto sensRes = getOptionString(
      runtime, options, u"sensitivity", validSensitivities, {});
  if (LLVM_UNLIKELY(sensRes == vm::ExecutionStatus::EXCEPTION))
    return sensRes.getStatus();
  if (!sensRes.getValue().empty()) {
    resolvedSensitivityValue_ = sensRes.getValue();
  } else if (resolvedUsageValue_ == u"sort") {
    resolvedSensitivityValue_ = u"variant";
  }

  // 29-30. ignorePunctuation
  auto ignorePuncValue =
      getOptionBool(runtime, options, u"ignorePunctuation", false);
  resolvedIgnorePunctuationValue_ = ignorePuncValue.value_or(false);

  // Open ICU collator.
  std::string localeICU =
      convertBCP47toICULocale(icu_, resolvedInternalLocale_);
  UErrorCode err = U_ZERO_ERROR;
  coll_ = icu_->ucol_open(localeICU.c_str(), &err);
  if (U_FAILURE(err) || !coll_) {
    err = U_ZERO_ERROR;
    coll_ = icu_->ucol_open("", &err);
  }

  if (coll_) {
    err = U_ZERO_ERROR;
    icu_->ucol_setAttribute(coll_, UCOL_NORMALIZATION_MODE, UCOL_ON, &err);
    setAttributes();
  }

  return vm::ExecutionStatus::RETURNED;
}

Options CollatorWindows::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(resolvedLocale_));
  options.emplace(u"usage", Option(resolvedUsageValue_));
  options.emplace(u"sensitivity", Option(resolvedSensitivityValue_));
  options.emplace(
      u"ignorePunctuation", Option(resolvedIgnorePunctuationValue_));
  options.emplace(u"collation", Option(resolvedCollationValue_));
  options.emplace(u"numeric", Option(resolvedNumericValue_));
  options.emplace(u"caseFirst", Option(resolvedCaseFirstValue_));
  return options;
}

double CollatorWindows::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  if (!coll_ || !icu_)
    return x.compare(y);

  UCollationResult result = icu_->ucol_strcoll(
      coll_,
      reinterpret_cast<const UChar *>(x.data()),
      static_cast<int32_t>(x.size()),
      reinterpret_cast<const UChar *>(y.data()),
      static_cast<int32_t>(y.size()));

  if (result == UCOL_LESS)
    return -1;
  if (result == UCOL_GREATER)
    return 1;
  return 0;
}

} // namespace

Collator::Collator() = default;
Collator::~Collator() = default;

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  const hermes_icu_vtable *icu = getIcuVtableForRuntime(runtime);
  if (!icu) {
    auto requestedLocales = winglob::getCanonicalLocales(runtime, locales);
    if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION))
      return vm::ExecutionStatus::EXCEPTION;
    return supportedLocales(
        runtime,
        winglob::getAvailableLocales(),
        requestedLocales.getValue(),
        options);
  }

  auto availableLocales = getAvailableLocalesU16(runtime, icu);
  auto requestedLocales = getCanonicalLocales(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  return supportedLocales(
      runtime, availableLocales, requestedLocales.getValue(), options);
}

vm::CallResult<std::unique_ptr<Collator>> Collator::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  const hermes_icu_vtable *icu = getIcuVtableForRuntime(runtime);
  if (icu) {
    auto instance = std::make_unique<CollatorWindows>();
    instance->usesIcu_ = true;
    if (LLVM_UNLIKELY(
            instance->initialize(runtime, icu, locales, options) ==
            vm::ExecutionStatus::EXCEPTION)) {
      return vm::ExecutionStatus::EXCEPTION;
    }
    return instance;
  }
  auto instance = std::make_unique<winglob::CollatorWinGlob>();
  if (LLVM_UNLIKELY(
          instance->initialize(runtime, locales, options) ==
          vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return instance;
}

Options Collator::resolvedOptions() noexcept {
  if (usesIcu_)
    return static_cast<CollatorWindows *>(this)->resolvedOptions();
  return static_cast<winglob::CollatorWinGlob *>(this)->resolvedOptions();
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  if (usesIcu_)
    return static_cast<CollatorWindows *>(this)->compare(x, y);
  return static_cast<winglob::CollatorWinGlob *>(this)->compare(x, y);
}

// ============================================================================
// DateTimeFormat (ported from Phase 1, unchanged)
// ============================================================================

namespace {
class DateTimeFormatWindows : public DateTimeFormat {
 public:
  DateTimeFormatWindows() = default;
  ~DateTimeFormatWindows();

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const hermes_icu_vtable *icu,
      const std::vector<std::u16string> &locales,
      const Options &inputOptions) noexcept;

  Options resolvedOptions() noexcept;
  std::u16string format(double jsTimeValue) noexcept;
  std::vector<Part> formatToParts(double x) noexcept;

 private:
  std::u16string locale_;
  std::u16string timeZone_;
  std::u16string weekday_;
  std::u16string era_;
  std::u16string year_;
  std::u16string month_;
  std::u16string day_;
  std::u16string dayPeriod_;
  std::u16string hour_;
  std::u16string minute_;
  std::u16string second_;
  std::u16string timeZoneName_;
  std::u16string dateStyle_;
  std::u16string timeStyle_;
  std::u16string calendar_;
  std::u16string numberingSystem_;
  std::u16string hourCycle_;
  int fractionalSecondDigits_ = 0;
  UDateFormat *dtf_ = nullptr;
  std::string locale8_;
  const hermes_icu_vtable *icu_ = nullptr;
  UDateFormat *getUDateFormatter(vm::Runtime &runtime);
  vm::CallResult<std::u16string> getDefaultHourCycle(vm::Runtime &runtime);
};
} // namespace

DateTimeFormatWindows::~DateTimeFormatWindows() {
  if (dtf_ && icu_) {
    icu_->udat_close(dtf_);
  }
}

DateTimeFormat::DateTimeFormat() = default;
DateTimeFormat::~DateTimeFormat() = default;

vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  const hermes_icu_vtable *icu = getIcuVtableForRuntime(runtime);
  if (!icu) {
    auto requestedLocales = winglob::getCanonicalLocales(runtime, locales);
    if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION))
      return vm::ExecutionStatus::EXCEPTION;
    return supportedLocales(
        runtime,
        winglob::getAvailableLocales(),
        requestedLocales.getValue(),
        options);
  }

  auto availableLocales = getAvailableLocalesU16(runtime, icu);
  auto requestedLocales = getCanonicalLocales(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  return supportedLocales(
      runtime, availableLocales, requestedLocales.getValue(), options);
}

vm::ExecutionStatus DateTimeFormatWindows::initialize(
    vm::Runtime &runtime,
    const hermes_icu_vtable *icu,
    const std::vector<std::u16string> &locales,
    const Options &inputOptions) noexcept {
  icu_ = icu;
  if (!icu_) {
    (void)runtime.raiseRangeError("DateTimeFormat requires ICU support");
    return vm::ExecutionStatus::EXCEPTION;
  }

  auto requestedLocalesRes = canonicalizeLocaleList(runtime, icu_, locales);
  if (requestedLocalesRes.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return requestedLocalesRes.getStatus();
  }
  if (!requestedLocalesRes.getValue().empty()) {
    locale_ = requestedLocalesRes.getValue().front();
  } else if (!locales.empty()) {
    locale_ = locales.front();
  } else {
    // Use system default locale.
    const char *defLoc = icu_->uloc_getDefault();
    if (defLoc)
      locale_ = toUTF16ASCII(convertICUtoBCP47Locale(icu_, defLoc));
    else
      locale_ = u"en-US";
  }

  // Save extension values from the locale before stripping for later use.
  std::u16string extHc = getUnicodeExtensionValue(locale_, u"hc");
  std::u16string extNu = getUnicodeExtensionValue(locale_, u"nu");
  std::u16string extCa = getUnicodeExtensionValue(locale_, u"ca");

  // ECMA-402 §11.1.2: Strip ALL unicode extensions from locale. Relevant
  // extensions (ca, nu, hc) will be re-added after option resolution only
  // if they match what the implementation actually uses.
  static const std::unordered_set<std::u16string> emptyKeys = {};
  locale_ = filterUnicodeExtensions(locale_, emptyKeys);

  auto conversion = UTF16toUTF8(runtime, locale_);
  if (conversion.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return conversion.getStatus();
  }
  locale8_ = conversion.getValue();

  Options options =
      toDateTimeOptions(runtime, inputOptions, u"any", u"date").getValue();
  std::unordered_map<std::u16string, std::u16string> opt;

  // localeMatcher
  auto matcher = getOptionString(
      runtime, options, u"localeMatcher", {u"lookup", u"best fit"},
      u"best fit");
  if (LLVM_UNLIKELY(matcher == vm::ExecutionStatus::EXCEPTION))
    return matcher.getStatus();
  opt.emplace(u"localeMatcher", matcher.getValue());

  // calendar (validate BCP47 unicode extension type format)
  auto calendarRes = getOptionString(runtime, options, u"calendar", {}, {});
  if (LLVM_UNLIKELY(calendarRes == vm::ExecutionStatus::EXCEPTION))
    return calendarRes.getStatus();
  bool hasCalendarOption = options.count(u"calendar") > 0;
  if (hasCalendarOption &&
      !isUnicodeExtensionType(calendarRes.getValue())) {
    return runtime.raiseRangeError("Invalid calendar value");
  }
  // Option takes precedence over extension; extension is fallback.
  if (hasCalendarOption) {
    calendar_ = calendarRes.getValue();
  } else if (!extCa.empty() && isUnicodeExtensionType(extCa)) {
    calendar_ = extCa;
  }
  opt.emplace(u"ca", calendar_);

  // numberingSystem (validate BCP47 unicode extension type format)
  auto nuSysRes =
      getOptionString(runtime, options, u"numberingSystem", {}, {});
  if (LLVM_UNLIKELY(nuSysRes == vm::ExecutionStatus::EXCEPTION))
    return nuSysRes.getStatus();
  bool hasNuOption = options.count(u"numberingSystem") > 0;
  if (hasNuOption && !isUnicodeExtensionType(nuSysRes.getValue())) {
    return runtime.raiseRangeError("Invalid numberingSystem value");
  }
  // Option takes precedence over extension; extension is fallback.
  // For nu extensions, only accept values from the CLDR numbering system
  // list. Reject abstract values (native/traditio/finance) and unknown
  // identifiers.
  static const std::unordered_set<std::u16string> validNuValues = {
      u"adlm",    u"ahom",    u"arab",    u"arabext", u"armn",
      u"armnlow", u"bali",    u"beng",    u"bhks",    u"brah",
      u"cakm",    u"cham",    u"cyrl",    u"deva",    u"diak",
      u"ethi",    u"fullwide",u"geor",    u"gong",    u"gonm",
      u"grek",    u"greklow", u"gujr",    u"guru",    u"hanidays",
      u"hanidec", u"hans",    u"hansfin", u"hant",    u"hantfin",
      u"hebr",    u"hmng",    u"hmnp",    u"java",    u"jpan",
      u"jpanfin", u"jpanyear",u"kali",    u"khmr",    u"knda",
      u"lana",    u"lanatham",u"laoo",    u"latn",    u"lepc",
      u"limb",    u"mathbold",u"mathdbl", u"mathmono",u"mathsanb",
      u"mathsans",u"mlym",    u"modi",    u"mong",    u"mroo",
      u"mtei",    u"mymr",    u"mymrshan",u"mymrtlng",u"newa",
      u"nkoo",    u"olck",    u"orya",    u"osma",    u"rohg",
      u"roman",   u"romanlow",u"saur",    u"segment", u"shrd",
      u"sind",    u"sinh",    u"sora",    u"sund",    u"takr",
      u"talu",    u"tamldec", u"telu",    u"thai",    u"tibt",
      u"tirh",    u"tnsa",    u"vaii",    u"wara",    u"wcho",
  };
  if (hasNuOption) {
    numberingSystem_ = nuSysRes.getValue();
  } else if (!extNu.empty() && validNuValues.count(extNu) > 0) {
    numberingSystem_ = extNu;
  }
  opt.emplace(u"nu", numberingSystem_);

  auto hour12 = getOptionBool(runtime, options, u"hour12", {});

  static const std::vector<std::u16string> hourCycles = {
      u"h11", u"h12", u"h23", u"h24"};
  auto hourCycleRes =
      getOptionString(runtime, options, u"hourCycle", hourCycles, {});
  std::u16string hourCycle = hourCycleRes.getValue();
  bool hasHourCycleOption = !hourCycle.empty();
  bool hasHour12Option = hour12.has_value();
  // Use hc extension value as fallback when no option is provided.
  if (!hasHourCycleOption && !hasHour12Option && !extHc.empty()) {
    if (extHc == u"h11" || extHc == u"h12" ||
        extHc == u"h23" || extHc == u"h24") {
      hourCycle = extHc;
    }
  }
  if (hasHour12Option) {
    hourCycle = u"";
  }
  opt.emplace(u"hc", hourCycle);
  hourCycle_ = hourCycle;

  // ECMA-402 §11.1.2: Re-add valid unicode extensions to the resolved locale
  // that came from the input locale (not overridden by explicit options).
  {
    std::u16string exts;
    // ca: re-add if from extension and not overridden by option
    if (!calendar_.empty() && !hasCalendarOption &&
        !extCa.empty() && calendar_ == extCa) {
      exts += u"ca-" + calendar_;
    }
    // hc: re-add if from extension and not overridden by option
    if (!extHc.empty() && !hasHourCycleOption && !hasHour12Option) {
      if (extHc == u"h11" || extHc == u"h12" ||
          extHc == u"h23" || extHc == u"h24") {
        if (!exts.empty())
          exts += u"-";
        exts += u"hc-" + extHc;
      }
    }
    // nu: re-add if from extension and not overridden by option
    if (!numberingSystem_.empty() && !hasNuOption &&
        !extNu.empty() && numberingSystem_ == extNu) {
      if (!exts.empty())
        exts += u"-";
      exts += u"nu-" + numberingSystem_;
    }
    if (!exts.empty()) {
      locale_ += u"-u-" + exts;
    }
  }

  auto timeZoneRes = options.find(u"timeZone");
  if (timeZoneRes != options.end()) {
    std::u16string tz(timeZoneRes->second.getString());
    if (tz.empty()) {
      return runtime.raiseRangeError("Invalid time zone");
    }
    // Try to validate and canonicalize the timezone via ICU.
    auto tzResult = validateAndCanonicalizeTimeZone(runtime, icu_, tz);
    if (tzResult.getStatus() == vm::ExecutionStatus::EXCEPTION)
      return vm::ExecutionStatus::EXCEPTION;
    timeZone_ = tzResult.getValue();
  }
  // If no timezone was specified, get the system default timezone.
  if (timeZone_.empty()) {
    UChar tzBuf[128];
    UErrorCode tzErr = U_ZERO_ERROR;
    int32_t tzLen =
        icu_->ucal_getDefaultTimeZone(tzBuf, 128, &tzErr);
    if (U_SUCCESS(tzErr) && tzLen > 0 && tzLen <= 128) {
      timeZone_.assign(
          reinterpret_cast<const char16_t *>(tzBuf), tzLen);
      // Canonicalize common aliases to "UTC".
      if (timeZone_ == u"Etc/UTC" || timeZone_ == u"Etc/GMT" ||
          timeZone_ == u"GMT") {
        timeZone_ = u"UTC";
      }
    }
  }

  // formatMatcher
  static const std::vector<std::u16string> validFormatMatchers = {
      u"basic", u"best fit"};
  auto fmRes = getOptionString(
      runtime, options, u"formatMatcher", validFormatMatchers, u"best fit");
  if (LLVM_UNLIKELY(fmRes == vm::ExecutionStatus::EXCEPTION))
    return fmRes.getStatus();

  // fractionalSecondDigits (integer 1-3)
  auto fsdRes = getNumberOption(
      runtime, options, u"fractionalSecondDigits", 1, 3, {});
  if (LLVM_UNLIKELY(fsdRes == vm::ExecutionStatus::EXCEPTION))
    return fsdRes.getStatus();
  fractionalSecondDigits_ = fsdRes.getValue().value_or(0);

  static const std::vector<std::u16string> dateStyles = {
      u"full", u"long", u"medium", u"short"};
  auto dateStyleRes =
      getOptionString(runtime, options, u"dateStyle", dateStyles, {});
  if (dateStyleRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return dateStyleRes.getStatus();
  dateStyle_ = dateStyleRes.getValue();

  static const std::vector<std::u16string> timeStyles = {
      u"full", u"long", u"medium", u"short"};
  auto timeStyleRes =
      getOptionString(runtime, options, u"timeStyle", timeStyles, {});
  if (timeStyleRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return timeStyleRes.getStatus();
  timeStyle_ = timeStyleRes.getValue();

  static const std::vector<std::u16string> weekdayValues = {
      u"narrow", u"short", u"long"};
  auto weekdayRes =
      getOptionString(runtime, options, u"weekday", weekdayValues, {});
  if (weekdayRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return weekdayRes.getStatus();
  weekday_ = weekdayRes.getValue();

  static const std::vector<std::u16string> eraValues = {
      u"narrow", u"short", u"long"};
  auto eraRes = getOptionString(runtime, options, u"era", eraValues, {});
  if (eraRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return eraRes.getStatus();
  era_ = *eraRes;

  static const std::vector<std::u16string> yearValues = {
      u"2-digit", u"numeric"};
  auto yearRes = getOptionString(runtime, options, u"year", yearValues, {});
  if (yearRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return yearRes.getStatus();
  year_ = *yearRes;

  static const std::vector<std::u16string> monthValues = {
      u"2-digit", u"numeric", u"narrow", u"short", u"long"};
  auto monthRes = getOptionString(runtime, options, u"month", monthValues, {});
  if (monthRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return monthRes.getStatus();
  month_ = *monthRes;

  static const std::vector<std::u16string> dayValues = {
      u"2-digit", u"numeric"};
  auto dayRes = getOptionString(runtime, options, u"day", dayValues, {});
  if (dayRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return dayRes.getStatus();
  day_ = *dayRes;

  static const std::vector<std::u16string> dayPeriodValues = {
      u"narrow", u"short", u"long"};
  auto dayPeriodRes =
      getOptionString(runtime, options, u"dayPeriod", dayPeriodValues, {});
  if (dayPeriodRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return dayPeriodRes.getStatus();
  dayPeriod_ = *dayPeriodRes;

  static const std::vector<std::u16string> hourValues = {
      u"2-digit", u"numeric"};
  auto hourRes = getOptionString(runtime, options, u"hour", hourValues, {});
  if (hourRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return hourRes.getStatus();
  hour_ = *hourRes;

  static const std::vector<std::u16string> minuteValues = {
      u"2-digit", u"numeric"};
  auto minuteRes =
      getOptionString(runtime, options, u"minute", minuteValues, {});
  if (minuteRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return minuteRes.getStatus();
  minute_ = *minuteRes;

  static const std::vector<std::u16string> secondValues = {
      u"2-digit", u"numeric"};
  auto secondRes =
      getOptionString(runtime, options, u"second", secondValues, {});
  if (secondRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return secondRes.getStatus();
  second_ = *secondRes;

  static const std::vector<std::u16string> timeZoneNameValues = {
      u"short", u"long", u"shortOffset", u"longOffset",
      u"shortGeneric", u"longGeneric"};
  auto timeZoneNameRes = getOptionString(
      runtime, options, u"timeZoneName", timeZoneNameValues, {});
  if (timeZoneNameRes.getStatus() == vm::ExecutionStatus::EXCEPTION)
    return timeZoneNameRes.getStatus();
  timeZoneName_ = *timeZoneNameRes;

  if (hour_.empty()) {
    hourCycle_ = u"";
  } else {
    std::u16string hcDefault = getDefaultHourCycle(runtime).getValue();
    auto hc = hourCycle_;
    if (hc.empty())
      hc = hcDefault;
    if (hour12.has_value()) {
      if (*hour12) {
        hc = (hcDefault == u"h11" || hcDefault == u"h23") ? u"h11" : u"h12";
      } else {
        hc = (hcDefault == u"h11" || hcDefault == u"h23") ? u"h23" : u"h24";
      }
    }
    hourCycle_ = hc;
  }

  // Apply resolved calendar and numberingSystem to the ICU locale string
  // so they take effect when creating the formatter.
  {
    std::string icuLocale = convertBCP47toICULocale(icu_, locale_);
    if (!calendar_.empty()) {
      std::string calUtf8(calendar_.begin(), calendar_.end());
      icuLocale +=
          (icuLocale.find('@') != std::string::npos ? ";" : "@");
      icuLocale += "calendar=" + calUtf8;
    }
    if (!numberingSystem_.empty()) {
      std::string nuUtf8(numberingSystem_.begin(), numberingSystem_.end());
      icuLocale +=
          (icuLocale.find('@') != std::string::npos ? ";" : "@");
      icuLocale += "numbers=" + nuUtf8;
    }
    locale8_ = icuLocale;
  }

  dtf_ = getUDateFormatter(runtime);
  return vm::ExecutionStatus::RETURNED;
}

vm::CallResult<std::unique_ptr<DateTimeFormat>> DateTimeFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &inputOptions) noexcept {
  const hermes_icu_vtable *icu = getIcuVtableForRuntime(runtime);
  if (icu) {
    auto instance = std::make_unique<DateTimeFormatWindows>();
    instance->usesIcu_ = true;
    if (LLVM_UNLIKELY(
            instance->initialize(runtime, icu, locales, inputOptions) ==
            vm::ExecutionStatus::EXCEPTION)) {
      return vm::ExecutionStatus::EXCEPTION;
    }
    return instance;
  }
  auto instance = std::make_unique<winglob::DateTimeFormatWinGlob>();
  if (LLVM_UNLIKELY(
          instance->initialize(runtime, locales, inputOptions) ==
          vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return instance;
}

Options DateTimeFormatWindows::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(locale_));
  // calendar: default to "gregory" if not explicitly set
  if (!calendar_.empty())
    options.emplace(u"calendar", Option(calendar_));
  else
    options.emplace(u"calendar", Option(std::u16string(u"gregory")));
  // numberingSystem: default to "latn" if not explicitly set
  if (!numberingSystem_.empty())
    options.emplace(u"numberingSystem", Option(numberingSystem_));
  else
    options.emplace(u"numberingSystem", Option(std::u16string(u"latn")));
  options.emplace(u"timeZone", Option(timeZone_));
  // hourCycle and hour12: only present when hour component is in the format
  if (!hourCycle_.empty()) {
    options.emplace(u"hourCycle", Option(hourCycle_));
    bool h12 = (hourCycle_ == u"h11" || hourCycle_ == u"h12");
    options.emplace(u"hour12", Option(h12));
  }
  // ECMA-402: only include component properties that were actually resolved.
  if (!weekday_.empty())
    options.emplace(u"weekday", weekday_);
  if (!era_.empty())
    options.emplace(u"era", era_);
  if (!year_.empty())
    options.emplace(u"year", year_);
  if (!month_.empty())
    options.emplace(u"month", month_);
  if (!day_.empty())
    options.emplace(u"day", day_);
  if (!hour_.empty())
    options.emplace(u"hour", hour_);
  if (!minute_.empty())
    options.emplace(u"minute", minute_);
  if (!second_.empty())
    options.emplace(u"second", second_);
  if (fractionalSecondDigits_ > 0) {
    options.emplace(
        u"fractionalSecondDigits",
        Option(static_cast<double>(fractionalSecondDigits_)));
  }
  if (!timeZoneName_.empty())
    options.emplace(u"timeZoneName", timeZoneName_);
  if (!dateStyle_.empty())
    options.emplace(u"dateStyle", dateStyle_);
  if (!timeStyle_.empty())
    options.emplace(u"timeStyle", timeStyle_);
  return options;
}

Options DateTimeFormat::resolvedOptions() noexcept {
  if (usesIcu_)
    return static_cast<DateTimeFormatWindows *>(this)->resolvedOptions();
  return static_cast<winglob::DateTimeFormatWinGlob *>(this)
      ->resolvedOptions();
}

std::u16string DateTimeFormatWindows::format(double jsTimeValue) noexcept {
  if (!icu_ || !dtf_)
    return u"";

  UDate date = static_cast<UDate>(jsTimeValue);
  std::u16string result;

  UErrorCode err = U_ZERO_ERROR;
  int32_t len = icu_->udat_format(dtf_, date, nullptr, 0, nullptr, &err);
  if (err == U_BUFFER_OVERFLOW_ERROR && len > 0) {
    result.resize(len);
    err = U_ZERO_ERROR;
    icu_->udat_format(
        dtf_, date,
        reinterpret_cast<UChar *>(&result[0]), len, nullptr, &err);
  }
  return result;
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  if (usesIcu_)
    return static_cast<DateTimeFormatWindows *>(this)->format(jsTimeValue);
  return static_cast<winglob::DateTimeFormatWinGlob *>(this)->format(
      jsTimeValue);
}

vm::CallResult<std::u16string> DateTimeFormatWindows::getDefaultHourCycle(
    vm::Runtime &runtime) {
  if (!icu_)
    return std::u16string(u"h12");

  UErrorCode err = U_ZERO_ERROR;
  UDateFormat *defaultDTF = icu_->udat_open(
      kUDAT_DEFAULT, kUDAT_DEFAULT, locale8_.c_str(),
      nullptr, -1, nullptr, -1, &err);
  if (U_FAILURE(err) || !defaultDTF)
    return std::u16string(u"h12");

  err = U_ZERO_ERROR;
  int32_t size = icu_->udat_toPattern(defaultDTF, 1, nullptr, 0, &err);
  std::u16string pattern;
  if (err == U_BUFFER_OVERFLOW_ERROR && size > 0) {
    pattern.resize(size + 1);
    err = U_ZERO_ERROR;
    icu_->udat_toPattern(
        defaultDTF, 1,
        reinterpret_cast<UChar *>(&pattern[0]),
        size + 1, &err);
  }
  icu_->udat_close(defaultDTF);

  for (int32_t i = 0; i < size; i++) {
    switch (pattern[i]) {
      case u'K': return std::u16string(u"h11");
      case u'h': return std::u16string(u"h12");
      case u'H': return std::u16string(u"h23");
      case u'k': return std::u16string(u"h24");
    }
  }
  return std::u16string(u"h12");
}

UDateFormat *DateTimeFormatWindows::getUDateFormatter(
    vm::Runtime &runtime) {
  if (!icu_)
    return nullptr;

  static std::u16string eLong = u"long", eShort = u"short",
                        eNarrow = u"narrow", eMedium = u"medium",
                        eFull = u"full", eNumeric = u"numeric",
                        eTwoDigit = u"2-digit",
                        eShortOffset = u"shortOffset",
                        eLongOffset = u"longOffset",
                        eShortGeneric = u"shortGeneric",
                        eLongGeneric = u"longGeneric";

  if (!timeStyle_.empty() || !dateStyle_.empty()) {
    int32_t dateStyleRes = kUDAT_DEFAULT;
    int32_t timeStyleRes = kUDAT_DEFAULT;
    if (!dateStyle_.empty()) {
      if (dateStyle_ == eFull) dateStyleRes = kUDAT_FULL;
      else if (dateStyle_ == eLong) dateStyleRes = kUDAT_LONG;
      else if (dateStyle_ == eMedium) dateStyleRes = kUDAT_MEDIUM;
      else if (dateStyle_ == eShort) dateStyleRes = kUDAT_SHORT;
    }
    if (!timeStyle_.empty()) {
      if (timeStyle_ == eFull) timeStyleRes = kUDAT_FULL;
      else if (timeStyle_ == eLong) timeStyleRes = kUDAT_LONG;
      else if (timeStyle_ == eMedium) timeStyleRes = kUDAT_MEDIUM;
      else if (timeStyle_ == eShort) timeStyleRes = kUDAT_SHORT;
    }
    UErrorCode err = U_ZERO_ERROR;
    UDateFormat *dtf = nullptr;
    if (!timeZone_.empty()) {
      dtf = icu_->udat_open(
          timeStyleRes, dateStyleRes, locale8_.c_str(),
          reinterpret_cast<const UChar *>(timeZone_.c_str()),
          static_cast<int32_t>(timeZone_.length()),
          nullptr, -1, &err);
    } else {
      dtf = icu_->udat_open(
          timeStyleRes, dateStyleRes, locale8_.c_str(),
          nullptr, -1, nullptr, -1, &err);
    }
    return dtf;
  }

  std::u16string skeleton;
  if (!weekday_.empty()) {
    if (weekday_ == eNarrow) skeleton += u"EEEEE";
    else if (weekday_ == eLong) skeleton += u"EEEE";
    else if (weekday_ == eShort) skeleton += u"EEE";
  }
  if (!timeZoneName_.empty()) {
    if (timeZoneName_ == eShort) skeleton += u"z";
    else if (timeZoneName_ == eLong) skeleton += u"zzzz";
    else if (timeZoneName_ == eShortOffset) skeleton += u"O";
    else if (timeZoneName_ == eLongOffset) skeleton += u"OOOO";
    else if (timeZoneName_ == eShortGeneric) skeleton += u"v";
    else if (timeZoneName_ == eLongGeneric) skeleton += u"vvvv";
  }
  if (!era_.empty()) {
    if (era_ == eNarrow) skeleton += u"GGGGG";
    else if (era_ == eShort) skeleton += u"G";
    else if (era_ == eLong) skeleton += u"GGGG";
  }
  if (!year_.empty()) {
    if (year_ == eNumeric) skeleton += u"y";
    else if (year_ == eTwoDigit) skeleton += u"yy";
  }
  if (!month_.empty()) {
    if (month_ == eTwoDigit) skeleton += u"MM";
    else if (month_ == eNumeric) skeleton += u'M';
    else if (month_ == eNarrow) skeleton += u"MMMMM";
    else if (month_ == eShort) skeleton += u"MMM";
    else if (month_ == eLong) skeleton += u"MMMM";
  }
  if (!day_.empty()) {
    if (day_ == eNumeric) skeleton += u"d";
    else if (day_ == eTwoDigit) skeleton += u"dd";
  }
  if (!hour_.empty()) {
    if (hourCycle_ == u"h12")
      skeleton += (hour_ == eTwoDigit) ? u"hh" : u"h";
    else if (hourCycle_ == u"h24")
      skeleton += (hour_ == eTwoDigit) ? u"kk" : u"k";
    else if (hourCycle_ == u"h23")
      skeleton += (hour_ == eTwoDigit) ? u"KK" : u"k";
    else
      skeleton += (hour_ == eTwoDigit) ? u"HH" : u"h";
  }
  if (!minute_.empty()) {
    skeleton += (minute_ == eTwoDigit) ? u"mm" : u"m";
  }
  if (!second_.empty()) {
    skeleton += (second_ == eTwoDigit) ? u"ss" : u"s";
  }

  UErrorCode dtpErr = U_ZERO_ERROR;
  UDateTimePatternGenerator *dtpGenerator =
      icu_->udatpg_open(locale8_.c_str(), &dtpErr);
  if (!dtpGenerator)
    return nullptr;

  UErrorCode bpErr = U_ZERO_ERROR;
  int32_t patternLength = icu_->udatpg_getBestPatternWithOptions(
      dtpGenerator,
      reinterpret_cast<const UChar *>(skeleton.data()),
      -1, kUDATPG_MATCH_ALL_FIELDS_LENGTH,
      nullptr, 0, &bpErr);

  std::u16string bestpattern;
  if (patternLength > 0) {
    bestpattern.resize(patternLength);
    bpErr = U_ZERO_ERROR;
    patternLength = icu_->udatpg_getBestPatternWithOptions(
        dtpGenerator,
        reinterpret_cast<const UChar *>(skeleton.data()),
        static_cast<int32_t>(skeleton.length()),
        kUDATPG_MATCH_ALL_FIELDS_LENGTH,
        reinterpret_cast<UChar *>(&bestpattern[0]),
        patternLength, &bpErr);
  }
  icu_->udatpg_close(dtpGenerator);

  UErrorCode openErr = U_ZERO_ERROR;
  UDateFormat *dtf = nullptr;
  if (!timeZone_.empty()) {
    dtf = icu_->udat_open(
        kUDAT_PATTERN, kUDAT_PATTERN, locale8_.c_str(),
        reinterpret_cast<const UChar *>(timeZone_.c_str()),
        static_cast<int32_t>(timeZone_.length()),
        reinterpret_cast<const UChar *>(bestpattern.data()),
        patternLength, &openErr);
  } else {
    dtf = icu_->udat_open(
        kUDAT_PATTERN, kUDAT_PATTERN, locale8_.c_str(),
        nullptr, -1,
        reinterpret_cast<const UChar *>(bestpattern.data()),
        patternLength, &openErr);
  }
  return dtf;
}

std::vector<Part> DateTimeFormatWindows::formatToParts(
    double jsTimeValue) noexcept {
  if (!icu_ || !dtf_)
    return {};

  // Open a field position iterator.
  UErrorCode fpiErr = U_ZERO_ERROR;
  UFieldPositionIterator *fpi = icu_->ufieldpositer_open(&fpiErr);
  if (U_FAILURE(fpiErr) || !fpi) {
    // Fallback: return single part with the full formatted string.
    Part part;
    part[u"type"] = u"literal";
    part[u"value"] = format(jsTimeValue);
    return {part};
  }

  // Format with field positions.
  UChar buf[256];
  UErrorCode st = U_ZERO_ERROR;
  int32_t len = icu_->udat_formatForFields(
      dtf_, jsTimeValue, buf, 256, fpi, &st);
  std::u16string formatted;
  if (st == U_BUFFER_OVERFLOW_ERROR) {
    // Retry with larger buffer.
    formatted.resize(len);
    icu_->ufieldpositer_close(fpi);
    fpi = nullptr;
    fpiErr = U_ZERO_ERROR;
    fpi = icu_->ufieldpositer_open(&fpiErr);
    if (U_FAILURE(fpiErr) || !fpi) {
      Part part;
      part[u"type"] = u"literal";
      part[u"value"] = format(jsTimeValue);
      return {part};
    }
    st = U_ZERO_ERROR;
    len = icu_->udat_formatForFields(
        dtf_, jsTimeValue,
        reinterpret_cast<UChar *>(formatted.data()),
        len + 1, fpi, &st);
  } else if (U_SUCCESS(st)) {
    formatted.assign(reinterpret_cast<const char16_t *>(buf), len);
  }
  if (U_FAILURE(st)) {
    icu_->ufieldpositer_close(fpi);
    return {};
  }

  // Collect field ranges.
  struct FieldRange {
    int32_t begin;
    int32_t end;
    int32_t type;
  };
  std::vector<FieldRange> fields;
  int32_t beginIdx = 0, endIdx = 0;
  int32_t fieldType = icu_->ufieldpositer_next(fpi, &beginIdx, &endIdx);
  while (fieldType >= 0) {
    fields.push_back({beginIdx, endIdx, fieldType});
    fieldType = icu_->ufieldpositer_next(fpi, &beginIdx, &endIdx);
  }
  icu_->ufieldpositer_close(fpi);

  // Sort fields by begin position.
  std::sort(
      fields.begin(),
      fields.end(),
      [](const FieldRange &a, const FieldRange &b) {
        return a.begin < b.begin;
      });

  // Build parts, filling gaps with "literal" parts.
  std::vector<Part> parts;
  int32_t pos = 0;
  for (const auto &f : fields) {
    if (f.begin > pos) {
      Part literal;
      literal[u"type"] = u"literal";
      literal[u"value"] = formatted.substr(pos, f.begin - pos);
      parts.push_back(std::move(literal));
    }
    Part part;
    part[u"type"] = dateFieldToPartType(f.type);
    part[u"value"] = formatted.substr(f.begin, f.end - f.begin);
    parts.push_back(std::move(part));
    pos = f.end;
  }
  if (pos < static_cast<int32_t>(formatted.size())) {
    Part literal;
    literal[u"type"] = u"literal";
    literal[u"value"] = formatted.substr(pos);
    parts.push_back(std::move(literal));
  }

  return parts;
}

std::vector<Part> DateTimeFormat::formatToParts(double x) noexcept {
  if (usesIcu_)
    return static_cast<DateTimeFormatWindows *>(this)->formatToParts(x);
  return static_cast<winglob::DateTimeFormatWinGlob *>(this)->formatToParts(x);
}

// ============================================================================
// NumberFormat (Phase 2: ICU-backed implementation)
// ============================================================================

namespace {

class NumberFormatWindows : public NumberFormat {
 public:
  NumberFormatWindows() = default;
  ~NumberFormatWindows() override;

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const hermes_icu_vtable *icu,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

  Options resolvedOptions() noexcept;
  std::u16string format(double number) noexcept;
  std::vector<Part> formatToParts(double number) noexcept;

 private:
  const hermes_icu_vtable *icu_ = nullptr;
  UNumberFormat *nf_ = nullptr;
  UNumberFormatter *unumf_ = nullptr;
  UFormattedNumber *unumfResult_ = nullptr;

  std::u16string locale_;
  std::u16string style_;
  std::u16string currency_;
  std::u16string currencyDisplay_;
  std::u16string unit_;
  std::u16string unitDisplay_;
  std::u16string notation_;
  std::u16string signDisplay_;
  std::u16string compactDisplay_;
  std::u16string numberingSystem_;
  std::u16string currencySign_;
  int minimumIntegerDigits_ = 1;
  int minimumFractionDigits_ = -1;
  int maximumFractionDigits_ = -1;
  int minimumSignificantDigits_ = -1;
  int maximumSignificantDigits_ = -1;
  bool useGrouping_ = true;
  bool useSignificantDigits_ = false;
};

NumberFormatWindows::~NumberFormatWindows() {
  if (nf_ && icu_) {
    icu_->unum_close(nf_);
  }
  if (unumfResult_ && icu_) {
    icu_->unumf_closeResult(unumfResult_);
  }
  if (unumf_ && icu_) {
    icu_->unumf_close(unumf_);
  }
}

vm::ExecutionStatus NumberFormatWindows::initialize(
    vm::Runtime &runtime,
    const hermes_icu_vtable *icu,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  icu_ = icu;
  if (!icu_) {
    (void)runtime.raiseRangeError("NumberFormat requires ICU support");
    return vm::ExecutionStatus::EXCEPTION;
  }

  auto requestedLocales = canonicalizeLocaleList(runtime, icu_, locales);
  if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;

  if (!requestedLocales.getValue().empty()) {
    locale_ = requestedLocales.getValue().front();
  } else {
    const char *defLoc = icu_->uloc_getDefault();
    if (defLoc)
      locale_ = toUTF16ASCII(convertICUtoBCP47Locale(icu_, defLoc));
    else
      locale_ = u"en-US";
  }

  // localeMatcher (ECMA-402 requires reading + validation)
  static const std::vector<std::u16string> validMatchers = {
      u"lookup", u"best fit"};
  auto matcherRes = getOptionString(
      runtime, options, u"localeMatcher", validMatchers, u"best fit");
  if (LLVM_UNLIKELY(matcherRes == vm::ExecutionStatus::EXCEPTION))
    return matcherRes.getStatus();

  // numberingSystem (BCP47 unicode extension type format)
  auto nuRes = getOptionString(runtime, options, u"numberingSystem", {}, {});
  if (LLVM_UNLIKELY(nuRes == vm::ExecutionStatus::EXCEPTION))
    return nuRes.getStatus();
  // Validate if the option was explicitly provided (even if empty).
  if (options.count(u"numberingSystem") &&
      !isUnicodeExtensionType(nuRes.getValue())) {
    return runtime.raiseRangeError("Invalid numberingSystem value");
  }
  numberingSystem_ = nuRes.getValue();

  // style
  static const std::vector<std::u16string> validStyles = {
      u"decimal", u"percent", u"currency", u"unit"};
  auto styleRes =
      getOptionString(runtime, options, u"style", validStyles, u"decimal");
  if (LLVM_UNLIKELY(styleRes == vm::ExecutionStatus::EXCEPTION))
    return styleRes.getStatus();
  style_ = styleRes.getValue();

  // currency (ECMA-402 steps 10-12)
  auto currencyRes = getOptionString(runtime, options, u"currency", {}, {});
  if (LLVM_UNLIKELY(currencyRes == vm::ExecutionStatus::EXCEPTION))
    return currencyRes.getStatus();
  currency_ = currencyRes.getValue();
  bool hasCurrency = options.find(u"currency") != options.end();
  // Step 11: currency not provided + style:currency → TypeError
  if (!hasCurrency && style_ == u"currency") {
    return runtime.raiseTypeError(
        "Currency code is required with currency style");
  }
  // Step 12: currency provided → must be well-formed (exactly 3 ASCII letters)
  if (hasCurrency) {
    bool wellFormed = (currency_.size() == 3);
    if (wellFormed) {
      for (auto c : currency_) {
        if (!((c >= u'A' && c <= u'Z') || (c >= u'a' && c <= u'z'))) {
          wellFormed = false;
          break;
        }
      }
    }
    if (!wellFormed) {
      return runtime.raiseRangeError("Invalid currency code");
    }
  }

  // currencyDisplay
  static const std::vector<std::u16string> validCurrencyDisplays = {
      u"code", u"symbol", u"narrowSymbol", u"name"};
  auto currencyDisplayRes = getOptionString(
      runtime, options, u"currencyDisplay", validCurrencyDisplays, u"symbol");
  if (LLVM_UNLIKELY(currencyDisplayRes == vm::ExecutionStatus::EXCEPTION))
    return currencyDisplayRes.getStatus();
  currencyDisplay_ = currencyDisplayRes.getValue();

  // unit
  auto unitRes = getOptionString(runtime, options, u"unit", {}, {});
  if (LLVM_UNLIKELY(unitRes == vm::ExecutionStatus::EXCEPTION))
    return unitRes.getStatus();
  unit_ = unitRes.getValue();
  if (style_ == u"unit" && unit_.empty()) {
    return runtime.raiseTypeError("Unit is required with unit style");
  }
  // Unit identifier validation (ECMA-402 §6.5)
  if (!unit_.empty() && !isWellFormedUnitIdentifier(unit_)) {
    return runtime.raiseRangeError("Invalid unit identifier");
  }

  // unitDisplay
  static const std::vector<std::u16string> validUnitDisplays = {
      u"short", u"narrow", u"long"};
  auto udRes = getOptionString(
      runtime, options, u"unitDisplay", validUnitDisplays, u"short");
  if (LLVM_UNLIKELY(udRes == vm::ExecutionStatus::EXCEPTION))
    return udRes.getStatus();
  unitDisplay_ = udRes.getValue();

  // notation
  static const std::vector<std::u16string> validNotations = {
      u"standard", u"scientific", u"engineering", u"compact"};
  auto notationRes = getOptionString(
      runtime, options, u"notation", validNotations, u"standard");
  if (LLVM_UNLIKELY(notationRes == vm::ExecutionStatus::EXCEPTION))
    return notationRes.getStatus();
  notation_ = notationRes.getValue();

  // signDisplay
  static const std::vector<std::u16string> validSignDisplays = {
      u"auto", u"never", u"always", u"exceptZero"};
  auto signRes = getOptionString(
      runtime, options, u"signDisplay", validSignDisplays, u"auto");
  if (LLVM_UNLIKELY(signRes == vm::ExecutionStatus::EXCEPTION))
    return signRes.getStatus();
  signDisplay_ = signRes.getValue();

  // compactDisplay
  static const std::vector<std::u16string> validCompactDisplays = {
      u"short", u"long"};
  auto compactRes = getOptionString(
      runtime, options, u"compactDisplay", validCompactDisplays, u"short");
  if (LLVM_UNLIKELY(compactRes == vm::ExecutionStatus::EXCEPTION))
    return compactRes.getStatus();
  compactDisplay_ = compactRes.getValue();

  // currencySign
  static const std::vector<std::u16string> validCurrencySigns = {
      u"standard", u"accounting"};
  auto csRes = getOptionString(
      runtime, options, u"currencySign", validCurrencySigns, u"standard");
  if (LLVM_UNLIKELY(csRes == vm::ExecutionStatus::EXCEPTION))
    return csRes.getStatus();
  currencySign_ = csRes.getValue();

  std::string localeICU = convertBCP47toICULocale(icu_, locale_);
  // Apply numberingSystem to ICU locale if explicitly set.
  if (!numberingSystem_.empty()) {
    std::string nuUtf8(numberingSystem_.begin(), numberingSystem_.end());
    if (localeICU.find('@') != std::string::npos)
      localeICU += ";numbers=" + nuUtf8;
    else
      localeICU += "@numbers=" + nuUtf8;
  }

  bool useUnitV2 = false;
  if (style_ == u"unit" && icu_->unumf_openForSkeletonAndLocale &&
      icu_->unumf_openResult) {
    // Try ICU Number Formatter v2 for unit formatting.
    // Build skeleton using ICU 68+ "unit/" syntax which takes ECMA-402
    // unit identifiers directly (e.g., "unit/kilometer-per-hour").
    std::string skeleton = "unit/";

    // Convert unit_ to UTF-8 for the skeleton
    std::string unitUtf8(unit_.begin(), unit_.end());
    skeleton += unitUtf8;

    // unitDisplay: "short" → "short", "narrow" → "narrow", "long" → "full-name"
    if (unitDisplay_ == u"long")
      skeleton += " unit-width-full-name";
    else if (unitDisplay_ == u"narrow")
      skeleton += " unit-width-narrow";
    else
      skeleton += " unit-width-short";

    // Convert skeleton to UTF-16 for ICU API
    std::u16string skeleton16(skeleton.begin(), skeleton.end());
    UErrorCode err = U_ZERO_ERROR;
    unumf_ = icu_->unumf_openForSkeletonAndLocale(
        reinterpret_cast<const UChar *>(skeleton16.data()),
        static_cast<int32_t>(skeleton16.size()),
        localeICU.c_str(),
        &err);
    if (U_SUCCESS(err) && unumf_) {
      err = U_ZERO_ERROR;
      unumfResult_ = icu_->unumf_openResult(&err);
      if (U_SUCCESS(err) && unumfResult_) {
        useUnitV2 = true;
        // Also create a basic decimal formatter for resolvedOptions defaults
        err = U_ZERO_ERROR;
        nf_ = icu_->unum_open(
            UNUM_DECIMAL, nullptr, 0, localeICU.c_str(), nullptr, &err);
      } else {
        // Clean up partial state.
        if (icu_->unumf_close)
          icu_->unumf_close(unumf_);
        unumf_ = nullptr;
        unumfResult_ = nullptr;
      }
    } else {
      // Unit skeleton not supported by this ICU provider (e.g. older system
      // icu.dll). Fall through to legacy decimal formatter.
      unumf_ = nullptr;
    }
  }

  if (!useUnitV2) {
    // Legacy ICU number format API (non-unit styles, or unit fallback)
    int32_t icuStyle = UNUM_DECIMAL;
    if (style_ == u"percent")
      icuStyle = UNUM_PERCENT;
    else if (style_ == u"currency") {
      if (currencyDisplay_ == u"name")
        icuStyle = UNUM_CURRENCY_PLURAL;
      else
        icuStyle = UNUM_CURRENCY;
    }

    UErrorCode err = U_ZERO_ERROR;
    nf_ = icu_->unum_open(
        icuStyle, nullptr, 0, localeICU.c_str(), nullptr, &err);
    if (U_FAILURE(err) || !nf_) {
      return runtime.raiseRangeError("Failed to create NumberFormat");
    }
  }

  // Set rounding mode to HALF_UP (ECMA-402 default for legacy NumberFormat).
  // ICU defaults to HALF_EVEN but test262 expects HALF_UP.
  constexpr int32_t UNUM_ROUND_HALFUP = 6;
  if (nf_) {
    icu_->unum_setAttribute(nf_, UNUM_ROUNDING_MODE, UNUM_ROUND_HALFUP);
  }

  // Set currency code
  if (style_ == u"currency" && !currency_.empty() && nf_) {
    std::u16string currUpper;
    for (auto c : currency_) {
      currUpper += (c >= u'a' && c <= u'z') ? (c - u'a' + u'A') : c;
    }
    {
      UErrorCode stErr = U_ZERO_ERROR;
      icu_->unum_setTextAttribute(
          nf_,
          UNUM_CURRENCY_CODE,
          reinterpret_cast<const UChar *>(currUpper.data()),
          static_cast<int32_t>(currUpper.size()),
          &stErr);
    }
  }

  // useGrouping
  auto useGroupingIt = options.find(u"useGrouping");
  if (useGroupingIt != options.end() && useGroupingIt->second.isBool()) {
    useGrouping_ = useGroupingIt->second.getBool();
  }
  if (!useGrouping_) {
    icu_->unum_setAttribute(nf_, UNUM_GROUPING_USED, 0);
  }

  // minimumIntegerDigits
  auto minIntRes = getNumberOption(
      runtime, options, u"minimumIntegerDigits", 1, 21, 1);
  if (LLVM_UNLIKELY(minIntRes == vm::ExecutionStatus::EXCEPTION))
    return minIntRes.getStatus();
  minimumIntegerDigits_ = minIntRes.getValue().value_or(1);
  if (minimumIntegerDigits_ > 1) {
    icu_->unum_setAttribute(nf_, UNUM_MIN_INTEGER_DIGITS, minimumIntegerDigits_);
  }

  // Get currency default fraction digits per ECMA-402 §15.5.3.
  // ECMA-402 requires ISO 4217 values. ICU CLDR data may differ for some
  // currencies, so we apply overrides where needed.
  int currencyDefaultMinFD = 2;
  int currencyDefaultMaxFD = 2;
  if (style_ == u"currency" && nf_) {
    currencyDefaultMinFD =
        icu_->unum_getAttribute(nf_, UNUM_MIN_FRACTION_DIGITS);
    currencyDefaultMaxFD =
        icu_->unum_getAttribute(nf_, UNUM_MAX_FRACTION_DIGITS);

    // ISO 4217 overrides where CLDR data disagrees with the standard.
    // ECMA-402 §15.5.3 requires ISO 4217 values, but ICU CLDR may use 0
    // for currencies whose fractional units are rarely used in practice.
    std::u16string currUpper;
    for (auto c : currency_) {
      currUpper += (c >= u'a' && c <= u'z') ? (c - u'a' + u'A') : c;
    }
    // Map of currencies where CLDR says 0 but ISO 4217 says 2 or 3.
    static const std::unordered_map<std::u16string, int> iso4217Overrides = {
        {u"AFN", 2}, {u"ALL", 2}, {u"COP", 2}, {u"HUF", 2},
        {u"IDR", 2}, {u"IQD", 3}, {u"IRR", 2}, {u"KPW", 2},
        {u"LAK", 2}, {u"LBP", 2}, {u"MGA", 2}, {u"MMK", 2},
        {u"MRO", 2}, {u"PKR", 2}, {u"SLL", 2}, {u"SOS", 2},
        {u"STD", 2}, {u"SYP", 2}, {u"YER", 2},
    };
    auto overrideIt = iso4217Overrides.find(currUpper);
    if (overrideIt != iso4217Overrides.end()) {
      int fd = overrideIt->second;
      currencyDefaultMinFD = fd;
      currencyDefaultMaxFD = fd;
      icu_->unum_setAttribute(nf_, UNUM_MIN_FRACTION_DIGITS, fd);
      icu_->unum_setAttribute(nf_, UNUM_MAX_FRACTION_DIGITS, fd);
    }
  }

  // significantDigits
  auto minSigRes = getNumberOption(
      runtime, options, u"minimumSignificantDigits", 1, 21, {});
  if (LLVM_UNLIKELY(minSigRes == vm::ExecutionStatus::EXCEPTION))
    return minSigRes.getStatus();
  auto maxSigRes = getNumberOption(
      runtime, options, u"maximumSignificantDigits", 1, 21, {});
  if (LLVM_UNLIKELY(maxSigRes == vm::ExecutionStatus::EXCEPTION))
    return maxSigRes.getStatus();

  if (minSigRes.getValue().has_value() || maxSigRes.getValue().has_value()) {
    useSignificantDigits_ = true;
    int minSig = minSigRes.getValue().value_or(1);
    int maxSig = maxSigRes.getValue().value_or(21);
    if (minSig > maxSig) {
      return runtime.raiseRangeError(
          "maximumSignificantDigits value is invalid.");
    }
    minimumSignificantDigits_ = minSig;
    maximumSignificantDigits_ = maxSig;
    icu_->unum_setAttribute(nf_, UNUM_SIGNIFICANT_DIGITS_USED, 1);
    icu_->unum_setAttribute(nf_, UNUM_MIN_SIGNIFICANT_DIGITS, minSig);
    icu_->unum_setAttribute(nf_, UNUM_MAX_SIGNIFICANT_DIGITS, maxSig);
  } else {
    int defaultMinFD = 0;
    int defaultMaxFD = 3;
    if (style_ == u"currency") {
      defaultMinFD = currencyDefaultMinFD;
      defaultMaxFD = currencyDefaultMaxFD;
    } else if (style_ == u"percent") {
      defaultMinFD = 0;
      defaultMaxFD = 0;
    }

    auto minFDRes = getNumberOption(
        runtime, options, u"minimumFractionDigits", 0, 20, {});
    if (LLVM_UNLIKELY(minFDRes == vm::ExecutionStatus::EXCEPTION))
      return minFDRes.getStatus();
    auto maxFDRes = getNumberOption(
        runtime, options, u"maximumFractionDigits", 0, 20, {});
    if (LLVM_UNLIKELY(maxFDRes == vm::ExecutionStatus::EXCEPTION))
      return maxFDRes.getStatus();

    int minFD = minFDRes.getValue().value_or(defaultMinFD);
    int maxFD = maxFDRes.getValue().value_or(
        defaultMaxFD > minFD ? defaultMaxFD : minFD);
    // Per ECMA-402: if only max is set, clamp min to at most max.
    if (!minFDRes.getValue().has_value() && minFD > maxFD) {
      minFD = maxFD;
    }
    if (minFD > maxFD) {
      return runtime.raiseRangeError(
          "minimumFractionDigits is greater than maximumFractionDigits");
    }
    minimumFractionDigits_ = minFD;
    maximumFractionDigits_ = maxFD;
    icu_->unum_setAttribute(nf_, UNUM_MIN_FRACTION_DIGITS, minFD);
    icu_->unum_setAttribute(nf_, UNUM_MAX_FRACTION_DIGITS, maxFD);
  }

  return vm::ExecutionStatus::RETURNED;
}

Options NumberFormatWindows::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(locale_));
  options.emplace(u"style", Option(style_));
  if (style_ == u"currency") {
    std::u16string currUpper;
    for (auto c : currency_) {
      currUpper += (c >= u'a' && c <= u'z') ? (c - u'a' + u'A') : c;
    }
    options.emplace(u"currency", Option(currUpper));
    options.emplace(u"currencyDisplay", Option(currencyDisplay_));
  }
  if (style_ == u"unit" && !unit_.empty()) {
    options.emplace(u"unit", Option(unit_));
    options.emplace(u"unitDisplay", Option(unitDisplay_));
  }
  if (style_ == u"currency") {
    options.emplace(u"currencySign", Option(currencySign_));
  }
  if (!numberingSystem_.empty()) {
    options.emplace(u"numberingSystem", Option(numberingSystem_));
  } else {
    options.emplace(u"numberingSystem", Option(std::u16string(u"latn")));
  }
  options.emplace(u"notation", Option(notation_));
  if (notation_ == u"compact") {
    options.emplace(u"compactDisplay", Option(compactDisplay_));
  }
  options.emplace(u"signDisplay", Option(signDisplay_));
  options.emplace(u"useGrouping", Option(useGrouping_));
  options.emplace(
      u"minimumIntegerDigits",
      Option(static_cast<double>(minimumIntegerDigits_)));
  if (useSignificantDigits_) {
    options.emplace(
        u"minimumSignificantDigits",
        Option(static_cast<double>(minimumSignificantDigits_)));
    options.emplace(
        u"maximumSignificantDigits",
        Option(static_cast<double>(maximumSignificantDigits_)));
  } else {
    options.emplace(
        u"minimumFractionDigits",
        Option(static_cast<double>(minimumFractionDigits_)));
    options.emplace(
        u"maximumFractionDigits",
        Option(static_cast<double>(maximumFractionDigits_)));
  }
  return options;
}

std::u16string NumberFormatWindows::format(double number) noexcept {
  if (!icu_) {
    auto s = std::to_string(number);
    return std::u16string(s.begin(), s.end());
  }

  // Normalize NaN to positive NaN. Hermes NaN-boxing may set the sign bit
  // on NaN values, causing ICU to format as "-NaN" instead of "NaN".
  if (std::isnan(number)) {
    number = std::numeric_limits<double>::quiet_NaN();
  }

  // Use Number Formatter v2 for unit formatting.
  if (unumf_ && unumfResult_) {
    UErrorCode err = U_ZERO_ERROR;
    icu_->unumf_formatDouble(unumf_, number, unumfResult_, &err);
    if (U_SUCCESS(err)) {
      UChar buf[128];
      err = U_ZERO_ERROR;
      int32_t len = icu_->unumf_resultToString(
          unumfResult_, buf, 128, &err);
      if (U_SUCCESS(err) && len > 0 && len <= 128) {
        return std::u16string(
            reinterpret_cast<const char16_t *>(buf), len);
      }
      if (err == U_BUFFER_OVERFLOW_ERROR && len > 0) {
        std::u16string result(len + 1, u' ');
        err = U_ZERO_ERROR;
        icu_->unumf_resultToString(
            unumfResult_,
            reinterpret_cast<UChar *>(&result[0]),
            len + 1, &err);
        result.resize(len);
        return result;
      }
    }
    // Fallback to plain number
    auto s = std::to_string(number);
    return std::u16string(s.begin(), s.end());
  }

  if (!nf_) {
    auto s = std::to_string(number);
    return std::u16string(s.begin(), s.end());
  }

  // Use a stack buffer for the common case to avoid two-pass.
  UChar smallBuf[128];
  UErrorCode err = U_ZERO_ERROR;
  int32_t len = icu_->unum_formatDouble(
      nf_, number, smallBuf, 128, nullptr, &err);
  if (U_SUCCESS(err) && len > 0 && len <= 128) {
    return std::u16string(
        reinterpret_cast<const char16_t *>(smallBuf), len);
  }
  if (err == U_BUFFER_OVERFLOW_ERROR && len > 0) {
    std::u16string result(len + 1, u' ');
    err = U_ZERO_ERROR;
    icu_->unum_formatDouble(
        nf_, number,
        reinterpret_cast<UChar *>(&result[0]), len + 1, nullptr, &err);
    result.resize(len);
    return result;
  }
  auto s = std::to_string(number);
  return std::u16string(s.begin(), s.end());
}

std::vector<Part> NumberFormatWindows::formatToParts(double number) noexcept {
  if (!icu_ || !nf_)
    return {};

  // Normalize NaN to positive NaN (same as format()).
  if (std::isnan(number)) {
    number = std::numeric_limits<double>::quiet_NaN();
  }

  // Open a field position iterator.
  UErrorCode fpiErr = U_ZERO_ERROR;
  UFieldPositionIterator *fpi = icu_->ufieldpositer_open(&fpiErr);
  if (U_FAILURE(fpiErr) || !fpi) {
    // Fallback: return single "integer" part with the full formatted string.
    Part part;
    part[u"type"] = u"integer";
    part[u"value"] = format(number);
    return {part};
  }

  // Format with field positions.
  UChar buf[128];
  UErrorCode st = U_ZERO_ERROR;
  int32_t len = icu_->unum_formatDoubleForFields(
      nf_, number, buf, 128, fpi, &st);
  std::u16string formatted;
  if (st == U_BUFFER_OVERFLOW_ERROR) {
    formatted.resize(len);
    icu_->ufieldpositer_close(fpi);
    fpi = nullptr;
    fpiErr = U_ZERO_ERROR;
    fpi = icu_->ufieldpositer_open(&fpiErr);
    if (U_FAILURE(fpiErr) || !fpi) {
      Part part;
      part[u"type"] = u"integer";
      part[u"value"] = format(number);
      return {part};
    }
    st = U_ZERO_ERROR;
    len = icu_->unum_formatDoubleForFields(
        nf_, number,
        reinterpret_cast<UChar *>(formatted.data()),
        len + 1, fpi, &st);
  } else if (U_SUCCESS(st)) {
    formatted.assign(reinterpret_cast<const char16_t *>(buf), len);
  }
  if (U_FAILURE(st)) {
    icu_->ufieldpositer_close(fpi);
    return {};
  }

  // Collect field ranges.
  struct FieldRange {
    int32_t begin;
    int32_t end;
    int32_t type;
  };
  std::vector<FieldRange> fields;
  int32_t beginIdx = 0, endIdx = 0;
  int32_t fieldType = icu_->ufieldpositer_next(fpi, &beginIdx, &endIdx);
  while (fieldType >= 0) {
    fields.push_back({beginIdx, endIdx, fieldType});
    fieldType = icu_->ufieldpositer_next(fpi, &beginIdx, &endIdx);
  }
  icu_->ufieldpositer_close(fpi);

  // Sort by begin position.
  std::sort(
      fields.begin(),
      fields.end(),
      [](const FieldRange &a, const FieldRange &b) {
        return a.begin < b.begin;
      });

  // Build parts, filling gaps with "literal" parts.
  std::vector<Part> parts;
  int32_t pos = 0;
  for (const auto &f : fields) {
    if (f.begin > pos) {
      Part literal;
      literal[u"type"] = u"literal";
      literal[u"value"] = formatted.substr(pos, f.begin - pos);
      parts.push_back(std::move(literal));
    }
    Part part;
    part[u"type"] = numberFieldToPartType(f.type);
    part[u"value"] = formatted.substr(f.begin, f.end - f.begin);
    parts.push_back(std::move(part));
    pos = f.end;
  }
  if (pos < static_cast<int32_t>(formatted.size())) {
    Part literal;
    literal[u"type"] = u"literal";
    literal[u"value"] = formatted.substr(pos);
    parts.push_back(std::move(literal));
  }

  return parts;
}

} // namespace

NumberFormat::NumberFormat() = default;
NumberFormat::~NumberFormat() = default;

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  const hermes_icu_vtable *icu = getIcuVtableForRuntime(runtime);
  if (!icu) {
    auto requestedLocales = winglob::getCanonicalLocales(runtime, locales);
    if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION))
      return vm::ExecutionStatus::EXCEPTION;
    return supportedLocales(
        runtime,
        winglob::getAvailableLocales(),
        requestedLocales.getValue(),
        options);
  }

  auto availableLocales = getAvailableLocalesU16(runtime, icu);
  auto requestedLocales = getCanonicalLocales(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  return supportedLocales(
      runtime, availableLocales, requestedLocales.getValue(), options);
}

vm::CallResult<std::unique_ptr<NumberFormat>> NumberFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  const hermes_icu_vtable *icu = getIcuVtableForRuntime(runtime);
  if (icu) {
    auto instance = std::make_unique<NumberFormatWindows>();
    instance->usesIcu_ = true;
    if (LLVM_UNLIKELY(
            instance->initialize(runtime, icu, locales, options) ==
            vm::ExecutionStatus::EXCEPTION)) {
      return vm::ExecutionStatus::EXCEPTION;
    }
    return instance;
  }
  auto instance = std::make_unique<winglob::NumberFormatWinGlob>();
  if (LLVM_UNLIKELY(
          instance->initialize(runtime, locales, options) ==
          vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return instance;
}

Options NumberFormat::resolvedOptions() noexcept {
  if (usesIcu_)
    return static_cast<NumberFormatWindows *>(this)->resolvedOptions();
  return static_cast<winglob::NumberFormatWinGlob *>(this)->resolvedOptions();
}

std::u16string NumberFormat::format(double number) noexcept {
  if (usesIcu_)
    return static_cast<NumberFormatWindows *>(this)->format(number);
  return static_cast<winglob::NumberFormatWinGlob *>(this)->format(number);
}

std::vector<Part> NumberFormat::formatToParts(double number) noexcept {
  if (usesIcu_)
    return static_cast<NumberFormatWindows *>(this)->formatToParts(number);
  return static_cast<winglob::NumberFormatWinGlob *>(this)->formatToParts(
      number);
}

} // namespace platform_intl
} // namespace hermes
