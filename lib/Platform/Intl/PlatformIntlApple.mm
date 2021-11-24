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
std::vector<std::u16string> nsStringArrayToU16StringArray(const NSArray<NSString *> *array) {
  auto size = [array count];
  std::vector<std::u16string> result;
  for (size_t i = 0; i < size; i++) {
    result.push_back(nsStringToU16String(array[i]));
  }
  return result;
}
NSArray<NSString *> *u16StringArrayToNSStringArray(const std::vector<std::u16string> &array) {
  auto size = array.size();
  NSMutableArray *result = [NSMutableArray arrayWithCapacity: size];
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
vm::CallResult<Option> getOption(
    Options &options,
    const std::u16string property,
    const std::u16string type,
    const std::vector<std::u16string> values,
    const Options &fallback) {
  // 1. Assert type(options) is object
  // 2. Let value be ? Get(options, property).
  auto value = options.find(property);
  // 3. If value is undefined, return fallback.
  if (value == options.end()) {
    auto fallbackIter = fallback.find(u"fallback");
    // Make sure fallback exists
    if (fallbackIter == options.end()) {
      return vm::ExecutionStatus::EXCEPTION;
    }
    return fallback.find(u"fallback")->second;
  }
  // 4. Assert: type is "boolean" or "string".
  // 5. If type is "boolean", then
  // a. Set value to ! ToBoolean(value).
  // 6. If type is "string", then
  // a. Set value to ? ToString(value).
    if(type != u"string" && type != u"boolean") {
      return vm::ExecutionStatus::EXCEPTION;
    }
  // 7. If values is not undefined and values does not contain an element equal to value, throw a RangeError exception.
  if (value->second.isString()) {
    if (!values.empty() && llvh::find(values, value->second.getString()) == values.end()) {
      return vm::ExecutionStatus::EXCEPTION;
    }
  } else {
    // How can find work with a bool?
//    if (!values.empty() && llvh::find(values, value->second.getBool()) == values.end()) {
//      return vm::ExecutionStatus::EXCEPTION;
//    }
  }
  // 8. Return value.
  return value->second;
}
// Implementation of
// https://402.ecma-international.org/8.0/#sec-getnumberoption
vm::CallResult<Option> getOptionNumber(
    const Options &options,
    const std::u16string property,
    const std::uint8_t minimum,
    const std::uint8_t maximum,
    const Options &fallback) {
//  1. Assert: Type(options) is Object.
//  2. Let value be ? Get(options, property).
  auto value = options.find(property);
  if (value == options.end()) {
    // Return fallback
    auto fallbackIter = fallback.find(u"fallback");
    // Make sure fallback exists
    if (fallbackIter == options.end()) {
      return vm::ExecutionStatus::EXCEPTION;
    }
    return fallback.find(u"fallback")->second;
  }
  // Check if it is in the allowed range
  if (value->second.getNumber() > maximum || value->second.getNumber() < minimum) {
    return vm::ExecutionStatus::EXCEPTION;
  }
//  3. Return ? DefaultNumberOption(value, minimum, maximum, fallback).
  return value->second;
}

// Implementation of
// https://tc39.es/ecma402/#sec-lookupsupportedlocales
std::vector<std::u16string> lookupSupportedLocales(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales) {
  // 1. Let subset be a new empty List.
  std::vector<std::u16string> subset;
  // 2. For each element locale of requestedLocales, do
  for (const std::u16string &locale: requestedLocales) {
    // a. Let noExtensionsLocale be the String value that is locale with any Unicode locale extension sequences removed.
    std::u16string noExtensionsLocale = toNoUnicodeExtensionsLocale(locale);
    // b. Let availableLocale be BestAvailableLocale(availableLocales, noExtensionsLocale).
    llvh::Optional<std::u16string> availableLocale = bestAvailableLocale(availableLocales, noExtensionsLocale);
    // c. If availableLocale is not undefined, append locale to the end of subset.
    if (!availableLocale) { subset.push_back(locale); }
  }

  // 3. Return subset.
  return subset;
}

// Implementation of
// https://tc39.es/ecma402/#sec-supportedlocales
vm::CallResult<std::vector<std::u16string>> supportedLocales(
    const std::vector<std::u16string> availableLocales,
    const std::vector<std::u16string> requestedLocales,
    Options &options) {
  // 1. Set options to ? CoerceOptionsToObject(options).
  // 2. Let matcher be ? GetOption(options, "localeMatcher", "string", « "lookup", "best fit" », "best fit").
  Options matcherFallback;
  std::u16string bestFit = u"best fit"; // If not declared, it will not work
  matcherFallback.emplace(std::u16string(u"fallback"), Option(bestFit));
  std::vector<std::u16string> vectorForMatcher = {u"lookup", u"best fit"};
  vm::CallResult<std::u16string> matcher = getOption(options, u"localeMatcher", u"string", vectorForMatcher, matcherFallback)->getString();
  if (LLVM_UNLIKELY(matcher == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  std::vector<std::u16string> supportedLocales;
  // Skip 3/4, as don't need to have independant implementations for best fit
    // a. Let supportedLocales be LookupSupportedLocales(availableLocales, requestedLocales).
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
    std::u16string localeMatcher, ca, nu, hc, hcDefault;// For opt
  };
// Table 7 https://tc39.es/ecma402/#table-datetimeformat-resolvedoptions-properties
  // For DateTimeFormat::impl_ in initialize
  std::u16string mFormatMatcher, mHourCycle, mWeekday, mEra, mYear, mMonth, mDay, mDayPeriod, mHour, mMinute, mSecond, mTimeZone, mTimeZoneName, mDateStyle, mTimeStyle, mCalendar, locale;
  uint8_t mFractionalSecondDigits;
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

  std::u16string COLLATION_CASEFIRST = u"colcasefirst"; // TODO:: double check this
  std::u16string COLLATION_CASEFIRST_CANON = u"kf";
};
std::u16string resolveCalendarAlias(
  std::u16string value) {
  // https://github.com/unicode-org/cldr/blob/master/common/bcp47/calendar.xml
  static std::vector<std::u16string> s_calendarAliasMappings = {u"gregorian", u"gregory"};
  if (std::find(s_calendarAliasMappings.begin(), s_calendarAliasMappings.end(), value) == s_calendarAliasMappings.end()) {
    // Not found in vector, return value
    return value;
  } else {
    // Found, so return it from vector?
    int pos = find(s_calendarAliasMappings.begin(), s_calendarAliasMappings.end(), value) - s_calendarAliasMappings.begin();;// Should still return value?
    return s_calendarAliasMappings.at(pos);
  }
}
std::u16string resolveNumberSystemAlias(
  std::u16string value) {
  // https://github.com/unicode-org/cldr/blob/master/common/bcp47/number.xml
  static std::vector<std::u16string> s_numberAliasMappings = {u"traditional", u"traditio"};
  if (std::find(s_numberAliasMappings.begin(), s_numberAliasMappings.end(), value) == s_numberAliasMappings.end()) {
    // Not found in vector, return value
    return value;
  } else {
    // Found, so return it from vector?
    int pos = find(s_numberAliasMappings.begin(), s_numberAliasMappings.end(), value) - s_numberAliasMappings.begin();
    return s_numberAliasMappings.at(pos);// Should still return value?
  }
}
std::u16string resolveCollationAlias(
  std::u16string value) {
  // https://github.com/unicode-org/cldr/blob/release-37/common/bcp47/collation.xml#L12
  // Should be a better way to do this?
  std::map<std::u16string, std::u16string> s_collationAliasMappings;
  s_collationAliasMappings.insert({u"dictionary", u"dict"});
  s_collationAliasMappings.insert({u"phonebook", u"phonebk"});
  s_collationAliasMappings.insert({u"traditional", u"trad"});
  s_collationAliasMappings.insert({u"gb2312han", u"gb2312"});
  if (s_collationAliasMappings.count(value) < 0) {
    // Not found in vector, return value
    return value;
  } else {
    // Found, so return it from vector?
    return s_collationAliasMappings.at(value);// Should still return value?
  }
}
std::u16string resolveKnownAliases(
  std::u16string key,
  std::u16string value) {
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
    std::u16string t = u"true";
    return t;
  }
  if (key == u"kn" || key == u"kf") {
    if (value == u"no") {
    std::u16string f = u"false";
    return f;
  }
  }
}

bool isValidKeyword(
  std::u16string key,
  std::u16string value){
  std::map<std::string, std::vector<std::string>> s_validKeywords;

  // Ref:: https://tc39.es/ecma402/#table-numbering-system-digits
  // It is a subset of
  // https://github.com/unicode-org/cldr/blob/master/common/bcp47/number.xml
  s_validKeywords.insert({"nu", {"adlm",
    "ahom",
    "arab",
    "arabext",
    "bali",
    "beng",
    "bhks",
    "brah",
    "cakm",
    "cham",
    "deva",
    "diak",
    "fullwide",
    "gong",
    "gonm",
    "gujr",
    "guru",
    "hanidec",
    "hmng",
    "hmnp",
    "java",
    "kali",
    "khmr",
    "knda",
    "lana",
    "lanatham",
    "laoo",
    "latn",
    "lepc",
    "limb",
    "mathbold",
    "mathdbl",
    "mathmono",
    "mathsanb",
    "mathsans",
    "mlym",
    "modi",
    "mong",
    "mroo",
    "mtei",
    "mymr",
    "mymrshan",
    "mymrtlng",
    "newa",
    "nkoo",
    "olck",
    "orya",
    "osma",
    "rohg",
    "saur",
    "segment",
    "shrd",
    "sind",
    "sinh",
    "sora",
    "sund",
    "takr",
    "talu",
    "tamldec",
    "telu",
    "thai",
    "tibt",
    "tirh",
    "vaii",
    "wara",
    "wcho"}
  });
  // Ref:: https://github.com/unicode-org/cldr/blob/release-37/common/bcp47/collation.xml
  // -- Minus "standard" & "search" which are not allowed as per spec:
  // https://tc39.es/ecma402/#sec-intl-collator-internal-slots
  s_validKeywords.insert({"co", {"big5han",
    "compat",
    "dict",
    "direct",
    "ducet",
    "emoji",
    "eor",
    "gb2312",
    "phonebk",
    "phonetic",
    "pinyin",
    "reformed",
    "searchjl",
    "stroke",
    "trad",
    "unihan",
    "zhuyin"}
  });
  s_validKeywords.insert({"ca", {
    "buddhist",
    "chinese",
    "coptic",
    "dangi",
    "ethioaa",
    "ethiopic",
    "gregory",
    "hebrew",
    "indian",
    "islamic",
    "islamic-umalqura",
    "islamic-tbla",
    "islamic-civil",
    "islamic-rgsa",
    "iso8601",
    "japanese",
    "persian",
    "roc"}
  });
//  Find not working with string/vector combo?
//  if (map::find(s_validKeywords.begin(), s_validKeywords.end(), key) != s_validKeywords.end()) {
//    // Not sure, return Arrays.asList(s_validKeywords.get(key)).contains(value); in Java
//    return false;
//  }
  return true;
};
void setUnicodeExtensions(
    std::u16string supportedExtendionKey,
    std::vector<std::u16string> valueList){
  std::vector<std::u16string> unicodeExtensionKeywords;
  // TODO: From BCP Parser
  //https://github.com/facebook/hermes/blob/17d7583650e5f4b59645e0607e24c7ccb15404ee/lib/Platform/Intl/java/com/facebook/hermes/intl/LocaleObjectAndroid.java#L247
};
// https://tc39.es/ecma402/#sec-resolvelocale
ResolveLocale resolveLocale(
    const std::vector<std::u16string> &availableLocales,
    const vm::CallResult<std::vector<std::u16string>> &requestedLocales,
    const Options &options,
    const std::vector<std::u16string> &relevantExtensionKeys
//  In line with Java, haven't included LocaleData
    ){
//  Skip 1/2/3, as we don't need to have independant implementations for best fit
    auto localeMatchResult = lookupMatcher(requestedLocales, availableLocales);
//  5. Let result be a new Record.
    ResolveLocale result;
    std::u16string value;
    std::vector<std::u16string> supportedExtensionAdditionKeys;
//  9. For each element key of relevantExtensionKeys, do
  for (std::u16string key : relevantExtensionKeys) {
    result.value = u"";
    if (localeMatchResult.extension != u"" ) { // 9.h.
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
        if (optionsValue.getString() != u"" && optionsValue.getString() != value) {
          supportedExtensionAdditionKeys.erase(std::remove
          (supportedExtensionAdditionKeys.begin(), supportedExtensionAdditionKeys.end(), key), supportedExtensionAdditionKeys.end());
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
      for (std::u16string supportedExtendionKey : supportedExtensionAdditionKeys) {
        std::vector<std::u16string> valueList;
        std::u16string keyValue = localeMatchResult.extension;
        keyValue = resolveKnownAliases(localeMatchResult.extension, keyValue);
        if (!keyValue.empty() && !isValidKeyword(supportedExtendionKey, keyValue)) {
          continue;
        }
        valueList.push_back(keyValue);
        // Void function but SHOULD affect localeMatchResult.matchedLocale?
        setUnicodeExtensions(supportedExtendionKey, valueList);
        localeMatchResult.matchedLocale = keyValue;// not right
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
  NSArray<NSString *> *nsAvailableLocales = [NSLocale availableLocaleIdentifiers];
  
  // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  vm::CallResult<std::vector<std::u16string>> requestedLocales = getCanonicalLocales(runtime, locales);
  std::vector<std::u16string> availableLocales = nsStringArrayToU16StringArray(nsAvailableLocales);
  
  // 3. Return ? (availableLocales, requestedLocales, options).
  // Get a non-const copy of options
  Options copyOptions = options;
  return supportedLocales(availableLocales, requestedLocales.getValue(), copyOptions);
}

// Implementation of 
// https://tc39.es/ecma402/#sec-todatetimeoptions
vm::CallResult<Options> toDateTimeOptions(Options &options, std::u16string required, std::u16string defaults) {
  // 1. If options is undefined, let options be null; otherwise let options be ? ToObject(options).
  // 2. Let options be OrdinaryObjectCreate(options).
  // 3. Let needDefaults be true.
  bool needDefaults = true;
  // 4. If required is "date" or "any", then
  if (required == u"date" || required == u"any") {
    // a. For each property name prop of « "weekday", "year", "month", "day" », do
    for (std::u16string prop : {u"weekday", u"year", u"month", u"day"}) {
      // i. Let value be ? Get(options, prop).
        if (options.find(prop) != options.end()) {
        // ii. If value is not undefined, let needDefaults be false.
        needDefaults = false;
      }
    }
  }
  // 5. If required is "time" or "any", then
  if (required == u"time" || required == u"any") {
      // a. For each property name prop of « "dayPeriod", "hour", "minute", "second", "fractionalSecondDigits" », do
    for (std::u16string prop : {u"hour", u"minute", u"second"}) {
      // i. Let value be ? Get(options, prop).
      if (options.find(prop) != options.end()) {
        // ii. If value is not undefined, let needDefaults be false.
        needDefaults = false;
      }
    }
  }
  // 6. Let dateStyle be ? Get(options, "dateStyle").
  auto dateStyle = options.find(u"dateStyle");
  // 7. Let timeStyle be ? Get(options, "timeStyle").
  auto timeStyle = options.find(u"timeStyle");
  // 8. If dateStyle is not undefined or timeStyle is not undefined, let needDefaults be false.
  if (dateStyle != options.end() || timeStyle != options.end()) {
    needDefaults = false;
  }
  // 9. If required is "date" and timeStyle is not undefined, then
  if (required == u"date" && timeStyle != options.end()) {
    // a. Throw a TypeError exception.
    return vm::ExecutionStatus::EXCEPTION;
  }
  // 10. If required is "time" and dateStyle is not undefined, then
  if (required == u"time" && dateStyle != options.end()) {
    // a. Throw a TypeError exception.
    return vm::ExecutionStatus::EXCEPTION;
  }
  std::u16string numeric = u"numeric";
  // 11. If needDefaults is true and defaults is either "date" or "all", then
  if (needDefaults && (defaults == u"date" || defaults == u"all")) {
    // a. For each property name prop of « "year", "month", "day" », do
    for (std::u16string prop : {u"year", u"month", u"day"}) {
      // i. Perform ? CreateDataPropertyOrThrow(options, prop, "numeric").
      options.emplace(std::u16string(prop), Option(numeric));
    }
  }
  // 12. If needDefaults is true and defaults is either "time" or "all", then
  if (needDefaults && (defaults == u"time" || defaults == u"all")) {
    // a. For each property name prop of « "hour", "minute", "second" », do
    for (std::u16string prop : {u"hour", u"minute", u"second"}) {
      // i. Perform ? CreateDataPropertyOrThrow(options, prop, "numeric").
      options.emplace(std::u16string(prop), Option(numeric));
    }
  }
  // 13. return options
  return options;
}
// https://tc39.es/ecma402/#sec-case-sensitivity-and-case-mapping
std::u16string normalizeTimeZoneName(
    std::u16string timeZoneName) {
    std::u16string normalized;
    std::uint8_t offset = 'a' - 'A';
    for (std::uint8_t idx = 0; idx < timeZoneName.length(); idx++) {
      unsigned char c = timeZoneName[idx];
      if (c >= 'a' && c <= 'z') {
        normalized.push_back((char) c - offset);
      }
      else {
        normalized.push_back(c);
      }
      }
    return normalized;
  }
bool isTimeZoneValid(
  std::u16string timeZoneValue) {
  NSDictionary<NSString *,NSString *> *dictListOfTimeZones = [NSTimeZone abbreviationDictionary];
  NSArray *NSListOfTimeZones = [dictListOfTimeZones allKeys];
  std::vector<std::u16string> vectorListOfTimeZones = nsStringArrayToU16StringArray(NSListOfTimeZones);
  for (std::u16string validTimeZone : vectorListOfTimeZones) {
    if (validTimeZone == timeZoneValue) {
      return true;
    }
  }
  return false;
}
//https://github.com/facebook/hermes/blob/33cf15ab318694423926c3762e001465b399ef38/lib/Platform/Intl/java/com/facebook/hermes/intl/PlatformDateTimeFormatterAndroid.java#L121
std::u16string getDefaultHourCycle(std::u16string locale) {
  NSString *l = u16StringToNSString(locale);
  NSLocale *strToLocale = [[NSLocale alloc] initWithLocaleIdentifier:l];
  NSString *formatStringForHours = [NSDateFormatter dateFormatFromTemplate:@"j" options:0 locale:strToLocale];
  std::u16string dateFormatPattern = nsStringToU16String(formatStringForHours);
  if (dateFormatPattern.find(u"h") < dateFormatPattern.length()) {
    return u"h12";
  }
  else if (dateFormatPattern.find(u"K") < dateFormatPattern.length()) {
    return u"h11";
  }
  else if (dateFormatPattern.find(u"H") < dateFormatPattern.length()) {
    return u"h23";
  }
  else {
    return u"h24";
  }
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
  // Create a copy of the unordered map &options
  Options copyOptions = options;
  auto optionsTo = toDateTimeOptions(copyOptions, u"any", u"date");
      if (optionsTo.getStatus() == vm::ExecutionStatus::EXCEPTION){
        return runtime->raiseTypeError("timeStyle or dateStyle is not defined");
      }
  auto optionsToUse = optionsTo.getValue();
  // 3. Let opt be a new Record.
  Impl::newRecord opt;
  // 4. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  //   "lookup", "best fit" », "best fit").
  std::vector<std::u16string> values = {u"lookup", u"best fit"};
  Options matcherFallback;
  std::u16string bestFit = u"best fit"; // If not declared, it will not work
  matcherFallback.emplace(std::u16string(u"fallback"), Option(bestFit));
  auto matcher = getOption(optionsToUse, u"localeMatcher", u"string", values, matcherFallback);
  // 5. Set opt.[[localeMatcher]] to matcher.
  opt.localeMatcher = matcher->getString();

  // 6. Let calendar be ? GetOption(options, "calendar", "string",
  //    undefined, undefined).
  std::vector<std::u16string> emptyVector;
  Options undefinedFallback;
  std::u16string undefined; // If this is truly undefined it will error throw?
  undefinedFallback.emplace(std::u16string(u"fallback"), Option(undefined));
  auto calendar = getOption(optionsToUse, u"calendar", u"string", emptyVector, undefinedFallback);
  // 7. If calendar is not undefined, then
  //    a. If calendar does not match the Unicode Locale Identifier type
  //       nonterminal, throw a RangeError exception.
  // 8. Set opt.[[ca]] to calendar.
  if (calendar->getString() != u"") {
      if (calendar->isString()) {
      opt.ca = calendar->getString();
  }
  else {
    return runtime->raiseRangeError("Incorrect calendar information provided");
  }
  }
  // 9. Let numberingSystem be ? GetOption(options, "numberingSystem",
  //    "string", undefined, undefined).
  auto numberingSystem = getOption(optionsToUse, u"numberingSystem", u"string", emptyVector, undefinedFallback);
  // 10. If numberingSystem is not undefined, then
  //     a. If numberingSystem does not match the Unicode Locale Identifier
  //        type nonterminal, throw a RangeError exception.
  // 11. Set opt.[[nu]] to numberingSystem.
  if (numberingSystem->isString()) {
      opt.nu = numberingSystem->getString();
  }
  else {
    return runtime->raiseRangeError("Incorrect numberingSystem information provided");
  }
        
  // 12. Let hour12 be ? GetOption(options, "hour12", "boolean",
  //     undefined, undefined).
  auto hour12 = getOption(optionsToUse, u"hour12", u"boolean", emptyVector, undefinedFallback);
  // 13. Let hourCycle be ? GetOption(options, "hourCycle", "string", «
  //     "h11", "h12", "h23", "h24" », undefined).
  std::vector<std::u16string> hourCycleVector = {u"h11", u"h12", u"h23", u"h24"};
  auto hourCycle = getOption(optionsToUse, u"hourCycle", u"string", hourCycleVector, undefinedFallback);
  // 14. If hour12 is not undefined, then
  //     a. Let hourCycle be null.
      if (hour12->isBool()) {
      hourCycle->getString() = u"null";
  }
  // 15. Set opt.[[hc]] to hourCycle.
  opt.hc = hourCycle->getString();
  
  // 16 - 23
  const std::vector<std::u16string> relevantExtensionKeys = {u"ca", u"nu", u"hc"};
  auto r = resolveLocale(locales, requestedLocales, options, relevantExtensionKeys);
  impl_->locale = r.locale; //needs fixing
  bool useDefaultCalendar;
  auto mResolvedLocaleObjectForResolvedOptions = r.locale;
  Option calendarResolved = r.ca;
  if (calendarResolved.isString()) {
        useDefaultCalendar = false;
        impl_->mCalendar = calendarResolved.getString();
  } else {
        useDefaultCalendar = true;
        // Get default calendar name i.e. gregorian
//        NSString *nstrCalendar = (NSString*)[NSCalendar id];
//        std::u16string defaultCalendar = nsStringToU16String(nstrCalendar);
    impl_->mCalendar = u"gregorian";// Placeholder for now, needs fixing
  }
  bool useDefaultNumberSystem;
  std::u16string mNumberingSystem;
  Option numberingSystemResolved = r.nu;
  if (numberingSystemResolved.isString()) {
        useDefaultNumberSystem = false;
        mNumberingSystem = numberingSystemResolved.getString();
      } else {
        useDefaultNumberSystem = true;
        mNumberingSystem = u"latn";
      }
  Option hourCycleResolved = r.hc;
  
// 24-27
//  24. Let timeZone be ? Get(options, "timeZone").
  std::u16string timeZone;
  std::u16string timeZoneValue;
//  Check find first to avoid error
//  25. If timeZone is undefined, then
  auto timeZoneIter = options.find(u"timeZone");
  if (timeZoneIter == options.end()) {
//  a. Let timeZone be DefaultTimeZone(), i.e. CST.
    NSTimeZone* defaultTimeZone = [NSTimeZone defaultTimeZone];
    NSString* nstrTime = [defaultTimeZone localizedName:NSTimeZoneNameStyleShortStandard
                                                  locale:[NSLocale currentLocale]];
    std::u16string defaultTime = nsStringToU16String(nstrTime);
    timeZoneValue = defaultTime;
  }
//  26. Else,
  else {
//  a. Let timeZone be ? ToString(timeZone).
    timeZoneValue = normalizeTimeZoneName(timeZoneIter->second.getString());
    bool validateTimeZone = isTimeZoneValid(timeZoneValue);
//  b. If the result of IsValidTimeZoneName(timeZone) is false, then
    if (!validateTimeZone) {
//  i. Throw a RangeError exception.
      return runtime->raiseRangeError("Incorrect timeZone information provided");
    }
  }
  impl_->mTimeZone = timeZoneValue;
  // 28 - 34
  const std::vector<std::u16string> matcherVector = {u"basic", u"best fit"};
  const std::vector<std::u16string> sizeVector = {u"long", u"short", u"narrow"};
  const std::vector<std::u16string> numberVector = {u"numeric", u"2-digit"};
  const std::vector<std::u16string> monthVector = {u"numeric", u"2-digit", u"long", u"short", u"narrow"};
  const std::vector<std::u16string> timeZoneNameVector = {u"long", u"short", u"shortOffset", u"longOffset", u"shortGeneric", u"longGeneric"};
  const std::vector<std::u16string> styleVector = {u"full", u"long", u"medium", u"short"};
  
  // 29b
  auto fractionalSecondDigitsOption = getOptionNumber(optionsToUse, u"fractionalSecondDigits", 1, 3, undefinedFallback);
  if (fractionalSecondDigitsOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid fractionalSecondDigits");
  }
  if (fractionalSecondDigitsOption->isNumber()) {
      impl_->mFractionalSecondDigits = fractionalSecondDigitsOption->getNumber();
  }
      
  auto formatMatcherOption = getOption(optionsToUse, u"formatMatcher", u"string", matcherVector, matcherFallback);
  if (formatMatcherOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid formatMatcher");
  }
  impl_->mFormatMatcher = formatMatcherOption->getString();
      
  auto weekdayOption = getOption(optionsToUse, u"weekday", u"string", sizeVector, undefinedFallback);
  if (weekdayOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid weekday");
  }
  impl_->mWeekday = weekdayOption->getString();
  
  auto eraOption = getOption(optionsToUse, u"era", u"string", sizeVector, undefinedFallback);
  if (eraOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid era");
  }
  impl_->mEra = eraOption->getString();
      
  auto yearOption = getOption(optionsToUse, u"year", u"string", numberVector, undefinedFallback);
  if (yearOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid year");
  }
  impl_->mYear = yearOption->getString();
  
  auto monthOption = getOption(optionsToUse, u"month", u"string", monthVector, undefinedFallback);
  if (monthOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid month");
  }
  impl_->mMonth = monthOption->getString();
      
  auto dayOption = getOption(optionsToUse, u"day", u"string", numberVector, undefinedFallback);
  if (dayOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid day");
  }
  impl_->mDay = dayOption->getString();
      
  auto hourOption = getOption(optionsToUse, u"hour", u"string", numberVector, undefinedFallback);
  if (hourOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid hour");
  }
  impl_->mHour = hourOption->getString();
      
  auto minuteOption = getOption(optionsToUse, u"minute", u"string", numberVector, undefinedFallback);
  if (minuteOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid minute");
  }
  impl_->mMinute = minuteOption->getString();
      
  auto secondOption = getOption(optionsToUse, u"second", u"string", numberVector, undefinedFallback);
  if (secondOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid second");
  }
  impl_->mSecond = secondOption->getString();
      
  auto timeZoneNameOption = getOption(optionsToUse, u"timeZoneName", u"string", timeZoneNameVector, undefinedFallback);
  if (timeZoneNameOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid timeZoneName");
  }
  impl_->mTimeZoneName = timeZoneNameOption->getString();
      
  auto dateStyleOption = getOption(optionsToUse, u"dateStyle", u"string", styleVector, undefinedFallback);
  if (dateStyleOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid dateStyle");
  }
  impl_->mDateStyle = dateStyleOption->getString();
      
  auto timeStyleOption = getOption(optionsToUse, u"timeStyle", u"string", styleVector, undefinedFallback);
  if (timeStyleOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid timeStyle");
  }
  impl_->mTimeStyle = timeStyleOption->getString();
      
  auto dayPeriodOption = getOption(optionsToUse, u"dayPeriod", u"string", sizeVector, undefinedFallback);
  if (dayPeriodOption.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return runtime->raiseRangeError("Invalid dayPeriod");
  }
  impl_->mDayPeriod = dayPeriodOption->getString();

  // 36
  if (impl_->mHour == u"") {
    impl_->mHourCycle = u"";
  } else {
    std::u16string hcDefaultNS = getDefaultHourCycle(impl_->locale);
    opt.hcDefault = hcDefaultNS;
    if (hourCycleResolved.isString()) {
      opt.hc = hcDefaultNS;
    } else {
      opt.hc = hourCycleResolved.getString();
    }
    if (hour12->isBool()) {
      if (hour12->getBool()) {// true
      if (hcDefaultNS == u"h11" || hcDefaultNS == u"h23") {
        opt.hc = u"h11";
      }
      else {
        opt.hc = u"h12";
      }
      }
    else {
      if (hcDefaultNS == u"h11" || hcDefaultNS == u"h23") {
        opt.hc = u"h23";
      }
      else {
        opt.hc = u"h24";
      }
    }
    }
    impl_->mHourCycle = opt.hc;
  }
  return vm::ExecutionStatus::RETURNED;
}
// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sec-intl.datetimeformat.prototype.resolvedoptions
Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  
  options.emplace(u"numberingSystem", Option(impl_->mFormatMatcher));
  options.emplace(u"calendar", Option(impl_->mCalendar));
  options.emplace(u"timeZone", Option(impl_->mTimeZone));
  
  if (impl_->mHourCycle != u"") {
    options.emplace(u"hourCycle", Option(impl_->mHourCycle));
    if (impl_->mHourCycle == u"h11" || impl_->mHourCycle == u"h12" ) {
      options.emplace(u"hour12", true);
    }
    else {
      options.emplace(u"hour12", false);
    }
  }
  if (impl_->mWeekday != u"") {
    options.emplace(u"weekday", Option(impl_->mWeekday));
  }
  if (impl_->mEra != u"") {
    options.emplace(u"era", Option(impl_->mEra));
  }
  if (impl_->mYear != u"") {
    options.emplace(u"year", Option(impl_->mYear));
  }
  if (impl_->mMonth != u"") {
    options.emplace(u"month", Option(impl_->mMonth));
  }
  if (impl_->mDay != u"") {
    options.emplace(u"day", Option(impl_->mDay));
  }
  if (impl_->mHour != u"") {
    options.emplace(u"hour", Option(impl_->mHour));
  }
  if (impl_->mMinute != u"") {
    options.emplace(u"minute", Option(impl_->mMinute));
  }
  if (impl_->mSecond != u"") {
    options.emplace(u"second", Option(impl_->mSecond));
  }
  if (impl_->mTimeZoneName != u"") {
    options.emplace(u"timeZoneName", Option(impl_->mTimeZoneName));
  }
  if (impl_->mDateStyle != u"") {
    options.emplace(u"dateStyle", Option(impl_->mDateStyle));
  }
  if (impl_->mTimeStyle != u"") {
    options.emplace(u"timeStyle", Option(impl_->mTimeStyle));
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
  eLongGeneric
};
enum_string formatDate (std::u16string const& inString) {
  if (inString == u"long") return eLong;
  if (inString == u"short") return eShort;
  if (inString == u"narrow") return eNarrow;
  if (inString == u"medium") return eMedium;
  if (inString == u"full") return eFull;
  if (inString == u"basic") return eBasic;
  if (inString == u"best fit") return eBestFit;
  if (inString == u"numeric") return eNumeric;
  if (inString == u"2-digit") return eTwoDigit;
  if (inString == u"shortOffset") return eShortOffset;
  if (inString == u"longOffset") return eLongOffset;
  if (inString == u"shortGeneric") return eShortGeneric;
  if (inString == u"longGeneric") return eLongGeneric;
  // Should something else just be returned by default to silence warning?
};
std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  // Last 3 digits of jsTimeValue need to be removed for this to work?
  auto timeInSeconds = jsTimeValue / 1000;
  NSDate *date = [NSDate dateWithTimeIntervalSince1970:timeInSeconds];
  NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];

  if (impl_->mTimeStyle != u"") {
    std::u16string timeStyle = impl_->mTimeStyle;
    switch(formatDate(timeStyle)) {
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
  if (impl_->mDateStyle != u"") {
    std::u16string dateStyle = impl_->mDateStyle;
    switch(formatDate(dateStyle)) {
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
  if (impl_->mTimeZone != u"") {
    dateFormatter.timeZone = [[NSTimeZone alloc] initWithName:u16StringToNSString(impl_->mTimeZone)];
  }
  if (impl_->locale != u"") {
    dateFormatter.locale = [[NSLocale alloc] initWithLocaleIdentifier:u16StringToNSString(impl_->locale)];
  }
  if (impl_->mCalendar != u"") {
    dateFormatter.calendar = [[NSCalendar alloc] initWithCalendarIdentifier:u16StringToNSString(impl_->mCalendar)];
  }
  
  // The following options cannot be used in conjunction with timeStyle or dateStyle
  // TODO: Where is best to error throw if the wrong options combination have been used?
  // Form a custom format string
  // It will be reordered according to locale later
  NSMutableString *customFormattedDate = [[NSMutableString alloc] init];
  if (impl_->mTimeZoneName != u"") {
    switch(formatDate(impl_->mTimeZoneName)) {
      case eShort: {
        [customFormattedDate appendString:@"z"];
        break; }
      case eLong: {
        [customFormattedDate appendString:@"zzzz"];
        break; }
      case eShortOffset: {
        [customFormattedDate appendString:@"O"];
        break; }
      case eLongOffset: {
        [customFormattedDate appendString:@"OOOO"];
        break; }
      case eShortGeneric: {
        [customFormattedDate appendString:@"v"];
        break; }
      case eLongGeneric: {
        [customFormattedDate appendString:@"vvvv"];
        break; }
      default: {
        [customFormattedDate appendString:@"z"];
      }
    }
  }
  if (impl_->mEra != u"") {
    switch(formatDate(impl_->mEra)) {
      case eNarrow: {
        [customFormattedDate appendString:@"GGGGG"];
        break; }
      case eShort: {
        [customFormattedDate appendString:@"G"];
        break; }
      case eLong: {
        [customFormattedDate appendString:@"GGGG"];
        break; }
      default: {
        [customFormattedDate appendString:@"G"];
      }
    }
  }
  if (impl_->mYear != u"") {
    switch(formatDate(impl_->mYear)) {
      case eNumeric: {
        [customFormattedDate appendString:@"yyyy"];
        break; }
      case eTwoDigit: {
        [customFormattedDate appendString:@"yy"];
        break; }
      default: {
        [customFormattedDate appendString:@"yyyy"];
      }
    }
  }
  if (impl_->mMonth != u"") {
    switch(formatDate(impl_->mMonth)) {
      case eNarrow: {
        [customFormattedDate appendString:@"MMMMM"];
        break; }
      case eNumeric: {
        [customFormattedDate appendString:@"M"];
        break; }
      case eTwoDigit: {
        [customFormattedDate appendString:@"MM"];
        break; }
      case eShort: {
        [customFormattedDate appendString:@"MMM"];
        break; }
      case eLong: {
        [customFormattedDate appendString:@"MMMM"];
        break; }
      default: {
        [customFormattedDate appendString:@"MMM"];
      }
    }
  }
  if (impl_->mWeekday != u"") {
    switch(formatDate(impl_->mWeekday)) {
      case eNarrow: {
        [customFormattedDate appendString:@"EEEEE"];
        break; }
      case eShort: {
        [customFormattedDate appendString:@"E"];
        break; }
      case eLong: {
        [customFormattedDate appendString:@"EEEE"];
        break; }
      default: {
        [customFormattedDate appendString:@"E"];
      }
    }
  }
  if (impl_->mDay != u"") {
    switch(formatDate(impl_->mDay)) {
      case eNumeric: {
        [customFormattedDate appendString:@"d"];
        break; }
      case eTwoDigit: {
        [customFormattedDate appendString:@"dd"];
        break; }
      default: {
        [customFormattedDate appendString:@"dd"];
      }
    }
  }
  // Hour/minute/second options temperamental, may stay one type, depending on locale and other custom options selected
  if (impl_->mHour != u"") {
  // Ignore the hour12 bool in the impl_ struct
  // a = AM/PM for 12 hr clocks, automatically added depending on locale
  // AM/PM not multilingual, de-DE should be "03 Uhr" not "3 AM"
  // K = h11 = 0-11
  // h = h12 = 1-12
  // H = h23 = 0-23
  // k = h24 = 1-24
    if (impl_->mHourCycle == u"h12") {
    switch(formatDate(impl_->mHour)) {
      case eNumeric: {
        [customFormattedDate appendString:@"h"];
        break; }
      case eTwoDigit: {
        [customFormattedDate appendString:@"hh"];
        break; }
      default: {
        [customFormattedDate appendString:@"hh"];
      }
    }
    }
    else if (impl_->mHourCycle == u"h24") {
      switch(formatDate(impl_->mHour)) {
        case eNumeric: {
          [customFormattedDate appendString:@"k"];
          break; }
        case eTwoDigit: {
          [customFormattedDate appendString:@"kk"];
          break; }
        default: {
          [customFormattedDate appendString:@"kk"];
        }
      }
      }
    else if (impl_->mHourCycle == u"h11") {
      switch(formatDate(impl_->mHour)) {
        case eNumeric: {
          [customFormattedDate appendString:@"K"];
          break; }
        case eTwoDigit: {
          [customFormattedDate appendString:@"KK"];
          break; }
        default: {
          [customFormattedDate appendString:@"KK"];
        }
      }
      }
    else { // h23
      switch(formatDate(impl_->mHour)) {
        case eNumeric: {
          [customFormattedDate appendString:@"H"];
          break; }
        case eTwoDigit: {
          [customFormattedDate appendString:@"HH"];
          break; }
        default: {
          [customFormattedDate appendString:@"HH"];
        }
      }
    }
}
  if (impl_->mMinute != u"") {
    switch(formatDate(impl_->mMinute)) {
      case eNumeric: {
        [customFormattedDate appendString:@"m"];
        break; }
      case eTwoDigit: {
        [customFormattedDate appendString:@"mm"];
        break; }
      default: {
        [customFormattedDate appendString:@"m"];
      }
    }
  }
  if (impl_->mSecond != u"") {
    switch(formatDate(impl_->mSecond)) {
      case eNumeric: {
        [customFormattedDate appendString:@"s"];
        break; }
      case eTwoDigit: {
        [customFormattedDate appendString:@"ss"];
        break; }
      default: {
        [customFormattedDate appendString:@"s"];
      }
    }
  }
  // How should this be checked?
  uint8_t fractionalSecondsRange = impl_->mFractionalSecondDigits;
  if (fractionalSecondsRange == 1 ||
      fractionalSecondsRange == 2 ||
      fractionalSecondsRange == 3 ) {
    // This currently outputs to 3 digits only with the date?
    switch(fractionalSecondsRange) {
      case 1: {
        [customFormattedDate appendString:@".S"];
      }
      case 2: {
        [customFormattedDate appendString:@".SS"];
      }
      case 3: {
        [customFormattedDate appendString:@".SSS"];
      }
    }
  }
  // Not supported - dayPeriod (at night/in the morning)
  // Set the custom date from all the concatonated NSStrings (locale will automatically seperate the order)
  // Only set a template format if it isn't empty
  if (![customFormattedDate  isEqual: @""]) {
    [dateFormatter setLocalizedDateFormatFromTemplate:customFormattedDate];
  }
  
  // If no options have been set, date will be empty, so set a format for default
  if ([[dateFormatter stringFromDate:date]  isEqual: @""]) {
    dateFormatter.dateStyle = NSDateFormatterShortStyle;
  }
  return nsStringToU16String([dateFormatter stringFromDate:date]);
}

// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sec-formatdatetimetoparts
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
