/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * WinGlob Intl fallback - Intl using Windows NLS APIs.
 *
 * Provides functional Intl when no ICU is available, using APIs
 * available since Windows Vista/7:
 *   - ResolveLocaleName / EnumSystemLocalesEx for locale resolution
 *   - LCMapStringEx for case conversion
 *   - CompareStringEx for collation
 *   - GetDateFormatEx / GetTimeFormatEx for date formatting
 *   - GetNumberFormatEx / GetCurrencyFormatEx for number formatting
 *
 * Not fully ECMA-402 compliant but functional for common usage.
 */

#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "PlatformIntlWinGlob.h"
#include "WinIntlUtils.h"

#include "hermes/Platform/Intl/BCP47Parser.h"

#include <windows.h>
#include <winnls.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <mutex>

namespace hermes {
namespace platform_intl {
namespace winglob {

// ============================================================================
// Locale infrastructure (internal helpers)
// ============================================================================

namespace {

/// Validate and normalize a BCP47 locale name using Windows NLS.
/// Returns empty string if locale is not recognized by Windows.
std::u16string resolveWindowsLocale(const std::u16string &bcp47) {
  wchar_t resolved[LOCALE_NAME_MAX_LENGTH] = {0};
  int ret = ResolveLocaleName(
      reinterpret_cast<LPCWSTR>(bcp47.c_str()),
      resolved,
      LOCALE_NAME_MAX_LENGTH);
  if (ret <= 0 || resolved[0] == L'\0')
    return {};
  return std::u16string(
      reinterpret_cast<const char16_t *>(resolved), ret - 1);
}

/// Get locale info string for a given locale and type.
std::u16string getLocaleInfo(
    const std::u16string &locale,
    LCTYPE type) {
  int size = GetLocaleInfoEx(
      reinterpret_cast<LPCWSTR>(locale.c_str()), type, nullptr, 0);
  if (size <= 0)
    return {};
  std::u16string result(size, u'\0');
  GetLocaleInfoEx(
      reinterpret_cast<LPCWSTR>(locale.c_str()),
      type,
      reinterpret_cast<LPWSTR>(&result[0]),
      size);
  // Remove trailing null.
  if (!result.empty() && result.back() == u'\0')
    result.pop_back();
  return result;
}

/// Get locale info as integer for a given locale and type.
int getLocaleInfoInt(const std::u16string &locale, LCTYPE type) {
  DWORD value = 0;
  int ret = GetLocaleInfoEx(
      reinterpret_cast<LPCWSTR>(locale.c_str()),
      type | LOCALE_RETURN_NUMBER,
      reinterpret_cast<LPWSTR>(&value),
      sizeof(value) / sizeof(wchar_t));
  return ret > 0 ? static_cast<int>(value) : 0;
}

/// Callback for EnumSystemLocalesEx.
static BOOL CALLBACK enumLocaleCallback(
    LPWSTR name,
    DWORD /*flags*/,
    LPARAM lParam) {
  auto *vec = reinterpret_cast<std::vector<std::u16string> *>(lParam);
  if (name && name[0]) {
    // Windows locale names are BCP47 (e.g., "en-US").
    vec->push_back(
        std::u16string(reinterpret_cast<const char16_t *>(name)));
  }
  return TRUE;
}

} // anonymous namespace

const std::vector<std::u16string> &getAvailableLocales() {
  static const auto *avail = [] {
    auto *v = new std::vector<std::u16string>();
    EnumSystemLocalesEx(
        enumLocaleCallback,
        LOCALE_ALL,
        reinterpret_cast<LPARAM>(v),
        nullptr);
    return v;
  }();
  return *avail;
}

std::u16string getDefaultLocale() {
  wchar_t buf[LOCALE_NAME_MAX_LENGTH] = {0};
  int ret = GetUserDefaultLocaleName(buf, LOCALE_NAME_MAX_LENGTH);
  if (ret <= 0)
    return u"en-US";
  return std::u16string(
      reinterpret_cast<const char16_t *>(buf), ret - 1);
}

namespace {

/// Resolve locale from a list of requested locales.
/// Returns the first locale recognized by Windows, or the system default.
std::u16string resolveLocaleFromList(
    const std::vector<std::u16string> &locales) {
  for (const auto &locale : locales) {
    // Parse to extract just the base locale (strip Unicode extensions).
    auto parsed = ParsedLocaleIdentifier::parse(locale);
    if (!parsed)
      continue;

    // Build base locale string without extensions.
    std::u16string base;
    const auto &lang = parsed->languageIdentifier;
    base = lang.languageSubtag;
    if (!lang.scriptSubtag.empty())
      base += u"-" + lang.scriptSubtag;
    if (!lang.regionSubtag.empty())
      base += u"-" + lang.regionSubtag;

    auto resolved = resolveWindowsLocale(base);
    if (!resolved.empty())
      return resolved;
  }
  return getDefaultLocale();
}

} // anonymous namespace

// ============================================================================
// getCanonicalLocales
// ============================================================================

vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales) {
  std::vector<std::u16string> result;

  for (const auto &locale : locales) {
    // Validate via BCP47 parser (structural validation).
    auto parsed = ParsedLocaleIdentifier::parse(locale);
    if (!parsed) {
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid language tag: ") +
          vm::TwineChar16(locale.c_str()));
    }

    // Use the BCP47 parser's canonicalize which handles case normalization
    // and subtag ordering.
    auto canonicalized = parsed->canonicalize();

    // Deduplicate.
    if (std::find(result.begin(), result.end(), canonicalized) ==
        result.end()) {
      result.push_back(std::move(canonicalized));
    }
  }

  return result;
}

// ============================================================================
// toLocaleLowerCase / toLocaleUpperCase
// ============================================================================

vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  if (str.empty())
    return std::u16string{};

  // Resolve locale for case mapping.
  LPCWSTR localeName = LOCALE_NAME_USER_DEFAULT;
  std::u16string resolved;
  if (!locales.empty()) {
    resolved = resolveLocaleFromList(locales);
    if (!resolved.empty())
      localeName = reinterpret_cast<LPCWSTR>(resolved.c_str());
  }

  DWORD flags = LCMAP_LOWERCASE | LCMAP_LINGUISTIC_CASING;

  // Two-pass: first get size, then convert.
  int size = LCMapStringEx(
      localeName,
      flags,
      reinterpret_cast<LPCWSTR>(str.c_str()),
      static_cast<int>(str.length()),
      nullptr,
      0,
      nullptr,
      nullptr,
      0);
  if (size <= 0)
    return runtime.raiseRangeError("toLocaleLowerCase failed");

  std::u16string result(size, u'\0');
  int written = LCMapStringEx(
      localeName,
      flags,
      reinterpret_cast<LPCWSTR>(str.c_str()),
      static_cast<int>(str.length()),
      reinterpret_cast<LPWSTR>(&result[0]),
      size,
      nullptr,
      nullptr,
      0);
  if (written <= 0)
    return runtime.raiseRangeError("toLocaleLowerCase failed");
  result.resize(written);
  return result;
}

vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  if (str.empty())
    return std::u16string{};

  LPCWSTR localeName = LOCALE_NAME_USER_DEFAULT;
  std::u16string resolved;
  if (!locales.empty()) {
    resolved = resolveLocaleFromList(locales);
    if (!resolved.empty())
      localeName = reinterpret_cast<LPCWSTR>(resolved.c_str());
  }

  DWORD flags = LCMAP_UPPERCASE | LCMAP_LINGUISTIC_CASING;

  int size = LCMapStringEx(
      localeName,
      flags,
      reinterpret_cast<LPCWSTR>(str.c_str()),
      static_cast<int>(str.length()),
      nullptr,
      0,
      nullptr,
      nullptr,
      0);
  if (size <= 0)
    return runtime.raiseRangeError("toLocaleUpperCase failed");

  std::u16string result(size, u'\0');
  int written = LCMapStringEx(
      localeName,
      flags,
      reinterpret_cast<LPCWSTR>(str.c_str()),
      static_cast<int>(str.length()),
      reinterpret_cast<LPWSTR>(&result[0]),
      size,
      nullptr,
      nullptr,
      0);
  if (written <= 0)
    return runtime.raiseRangeError("toLocaleUpperCase failed");
  result.resize(written);
  return result;
}

// ============================================================================
// Collator
// ============================================================================

CollatorWinGlob::~CollatorWinGlob() = default;

vm::ExecutionStatus CollatorWinGlob::initialize(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // Parse usage option.
  static const std::vector<std::u16string> validUsageValues = {
      u"sort", u"search"};
  auto usageRes =
      getOptionString(runtime, options, u"usage", validUsageValues, u"sort");
  if (LLVM_UNLIKELY(usageRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  usage_ = std::move(usageRes.getValue());

  // Parse sensitivity option.
  static const std::vector<std::u16string> validSensitivityValues = {
      u"base", u"accent", u"case", u"variant"};
  std::u16string defaultSensitivity =
      (usage_ == u"sort") ? u"variant" : u"base";
  auto sensitivityRes = getOptionString(
      runtime,
      options,
      u"sensitivity",
      validSensitivityValues,
      defaultSensitivity);
  if (LLVM_UNLIKELY(sensitivityRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  sensitivity_ = std::move(sensitivityRes.getValue());

  // Parse ignorePunctuation option.
  ignorePunctuation_ =
      getOptionBool(runtime, options, u"ignorePunctuation", false)
          .value_or(false);

  // Parse numeric option.
  numeric_ =
      getOptionBool(runtime, options, u"numeric", false).value_or(false);

  // Parse caseFirst option.
  static const std::vector<std::u16string> validCaseFirstValues = {
      u"upper", u"lower", u"false"};
  auto caseFirstRes = getOptionString(
      runtime, options, u"caseFirst", validCaseFirstValues, u"false");
  if (LLVM_UNLIKELY(caseFirstRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  caseFirst_ = std::move(caseFirstRes.getValue());

  // Resolve locale.
  locale_ = resolveLocaleFromList(locales);

  // Build CompareStringEx flags.
  compareFlags_ = 0;
  if (sensitivity_ == u"base") {
    compareFlags_ |= NORM_IGNORECASE | NORM_IGNORENONSPACE;
  } else if (sensitivity_ == u"accent") {
    compareFlags_ |= NORM_IGNORECASE;
  } else if (sensitivity_ == u"case") {
    compareFlags_ |= NORM_IGNORENONSPACE;
  }
  // sensitivity == "variant" → no flags (default).

  if (numeric_) {
    compareFlags_ |= SORT_DIGITSASNUMBERS;
  }

  if (ignorePunctuation_) {
    compareFlags_ |= NORM_IGNORESYMBOLS;
  }

  return vm::ExecutionStatus::RETURNED;
}

Options CollatorWinGlob::resolvedOptions() noexcept {
  Options result;
  result.emplace(u"locale", Option(locale_));
  result.emplace(u"usage", Option(usage_));
  result.emplace(u"sensitivity", Option(sensitivity_));
  result.emplace(
      u"ignorePunctuation", Option(ignorePunctuation_));
  result.emplace(u"collation", Option(std::u16string(u"default")));
  result.emplace(u"numeric", Option(numeric_));
  result.emplace(u"caseFirst", Option(caseFirst_));
  return result;
}

double CollatorWinGlob::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  int result = CompareStringEx(
      reinterpret_cast<LPCWSTR>(locale_.c_str()),
      compareFlags_,
      reinterpret_cast<LPCWSTR>(x.c_str()),
      static_cast<int>(x.size()),
      reinterpret_cast<LPCWSTR>(y.c_str()),
      static_cast<int>(y.size()),
      nullptr,
      nullptr,
      0);
  if (result == CSTR_LESS_THAN)
    return -1;
  if (result == CSTR_GREATER_THAN)
    return 1;
  return 0; // CSTR_EQUAL or error
}

// ============================================================================
// NumberFormat
// ============================================================================

NumberFormatWinGlob::~NumberFormatWinGlob() = default;

vm::ExecutionStatus NumberFormatWinGlob::initialize(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // Parse style option.
  static const std::vector<std::u16string> validStyleValues = {
      u"decimal", u"percent", u"currency", u"unit"};
  auto styleRes =
      getOptionString(runtime, options, u"style", validStyleValues, u"decimal");
  if (LLVM_UNLIKELY(styleRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  style_ = std::move(styleRes.getValue());

  // Parse currency option (required for currency style).
  auto currencyIt = options.find(u"currency");
  if (currencyIt != options.end()) {
    currency_ = currencyIt->second.getString();
  } else if (style_ == u"currency") {
    (void)runtime.raiseTypeError(
        "Currency code is required with currency style.");
    return vm::ExecutionStatus::EXCEPTION;
  }

  // Parse currencyDisplay option.
  static const std::vector<std::u16string> validCurrencyDisplay = {
      u"symbol", u"narrowSymbol", u"code", u"name"};
  auto currDisp = getOptionString(
      runtime, options, u"currencyDisplay", validCurrencyDisplay, u"symbol");
  if (LLVM_UNLIKELY(currDisp == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  currencyDisplay_ = std::move(currDisp.getValue());

  // Parse unit option (required for unit style).
  auto unitIt = options.find(u"unit");
  if (unitIt != options.end()) {
    unit_ = unitIt->second.getString();
  } else if (style_ == u"unit") {
    (void)runtime.raiseTypeError("Unit is required with unit style.");
    return vm::ExecutionStatus::EXCEPTION;
  }

  // Parse unitDisplay option.
  static const std::vector<std::u16string> validUnitDisplay = {
      u"short", u"narrow", u"long"};
  auto unitDisp = getOptionString(
      runtime, options, u"unitDisplay", validUnitDisplay, u"short");
  if (LLVM_UNLIKELY(unitDisp == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  unitDisplay_ = std::move(unitDisp.getValue());

  // Parse notation.
  static const std::vector<std::u16string> validNotation = {
      u"standard", u"scientific", u"engineering", u"compact"};
  auto notRes = getOptionString(
      runtime, options, u"notation", validNotation, u"standard");
  if (LLVM_UNLIKELY(notRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  notation_ = std::move(notRes.getValue());

  // Parse useGrouping.
  useGrouping_ =
      getOptionBool(runtime, options, u"useGrouping", true).value_or(true);

  // Resolve locale.
  locale_ = resolveLocaleFromList(locales);

  // Query locale info for number formatting.
  decimalSep_ = getLocaleInfo(locale_, LOCALE_SDECIMAL);
  if (decimalSep_.empty())
    decimalSep_ = u".";
  thousandsSep_ = getLocaleInfo(locale_, LOCALE_STHOUSAND);
  if (thousandsSep_.empty())
    thousandsSep_ = u",";

  // Determine default fraction digits based on style.
  int defaultMinFrac = 0;
  int defaultMaxFrac = 3;
  if (style_ == u"currency") {
    int currDigits = getLocaleInfoInt(locale_, LOCALE_ICURRDIGITS);
    if (currDigits <= 0)
      currDigits = 2;
    defaultMinFrac = currDigits;
    defaultMaxFrac = currDigits;
    // Get currency symbol.
    if (currencyDisplay_ == u"code") {
      currencySymbol_ = currency_;
    } else {
      currencySymbol_ = getLocaleInfo(locale_, LOCALE_SCURRENCY);
      if (currencySymbol_.empty())
        currencySymbol_ = currency_;
    }
  } else if (style_ == u"percent") {
    defaultMinFrac = 0;
    defaultMaxFrac = 0;
  }

  // Parse minimumIntegerDigits.
  auto minIntRes = getNumberOption(
      runtime, options, u"minimumIntegerDigits", 1, 21, 1);
  if (LLVM_UNLIKELY(minIntRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  minimumIntegerDigits_ = minIntRes.getValue().value_or(1);

  // Parse minimumFractionDigits.
  auto minFracRes = getNumberOption(
      runtime,
      options,
      u"minimumFractionDigits",
      0,
      20,
      std::nullopt);
  if (LLVM_UNLIKELY(minFracRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;

  // Parse maximumFractionDigits.
  auto maxFracRes = getNumberOption(
      runtime,
      options,
      u"maximumFractionDigits",
      0,
      20,
      std::nullopt);
  if (LLVM_UNLIKELY(maxFracRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;

  // Apply ECMA-402 fraction digits defaults.
  if (minFracRes.getValue().has_value()) {
    minimumFractionDigits_ = minFracRes.getValue().value();
  } else {
    minimumFractionDigits_ = defaultMinFrac;
  }

  if (maxFracRes.getValue().has_value()) {
    maximumFractionDigits_ = maxFracRes.getValue().value();
  } else {
    maximumFractionDigits_ =
        std::max(defaultMaxFrac, minimumFractionDigits_);
  }

  // Clamp: max must be >= min.
  if (maximumFractionDigits_ < minimumFractionDigits_) {
    (void)runtime.raiseRangeError(
        "maximumFractionDigits value is out of range.");
    return vm::ExecutionStatus::EXCEPTION;
  }

  return vm::ExecutionStatus::RETURNED;
}

Options NumberFormatWinGlob::resolvedOptions() noexcept {
  Options result;
  result.emplace(u"locale", Option(locale_));
  result.emplace(u"style", Option(style_));
  result.emplace(u"notation", Option(notation_));
  result.emplace(
      u"minimumIntegerDigits",
      Option(static_cast<double>(minimumIntegerDigits_)));
  result.emplace(
      u"minimumFractionDigits",
      Option(static_cast<double>(minimumFractionDigits_)));
  result.emplace(
      u"maximumFractionDigits",
      Option(static_cast<double>(maximumFractionDigits_)));
  result.emplace(u"useGrouping", Option(useGrouping_));
  if (style_ == u"currency") {
    result.emplace(u"currency", Option(currency_));
    result.emplace(u"currencyDisplay", Option(currencyDisplay_));
  }
  if (style_ == u"unit") {
    result.emplace(u"unit", Option(unit_));
    result.emplace(u"unitDisplay", Option(unitDisplay_));
  }
  return result;
}

std::u16string NumberFormatWinGlob::format(double number) noexcept {
  // Handle special values.
  if (std::isnan(number))
    return u"NaN";
  if (std::isinf(number))
    return number > 0 ? u"\u221E" : u"-\u221E";

  if (style_ == u"percent") {
    number *= 100.0;
    auto formatted = formatDecimal(number);
    return formatted + u"%";
  }

  if (style_ == u"currency") {
    return formatCurrency(number);
  }

  if (style_ == u"unit") {
    auto formatted = formatDecimal(number);
    // Simple unit display (not localized).
    return formatted + u" " + unit_;
  }

  return formatDecimal(number);
}

std::vector<Part> NumberFormatWinGlob::formatToParts(double number) noexcept {
  // WinGlob doesn't support field decomposition.
  // Return single part with the full formatted string.
  Part part;
  part[u"type"] = u"integer";
  part[u"value"] = format(number);
  return {part};
}

std::u16string NumberFormatWinGlob::formatDecimal(double number) noexcept {
  bool negative = number < 0;
  if (negative)
    number = -number;

  // Convert number to string for GetNumberFormatEx input.
  // GetNumberFormatEx requires a string like "1234.56" with '.' as decimal.
  char buf[128];
  std::snprintf(buf, sizeof(buf), "%.*f", maximumFractionDigits_, number);
  std::u16string numStr = toUTF16ASCII(std::string(buf));

  NUMBERFMTW fmt = {};
  fmt.NumDigits = static_cast<UINT>(maximumFractionDigits_);
  fmt.LeadingZero = 1;
  fmt.Grouping = useGrouping_ ? 3 : 0;
  fmt.lpDecimalSep = const_cast<LPWSTR>(
      reinterpret_cast<LPCWSTR>(decimalSep_.c_str()));
  fmt.lpThousandSep = const_cast<LPWSTR>(
      reinterpret_cast<LPCWSTR>(thousandsSep_.c_str()));
  fmt.NegativeOrder = 1; // "- N"

  // Two-pass buffer pattern.
  int size = GetNumberFormatEx(
      reinterpret_cast<LPCWSTR>(locale_.c_str()),
      0,
      reinterpret_cast<LPCWSTR>(numStr.c_str()),
      &fmt,
      nullptr,
      0);
  if (size <= 0) {
    // Fallback: manual formatting.
    auto s = std::string(buf);
    std::u16string result(s.begin(), s.end());
    if (negative)
      result = u"-" + result;
    return result;
  }

  std::u16string result(size, u'\0');
  GetNumberFormatEx(
      reinterpret_cast<LPCWSTR>(locale_.c_str()),
      0,
      reinterpret_cast<LPCWSTR>(numStr.c_str()),
      &fmt,
      reinterpret_cast<LPWSTR>(&result[0]),
      size);
  // Remove trailing null.
  if (!result.empty() && result.back() == u'\0')
    result.pop_back();

  if (negative)
    result = u"-" + result;
  return result;
}

std::u16string NumberFormatWinGlob::formatCurrency(double number) noexcept {
  bool negative = number < 0;
  if (negative)
    number = -number;

  char buf[128];
  std::snprintf(buf, sizeof(buf), "%.*f", maximumFractionDigits_, number);
  std::u16string numStr = toUTF16ASCII(std::string(buf));

  CURRENCYFMTW fmt = {};
  fmt.NumDigits = static_cast<UINT>(maximumFractionDigits_);
  fmt.LeadingZero = 1;
  fmt.Grouping = useGrouping_ ? 3 : 0;
  fmt.lpDecimalSep = const_cast<LPWSTR>(
      reinterpret_cast<LPCWSTR>(decimalSep_.c_str()));
  fmt.lpThousandSep = const_cast<LPWSTR>(
      reinterpret_cast<LPCWSTR>(thousandsSep_.c_str()));
  fmt.lpCurrencySymbol = const_cast<LPWSTR>(
      reinterpret_cast<LPCWSTR>(currencySymbol_.c_str()));
  fmt.PositiveOrder = 0; // "$N"
  fmt.NegativeOrder = 1; // "-$N"

  int size = GetCurrencyFormatEx(
      reinterpret_cast<LPCWSTR>(locale_.c_str()),
      0,
      reinterpret_cast<LPCWSTR>(numStr.c_str()),
      &fmt,
      nullptr,
      0);
  if (size <= 0) {
    // Fallback.
    auto s = std::string(buf);
    std::u16string result = currencySymbol_;
    result.append(s.begin(), s.end());
    if (negative)
      result = u"-" + result;
    return result;
  }

  std::u16string result(size, u'\0');
  GetCurrencyFormatEx(
      reinterpret_cast<LPCWSTR>(locale_.c_str()),
      0,
      reinterpret_cast<LPCWSTR>(numStr.c_str()),
      &fmt,
      reinterpret_cast<LPWSTR>(&result[0]),
      size);
  if (!result.empty() && result.back() == u'\0')
    result.pop_back();

  if (negative)
    result = u"-" + result;
  return result;
}

// ============================================================================
// DateTimeFormat
// ============================================================================

DateTimeFormatWinGlob::~DateTimeFormatWinGlob() = default;

namespace {

/// Convert JS timestamp (ms since epoch) to local SYSTEMTIME.
SYSTEMTIME jsTimeToLocalSystemTime(double jsTimeValue) {
  // JS epoch is 1970-01-01. Windows FILETIME epoch is 1601-01-01.
  // Difference is 11644473600 seconds = 116444736000000000 * 100ns ticks.
  static constexpr int64_t kEpochDiff = 116444736000000000LL;

  int64_t ms = static_cast<int64_t>(jsTimeValue);
  // Convert ms to 100ns ticks and add epoch offset.
  int64_t ticks = ms * 10000LL + kEpochDiff;

  FILETIME ft;
  ft.dwLowDateTime = static_cast<DWORD>(ticks & 0xFFFFFFFF);
  ft.dwHighDateTime = static_cast<DWORD>(ticks >> 32);

  SYSTEMTIME utcSt;
  FileTimeToSystemTime(&ft, &utcSt);

  SYSTEMTIME localSt;
  SystemTimeToTzSpecificLocalTime(nullptr, &utcSt, &localSt);
  return localSt;
}

} // anonymous namespace

vm::ExecutionStatus DateTimeFormatWinGlob::initialize(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &inputOptions) noexcept {
  // Apply ECMA-402 defaults via toDateTimeOptions.
  auto optionsRes = toDateTimeOptions(runtime, inputOptions, u"any", u"date");
  if (LLVM_UNLIKELY(optionsRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  Options options = std::move(optionsRes.getValue());

  // Resolve locale.
  locale_ = resolveLocaleFromList(locales);

  // Parse dateStyle and timeStyle.
  static const std::vector<std::u16string> validDateStyleValues = {
      u"full", u"long", u"medium", u"short"};
  auto dateStyleIt = options.find(u"dateStyle");
  if (dateStyleIt != options.end())
    dateStyle_ = dateStyleIt->second.getString();
  auto timeStyleIt = options.find(u"timeStyle");
  if (timeStyleIt != options.end())
    timeStyle_ = timeStyleIt->second.getString();

  // Parse individual component options.
  auto getOpt = [&](const std::u16string &key) -> std::u16string {
    auto it = options.find(key);
    if (it == options.end())
      return {};
    return it->second.getString();
  };

  weekday_ = getOpt(u"weekday");
  era_ = getOpt(u"era");
  year_ = getOpt(u"year");
  month_ = getOpt(u"month");
  day_ = getOpt(u"day");
  hour_ = getOpt(u"hour");
  minute_ = getOpt(u"minute");
  second_ = getOpt(u"second");
  timeZoneName_ = getOpt(u"timeZoneName");

  // Parse hourCycle.
  static const std::vector<std::u16string> validHourCycles = {
      u"h11", u"h12", u"h23", u"h24"};
  auto hcRes = getOptionString(
      runtime, options, u"hourCycle", validHourCycles, u"");
  if (LLVM_UNLIKELY(hcRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  hourCycle_ = std::move(hcRes.getValue());

  // Parse hour12 option (overrides hourCycle).
  auto hour12 = getOptionBool(runtime, options, u"hour12", std::nullopt);
  if (hour12.has_value()) {
    hourCycle_ = hour12.value() ? u"h12" : u"h23";
  }

  // If no hourCycle specified, detect from locale.
  if (hourCycle_.empty() && !hour_.empty()) {
    auto timeFormat = getLocaleInfo(locale_, LOCALE_STIMEFORMAT);
    // If format contains 'H' (uppercase), it's 24-hour.
    if (timeFormat.find(u'H') != std::u16string::npos)
      hourCycle_ = u"h23";
    else
      hourCycle_ = u"h12";
  }

  // Build format strings.
  if (!dateStyle_.empty() || !timeStyle_.empty()) {
    // Style-based formatting (use predefined flags).
    if (!dateStyle_.empty()) {
      formatDate_ = true;
      if (dateStyle_ == u"full" || dateStyle_ == u"long")
        dateFlags_ = DATE_LONGDATE;
      else
        dateFlags_ = DATE_SHORTDATE;
    }
    if (!timeStyle_.empty()) {
      formatTime_ = true;
      if (timeStyle_ == u"short" || timeStyle_ == u"medium")
        timeFlags_ = TIME_NOSECONDS;
      else
        timeFlags_ = 0;
    }
  } else {
    // Individual component formatting — build custom format strings.
    bool hasDateComponent =
        !year_.empty() || !month_.empty() || !day_.empty() ||
        !weekday_.empty() || !era_.empty();
    bool hasTimeComponent =
        !hour_.empty() || !minute_.empty() || !second_.empty();

    if (hasDateComponent) {
      formatDate_ = true;
      std::u16string dateParts;

      if (!weekday_.empty()) {
        if (weekday_ == u"long")
          dateParts += u"dddd";
        else
          dateParts += u"ddd";
        if (!month_.empty() || !day_.empty() || !year_.empty())
          dateParts += u", ";
      }

      if (!month_.empty()) {
        if (month_ == u"long")
          dateParts += u"MMMM";
        else if (month_ == u"short")
          dateParts += u"MMM";
        else if (month_ == u"2-digit")
          dateParts += u"MM";
        else
          dateParts += u"M";
      }

      if (!day_.empty()) {
        if (!month_.empty())
          dateParts += u" ";
        if (day_ == u"2-digit")
          dateParts += u"dd";
        else
          dateParts += u"d";
      }

      if (!year_.empty()) {
        if (!month_.empty() || !day_.empty())
          dateParts += u", ";
        if (year_ == u"2-digit")
          dateParts += u"yy";
        else
          dateParts += u"yyyy";
      }

      if (!dateParts.empty()) {
        // Wrap in single quotes to escape format characters.
        dateFormatStr_ = dateParts;
      } else {
        // Fallback to short date.
        dateFlags_ = DATE_SHORTDATE;
      }
    }

    if (hasTimeComponent) {
      formatTime_ = true;
      std::u16string timeParts;
      bool is24h = (hourCycle_ == u"h23" || hourCycle_ == u"h24");

      if (!hour_.empty()) {
        if (is24h)
          timeParts += (hour_ == u"2-digit") ? u"HH" : u"H";
        else
          timeParts += (hour_ == u"2-digit") ? u"hh" : u"h";
      }

      if (!minute_.empty()) {
        if (!timeParts.empty())
          timeParts += u":";
        timeParts += (minute_ == u"2-digit") ? u"mm" : u"m";
      }

      if (!second_.empty()) {
        if (!timeParts.empty())
          timeParts += u":";
        timeParts += (second_ == u"2-digit") ? u"ss" : u"s";
      }

      if (!hour_.empty() && !is24h)
        timeParts += u" tt";

      if (!timeParts.empty()) {
        timeFormatStr_ = timeParts;
      } else {
        timeFlags_ = 0;
      }
    }

    // If no components specified at all, default to short date.
    if (!formatDate_ && !formatTime_) {
      formatDate_ = true;
      dateFlags_ = DATE_SHORTDATE;
    }
  }

  return vm::ExecutionStatus::RETURNED;
}

Options DateTimeFormatWinGlob::resolvedOptions() noexcept {
  Options result;
  result.emplace(u"locale", Option(locale_));
  result.emplace(u"timeZone", Option(std::u16string(u"UTC")));

  if (!dateStyle_.empty())
    result.emplace(u"dateStyle", Option(dateStyle_));
  if (!timeStyle_.empty())
    result.emplace(u"timeStyle", Option(timeStyle_));

  // Report individual components if no dateStyle/timeStyle.
  if (dateStyle_.empty() && timeStyle_.empty()) {
    if (!weekday_.empty())
      result.emplace(u"weekday", Option(weekday_));
    if (!era_.empty())
      result.emplace(u"era", Option(era_));
    if (!year_.empty())
      result.emplace(u"year", Option(year_));
    if (!month_.empty())
      result.emplace(u"month", Option(month_));
    if (!day_.empty())
      result.emplace(u"day", Option(day_));
    if (!hour_.empty()) {
      result.emplace(u"hour", Option(hour_));
      result.emplace(u"hourCycle", Option(hourCycle_));
    }
    if (!minute_.empty())
      result.emplace(u"minute", Option(minute_));
    if (!second_.empty())
      result.emplace(u"second", Option(second_));
    if (!timeZoneName_.empty())
      result.emplace(u"timeZoneName", Option(timeZoneName_));
  }
  return result;
}

std::u16string DateTimeFormatWinGlob::format(double jsTimeValue) noexcept {
  if (std::isnan(jsTimeValue))
    return u"Invalid Date";

  SYSTEMTIME st = jsTimeToLocalSystemTime(jsTimeValue);
  std::u16string result;

  if (formatDate_) {
    wchar_t buf[256] = {0};
    LPCWSTR fmtStr = dateFormatStr_.empty()
        ? nullptr
        : reinterpret_cast<LPCWSTR>(dateFormatStr_.c_str());
    DWORD flags = dateFormatStr_.empty() ? dateFlags_ : 0;
    int len = GetDateFormatEx(
        reinterpret_cast<LPCWSTR>(locale_.c_str()),
        flags,
        &st,
        fmtStr,
        buf,
        256,
        nullptr);
    if (len > 1) // len includes null terminator.
      result.assign(reinterpret_cast<const char16_t *>(buf), len - 1);
  }

  if (formatDate_ && formatTime_ && !result.empty())
    result += u", ";

  if (formatTime_) {
    wchar_t buf[256] = {0};
    LPCWSTR fmtStr = timeFormatStr_.empty()
        ? nullptr
        : reinterpret_cast<LPCWSTR>(timeFormatStr_.c_str());
    DWORD flags = timeFormatStr_.empty() ? timeFlags_ : 0;
    int len = GetTimeFormatEx(
        reinterpret_cast<LPCWSTR>(locale_.c_str()),
        flags,
        &st,
        fmtStr,
        buf,
        256);
    if (len > 1)
      result.append(reinterpret_cast<const char16_t *>(buf), len - 1);
  }

  return result;
}

std::vector<Part> DateTimeFormatWinGlob::formatToParts(
    double jsTimeValue) noexcept {
  // WinGlob doesn't support field decomposition.
  // Return single literal part with the full formatted string.
  Part part;
  part[u"type"] = u"literal";
  part[u"value"] = format(jsTimeValue);
  return {part};
}

} // namespace winglob
} // namespace platform_intl
} // namespace hermes
