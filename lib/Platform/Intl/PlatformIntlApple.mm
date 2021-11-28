/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/PlatformIntl.h"

#import <Foundation/Foundation.h>

namespace hermes {
namespace platform_intl {
namespace {
NSString *u16StringToNSString(std::u16string src) {
  auto size = src.size();
  const auto *cString = (const unichar *)src.c_str();
  return [NSString stringWithCharacters:cString length:size];
}
std::u16string nsStringToU16String(NSString *src) {
  auto size = src.length;
  std::u16string result;
  result.resize(size);
  [src getCharacters:(unichar *)&result[0] range:NSMakeRange(0, size)];
  return result;
}
std::u16string getDefaultLocale() {
  // Environment variable used for testing only
  const char *testLocale = std::getenv("_HERMES_TEST_LOCALE");
  if (testLocale) {
    NSString *nsTestLocale = [NSString stringWithUTF8String:testLocale];
    return nsStringToU16String(nsTestLocale);
  }
  NSString *defLocale = [[NSLocale currentLocale] localeIdentifier];
  return nsStringToU16String(defLocale);
}
// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sec-bestavailablelocale
llvh::Optional<std::u16string> bestAvailableLocale(
    const std::vector<std::u16string> &availableLocales,
    const std::u16string &locale) {
  // 1. Let candidate be locale
  std::u16string candidate = locale;

  // 2. Repeat
  while (true) {
    // a. If availableLocales contains an element equal to candidate, return
    // candidate.
    if (llvh::find(availableLocales, candidate) != availableLocales.end())
      return candidate;

    // b. Let pos be the character index of the last occurrence of "-" (U+002D)
    // within candidate.
    size_t pos = candidate.rfind(u'-');

    // ...If that character does not occur, return undefined.
    if (pos == std::u16string::npos)
      return llvh::None;

    // c. If pos ≥ 2 and the character "-" occurs at index pos-2 of candidate,
    // decrease pos by 2.
    if (pos >= 2 && candidate[pos - 2] == '-')
      pos -= 2;

    // d. Let candidate be the substring of candidate from position 0,
    // inclusive, to position pos, exclusive.
    candidate.resize(pos);
  }
}

// Implementer note: For more information review
// https://402.ecma-international.org/7.0/#sec-unicode-locale-extension-sequences
std::u16string toNoUnicodeExtensionsLocale(const std::u16string &locale) {
  std::vector<std::u16string> subtags;
  auto s = locale.begin();
  const auto e = locale.end();
  while (true) {
    auto tagEnd = std::find(s, e, u'-');
    subtags.emplace_back(s, tagEnd);
    if (tagEnd == e)
      break;
    s = tagEnd + 1;
  }
  std::u16string result;
  size_t size = subtags.size();
  for (size_t i = 0; i < size;) {
    if (i > 0)
      result.append(u"-");
    result.append(subtags[i]);
    i++;
    // If next tag is a private marker and there are remaining tags
    if (subtags[i] == u"u" && i < size - 1)
      // Skip those tags until you reach end or another singleton subtag
      while (i < size && subtags[i].size() > 1)
        i++;
  }
  return result;
}
// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sec-lookupmatcher
struct LocaleMatch {
  std::u16string locale;
  std::u16string extension;
};
LocaleMatch lookupMatcher(
    const std::vector<std::u16string> &requestedLocales,
    const std::vector<std::u16string> &availableLocales) {
  // 1. Let result be a new Record.
  LocaleMatch result;
  // 2. For each element locale of requestedLocales, do
  for (const std::u16string &locale : requestedLocales) {
    // a. Let noExtensionsLocale be the String value that is locale with
    // any Unicode locale extension sequences removed.
    std::u16string noExtensionsLocale = toNoUnicodeExtensionsLocale(locale);
    // b. Let availableLocale be BestAvailableLocale(availableLocales,
    // noExtensionsLocale).
    llvh::Optional<std::u16string> availableLocale =
        bestAvailableLocale(availableLocales, noExtensionsLocale);
    // c. If availableLocale is not undefined, then
    if (availableLocale) {
      // i. Set result.[[locale]] to availableLocale.
      result.locale = std::move(*availableLocale);
      // ii. If locale and noExtensionsLocale are not the same String value,
      // then
      // 1. Let extension be the String value consisting of the substring of
      // the Unicode locale extension sequence within locale.
      // 2. Set result.[[extension]] to extension.
      result.extension = locale.substr(noExtensionsLocale.length());
      // iii. Return result.
      return result;
    }
  }
  // availableLocale was undefined, so set result.[[locale]] to defLocale.
  result.locale = getDefaultLocale();
  // 5. Return result.
  return result;
}
}

// Implementation of https://tc39.es/ecma402/#sec-canonicalizelocalelist
vm::CallResult<std::vector<std::u16string>> canonicalizeLocaleList(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
  // 1. If locales is undefined, then
  //   a. Return a new empty List
  // Not needed, this validation occurs closer to VM in 'normalizeLocales'.
  // 2. Let seen be a new empty List.
  std::vector<std::u16string> seen;
  // 3. If Type(locales) is String or Type(locales) is Object and locales has an
  // [[InitializedLocale]] internal slot, then
  // 4. Else
  // We don't yet support Locale object -
  // https://tc39.es/ecma402/#locale-objects As of now, 'locales' can only be a
  // string list/array. Validation occurs in normalizeLocaleList, so this
  // function just takes a vector of strings.
  // 5. Let len be ? ToLength(? Get(O, "length")).
  // 6. Let k be 0.
  // 7. Repeat, while k < len
  for (std::u16string locale : locales) {
    // TODO - BCP 47 tag validation
    // 7.c.vi. Let canonicalizedTag be CanonicalizeUnicodeLocaleId(tag).
    auto *localeNSString = u16StringToNSString(locale);
    NSString *canonicalizedTagNSString =
        [NSLocale canonicalLocaleIdentifierFromString:localeNSString];
    auto canonicalizedTag = nsStringToU16String(canonicalizedTagNSString);
    // 7.c.vii. If canonicalizedTag is not an element of seen, append
    // canonicalizedTag as the last element of seen.
    if (std::find(seen.begin(), seen.end(), canonicalizedTag) == seen.end()) {
      seen.push_back(std::move(canonicalizedTag));
    }
  }
  return seen;
}

// https://tc39.es/ecma402/#sec-canonicalizelocalelist
vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
  return canonicalizeLocaleList(runtime, locales);
}

vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"lowered");
}
vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"uppered");
}

struct Collator::Impl {
  std::u16string locale;
};

Collator::Collator() : impl_(std::make_unique<Impl>()) {}
Collator::~Collator() {}

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus Collator::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  return vm::ExecutionStatus::RETURNED;
}

Options Collator::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return x.compare(y);
}

struct DateTimeFormat::Impl {
  std::u16string locale;
};

DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}
DateTimeFormat::~DateTimeFormat() {}

vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  return vm::ExecutionStatus::RETURNED;
}

Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  auto s = std::to_string(jsTimeValue);
  return std::u16string(s.begin(), s.end());
}

std::vector<std::unordered_map<std::u16string, std::u16string>>
DateTimeFormat::formatToParts(double jsTimeValue) noexcept {
  std::unordered_map<std::u16string, std::u16string> part;
  part[u"type"] = u"integer";
  // This isn't right, but I didn't want to do more work for a stub.
  std::string s = std::to_string(jsTimeValue);
  part[u"value"] = {s.begin(), s.end()};
  return std::vector<std::unordered_map<std::u16string, std::u16string>>{part};
}

struct NumberFormat::Impl {
  uint8_t mResolvedMinimumIntegerDigits, mResolvedMinimumFractionDigits, mResolvedMaximumFractionDigits;
  // impl_->
  std::u16string currency, notation, CompactDisplay, numberFormat, style, locale;
  bool UseGrouping;
  struct newRecord {
      std::u16string localeMatcher, ca, nu;// For opt
    };
};

NumberFormat::NumberFormat() : impl_(std::make_unique<Impl>()) {}
NumberFormat::~NumberFormat() {}

// https://tc39.es/ecma402/#sec-intl.numberformat.supportedlocalesof
vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
    //  1. Let availableLocales be %NumberFormat%.[[AvailableLocales]].
    NSArray<NSString *> *nsAvailableLocales =
        [NSLocale availableLocaleIdentifiers];

    // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
    vm::CallResult<std::vector<std::u16string>> requestedLocales =
        getCanonicalLocales(runtime, locales);
    std::vector<std::u16string> availableLocales =
        nsStringArrayToU16StringArray(nsAvailableLocales);

    // 3. Return ? SupportedLocales(availableLocales, requestedLocales, options).
    // Get a non-const copy of options
    Options copyOptions = options;
    return supportedLocales(
        availableLocales, requestedLocales.getValue(), copyOptions);
}

// https://tc39.es/ecma402/#sec-defaultnumberoption
vm::CallResult<std::uint8_t> defaultNumberOption(const std::uint8_t value,
  const std::uint8_t minimum, const std::uint8_t maximum,
  const std::uint8_t fallback) {
//  1. If value is undefined, return fallback.
  if (!value) {
    return fallback;
  }
//  2. Set value to ? ToNumber(value).
//  3. If value is NaN or less than minimum or greater than maximum, throw a RangeError exception.
  if (value == NAN || value < minimum || value > maximum) {
    return vm::ExecutionStatus::EXCEPTION;
  }
//  4. Return floor(value).
  return std::floor(value);
}
// https://tc39.es/ecma402/#sec-issanctionedsimpleunitidentifier
bool isSanctionedSimpleUnitIdentifier(std::u16string unitIdentifier) {
  // This vector should be kept alphabetically ordered so that we can binary search in it (Table 2)
  static std::vector<std::u16string> s_sanctionedSimpleUnitIdentifiers = {
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
    u"mile",
    u"mile-scandinavian",
    u"milliliter",
    u"millimeter",
    u"millisecond",
    u"minute",
    u"month",
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
    u"year"
  };
//  1. If unitIdentifier is listed in Table 2 below, return true.
//  2. Else, return false.
  if (std::binary_search(s_sanctionedSimpleUnitIdentifiers.begin(), s_sanctionedSimpleUnitIdentifiers.end(), unitIdentifier)) {
    return true;
  }
  return false;
};
// https://tc39.es/ecma402/#sec-iswellformedunitidentifier
bool isWellFormedUnitIdentifier(std::u16string unitIdentifier) {
  //  1. If the result of IsSanctionedSimpleUnitIdentifier(unitIdentifier) is true, then
  //  a. Return true.
  if (isSanctionedSimpleUnitIdentifier(unitIdentifier)) {
    return true;
  }
  //  2. If the substring "-per-" does not occur exactly once in unitIdentifier, then
  //  a. Return false.
  auto found = unitIdentifier.find(u"-per-");
  if (found == std::string::npos) {// Not found
    return false;
  }
  //  3. Let numerator be the substring of unitIdentifier from the beginning to just before "-per-".
  //  4. If the result of IsSanctionedSimpleUnitIdentifier(numerator) is false, then
  //  a. Return false.
  std::u16string numerator = unitIdentifier.substr(0, found);
  if (!isSanctionedSimpleUnitIdentifier(numerator)) {
    return false;
  }
  //  5. Let denominator be the substring of unitIdentifier from just after "-per-" to the end.
  //  6. If the result of IsSanctionedSimpleUnitIdentifier(denominator) is false, then
  //  a. Return false.
  std::u16string demoninator = unitIdentifier.substr((found+5), unitIdentifier.length());
  if (!isSanctionedSimpleUnitIdentifier(numerator)) {
    return false;
  }
  //  7. Return true.
  return true;
}
// https://tc39.es/ecma402/#sec-case-sensitivity-and-case-mapping
// Note that we should convert only upper case translation in ASCII range.
std::u16string normalizeCurrencyCode(std::u16string currencyCode) {
  std::u16string normalized;
  std::uint8_t offset = 'a' - 'A';
  for (std::uint8_t idx = 0; idx < currencyCode.length(); idx++) {
    unsigned char c = currencyCode[idx];
    if (c >= 'a' && c <= 'z') {
      normalized.push_back((char) c - offset);
    }
    else {
      normalized.push_back(c);
    }
    }
  return normalized;
}

// https://tc39.es/ecma402/#sec-iswellformedcurrencycode
bool isWellFormedCurrencyCode(std::u16string currencyCode) {
//  1. Let normalized be the result of mapping currency to upper case as described in 6.1.
  std::u16string normalized = normalizeCurrencyCode(currencyCode);
//  2. If the number of elements in normalized is not 3, return false.
  if (normalized.size() != 3) {
    return false;
  }
//  3. If normalized contains any character that is not in the range "A" to "Z" (U+0041 to U+005A), return false.
  if (!std::all_of(std::begin(normalized), std::end(normalized),
                   [](char c){ return std::isupper(c); })) {
    return false;
  }
//  4. Return true.
  return true;
}
// https://tc39.es/ecma402/#sec-setnumberformatunitoptions
vm::CallResult<Options> setNumberFormatUnitOptions(Options &options) {
//  1. Assert: Type(intlObj) is Object.
//  2. Assert: Type(options) is Object.
//  3. Let style be ? GetOption(options, "style", "string", « "decimal", "percent", "currency", "unit" », "decimal").
//  4. Set intlObj.[[Style]] to style.
  const std::vector<std::u16string> styleVector = {u"decimal", u"percent", u"currency", u"unit"};
  Options decimalFallback;
  std::u16string decimal = u"decimal"; // If not declared, it will not work
  decimalFallback.emplace(std::u16string(u"fallback"), Option(decimal));
  auto style = getOption(options, u"style", u"string", styleVector, decimalFallback);
  // Want to use impl_ but it can't be accessed here?
  // impl_->mResolvedStyle = style;
  options.emplace(std::u16string(u"style"), Option(style->getString()));
//  Let currency be ? GetOption(options, "currency", "string", undefined, undefined).
  std::vector<std::u16string> undefinedVector;
  Options undefinedFallback;
  std::u16string undefined; // If this is truly undefined it will error throw?
  undefinedFallback.emplace(std::u16string(u"fallback"), Option(undefined));
  auto currency = getOption(options, u"currency", u"string", undefinedVector, undefinedFallback);
//  6. If currency is undefined, then
  if (currency->getString() == u"") {
//  a. If style is "currency", throw a TypeError exception.
    if (style->getString() == u"currency") {
      // Throw typeError through initialize?
      return vm::ExecutionStatus::EXCEPTION;
    }
    //  7. Else,
    else {
    //  a. If the result of IsWellFormedCurrencyCode(currency) is false, throw a RangeError exception.
      if (!isWellFormedCurrencyCode(currency->getString())) {
        return vm::ExecutionStatus::EXCEPTION;
      }
    }
  }
//  8. Let currencyDisplay be ? GetOption(options, "currencyDisplay", "string", « "code", "symbol", "narrowSymbol", "name" », "symbol").
  std::vector<std::u16string> currencyDisplayVector = {u"code", u"symbol", u"narrowSymbol", u"name"};
  Options symbolFallback;
  std::u16string symbol = u"symbol"; // If not declared, it will not work
  symbolFallback.emplace(std::u16string(u"fallback"), Option(symbol));
  auto currencyDisplay = getOption(options, u"currencyDisplay", u"string", currencyDisplayVector, symbolFallback);
//  9. Let currencySign be ? GetOption(options, "currencySign", "string", « "standard", "accounting" », "standard").
  std::vector<std::u16string> currencySignVector = {u"standard", u"accounting"};
  Options standardFallback;
  std::u16string standard = u"standard"; // If not declared, it will not work
  standardFallback.emplace(std::u16string(u"fallback"), Option(standard));
  auto currencySign = getOption(options, u"currencySign", u"string", currencySignVector, standardFallback);
//  10. Let unit be ? GetOption(options, "unit", "string", undefined, undefined).
  auto unit = getOption(options, u"unit", u"string", undefinedVector, undefinedFallback);
//  11. If unit is undefined, then
  if (unit->getString() == u"") {
//  a. If style is "unit", throw a TypeError exception.
    if (style->getString() == u"unit") {
      return vm::ExecutionStatus::EXCEPTION;
    }
//  12. Else,
    else {
//  a. If the result of IsWellFormedUnitIdentifier(unit) is false, throw a RangeError exception.
      if (!isWellFormedUnitIdentifier(unit->getString())) {
        return vm::ExecutionStatus::EXCEPTION;
      }
    }
  }
//  13. Let unitDisplay be ? GetOption(options, "unitDisplay", "string", « "short", "narrow", "long" », "short").
  Options shortFallback;
  std::u16string shortfb = u"short"; // If not declared, it will not work
  symbolFallback.emplace(std::u16string(u"fallback"), Option(shortfb));
  auto unitDisplay = getOption(options, u"unitDisplay", u"string", {u"short", u"narrow", u"long"}, shortFallback);
//  14. If style is "currency", then
  if (style->getString() == u"currency") {
//  a. Let currency be the result of converting currency to upper case as specified in 6.1.
//  b. Set intlObj.[[Currency]] to currency.
//  c. Set intlObj.[[CurrencyDisplay]] to currencyDisplay.
//  d. Set intlObj.[[CurrencySign]] to currencySign.
    options.emplace(std::u16string(u"currency"), Option(normalizeCurrencyCode(currency->getString())));
    options.emplace(std::u16string(u"currencyDisplay"), Option(currencyDisplay->getString()));
    options.emplace(std::u16string(u"currencySign"), Option(currencySign->getString()));
  }
  if (style->getString() == u"unit") {
//  15. If style is "unit", then
//  a. Set intlObj.[[Unit]] to unit.
//  b. Set intlObj.[[UnitDisplay]] to unitDisplay.
    options.emplace(std::u16string(u"unit"), Option(unit->getString()));
    options.emplace(std::u16string(u"unitDisplay"), Option(unitDisplay->getString()));
  }
  return vm::ExecutionStatus::RETURNED;
}

// https://tc39.es/ecma402/#sec-setnfdigitoptions
vm::CallResult<Options> setNumberFormatDigitOptions(Options &options) {
  uint8_t mnfdDefault = 0;
  uint8_t mxfdDefault = 0; // placeholders, need changing
  //  1. Let mnid be ? GetNumberOption(options, "minimumIntegerDigits,", 1, 21, 1).
  Options oneFallback;
  std::u16string one = u"1"; // If not declared, it will not work
  oneFallback.emplace(std::u16string(u"fallback"), Option(one));
  auto mnid = getOptionNumber(options, u"minimumIntegerDigits", 1, 21, oneFallback);
//  2. Let mnfd be ? Get(options, "minimumFractionDigits").
  auto mnfd = options.find(u"minimumFractionDigits");
//  3. Let mxfd be ? Get(options, "maximumFractionDigits").
  auto mxfd = options.find(u"maximumFractionDigits");
//  4. Let mnsd be ? Get(options, "minimumSignificantDigits").
  auto mnsd = options.find(u"minimumSignificantDigits");
//  5. Let mxsd be ? Get(options, "maximumSignificantDigits").
  auto mxsd = options.find(u"maximumSignificantDigits");
//  6. Set intlObj.[[MinimumIntegerDigits]] to mnid.
//  May need to be rounded?
  options.emplace(std::u16string(u"minimumIntegerDigits"), Option(mnid->getString()));
  bool hasSd, hasFd, needSd;
//  If mnsd is not undefined or mxsd is not undefined, then
  if (mnsd->second.getNumber() != NAN || mxsd->second.getNumber() != NAN) {
    // mRoundingType = SIGNIFICANT_DIGITS?
    uint8_t dmnsd = defaultNumberOption(mnsd->second.getNumber(), 1, 21, 1).getValue();
    uint8_t dmxsd = defaultNumberOption(mxsd->second.getNumber(), mnsd->second.getNumber(), 21, 21).getValue();
    options.emplace(std::u16string(u"minimumSignificantDigits"), Option(std::floor(dmnsd)));
    options.emplace(std::u16string(u"maximumSignificantDigits"), Option(std::floor(dmxsd)));
  } else if (mnfd != options.end() || mxfd != options.end()) {
    // mRoundingType = FRACTION_DIGITS?
    // TODO: Find out what mnfdDefault/mxfdDefault is, currently going on 0
    
    auto dmnfd = defaultNumberOption(mnfd->second.getNumber(), 0, 20, mnfdDefault);
    //  Object mxfdActualDefault = JSObjects.newNumber(Math.max(JSObjects.getJavaDouble(mnfd), JSObjects.getJavaDouble(mxfdDefault)));
    uint8_t mxfdActualDefault = std::max(mnfd->second.getNumber(), mxfdDefault);
    auto dmxfd = defaultNumberOption(mxfd->second.getNumber(), mnfd->second.getNumber(), 20, mxfdActualDefault);
    options.emplace(std::u16string(u"minimumFractionDigits"), Option(std::floor(dmnfd.getValue())));
    options.emplace(std::u16string(u"maximumFractionDigits"), Option(std::floor(dmxfd.getValue())));
  } else if (options.find(u"notation")->second.getString() == u"compact") {
    // mRoundingType = COMPACT_ROUNDING;
  } else if (options.find(u"notation")->second.getString() == u"engineering") {
// The default setting for engineering notation.
// This is not based on the spec, but is required by our implementation of engineering
// notation.
// From
// https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1DecimalFormat.html
// If areSignificantDigitsUsed() returns false, then the minimum number of significant digits
// shown is one,
// and the maximum number of significant digits shown is the sum of the minimum integer and
// maximum fraction digits,
// and is unaffected by the maximum integer digits.
//
// In short, the minimum integer will be set to 1 and hence to achieve maximum default
// fraction digits of "3" (as in spec), we should set the maximum fraction digits to "5"
    // mRoundingType = FRACTION_DIGITS;
    // mResolvedMaximumFractionDigits = 5;
  } else {
    // mRoundingType = FRACTION_DIGITS;
    options.emplace(std::u16string(u"minimumFractionDigits"), Option(std::floor(mnfdDefault)));
    options.emplace(std::u16string(u"maximumFractionDigits"), Option(std::floor(mxfdDefault)));
  }
}
// TODO: Add isLocaleIdType
// TODO: Add CurrencyDigits https://tc39.es/ecma402/#sec-currencydigits
uint8_t getCurrencyDigits(std::u16string currency) {
//  1. If the ISO 4217 currency and funds code list contains currency as an alphabetic code, return the minor unit value corresponding to the currency from the list; otherwise, return 2.
// I think NSNumberFormatter takes a locale, not currency code. Could we make a list to check the currency string against the 40 currencies that don't use 2?
  return 2;
};
// https://tc39.es/ecma402/#sec-initializenumberformat
vm::ExecutionStatus NumberFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
// 1. Let requestedLocales be ? CanonicalizeLocaleList(locales).
    const vm::CallResult<std::vector<std::u16string>> requestedLocales =
            getCanonicalLocales(runtime, locales);
    if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION)) {
        return vm::ExecutionStatus::EXCEPTION;
      }
// 2. Set options to ? CoerceOptionsToObject(options).
// 3. Let opt be a new Record.
    Impl::newRecord opt;
// Create a copy of the unordered map &options
    Options copyOptions = options;
// 4. Let matcher be ? GetOption(options, "localeMatcher", "string", « "lookup", "best fit" », "best fit").
    std::vector<std::u16string> values = {u"lookup", u"best fit"};
    Options matcherFallback;
    std::u16string bestFit = u"best fit"; // If not declared, it will not work
    matcherFallback.emplace(std::u16string(u"fallback"), Option(bestFit));
    auto matcher = getOption(copyOptions, u"localeMatcher", u"string", values, matcherFallback);
// 5. Set opt.[[localeMatcher]] to matcher.
    opt.localeMatcher = matcher->getString();
    
    std::vector<std::u16string> emptyVector;
    Options undefinedFallback;
    std::u16string undefined; // If this is truly undefined it will error throw?
    undefinedFallback.emplace(std::u16string(u"fallback"), Option(undefined));
// 6. Let numberingSystem be ? GetOption(options, "numberingSystem", "string", undefined, undefined).
    auto numberingSystem = getOption(copyOptions, u"numberingSystem", u"string", emptyVector, undefinedFallback);
// 7. If numberingSystem is not undefined, then
//   a. If numberingSystem does not match the Unicode Locale Identifier type nonterminal, throw a RangeError exception.
// 8. Set opt.[[nu]] to numberingSystem.
    if (numberingSystem->isString()) {
          opt.nu = numberingSystem->getString();
      }
      else {
        return runtime->raiseRangeError("Incorrect numberingSystem information provided");
      }
// TODO: Once resolveLocale has been resolved
// Let r be ResolveLocale(%NumberFormat%.[[AvailableLocales]], requestedLocales, opt, %NumberFormat%.[[RelevantExtensionKeys]], localeData).
// 11. Set numberFormat.[[Locale]] to r.[[locale]].
// 12. Set numberFormat.[[DataLocale]] to r.[[dataLocale]].
// 13. Set numberFormat.[[NumberingSystem]] to r.[[nu]].
      
// Perform ? SetNumberFormatUnitOptions(numberFormat, options).
  setNumberFormatUnitOptions(copyOptions);
// 15. Let style be numberFormat.[[Style]].
  auto style = options.find(u"style");
  uint8_t mnfdDefault, mxfdDefault, cDigits;
// 16. If style is "currency", then
  if (style->second.getString() == u"currency") {
// a. Let currency be numberFormat.[[Currency]].
    std::u16string currency = impl_->currency;
// b. Let cDigits be CurrencyDigits(currency).
    cDigits = getCurrencyDigits(currency);
// c. Let mnfdDefault be cDigits.
    mnfdDefault = cDigits;
// d. Let mxfdDefault be cDigits.
    mxfdDefault = cDigits;
  } else {
// 17. Else,
// a. Let mnfdDefault be 0.
    mnfdDefault = 0;
// b. If style is "percent", then
    if (style->second.getString() == u"percent")  {
// i. Let mxfdDefault be 0.
      mxfdDefault = 0;
    }
    else {
// c. Else,
// i. Let mxfdDefault be 3.
      mxfdDefault = 3;
    }
  }
// 18. Let notation be ? GetOption(options, "notation", "string", « "standard", "scientific", "engineering", "compact" », "standard").
  Options standardFallback;
  std::u16string standard = u"standard"; // If not declared, it will not work
  standardFallback.emplace(std::u16string(u"fallback"), Option(standard));
  auto notation = getOption(copyOptions, u"notation", u"string", {u"standard", u"scientific", u"engineering", u"compact"}, standardFallback);
// 19. Set numberFormat.[[Notation]] to notation.
  impl_->notation = notation->getString();

// Perform ? SetNumberFormatDigitOptions(numberFormat, options, mnfdDefault, mxfdDefault, notation).
  setNumberFormatDigitOptions(copyOptions);
// 21. Let compactDisplay be ? GetOption(options, "compactDisplay", "string", « "short", "long" », "short").
  Options shortFallback;
  std::u16string shortfb = u"short"; // If not declared, it will not work
  shortFallback.emplace(std::u16string(u"fallback"), Option(shortfb));
  auto compactDisplay = getOption(copyOptions, u"compactDisplay", u"string", {u"short", u"long"}, shortFallback);
// 22. If notation is "compact", then
    if (notation->getString() == u"compact") {
//   a. Set numberFormat.[[CompactDisplay]] to compactDisplay.
      impl_->CompactDisplay = compactDisplay->getString();
    }
// 23. Let useGrouping be ? GetOption(options, "useGrouping", "boolean", undefined, true).
  Options trueFallback;
  std::vector<std::u16string> undefinedVector;
  std::u16string truefb = u"true"; // If not declared, it will not work
  trueFallback.emplace(std::u16string(u"fallback"), Option(truefb));
  auto useGrouping = getOption(copyOptions, u"useGrouping", u"boolean", undefinedVector, trueFallback);
// 24. Set numberFormat.[[UseGrouping]] to useGrouping.
  impl_->UseGrouping = useGrouping->getBool();
// 25. Let signDisplay be ? GetOption(options, "signDisplay", "string", « "auto", "never", "always", "exceptZero" », "auto").
  Options autoFallback;
  std::u16string autofb = u"auto"; // If not declared, it will not work
  autoFallback.emplace(std::u16string(u"fallback"), Option(autofb));
  auto signDisplay = getOption(copyOptions, u"signDisplay", u"string", {u"auto", u"never", u"always", u"exceptZero"}, autoFallback);
// 26. Set numberFormat.[[SignDisplay]] to signDisplay.
  impl_->numberFormat = signDisplay->getString();
  return vm::ExecutionStatus::RETURNED;
}

Options NumberFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

std::u16string NumberFormat::format(double number) noexcept {
  auto s = std::to_string(number);
  return std::u16string(s.begin(), s.end());
}

std::vector<std::unordered_map<std::u16string, std::u16string>>
NumberFormat::formatToParts(double number) noexcept {
  std::unordered_map<std::u16string, std::u16string> part;
  part[u"type"] = u"integer";
  // This isn't right, but I didn't want to do more work for a stub.
  std::string s = std::to_string(number);
  part[u"value"] = {s.begin(), s.end()};
  return std::vector<std::unordered_map<std::u16string, std::u16string>>{part};
}

} // namespace platform_intl
} // namespace hermes
