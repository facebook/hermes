/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

#ifndef HERMES_PLATFORM_INTL_WIN_INTL_UTILS_H
#define HERMES_PLATFORM_INTL_WIN_INTL_UTILS_H

#include "hermes/Platform/Intl/PlatformIntl.h"
#include "hermes/Platform/Intl/hermes_icu.h"

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace hermes {
namespace platform_intl {

// ICU constants (matching ICU headers we don't include).
// These are stable ABI values that haven't changed since ICU 2.x.
constexpr int32_t kULOC_FULLNAME_CAPACITY = 157;

// UDateFormatStyle values
constexpr int32_t kUDAT_FULL = 0;
constexpr int32_t kUDAT_LONG = 1;
constexpr int32_t kUDAT_MEDIUM = 2;
constexpr int32_t kUDAT_SHORT = 3;
constexpr int32_t kUDAT_DEFAULT = kUDAT_MEDIUM;
constexpr int32_t kUDAT_NONE = -1;
constexpr int32_t kUDAT_PATTERN = -2;

// UDateTimePatternMatchOptions
constexpr int32_t kUDATPG_MATCH_ALL_FIELDS_LENGTH = 0xFFFF;

// UColAttribute values
constexpr int32_t UCOL_FRENCH_COLLATION = 0;
constexpr int32_t UCOL_ALTERNATE_HANDLING = 1;
constexpr int32_t UCOL_CASE_FIRST = 2;
constexpr int32_t UCOL_CASE_LEVEL = 3;
constexpr int32_t UCOL_NORMALIZATION_MODE = 4;
constexpr int32_t UCOL_STRENGTH = 5;
constexpr int32_t UCOL_NUMERIC_COLLATION = 7;

// UColAttributeValue values
constexpr int32_t UCOL_DEFAULT = -1;
constexpr int32_t UCOL_PRIMARY = 0;
constexpr int32_t UCOL_SECONDARY = 1;
constexpr int32_t UCOL_TERTIARY = 2;
constexpr int32_t UCOL_OFF = 16;
constexpr int32_t UCOL_ON = 17;
constexpr int32_t UCOL_SHIFTED = 20;
constexpr int32_t UCOL_NON_IGNORABLE = 21;
constexpr int32_t UCOL_LOWER_FIRST = 24;
constexpr int32_t UCOL_UPPER_FIRST = 25;

// UCollationResult values
constexpr int32_t UCOL_LESS = -1;
constexpr int32_t UCOL_EQUAL = 0;
constexpr int32_t UCOL_GREATER = 1;

// ULocAvailableType
constexpr int32_t ULOC_AVAILABLE_DEFAULT = 0;

// UAcceptResult
constexpr int32_t ULOC_ACCEPT_FAILED = 0;
constexpr int32_t ULOC_ACCEPT_VALID = 1;
constexpr int32_t ULOC_ACCEPT_FALLBACK = 2;

// UNumberFormatStyle values (from unicode/unum.h)
constexpr int32_t UNUM_DECIMAL = 1;
constexpr int32_t UNUM_CURRENCY = 2;
constexpr int32_t UNUM_PERCENT = 3;
constexpr int32_t UNUM_SCIENTIFIC = 4;
constexpr int32_t UNUM_CURRENCY_ISO = 10;
constexpr int32_t UNUM_CURRENCY_PLURAL = 11;
constexpr int32_t UNUM_CURRENCY_ACCOUNTING = 12;

// UNumberFormatAttribute values (from unicode/unum.h)
constexpr int32_t UNUM_GROUPING_USED = 1;
constexpr int32_t UNUM_MAX_INTEGER_DIGITS = 3;
constexpr int32_t UNUM_MIN_INTEGER_DIGITS = 4;
constexpr int32_t UNUM_MAX_FRACTION_DIGITS = 6;
constexpr int32_t UNUM_MIN_FRACTION_DIGITS = 7;
constexpr int32_t UNUM_ROUNDING_MODE = 11;
constexpr int32_t UNUM_SIGNIFICANT_DIGITS_USED = 16;
constexpr int32_t UNUM_MIN_SIGNIFICANT_DIGITS = 17;
constexpr int32_t UNUM_MAX_SIGNIFICANT_DIGITS = 18;

// UNumberFormatTextAttribute values (from unicode/unum.h)
constexpr int32_t UNUM_CURRENCY_CODE = 5;

/// Convert UTF-8 string to UTF-16.
vm::CallResult<std::u16string> UTF8toUTF16(
    vm::Runtime &runtime,
    std::string_view in);

/// Convert UTF-16 string to UTF-8.
vm::CallResult<std::string> UTF16toUTF8(
    vm::Runtime &runtime,
    const std::u16string &in);

/// Normalize a BCP47 language tag using ICU locale operations.
/// Requires an active ICU vtable (icu != nullptr).
vm::CallResult<std::u16string> normalizeLanguageTag(
    vm::Runtime &runtime,
    const hermes_icu_vtable *icu,
    const std::u16string &locale);

/// Canonicalize a list of locales per ECMA-402.
/// Requires an active ICU vtable (icu != nullptr).
vm::CallResult<std::vector<std::u16string>> canonicalizeLocaleList(
    vm::Runtime &runtime,
    const hermes_icu_vtable *icu,
    const std::vector<std::u16string> &locales);

/// Get a string option from options, validating against allowed values.
vm::CallResult<std::u16string> getOptionString(
    vm::Runtime &runtime,
    const Options &options,
    const std::u16string &property,
    const std::vector<std::u16string> &validValues,
    const std::u16string &fallback);

/// Get a bool option from options.
std::optional<bool> getOptionBool(
    vm::Runtime &runtime,
    const Options &options,
    const std::u16string &property,
    std::optional<bool> fallback);

/// https://402.ecma-international.org/8.0/#sec-bestavailablelocale
std::optional<std::u16string> bestAvailableLocale(
    const std::vector<std::u16string> &availableLocales,
    const std::u16string &locale);

/// https://402.ecma-international.org/8.0/#sec-lookupsupportedlocales
std::vector<std::u16string> lookupSupportedLocales(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales);

/// https://402.ecma-international.org/8.0/#sec-supportedlocales
vm::CallResult<std::vector<std::u16string>> supportedLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales,
    const Options &options);

/// https://402.ecma-international.org/8.0/#sec-todatetimeoptions
vm::CallResult<Options> toDateTimeOptions(
    vm::Runtime &runtime,
    Options options,
    std::u16string_view required,
    std::u16string_view defaults);

/// Get a numeric option, validate range, default.
vm::CallResult<std::optional<int>> getNumberOption(
    vm::Runtime &runtime,
    const Options &options,
    const std::u16string &property,
    int minimum,
    int maximum,
    std::optional<int> fallback);

/// Convert BCP47 language tag to ICU locale ID via vtable.
std::string convertBCP47toICULocale(
    const hermes_icu_vtable *icu,
    const std::u16string &localeBCP47);

/// Convert ICU locale ID to BCP47 language tag via vtable.
std::string convertICUtoBCP47Locale(
    const hermes_icu_vtable *icu,
    const char *localeICU);

/// Convert UTF-16 string to ASCII UTF-8 (for locale IDs).
std::string toUTF8ASCII(const std::u16string &str);

/// Convert ASCII UTF-8 to UTF-16 (for locale IDs).
std::u16string toUTF16ASCII(const std::string &str);

/// Convert string to bool ("true"/"false").
bool convertToBool(const std::u16string &str);

/// Lowercase ASCII characters in a string.
std::string toLowerASCII(const std::string &str);

/// Validate and canonicalize a timezone identifier.
/// Returns the canonical IANA timezone name, or throws RangeError for invalid.
vm::CallResult<std::u16string> validateAndCanonicalizeTimeZone(
    vm::Runtime &runtime,
    const hermes_icu_vtable *icu,
    const std::u16string &timeZone);

// ============================================================================
// UDateFormatField constants (for formatToParts field mapping)
// ============================================================================

constexpr int32_t kUDAT_ERA_FIELD = 0;
constexpr int32_t kUDAT_YEAR_FIELD = 1;
constexpr int32_t kUDAT_MONTH_FIELD = 2;
constexpr int32_t kUDAT_DATE_FIELD = 3;
constexpr int32_t kUDAT_HOUR_OF_DAY1_FIELD = 4;
constexpr int32_t kUDAT_HOUR_OF_DAY0_FIELD = 5;
constexpr int32_t kUDAT_MINUTE_FIELD = 6;
constexpr int32_t kUDAT_SECOND_FIELD = 7;
constexpr int32_t kUDAT_FRACTIONAL_SECOND_FIELD = 8;
constexpr int32_t kUDAT_DAY_OF_WEEK_FIELD = 9;
constexpr int32_t kUDAT_AM_PM_FIELD = 14;
constexpr int32_t kUDAT_HOUR1_FIELD = 15;
constexpr int32_t kUDAT_HOUR0_FIELD = 16;
constexpr int32_t kUDAT_TIMEZONE_FIELD = 17;
constexpr int32_t kUDAT_YEAR_WOY_FIELD = 18;
constexpr int32_t kUDAT_RELATED_YEAR_FIELD = 34;

// UNumberFormatFields constants (for formatToParts field mapping)
constexpr int32_t kUNUM_INTEGER_FIELD = 0;
constexpr int32_t kUNUM_FRACTION_FIELD = 1;
constexpr int32_t kUNUM_DECIMAL_SEPARATOR_FIELD = 2;
constexpr int32_t kUNUM_EXPONENT_SYMBOL_FIELD = 3;
constexpr int32_t kUNUM_EXPONENT_SIGN_FIELD = 4;
constexpr int32_t kUNUM_EXPONENT_FIELD = 5;
constexpr int32_t kUNUM_GROUPING_SEPARATOR_FIELD = 6;
constexpr int32_t kUNUM_CURRENCY_FIELD = 7;
constexpr int32_t kUNUM_PERCENT_FIELD = 8;
constexpr int32_t kUNUM_PERMILL_FIELD = 9;
constexpr int32_t kUNUM_SIGN_FIELD = 10;

/// Map a UDateFormatField value to the ECMA-402 part type string.
/// Returns u"literal" for unknown fields.
const char16_t *dateFieldToPartType(int32_t field);

/// Map a UNumberFormatFields value to the ECMA-402 part type string.
/// Returns u"literal" for unknown fields.
const char16_t *numberFieldToPartType(int32_t field);

/// Filter unicode extensions in a BCP47 locale string.
/// Keeps only extensions whose key is in \p relevantKeys and whose value
/// passes basic validation. Strips all other `-u-*` extension key-value pairs.
/// Returns the filtered locale string.
std::u16string filterUnicodeExtensions(
    const std::u16string &locale,
    const std::unordered_set<std::u16string> &relevantKeys);

/// Remove a specific unicode extension key from a BCP47 locale string.
/// Returns the locale with the specified key-value pair removed.
std::u16string removeUnicodeExtensionKey(
    const std::u16string &locale,
    const std::u16string &key);

/// Get the value of a unicode extension key from a BCP47 locale string.
/// Returns empty string if the key is not present.
std::u16string getUnicodeExtensionValue(
    const std::u16string &locale,
    const std::u16string &key);

/// Map ECMA-402 simple unit identifier to CLDR measure unit identifier.
/// Returns empty string if the unit is not recognized.
std::string ecma402UnitToCldrMeasureUnit(const std::u16string &unit);

} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORM_INTL_WIN_INTL_UTILS_H
