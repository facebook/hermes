/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>
#include <unordered_set>
#include "hermes/Platform/Intl/PlatformIntl.h"

namespace hermes {
namespace platform_intl {
namespace {
NSString *u16StringToNSString(const std::u16string &src) {
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
std::vector<std::u16string> nsStringArrayToU16StringArray(
    const NSArray<NSString *> *array) {
  auto size = [array count];
  std::vector<std::u16string> result;
  for (size_t i = 0; i < size; i++) {
    result.push_back(nsStringToU16String(array[i]));
  }
  return result;
}
NSArray<NSString *> *u16StringArrayToNSStringArray(
    const std::vector<std::u16string> &array) {
  auto size = array.size();
  NSMutableArray *result = [NSMutableArray arrayWithCapacity:size];
  for (size_t i = 0; i < size; i++) {
    result[i] = u16StringToNSString(array[i]);
  }
  return result;
}
const std::vector<std::u16string> &getAvailableLocales() {
  static const std::vector<std::u16string> *availableLocales = [] {
    NSArray<NSString *> *availableLocales =
        [NSLocale availableLocaleIdentifiers];
    // Intentionally leaked to avoid destruction order problems.
    auto *vec = new std::vector<std::u16string>();
    for (id str in availableLocales)
      vec->push_back(nsStringToU16String(str));
    return vec;
  }();
  return *availableLocales;
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
  std::u16string matchedLocale;
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
      // the Unicode locale extension sequence within locale, starting -u.
      // 2. Set result.[[extension]] to extension.
      if (noExtensionsLocale.length() > 5) {
        result.extension = locale.substr(5, noExtensionsLocale.length());
      }
      // iii. Return result.
      return result;
    }
  }
  // availableLocale was undefined, so set result.[[locale]] to defLocale.
  result.locale = getDefaultLocale();
  // 5. Return result.
  return result;
}
// Implementer note: This method corresponds roughly to
// https://402.ecma-international.org/7.0/#sec-lookupsupportedlocales
std::vector<std::u16string> lookupSupportedLocales(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales) {
  // 1. Let subset be a new empty List.
  std::vector<std::u16string> subset;
  // 2. For each element locale of requestedLocales in List order, do
  for (const std::u16string &locale : requestedLocales) {
    // a. Let noExtensionsLocale be the String value that is locale with all
    // Unicode locale extension sequences removed.
    std::u16string noExtensionsLocale = toNoUnicodeExtensionsLocale(locale);
    // b. Let availableLocale be BestAvailableLocale(availableLocales,
    // noExtensionsLocale).
    llvh::Optional<std::u16string> availableLocale =
        bestAvailableLocale(availableLocales, noExtensionsLocale);
    // c. If availableLocale is not undefined, append locale to the end of
    // subset.
    if (availableLocale) {
      subset.push_back(locale);
    }
  }
  // 3. Return subset.
  return subset;
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

vm::CallResult<std::u16string> localeListToLocaleString(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
  // 3. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  vm::CallResult<std::vector<std::u16string>> requestedLocales =
      canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }

  // 4. If requestedLocales is not an empty List, then
  // a. Let requestedLocale be requestedLocales[0].
  // 5. Else,
  // a. Let requestedLocale be DefaultLocale().
  std::u16string requestedLocale = requestedLocales->empty()
      ? getDefaultLocale()
      : std::move(requestedLocales->front());
  // 6. Let noExtensionsLocale be the String value that is requestedLocale with
  // any Unicode locale extension sequences (6.2.1) removed.
  std::u16string noExtensionsLocale =
      toNoUnicodeExtensionsLocale(requestedLocale);

  // 7. Let availableLocales be a List with language tags that includes the
  // languages for which the Unicode Character Database contains language
  // sensitive case mappings. Implementations may add additional language tags
  // if they support case mapping for additional locales.
  // 8. Let locale be BestAvailableLocale(availableLocales, noExtensionsLocale).
  // Convert to C++ array for bestAvailableLocale function
  const std::vector<std::u16string> &availableLocales = getAvailableLocales();
  llvh::Optional<std::u16string> locale =
      bestAvailableLocale(availableLocales, noExtensionsLocale);
  // 9. If locale is undefined, let locale be "und".
  return locale.getValueOr(u"und");
}
// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sup-string.prototype.tolocalelowercase
vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  NSString *nsStr = u16StringToNSString(str);
  // Steps 3-9 in localeListToLocaleString()
  vm::CallResult<std::u16string> locale =
      localeListToLocaleString(runtime, locales);
  // 10. Let cpList be a List containing in order the code points of S as
  // defined in es2022, 6.1.4, starting at the first element of S.
  // 11. Let cuList be a List where the elements are the result of a lower case
  // transformation of the ordered code points in cpList according to the
  // Unicode Default Case Conversion algorithm or an implementation-defined
  // conversion algorithm. A conforming implementation's lower case
  // transformation algorithm must always yield the same cpList given the same
  // cuList and locale.
  // 12. Let L be a String whose elements are the UTF-16 Encoding (defined in
  // es2022, 6.1.4) of the code points of cuList.
  NSString *L = u16StringToNSString(locale.getValue());
  // 13. Return L.
  return nsStringToU16String([nsStr
      lowercaseStringWithLocale:[[NSLocale alloc] initWithLocaleIdentifier:L]]);
}

// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sup-string.prototype.tolocaleuppercase
vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  NSString *nsStr = u16StringToNSString(str);
  // Steps 3-9 in localeListToLocaleString()
  vm::CallResult<std::u16string> locale =
      localeListToLocaleString(runtime, locales);
  // 10. Let cpList be a List containing in order the code points of S as
  // defined in es2022, 6.1.4, starting at the first element of S.
  // 11. Let cuList be a List where the elements are the result of a lower case
  // transformation of the ordered code points in cpList according to the
  // Unicode Default Case Conversion algorithm or an implementation-defined
  // conversion algorithm. A conforming implementation's lower case
  // transformation algorithm must always yield the same cpList given the same
  // cuList and locale.
  // 12. Let L be a String whose elements are the UTF-16 Encoding (defined in
  // es2022, 6.1.4) of the code points of cuList.
  NSString *L = u16StringToNSString(locale.getValue());
  // 13. Return L.
  return nsStringToU16String([nsStr
      uppercaseStringWithLocale:[[NSLocale alloc] initWithLocaleIdentifier:L]]);
}
// Implementation of
// https://tc39.es/ecma402/#sec-getoption
// Split into getOptionString and getOptionBool to help readability
vm::CallResult<llvh::Optional<std::u16string>> getOptionString(
    const Options &options,
    const std::u16string &property,
    llvh::ArrayRef<std::u16string> values,
    llvh::Optional<std::u16string> fallback,
    vm::Runtime *runtime) {
  // 1. Assert type(options) is object
  // 2. Let value be ? Get(options, property).
  auto value = options.find(property);
  // 3. If value is undefined, return fallback.
  if (value == options.end()) {
    return fallback;
  }
  // 4. Assert: type is "boolean" or "string".
  // 5. If type is "boolean", then
  // a. Set value to ! ToBoolean(value).
  // 6. If type is "string", then
  // a. Set value to ? ToString(value).
  // 7. If values is not undefined and values does not contain an element equal
  // to value, throw a RangeError exception.
  if (!value->second.isString() ||
      (!values.empty() &&
       llvh::find(values, value->second.getString()) == values.end())) {
    return runtime->raiseRangeError(
        vm::TwineChar16("Value ") +
        vm::TwineChar16(value->second.getString().c_str()) +
        vm::TwineChar16(
            " out of range for Intl.DateTimeFormat options property ") +
        vm::TwineChar16(property.c_str()));
  }
  // 8. Return value.
  return llvh::Optional<std::u16string>(value->second.getString());
}
vm::CallResult<llvh::Optional<bool>> getOptionBool(
    const Options &options,
    const std::u16string &property,
    llvh::Optional<bool> fallback,
    vm::Runtime *runtime) {
  //  1. Assert: Type(options) is Object.
  //  2. Let value be ? Get(options, property).
  auto value = options.find(property);
  //  3. If value is undefined, return fallback.
  if (value == options.end()) {
    return fallback;
  }
  if (!value->second.isBool()) {
    return runtime->raiseRangeError(
        vm::TwineChar16("Value ") + vm::TwineChar16(value->second.getBool()) +
        vm::TwineChar16(
            " out of range for Intl.DateTimeFormat options property ") +
        vm::TwineChar16(property.c_str()));
  }
  //  8. Return value.
  return llvh::Optional<bool>(value->second.getBool());
}
// https://tc39.es/ecma402/#sec-defaultnumberoption
vm::CallResult<llvh::Optional<uint8_t>> defaultNumberOption(
    const Options &options,
    const std::u16string &property,
    llvh::Optional<Option> value,
    const std::uint8_t minimum,
    const std::uint8_t maximum,
    llvh::Optional<uint8_t> fallback,
    vm::Runtime *runtime) {
  //  1. If value is undefined, return fallback.
  if (!value) {
    return fallback;
  }
  //  2. Set value to ? ToNumber(value).
  //  3. If value is NaN or less than minimum or greater than maximum, throw a
  //  RangeError exception.
  if (!value->isNumber() || std::isnan(value->getNumber()) ||
      value->getNumber() < minimum || value->getNumber() > maximum) {
    return runtime->raiseRangeError(
        vm::TwineChar16(property.c_str()) +
        vm::TwineChar16(" value is out of range"));
  }
  //  4. Return floor(value).
  return llvh::Optional<uint8_t>(std::floor(value->getNumber()));
}
// Implementation of
// https://402.ecma-international.org/8.0/#sec-getnumberoption
vm::CallResult<llvh::Optional<uint8_t>> getNumberOption(
    const Options &options,
    const std::u16string &property,
    const std::uint8_t minimum,
    const std::uint8_t maximum,
    llvh::Optional<uint8_t> fallback,
    vm::Runtime *runtime) {
  //  1. Assert: Type(options) is Object.
  //  2. Let value be ? Get(options, property).
  llvh::Optional<Option> value;
  auto iter = options.find(property);
  if (iter != options.end()) {
    value = Option(iter->second);
  }
  //  3. Return ? DefaultNumberOption(value, minimum, maximum, fallback).
  auto defaultNumber = defaultNumberOption(
      options, property, value, minimum, maximum, fallback, runtime);
  if (defaultNumber == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  } else {
    return llvh::Optional<uint8_t>(defaultNumber.getValue());
  }
}

// Implementation of
// https://tc39.es/ecma402/#sec-supportedlocales
vm::CallResult<std::vector<std::u16string>> supportedLocales(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales,
    const Options &options,
    vm::Runtime *runtime) {
  // 1. Set options to ? CoerceOptionsToObject(options).
  // 2. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  // "lookup", "best fit" », "best fit").
  vm::CallResult<std::u16string> matcher =
      getOptionString(
          options,
          u"localeMatcher",
          {u"lookup", u"best fit"},
          llvh::Optional<std::u16string>(u"best fit"),
          runtime)
          ->getValue();
  if (LLVM_UNLIKELY(matcher == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  std::vector<std::u16string> supportedLocales;
  // Skip 3/4, as don't need to have independant implementations for best fit
  // a. Let supportedLocales be LookupSupportedLocales(availableLocales,
  // requestedLocales).
  supportedLocales = lookupSupportedLocales(availableLocales, requestedLocales);
  // 5. Return CreateArrayFromList(supportedLocales).
  return supportedLocales;
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

// Implementation of
// https://tc39.es/ecma402/#datetimeformat-objects
struct DateTimeFormat::Impl {
  // Table 7
  // https://tc39.es/ecma402/#table-datetimeformat-resolvedoptions-properties
  // For DateTimeFormat::impl_ in initialize
  std::u16string FormatMatcher, HourCycle, Weekday, Era, Year, Month, Day,
      DayPeriod, Hour, Minute, Second, TimeZone, TimeZoneName, DateStyle,
      TimeStyle, Calendar, NumberingSystem, locale;
  uint8_t FractionalSecondDigits;
  bool hour12;
};

DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}
DateTimeFormat::~DateTimeFormat() {}

// Implementation of
// https://tc39.es/ecma402/#sec-resolvelocale
std::u16string resolveAlias(
    const std::u16string &value,
    const std::map<std::u16string, std::u16string> &mappings) {
  if (mappings.find(value) == mappings.end()) {
    return value;
  } else {
    return mappings.at(value);
  }
}
std::u16string resolveKnownAliases(
    const std::u16string &key,
    const std::u16string &value) {
  if (key == u"ca") {
    return resolveAlias(value, {{u"gregorian", u"gregory"}});
  }
  if (key == u"nu") {
    return resolveAlias(value, {{u"traditional", u"traditio"}});
  }
  if (key == u"co") {
    return resolveAlias(
        value,
        {{u"dictionary", u"dict"},
         {u"phonebook", u"phonebk"},
         {u"traditional", u"trad"},
         {u"gb2312han", u"gb2312"}});
  }
  if (key == u"kn" && value == u"yes") {
    return std::u16string(u"true");
  }
  if (key == u"kn" || key == u"kf") {
    if (value == u"no") {
      return std::u16string(u"false");
    }
  }
  return u"";
}
bool containsValidKeyword(
    const std::vector<std::u16string> &values,
    const std::u16string &value) {
  if (std::binary_search(values.begin(), values.end(), value)) {
    return true;
  }
  return false;
}
bool isValidKeyword(const std::u16string &key, const std::u16string &value) {
  static std::vector<std::u16string> keys = {u"nu", u"co", u"ca"};
  auto iter = std::find(keys.begin(), keys.end(), key);
  if (iter != keys.end()) {
    return false; // invalid key
  }
  // Ref:: https://tc39.es/ecma402/#table-numbering-system-digits
  // It is a subset of
  // https://github.com/unicode-org/cldr/blob/master/common/bcp47/number.xml

  static std::vector<std::u16string> nuVector = {
      u"adlm",     u"ahom",     u"arab",     u"arabext",  u"bali",
      u"beng",     u"bhks",     u"brah",     u"cakm",     u"cham",
      u"deva",     u"diak",     u"fullwide", u"gong",     u"gonm",
      u"gujr",     u"guru",     u"hanidec",  u"hmng",     u"hmnp",
      u"java",     u"kali",     u"khmr",     u"knda",     u"lana",
      u"lanatham", u"laoo",     u"latn",     u"lepc",     u"limb",
      u"mathbold", u"mathdbl",  u"mathmono", u"mathsanb", u"mathsans",
      u"mlym",     u"modi",     u"mong",     u"mroo",     u"mtei",
      u"mymr",     u"mymrshan", u"mymrtlng", u"newa",     u"nkoo",
      u"olck",     u"orya",     u"osma",     u"rohg",     u"saur",
      u"segment",  u"shrd",     u"sind",     u"sinh",     u"sora",
      u"sund",     u"takr",     u"talu",     u"tamldec",  u"telu",
      u"thai",     u"tibt",     u"tirh",     u"vaii",     u"wara",
      u"wcho"};
  // Ref::
  // https://github.com/unicode-org/cldr/blob/release-37/common/bcp47/collation.xml
  // -- Minus "standard" & "search" which are not allowed as per spec:
  // https://tc39.es/ecma402/#sec-intl-collator-internal-slots
  static std::vector<std::u16string> coVector = {
      u"big5han",
      u"compat",
      u"dict",
      u"direct",
      u"ducet",
      u"emoji",
      u"eor",
      u"gb2312",
      u"phonebk",
      u"phonetic",
      u"pinyin",
      u"reformed",
      u"searchjl",
      u"stroke",
      u"trad",
      u"unihan",
      u"zhuyin"};
  static std::vector<std::u16string> caVector = {
      u"buddhist",
      u"chinese",
      u"coptic",
      u"dangi",
      u"ethioaa",
      u"ethiopic",
      u"gregory",
      u"hebrew",
      u"indian",
      u"islamic",
      u"islamic-umalqura",
      u"islamic-tbla",
      u"islamic-civil",
      u"islamic-rgsa",
      u"iso8601",
      u"japanese",
      u"persian",
      u"roc"};
  auto pos = iter - keys.begin();
  switch (pos) {
    case 0:
      return containsValidKeyword(nuVector, value);
    case 1:
      return containsValidKeyword(coVector, value);
    case 2:
      return containsValidKeyword(caVector, value);
    default:
      return false;
  }
};
// https://tc39.es/ecma262/#sec-stringindexof
int8_t stringIndexOf(
    const std::u16string &string,
    const std::u16string &searchValue,
    uint8_t fromIndex) {
  //  1. Let len be the length of string.
  uint8_t len = string.length();
  //  2. If searchValue is the empty String and fromIndex ≤ len, return
  //  fromIndex.
  if (searchValue == u"" && fromIndex <= len) {
    return fromIndex;
  }
  //  3. Let searchLen be the length of searchValue.
  uint8_t searchLen = searchValue.length();
  //  4. For each integer i starting with fromIndex such that i ≤ len -
  //  searchLen, in ascending order, do
  for (uint8_t i = fromIndex; i <= len - searchLen; i++) {
    //    a. Let candidate be the substring of string from i to i + searchLen.
    std::u16string candidate = string.substr(i, searchLen);
    //    b. If candidate is the same sequence of code units as searchValue,
    //    return i.
    if (candidate == searchValue) {
      return i;
    }
  }
  //  5. Return -1.
  return -1;
}
// https://tc39.es/ecma402/#sec-unicode-extension-components
std::unordered_map<std::u16string, std::u16string> unicodeExtensionComponents(
    const std::u16string &extension) {
  std::unordered_map<std::u16string, std::u16string> record;
  //  1. Let attributes be a new empty List.
  std::vector<std::u16string> attributes;
  //  2. Let keywords be a new empty List.
  std::vector<std::u16string> keywords;
  //  3. Let keyword be undefined.
  std::u16string keyword;
  //  4. Let size be the length of extension.
  auto size = extension.length();
  //  5. Let k be 3.
  uint8_t k = 3;
  uint8_t len;
  //  6. Repeat, while k < size
  while (k < size) {
    //    a. Let e be ! StringIndexOf(extension, "-", k).
    auto e = stringIndexOf(extension, u"-", k);
    //    b. If e = -1, let len be size - k; else let len be e - k.
    if (e == -1) {
      len = size - k;
    } else {
      len = e - k;
    }
    //    c. Let subtag be the String value equal to the substring of extension
    //    consisting of the code units at indices k (inclusive) through k + len
    //    (exclusive).
    std::u16string subtag = extension.substr(k, len);
    //    d. If keyword is undefined and len ≠ 2, then
    if (keyword == u"" && len != 2) {
      //    i. If subtag is not an element of attributes, then
      if (std::find(attributes.begin(), attributes.end(), subtag) ==
          attributes.end()) {
        //    1. Append subtag to attributes.
        attributes.push_back(subtag);
      }
    }
    //    e. Else if len = 2, then
    else if (len == 2) {
      //    i. If keyword is not undefined and keywords does not contain an
      //    element whose [[Key]] is the same as keyword.[[Key]], then
      if (keyword != u"u" &&
          std::find(keywords.begin(), keywords.end(), keyword) ==
              keywords.end()) {
        //    1. Append keyword to keywords.
        keywords.push_back(subtag);
        //    ii. Set keyword to the Record { [[Key]]: subtag, [[Value]]: "" }.
        // record.insert(std::pair<std::u16string, std::u16string> (subtag,
        // u""));
      }
    }
    //    f. Else,
    else {
      //    i. If keyword.[[Value]] is the empty String, then
      if (keyword == u"") {
        //    1. Set keyword.[[Value]] to subtag.
        keyword = subtag;
      }
      //    ii. Else,
      else {
        //    1. Set keyword.[[Value]] to the string-concatenation of
        //    keyword.[[Value]], "-", and subtag.
        keyword = keyword + u"-" + subtag;
      }
    }
    //    g. Let k be k + len + 1.
    k = k + len + 1;
  }
  //  7. If keyword is not undefined and keywords does not contain an element
  //  whose [[Key]] is the same as keyword.[[Key]], then
  if (keyword != u"" &&
      std::find(keywords.begin(), keywords.end(), keyword) == keywords.end()) {
    //  a. Append keyword to keywords.
    keywords.push_back(keyword);
  }
  //  8. Return the Record { [[Attributes]]: attributes, [[Keywords]]: keywords
  //  }.
  for (std::vector<std::u16string>::size_type i = 0; i != keywords.size();
       i++) {
    record.emplace(
        std::pair<std::u16string, std::u16string>(keywords[i], attributes[i]));
  }
  return record;
}
// https://tc39.es/ecma402/#sec-insert-unicode-extension-and-canonicalize
std::u16string setUnicodeExtensions(
    const std::u16string &locale,
    const std::u16string &extension) {
  std::vector<std::u16string> unicodeExtensionKeywords;
  // TODO: From BCP Parser
  // https://github.com/facebook/hermes/blob/17d7583650e5f4b59645e0607e24c7ccb15404ee/lib/Platform/Intl/java/com/facebook/hermes/intl/LocaleObjectAndroid.java#L247
  return extension;
};
// https://tc39.es/ecma402/#sec-resolvelocale
const std::unordered_map<std::u16string, std::u16string> resolveLocale(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales,
    const std::unordered_map<std::u16string, std::u16string> &opt,
    const std::vector<std::u16string> &relevantExtensionKeys
    //  In line with Java, haven't included LocaleData
) {
  //  Skip 1/2/3, as we don't need to have independant implementations for best
  //  fit
  std::unordered_map<std::u16string, std::u16string> keywords;
  auto r = lookupMatcher(requestedLocales, availableLocales);
  //  4. Let foundLocale be r.[[locale]].
  auto foundLocale = r.locale;
  //  5. Let result be a new Record.
  std::unordered_map<std::u16string, std::u16string> result;
  //  6. Set result.[[dataLocale]] to foundLocale.
  result.emplace(
      std::pair<std::u16string, std::u16string>(u"dataLocale", foundLocale));
  //  7. If r has an [[extension]] field, then
  if (r.extension != u"") {
    //  a. Let components be ! UnicodeExtensionComponents(r.[[extension]]).
    //  b. Let keywords be components.[[Keywords]].
    keywords = unicodeExtensionComponents(r.extension);
  }
  //  8. Let supportedExtension be "-u".
  std::u16string supportedExtension = u"-u";

  std::u16string value;
  //  9. For each element key of relevantExtensionKeys, do
  for (std::u16string key : relevantExtensionKeys) {
    //    a. Let foundLocaleData be localeData.[[<foundLocale>]].
    result.emplace(std::pair<std::u16string, std::u16string>(
        u"foundLocaleData", foundLocale));
    //    b. Assert: Type(foundLocaleData) is Record.
    //    c. Let keyLocaleData be foundLocaleData.[[<key>]].
    auto keyLocaleData = key;
    //    d. Assert: Type(keyLocaleData) is List.
    //    e. Let value be keyLocaleData[0].
    //    f. Assert: Type(value) is either String or Null.
    //    g. Let supportedExtensionAddition be "".
    std::u16string supportedExtensionAddition;
    //    h. If r has an [[extension]] field, then
    if (r.extension != u"") {
      //    i. If keywords contains an element whose [[Key]] is the same as key,
      //    then
      if (keywords.find(key) != keywords.end()) {
        //    1. Let entry be the element of keywords whose [[Key]] is the same
        //    as key.
        //    2. Let requestedValue be entry.[[Value]].
        std::u16string requestedValue = keywords.find(key)->second;
        //    3. If requestedValue is not the empty String, then
        if (requestedValue != u"") {
          //      a. If keyLocaleData contains requestedValue, then
          if (keyLocaleData.find(requestedValue)) {
            //      i. Let value be requestedValue.
            value = requestedValue;
            //      ii. Let supportedExtensionAddition be the
            //      string-concatenation of "-", key, "-", and value.
            supportedExtensionAddition = u"-" + key + u"-" + value;
          } else if (keyLocaleData.find(u"true")) {
            //      4. Else if keyLocaleData contains "true", then
            //      a. Let value be "true".
            value = u"true";
            //      b. Let supportedExtensionAddition be the
            //      string-concatenation of "-" and key.
            supportedExtensionAddition = u"-" + key;
          }
        }
      }
      //      i. If options has a field [[<key>]], then
      if (opt.find(key) != opt.end()) {
        //      i. Let optionsValue be options.[[<key>]].
        auto optionsValue = opt.find(key);
        //      ii. Assert: Type(optionsValue) is either String, Undefined, or
        //      Null. iii. If Type(optionsValue) is String, then
        if (optionsValue->second != u"") {
          // 1. Let optionsValue be the string optionsValue after performing the
          // algorithm steps to transform Unicode extension values to canonical
          // syntax per Unicode Technical Standard #35 LDML § 3.2.1 Canonical
          // Unicode Locale Identifiers, treating key as ukey and optionsValue
          // as uvalue productions.
          std::u16string resolveOptionsValue =
              resolveKnownAliases(key, optionsValue->second);
          // 2. Let optionsValue be the string optionsValue after performing the
          // algorithm steps to replace Unicode extension values with their
          // canonical form per Unicode Technical Standard #35 LDML § 3.2.1
          // Canonical Unicode Locale Identifiers, treating key as ukey and
          // optionsValue as uvalue productions.
          if (isValidKeyword(key, resolveOptionsValue)) {
            //          3. If optionsValue is the empty String, then
            //          a. Let optionsValue be "true".
            if (optionsValue->second == u"") {
              value = u"true";
            }
          }
          //   iv. If keyLocaleData contains optionsValue, then
          if (keyLocaleData.find(optionsValue->second)) {
            //   1. If SameValue(optionsValue, value) is false, then
            if (optionsValue->second != value) {
              //   a. Let value be optionsValue.
              value = optionsValue->second;
              //   b. Let supportedExtensionAddition be "".
              supportedExtensionAddition = u"";
            }
          }
        }
        //    j. Set result.[[<key>]] to value.
        result.emplace(std::pair<std::u16string, std::u16string>(key, value));
        //    k. Append supportedExtensionAddition to supportedExtension.
        supportedExtension += supportedExtensionAddition;
      }
    }
    //    10. If the number of elements in supportedExtension is greater than 2,
    //    then
    if (supportedExtension.size() > 2) {
      //    a. Let foundLocale be
      //    InsertUnicodeExtensionAndCanonicalize(foundLocale,
      //    supportedExtension).
      // TODO: setUnicodeExtensions
      foundLocale = setUnicodeExtensions(foundLocale, supportedExtension);
    }
  }
  //  11. Set result.[[locale]] to foundLocale.
  result.emplace(
      std::pair<std::u16string, std::u16string>(u"locale", foundLocale));
  //  12. Return result.
  return result;
}
// Implementation of
// https://tc39.es/ecma402/#sec-intl.datetimeformat.supportedlocalesof
vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let availableLocales be %DateTimeFormat%.[[AvailableLocales]].
  // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  auto requestedLocales = getCanonicalLocales(runtime, locales);
  std::vector<std::u16string> availableLocales = getAvailableLocales();
  // 3. Return ? (availableLocales, requestedLocales, options).
  return supportedLocales(
      availableLocales, requestedLocales.getValue(), options, runtime);
}

// Implementation of
// https://tc39.es/ecma402/#sec-todatetimeoptions
vm::CallResult<Options> toDateTimeOptions(
    const Options &options,
    const std::u16string &required,
    const std::u16string &defaults,
    vm::Runtime *runtime) {
  // 1. If options is undefined, let options be null; otherwise let options be ?
  // ToObject(options).
  // 2. Let options be OrdinaryObjectCreate(options).
  Options copyOfOptions = options;
  // 3. Let needDefaults be true.
  bool needDefaults = true;
  // 4. If required is "date" or "any", then
  if (required == u"date" || required == u"any") {
    // a. For each property name prop of « "weekday", "year", "month", "day" »,
    // do
    for (std::u16string prop : {u"weekday", u"year", u"month", u"day"}) {
      // i. Let value be ? Get(options, prop).
      if (copyOfOptions.find(prop) != copyOfOptions.end()) {
        // ii. If value is not undefined, let needDefaults be false.
        needDefaults = false;
      }
    }
  }
  // 5. If required is "time" or "any", then
  if (required == u"time" || required == u"any") {
    // a. For each property name prop of « "dayPeriod", "hour", "minute",
    // "second", "fractionalSecondDigits" », do
    for (std::u16string prop :
         {u"dayPeriod",
          u"hour",
          u"minute",
          u"second",
          u"fractionalSecondDigits"}) {
      // i. Let value be ? Get(options, prop).
      if (copyOfOptions.find(prop) != copyOfOptions.end()) {
        // ii. If value is not undefined, let needDefaults be false.
        needDefaults = false;
      }
    }
  }
  // 6. Let dateStyle be ? Get(options, "dateStyle").
  auto dateStyle = copyOfOptions.find(u"dateStyle");
  // 7. Let timeStyle be ? Get(options, "timeStyle").
  auto timeStyle = copyOfOptions.find(u"timeStyle");
  // 8. If dateStyle is not undefined or timeStyle is not undefined, let
  // needDefaults be false.
  if (dateStyle != copyOfOptions.end() || timeStyle != copyOfOptions.end()) {
    needDefaults = false;
  }
  // 9. If required is "date" and timeStyle is not undefined, then
  if (required == u"date" && timeStyle != copyOfOptions.end()) {
    // a. Throw a TypeError exception.
    return runtime->raiseTypeError("timeStyle is not defined");
  }
  // 10. If required is "time" and dateStyle is not undefined, then
  if (required == u"time" && dateStyle != copyOfOptions.end()) {
    // a. Throw a TypeError exception.
    return runtime->raiseTypeError("dateStyle is not defined");
  }
  // 11. If needDefaults is true and defaults is either "date" or "all", then
  if (needDefaults && (defaults == u"date" || defaults == u"all")) {
    // a. For each property name prop of « "year", "month", "day" », do
    for (std::u16string prop : {u"year", u"month", u"day"}) {
      // i. Perform ? CreateDataPropertyOrThrow(options, prop, "numeric").
      copyOfOptions.emplace(
          std::u16string(prop), Option(std::u16string(u"numeric")));
    }
  }
  // 12. If needDefaults is true and defaults is either "time" or "all", then
  if (needDefaults && (defaults == u"time" || defaults == u"all")) {
    // a. For each property name prop of « "hour", "minute", "second" », do
    for (std::u16string prop : {u"hour", u"minute", u"second"}) {
      // i. Perform ? CreateDataPropertyOrThrow(options, prop, "numeric").
      copyOfOptions.emplace(
          std::u16string(prop), Option(std::u16string(u"numeric")));
    }
  }
  // 13. return options
  return copyOfOptions;
}
// https://tc39.es/ecma402/#sec-case-sensitivity-and-case-mapping
std::u16string normalizeTimeZoneName(std::u16string timeZoneName) {
  std::u16string normalized;
  std::uint8_t offset = 'a' - 'A';
  for (std::uint8_t idx = 0; idx < timeZoneName.length(); idx++) {
    unsigned char c = timeZoneName[idx];
    if (c >= 'a' && c <= 'z') {
      normalized.push_back((char)c - offset);
    } else {
      normalized.push_back(c);
    }
  }
  return normalized;
}
std::u16string getDefaultTimeZone(NSLocale *locale) {
  NSTimeZone *defaultTimeZone = [NSTimeZone defaultTimeZone];
  NSString *nstrTime =
      [defaultTimeZone localizedName:NSTimeZoneNameStyleShortStandard
                              locale:locale];
  return nsStringToU16String(nstrTime);
}
static NSDictionary<NSString *, NSString *> *getDictOfTimeZoneNames() {
  return [NSTimeZone abbreviationDictionary];
}
bool isTimeZoneValid(const std::u16string &timeZoneValue) {
  static NSDictionary<NSString *, NSString *> *dictListOfTimeZones =
      getDictOfTimeZoneNames();
  NSString *NSTimeValue = u16StringToNSString(timeZoneValue);
  for (NSString *key in dictListOfTimeZones) {
    if ([key isEqualToString:NSTimeValue]) {
      return true;
    }
  }
  return false;
}
// https://github.com/facebook/hermes/blob/33cf15ab318694423926c3762e001465b399ef38/lib/Platform/Intl/java/com/facebook/hermes/intl/PlatformDateTimeFormatterAndroid.java#L121
std::u16string searchForDateFormatPattern(
    const std::u16string &dateFormatPattern,
    const std::vector<std::u16string> &values) {
  if (dateFormatPattern.find(u"h") < dateFormatPattern.length()) {
    return u"h12";
  } else if (dateFormatPattern.find(u"K") < dateFormatPattern.length()) {
    return u"h11";
  } else if (dateFormatPattern.find(u"H") < dateFormatPattern.length()) {
    return u"h23";
  } else {
    return u"h24";
  }
}
std::u16string getDefaultHourCycle(NSLocale *locale) {
  static std::u16string dateFormatPattern =
      nsStringToU16String([NSDateFormatter dateFormatFromTemplate:@"j"
                                                          options:0
                                                           locale:locale]);
  return searchForDateFormatPattern(dateFormatPattern, {u"h", u"K", u"H", u""});
}
// Implementation of
// https://tc39.es/ecma402/#sec-initializedatetimeformat
vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  const vm::CallResult<std::vector<std::u16string>> requestedLocales =
      getCanonicalLocales(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  // 2. Let options be ? ToDateTimeOptions(options, "any", "date").
  auto optionsTo = toDateTimeOptions(options, u"any", u"date", runtime);
  if (optionsTo == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  auto optionsToUse = optionsTo.getValue();
  // 3. Let opt be a new Record.
  std::unordered_map<std::u16string, std::u16string> mapOpt;
  // 4. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  //   "lookup", "best fit" », "best fit").
  auto matcher = getOptionString(
      optionsToUse,
      u"localeMatcher",
      {u"lookup", u"best fit"},
      llvh::Optional<std::u16string>(u"best fit"),
      runtime);
  // 5. Set opt.[[localeMatcher]] to matcher.
  mapOpt.emplace(std::pair<std::u16string, std::u16string>(
      u"localeMatcher", matcher->getValue()));
  // 6. Let calendar be ? GetOption(options, "calendar", "string",
  //    undefined, undefined).
  auto calendar = getOptionString(
      optionsToUse, u"calendar", {}, llvh::Optional<std::u16string>(), runtime);
  // 7. If calendar is not undefined, then
  //    a. If calendar does not match the Unicode Locale Identifier type
  //       nonterminal, throw a RangeError exception.
  // 8. Set opt.[[ca]] to calendar.
  if (calendar == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Incorrect calendar information provided");
  }
  mapOpt.emplace(std::pair<std::u16string, std::u16string>(u"ca", u""));
  mapOpt.emplace(std::pair<std::u16string, std::u16string>(u"nu", u""));
  mapOpt.emplace(std::pair<std::u16string, std::u16string>(u"hc", u""));
  if (calendar->hasValue()) {
    mapOpt.find(u"ca")->second = calendar->getValue();
  }
  // 9. Let numberingSystem be ? GetOption(options, "numberingSystem",
  //    "string", undefined, undefined).
  auto numberingSystem = getOptionString(
      optionsToUse,
      u"numberingSystem",
      {},
      llvh::Optional<std::u16string>(),
      runtime);
  // 10. If numberingSystem is not undefined, then
  //     a. If numberingSystem does not match the Unicode Locale Identifier
  //        type nonterminal, throw a RangeError exception.
  // 11. Set opt.[[nu]] to numberingSystem.
  if (numberingSystem == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError(
        "Incorrect numberingSystem information provided");
  }
  if (numberingSystem->hasValue()) {
    mapOpt.find(u"nu")->second = numberingSystem->getValue();
  }
  // 12. Let hour12 be ? GetOption(options, "hour12", "boolean",
  //     undefined, undefined).
  auto hour12 =
      getOptionBool(optionsToUse, u"hour12", llvh::Optional<bool>(), runtime);
  if (hour12 == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  // 13. Let hourCycle be ? GetOption(options, "hourCycle", "string", «
  //     "h11", "h12", "h23", "h24" », undefined).
  auto hourCycle = getOptionString(
      optionsToUse,
      u"hourCycle",
      {u"h11", u"h12", u"h23", u"h24"},
      llvh::Optional<std::u16string>(),
      runtime);
  // 14. If hour12 is not undefined, then
  //     a. Let hourCycle be null.
  if (hour12->hasValue()) {
    if (hourCycle->hasValue()) {
      hourCycle->getValue() = u"";
      // 15. Set opt.[[hc]] to hourCycle.
      mapOpt.find(u"hc")->second = hourCycle->getValue();
    }
  }
  // 16 - 23
  auto r = resolveLocale(
      locales, requestedLocales.getValue(), mapOpt, {u"ca", u"nu", u"hc"});
  if (r.find(u"locale") != r.end()) {
    impl_->locale = r.find(u"locale")->second;
  }
  // Get default locale as NS for future defaults
  NSLocale *defaultNSLocale = [[NSLocale alloc]
      initWithLocaleIdentifier:u16StringToNSString(impl_->locale)];
  bool useDefaultCalendar;
  std::u16string calendarResolved;
  if (r.find(u"ca") != r.end()) {
    calendarResolved = r.find(u"ca")->second;
  }
  if (calendarResolved != u"") {
    useDefaultCalendar = false;
    impl_->Calendar = calendarResolved;
  } else {
    useDefaultCalendar = true;
    impl_->Calendar = u"gregory";
  }
  bool useDefaultNumberSystem;
  std::u16string numberingSystemResolved;
  if (r.find(u"nu") != r.end()) {
    numberingSystemResolved = r.find(u"nu")->second;
  }
  if (numberingSystemResolved != u"") {
    useDefaultNumberSystem = false;
    impl_->NumberingSystem = numberingSystemResolved;
  } else {
    useDefaultNumberSystem = true;
    impl_->NumberingSystem = u"latn";
  }
  std::u16string hourCycleResolved;
  if (r.find(u"hc") != r.end()) {
    hourCycleResolved = r.find(u"hc")->second;
  }

  // 24-27
  //  24. Let timeZone be ? Get(options, "timeZone").
  std::u16string timeZoneValue;
  //  Check find first to avoid error
  //  25. If timeZone is undefined, then
  auto timeZoneIter = options.find(u"timeZone");
  if (timeZoneIter == options.end()) {
    //  a. Let timeZone be DefaultTimeZone(), i.e. CST.
    timeZoneValue = getDefaultTimeZone(defaultNSLocale);
  }
  //  26. Else,
  else {
    //  a. Let timeZone be ? ToString(timeZone).
    timeZoneValue = normalizeTimeZoneName(timeZoneIter->second.getString());
    bool validateTimeZone = isTimeZoneValid(timeZoneValue);
    //  b. If the result of IsValidTimeZoneName(timeZone) is false, then
    if (!validateTimeZone) {
      //  i. Throw a RangeError exception.
      return runtime->raiseRangeError(
          "Incorrect timeZone information provided");
    }
  }
  impl_->TimeZone = timeZoneValue;
  // 28 - 34
  auto formatMatcherOption = getOptionString(
      optionsToUse,
      u"formatMatcher",
      {u"basic", u"best fit"},
      llvh::Optional<std::u16string>(u"best fit"),
      runtime);
  if (formatMatcherOption == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  // Expected to be a string 'best fit' by default
  impl_->FormatMatcher = formatMatcherOption->getValue();
  // 33. Set dateTimeFormat.[[DateStyle]] to dateStyle.
  auto dateStyleOption = getOptionString(
      optionsToUse,
      u"dateStyle",
      {u"full", u"long", u"medium", u"short"},
      llvh::Optional<std::u16string>(),
      runtime);
  if (dateStyleOption == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  if (dateStyleOption->hasValue()) {
    impl_->DateStyle = dateStyleOption->getValue();
  }
  // 34. Let timeStyle be ? GetOption(options, "timeStyle", "string", « "full",
  // "long", "medium", "short" », undefined).
  // 35. Set dateTimeFormat.[[TimeStyle]] to timeStyle.
  auto timeStyleOption = getOptionString(
      optionsToUse,
      u"timeStyle",
      {u"full", u"long", u"medium", u"short"},
      llvh::Optional<std::u16string>(),
      runtime);
  if (timeStyleOption == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  if (timeStyleOption->hasValue()) {
    impl_->TimeStyle = timeStyleOption->getValue();
  }
  auto fractionalSecondDigitsOption = getNumberOption(
      optionsToUse,
      u"fractionalSecondDigits",
      1,
      3,
      llvh::Optional<uint8_t>(),
      runtime);
  auto weekdayOption = getOptionString(
      optionsToUse,
      u"weekday",
      {u"long", u"short", u"narrow"},
      llvh::Optional<std::u16string>(),
      runtime);
  auto eraOption = getOptionString(
      optionsToUse,
      u"era",
      {u"long", u"short", u"narrow"},
      llvh::Optional<std::u16string>(),
      runtime);
  auto yearOption = getOptionString(
      optionsToUse,
      u"year",
      {u"numeric", u"2-digit"},
      llvh::Optional<std::u16string>(),
      runtime);
  auto monthOption = getOptionString(
      optionsToUse,
      u"month",
      {u"numeric", u"2-digit", u"long", u"short", u"narrow"},
      llvh::Optional<std::u16string>(),
      runtime);
  auto dayOption = getOptionString(
      optionsToUse,
      u"day",
      {u"numeric", u"2-digit"},
      llvh::Optional<std::u16string>(),
      runtime);
  auto hourOption = getOptionString(
      optionsToUse,
      u"hour",
      {u"numeric", u"2-digit"},
      llvh::Optional<std::u16string>(),
      runtime);
  auto minuteOption = getOptionString(
      optionsToUse,
      u"minute",
      {u"numeric", u"2-digit"},
      llvh::Optional<std::u16string>(),
      runtime);
  auto secondOption = getOptionString(
      optionsToUse,
      u"second",
      {u"numeric", u"2-digit"},
      llvh::Optional<std::u16string>(),
      runtime);
  auto timeZoneNameOption = getOptionString(
      optionsToUse,
      u"timeZoneName",
      {u"long",
       u"short",
       u"shortOffset",
       u"longOffset",
       u"shortGeneric",
       u"longGeneric"},
      llvh::Optional<std::u16string>(),
      runtime);
  auto dayPeriodOption = getOptionString(
      optionsToUse,
      u"dayPeriod",
      {u"long", u"short", u"narrow"},
      llvh::Optional<std::u16string>(),
      runtime);
  std::vector<vm::CallResult<llvh::Optional<std::u16string>>> optionsVector = {
      weekdayOption,
      eraOption,
      yearOption,
      monthOption,
      dayOption,
      hourOption,
      minuteOption,
      secondOption,
      timeZoneNameOption,
      dayPeriodOption};
  // 36. If dateStyle is not undefined or timeStyle is not undefined, then
  if (timeStyleOption->hasValue() || dateStyleOption->hasValue()) {
    // a. For each row in Table 4, except the header row, do
    // i. Let prop be the name given in the Property column of the row.
    // ii. Let p be opt.[[<prop>]].
    // iii. If p is not undefined, then
    // 1. Throw a TypeError exception. (timeStyle/dateStyle can't be used with
    // table 4 options)
    if (fractionalSecondDigitsOption->hasValue()) {
      return runtime->raiseTypeError(
          "Invalid option of fractionalSecondDigits with timeStyle/dateStyle option");
    }
    for (auto const &value : optionsVector) {
      if (value->hasValue()) {
        return runtime->raiseTypeError(
            vm::TwineChar16("Invalid option of ") +
            vm::TwineChar16(value->getValue().c_str()) +
            vm::TwineChar16(" with timeStyle/dateStyle option"));
      }
    }
  }
  // 38. For each row in Table 4, except the header row, in table order, do
  // a. Let prop be the name given in the Property column of the row.
  // b. If bestFormat has a field [[<prop>]], then
  // i. Let p be bestFormat.[[<prop>]].
  // ii. Set dateTimeFormat's internal slot whose name is the Internal Slot
  // column of the row to p
  if (fractionalSecondDigitsOption.getStatus() ==
      vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  if (fractionalSecondDigitsOption->hasValue()) {
    impl_->FractionalSecondDigits = fractionalSecondDigitsOption->getValue();
  }
  for (auto const &value : optionsVector) {
    if (value.getStatus() == vm::ExecutionStatus::EXCEPTION) {
      return vm::ExecutionStatus::EXCEPTION;
    }
  }
  if (weekdayOption->hasValue()) {
    impl_->Weekday = weekdayOption->getValue();
  }
  if (eraOption->hasValue()) {
    impl_->Era = eraOption->getValue();
  }
  if (yearOption->hasValue()) {
    impl_->Year = yearOption->getValue();
  }
  if (monthOption->hasValue()) {
    impl_->Month = monthOption->getValue();
  }
  if (dayOption->hasValue()) {
    impl_->Day = dayOption->getValue();
  }
  if (hourOption->hasValue()) {
    impl_->Hour = hourOption->getValue();
  }
  if (minuteOption->hasValue()) {
    impl_->Minute = minuteOption->getValue();
  }
  if (secondOption->hasValue()) {
    impl_->Second = secondOption->getValue();
  }
  if (timeZoneNameOption->hasValue()) {
    impl_->TimeZoneName = timeZoneNameOption->getValue();
  }
  if (dayPeriodOption->hasValue()) {
    impl_->DayPeriod = dayPeriodOption->getValue();
  }

  // 39
  if (impl_->Hour == u"") {
    impl_->HourCycle = u"";
  } else {
    std::u16string hcDefaultNS = getDefaultHourCycle(defaultNSLocale);
    mapOpt.insert(
        std::pair<std::u16string, std::u16string>(u"hcDefault", hcDefaultNS));
    if (hourCycleResolved == u"") {
      mapOpt.find(u"hc")->second = hcDefaultNS;
    } else {
      mapOpt.find(u"hc")->second = hourCycleResolved;
    }
    if (hour12->hasValue()) {
      if (hour12->getValue() == true) { // true
        if (hcDefaultNS == u"h11" || hcDefaultNS == u"h23") {
          mapOpt.find(u"hc")->second = u"h11";
        } else {
          mapOpt.find(u"hc")->second = u"h12";
        }
      } else {
        if (hcDefaultNS == u"h11" || hcDefaultNS == u"h23") {
          mapOpt.find(u"hc")->second = u"h23";
        } else {
          mapOpt.find(u"hc")->second = u"h24";
        }
      }
    }
    impl_->HourCycle = mapOpt.find(u"hc")->second;
  }
  return vm::ExecutionStatus::RETURNED;
}
// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sec-intl.datetimeformat.prototype.resolvedoptions
Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));

  options.emplace(u"numberingSystem", Option(impl_->NumberingSystem));
  options.emplace(u"calendar", Option(impl_->Calendar));
  options.emplace(u"timeZone", Option(impl_->TimeZone));

  if (impl_->HourCycle != u"") {
    options.emplace(u"hourCycle", Option(impl_->HourCycle));
    if (impl_->HourCycle == u"h11" || impl_->HourCycle == u"h12") {
      options.emplace(u"hour12", true);
    } else {
      options.emplace(u"hour12", false);
    }
  }
  if (impl_->Weekday != u"") {
    options.emplace(u"weekday", Option(impl_->Weekday));
  }
  if (impl_->Era != u"") {
    options.emplace(u"era", Option(impl_->Era));
  }
  if (impl_->Year != u"") {
    options.emplace(u"year", Option(impl_->Year));
  }
  if (impl_->Month != u"") {
    options.emplace(u"month", Option(impl_->Month));
  }
  if (impl_->Day != u"") {
    options.emplace(u"day", Option(impl_->Day));
  }
  if (impl_->Hour != u"") {
    options.emplace(u"hour", Option(impl_->Hour));
  }
  if (impl_->Minute != u"") {
    options.emplace(u"minute", Option(impl_->Minute));
  }
  if (impl_->Second != u"") {
    options.emplace(u"second", Option(impl_->Second));
  }
  if (impl_->TimeZoneName != u"") {
    options.emplace(u"timeZoneName", Option(impl_->TimeZoneName));
  }
  if (impl_->DateStyle != u"") {
    options.emplace(u"dateStyle", Option(impl_->DateStyle));
  }
  if (impl_->TimeStyle != u"") {
    options.emplace(u"timeStyle", Option(impl_->TimeStyle));
  }
  return options;
}

enum enum_string {
  eLong,
  eShort,
  eNarrow,
  eMedium,
  eFull,
  eBasic,
  eBestFit,
  eNumeric,
  eTwoDigit,
  eShortOffset,
  eLongOffset,
  eShortGeneric,
  eLongGeneric,
  eNull
};
enum_string formatDate(std::u16string const &inString) {
  if (inString == u"long")
    return eLong;
  if (inString == u"short")
    return eShort;
  if (inString == u"narrow")
    return eNarrow;
  if (inString == u"medium")
    return eMedium;
  if (inString == u"full")
    return eFull;
  if (inString == u"basic")
    return eBasic;
  if (inString == u"best fit")
    return eBestFit;
  if (inString == u"numeric")
    return eNumeric;
  if (inString == u"2-digit")
    return eTwoDigit;
  if (inString == u"shortOffset")
    return eShortOffset;
  if (inString == u"longOffset")
    return eLongOffset;
  if (inString == u"shortGeneric")
    return eShortGeneric;
  if (inString == u"longGeneric")
    return eLongGeneric;
  return eNull;
};
std::u16string DateTimeFormat::getNSDateFormat() noexcept {
  NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];

  if (impl_->TimeStyle != u"") {
    switch (formatDate(impl_->TimeStyle)) {
      case eFull:
        dateFormatter.timeStyle = NSDateFormatterFullStyle;
        break;
      case eLong:
        dateFormatter.timeStyle = NSDateFormatterLongStyle;
        break;
      case eMedium:
        dateFormatter.timeStyle = NSDateFormatterMediumStyle;
        break;
      case eShort:
        dateFormatter.timeStyle = NSDateFormatterShortStyle;
        break;
      default:
        dateFormatter.timeStyle = NSDateFormatterShortStyle;
    }
  }
  if (impl_->DateStyle != u"") {
    switch (formatDate(impl_->DateStyle)) {
      case eFull:
        dateFormatter.dateStyle = NSDateFormatterFullStyle;
        break;
      case eLong:
        dateFormatter.dateStyle = NSDateFormatterLongStyle;
        break;
      case eMedium:
        dateFormatter.dateStyle = NSDateFormatterMediumStyle;
        break;
      case eShort:
        dateFormatter.dateStyle = NSDateFormatterShortStyle;
        break;
      default:
        dateFormatter.dateStyle = NSDateFormatterShortStyle;
    }
  }
  if (impl_->TimeZone != u"") {
    dateFormatter.timeZone =
        [[NSTimeZone alloc] initWithName:u16StringToNSString(impl_->TimeZone)];
  }
  if (impl_->locale != u"") {
    dateFormatter.locale = [[NSLocale alloc]
        initWithLocaleIdentifier:u16StringToNSString(impl_->locale)];
  }
  if (impl_->Calendar != u"") {
    dateFormatter.calendar = [[NSCalendar alloc]
        initWithCalendarIdentifier:u16StringToNSString(impl_->Calendar)];
  }
  // TODO: Set numberingSystem if not default latn(0-9)

  // The following options cannot be used in conjunction with timeStyle or
  // dateStyle
  // Form a custom format string It will be reordered according to
  // locale later
  NSMutableString *customFormattedDate = [[NSMutableString alloc] init];
  if (impl_->TimeZoneName != u"") {
    switch (formatDate(impl_->TimeZoneName)) {
      case eShort: {
        [customFormattedDate appendString:@"z"];
        break;
      }
      case eLong: {
        [customFormattedDate appendString:@"zzzz"];
        break;
      }
      case eShortOffset: {
        [customFormattedDate appendString:@"O"];
        break;
      }
      case eLongOffset: {
        [customFormattedDate appendString:@"OOOO"];
        break;
      }
      case eShortGeneric: {
        [customFormattedDate appendString:@"v"];
        break;
      }
      case eLongGeneric: {
        [customFormattedDate appendString:@"vvvv"];
        break;
      }
      default: {
        [customFormattedDate appendString:@"z"];
      }
    }
  }
  if (impl_->Era != u"") {
    switch (formatDate(impl_->Era)) {
      case eNarrow: {
        [customFormattedDate appendString:@"GGGGG"];
        break;
      }
      case eShort: {
        [customFormattedDate appendString:@"G"];
        break;
      }
      case eLong: {
        [customFormattedDate appendString:@"GGGG"];
        break;
      }
      default: {
        [customFormattedDate appendString:@"G"];
      }
    }
  }
  if (impl_->Year != u"") {
    switch (formatDate(impl_->Year)) {
      case eNumeric: {
        [customFormattedDate appendString:@"yyyy"];
        break;
      }
      case eTwoDigit: {
        [customFormattedDate appendString:@"yy"];
        break;
      }
      default: {
        [customFormattedDate appendString:@"yyyy"];
      }
    }
  }
  if (impl_->Month != u"") {
    switch (formatDate(impl_->Month)) {
      case eNarrow: {
        [customFormattedDate appendString:@"MMMMM"];
        break;
      }
      case eNumeric: {
        [customFormattedDate appendString:@"M"];
        break;
      }
      case eTwoDigit: {
        [customFormattedDate appendString:@"MM"];
        break;
      }
      case eShort: {
        [customFormattedDate appendString:@"MMM"];
        break;
      }
      case eLong: {
        [customFormattedDate appendString:@"MMMM"];
        break;
      }
      default: {
        [customFormattedDate appendString:@"MMM"];
      }
    }
  }
  if (impl_->Weekday != u"") {
    switch (formatDate(impl_->Weekday)) {
      case eNarrow: {
        [customFormattedDate appendString:@"EEEEE"];
        break;
      }
      case eShort: {
        [customFormattedDate appendString:@"E"];
        break;
      }
      case eLong: {
        [customFormattedDate appendString:@"EEEE"];
        break;
      }
      default: {
        [customFormattedDate appendString:@"E"];
      }
    }
  }
  if (impl_->Day != u"") {
    switch (formatDate(impl_->Day)) {
      case eNumeric: {
        [customFormattedDate appendString:@"d"];
        break;
      }
      case eTwoDigit: {
        [customFormattedDate appendString:@"dd"];
        break;
      }
      default: {
        [customFormattedDate appendString:@"dd"];
      }
    }
  }
  if (impl_->Hour != u"") {
    // Ignore the hour12 bool in the impl_ struct
    // a = AM/PM for 12 hr clocks, automatically added depending on locale
    // AM/PM not multilingual, de-DE should be "03 Uhr" not "3 AM"
    // K = h11 = 0-11
    // h = h12 = 1-12
    // H = h23 = 0-23
    // k = h24 = 1-24
    if (impl_->HourCycle == u"h12") {
      switch (formatDate(impl_->Hour)) {
        case eNumeric: {
          [customFormattedDate appendString:@"h"];
          break;
        }
        case eTwoDigit: {
          [customFormattedDate appendString:@"hh"];
          break;
        }
        default: {
          [customFormattedDate appendString:@"hh"];
        }
      }
    } else if (impl_->HourCycle == u"h24") {
      switch (formatDate(impl_->Hour)) {
        case eNumeric: {
          [customFormattedDate appendString:@"k"];
          break;
        }
        case eTwoDigit: {
          [customFormattedDate appendString:@"kk"];
          break;
        }
        default: {
          [customFormattedDate appendString:@"kk"];
        }
      }
    } else if (impl_->HourCycle == u"h11") {
      switch (formatDate(impl_->Hour)) {
        case eNumeric: {
          [customFormattedDate appendString:@"K"];
          break;
        }
        case eTwoDigit: {
          [customFormattedDate appendString:@"KK"];
          break;
        }
        default: {
          [customFormattedDate appendString:@"KK"];
        }
      }
    } else { // h23
      switch (formatDate(impl_->Hour)) {
        case eNumeric: {
          [customFormattedDate appendString:@"H"];
          break;
        }
        case eTwoDigit: {
          [customFormattedDate appendString:@"HH"];
          break;
        }
        default: {
          [customFormattedDate appendString:@"HH"];
        }
      }
    }
  }
  if (impl_->Minute != u"") {
    switch (formatDate(impl_->Minute)) {
      case eNumeric: {
        [customFormattedDate appendString:@"m"];
        break;
      }
      case eTwoDigit: {
        [customFormattedDate appendString:@"mm"];
        break;
      }
      default: {
        [customFormattedDate appendString:@"m"];
      }
    }
  }
  if (impl_->Second != u"") {
    switch (formatDate(impl_->Second)) {
      case eNumeric: {
        [customFormattedDate appendString:@"s"];
        break;
      }
      case eTwoDigit: {
        [customFormattedDate appendString:@"ss"];
        break;
      }
      default: {
        [customFormattedDate appendString:@"s"];
      }
    }
  }
  if (impl_->FractionalSecondDigits != NAN) {
    // This currently outputs to 3 digits only with the date?
    switch (impl_->FractionalSecondDigits) {
      case 1: {
        [customFormattedDate appendString:@"S"];
      }
      case 2: {
        [customFormattedDate appendString:@"SS"];
      }
      case 3: {
        [customFormattedDate appendString:@"SSS"];
      }
    }
  }
  // Not supported - dayPeriod (at night/in the morning)
  // Set the custom date from all the concatonated NSStrings (locale will
  // automatically seperate the order) Only set a template format if it isn't
  // empty
  if (customFormattedDate.length > 0) {
    [dateFormatter setLocalizedDateFormatFromTemplate:customFormattedDate];
  }

  NSString *df = dateFormatter.dateFormat;
  return nsStringToU16String(df);
}
std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  auto timeInSeconds = jsTimeValue / 1000;
  NSDate *date = [NSDate dateWithTimeIntervalSince1970:timeInSeconds];
  NSString *df = u16StringToNSString(getNSDateFormat());
  NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
  dateFormatter.dateFormat = df;
  // Resetting these as it wouldn't pass through getNSDateFormat
  if (impl_->TimeZone != u"") {
    dateFormatter.timeZone =
        [[NSTimeZone alloc] initWithName:u16StringToNSString(impl_->TimeZone)];
  }
  if (impl_->locale != u"") {
    dateFormatter.locale = [[NSLocale alloc]
        initWithLocaleIdentifier:u16StringToNSString(impl_->locale)];
  }
  if (impl_->Calendar != u"") {
    dateFormatter.calendar = [[NSCalendar alloc]
        initWithCalendarIdentifier:u16StringToNSString(impl_->Calendar)];
  }
  // If no options have been set, date will be empty, so set a format for
  // default
  if ([[dateFormatter stringFromDate:date] isEqual:@""]) {
    dateFormatter.dateStyle = NSDateFormatterShortStyle;
  }
  return nsStringToU16String([dateFormatter stringFromDate:date]);
}
std::u16string returnTypeOfDate(const char &formatter) {
  if (formatter == 'a') {
    return u"dayPeriod";
  }
  if (formatter == 'z' || formatter == 'v' || formatter == 'O') {
    return u"timeZoneName";
  }
  if (formatter == 'G') {
    return u"era";
  }
  if (formatter == 'y') {
    return u"year";
  }
  if (formatter == 'M') {
    return u"month";
  }
  if (formatter == 'E') {
    return u"weekday";
  }
  if (formatter == 'd') {
    return u"day";
  }
  if (formatter == 'h' || formatter == 'k' || formatter == 'K' ||
      formatter == 'H') {
    return u"hour";
  }
  if (formatter == 'm') {
    return u"minute";
  }
  if (formatter == 's') {
    return u"second";
  }
  if (formatter == 'S') {
    return u"fractionalSecond";
  }
  return u"literal";
}
std::u16string removeDuplicatesFromString(const std::u16string &s) {
  std::u16string uniqueLetters;
  std::unordered_set<char> exists;
  for (const auto &el : s) {
    if (exists.insert(el).second) {
      uniqueLetters += el;
    }
  }
  return uniqueLetters;
}

// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sec-formatdatetimetoparts
std::vector<std::unordered_map<std::u16string, std::u16string>>
DateTimeFormat::formatToParts(double jsTimeValue) noexcept {
  std::vector<std::unordered_map<std::u16string, std::u16string>> formatMap;
  std::unordered_map<std::u16string, std::u16string> part;
  std::vector<std::u16string> splitAnswer;
  // Get custom formatted string i.e. dd-mm-yyyy
  std::u16string formatter = getNSDateFormat();
  // Remove punctuation and make elements unique
  std::remove_if(formatter.begin(), formatter.end(), ispunct);
  std::remove_if(formatter.begin(), formatter.end(), isspace);
  std::u16string uniqueFormat = removeDuplicatesFromString(formatter);
  // Get the answer in string form i.e. 01/02/2020
  std::u16string formattedAnswer = format(jsTimeValue);
  // Does not work for complex cases i.e. o'clock/in the morning
  std::u16string currentPart;
  std::string::size_type sz = formattedAnswer.length() - 1;
  int dateTypeCounter = 0;
  for (std::string::size_type i = 0; i < formattedAnswer.size(); ++i) {
    if (isalnum(formattedAnswer[i])) {
      currentPart += formattedAnswer[i];
    } else {
      if (currentPart != u"") {
        splitAnswer.push_back(currentPart);
        part[u"type"] = returnTypeOfDate(uniqueFormat[dateTypeCounter]);
        dateTypeCounter++;
        part[u"value"] = {currentPart.begin(), currentPart.end()};
        formatMap.push_back(part);
        currentPart = u"";
      }
      currentPart += formattedAnswer[i];
      splitAnswer.push_back(currentPart);
      part[u"type"] = u"literal";
      part[u"value"] = {currentPart.begin(), currentPart.end()};
      formatMap.push_back(part);
      currentPart = u"";
    }
    if (i == sz) { // last index
      splitAnswer.push_back(currentPart);
      part[u"type"] = returnTypeOfDate(uniqueFormat[dateTypeCounter]);
      part[u"value"] = {currentPart.begin(), currentPart.end()};
      formatMap.push_back(part);
    }
  }
  return std::vector<std::unordered_map<std::u16string, std::u16string>>{
      formatMap};
}

struct NumberFormat::Impl {
  std::u16string locale;
};

NumberFormat::NumberFormat() : impl_(std::make_unique<Impl>()) {}
NumberFormat::~NumberFormat() {}

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

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
