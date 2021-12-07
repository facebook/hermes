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
    const vm::CallResult<std::vector<std::u16string>> &requestedLocales,
    const std::vector<std::u16string> &availableLocales) {
  // 1. Let result be a new Record.
  LocaleMatch result;
  // 2. For each element locale of requestedLocales, do
  for (const std::u16string &locale : *requestedLocales) {
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
    return runtime->raiseTypeError(vm::TwineChar16("Option for property ") + vm::TwineChar16(property.c_str()) + vm::TwineChar16(" is not a bool"));
  }
  //  8. Return value.
  return llvh::Optional<bool>(value->second.getBool());
}
// https://tc39.es/ecma402/#sec-defaultnumberoption
vm::CallResult<llvh::Optional<uint8_t>> defaultNumberOption(
    const double value,
    const std::uint8_t minimum,
    const std::uint8_t maximum,
    llvh::Optional<uint8_t> fallback,
    vm::Runtime *runtime) {
  //  1. If value is undefined, return fallback.
  if (!value && value != 0) {
    return fallback;
  }
  //  2. Set value to ? ToNumber(value).
  //  3. If value is NaN or less than minimum or greater than maximum, throw a
  //  RangeError exception.
  if (std::isnan(value) || value < minimum || value > maximum) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  //  4. Return floor(value).
  return llvh::Optional<uint8_t>(std::floor(value));
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
  auto value = options.find(property);
  if (value == options.end()) {
    return fallback;
  }
  //  3. Return ? DefaultNumberOption(value, minimum, maximum, fallback).
  auto defaultNumber = defaultNumberOption(
      value->second.getNumber(),
      minimum,
      maximum,
      llvh::Optional<uint8_t>(fallback),
      runtime);
  if (defaultNumber == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError(vm::TwineChar16(property.c_str()) + vm::TwineChar16(" value is out of range"));
  } else {
    return llvh::Optional<uint8_t>(defaultNumber.getValue());
  }
}

// Implementation of
// https://tc39.es/ecma402/#sec-lookupsupportedlocales
std::vector<std::u16string> lookupSupportedLocales(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales) {
  // 1. Let subset be a new empty List.
  std::vector<std::u16string> subset;
  // 2. For each element locale of requestedLocales, do
  for (const std::u16string &locale : requestedLocales) {
    // a. Let noExtensionsLocale be the String value that is locale with any
    // Unicode locale extension sequences removed.
    std::u16string noExtensionsLocale = toNoUnicodeExtensionsLocale(locale);
    // b. Let availableLocale be BestAvailableLocale(availableLocales,
    // noExtensionsLocale).
    llvh::Optional<std::u16string> availableLocale =
        bestAvailableLocale(availableLocales, noExtensionsLocale);
    // c. If availableLocale is not undefined, append locale to the end of
    // subset.
    if (!availableLocale) {
      subset.push_back(locale);
    }
  }

  // 3. Return subset.
  return subset;
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
  struct newRecord {
    std::u16string localeMatcher, ca, nu, hc, hcDefault; // For opt
  };
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
struct ResolveLocale {
  std::u16string key, value, dataLocale, resolveLocale, locale, ca, nu, hc;
};
struct UnicodeExtensionKeys {
  std::u16string CALENDAR = u"calendar";
  std::u16string CALENDAR_CANON = u"ca";

  std::u16string NUMERINGSYSTEM = u"numbers";
  std::u16string NUMERINGSYSTEM_CANON = u"nu";

  std::u16string HOURCYCLE = u"hours";
  std::u16string HOURCYCLE_CANON = u"hc";

  std::u16string COLLATION = u"collation";
  std::u16string COLLATION_CANON = u"co";

  std::u16string COLLATION_NUMERIC = u"colnumeric";
  std::u16string COLLATION_NUMERIC_CANON = u"kn";

  std::u16string COLLATION_CASEFIRST =
      u"colcasefirst"; // TODO:: double check this
  std::u16string COLLATION_CASEFIRST_CANON = u"kf";
};
std::u16string resolveCalendarAlias(const std::u16string &value) {
  // https://github.com/unicode-org/cldr/blob/master/common/bcp47/calendar.xml
  static std::vector<std::u16string> s_calendarAliasMappings = {
      u"gregorian", u"gregory"};
  if (std::find(
          s_calendarAliasMappings.begin(),
          s_calendarAliasMappings.end(),
          value) == s_calendarAliasMappings.end()) {
    return value;
  } else {
    int pos = find(
                  s_calendarAliasMappings.begin(),
                  s_calendarAliasMappings.end(),
                  value) -
        s_calendarAliasMappings.begin();
    return s_calendarAliasMappings.at(pos);
  }
}
std::u16string resolveNumberSystemAlias(const std::u16string &value) {
  // https://github.com/unicode-org/cldr/blob/master/common/bcp47/number.xml
  static std::vector<std::u16string> s_numberAliasMappings = {
      u"traditional", u"traditio"};
  if (std::find(
          s_numberAliasMappings.begin(), s_numberAliasMappings.end(), value) ==
      s_numberAliasMappings.end()) {
    return value;
  } else {
    int pos =
        find(
            s_numberAliasMappings.begin(), s_numberAliasMappings.end(), value) -
        s_numberAliasMappings.begin();
    return s_numberAliasMappings.at(pos);
  }
}
std::u16string resolveCollationAlias(const std::u16string &value) {
  // https://github.com/unicode-org/cldr/blob/release-37/common/bcp47/collation.xml#L12
  static std::vector<std::u16string> s_collationAliasMappings = {
      u"dictionary",
      u"dict",
      u"phonebook",
      u"phonebk",
      u"traditional",
      u"trad",
      u"gb2312han",
      u"gb2312"};
  if (std::find(
          s_collationAliasMappings.begin(),
          s_collationAliasMappings.end(),
          value) == s_collationAliasMappings.end()) {
    return value;
  } else {
    int pos = find(
                  s_collationAliasMappings.begin(),
                  s_collationAliasMappings.end(),
                  value) -
        s_collationAliasMappings.begin();
    return s_collationAliasMappings.at(pos);
  }
}
std::u16string resolveKnownAliases(
    const std::u16string &key,
    const std::u16string &value) {
  if (key == u"ca") {
    return resolveCalendarAlias(value);
  }
  if (key == u"nu") {
    return resolveNumberSystemAlias(value);
  }
  if (key == u"co") {
    return resolveCollationAlias(value);
  }
  if (key == u"kn" && value == u"yes") {
    return std::u16string(u"true");
  }
  if (key == u"kn" || key == u"kf") {
    if (value == u"no") {
      return std::u16string(u"false");
    }
  }
}
bool binarySeachForValidKeyword(
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
  uint8_t pos = iter - keys.begin();
  switch (pos) {
    case 0:
      return binarySeachForValidKeyword(nuVector, value);
    case 1:
      return binarySeachForValidKeyword(coVector, value);
    case 2:
      return binarySeachForValidKeyword(caVector, value);
    default:
      return false;
  }
};
void setUnicodeExtensions(
    const std::u16string &supportedExtendionKey,
    const std::vector<std::u16string> &valueList) {
  std::vector<std::u16string> unicodeExtensionKeywords;
  // TODO: From BCP Parser
  // https://github.com/facebook/hermes/blob/17d7583650e5f4b59645e0607e24c7ccb15404ee/lib/Platform/Intl/java/com/facebook/hermes/intl/LocaleObjectAndroid.java#L247
};
// https://tc39.es/ecma402/#sec-resolvelocale
ResolveLocale resolveLocale(
    const std::vector<std::u16string> &availableLocales,
    const vm::CallResult<std::vector<std::u16string>> &requestedLocales,
    const Options &options,
    const std::vector<std::u16string> &relevantExtensionKeys
    //  In line with Java, haven't included LocaleData
) {
  //  Skip 1/2/3, as we don't need to have independant implementations for best
  //  fit
  auto localeMatchResult = lookupMatcher(requestedLocales, availableLocales);
  //  5. Let result be a new Record.
  ResolveLocale result;
  std::u16string value;
  std::vector<std::u16string> supportedExtensionAdditionKeys;
  //  9. For each element key of relevantExtensionKeys, do
  for (std::u16string key : relevantExtensionKeys) {
    result.value = u"";
    if (localeMatchResult.extension != u"") { // 9.h.
      if (localeMatchResult.extension.find(key)) { // 9.h.i.
        std::u16string requestedValue = localeMatchResult.extension;
        if (requestedValue != u"") {
          value = requestedValue;
        } else {
          value = u"true";
        }
        supportedExtensionAdditionKeys.push_back(key);
      }
    }
    if (options.find(key) != options.end()) { // 9.i.
      auto optionsValue = options.find(key)->second;
      if (optionsValue.isString()) {
        if (optionsValue.getString() == u"") {
          optionsValue = true;
        }
        if (optionsValue.getString() != u"" &&
            optionsValue.getString() != value) {
          supportedExtensionAdditionKeys.erase(
              std::remove(
                  supportedExtensionAdditionKeys.begin(),
                  supportedExtensionAdditionKeys.end(),
                  key),
              supportedExtensionAdditionKeys.end());
          value = optionsValue.getString();
        }
      }
      if (resolveKnownAliases(key, value) != u"") {
        if (!value.empty() && !isValidKeyword(key, value)) {
          result.key = u"";
          continue;
        }
        result.key = value;
      }
      for (std::u16string supportedExtendionKey :
           supportedExtensionAdditionKeys) {
        std::vector<std::u16string> valueList;
        std::u16string keyValue = localeMatchResult.extension;
        keyValue = resolveKnownAliases(localeMatchResult.extension, keyValue);
        if (!keyValue.empty() &&
            !isValidKeyword(supportedExtendionKey, keyValue)) {
          continue;
        }
        valueList.push_back(keyValue);
        // Void function but SHOULD affect localeMatchResult.matchedLocale?
        setUnicodeExtensions(supportedExtendionKey, valueList);
        localeMatchResult.matchedLocale = keyValue; // not right
      }
    }
  }
  result.locale = localeMatchResult.locale;
  return result;
}
// Implementation of
// https://tc39.es/ecma402/#sec-intl.datetimeformat.supportedlocalesof
vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let availableLocales be %DateTimeFormat%.[[AvailableLocales]].
  NSArray<NSString *> *nsAvailableLocales =
      [NSLocale availableLocaleIdentifiers];

  // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  vm::CallResult<std::vector<std::u16string>> requestedLocales =
      getCanonicalLocales(runtime, locales);
  std::vector<std::u16string> availableLocales =
      nsStringArrayToU16StringArray(nsAvailableLocales);

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
  NSDictionary<NSString *, NSString *> *dictListOfTimeZones;
  return dictListOfTimeZones = [NSTimeZone abbreviationDictionary];
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
  Impl::newRecord opt;
  // 4. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  //   "lookup", "best fit" », "best fit").
  auto matcher = getOptionString(
      optionsToUse,
      u"localeMatcher",
      {u"lookup", u"best fit"},
      llvh::Optional<std::u16string>(u"best fit"),
      runtime);
  // 5. Set opt.[[localeMatcher]] to matcher.
  opt.localeMatcher = matcher->getValue();

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
  if (calendar->hasValue()) {
    opt.ca = calendar->getValue();
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
    opt.nu = numberingSystem->getValue();
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
      opt.hc = hourCycle->getValue();
    }
  }

  // 16 - 23
  static const std::vector<std::u16string> relevantExtensionKeys = {
      u"ca", u"nu", u"hc"};
  auto r =
      resolveLocale(locales, requestedLocales, options, relevantExtensionKeys);
  impl_->locale = r.locale; // needs fixing
  // Get default locale as NS for future defaults
  NSLocale *defaultNSLocale = [[NSLocale alloc]
      initWithLocaleIdentifier:u16StringToNSString(impl_->locale)];
  bool useDefaultCalendar;
  auto mResolvedLocaleObjectForResolvedOptions = r.locale;
  Option calendarResolved = r.ca;
  if (calendarResolved.getString() != u"") {
    useDefaultCalendar = false;
    impl_->Calendar = calendarResolved.getString();
  } else {
    useDefaultCalendar = true;
    impl_->Calendar = u"gregorian"; // Placeholder for now
  }
  bool useDefaultNumberSystem;
  Option numberingSystemResolved = r.nu;
  if (numberingSystemResolved.getString() != u"") {
    useDefaultNumberSystem = false;
    impl_->NumberingSystem = numberingSystemResolved.getString();
  } else {
    useDefaultNumberSystem = true;
    impl_->NumberingSystem = u"latn";
  }
  Option hourCycleResolved = r.hc;

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
    opt.hcDefault = hcDefaultNS;
    if (hourCycleResolved.isString()) {
      opt.hc = hcDefaultNS;
    } else {
      opt.hc = hourCycleResolved.getString();
    }
    if (hour12->hasValue()) {
      if (hour12->getValue() == true) { // true
        if (hcDefaultNS == u"h11" || hcDefaultNS == u"h23") {
          opt.hc = u"h11";
        } else {
          opt.hc = u"h12";
        }
      } else {
        if (hcDefaultNS == u"h11" || hcDefaultNS == u"h23") {
          opt.hc = u"h23";
        } else {
          opt.hc = u"h24";
        }
      }
    }
    impl_->HourCycle = opt.hc;
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
  if (![customFormattedDate isEqual:@""]) {
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
  std::u16string arr;
  std::unordered_map<char, char> exists;
  for (const auto &el : s) {
    if (exists.insert({el, 0}).second) {
      arr += el;
    }
  }
  return arr;
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
