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
  // impl_->
  uint8_t minimumIntegerDigits, minimumFractionDigits, maximumFractionDigits, minimumSignificantDigits, maximumSignificantDigits;
  std::u16string numberingSystem, currency, currencyDisplay, currencySign, notation, compactDisplay, signDisplay, unit, unitDisplay, style, locale;
  bool useGrouping;
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
  const std::uint8_t fallback, vm::Runtime *runtime) {
//  1. If value is undefined, return fallback.
  if (!value) {
    return fallback;
  }
//  2. Set value to ? ToNumber(value).
//  3. If value is NaN or less than minimum or greater than maximum, throw a RangeError exception.
  if (value == NAN || value < minimum || value > maximum) {
    return runtime->raiseRangeError("Number is invalid");
  }
//  4. Return floor(value).
  return std::floor(value);
}
// https://tc39.es/ecma402/#sec-issanctionedsimpleunitidentifier
bool isSanctionedSimpleUnitIdentifier(const std::u16string &unitIdentifier) {
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
bool isWellFormedUnitIdentifier(const std::u16string &unitIdentifier) {
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
std::u16string normalizeCurrencyCode(const std::u16string &currencyCode) {
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
bool isWellFormedCurrencyCode(const std::u16string &currencyCode) {
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
vm::CallResult<Options> setNumberFormatUnitOptions(Options &options, vm::Runtime *runtime) {
//  1. Assert: Type(intlObj) is Object.
//  2. Assert: Type(options) is Object.
//  3. Let style be ? GetOption(options, "style", "string", « "decimal", "percent", "currency", "unit" », "decimal").
//  4. Set intlObj.[[Style]] to style.
  Options decimalFallback;
  decimalFallback.emplace(std::u16string(u"fallback"), Option(std::u16string(u"decimal")));
  auto style = getOption(options, u"style", u"string", {u"decimal", u"percent", u"currency", u"unit"}, decimalFallback, runtime);
  options.emplace(std::u16string(u"style"), Option(style->getString()));
//  Let currency be ? GetOption(options, "currency", "string", undefined, undefined).
  Options undefinedFallback;
  undefinedFallback.emplace(std::u16string(u"fallback"), Option(std::u16string(u"")));
  auto currency = getOption(options, u"currency", u"string", {}, undefinedFallback, runtime);
//  6. If currency is undefined, then
  if (currency->getString() == u"") {
//  a. If style is "currency", throw a TypeError exception.
    if (style->getString() == u"currency") {
      return runtime->raiseTypeError("Currency is undefined");
    }
    //  7. Else,
    else {
    //  a. If the result of IsWellFormedCurrencyCode(currency) is false, throw a RangeError exception.
      if (!isWellFormedCurrencyCode(currency->getString())) {
        return runtime->raiseRangeError("Currency is invalid");
      }
    }
  }
//  8. Let currencyDisplay be ? GetOption(options, "currencyDisplay", "string", « "code", "symbol", "narrowSymbol", "name" », "symbol").
  Options symbolFallback;
  symbolFallback.emplace(std::u16string(u"fallback"), Option(std::u16string(u"symbol")));
  auto currencyDisplay = getOption(options, u"currencyDisplay", u"string", {u"code", u"symbol", u"narrowSymbol", u"name"}, symbolFallback, runtime);
//  9. Let currencySign be ? GetOption(options, "currencySign", "string", « "standard", "accounting" », "standard").
  Options standardFallback;
  standardFallback.emplace(std::u16string(u"fallback"), Option(std::u16string(u"standard")));
  auto currencySign = getOption(options, u"currencySign", u"string", {u"standard", u"accounting"}, standardFallback, runtime);
//  10. Let unit be ? GetOption(options, "unit", "string", undefined, undefined).
  auto unit = getOption(options, u"unit", u"string", {}, undefinedFallback, runtime);
//  11. If unit is undefined, then
  if (unit->getString() == u"") {
//  a. If style is "unit", throw a TypeError exception.
    if (style->getString() == u"unit") {
      return runtime->raiseTypeError("Unit is undefined");
    }
//  12. Else,
    else {
//  a. If the result of IsWellFormedUnitIdentifier(unit) is false, throw a RangeError exception.
      if (!isWellFormedUnitIdentifier(unit->getString())) {
        return runtime->raiseRangeError("Unit is invalid");
      }
    }
  }
//  13. Let unitDisplay be ? GetOption(options, "unitDisplay", "string", « "short", "narrow", "long" », "short").
  Options shortFallback;
  symbolFallback.emplace(std::u16string(u"fallback"), Option(std::u16string(u"short")));
  auto unitDisplay = getOption(options, u"unitDisplay", u"string", {u"short", u"narrow", u"long"}, shortFallback, runtime);
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
vm::CallResult<Options> setNumberFormatDigitOptions(Options &options, uint8_t mnfdDefault, uint8_t mxfdDefault, std::u16string &notation, vm::Runtime *runtime) {
  //  1. Let mnid be ? GetNumberOption(options, "minimumIntegerDigits,", 1, 21, 1).
  Options oneFallback;
  oneFallback.emplace(std::u16string(u"fallback"), 1);
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
  bool hasSd, hasFd, needSd, needFd;
//  If mnsd is not undefined or mxsd is not undefined, then
  if (mnsd->second.getNumber() != NAN || mxsd->second.getNumber() != NAN) {
//    a. Let hasSd be true.
    hasSd = true;
//    8. Else,
//    a. Let hasSd be false.
  } else {
    hasSd = false;
  }
//    9. If mnfd is not undefined or mxfd is not undefined, then
  if (mnfd->second.getNumber() != NAN || mxfd->second.getNumber() != NAN) {
//    a. Let hasFd be true.
    hasFd = true;
//    10. Else,
//    a. Let hasFd be false.
  } else {
    hasFd = false;
  }
//  11. Let needSd be hasSd.
  needSd = hasSd;
//  12. If hasSd is true, or hasFd is false and notation is "compact", then
  if (hasSd == true || (hasFd == false && notation == u"compact")) {
    //  a. Let needFd be false.
    needFd = false;
  } else {
//  13. Else,
//  a. Let needFd be true.
    needFd = true;
  }
//  14. If needSd is true, then
    if (needSd == true) {
//  a. Assert: hasSd is true.
//  b. Set mnsd to ? DefaultNumberOption(mnsd, 1, 21, 1).
    auto rmnsd = defaultNumberOption(mnsd->second.getNumber(), 1, 21, 1, runtime);
//  c. Set mxsd to ? DefaultNumberOption(mxsd, mnsd, 21, 21).
    auto rmxsd = defaultNumberOption(mxsd->second.getNumber(), mnsd->second.getNumber(), 21, 21, runtime);
//  d. Set intlObj.[[MinimumSignificantDigits]] to mnsd.
      options.emplace(std::u16string(u"minimumSignificantDigits"), (rmnsd));
//  e. Set intlObj.[[MaximumSignificantDigits]] to mxsd.
      options.emplace(std::u16string(u"maximumSignificantDigits"), (rmxsd));
  }
//  15. If needFd is true, then
    if (needFd == true) {
//  a. If hasFd is true, then
    if (hasFd == true) {
//  i. Set mnfd to ? DefaultNumberOption(mnfd, 0, 20, undefined).
      auto rmnfd = defaultNumberOption(mnfd->second.getNumber(), 0, 20, NAN, runtime);
//  ii. Set mxfd to ? DefaultNumberOption(mxfd, 0, 20, undefined).
      auto rmxfd = defaultNumberOption(mxfd->second.getNumber(), 0, 20, NAN, runtime);
//  iii. If mnfd is undefined, set mnfd to min(mnfdDefault, mxfd).
      if (rmnfd.getValue() == NAN) {
        rmnfd = std::min(mnfdDefault, rmxfd.getValue());
      }
//  iv. Else if mxfd is undefined, set mxfd to max(mxfdDefault, mnfd).
      else if (rmxfd.getValue() == NAN) {
        rmxfd = std::max(mxfdDefault, rmnfd.getValue());
      }
//  v. Else if mnfd is greater than mxfd, throw a RangeError exception.
      else if (rmnfd.getValue() > rmxfd.getValue()) {
        return runtime->raiseRangeError("minimumFractionDigits is greater than maximumFractionDigits");
      }
//  vi. Set intlObj.[[MinimumFractionDigits]] to mnfd.
      options.emplace(std::u16string(u"minimumFractionDigits"), (rmnfd));
//  vii. Set intlObj.[[MaximumFractionDigits]] to mxfd.
      options.emplace(std::u16string(u"maximumFractionDigits"), (rmxfd));
    }
//  b. Else,
    else {
//  i. Set intlObj.[[MinimumFractionDigits]] to mnfdDefault.
      options.emplace(std::u16string(u"minimumFractionDigits"), (mnfdDefault));
//  ii. Set intlObj.[[MaximumFractionDigits]] to mxfdDefault.
      options.emplace(std::u16string(u"maximumFractionDigits"), (mxfdDefault));
    }
//    16. If needSd is false and needFd is false, then
    if (needSd == false && needFd == false) {
//    a. Set intlObj.[[RoundingType]] to compactRounding.
      options.emplace(std::u16string(u"roundingType"), std::u16string(u"compactRounding"));
    }
//    17. Else if hasSd is true, then
    else if (hasSd == true) {
//    a. Set intlObj.[[RoundingType]] to significantDigits.
      options.emplace(std::u16string(u"roundingType"), std::u16string(u"significantDigits"));
    }
//    18. Else,
    else {
//    a. Set intlObj.[[RoundingType]] to fractionDigits.
      options.emplace(std::u16string(u"roundingType"), std::u16string(u"fractionDigits"));
    }
  }
}
// TODO: Add isLocaleIdType
uint8_t getCurrencyDigits(const std::u16string &currency) {
//  1. If the ISO 4217 currency and funds code list contains currency as an alphabetic code, return the minor unit value corresponding to the currency from the list; otherwise, return 2.
// https://en.wikipedia.org/wiki/ISO_4217#Active_codes
  static std::vector<std::u16string> currencyCodes = {u"BHD", u"BIF", u"CLF", u"CLP", u"DJF", u"GNF", u"IQD", u"ISK", u"JOD", u"JPY", u"KMF", u"KRW", u"KWD", u"LYD", u"OMR", u"PYG", u"RWF", u"TND", u"UGX", u"UYI", u"UYW", u"VND", u"VUV", u"XAF", u"XOF", u"XPF"};
  if (std::binary_search(currencyCodes.begin(), currencyCodes.end(), currency)) {
    uint8_t pos = find(currencyCodes.begin(), currencyCodes.end(), currency) -
    currencyCodes.begin();
    if (pos < currencyCodes.size()) {// Not out of bounds
      static std::vector<uint8_t> currencyDigits = {3, 0, 4, 0, 0, 0, 3, 0, 3, 0, 0, 0, 3, 3, 3, 0, 0, 3, 0, 0, 4, 0, 0, 0, 0, 0};
      return currencyDigits.at(pos);
    }
  }
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
    matcherFallback.emplace(std::u16string(u"fallback"), Option(std::u16string(u"best fit")));
    auto matcher = getOption(copyOptions, u"localeMatcher", u"string", values, matcherFallback, runtime);
// 5. Set opt.[[localeMatcher]] to matcher.
    opt.localeMatcher = matcher->getString();
    
    Options undefinedFallback;
    undefinedFallback.emplace(std::u16string(u"fallback"), Option(std::u16string(u"")));
// 6. Let numberingSystem be ? GetOption(options, "numberingSystem", "string", undefined, undefined).
      auto numberingSystem = getOption(copyOptions, u"numberingSystem", u"string", {}, undefinedFallback, runtime);
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
  setNumberFormatUnitOptions(copyOptions, runtime);
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
  standardFallback.emplace(std::u16string(u"fallback"), Option(std::u16string(u"standard")));
  auto notation = getOption(copyOptions, u"notation", u"string", {u"standard", u"scientific", u"engineering", u"compact"}, standardFallback, runtime);
// 19. Set numberFormat.[[Notation]] to notation.
  impl_->notation = notation->getString();

// Perform ? SetNumberFormatDigitOptions(numberFormat, options, mnfdDefault, mxfdDefault, notation).
  setNumberFormatDigitOptions(copyOptions, mnfdDefault, mxfdDefault, impl_->notation, runtime);
// 21. Let compactDisplay be ? GetOption(options, "compactDisplay", "string", « "short", "long" », "short").
  Options shortFallback;
  shortFallback.emplace(std::u16string(u"fallback"), Option(std::u16string(u"short")));
  auto compactDisplay = getOption(copyOptions, u"compactDisplay", u"string", {u"short", u"long"}, shortFallback, runtime);
// 22. If notation is "compact", then
    if (notation->getString() == u"compact") {
//   a. Set numberFormat.[[CompactDisplay]] to compactDisplay.
      impl_->compactDisplay = compactDisplay->getString();
    }
// 23. Let useGrouping be ? GetOption(options, "useGrouping", "boolean", undefined, true).
  Options trueFallback;
  trueFallback.emplace(std::u16string(u"fallback"), true);
      auto useGrouping = getOption(copyOptions, u"useGrouping", u"boolean", {}, trueFallback, runtime);
// 24. Set numberFormat.[[UseGrouping]] to useGrouping.
  impl_->useGrouping = useGrouping->getBool();
// 25. Let signDisplay be ? GetOption(options, "signDisplay", "string", « "auto", "never", "always", "exceptZero" », "auto").
  Options autoFallback;
  autoFallback.emplace(std::u16string(u"fallback"), Option(std::u16string(u"auto")));
  auto signDisplay = getOption(copyOptions, u"signDisplay", u"string", {u"auto", u"never", u"always", u"exceptZero"}, autoFallback, runtime);
// 26. Set numberFormat.[[SignDisplay]] to signDisplay.
  impl_->signDisplay = signDisplay->getString();
  return vm::ExecutionStatus::RETURNED;
}

// https://tc39.es/ecma402/#sec-intl.numberformat.prototype.resolvedoptions
Options NumberFormat::resolvedOptions() noexcept {
  Options options;
//  5. For each row of Table 11, except the header row, in table order, do
//  a. Let p be the Property value of the current row.
//  b. Let v be the value of nf's internal slot whose name is the Internal Slot value of the current row.
//  c. If v is not undefined, then
//  i. Perform ! CreateDataPropertyOrThrow(options, p, v).
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  if (impl_->numberingSystem != u"") {
      options.emplace(u"numberingSystem", Option(impl_->numberingSystem));
  }
  if (impl_->style != u"") {
      options.emplace(u"style", Option(impl_->style));
  }
  if (impl_->currency != u"") {
      options.emplace(u"currency", Option(impl_->currency));
  }
  if (impl_->currencyDisplay != u"") {
  options.emplace(u"currencyDisplay", Option(impl_->currencyDisplay));
  }
  if (impl_->currencySign != u"") {
      options.emplace(u"currencySign", Option(impl_->currencySign));
  }
  if (impl_->unit != u"") {
  options.emplace(u"unit", Option(impl_->unit));
  }
  if (impl_->unitDisplay != u"") {
      options.emplace(u"unitDisplay", Option(impl_->unitDisplay));
  }
  if (impl_->minimumIntegerDigits != NAN) {
  options.emplace(u"minimumIntegerDigits", impl_->minimumIntegerDigits);
  }
  if (impl_->minimumFractionDigits != NAN) {
      options.emplace(u"minimumFractionDigits", impl_->minimumFractionDigits);
  }
  if (impl_->maximumFractionDigits != NAN) {
  options.emplace(u"maximumFractionDigits", impl_->maximumFractionDigits);
  }
  if (impl_->minimumSignificantDigits != NAN) {
  options.emplace(u"minimumSignificantDigits", impl_->minimumSignificantDigits);
  }
  if (impl_->maximumSignificantDigits != NAN) {
      options.emplace(u"maximumSignificantDigits", impl_->maximumSignificantDigits);
  }
  if (impl_->useGrouping == true || impl_->useGrouping == false) {
      options.emplace(u"useGrouping", Option(impl_->useGrouping));
  }
  if (impl_->notation != u"") {
      options.emplace(u"notation", Option(impl_->notation));
  }
  if (impl_->compactDisplay != u"") {
      options.emplace(u"compactDisplay", Option(impl_->compactDisplay));
  }
  if (impl_->signDisplay != u"") {
    options.emplace(u"signDisplay", Option(impl_->signDisplay));
}
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
