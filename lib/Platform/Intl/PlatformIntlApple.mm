/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/BCP47Parser.h"
#include "hermes/Platform/Intl/PlatformIntl.h"

#import <Foundation/Foundation.h>

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
    llvh::ArrayRef<std::u16string> relevantExtensionKeys) {
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
  for (const auto &key : relevantExtensionKeys) {
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
  // https://tc39.es/ecma402/#locale-objects As of now, 'locales' can only be a
  // string list/array. Validation occurs in normalizeLocaleList, so this
  // function just takes a vector of strings.
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
  if (defaultNumber == vm::ExecutionStatus::EXCEPTION) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return std::optional<uint8_t>(defaultNumber.getValue());
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
  auto usageRes = getOptionString(
      runtime, options, u"usage", {u"sort", u"search"}, {u"sort"});
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
  auto localeMatcherRes = getOptionString(
      runtime,
      options,
      u"localeMatcher",
      {u"lookup", u"best fit"},
      u"best fit");
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
  auto caseFirstRes = getOptionString(
      runtime, options, u"caseFirst", {u"upper", u"lower", u"false"}, {});
  if (LLVM_UNLIKELY(caseFirstRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 17. Set opt.[[kf]] to caseFirst.
  if (auto caseFirstOpt = *caseFirstRes)
    opt.emplace(u"kf", *caseFirstOpt);
  // 18. Let relevantExtensionKeys be %Collator%.[[RelevantExtensionKeys]].
  std::vector<std::u16string> relevantExtensionKeys = {u"co", u"kn", u"kf"};
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
  auto sensitivityRes = getOptionString(
      runtime,
      options,
      u"sensitivity",
      {u"base", u"accent", u"case", u"variant"},
      {});
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

struct DateTimeFormat::Impl {
  std::u16string locale;
};

DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}
DateTimeFormat::~DateTimeFormat() {}

vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime &runtime,
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
