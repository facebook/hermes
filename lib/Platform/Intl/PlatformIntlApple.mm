/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/BCP47Parser.h"
#include "hermes/Platform/Intl/PlatformIntl.h"

#import <Foundation/Foundation.h>
#include <unordered_set>

static_assert(__has_feature(objc_arc), "arc must be enabled");

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
const std::vector<std::u16string> &getAvailableLocales() {
  static const std::vector<std::u16string> *availableLocales = [] {
    NSArray<NSString *> *availableLocales =
        [NSLocale availableLocaleIdentifiers];
    // Intentionally leaked to avoid destruction order problems.
    auto *vec = new std::vector<std::u16string>();
    for (id str in availableLocales) {
      auto u16str = nsStringToU16String(str);
      // NSLocale sometimes gives locale identifiers with an underscore instead
      // of a dash. We only consider dashes valid, so fix up any identifiers we
      // get from NSLocale.
      std::replace(u16str.begin(), u16str.end(), u'_', u'-');
      // Some locales may still not be properly canonicalized (e.g. en_US_POSIX
      // should be en-US-posix).
      if (auto parsed = ParsedLocaleIdentifier::parse(u16str))
        vec->push_back(parsed->canonicalize());
    }
    return vec;
  }();
  return *availableLocales;
}
const std::u16string &getDefaultLocale() {
  static const std::u16string *defLocale = new std::u16string([] {
    // Environment variable used for testing only
    const char *testLocale = std::getenv("_HERMES_TEST_LOCALE");
    if (testLocale) {
      NSString *nsTestLocale = [NSString stringWithUTF8String:testLocale];
      return nsStringToU16String(nsTestLocale);
    }
    NSString *nsDefLocale = [[NSLocale currentLocale] localeIdentifier];
    auto defLocale = nsStringToU16String(nsDefLocale);
    // See the comment in getAvailableLocales.
    std::replace(defLocale.begin(), defLocale.end(), u'_', u'-');
    if (auto parsed = ParsedLocaleIdentifier::parse(defLocale))
      return parsed->canonicalize();
    return std::u16string(u"und");
  }());
  return *defLocale;
}

/// https://402.ecma-international.org/8.0/#sec-bestavailablelocale
std::optional<std::u16string> bestAvailableLocale(
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
      return std::nullopt;

    // c. If pos ≥ 2 and the character "-" occurs at index pos-2 of candidate,
    // decrease pos by 2.
    if (pos >= 2 && candidate[pos - 2] == '-')
      pos -= 2;

    // d. Let candidate be the substring of candidate from position 0,
    // inclusive, to position pos, exclusive.
    candidate.resize(pos);
  }
}

struct LocaleMatch {
  std::u16string locale;
  std::map<std::u16string, std::u16string> extensions;
};
/// https://402.ecma-international.org/8.0/#sec-lookupmatcher
LocaleMatch lookupMatcher(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales) {
  // 1. Let result be a new Record.
  LocaleMatch result;
  // 2. For each element locale of requestedLocales, do
  for (const std::u16string &locale : requestedLocales) {
    // a. Let noExtensionsLocale be the String value that is locale with
    // any Unicode locale extension sequences removed.
    // In practice, we can skip this step because availableLocales never
    // contains any extensions, so bestAvailableLocale will trim away any
    // unicode extensions.
    // b. Let availableLocale be BestAvailableLocale(availableLocales,
    // noExtensionsLocale).
    std::optional<std::u16string> availableLocale =
        bestAvailableLocale(availableLocales, locale);
    // c. If availableLocale is not undefined, then
    if (availableLocale) {
      // i. Set result.[[locale]] to availableLocale.
      result.locale = std::move(*availableLocale);
      // ii. If locale and noExtensionsLocale are not the same String value,
      // then
      // 1. Let extension be the String value consisting of the substring of
      // the Unicode locale extension sequence within locale.
      // 2. Set result.[[extension]] to extension.
      auto parsed = ParsedLocaleIdentifier::parse(locale);
      result.extensions = std::move(parsed->unicodeExtensionKeywords);
      // iii. Return result.
      return result;
    }
  }
  // availableLocale was undefined, so set result.[[locale]] to defLocale.
  result.locale = getDefaultLocale();
  // 5. Return result.
  return result;
}

struct ResolvedLocale {
  std::u16string locale;
  std::u16string dataLocale;
  std::unordered_map<std::u16string, std::u16string> extensions;
};
/// https://402.ecma-international.org/8.0/#sec-resolvelocale
ResolvedLocale resolveLocale(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales,
    const std::unordered_map<std::u16string, std::u16string> &options,
    llvh::ArrayRef<std::u16string_view> relevantExtensionKeys) {
  // 1. Let matcher be options.[[localeMatcher]].
  // 2. If matcher is "lookup", then
  // a. Let r be LookupMatcher(availableLocales, requestedLocales).
  // 3. Else,
  // a. Let r be BestFitMatcher(availableLocales, requestedLocales).
  auto r = lookupMatcher(availableLocales, requestedLocales);
  // 4. Let foundLocale be r.[[locale]].
  auto foundLocale = r.locale;
  // 5. Let result be a new Record.
  ResolvedLocale result;
  // 6. Set result.[[dataLocale]] to foundLocale.
  result.dataLocale = foundLocale;
  // 7. If r has an [[extension]] field, then
  // a. Let components be ! UnicodeExtensionComponents(r.[[extension]]).
  // b. Let keywords be components.[[Keywords]].
  // 8. Let supportedExtension be "-u".
  std::u16string supportedExtension = u"-u";
  // 9. For each element key of relevantExtensionKeys, do
  for (const auto &keyView : relevantExtensionKeys) {
    // TODO(T116352920): Make relevantExtensionKeys an ArrayRef<std::u16string>
    // and remove this temporary once we have constexpr std::u16string.
    std::u16string key{keyView};
    // a. Let foundLocaleData be localeData.[[<foundLocale>]].
    // NOTE: We don't actually have access to the underlying locale data, so we
    // accept everything and defer to NSLocale.
    // b. Assert: Type(foundLocaleData) is Record.
    // c. Let keyLocaleData be foundLocaleData.[[<key>]].
    // d. Assert: Type(keyLocaleData) is List.
    // e. Let value be keyLocaleData[0].
    // f. Assert: Type(value) is either String or Null.
    // g. Let supportedExtensionAddition be "".
    // h. If r has an [[extension]] field, then
    auto extIt = r.extensions.find(key);
    std::optional<std::u16string> value;
    std::u16string supportedExtensionAddition;
    // i. If keywords contains an element whose [[Key]] is the same as key, then
    if (extIt != r.extensions.end()) {
      // 1. Let entry be the element of keywords whose [[Key]] is the same as
      // key.
      // 2. Let requestedValue be entry.[[Value]].
      // 3. If requestedValue is not the empty String, then
      // a. If keyLocaleData contains requestedValue, then
      // i. Let value be requestedValue.
      // ii. Let supportedExtensionAddition be the string-concatenation of "-",
      // key, "-", and value.
      // 4. Else if keyLocaleData contains "true", then
      // a. Let value be "true".
      // b. Let supportedExtensionAddition be the string-concatenation of "-"
      // and key.
      supportedExtensionAddition.append(u"-").append(key);
      if (extIt->second.empty())
        value = u"true";
      else {
        value = extIt->second;
        supportedExtensionAddition.append(u"-").append(*value);
      }
    }
    // i. If options has a field [[<key>]], then
    auto optIt = options.find(key);
    if (optIt != options.end()) {
      // i. Let optionsValue be options.[[<key>]].
      std::u16string optionsValue = optIt->second;
      // ii. Assert: Type(optionsValue) is either String, Undefined, or Null.
      // iii. If Type(optionsValue) is String, then
      // 1. Let optionsValue be the string optionsValue after performing the
      // algorithm steps to transform Unicode extension values to canonical
      // syntax per Unicode Technical Standard #35 LDML § 3.2.1 Canonical
      // Unicode Locale Identifiers, treating key as ukey and optionsValue as
      // uvalue productions.
      // 2. Let optionsValue be the string optionsValue after performing the
      // algorithm steps to replace Unicode extension values with their
      // canonical form per Technical Standard #35 LDML § 3.2.1 Canonical
      // Unicode Locale Identifiers, treating key as ukey and optionsValue as
      // uvalue productions
      // 3. If optionsValue is the empty String, then
      if (optionsValue.empty()) {
        // a. Let optionsValue be "true".
        optionsValue = u"true";
      }
      // iv. If keyLocaleData contains optionsValue, then
      // 1. If SameValue(optionsValue, value) is false, then
      if (optionsValue != value) {
        // a. Let value be optionsValue.
        value = optionsValue;
        // b. Let supportedExtensionAddition be "".
        supportedExtensionAddition = u"";
      }
    }
    // j. Set result.[[<key>]] to value.
    if (value)
      result.extensions.emplace(key, std::move(*value));
    // k. Append supportedExtensionAddition to supportedExtension.
    supportedExtension.append(supportedExtensionAddition);
  }
  // 10. If the number of elements in supportedExtension is greater than 2, then
  if (supportedExtension.size() > 2) {
    // a. Let foundLocale be InsertUnicodeExtensionAndCanonicalize(foundLocale,
    // supportedExtension).
    foundLocale.append(supportedExtension);
  }
  // 11. Set result.[[locale]] to foundLocale.
  result.locale = std::move(foundLocale);
  // 12. Return result.
  return result;
}

/// https://402.ecma-international.org/8.0/#sec-lookupsupportedlocales
std::vector<std::u16string> lookupSupportedLocales(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales) {
  // 1. Let subset be a new empty List.
  std::vector<std::u16string> subset;
  // 2. For each element locale of requestedLocales in List order, do
  for (const std::u16string &locale : requestedLocales) {
    // a. Let noExtensionsLocale be the String value that is locale with all
    // Unicode locale extension sequences removed.
    // We can skip this step, see the comment in lookupMatcher.
    // b. Let availableLocale be BestAvailableLocale(availableLocales,
    // noExtensionsLocale).
    std::optional<std::u16string> availableLocale =
        bestAvailableLocale(availableLocales, locale);
    // c. If availableLocale is not undefined, append locale to the end of
    // subset.
    if (availableLocale) {
      subset.push_back(locale);
    }
  }
  // 3. Return subset.
  return subset;
}

/// https://402.ecma-international.org/8.0/#sec-supportedlocales
std::vector<std::u16string> supportedLocales(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales) {
  // 1. Set options to ? CoerceOptionsToObject(options).
  // 2. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  //    "lookup", "best fit" », "best fit").
  // 3. If matcher is "best fit", then
  //   a. Let supportedLocales be BestFitSupportedLocales(availableLocales,
  //      requestedLocales).
  // 4. Else,
  //   a. Let supportedLocales be LookupSupportedLocales(availableLocales,
  //      requestedLocales).
  // 5. Return CreateArrayFromList(supportedLocales).

  // We do not implement a BestFitMatcher, so we can just use LookupMatcher.
  return lookupSupportedLocales(availableLocales, requestedLocales);
}

/// https://402.ecma-international.org/8.0/#sec-canonicalizelocalelist
vm::CallResult<std::vector<std::u16string>> canonicalizeLocaleList(
    vm::Runtime &runtime,
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
  // https://402.ecma-international.org/8.0/#locale-objects As of now, 'locales'
  // can only be a string list/array. Validation occurs in normalizeLocaleList,
  // so this function just takes a vector of strings.
  // 5. Let len be ? ToLength(? Get(O, "length")).
  // 6. Let k be 0.
  // 7. Repeat, while k < len
  for (const auto &locale : locales) {
    // 7.c.vi. Let canonicalizedTag be CanonicalizeUnicodeLocaleId(tag).
    auto parsedOpt = ParsedLocaleIdentifier::parse(locale);
    if (!parsedOpt)
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid language tag: ") +
          vm::TwineChar16(locale.c_str()));
    auto canonicalizedTag = parsedOpt->canonicalize();

    // 7.c.vii. If canonicalizedTag is not an element of seen, append
    // canonicalizedTag as the last element of seen.
    if (std::find(seen.begin(), seen.end(), canonicalizedTag) == seen.end()) {
      seen.push_back(std::move(canonicalizedTag));
    }
  }
  return seen;
}

vm::CallResult<std::u16string> localeListToLocaleString(
    vm::Runtime &runtime,
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
  // We can skip this step, see the comment in lookupMatcher.
  // 7. Let availableLocales be a List with language tags that includes the
  // languages for which the Unicode Character Database contains language
  // sensitive case mappings. Implementations may add additional language tags
  // if they support case mapping for additional locales.
  // 8. Let locale be BestAvailableLocale(availableLocales, noExtensionsLocale).
  // Convert to C++ array for bestAvailableLocale function
  const std::vector<std::u16string> &availableLocales = getAvailableLocales();
  std::optional<std::u16string> locale =
      bestAvailableLocale(availableLocales, requestedLocale);
  // 9. If locale is undefined, let locale be "und".
  return locale.value_or(u"und");
}

/// https://402.ecma-international.org/8.0/#sec-getoption
/// Split into getOptionString and getOptionBool to help readability
vm::CallResult<std::optional<std::u16string>> getOptionString(
    vm::Runtime &runtime,
    const Options &options,
    const std::u16string &property,
    llvh::ArrayRef<std::u16string_view> values,
    std::optional<std::u16string_view> fallback) {
  // 1. Assert type(options) is object
  // 2. Let value be ? Get(options, property).
  auto valueIt = options.find(property);
  // 3. If value is undefined, return fallback.
  if (valueIt == options.end())
    return std::optional<std::u16string>(fallback);

  const auto &value = valueIt->second.getString();
  // 4. Assert: type is "boolean" or "string".
  // 5. If type is "boolean", then
  // a. Set value to ! ToBoolean(value).
  // 6. If type is "string", then
  // a. Set value to ? ToString(value).
  // 7. If values is not undefined and values does not contain an element equal
  // to value, throw a RangeError exception.
  if (!values.empty() && llvh::find(values, value) == values.end())
    return runtime.raiseRangeError(
        vm::TwineChar16(property.c_str()) +
        vm::TwineChar16(" value is invalid."));

  // 8. Return value.
  return std::optional<std::u16string>(value);
}

std::optional<bool> getOptionBool(
    vm::Runtime &runtime,
    const Options &options,
    const std::u16string &property,
    std::optional<bool> fallback) {
  //  1. Assert: Type(options) is Object.
  //  2. Let value be ? Get(options, property).
  auto value = options.find(property);
  //  3. If value is undefined, return fallback.
  if (value == options.end()) {
    return fallback;
  }
  //  8. Return value.
  return value->second.getBool();
}

/// https://402.ecma-international.org/8.0/#sec-defaultnumberoption
vm::CallResult<std::optional<uint8_t>> defaultNumberOption(
    vm::Runtime &runtime,
    const std::u16string &property,
    std::optional<Option> value,
    const std::uint8_t minimum,
    const std::uint8_t maximum,
    std::optional<uint8_t> fallback) {
  //  1. If value is undefined, return fallback.
  if (!value) {
    return fallback;
  }
  //  2. Set value to ? ToNumber(value).
  //  3. If value is NaN or less than minimum or greater than maximum, throw a
  //  RangeError exception.
  if (std::isnan(value->getNumber()) || value->getNumber() < minimum ||
      value->getNumber() > maximum) {
    return runtime.raiseRangeError(
        vm::TwineChar16(property.c_str()) +
        vm::TwineChar16(" value is invalid."));
  }
  //  4. Return floor(value).
  return std::optional<uint8_t>(std::floor(value->getNumber()));
}

/// https://402.ecma-international.org/8.0/#sec-getnumberoption
vm::CallResult<std::optional<uint8_t>> getNumberOption(
    vm::Runtime &runtime,
    const Options &options,
    const std::u16string &property,
    const std::uint8_t minimum,
    const std::uint8_t maximum,
    std::optional<uint8_t> fallback) {
  //  1. Assert: Type(options) is Object.
  //  2. Let value be ? Get(options, property).
  std::optional<Option> value;
  auto iter = options.find(property);
  if (iter != options.end()) {
    value = Option(iter->second);
  }
  //  3. Return ? DefaultNumberOption(value, minimum, maximum, fallback).
  auto defaultNumber =
      defaultNumberOption(runtime, property, value, minimum, maximum, fallback);
  if (LLVM_UNLIKELY(defaultNumber == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return std::optional<uint8_t>(defaultNumber.getValue());
}

// Implementation of
// https://402.ecma-international.org/8.0/#sec-todatetimeoptions
vm::CallResult<Options> toDateTimeOptions(
    vm::Runtime &runtime,
    Options options,
    std::u16string_view required,
    std::u16string_view defaults) {
  // 1. If options is undefined, let options be null; otherwise let options be ?
  // ToObject(options).
  // 2. Let options be OrdinaryObjectCreate(options).
  // 3. Let needDefaults be true.
  bool needDefaults = true;
  // 4. If required is "date" or "any", then
  if (required == u"date" || required == u"any") {
    // a. For each property name prop of « "weekday", "year", "month", "day" »,
    // do
    // TODO(T116352920): Make this a std::u16string props[] once we have
    // constexpr std::u16string.
    static constexpr std::u16string_view props[] = {
        u"weekday", u"year", u"month", u"day"};
    for (const auto &prop : props) {
      // i. Let value be ? Get(options, prop).
      if (options.find(std::u16string(prop)) != options.end()) {
        // ii. If value is not undefined, let needDefaults be false.
        needDefaults = false;
      }
    }
  }
  // 5. If required is "time" or "any", then
  if (required == u"time" || required == u"any") {
    // a. For each property name prop of « "dayPeriod", "hour", "minute",
    // "second", "fractionalSecondDigits" », do
    static constexpr std::u16string_view props[] = {
        u"dayPeriod", u"hour", u"minute", u"second", u"fractionalSecondDigits"};
    for (const auto &prop : props) {
      // i. Let value be ? Get(options, prop).
      if (options.find(std::u16string(prop)) != options.end()) {
        // ii. If value is not undefined, let needDefaults be false.
        needDefaults = false;
      }
    }
  }
  // 6. Let dateStyle be ? Get(options, "dateStyle").
  auto dateStyle = options.find(u"dateStyle");
  // 7. Let timeStyle be ? Get(options, "timeStyle").
  auto timeStyle = options.find(u"timeStyle");
  // 8. If dateStyle is not undefined or timeStyle is not undefined, let
  // needDefaults be false.
  if (dateStyle != options.end() || timeStyle != options.end()) {
    needDefaults = false;
  }
  // 9. If required is "date" and timeStyle is not undefined, then
  if (required == u"date" && timeStyle != options.end()) {
    // a. Throw a TypeError exception.
    return runtime.raiseTypeError(
        "Unexpectedly found timeStyle option for \"date\" property");
  }
  // 10. If required is "time" and dateStyle is not undefined, then
  if (required == u"time" && dateStyle != options.end()) {
    // a. Throw a TypeError exception.
    return runtime.raiseTypeError(
        "Unexpectedly found dateStyle option for \"time\" property");
  }
  // 11. If needDefaults is true and defaults is either "date" or "all", then
  if (needDefaults && (defaults == u"date" || defaults == u"all")) {
    // a. For each property name prop of « "year", "month", "day" », do
    static constexpr std::u16string_view props[] = {u"year", u"month", u"day"};
    for (const auto &prop : props) {
      // i. Perform ? CreateDataPropertyOrThrow(options, prop, "numeric").
      options.emplace(prop, Option(std::u16string(u"numeric")));
    }
  }
  // 12. If needDefaults is true and defaults is either "time" or "all", then
  if (needDefaults && (defaults == u"time" || defaults == u"all")) {
    // a. For each property name prop of « "hour", "minute", "second" », do
    static constexpr std::u16string_view props[] = {
        u"hour", u"minute", u"second"};
    for (const auto &prop : props) {
      // i. Perform ? CreateDataPropertyOrThrow(options, prop, "numeric").
      options.emplace(prop, Option(std::u16string(u"numeric")));
    }
  }
  // 13. return options
  return options;
}

// https://402.ecma-international.org/8.0/#sec-case-sensitivity-and-case-mapping
std::u16string toASCIIUppercase(std::u16string_view tz) {
  std::u16string result;
  std::uint8_t offset = 'a' - 'A';
  for (char16_t c16 : tz) {
    if (c16 >= 'a' && c16 <= 'z') {
      result.push_back((char)c16 - offset);
    } else {
      result.push_back(c16);
    }
  }
  return result;
}

static std::unordered_map<std::u16string, std::u16string> validTimeZoneNames() {
  std::unordered_map<std::u16string, std::u16string> result;
  static auto nsTimeZoneNames = [NSTimeZone.knownTimeZoneNames
      arrayByAddingObjectsFromArray:NSTimeZone.abbreviationDictionary.allKeys];
  for (NSString *timeZoneName in nsTimeZoneNames) {
    result.emplace(
        nsStringToU16String(timeZoneName.uppercaseString),
        nsStringToU16String(timeZoneName));
  }
  return result;
}

// https://402.ecma-international.org/8.0/#sec-canonicalizetimezonename
std::u16string canonicalizeTimeZoneName(std::u16string_view tz) {
  // 1. Let ianaTimeZone be the Zone or Link name of the IANA Time Zone Database
  // such that timeZone, converted to upper case as described in 6.1, is equal
  // to ianaTimeZone, converted to upper case as described in 6.1.
  static const auto timeZones = validTimeZoneNames();
  auto ianaTimeZoneIt = timeZones.find(toASCIIUppercase(tz));
  auto ianaTimeZone = (ianaTimeZoneIt != timeZones.end())
      ? ianaTimeZoneIt->second
      : std::u16string(tz);
  // NOTE: We don't use actual IANA database, so we leave (2) unimplemented.
  // 2. If ianaTimeZone is a Link name, let ianaTimeZone be the corresponding
  // Zone name as specified in the "backward" file of the IANA Time Zone
  // Database.
  // 3. If ianaTimeZone is "Etc/UTC" or "Etc/GMT", return "UTC".
  if (ianaTimeZone == u"Etc/UTC" || ianaTimeZone == u"Etc/GMT")
    ianaTimeZone = u"UTC";
  // 4. Return ianaTimeZone.
  return ianaTimeZone;
}

// https://402.ecma-international.org/8.0/#sec-defaulttimezone
std::u16string getDefaultTimeZone(NSLocale *locale) {
  return nsStringToU16String(NSTimeZone.defaultTimeZone.name);
}

// https://402.ecma-international.org/8.0/#sec-isvalidtimezonename
static bool isValidTimeZoneName(std::u16string_view tz) {
  static const auto timeZones = validTimeZoneNames();
  return timeZones.find(toASCIIUppercase(tz)) != timeZones.end();
}

// https://www.unicode.org/reports/tr35/tr35-31/tr35-dates.html#Date_Field_Symbol_Table
std::u16string getDefaultHourCycle(NSLocale *locale) {
  auto dateFormatPattern =
      nsStringToU16String([NSDateFormatter dateFormatFromTemplate:@"j"
                                                          options:0
                                                           locale:locale]);
  for (char16_t c16 : dateFormatPattern) {
    if (c16 == u'h') {
      return u"h12";
    } else if (c16 == u'K') {
      return u"h11";
    } else if (c16 == u'H') {
      return u"h23";
    }
  }
  return u"h24";
}

template <size_t N, size_t P = 0>
static constexpr bool isSorted(const std::u16string_view (&v)[N]) {
  if constexpr (P < N - 1) {
    return v[P] < v[P + 1] && isSorted<N, P + 1>(v);
  }
  return true;
}

// https://402.ecma-international.org/8.0/#sec-issanctionedsimpleunitidentifier
bool isSanctionedSimpleUnitIdentifier(std::u16string_view unitIdentifier) {
  static constexpr std::u16string_view sanctionedIdentifiers[] = {
      u"acre",       u"bit",        u"byte",
      u"celsius",    u"centimeter", u"day",
      u"degree",     u"fahrenheit", u"fluid-ounce",
      u"foot",       u"gallon",     u"gigabit",
      u"gigabyte",   u"gram",       u"hectare",
      u"hour",       u"inch",       u"kilobit",
      u"kilobyte",   u"kilogram",   u"kilometer",
      u"liter",      u"megabit",    u"megabyte",
      u"meter",      u"mile",       u"mile-scandinavian",
      u"milliliter", u"millimeter", u"millisecond",
      u"minute",     u"month",      u"ounce",
      u"percent",    u"petabyte",   u"pound",
      u"second",     u"stone",      u"terabit",
      u"terabyte",   u"week",       u"yard",
      u"year"};

  static_assert(
      isSorted(sanctionedIdentifiers), "keep sanctionedIdentifiers sorted");
  //  1. If unitIdentifier is listed in Table 2 (sanctionedIdentifiers) above,
  //  return true
  //  2. Else, return false.
  return std::binary_search(
      std::begin(sanctionedIdentifiers),
      std::end(sanctionedIdentifiers),
      unitIdentifier);
}

// https://402.ecma-international.org/8.0/#sec-iswellformedunitidentifier
bool isWellFormedUnitIdentifier(std::u16string_view unitIdentifier) {
  //  1. If the result of IsSanctionedSimpleUnitIdentifier(unitIdentifier) is
  //  true, then a. Return true.
  if (isSanctionedSimpleUnitIdentifier(unitIdentifier))
    return true;
  //  2. If the substring "-per-" does not occur exactly once in unitIdentifier,
  //  then a. Return false.
  std::u16string_view fractionDelimiter = u"-per-";
  auto foundPos = unitIdentifier.find(fractionDelimiter);
  if (foundPos == std::u16string_view::npos)
    return false;
  //  3. Let numerator be the substring of unitIdentifier from the beginning to
  //  just before "-per-".
  //  4. If the result of IsSanctionedSimpleUnitIdentifier(numerator) is false,
  //  then a. Return false.
  auto numerator = unitIdentifier.substr(0, foundPos);
  if (!isSanctionedSimpleUnitIdentifier(numerator))
    return false;
  //  5. Let denominator be the substring of unitIdentifier from just after
  //  "-per-" to the end.
  //  6. If the result of IsSanctionedSimpleUnitIdentifier(denominator) is
  //  false, then a. Return false.
  const size_t denominatorPos = foundPos + fractionDelimiter.size();
  auto denominator = unitIdentifier.substr(
      denominatorPos, unitIdentifier.length() - denominatorPos);
  if (!isSanctionedSimpleUnitIdentifier(denominator))
    return false;
  //  7. Return true.
  return true;
}

// https://402.ecma-international.org/8.0/#sec-iswellformedcurrencycode
bool isWellFormedCurrencyCode(std::u16string_view currencyCode) {
  //  1. Let normalized be the result of mapping currency to upper case as
  //  described in 6.1.
  auto normalized = toASCIIUppercase(currencyCode);
  //  2. If the number of elements in normalized is not 3, return false.
  if (normalized.size() != 3)
    return false;
  //  3. If normalized contains any character that is not in the range "A" to
  //  "Z" (U+0041 to U+005A), return false.
  if (!llvh::all_of(normalized, [](auto c) { return c >= u'A' && c <= u'Z'; }))
    return false;
  //  4. Return true.
  return true;
}

struct CurrencyInfo {
  std::u16string_view code;
  uint8_t digits;
};

template <size_t N, size_t P = 0>
static constexpr bool isSorted(const CurrencyInfo (&v)[N]) {
  if constexpr (P < N - 1) {
    return v[P].code < v[P + 1].code && isSorted<N, P + 1>(v);
  }
  return true;
}

uint8_t getCurrencyDigits(std::u16string_view code) {
  //  https://en.wikipedia.org/wiki/ISO_4217#Active_codes
  static constexpr CurrencyInfo currencies[] = {
      {u"BHD", 3}, {u"BIF", 0}, {u"CLF", 4}, {u"CLP", 0}, {u"DJF", 0},
      {u"GNF", 0}, {u"IQD", 3}, {u"ISK", 0}, {u"JOD", 3}, {u"JPY", 0},
      {u"KMF", 0}, {u"KRW", 0}, {u"KWD", 3}, {u"LYD", 3}, {u"OMR", 3},
      {u"PYG", 0}, {u"RWF", 0}, {u"TND", 3}, {u"UGX", 0}, {u"UYI", 0},
      {u"UYW", 4}, {u"VND", 0}, {u"VUV", 0}, {u"XAF", 0}, {u"XOF", 0},
      {u"XPF", 0}};
  //  1. If the ISO 4217 currency and funds code list contains currency as an
  //  alphabetic code, return the minor unit value corresponding to the currency
  //  from the list; otherwise, return 2.
  static_assert(isSorted(currencies), "keep currencies sorted by their code");
  auto it = llvh::lower_bound(currencies, code, [](auto currency, auto toFind) {
    return currency.code < toFind;
  });
  return (it != std::end(currencies) && it->code == code) ? it->digits : 2;
}
}

/// https://402.ecma-international.org/8.0/#sec-intl.getcanonicallocales
vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales) {
  // 1. Let ll be ? CanonicalizeLocaleList(locales).
  // 2. Return CreateArrayFromList(ll).
  return canonicalizeLocaleList(runtime, locales);
}

/// https://402.ecma-international.org/8.0/#sup-string.prototype.tolocalelowercase
vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime &runtime,
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

/// https://402.ecma-international.org/8.0/#sup-string.prototype.tolocaleuppercase
vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime &runtime,
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

struct Collator::Impl {
  NSLocale *nsLocale;
  NSStringCompareOptions nsCompareOptions;
  std::u16string locale;
  std::u16string usage;
  std::u16string collation;
  std::u16string caseFirst;
  std::u16string sensitivity;
  bool numeric;
  bool ignorePunctuation;
};

Collator::Collator() : impl_(std::make_unique<Impl>()) {}
Collator::~Collator() {}

/// https://402.ecma-international.org/8.0/#sec-intl.collator.supportedlocalesof
vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let availableLocales be %Collator%.[[AvailableLocales]].
  const auto &availableLocales = getAvailableLocales();
  // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  auto requestedLocalesRes = canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocalesRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 3. Return ? SupportedLocales(availableLocales, requestedLocales, options)
  return supportedLocales(availableLocales, *requestedLocalesRes);
}

/// https://402.ecma-international.org/8.0/#sec-initializecollator
vm::ExecutionStatus Collator::initialize(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  auto requestedLocalesRes = canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocalesRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 2. Set options to ? CoerceOptionsToObject(options).
  // 3. Let usage be ? GetOption(options, "usage", "string", « "sort", "search"
  // », "sort").
  static constexpr std::u16string_view usageOpts[] = {u"sort", u"search"};
  auto usageRes =
      getOptionString(runtime, options, u"usage", usageOpts, {u"sort"});
  if (LLVM_UNLIKELY(usageRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 4. Set collator.[[Usage]] to usage.
  impl_->usage = std::move(**usageRes);
  // 5. If usage is "sort", then
  // a. Let localeData be %Collator%.[[SortLocaleData]].
  // 6. Else,
  // a. Let localeData be %Collator%.[[SearchLocaleData]].
  // 7. Let opt be a new Record.
  std::unordered_map<std::u16string, std::u16string> opt;
  // 8. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  //    "lookup", "best fit" », "best fit").
  // 9. Set opt.[[localeMatcher]] to matcher.
  // We only implement lookup matcher, so we don't need to record this.
  static constexpr std::u16string_view localeMatcherOpts[] = {
      u"lookup", u"best fit"};
  auto localeMatcherRes = getOptionString(
      runtime, options, u"localeMatcher", localeMatcherOpts, u"best fit");
  if (LLVM_UNLIKELY(localeMatcherRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 10. Let collation be ? GetOption(options, "collation", "string", undefined,
  // undefined).
  auto collationRes = getOptionString(runtime, options, u"collation", {}, {});
  if (LLVM_UNLIKELY(collationRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 11. If collation is not undefined, then
  // a. If collation does not match the Unicode Locale Identifier type
  // nonterminal, throw a RangeError exception.
  // 12. Set opt.[[co]] to collation.
  if (auto &collationOpt = *collationRes) {
    if (!isUnicodeExtensionType(*collationOpt)) {
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid collation: ") +
          vm::TwineChar16(collationOpt->c_str()));
    }
    opt.emplace(u"co", std::move(*collationOpt));
  }
  // 13. Let numeric be ? GetOption(options, "numeric", "boolean", undefined,
  // undefined).
  auto numericOpt = getOptionBool(runtime, options, u"numeric", {});
  // 14. If numeric is not undefined, then
  // a. Let numeric be ! ToString(numeric).
  // 15. Set opt.[[kn]] to numeric.
  if (numericOpt)
    opt.emplace(u"kn", *numericOpt ? u"true" : u"false");
  // 16. Let caseFirst be ? GetOption(options, "caseFirst", "string", « "upper",
  // "lower", "false" », undefined).
  static constexpr std::u16string_view caseFirstOpts[] = {
      u"upper", u"lower", u"false"};
  auto caseFirstRes =
      getOptionString(runtime, options, u"caseFirst", caseFirstOpts, {});
  if (LLVM_UNLIKELY(caseFirstRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 17. Set opt.[[kf]] to caseFirst.
  if (auto caseFirstOpt = *caseFirstRes)
    opt.emplace(u"kf", *caseFirstOpt);
  // 18. Let relevantExtensionKeys be %Collator%.[[RelevantExtensionKeys]].
  static constexpr std::u16string_view relevantExtensionKeys[] = {
      u"co", u"kn", u"kf"};
  // 19. Let r be ResolveLocale(%Collator%.[[AvailableLocales]],
  // requestedLocales, opt,relevantExtensionKeys, localeData).
  auto r = resolveLocale(
      getAvailableLocales(), *requestedLocalesRes, opt, relevantExtensionKeys);
  // 20. Set collator.[[Locale]] to r.[[locale]].
  impl_->locale = std::move(r.locale);
  // 21. Let collation be r.[[co]].
  auto coIt = r.extensions.find(u"co");
  // 22. If collation is null, let collation be "default".
  // 23. Set collator.[[Collation]] to collation.
  if (coIt == r.extensions.end())
    impl_->collation = u"default";
  else
    impl_->collation = std::move(coIt->second);
  // 24. If relevantExtensionKeys contains "kn", then
  // a. Set collator.[[Numeric]] to ! SameValue(r.[[kn]], "true").
  auto knIt = r.extensions.find(u"kn");
  if (knIt == r.extensions.end())
    impl_->numeric = false;
  else
    impl_->numeric = (knIt->second == u"true");

  // 25. If relevantExtensionKeys contains "kf", then
  // a. Set collator.[[CaseFirst]] to r.[[kf]].
  auto kfIt = r.extensions.find(u"kf");
  if (kfIt == r.extensions.end())
    impl_->caseFirst = u"false";
  else
    impl_->caseFirst = kfIt->second;

  // 26. Let sensitivity be ? GetOption(options, "sensitivity", "string", «
  // "base", "accent", "case", "variant" », undefined).
  static constexpr std::u16string_view sensitivityOpts[] = {
      u"base", u"accent", u"case", u"variant"};
  auto sensitivityRes =
      getOptionString(runtime, options, u"sensitivity", sensitivityOpts, {});
  if (LLVM_UNLIKELY(sensitivityRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 27. If sensitivity is undefined, then
  // a. If usage is "sort", then
  // i. Let sensitivity be "variant".
  // b. Else,
  // i. Let dataLocale be r.[[dataLocale]].
  // ii. Let dataLocaleData be localeData.[[<dataLocale>]].
  // iii. Let sensitivity be dataLocaleData.[[sensitivity]].
  // 28. Set collator.[[Sensitivity]] to sensitivity.
  if (auto &sensitivityOpt = *sensitivityRes)
    impl_->sensitivity = std::move(*sensitivityOpt);
  else
    impl_->sensitivity = u"variant";

  // 29. Let ignorePunctuation be ? GetOption(options, "ignorePunctuation",
  // "boolean", undefined,false).
  auto ignorePunctuationOpt =
      getOptionBool(runtime, options, u"ignorePunctuation", false);
  // 30. Set collator.[[IgnorePunctuation]] to ignorePunctuation.
  impl_->ignorePunctuation = *ignorePunctuationOpt;

  // Set up the state for calling into Obj-C APIs.
  NSStringCompareOptions cmpOpts = 0;
  if (impl_->numeric)
    cmpOpts |= NSNumericSearch;
  if (impl_->sensitivity == u"base")
    cmpOpts |= (NSDiacriticInsensitiveSearch | NSCaseInsensitiveSearch);
  else if (impl_->sensitivity == u"accent")
    cmpOpts |= NSCaseInsensitiveSearch;
  else if (impl_->sensitivity == u"case")
    cmpOpts |= NSDiacriticInsensitiveSearch;
  impl_->nsCompareOptions = cmpOpts;

  std::u16string nsLocaleExtensions;
  if (impl_->collation != u"default")
    nsLocaleExtensions.append(u"-co-").append(impl_->collation);
  else if (impl_->usage == u"search")
    nsLocaleExtensions.append(u"-co-search");
  if (impl_->caseFirst != u"false")
    nsLocaleExtensions.append(u"-kf-").append(impl_->caseFirst);
  auto nsLocaleIdentifier = r.dataLocale;
  if (!nsLocaleExtensions.empty())
    nsLocaleIdentifier.append(u"-u").append(nsLocaleExtensions);
  impl_->nsLocale = [NSLocale
      localeWithLocaleIdentifier:u16StringToNSString(nsLocaleIdentifier)];
  // 31. Return collator.
  return vm::ExecutionStatus::RETURNED;
}

/// https://402.ecma-international.org/8.0/#sec-intl.collator.prototype.resolvedoptions
Options Collator::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"usage", Option(impl_->usage));
  options.emplace(u"sensitivity", Option(impl_->sensitivity));
  options.emplace(u"ignorePunctuation", Option(impl_->ignorePunctuation));
  options.emplace(u"collation", Option(impl_->collation));
  options.emplace(u"numeric", Option(impl_->numeric));
  options.emplace(u"caseFirst", Option(impl_->caseFirst));
  return options;
}

/// https://402.ecma-international.org/8.0/#sec-intl.collator.prototype.compare
double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  NSString *nsX = u16StringToNSString(x);
  NSString *nsY = u16StringToNSString(y);
  if (impl_->ignorePunctuation) {
    // Unfortunately, NSLocale does not provide a way to specify alternate
    // handling, so we simulate it by manually stripping punctuation and
    // whitespace.
    auto *set = [NSMutableCharacterSet punctuationCharacterSet];
    [set formUnionWithCharacterSet:[NSCharacterSet
                                       whitespaceAndNewlineCharacterSet]];
    auto removePunctuation = [&](NSString *nsStr) {
      auto *res = [NSMutableString new];
      for (size_t i = 0; i < nsStr.length; ++i)
        if (![set characterIsMember:[nsStr characterAtIndex:i]])
          [res appendFormat:@"%c", [nsStr characterAtIndex:i]];
      return res;
    };
    nsX = removePunctuation(nsX);
    nsY = removePunctuation(nsY);
  }
  return [nsX compare:nsY
              options:impl_->nsCompareOptions
                range:NSMakeRange(0, nsX.length)
               locale:impl_->nsLocale];
}

// Implementation of
// https://402.ecma-international.org/8.0/#datetimeformat-objects
struct DateTimeFormat::Impl {
  // https://402.ecma-international.org/8.0/#sec-properties-of-intl-datetimeformat-instances
  // Intl.DateTimeFormat instances have an [[InitializedDateTimeFormat]]
  // internal slot.
  // NOTE: InitializedDateTimeFormat is not implemented.
  // Intl.DateTimeFormat instances also have several internal
  // slots that are computed by the constructor:
  // [[Locale]] is a String value with the language tag of the locale whose
  // localization is used for formatting.
  std::u16string locale;
  // [[Calendar]] is a String value with the "type" given in Unicode Technical
  // Standard 35 for the calendar used for formatting.
  std::optional<std::u16string> calendar;
  // [[NumberingSystem]] is a String value with the "type" given in Unicode
  // Technical Standard 35 for the numbering system used for formatting.
  // NOTE: Even though NSDateFormatter formats date and time using different
  // numbering systems based on its "locale" value, it does not allow to set/get
  // the numbering system value explicitly. So we consider this feature
  // unsupported.
  // [[TimeZone]] is a String value with the IANA time zone name of the time
  // zone used for formatting.
  std::u16string timeZone;
  // [[Weekday]], [[Era]], [[Year]], [[Month]], [[Day]], [[DayPeriod]],
  // [[Hour]], [[Minute]], [[Second]], [[TimeZoneName]] are each either
  // undefined, indicating that the component is not used for formatting, or one
  // of the String values given in Table 4, indicating how the component should
  // be presented in the formatted output.
  std::optional<std::u16string> weekday;
  std::optional<std::u16string> era;
  std::optional<std::u16string> year;
  std::optional<std::u16string> month;
  std::optional<std::u16string> day;
  std::optional<std::u16string> dayPeriod;
  std::optional<std::u16string> hour;
  std::optional<std::u16string> minute;
  std::optional<std::u16string> second;
  std::optional<std::u16string> timeZoneName;
  // [[FractionalSecondDigits]] is either undefined or a positive, non-zero
  // integer Number value indicating the fraction digits to be used for
  // fractional seconds. Numbers will be rounded or padded with trailing zeroes
  // if necessary.
  std::optional<uint8_t> fractionalSecondDigits;
  // [[HourCycle]] is a String value indicating whether the 12-hour format
  // ("h11", "h12") or the 24-hour format ("h23", "h24") should be used. "h11"
  // and "h23" start with hour 0 and go up to 11 and 23 respectively. "h12" and
  // "h24" start with hour 1 and go up to 12 and 24. [[HourCycle]] is only used
  // when [[Hour]] is not undefined.
  std::optional<std::u16string> hourCycle;
  // [[DateStyle]], [[TimeStyle]] are each either undefined, or a String value
  // with values "full", "long", "medium", or "short".
  std::optional<std::u16string> dateStyle;
  std::optional<std::u16string> timeStyle;
  // [[Pattern]] is a String value as described in 11.3.3.
  // [[RangePatterns]] is a Record as described in 11.3.3.
  // Finally, Intl.DateTimeFormat instances have a [[BoundFormat]]
  // internal slot that caches the function returned by the format accessor
  // (11.4.3).
  // NOTE: Pattern and RangePatterns are not implemented. BoundFormat is
  // implemented in Intl.cpp.
  NSDateFormatter *getNSDateFormatter() noexcept;
};

DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}
DateTimeFormat::~DateTimeFormat() {}

// Implementation of
// https://402.ecma-international.org/8.0/#sec-intl.datetimeformat.supportedlocalesof
vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let availableLocales be %DateTimeFormat%.[[AvailableLocales]].
  // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  auto requestedLocales = getCanonicalLocales(runtime, locales);
  const std::vector<std::u16string> &availableLocales = getAvailableLocales();
  // 3. Return ? (availableLocales, requestedLocales, options).
  return supportedLocales(availableLocales, requestedLocales.getValue());
}

// Implementation of
// https://402.ecma-international.org/8.0/#sec-initializedatetimeformat
vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &inputOptions) noexcept {
  // 1. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  auto requestedLocalesRes = canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocalesRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 2. Let options be ? ToDateTimeOptions(options, "any", "date").
  auto optionsRes = toDateTimeOptions(runtime, inputOptions, u"any", u"date");
  if (LLVM_UNLIKELY(optionsRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto options = *optionsRes;
  // 3. Let opt be a new Record.
  std::unordered_map<std::u16string, std::u16string> opt;
  // 4. Let matcher be ? GetOption(options, "localeMatcher", "string",
  // «"lookup", "best fit" », "best fit").
  auto matcherRes = getOptionString(
      runtime,
      options,
      u"localeMatcher",
      {u"lookup", u"best fit"},
      u"best fit");
  if (LLVM_UNLIKELY(matcherRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 5. Set opt.[[localeMatcher]] to matcher.
  auto matcherOpt = *matcherRes;
  opt.emplace(u"localeMatcher", *matcherOpt);
  // 6. Let calendar be ? GetOption(options, "calendar", "string",
  // undefined, undefined).
  auto calendarRes = getOptionString(runtime, options, u"calendar", {}, {});
  // 7. If calendar is not undefined, then
  if (LLVM_UNLIKELY(calendarRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 8. Set opt.[[ca]] to calendar.
  if (auto calendarOpt = *calendarRes) {
    // a. If calendar does not match the Unicode Locale Identifier type
    // nonterminal, throw a RangeError exception.
    if (!isUnicodeExtensionType(*calendarOpt))
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid calendar: ") +
          vm::TwineChar16(calendarOpt->c_str()));
    opt.emplace(u"ca", *calendarOpt);
  }
  // 9. Let numberingSystem be ? GetOption(options, "numberingSystem",
  // "string", undefined, undefined).
  // 10. If numberingSystem is not undefined, then
  // a. If numberingSystem does not match the Unicode Locale Identifier
  // type nonterminal, throw a RangeError exception.
  // 11. Set opt.[[nu]] to numberingSystem.
  opt.emplace(u"nu", u"");
  // 12. Let hour12 be ? GetOption(options, "hour12", "boolean",
  // undefined, undefined).
  auto hour12 = getOptionBool(runtime, options, u"hour12", {});
  // 13. Let hourCycle be ? GetOption(options, "hourCycle", "string", «
  // "h11", "h12", "h23", "h24" », undefined).
  static constexpr std::u16string_view hourCycles[] = {
      u"h11", u"h12", u"h23", u"h24"};
  auto hourCycleRes =
      getOptionString(runtime, options, u"hourCycle", hourCycles, {});
  if (LLVM_UNLIKELY(hourCycleRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto hourCycleOpt = *hourCycleRes;
  // 14. If hour12 is not undefined, then
  if (hour12.has_value())
    // a. Let hourCycle be null.
    // NOTE: We would normally just don't add this to the "opt" map, but
    // resolveLocale actually checks for presence of keys, even if values are
    // null or undefined.
    hourCycleOpt = u"";
  if (hourCycleOpt.has_value())
    // 15. Set opt.[[hc]] to hourCycle.
    opt.emplace(u"hc", *hourCycleOpt);
  // 16. Let localeData be %DateTimeFormat%.[[LocaleData]].
  // NOTE: We don't actually have access to the underlying locale data, so we
  // will use NSLocale.currentLocale instance as a substitute
  auto localeData = NSLocale.currentLocale;
  // 17. Let r be ResolveLocale(%DateTimeFormat%.[[AvailableLocales]],
  // requestedLocales, opt, %DateTimeFormat%.[[RelevantExtensionKeys]],
  // localeData).
  static constexpr std::u16string_view relevantExtensionKeys[] = {
      u"ca", u"nu", u"hc"};
  auto r = resolveLocale(
      getAvailableLocales(), *requestedLocalesRes, opt, relevantExtensionKeys);
  // 18. Set dateTimeFormat.[[Locale]] to r.[[locale]].
  impl_->locale = std::move(r.locale);
  // 19. Let calendar be r.[[ca]].
  auto caIt = r.extensions.find(u"ca");
  // 20. Set dateTimeFormat.[[Calendar]] to calendar.
  if (caIt != r.extensions.end())
    impl_->calendar = std::move(caIt->second);
  // 21. Set dateTimeFormat.[[HourCycle]] to r.[[hc]].
  auto hcIt = r.extensions.find(u"hc");
  if (hcIt != r.extensions.end())
    impl_->hourCycle = std::move(hcIt->second);
  // 22. Set dateTimeFormat.[[NumberingSystem]] to r.[[nu]].
  // 23. Let dataLocale be r.[[dataLocale]].
  auto dataLocale = r.dataLocale;
  // 24. Let timeZone be ? Get(options, "timeZone").
  auto timeZoneIt = options.find(u"timeZone");
  std::u16string timeZone;
  //  25. If timeZone is undefined, then
  if (timeZoneIt == options.end()) {
    // a. Let timeZone be DefaultTimeZone().
    timeZone = getDefaultTimeZone(localeData);
    // 26. Else,
  } else {
    // a. Let timeZone be ? ToString(timeZone).
    timeZone = timeZoneIt->second.getString();
    // b. If the result of IsValidTimeZoneName(timeZone) is false, then
    if (!isValidTimeZoneName(timeZone)) {
      // i. Throw a RangeError exception.
      return runtime.raiseRangeError("Incorrect timeZone information provided");
    }
    // c. Let timeZone be CanonicalizeTimeZoneName(timeZone).
    timeZone = canonicalizeTimeZoneName(timeZone);
  }
  // 27. Set dateTimeFormat.[[TimeZone]] to timeZone.
  impl_->timeZone = timeZone;
  // 28. Let opt be a new Record.
  // 29. For each row of Table 4, except the header row, in table order, do
  // a. Let prop be the name given in the Property column of the row.
  // b. If prop is "fractionalSecondDigits", then
  // i. Let value be ? GetNumberOption(options, "fractionalSecondDigits", 1,
  // 3, undefined).
  // d. Set opt.[[<prop>]] to value.
  // c. Else,
  // i. Let value be ? GetOption(options, prop, "string", « the strings
  // given in the Values column of the row », undefined).
  // d. Set opt.[[<prop>]] to value.
  // 30. Let dataLocaleData be localeData.[[<dataLocale>]].
  // 31. Let matcher be ? GetOption(options, "formatMatcher", "string", «
  // "basic", "best fit" », "best fit").
  // 32. Let dateStyle be ? GetOption(options, "dateStyle", "string", « "full",
  // "long", "medium", "short" », undefined).
  static constexpr std::u16string_view dateStyles[] = {
      u"full", u"long", u"medium", u"short"};
  auto dateStyleRes =
      getOptionString(runtime, options, u"dateStyle", dateStyles, {});
  if (LLVM_UNLIKELY(dateStyleRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 33. Set dateTimeFormat.[[DateStyle]] to dateStyle.
  impl_->dateStyle = *dateStyleRes;
  // 34. Let timeStyle be ? GetOption(options, "timeStyle", "string", « "full",
  // "long", "medium", "short" », undefined).
  static constexpr std::u16string_view timeStyles[] = {
      u"full", u"long", u"medium", u"short"};
  auto timeStyleRes =
      getOptionString(runtime, options, u"timeStyle", timeStyles, {});
  if (LLVM_UNLIKELY(timeStyleRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 35. Set dateTimeFormat.[[TimeStyle]] to timeStyle.
  impl_->timeStyle = *timeStyleRes;

  // Initialize properties using values from the input options.
  static constexpr std::u16string_view weekdayValues[] = {
      u"narrow", u"short", u"long"};
  auto weekdayRes =
      getOptionString(runtime, inputOptions, u"weekday", weekdayValues, {});
  if (LLVM_UNLIKELY(weekdayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->weekday = *weekdayRes;

  static constexpr std::u16string_view eraValues[] = {
      u"narrow", u"short", u"long"};
  auto eraRes = getOptionString(runtime, inputOptions, u"era", eraValues, {});
  if (LLVM_UNLIKELY(eraRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->era = *eraRes;

  static constexpr std::u16string_view yearValues[] = {u"2-digit", u"numeric"};
  auto yearRes =
      getOptionString(runtime, inputOptions, u"year", yearValues, {});
  if (LLVM_UNLIKELY(yearRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->year = *yearRes;

  static constexpr std::u16string_view monthValues[] = {
      u"2-digit", u"numeric", u"narrow", u"short", u"long"};
  auto monthRes =
      getOptionString(runtime, inputOptions, u"month", monthValues, {});
  if (LLVM_UNLIKELY(monthRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->month = *monthRes;

  static constexpr std::u16string_view dayValues[] = {u"2-digit", u"numeric"};
  auto dayRes = getOptionString(runtime, inputOptions, u"day", dayValues, {});
  if (LLVM_UNLIKELY(dayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->day = *dayRes;

  static constexpr std::u16string_view dayPeriodValues[] = {
      u"narrow", u"short", u"long"};
  auto dayPeriodRes =
      getOptionString(runtime, inputOptions, u"dayPeriod", dayPeriodValues, {});
  if (LLVM_UNLIKELY(dayPeriodRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->dayPeriod = *dayPeriodRes;

  static constexpr std::u16string_view hourValues[] = {u"2-digit", u"numeric"};
  auto hourRes =
      getOptionString(runtime, inputOptions, u"hour", hourValues, {});
  if (LLVM_UNLIKELY(hourRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->hour = *hourRes;

  static constexpr std::u16string_view minuteValues[] = {
      u"2-digit", u"numeric"};
  auto minuteRes =
      getOptionString(runtime, inputOptions, u"minute", minuteValues, {});
  if (LLVM_UNLIKELY(minuteRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->minute = *minuteRes;

  static constexpr std::u16string_view secondValues[] = {
      u"2-digit", u"numeric"};
  auto secondRes =
      getOptionString(runtime, inputOptions, u"second", secondValues, {});
  if (LLVM_UNLIKELY(secondRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->second = *secondRes;

  auto fractionalSecondDigitsRes = getNumberOption(
      runtime, inputOptions, u"fractionalSecondDigits", 1, 3, {});
  if (LLVM_UNLIKELY(
          fractionalSecondDigitsRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->fractionalSecondDigits = *fractionalSecondDigitsRes;

  // NOTE: "shortOffset", "longOffset", "shortGeneric", "longGeneric"
  // are specified here:
  // https://tc39.es/proposal-intl-extend-timezonename
  // they are not in ecma402 spec, but there is a test for them:
  // "test262/test/intl402/DateTimeFormat/constructor-options-timeZoneName-valid.js"
  static constexpr std::u16string_view timeZoneNameValues[] = {
      u"short",
      u"long",
      u"shortOffset",
      u"longOffset",
      u"shortGeneric",
      u"longGeneric"};
  auto timeZoneNameRes = getOptionString(
      runtime, inputOptions, u"timeZoneName", timeZoneNameValues, {});
  if (LLVM_UNLIKELY(timeZoneNameRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  impl_->timeZoneName = *timeZoneNameRes;
  // NOTE: We don't have access to localeData, instead we'll defer to NSLocale
  // wherever it is needed.
  // 36. If dateStyle is not undefined or timeStyle is not undefined, then
  // a. For each row in Table 4, except the header row, do
  // i. Let prop be the name given in the Property column of the row.
  // ii. Let p be opt.[[<prop>]].
  // iii. If p is not undefined, then
  // 1. Throw a TypeError exception.
  // b. Let styles be dataLocaleData.[[styles]].[[<calendar>]].
  // c. Let bestFormat be DateTimeStyleFormat(dateStyle, timeStyle, styles).
  // 37. Else,
  // a. Let formats be dataLocaleData.[[formats]].[[<calendar>]].
  // b. If matcher is "basic", then
  // i. Let bestFormat be BasicFormatMatcher(opt, formats).
  // c. Else,
  // i. Let bestFormat be BestFitFormatMatcher(opt, formats).
  // 38. For each row in Table 4, except the header row, in table order, do
  // for (auto const &row : table4) {
  // a. Let prop be the name given in the Property column of the row.
  // auto prop = row.first;
  // b. If bestFormat has a field [[<prop>]], then
  // i. Let p be bestFormat.[[<prop>]].
  // ii. Set dateTimeFormat's internal slot whose name is the Internal
  // Slot column of the row to p.
  // 39. If dateTimeFormat.[[Hour]] is undefined, then
  if (!impl_->hour.has_value()) {
    // a. Set dateTimeFormat.[[HourCycle]] to undefined.
    impl_->hourCycle = std::nullopt;
    // b. Let pattern be bestFormat.[[pattern]].
    // c. Let rangePatterns be bestFormat.[[rangePatterns]].
    // 40. Else,
  } else {
    // a. Let hcDefault be dataLocaleData.[[hourCycle]].
    auto hcDefault = getDefaultHourCycle(localeData);
    // b. Let hc be dateTimeFormat.[[HourCycle]].
    auto hc = impl_->hourCycle;
    // c. If hc is null, then
    if (!hc.has_value())
      // i. Set hc to hcDefault.
      hc = hcDefault;
    // d. If hour12 is not undefined, then
    if (hour12.has_value()) {
      // i. If hour12 is true, then
      if (*hour12 == true) {
        // 1. If hcDefault is "h11" or "h23", then
        if (hcDefault == u"h11" || hcDefault == u"h23") {
          // a. Set hc to "h11".
          hc = u"h11";
          // 2. Else,
        } else {
          // a. Set hc to "h12".
          hc = u"h12";
        }
        // ii. Else,
      } else {
        // 1. Assert: hour12 is false.
        // 2. If hcDefault is "h11" or "h23", then
        if (hcDefault == u"h11" || hcDefault == u"h23") {
          // a. Set hc to "h23".
          hc = u"h23";
          // 3. Else,
        } else {
          // a. Set hc to "h24".
          hc = u"h24";
        }
      }
    }
    // e. Set dateTimeFormat.[[HourCycle]] to hc.
    impl_->hourCycle = hc;
    // f. If dateTimeformat.[[HourCycle]] is "h11" or "h12", then
    // i. Let pattern be bestFormat.[[pattern12]].
    // ii. Let rangePatterns be bestFormat.[[rangePatterns12]].
    // g. Else,
    // i. Let pattern be bestFormat.[[pattern]].
    // ii. Let rangePatterns be bestFormat.[[rangePatterns]].
  }
  // 41. Set dateTimeFormat.[[Pattern]] to pattern.
  // 42. Set dateTimeFormat.[[RangePatterns]] to rangePatterns.
  // 43. Return dateTimeFormat.
  return vm::ExecutionStatus::RETURNED;
}
// Implementer note: This method corresponds roughly to
// https://402.ecma-international.org/8.0/#sec-intl.datetimeformat.prototype.resolvedoptions
Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"timeZone", Option(impl_->timeZone));
  if (impl_->calendar)
    options.emplace(u"calendar", Option(*impl_->calendar));
  if (impl_->hourCycle.has_value()) {
    options.emplace(u"hourCycle", *impl_->hourCycle);
    options.emplace(
        u"hour12", impl_->hourCycle == u"h11" || impl_->hourCycle == u"h12");
  }
  if (impl_->weekday.has_value())
    options.emplace(u"weekday", *impl_->weekday);
  if (impl_->era.has_value())
    options.emplace(u"era", *impl_->era);
  if (impl_->year.has_value())
    options.emplace(u"year", *impl_->year);
  if (impl_->month.has_value())
    options.emplace(u"month", *impl_->month);
  if (impl_->day.has_value())
    options.emplace(u"day", *impl_->day);
  if (impl_->hour.has_value())
    options.emplace(u"hour", *impl_->hour);
  if (impl_->minute.has_value())
    options.emplace(u"minute", *impl_->minute);
  if (impl_->second.has_value())
    options.emplace(u"second", *impl_->second);
  if (impl_->timeZoneName.has_value())
    options.emplace(u"timeZoneName", *impl_->timeZoneName);
  if (impl_->dateStyle.has_value())
    options.emplace(u"dateStyle", *impl_->dateStyle);
  if (impl_->timeStyle.has_value())
    options.emplace(u"timeStyle", *impl_->timeStyle);
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
static enum_string formatDate(std::u16string const &inString) {
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

NSDateFormatter *DateTimeFormat::Impl::getNSDateFormatter() noexcept {
  NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
  if (timeStyle.has_value()) {
    switch (formatDate(*timeStyle)) {
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
  if (dateStyle.has_value()) {
    switch (formatDate(*dateStyle)) {
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
  dateFormatter.timeZone =
      [[NSTimeZone alloc] initWithName:u16StringToNSString(timeZone)];
  dateFormatter.locale =
      [[NSLocale alloc] initWithLocaleIdentifier:u16StringToNSString(locale)];
  if (calendar)
    dateFormatter.calendar = [[NSCalendar alloc]
        initWithCalendarIdentifier:u16StringToNSString(*calendar)];
  if (timeStyle.has_value() || dateStyle.has_value())
    return dateFormatter;
  // The following options cannot be used in conjunction with timeStyle or
  // dateStyle
  // Form a custom format string It will be reordered according to
  // locale later
  NSMutableString *customFormattedDate = [[NSMutableString alloc] init];
  if (timeZoneName.has_value()) {
    switch (formatDate(*timeZoneName)) {
      case eShort:
        [customFormattedDate appendString:@"z"];
        break;
      case eLong:
        [customFormattedDate appendString:@"zzzz"];
        break;
      case eShortOffset:
        [customFormattedDate appendString:@"O"];
        break;
      case eLongOffset:
        [customFormattedDate appendString:@"OOOO"];
        break;
      case eShortGeneric:
        [customFormattedDate appendString:@"v"];
        break;
      case eLongGeneric:
        [customFormattedDate appendString:@"vvvv"];
        break;
      default:
        [customFormattedDate appendString:@"z"];
    }
  }
  if (era.has_value()) {
    switch (formatDate(*era)) {
      case eNarrow:
        [customFormattedDate appendString:@"GGGGG"];
        break;
      case eShort:
        [customFormattedDate appendString:@"G"];
        break;
      case eLong:
        [customFormattedDate appendString:@"GGGG"];
        break;
      default:
        [customFormattedDate appendString:@"G"];
    }
  }
  if (year.has_value()) {
    switch (formatDate(*year)) {
      case eNumeric:
        [customFormattedDate appendString:@"yyyy"];
        break;
      case eTwoDigit:
        [customFormattedDate appendString:@"yy"];
        break;
      default:
        [customFormattedDate appendString:@"yyyy"];
    }
  }
  if (month.has_value()) {
    switch (formatDate(*month)) {
      case eNarrow:
        [customFormattedDate appendString:@"MMMMM"];
        break;
      case eNumeric:
        [customFormattedDate appendString:@"M"];
        break;
      case eTwoDigit:
        [customFormattedDate appendString:@"MM"];
        break;
      case eShort:
        [customFormattedDate appendString:@"MMM"];
        break;
      case eLong:
        [customFormattedDate appendString:@"MMMM"];
        break;
      default:
        [customFormattedDate appendString:@"MMM"];
    }
  }
  if (weekday.has_value()) {
    switch (formatDate(*weekday)) {
      case eNarrow:
        [customFormattedDate appendString:@"EEEEE"];
        break;
      case eShort:
        [customFormattedDate appendString:@"E"];
        break;
      case eLong:
        [customFormattedDate appendString:@"EEEE"];
        break;
      default:
        [customFormattedDate appendString:@"E"];
    }
  }
  if (day.has_value()) {
    switch (formatDate(*day)) {
      case eNumeric:
        [customFormattedDate appendString:@"d"];
        break;
      case eTwoDigit:
        [customFormattedDate appendString:@"dd"];
        break;
      default:
        [customFormattedDate appendString:@"dd"];
    }
  }
  if (hour.has_value()) {
    // Ignore the hour12 bool in the impl_ struct
    // a = AM/PM for 12 hr clocks, automatically added depending on locale
    // AM/PM not multilingual, de-DE should be "03 Uhr" not "3 AM"
    // K = h11 = 0-11
    // h = h12 = 1-12
    // H = h23 = 0-23
    // k = h24 = 1-24
    if (hourCycle == u"h12") {
      switch (formatDate(*hour)) {
        case eNumeric:
          [customFormattedDate appendString:@"h"];
          break;
        case eTwoDigit:
          [customFormattedDate appendString:@"hh"];
          break;
        default:
          [customFormattedDate appendString:@"hh"];
      }
    } else if (hourCycle == u"h24") {
      switch (formatDate(*hour)) {
        case eNumeric:
          [customFormattedDate appendString:@"k"];
          break;
        case eTwoDigit:
          [customFormattedDate appendString:@"kk"];
          break;
        default:
          [customFormattedDate appendString:@"kk"];
      }
    } else if (hourCycle == u"h11") {
      switch (formatDate(*hour)) {
        case eNumeric:
          [customFormattedDate appendString:@"K"];
          break;
        case eTwoDigit:
          [customFormattedDate appendString:@"KK"];
          break;
        default:
          [customFormattedDate appendString:@"KK"];
      }
    } else { // h23
      switch (formatDate(*hour)) {
        case eNumeric:
          [customFormattedDate appendString:@"H"];
          break;
        case eTwoDigit:
          [customFormattedDate appendString:@"HH"];
          break;
        default:
          [customFormattedDate appendString:@"HH"];
      }
    }
  }
  if (minute.has_value()) {
    switch (formatDate(*minute)) {
      case eNumeric:
        [customFormattedDate appendString:@"m"];
        break;
      case eTwoDigit:
        [customFormattedDate appendString:@"mm"];
        break;
      default:
        [customFormattedDate appendString:@"m"];
    }
  }
  if (second.has_value()) {
    switch (formatDate(*second)) {
      case eNumeric:
        [customFormattedDate appendString:@"s"];
        break;
      case eTwoDigit:
        [customFormattedDate appendString:@"ss"];
        break;
      default:
        [customFormattedDate appendString:@"s"];
    }
  }
  if (fractionalSecondDigits.has_value()) {
    // This currently outputs to 3 digits only with the date?
    switch (*fractionalSecondDigits) {
      case 1:
        [customFormattedDate appendString:@"S"];
      case 2:
        [customFormattedDate appendString:@"SS"];
      case 3:
        [customFormattedDate appendString:@"SSS"];
    }
  }
  // Not supported - dayPeriod (at night/in the morning)
  // Set the custom date from all the concatonated NSStrings (locale will
  // automatically separate the order) Only set a template format if it isn't
  // empty
  if (customFormattedDate.length > 0) {
    [dateFormatter setLocalizedDateFormatFromTemplate:customFormattedDate];
  } else {
    dateFormatter.dateStyle = NSDateFormatterShortStyle;
  }

  return dateFormatter;
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  auto timeInSeconds = jsTimeValue / 1000;
  NSDate *date = [NSDate dateWithTimeIntervalSince1970:timeInSeconds];
  NSDateFormatter *dateFormatter = impl_->getNSDateFormatter();
  return nsStringToU16String([dateFormatter stringFromDate:date]);
}

static std::u16string returnTypeOfDate(const char16_t &c16) {
  if (c16 == u'a')
    return u"dayPeriod";
  if (c16 == u'z' || c16 == u'v' || c16 == u'O')
    return u"timeZoneName";
  if (c16 == u'G')
    return u"era";
  if (c16 == u'y')
    return u"year";
  if (c16 == u'M')
    return u"month";
  if (c16 == u'E')
    return u"weekday";
  if (c16 == u'd')
    return u"day";
  if (c16 == u'h' || c16 == u'k' || c16 == u'K' || c16 == u'H')
    return u"hour";
  if (c16 == u'm')
    return u"minute";
  if (c16 == u's')
    return u"second";
  if (c16 == u'S')
    return u"fractionalSecond";
  return u"literal";
}

// Implementer note: This method corresponds roughly to
// https://402.ecma-international.org/8.0/#sec-formatdatetimetoparts
std::vector<Part> DateTimeFormat::formatToParts(double x) noexcept {
  // NOTE: We dont have access to localeData.patterns. Instead we use
  // NSDateFormatter's foramt string, and break it into components.
  // 1. Let parts be ? PartitionDateTimePattern(dateTimeFormat, x).
  auto fmt = nsStringToU16String(impl_->getNSDateFormatter().dateFormat);
  std::unique(fmt.begin(), fmt.end());
  auto formattedDate = format(x);
  // 2. Let result be ArrayCreate(0).
  std::vector<Part> result;
  // 3. Let n be 0.
  // 4. For each Record { [[Type]], [[Value]] } part in parts, do
  // a. Let O be OrdinaryObjectCreate(%Object.prototype%).
  // b. Perform ! CreateDataPropertyOrThrow(O, "type", part.[[Type]]).
  // c. Perform ! CreateDataPropertyOrThrow(O, "value", part.[[Value]]).
  // d. Perform ! CreateDataProperty(result, ! ToString(n), O).
  // e. Increment n by 1.
  std::u16string currentPart;
  unsigned n = 0;
  static auto alphanumerics = NSCharacterSet.alphanumericCharacterSet;
  for (char16_t c16 : formattedDate) {
    if ([alphanumerics characterIsMember:c16]) {
      currentPart += c16;
      continue;
    }
    if (currentPart != u"") {
      result.push_back(
          {{u"type", returnTypeOfDate(fmt[n])}, {u"value", currentPart}});
      currentPart = u"";
      n++;
    }
    result.push_back({{u"type", u"literal"}, {u"value", {c16}}});
    n++;
  }
  // Last format string component.
  result.push_back(
      {{u"type", returnTypeOfDate(fmt[n])}, {u"value", currentPart}});
  // 5. Return result.
  return result;
}

struct NumberFormat::Impl {
  // https://402.ecma-international.org/8.0/#sec-properties-of-intl-numberformat-instances
  // Intl.NumberFormat instances have an [[InitializedNumberFormat]] internal
  // slot.
  // NOTE: InitializedNumberFormat is not implemented.
  // [[Locale]] is a String value with the language tag of the locale whose
  // localization is used for formatting.
  std::u16string locale;
  // [[DataLocale]] is a String value with the language tag of the nearest
  // locale for which the implementation has data to perform the formatting
  // operation. It will be a parent locale of [[Locale]].
  std::u16string dataLocale;
  // [[NumberingSystem]] is a String value with the "type" given in Unicode
  // Technical Standard 35 for the numbering system used for formatting.
  // NOTE: Even though NSNumberFormatter formats numbers and time using
  // different numbering systems based on its "locale" value, it does not allow
  // to set/get the numbering system value explicitly. So we consider this
  // feature unsupported.
  // [[Style]] is one of the String values "decimal", "currency", "percent", or
  // "unit", identifying the type of quantity being measured.
  std::u16string style;
  // [[Currency]] is a String value with the currency code identifying the
  // currency to be used if formatting with the "currency" unit type. It is only
  // used when [[Style]] has the value "currency".
  std::optional<std::u16string> currency;
  // [[CurrencyDisplay]] is one of the String values "code", "symbol",
  // "narrowSymbol", or "name", specifying whether to display the currency as an
  // ISO 4217 alphabetic currency code, a localized currency symbol, or a
  // localized currency name if formatting with the "currency" style. It is only
  // used when [[Style]] has the value "currency".
  std::optional<std::u16string> currencyDisplay;
  // [[CurrencySign]] is one of the String values "standard" or "accounting",
  // specifying whether to render negative numbers in accounting format, often
  // signified by parenthesis. It is only used when [[Style]] has the value
  // "currency" and when [[SignDisplay]] is not "never".
  std::optional<std::u16string> currencySign;
  // [[Unit]] is a core unit identifier, as defined by Unicode Technical
  // Standard #35, Part 2, Section 6. It is only used when [[Style]] has the
  // value "unit".
  std::optional<std::u16string> unit;
  // [[UnitDisplay]] is one of the String values "short", "narrow", or "long",
  // specifying whether to display the unit as a symbol, narrow symbol, or
  // localized long name if formatting with the "unit" style. It is only used
  // when [[Style]] has the value "unit".
  std::optional<std::u16string> unitDisplay;
  // [[MinimumIntegerDigits]] is a non-negative integer Number value indicating
  // the minimum integer digits to be used. Numbers will be padded with leading
  // zeroes if necessary.
  uint8_t minimumIntegerDigits;

  struct NumDigits {
    uint8_t minimum;
    uint8_t maximum;
  };
  // [[MinimumFractionDigits]] and [[MaximumFractionDigits]] are non-negative
  // integer Number values indicating the minimum and maximum fraction digits to
  // be used. Numbers will be rounded or padded with trailing zeroes if
  // necessary. These properties are only used when [[RoundingType]] is
  // fractionDigits.
  std::optional<NumDigits> fractionDigits;
  // [[MinimumSignificantDigits]] and [[MaximumSignificantDigits]] are positive
  // integer Number values indicating the minimum and maximum fraction digits to
  // be shown. If present, the formatter uses however many fraction digits are
  // required to display the specified number of significant digits. These
  // properties are only used when [[RoundingType]] is significantDigits.
  std::optional<NumDigits> significantDigits;
  // [[UseGrouping]] is a Boolean value indicating whether a grouping separator
  // should be used.
  bool useGrouping;
  // [[RoundingType]] is one of the values fractionDigits, significantDigits, or
  // compactRounding, indicating which rounding strategy to use. If
  // fractionDigits, the number is rounded according to
  // [[MinimumFractionDigits]] and [[MaximumFractionDigits]], as described
  // above. If significantDigits, the number is rounded according to
  // [[MinimumSignificantDigits]] and [[MaximumSignificantDigits]] as described
  // above. If compactRounding, the number is rounded to 1 maximum fraction
  // digit if there is 1 digit before the decimal separator, and otherwise round
  // to 0 fraction digits.
  std::u16string roundingType;
  // [[Notation]] is one of the String values "standard", "scientific",
  // "engineering", or "compact", specifying whether the number should be
  // displayed without scaling, scaled to the units place with the power of ten
  // in scientific notation, scaled to the nearest thousand with the power of
  // ten in scientific notation, or scaled to the nearest locale-dependent
  // compact decimal notation power of ten with the corresponding compact
  // decimal notation affix.
  std::u16string notation;
  // [[CompactDisplay]] is one of the String values "short" or "long",
  // specifying whether to display compact notation affixes in short form ("5K")
  // or long form ("5 thousand") if formatting with the "compact" notation. It
  // is only used when [[Notation]] has the value "compact".
  std::optional<std::u16string> compactDisplay;
  // [[SignDisplay]] is one of the String values "auto", "always", "never", or
  // "exceptZero", specifying whether to show the sign on negative numbers only,
  // positive and negative numbers including zero, neither positive nor negative
  // numbers, or positive and negative numbers but not zero. In scientific
  // notation, this slot affects the sign display of the mantissa but not the
  // exponent.
  std::u16string signDisplay;
  // Finally, Intl.NumberFormat instances have a [[BoundFormat]] internal slot
  // that caches the function returned by the format accessor (15.4.3).
  // NOTE: BoundFormat is not implemented.
  vm::ExecutionStatus setNumberFormatUnitOptions(
      vm::Runtime &runtime,
      const Options &options) noexcept;
  vm::ExecutionStatus setNumberFormatDigitOptions(
      vm::Runtime &runtime,
      const Options &options,
      uint8_t mnfdDefault,
      uint8_t mxfdDefault,
      std::u16string_view notation) noexcept;
  std::u16string format(double number) noexcept;
};

NumberFormat::NumberFormat() : impl_(std::make_unique<Impl>()) {}
NumberFormat::~NumberFormat() {}

// https://402.ecma-international.org/8.0/#sec-intl.numberformat.supportedlocalesof
vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let availableLocales be %DateTimeFormat%.[[AvailableLocales]].
  auto availableLocales = getAvailableLocales();
  // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  auto requestedLocales = getCanonicalLocales(runtime, locales);
  // 3. Return ? (availableLocales, requestedLocales, options).
  return supportedLocales(availableLocales, requestedLocales.getValue());
}

// https://402.ecma-international.org/8.0/#sec-setnumberformatunitoptions
vm::ExecutionStatus NumberFormat::Impl::setNumberFormatUnitOptions(
    vm::Runtime &runtime,
    const Options &options) noexcept {
  //  1. Assert: Type(intlObj) is Object.
  //  2. Assert: Type(options) is Object.
  //  3. Let style be ? GetOption(options, "style", "string", « "decimal",
  //  "percent", "currency", "unit" », "decimal").
  static constexpr std::u16string_view styleValues[] = {
      u"decimal", u"percent", u"currency", u"unit"};
  auto styleRes =
      getOptionString(runtime, options, u"style", styleValues, u"decimal");
  if (LLVM_UNLIKELY(styleRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto styleOpt = *styleRes;
  //  4. Set intlObj.[[Style]] to style.
  style = *styleOpt;
  //  5. Let currency be ? GetOption(options, "currency", "string", undefined,
  //  undefined).
  auto currencyRes = getOptionString(runtime, options, u"currency", {}, {});
  if (LLVM_UNLIKELY(currencyRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto currencyOpt = *currencyRes;
  //  6. If currency is undefined, then
  if (!currencyOpt) {
    //  a. If style is "currency", throw a TypeError exception.
    if (style == u"currency")
      return runtime.raiseTypeError("Currency is undefined");
    //  7. Else,
  } else {
    //  a. If the result of IsWellFormedCurrencyCode(currency) is false, throw
    //  a RangeError exception.
    if (!isWellFormedCurrencyCode(*currencyOpt))
      return runtime.raiseRangeError("Currency is invalid");
  }
  //  8. Let currencyDisplay be ? GetOption(options, "currencyDisplay",
  //  "string", « "code", "symbol", "narrowSymbol", "name" », "symbol").
  static constexpr std::u16string_view currencyDisplayValues[] = {
      u"code", u"symbol", u"narrowSymbol", u"name"};
  auto currencyDisplayRes = getOptionString(
      runtime, options, u"currencyDisplay", currencyDisplayValues, u"symbol");
  if (LLVM_UNLIKELY(currencyDisplayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto currencyDisplayOpt = *currencyDisplayRes;
  //  9. Let currencySign be ? GetOption(options, "currencySign", "string", «
  //  "standard", "accounting" », "standard").
  static constexpr std::u16string_view currencySignValues[] = {
      u"standard", u"accounting"};
  auto currencySignRes = getOptionString(
      runtime, options, u"currencySign", currencySignValues, u"standard");
  if (LLVM_UNLIKELY(currencySignRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto currencySignOpt = *currencySignRes;
  //  10. Let unit be ? GetOption(options, "unit", "string", undefined,
  //  undefined).
  auto unitRes = getOptionString(runtime, options, u"unit", {}, {});
  if (LLVM_UNLIKELY(unitRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto unitOpt = *unitRes;
  //  11. If unit is undefined, then
  if (!unitOpt) {
    //  a. If style is "unit", throw a TypeError exception.
    if (style == u"unit")
      return runtime.raiseTypeError("Unit is undefined");
    //  12. Else,
  } else {
    //  a. If the result of IsWellFormedUnitIdentifier(unit) is false, throw a
    //  RangeError exception.
    if (!isWellFormedUnitIdentifier(*unitOpt))
      return runtime.raiseRangeError("Unit is invalid");
  }
  //  13. Let unitDisplay be ? GetOption(options, "unitDisplay", "string", «
  //  "short", "narrow", "long" », "short").
  static constexpr std::u16string_view unitDisplayValues[] = {
      u"short", u"narrow", u"long"};
  auto unitDisplayRes = getOptionString(
      runtime, options, u"unitDisplay", unitDisplayValues, u"short");
  if (LLVM_UNLIKELY(unitDisplayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto unitDisplayOpt = *unitDisplayRes;
  //  14. If style is "currency", then
  if (style == u"currency") {
    //  a. Let currency be the result of converting currency to upper case as
    //  specified in 6.1.
    //  b. Set intlObj.[[Currency]] to currency.
    currency = toASCIIUppercase(*currencyOpt);
    //  c. Set intlObj.[[CurrencyDisplay]] to currencyDisplay.
    currencyDisplay = *currencyDisplayOpt;
    //  d. Set intlObj.[[CurrencySign]] to currencySign.
    currencySign = *currencySignOpt;
  }
  //  15. If style is "unit", then
  if (style == u"unit") {
    //  a. Set intlObj.[[Unit]] to unit.
    //  b. Set intlObj.[[UnitDisplay]] to unitDisplay.
    unit = *unitOpt;
    unitDisplay = *unitDisplayOpt;
  }
  return vm::ExecutionStatus::RETURNED;
}

// https://402.ecma-international.org/8.0/#sec-setnfdigitoptions
vm::ExecutionStatus NumberFormat::Impl::setNumberFormatDigitOptions(
    vm::Runtime &runtime,
    const Options &options,
    uint8_t mnfdDefault,
    uint8_t mxfdDefault,
    std::u16string_view notation) noexcept {
  // 1. Assert: Type(intlObj) is Object.
  // 2. Assert: Type(options) is Object.
  // 3. Assert: Type(mnfdDefault) is Number.
  // 4. Assert: Type(mxfdDefault) is Number.
  // 5. Let mnid be ? GetNumberOption(options, "minimumIntegerDigits,", 1, 21,
  // 1).
  auto mnidRes =
      getNumberOption(runtime, options, u"minimumIntegerDigits", 1, 21, 1);
  if (LLVM_UNLIKELY(mnidRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto mnidOpt = *mnidRes;
  // 6. Let mnfd be ? Get(options, "minimumFractionDigits").
  auto mnfdIt = options.find(u"minimumFractionDigits");
  // 7. Let mxfd be ? Get(options, "maximumFractionDigits").
  auto mxfdIt = options.find(u"maximumFractionDigits");
  // 8. Let mnsd be ? Get(options, "minimumSignificantDigits").
  auto mnsdIt = options.find(u"minimumSignificantDigits");
  // 9. Let mxsd be ? Get(options, "maximumSignificantDigits").
  auto mxsdIt = options.find(u"maximumSignificantDigits");
  // 10. Set intlObj.[[MinimumIntegerDigits]] to mnid.
  minimumIntegerDigits = *mnidOpt;
  // 11. If mnsd is not undefined or mxsd is not undefined, then
  if (mnsdIt != options.end() || mxsdIt != options.end()) {
    // a. Set intlObj.[[RoundingType]] to significantDigits.
    roundingType = u"significantDigits";
    // b. Let mnsd be ? DefaultNumberOption(mnsd, 1, 21, 1).
    auto mnsdValue =
        mnsdIt == options.end() ? std::nullopt : std::optional(mnsdIt->second);
    auto mnsdRes = defaultNumberOption(
        runtime, u"minimumSignificantDigits", mnsdValue, 1, 21, 1);
    if (LLVM_UNLIKELY(mnsdRes == vm::ExecutionStatus::EXCEPTION))
      return vm::ExecutionStatus::EXCEPTION;
    auto mnsdOpt = *mnsdRes;
    // c. Let mxsd be ? DefaultNumberOption(mxsd, mnsd, 21, 21).
    auto mxsdValue =
        mxsdIt == options.end() ? std::nullopt : std::optional(mxsdIt->second);
    auto mxsdRes = defaultNumberOption(
        runtime, u"maximumSignificantDigits", mxsdValue, *mnsdOpt, 21, 21);
    if (LLVM_UNLIKELY(mxsdRes == vm::ExecutionStatus::EXCEPTION))
      return vm::ExecutionStatus::EXCEPTION;
    auto mxsdOpt = *mxsdRes;
    // d. Set intlObj.[[MinimumSignificantDigits]] to mnsd.
    // e. Set intlObj.[[MaximumSignificantDigits]] to mxsd.
    significantDigits = {*mnsdOpt, *mxsdOpt};
    // 12. Else if mnfd is not undefined or mxfd is not undefined, then
  } else if (mnfdIt != options.end() || mxfdIt != options.end()) {
    // a. Set intlObj.[[RoundingType]] to fractionDigits.
    roundingType = u"fractionDigits";
    // b. Let mnfd be ? DefaultNumberOption(mnfd, 0, 20, undefined).
    auto mnfdValue =
        mnfdIt == options.end() ? std::nullopt : std::optional(mnfdIt->second);
    auto mnfdRes = defaultNumberOption(
        runtime, u"minimumFractionDigits", mnfdValue, 0, 20, {});
    if (LLVM_UNLIKELY(mnfdRes == vm::ExecutionStatus::EXCEPTION))
      return vm::ExecutionStatus::EXCEPTION;
    auto mnfdOpt = *mnfdRes;
    // c. Let mxfd be ? DefaultNumberOption(mxfd, 0, 20, undefined).
    auto mxfdValue =
        mxfdIt == options.end() ? std::nullopt : std::optional(mxfdIt->second);
    auto mxfdRes = defaultNumberOption(
        runtime, u"maximumFractionDigits", mxfdValue, 0, 20, {});
    if (LLVM_UNLIKELY(mxfdRes == vm::ExecutionStatus::EXCEPTION))
      return vm::ExecutionStatus::EXCEPTION;
    auto mxfdOpt = *mxfdRes;
    // d. If mnfd is undefined, set mnfd to min(mnfdDefault, mxfd).
    if (!mnfdOpt) {
      mnfdOpt = std::min(mnfdDefault, *mxfdOpt);
      // e. Else if mxfd is undefined, set mxfd to max(mxfdDefault, mnfd).
    } else if (!mxfdOpt) {
      mxfdOpt = std::max(mxfdDefault, *mnfdOpt);
      // f. Else if mnfd is greater than mxfd, throw a RangeError exception.
    } else if (*mnfdOpt > *mxfdOpt) {
      return runtime.raiseRangeError(
          "minimumFractionDigits is greater than maximumFractionDigits");
    }
    // g. Set intlObj.[[MinimumFractionDigits]] to mnfd.
    // h. Set intlObj.[[MaximumFractionDigits]] to mxfd.
    fractionDigits = {*mnfdOpt, *mxfdOpt};
    // 13. Else if notation is "compact", then
  } else if (notation == u"compact") {
    // a. Set intlObj.[[RoundingType]] to compactRounding.
    roundingType = u"compactRounding";
    // 14. Else,
  } else {
    // a. Set intlObj.[[RoundingType]] to fractionDigits.
    roundingType = u"fractionDigits";
    // b. Set intlObj.[[MinimumFractionDigits]] to mnfdDefault.
    // c. Set intlObj.[[MaximumFractionDigits]] to mxfdDefault.
    fractionDigits = {mnfdDefault, mxfdDefault};
  }
  return vm::ExecutionStatus::RETURNED;
}

// https://402.ecma-international.org/8.0/#sec-initializenumberformat
vm::ExecutionStatus NumberFormat::initialize(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  const auto requestedLocales = canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 2. Set options to ? CoerceOptionsToObject(options).
  // 3. Let opt be a new Record.
  std::unordered_map<std::u16string, std::u16string> opt;
  // Create a copy of the unordered map &options
  // 4. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  // "lookup", "best fit" », "best fit").
  static constexpr std::u16string_view localeMatcherValues[] = {
      u"lookup", u"best fit"};
  auto matcherRes = getOptionString(
      runtime, options, u"localeMatcher", localeMatcherValues, u"best fit");
  if (LLVM_UNLIKELY(matcherRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto matcherOpt = *matcherRes;
  // 5. Set opt.[[localeMatcher]] to matcher.
  opt.emplace(u"localeMatcher", *matcherOpt);
  // 6. Let numberingSystem be ? GetOption(options, "numberingSystem", "string",
  // undefined, undefined).
  auto numberingSystemRes =
      getOptionString(runtime, options, u"numberingSystem", {}, {});
  if (LLVM_UNLIKELY(numberingSystemRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto numberingSystemOpt = *numberingSystemRes;
  // 7. If numberingSystem is not undefined, then
  //   a. If numberingSystem does not match the Unicode Locale Identifier type
  //   nonterminal, throw a RangeError exception.
  // 8. Set opt.[[nu]] to numberingSystem.
  if (numberingSystemOpt) {
    auto numberingSystem = *numberingSystemOpt;
    if (!isUnicodeExtensionType(numberingSystem)) {
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid numbering system: ") +
          vm::TwineChar16(numberingSystem.c_str()));
    }
    opt.emplace(u"nu", numberingSystem);
  }
  // 9. Let localeData be %NumberFormat%.[[LocaleData]].
  // 10. Let r be ResolveLocale(%NumberFormat%.[[AvailableLocales]],
  // requestedLocales, opt, %NumberFormat%.[[RelevantExtensionKeys]],
  // localeData).
  static constexpr std::u16string_view relevantExtensionKeys[] = {u"nu"};
  auto r = resolveLocale(
      getAvailableLocales(), *requestedLocales, opt, relevantExtensionKeys);
  // 11. Set numberFormat.[[Locale]] to r.[[locale]].
  impl_->locale = r.locale;
  // 12. Set numberFormat.[[DataLocale]] to r.[[dataLocale]].
  impl_->dataLocale = r.dataLocale;
  // 13. Set numberFormat.[[NumberingSystem]] to r.[[nu]].
  // 14. Perform ? SetNumberFormatUnitOptions(numberFormat, options).
  auto setUnitOptionsRes = impl_->setNumberFormatUnitOptions(runtime, options);
  if (LLVM_UNLIKELY(setUnitOptionsRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 15. Let style be numberFormat.[[Style]].
  auto style = impl_->style;
  uint8_t mnfdDefault, mxfdDefault;
  // 16. If style is "currency", then
  if (style == u"currency") {
    // a. Let currency be numberFormat.[[Currency]].
    // b. Let cDigits be CurrencyDigits(currency).
    auto cDigits = getCurrencyDigits(*impl_->currency);
    // c. Let mnfdDefault be cDigits.
    mnfdDefault = cDigits;
    // d. Let mxfdDefault be cDigits.
    mxfdDefault = cDigits;
  } else {
    // 17. Else,
    // a. Let mnfdDefault be 0.
    mnfdDefault = 0;
    // b. If style is "percent", then
    if (style == u"percent") {
      // i. Let mxfdDefault be 0.
      mxfdDefault = 0;
    } else {
      // c. Else,
      // i. Let mxfdDefault be 3.
      mxfdDefault = 3;
    }
  }
  // 18. Let notation be ? GetOption(options, "notation", "string", «
  // "standard", "scientific", "engineering", "compact" », "standard").
  static constexpr std::u16string_view notationValues[] = {
      u"standard", u"scientific", u"engineering", u"compact"};
  auto notationRes = getOptionString(
      runtime, options, u"notation", notationValues, u"standard");
  if (LLVM_UNLIKELY(notationRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto notationOpt = *notationRes;
  // 19. Set numberFormat.[[Notation]] to notation.
  impl_->notation = *notationOpt;
  // Perform ? SetNumberFormatDigitOptions(numberFormat, options, mnfdDefault,
  // mxfdDefault, notation).
  auto setDigitOptionsRes = impl_->setNumberFormatDigitOptions(
      runtime, options, mnfdDefault, mxfdDefault, impl_->notation);
  if (LLVM_UNLIKELY(setDigitOptionsRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 21. Let compactDisplay be ? GetOption(options, "compactDisplay", "string",
  // « "short", "long" », "short").
  static constexpr std::u16string_view compactDisplayValues[] = {
      u"short", u"long"};
  auto compactDisplayRes = getOptionString(
      runtime, options, u"compactDisplay", compactDisplayValues, u"short");
  if (LLVM_UNLIKELY(compactDisplayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto compactDisplayOpt = *compactDisplayRes;
  // 22. If notation is "compact", then
  if (*notationOpt == u"compact")
    //   a. Set numberFormat.[[CompactDisplay]] to compactDisplay.
    impl_->compactDisplay = *compactDisplayOpt;
  // 23. Let useGrouping be ? GetOption(options, "useGrouping", "boolean",
  // undefined, true).
  auto useGroupingOpt = getOptionBool(runtime, options, u"useGrouping", true);
  // 24. Set numberFormat.[[UseGrouping]] to useGrouping.
  impl_->useGrouping = *useGroupingOpt;
  // 25. Let signDisplay be ? GetOption(options, "signDisplay", "string", «
  // "auto", "never", "always", "exceptZero" », "auto").
  static constexpr std::u16string_view signDisplayValues[] = {
      u"auto", u"never", u"always", u"exceptZero"};
  auto signDisplayRes = getOptionString(
      runtime, options, u"signDisplay", signDisplayValues, u"auto");
  if (LLVM_UNLIKELY(signDisplayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto signDisplayOpt = *signDisplayRes;
  // 26. Set numberFormat.[[SignDisplay]] to signDisplay.
  impl_->signDisplay = *signDisplayOpt;
  return vm::ExecutionStatus::RETURNED;
}

// https://402.ecma-international.org/8.0/#sec-intl.numberformat.prototype.resolvedoptions
Options NumberFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", impl_->locale);
  options.emplace(u"style", impl_->style);
  if (impl_->currency)
    options.emplace(u"currency", *impl_->currency);
  if (impl_->currencyDisplay)
    options.emplace(u"currencyDisplay", *impl_->currencyDisplay);
  if (impl_->currencySign)
    options.emplace(u"currencySign", *impl_->currencySign);
  if (impl_->unit)
    options.emplace(u"unit", *impl_->unit);
  if (impl_->unitDisplay)
    options.emplace(u"unitDisplay", *impl_->unitDisplay);
  options.emplace(u"minimumIntegerDigits", (double)impl_->minimumIntegerDigits);
  if (impl_->fractionDigits) {
    options.emplace(
        u"minimumFractionDigits", (double)impl_->fractionDigits->minimum);
    options.emplace(
        u"maximumFractionDigits", (double)impl_->fractionDigits->maximum);
  }
  if (impl_->significantDigits) {
    options.emplace(
        u"minimumSignificantDigits", (double)impl_->significantDigits->minimum);
    options.emplace(
        u"maximumSignificantDigits", (double)impl_->significantDigits->maximum);
  }
  options.emplace(u"useGrouping", impl_->useGrouping);
  options.emplace(u"roundingType", impl_->roundingType);
  options.emplace(u"notation", impl_->notation);
  if (impl_->compactDisplay)
    options.emplace(u"compactDisplay", *impl_->compactDisplay);
  options.emplace(u"signDisplay", impl_->signDisplay);
  return options;
}

// https://402.ecma-international.org/8.0/#sec-formatnumber
std::u16string NumberFormat::Impl::format(double number) noexcept {
  // NOTE: NSNumberFormatter has following limitations:
  // - "scientific" notation is supprted, "engineering" and "compact" are not.
  // - roundingType is not supported.
  // - compactDisplay is not supported.
  // - signDisplay is not supported.
  // - NSNumberFormatter has maximumIntegerDigits, which is 42 by default
  auto nsLocale =
      [NSLocale localeWithLocaleIdentifier:u16StringToNSString(dataLocale)];
  auto nf = [NSNumberFormatter new];
  nf.locale = nsLocale;
  if (style == u"decimal") {
    nf.numberStyle = NSNumberFormatterDecimalStyle;
    if (notation == u"scientific") {
      nf.numberStyle = NSNumberFormatterScientificStyle;
    }
  } else if (style == u"currency") {
    nf.numberStyle = NSNumberFormatterCurrencyStyle;
    nf.currencyCode = u16StringToNSString(*currency);
    if (currencyDisplay == u"code") {
      nf.numberStyle = NSNumberFormatterCurrencyISOCodeStyle;
    } else if (currencyDisplay == u"symbol") {
      nf.numberStyle = NSNumberFormatterCurrencyStyle;
    } else if (currencyDisplay == u"narrowSymbol") {
      nf.numberStyle = NSNumberFormatterCurrencyStyle;
    } else if (currencyDisplay == u"name") {
      nf.numberStyle = NSNumberFormatterCurrencyPluralStyle;
    }
    if (signDisplay != u"never" && currencySign == u"accounting") {
      nf.numberStyle = NSNumberFormatterCurrencyAccountingStyle;
    }
  } else if (style == u"percent") {
    nf.numberStyle = NSNumberFormatterPercentStyle;
  } else if (style == u"unit") {
    nf.numberStyle = NSNumberFormatterNoStyle;
  }
  nf.minimumIntegerDigits = minimumIntegerDigits;
  if (fractionDigits) {
    nf.minimumFractionDigits = fractionDigits->minimum;
    nf.maximumFractionDigits = fractionDigits->maximum;
  }
  if (significantDigits) {
    nf.minimumSignificantDigits = significantDigits->minimum;
    nf.maximumSignificantDigits = significantDigits->maximum;
  }
  nf.usesGroupingSeparator = useGrouping;
  if (style == u"unit") {
    auto mf = [NSMeasurementFormatter new];
    mf.numberFormatter = nf;
    mf.locale = nsLocale;
    if (unitDisplay == u"short") {
      mf.unitStyle = NSFormattingUnitStyleShort;
    } else if (unitDisplay == u"narrow") {
      mf.unitStyle = NSFormattingUnitStyleMedium;
    } else if (unitDisplay == u"long") {
      mf.unitStyle = NSFormattingUnitStyleLong;
    }
    auto u = [[NSUnit alloc] initWithSymbol:u16StringToNSString(*unit)];
    auto m = [[NSMeasurement alloc] initWithDoubleValue:number unit:u];
    return nsStringToU16String([mf stringFromMeasurement:m]);
  }
  return nsStringToU16String(
      [nf stringFromNumber:[NSNumber numberWithDouble:number]]);
}

std::u16string NumberFormat::format(double number) noexcept {
  return impl_->format(number);
}

std::vector<std::unordered_map<std::u16string, std::u16string>>
NumberFormat::formatToParts(double number) noexcept {
  llvm_unreachable("formatToParts is unimplemented on Apple platforms");
}

} // namespace platform_intl
} // namespace hermes
