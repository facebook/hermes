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
  std::u16string mResolvedStyle, mResolvedCurrency, mResolvedCurrencyDisplay, mResolvedCurrencySign, mResolvedUnit, mResolvedUnitDisplay, mRoundingType, locale;
  uint8_t mResolvedMinimumIntegerDigits, mResolvedMinimumFractionDigits, mResolvedMaximumFractionDigits;
};

NumberFormat::NumberFormat() : impl_(std::make_unique<Impl>()) {}
NumberFormat::~NumberFormat() {}

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
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
// TODO: isWellFormedUnitIdentifier

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
vm::CallResult<Options> setNumberFormatUnitOptions(const Options &options) {
//  1. Assert: Type(intlObj) is Object.
//  2. Assert: Type(options) is Object.
//  3. Let style be ? GetOption(options, "style", "string", « "decimal", "percent", "currency", "unit" », "decimal").
//  4. Set intlObj.[[Style]] to style.
  const std::vector<std::u16string> styleVector = {u"decimal", u"percent", u"currency", u"unit"};
  auto style = getOption(options, u"style", u"string", styleVector, u"decimal");
  // Want to use impl_ but it can't be accessed here?
  impl_->mResolvedStyle = style;
//  Let currency be ? GetOption(options, "currency", "string", undefined, undefined).
//  TODO: undefined fallbacks
  auto currency = getOption(options, u"currency", u"string", undefined, undefined);
//  6. If currency is undefined, then
  if (currency->getString() == u"") {
//  a. If style is "currency", throw a TypeError exception.
    if (impl_->mResolvedStyle == u"currency") {
      // Throw typeError through initialize?
      return vm::ExecutionStatus::EXCEPTION;
    }
    //  7. Else,
    else {
    //  a. If the result of IsWellFormedCurrencyCode(currency) is false, throw a RangeError exception.
      if (!isWellFormedCurrencyCode(currency)) {
        return vm::ExecutionStatus::EXCEPTION;
      }
    }
  }
//  8. Let currencyDisplay be ? GetOption(options, "currencyDisplay", "string", « "code", "symbol", "narrowSymbol", "name" », "symbol").
  std::vector<std::u16string> currencyDisplayVector = {u"code", u"symbol", u"narrowSymbol", u"name"};
  auto currencyDisplay = getOption(options, u"currencyDisplay", u"string", currencyDisplayVector, u"symbol");
//  9. Let currencySign be ? GetOption(options, "currencySign", "string", « "standard", "accounting" », "standard").
  std::vector<std::u16string> currencySignVector = {u"standard", u"accounting"};
  auto currencySign = getOption(options, u"currencySign", u"string", currencySignVector, u"standard");
//  10. Let unit be ? GetOption(options, "unit", "string", undefined, undefined).
  auto unit = getOption(options, u"unit", u"string", undefined, undefined);
//  11. If unit is undefined, then
  if (unit->getString() == u"") {
//  a. If style is "unit", throw a TypeError exception.
    if (impl_->mResolvedStyle == u"unit") {
      return runtime->raiseTypeError("Expected unit !");
    }
//  12. Else,
    else {
//  a. If the result of IsWellFormedUnitIdentifier(unit) is false, throw a RangeError exception.
      if (!isWellFormedCurrencyCode(unit)) {
        return runtime->raiseRangeError("Expected unit !");
      }
    }
  }
//  13. Let unitDisplay be ? GetOption(options, "unitDisplay", "string", « "short", "narrow", "long" », "short").
  auto unitDisplay = getOption(options, u"unitDisplay", u"string", {u"short", u"narrow", u"long"}, u"short");
//  14. If style is "currency", then
  if (impl_->mResolvedStyle == u"currency") {
//  a. Let currency be the result of converting currency to upper case as specified in 6.1.
//  b. Set intlObj.[[Currency]] to currency.
//  c. Set intlObj.[[CurrencyDisplay]] to currencyDisplay.
//  d. Set intlObj.[[CurrencySign]] to currencySign.
    impl_->mResolvedCurrency = normalizeCurrencyCode(currency);
    impl_->mCurrencyDisplay = currencyDisplay;
    impl_->mCurrencySign = currencySign;
  }
  if (impl_->mResolvedStyle == u"unit") {
//  15. If style is "unit", then
//  a. Set intlObj.[[Unit]] to unit.
//  b. Set intlObj.[[UnitDisplay]] to unitDisplay.
    impl_->mResolvedUnit = unit->getString();
    impl_->mResolvedUnitDisplay = unitDisplay->getString();
  }
  return vm::ExecutionStatus::RETURNED;
}

// https://tc39.es/ecma402/#sec-setnfdigitoptions
vm::CallResult<Options> setNumberFormatDigitOptions(const Options &options) {
//  1. Let mnid be ? GetNumberOption(options, "minimumIntegerDigits,", 1, 21, 1).
  auto mnid = getOptionNumber(options, u"minimumIntegerDigits", 1, 21, 1);
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
  impl_->mResolvedMinimumIntegerDigits = mnid.getNumber();
  bool hasSd, hasFd, needSd;
//  If mnsd is not undefined or mxsd is not undefined, then
  if (mnsd != u"" || mxsd != u"") {
    // mRoundingType = SIGNIFICANT_DIGITS?
    mnsd = defaultNumberOption(mnsd->second.getNumber(), 1, 21, 1);
    mxsd = defaultNumberOption(mxsd->second.getNumber(), mnsd->second.getNumber(), 21, 21);
    impl_->mResolvedMinimumSignificantDigits = std::floor(mnsd);
    impl_-> mResolvedMaximumSignificantDigits = std::floor(mxsd);
  } else if (mnfd != u"" || mxfd != u"") {
    // mRoundingType = FRACTION_DIGITS?
    // TODO: Find out what mnfdDefault is
    mnfd = defaultNumberOption(mnfd->second.getNumber(), 0, 20, mnfdDefault);
    //  Object mxfdActualDefault = JSObjects.newNumber(Math.max(JSObjects.getJavaDouble(mnfd), JSObjects.getJavaDouble(mxfdDefault)));
    mxfd = defaultNumberOption(mxfd, mnfd, 20, mxfdActualDefault);
    impl_->mResolvedMinimumFractionDigits = std::floor(mnfd);
    impl_->mResolvedMaximumFractionDigits = std::floor(mxfd);
  } else if (mResolvedNotation == COMPACT) {
    impl_->mRoundingType = COMPACT_ROUNDING;
  } else if (mResolvedNotation == ENGINEERING) {
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
    impl_->mRoundingType = FRACTION_DIGITS;
    mResolvedMaximumFractionDigits = 5;
  } else {
    impl_->mRoundingType = FRACTION_DIGITS;
    impl_->mResolvedMinimumFractionDigits = std::floor(mnfdDefault);
    impl_->mResolvedMaximumFractionDigits = std::floor(mxfdDefault);
  }
}
// TODO: Add isLocaleIdType

vm::ExecutionStatus NumberFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
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
