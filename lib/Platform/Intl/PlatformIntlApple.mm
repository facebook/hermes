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
  options.emplace(u"numeric", Option(false));
  options.emplace(u"timeZone", Option(impl_->timeZone));
  if (impl_->calendar)
    options.emplace(u"calendar", Option(*impl_->calendar));

  if (impl_->hourCycle.has_value()) {
    options.emplace(u"hourCycle", *impl_->hourCycle);
    if (impl_->hourCycle == u"h11" || impl_->hourCycle == u"h12") {
      options.emplace(u"hour12", true);
    } else {
      options.emplace(u"hour12", false);
    }
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
  std::u16string locale;
};

NumberFormat::NumberFormat() : impl_(std::make_unique<Impl>()) {}
NumberFormat::~NumberFormat() {}

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus NumberFormat::initialize(
    vm::Runtime &runtime,
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
