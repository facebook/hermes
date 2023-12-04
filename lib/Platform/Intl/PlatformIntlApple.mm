/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/BCP47Parser.h"
#include "hermes/Platform/Intl/PlatformIntl.h"
#include "hermes/Platform/Intl/TzdbBackward.h"

#import <Foundation/Foundation.h>
#include <algorithm>
#include <array>
#include <shared_mutex>
#include <thread>
#include <unordered_set>

static_assert(__has_feature(objc_arc), "arc must be enabled");

namespace hermes {
namespace platform_intl {

// StringRecord contains a mapping of strings to strings or null (optional)
typedef std::unordered_map<std::u16string, std::optional<std::u16string>>
    StringRecord;

namespace {
NSString *u16StringToNSString(const std::u16string_view &src) {
  auto size = src.size();
  const auto *cString = (const unichar *)src.data();
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
      // Note that we do not need to handle legacy extensions here (unlike
      // getDefaultLocale), since the documentation for
      // availableLocaleIdentifiers states that they will only contain a
      // language, country, and script code.
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
    NSString *nsLocale = testLocale
        ? [NSString stringWithUTF8String:testLocale]
        : [[NSLocale currentLocale] localeIdentifier];
    auto defLocale = nsStringToU16String(nsLocale);

    // The locale identifier may occasionally contain legacy style locale
    // extensions. We cannot handle them so remove them before parsing the tag.
    size_t delimIdx = defLocale.find(u'@');
    if (delimIdx != std::u16string::npos)
      defLocale.resize(delimIdx);

    // See the comment in getAvailableLocales.
    std::replace(defLocale.begin(), defLocale.end(), u'_', u'-');
    if (auto parsed = ParsedLocaleIdentifier::parse(defLocale))
      return parsed->canonicalize();
    return std::u16string(u"und");
  }());
  return *defLocale;
}

// From
// https://github.com/eggert/tz/blob/a8761eebe979a56dfc234698e0e4f001c95329e7/etcetera
std::map<std::u16string, std::u16string> etc_timezones{
    // Zone    Etc/GMT-14    14    -    +14
    {u"Etc/GMT-14", u"GMT+14"},
    // Zone    Etc/GMT-13    13    -    +13
    {u"Etc/GMT-13", u"GMT+13"},
    // Zone    Etc/GMT-12    12    -    +12
    {u"Etc/GMT-12", u"GMT+12"},
    // Zone    Etc/GMT-11    11    -    +11
    {u"Etc/GMT-11", u"GMT+11"},
    // Zone    Etc/GMT-10    10    -    +10
    {u"Etc/GMT-10", u"UTC+10"},
    // Zone    Etc/GMT-9    9    -    +09
    {u"Etc/GMT-9", u"GMT+9"},
    // Zone    Etc/GMT-8    8    -    +08
    {u"Etc/GMT-8", u"GMT+8"},
    // Zone    Etc/GMT-7    7    -    +07
    {u"Etc/GMT-7", u"GMT+7"},
    // Zone    Etc/GMT-6    6    -    +06
    {u"Etc/GMT-6", u"GMT+6"},
    // Zone    Etc/GMT-5    5    -    +05
    {u"Etc/GMT-5", u"GMT+5"},
    // Zone    Etc/GMT-4    4    -    +04
    {u"Etc/GMT-4", u"GMT+4"},
    // Zone    Etc/GMT-3    3    -    +03
    {u"Etc/GMT-3", u"GMT+3"},
    // Zone    Etc/GMT-2    2    -    +02
    {u"Etc/GMT-2", u"GMT+2"},
    // Zone    Etc/GMT-1    1    -    +01
    {u"Etc/GMT-1", u"GMT+1"},
    // Zone    Etc/GMT+1    -1    -    -01
    {u"Etc/GMT+1", u"GMT-1"},
    // Zone    Etc/GMT+2    -2    -    -02
    {u"Etc/GMT+2", u"GMT-2"},
    // Zone    Etc/GMT+3    -3    -    -03
    {u"Etc/GMT+3", u"GMT-3"},
    // Zone    Etc/GMT+4    -4    -    -04
    {u"Etc/GMT+4", u"GMT-4"},
    // Zone    Etc/GMT+5    -5    -    -05
    {u"Etc/GMT+5", u"GMT-5"},
    // Zone    Etc/GMT+6    -6    -    -06
    {u"Etc/GMT+6", u"GMT-6"},
    // Zone    Etc/GMT+7    -7    -    -07
    {u"Etc/GMT+7", u"GMT-7"},
    // Zone    Etc/GMT+8    -8    -    -08
    {u"Etc/GMT+8", u"GMT-8"},
    // Zone    Etc/GMT+9    -9    -    -09
    {u"Etc/GMT+9", u"GMT-9"},
    // Zone    Etc/GMT+10    -10    -    -10
    {u"Etc/GMT+10", u"GMT-10"},
    // Zone    Etc/GMT+11    -11    -    -11
    {u"Etc/GMT+11", u"GMT-11"},
    // Zone    Etc/GMT+12    -12    -    -12
    {u"Etc/GMT+12", u"GMT-12"},
};

std::map<std::u16string, std::u16string> missing_iana_timezones{
    // Due to historical reasons, these timezones are treated differently.
    {u"Etc/UTC", u"UTC"},
    {u"Etc/GMT", u"UTC"},
    {u"GMT", u"UTC"},
    // These are missing from Apple's NSTimeZone implementation but expected
    // in test262 and standard IANA implementations
    {u"MET", u"Europe/Berlin"},
    {u"US/Pacific-New", u"America/Los_Angeles"},
    {u"EST5EDT", u"America/New_York"},
    {u"CST6CDT", u"America/Chicago"},
    {u"MST7MDT", u"America/Denver"},
    {u"PST8PDT", u"America/Los_Angeles"},
};

// These are link names which are verified to work with NSTimeZone
// but aren't listed by knownTimeZoneNames
std::vector<std::u16string> iana_links{u"Asia/Kolkata", u"Europe/Kyiv"};

static std::unordered_map<std::u16string_view, std::u16string>
    supportedCalendars{
        {u"buddhist", nsStringToU16String(NSCalendarIdentifierBuddhist)},
        {u"chinese", nsStringToU16String(NSCalendarIdentifierChinese)},
        {u"coptic", nsStringToU16String(NSCalendarIdentifierCoptic)},
        {u"ethiopic",
         nsStringToU16String(NSCalendarIdentifierEthiopicAmeteMihret)},
        {u"ethioaa",
         nsStringToU16String(NSCalendarIdentifierEthiopicAmeteAlem)},
        {u"gregory", nsStringToU16String(NSCalendarIdentifierGregorian)},
        {u"hebrew", nsStringToU16String(NSCalendarIdentifierHebrew)},
        {u"indian", nsStringToU16String(NSCalendarIdentifierIndian)},
        {u"islamic", nsStringToU16String(NSCalendarIdentifierIslamic)},
        {u"islamic-civil",
         nsStringToU16String(NSCalendarIdentifierIslamicCivil)},
        {u"islamic-tbla",
         nsStringToU16String(NSCalendarIdentifierIslamicTabular)},
        {u"islamic-umalqura",
         nsStringToU16String(NSCalendarIdentifierIslamicUmmAlQura)},
        {u"iso8601", nsStringToU16String(NSCalendarIdentifierISO8601)},
        {u"japanese", nsStringToU16String(NSCalendarIdentifierJapanese)},
        {u"persian", nsStringToU16String(NSCalendarIdentifierPersian)},
        {u"roc", nsStringToU16String(NSCalendarIdentifierRepublicOfChina)},
    };

static std::vector<std::u16string_view> keysOf(
    const std::unordered_map<std::u16string_view, std::u16string> map_) {
  std::vector<std::u16string_view> keys;
  for (const auto &[key, value] : map_) {
    keys.push_back(std::u16string_view(key));
  }
  return keys;
}

std::unordered_map<std::u16string, std::u16string_view> reverseMap(
    std::unordered_map<std::u16string_view, std::u16string> map) {
  static std::unordered_map<std::u16string, std::u16string_view> reversed;

  for (const auto &[key, value] : map) {
    reversed.emplace(std::u16string(value), key);
  }

  return reversed;
}

static std::vector<std::u16string_view> getSupportedCalendarIdentifiers() {
  static std::vector<std::u16string_view> supportedCalendarIdentifiers =
      keysOf(supportedCalendars);

  return supportedCalendarIdentifiers;
}

static std::u16string_view getNSCalendarIdentifier(std::u16string_view ca) {
  auto entry = supportedCalendars.find(ca);

  // Fallback to "gregorian" if no matching calendar is found...
  if (entry == supportedCalendars.end()) {
    return u"gregory";
  }

  return std::u16string_view(entry->second);
}

static std::optional<std::u16string_view> fromNSCalendarIdentifier(
    NSString *identifier) {
  static std::unordered_map<std::u16string, std::u16string_view>
      supportedCalendarsReverse = reverseMap(supportedCalendars);
  auto entry = supportedCalendarsReverse.find(nsStringToU16String(identifier));

  if (entry == supportedCalendarsReverse.end()) {
    return {};
  }

  return entry->second;
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

std::vector<std::optional<std::u16string_view>> getKeyLocaleDataCalendars(
    NSLocale *foundLocaleData) {
  auto keyLocaleData =
      fromNSCalendarIdentifier(foundLocaleData.calendarIdentifier);

  std::vector<std::optional<std::u16string_view>> calendarIdentifiers;

  if (keyLocaleData) {
    calendarIdentifiers.push_back(*keyLocaleData);
  }

  for (auto ca : getSupportedCalendarIdentifiers()) {
    // Avoid adding the same calendar twice...
    if (ca == keyLocaleData) {
      continue;
    }

    calendarIdentifiers.push_back(ca);
  }

  return calendarIdentifiers;
}

std::vector<std::optional<std::u16string_view>> getKeyLocaleData(
    NSLocale *foundLocaleData,
    std::u16string_view key) {
  static std::vector<std::optional<std::u16string_view>> hourCycleOptions(
      {std::nullopt, u"h11", u"h12", u"h23", u"h24"});
  static std::vector<std::optional<std::u16string_view>> numericOptions(
      {u"true", u"false"});
  static std::vector<std::optional<std::u16string_view>> caseFirstOptions(
      {u"false", u"upper", u"lower"});
  // nu is "" currently, this should be updated to include the list of
  // valid numbering systems in the future.
  static std::vector<std::optional<std::u16string_view>> numberingSystemOptions;
  static std::vector<std::optional<std::u16string_view>> emptyOptions;

  if (key == u"ca") {
    return getKeyLocaleDataCalendars(foundLocaleData);
  } else if (key == u"hc") {
    return hourCycleOptions;
  } else if (key == u"kn") {
    return numericOptions;
  } else if (key == u"kf") {
    return caseFirstOptions;
  } else if (key == u"nu") {
    return numberingSystemOptions;
  }

  return emptyOptions;
}

struct ResolvedLocale {
  std::u16string locale;
  std::u16string dataLocale;
  StringRecord extensions;
};

/// https://402.ecma-international.org/8.0/#sec-resolvelocale
ResolvedLocale resolveLocale(
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales,
    const StringRecord &options,
    llvh::ArrayRef<std::u16string_view> relevantExtensionKeys) {
  // 1. Let matcher be options.[[localeMatcher]].
  // 2. If matcher is "lookup", then
  //   a. Let r be ! LookupMatcher(availableLocales, requestedLocales).
  // 3. Else,
  //   a. Let r be ! BestFitMatcher(availableLocales, requestedLocales).
  auto r = lookupMatcher(availableLocales, requestedLocales);
  // 4. Let foundLocale be r.[[locale]].
  auto foundLocale = r.locale;
  // 5. Let result be a new Record.
  ResolvedLocale result;
  // 6. Set result.[[dataLocale]] to foundLocale.
  result.dataLocale = foundLocale;
  // 7. If r has an [[extension]] field, then
  //   a. Let components be ! UnicodeExtensionComponents(r.[[extension]]).
  //   b. Let keywords be components.[[Keywords]].
  // 8. Let supportedExtension be "-u".
  std::u16string supportedExtension = u"-u";
  // 9. For each element key of relevantExtensionKeys, do
  for (const auto &keyView : relevantExtensionKeys) {
    // TODO(T116352920): Make relevantExtensionKeys an
    // ArrayRef<std::u16string> and remove this temporary once we have
    // constexpr std::u16string.
    std::u16string key{keyView};
    // a. Let foundLocaleData be localeData.[[<foundLocale>]].
    auto *foundLocaleData = [[NSLocale alloc]
        initWithLocaleIdentifier:u16StringToNSString(foundLocale)];
    // b. Assert: Type(foundLocaleData) is Record.
    // c. Let keyLocaleData be foundLocaleData.[[<key>]].
    auto keyLocaleData = getKeyLocaleData(foundLocaleData, key);
    // d. Assert: Type(keyLocaleData) is List.
    // e. Let value be keyLocaleData[0].
    std::optional<std::u16string_view> value =
        keyLocaleData.empty() ? std::nullopt : keyLocaleData[0];
    // f. Assert: Type(value) is either String or Null.
    // g. Let supportedExtensionAddition be "".
    std::u16string supportedExtensionAddition = u"";
    // h. If r has an [[extension]] field, then
    if (!r.extensions.empty()) {
      // i. If keywords contains an element whose [[Key]] is the same as key,
      // then
      auto entry = r.extensions.find(key);
      if (entry != r.extensions.end()) {
        // 1. Let entry be the element of keywords whose [[Key]] is the same
        // as key.
        // 2. Let requestedValue be entry.[[Value]].
        auto requestedValue = entry->second;
        // 3. If requestedValue is not the empty String, then
        if (!requestedValue.empty()) {
          // a. If keyLocaleData contains requestedValue, then
          if (std::find(
                  keyLocaleData.begin(), keyLocaleData.end(), requestedValue) !=
              keyLocaleData.end()) {
            // i. Let value be requestedValue.
            value = requestedValue;
            // ii. Let supportedExtensionAddition be the string-concatenation
            // of
            // "-", key, "-", and value.
            supportedExtensionAddition =
                std::u16string(u"-").append(key).append(u"-").append(*value);
          }
          // 4. Else if keyLocaleData contains "true", then
        } else if (
            std::find(keyLocaleData.begin(), keyLocaleData.end(), u"true") !=
            keyLocaleData.end()) {
          // a. Let value be "true".
          value = u"true";
          // b. Let supportedExtensionAddition be the string-concatenation of
          // "-" and key.
          supportedExtensionAddition = std::u16string(u"-").append(key);
        }
      }
    }
    // i. If options has a field [[<key>]], then
    auto optIt = options.find(key);
    if (optIt != options.end()) {
      // i. Let optionsValue be options.[[<key>]].
      std::optional<std::u16string> optionsValue = optIt->second;
      // ii. Assert: Type(optionsValue) is either String, Undefined, or Null.
      // iii. If Type(optionsValue) is String, then
      if (optionsValue) {
        // 1. Let optionsValue be the string optionsValue after performing the
        // algorithm steps to transform Unicode extension values to canonical
        // syntax per Unicode Technical Standard #35 LDML § 3.2.1 Canonical
        // Unicode Locale Identifiers, treating key as ukey and optionsValue
        // as uvalue productions.
        // 2. Let optionsValue be the string optionsValue after performing the
        // algorithm steps to replace Unicode extension values with their
        // canonical form per Unicode Technical Standard #35 LDML § 3.2.1
        // Canonical Unicode Locale Identifiers, treating key as ukey and
        // optionsValue as uvalue productions.
        // 3. If optionsValue is the empty String, then
        if (optionsValue->empty()) {
          // a. Let optionsValue be "true".
          optionsValue = u"true";
        }
      }
      // iv. If SameValue(optionsValue, value) is false and keyLocaleData
      // contains optionsValue, then
      if (optionsValue != value &&
          std::find(keyLocaleData.begin(), keyLocaleData.end(), optionsValue) !=
              keyLocaleData.end()) {
        // 1. Let value be optionsValue.
        value = optionsValue;
        // 2. Let supportedExtensionAddition be "".
        supportedExtensionAddition = u"";
      }
    }
    // j. Set result.[[<key>]] to value.
    result.extensions.emplace(key, value);
    // k. Set supportedExtension to the string-concatenation of
    // supportedExtension and supportedExtensionAddition.
    supportedExtension = supportedExtension + supportedExtensionAddition;
  }
  // 10. If supportedExtension is not "-u", then
  if (supportedExtension != u"-u") {
    // a. Set foundLocale to
    // InsertUnicodeExtensionAndCanonicalize(foundLocale, supportedExtension).
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

// See https://www.rfc-editor.org/rfc/rfc5646.html#section-2.1
std::optional<std::u16string> canonicalizeLocaleId(
    const std::u16string &localeId) {
  // This is the list of "regular" locale IDs that are grandfathered
  static std::unordered_map<std::u16string, std::u16string> regular{
      {u"art-lojban", u"jbo"},
      {u"cel-gaulish", u"xtg"},

      {u"zh-guoyu", u"zh"},
      {u"zh-hakka", u"hak"},

      {u"zh-xiang", u"hsn"},
      {u"en-GB-oed", u"en-GB-oxendict"},
  };

  static std::unordered_map<std::u16string, std::u16string> regularNonUts35{
      {u"no-bok", u"nb"},
      {u"no-nyn", u"nn"},
      {u"zh-min", u"cdo"},
      {u"zh-min-nan", u"nan"},
  };

  // All IDs starting with i-
  auto isI = localeId[0] == u'i' && localeId[1] == u'-';
  // And all IDs starting with sgn- are grandfathered...
  auto isSgn = localeId[0] == u's' && localeId[1] == u'g' &&
      localeId[2] == u'n' && localeId[3] == u'-';

  if (isI || isSgn) {
    return {};
  }

  if (regularNonUts35.find(localeId) != regularNonUts35.end()) {
    return {};
  }

  auto it = regular.find(localeId);
  if (it != regular.end()) {
    return it->second;
  }
  return localeId;
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
  // 3. If Type(locales) is String or Type(locales) is Object and locales has
  // an
  // [[InitializedLocale]] internal slot, then
  // 4. Else
  // We don't yet support Locale object -
  // https://402.ecma-international.org/8.0/#locale-objects As of now,
  // 'locales' can only be a string list/array. Validation occurs in
  // normalizeLocaleList, so this function just takes a vector of strings.
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
    auto canonicalizedTag =
        canonicalizeLocaleId(parsedOpt->canonicalizeLocaleId());
    if (!canonicalizedTag) {
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid language tag: ") +
          vm::TwineChar16(locale.c_str()));
    }
    auto canonicalized =
        *canonicalizedTag + parsedOpt->canonicalizeExtensionString();

    // 7.c.vii. If canonicalizedTag is not an element of seen, append
    // canonicalizedTag as the last element of seen.
    if (std::find(seen.begin(), seen.end(), canonicalizedTag) == seen.end()) {
      seen.push_back(std::move(canonicalized));
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
  // 6. Let noExtensionsLocale be the String value that is requestedLocale
  // with any Unicode locale extension sequences (6.2.1) removed. We can skip
  // this step, see the comment in lookupMatcher.
  // 7. Let availableLocales be a List with language tags that includes the
  // languages for which the Unicode Character Database contains language
  // sensitive case mappings. Implementations may add additional language tags
  // if they support case mapping for additional locales.
  // 8. Let locale be BestAvailableLocale(availableLocales,
  // noExtensionsLocale). Convert to C++ array for bestAvailableLocale
  // function
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
  // 7. If values is not undefined and values does not contain an element
  // equal to value, throw a RangeError exception.
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

/// https://402.ecma-international.org/8.0/#sec-supportedlocales
vm::CallResult<std::vector<std::u16string>> supportedLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales,
    const Options &options) {
  // 1. Set options to ? CoerceOptionsToObject(options).
  // 2. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  //    "lookup", "best fit" », "best fit").
  static constexpr std::u16string_view localeMatcherOpts[] = {
      u"lookup", u"best fit"};
  auto localeMatcherRes = getOptionString(
      runtime, options, u"localeMatcher", localeMatcherOpts, u"best fit");
  if (LLVM_UNLIKELY(localeMatcherRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
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

// Implementation of
// https://402.ecma-international.org/8.0/#sec-todatetimeoptions
vm::CallResult<Options> toDateTimeOptions(
    vm::Runtime &runtime,
    Options options,
    std::u16string_view required,
    std::u16string_view defaults) {
  // 1. If options is undefined, let options be null; otherwise let options be
  // ? ToObject(options).
  // 2. Let options be OrdinaryObjectCreate(options).
  // 3. Let needDefaults be true.
  bool needDefaults = true;
  // 4. If required is "date" or "any", then
  if (required == u"date" || required == u"any") {
    // a. For each property name prop of « "weekday", "year", "month", "day"
    // », do
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

namespace {
/// Thread safe management of time zone names map.
class TimeZoneNames {
 public:
  /// Initializing the underlying map with all known time zone names in
  /// NSTimeZone.
  TimeZoneNames() {
    auto nsKnownTimeZoneNames = NSTimeZone.knownTimeZoneNames;
    for (NSString *timeZoneName in nsKnownTimeZoneNames) {
      auto canonical = nsStringToU16String(timeZoneName);
      if (canonical != u"Etc/UTC" && canonical != u"Etc/GMT" &&
          canonical != u"UTC") {
        timeZoneNamesVector_.emplace_back(canonical);
      }

      auto upper = toASCIIUppercase(canonical);
      timeZoneCanonicalNamesMap_.emplace(
          std::move(upper), std::move(canonical));
    }
    auto nsTimeZoneAbbreviations = NSTimeZone.abbreviationDictionary.allKeys;
    for (NSString *timeZoneName in nsTimeZoneAbbreviations) {
      auto canonical = nsStringToU16String(timeZoneName);
      auto upper = toASCIIUppercase(canonical);
      timeZoneCanonicalNamesMap_.emplace(
          std::move(upper), std::move(canonical));
    }

    // Etc/ timezones are canonically valid.
    for (auto const &entry : etc_timezones) {
      auto upper = toASCIIUppercase(entry.first);
      timeZoneNameOverridesMap_.emplace(upper, entry.second);
      timeZoneCanonicalNamesMap_.emplace(upper, entry.first);
    }

    for (auto const &entry : missing_iana_timezones) {
      auto upper = toASCIIUppercase(entry.first);

      if (entry.first != u"Etc/UTC" && entry.first != u"Etc/GMT" &&
          entry.first != u"UTC") {
        timeZoneNamesVector_.emplace_back(entry.first);
      }
      timeZoneNameOverridesMap_.emplace(upper, entry.second);
      timeZoneCanonicalNamesMap_.emplace(upper, entry.first);
    }

    for (auto entry : Tzdb::Backward::parse()) {
      auto upper = toASCIIUppercase(entry.first);
      timeZoneCanonicalNamesMap_.emplace(upper, entry.first);
      timeZoneNameLinkMap_.emplace(upper, entry.second);
    }

    for (auto const &link : iana_links) {
      auto upper = toASCIIUppercase(link);

      timeZoneCanonicalNamesMap_.emplace(std::move(upper), link);
    }

    std::sort(timeZoneNamesVector_.begin(), timeZoneNamesVector_.end());
  }

  /// Check if \p tz is a valid time zone name.
  bool contains(std::u16string_view tz) const {
    std::shared_lock lock(mutex_);
    auto upper = toASCIIUppercase(tz);
    return timeZoneCanonicalNamesMap_.find(upper) !=
        timeZoneCanonicalNamesMap_.end();
  }

  /// Get canonical time zone name for \p tz. Note that \p tz must
  /// be a valid key in the map.
  std::u16string getCanonical(std::u16string_view tz) const {
    std::shared_lock lock(mutex_);
    auto upper = toASCIIUppercase(tz);
    auto ianaTimeZoneIt = timeZoneCanonicalNamesMap_.find(upper);

    assert(
        ianaTimeZoneIt != timeZoneCanonicalNamesMap_.end() &&
        "getCanonical() must be called on valid time zone name.");
    return ianaTimeZoneIt->second;
  }

  std::u16string getNSName(std::u16string_view tz) const {
    std::shared_lock lock(mutex_);
    auto upper = toASCIIUppercase(tz);

    auto ianaTimeZoneOverrideIt = timeZoneNameOverridesMap_.find(upper);
    if (ianaTimeZoneOverrideIt != timeZoneNameOverridesMap_.end()) {
      return ianaTimeZoneOverrideIt->second;
    }

    auto ianaTimeZoneLinkIt = timeZoneNameLinkMap_.find(upper);
    if (ianaTimeZoneLinkIt != timeZoneNameLinkMap_.end()) {
      return ianaTimeZoneLinkIt->second;
    }

    auto ianaTimeZoneIt = timeZoneCanonicalNamesMap_.find(upper);

    assert(
        ianaTimeZoneIt != timeZoneCanonicalNamesMap_.end() &&
        "getNSTimeZone() must be called on valid time zone name.");
    return ianaTimeZoneIt->second;
  }

  /// Update the time zone name map with \p tz if it does not exist yet.
  void update(std::u16string_view tz) {
    auto upper = toASCIIUppercase(tz);
    // Read lock and check if tz is already in the map.
    {
      std::shared_lock lock(mutex_);
      if (timeZoneCanonicalNamesMap_.find(upper) !=
          timeZoneCanonicalNamesMap_.end()) {
        return;
      }
    }
    // If not, write lock and insert it into the map.
    {
      std::unique_lock lock(mutex_);
      timeZoneCanonicalNamesMap_.emplace(upper, tz);
      timeZoneNamesVector_.emplace_back(tz);
    }
  }

  std::vector<std::u16string_view> availableCanonicalTimeZones() {
    std::vector<std::u16string_view> keys;
    for (const auto &name : timeZoneNamesVector_) {
      if (name == u"")
        continue;

      auto found = std::find(keys.begin(), keys.end(), name) != keys.end();
      if (!found)
        keys.emplace_back(name);
    }
    return keys;
  }

 private:
  /// Map from upper case time zone name to canonical time zone name.
  std::unordered_map<std::u16string, std::u16string> timeZoneCanonicalNamesMap_;
  std::unordered_map<std::u16string, std::u16string> timeZoneNameOverridesMap_;
  std::unordered_map<std::u16string, std::u16string> timeZoneNameLinkMap_;
  std::vector<std::u16string> timeZoneNamesVector_;
  mutable std::shared_mutex mutex_;
};
} // namespace

static TimeZoneNames &validTimeZoneNames() {
  static TimeZoneNames validTimeZoneNames;
  return validTimeZoneNames;
}

// https://402.ecma-international.org/8.0/#sec-defaulttimezone
// There is a weird quirk on iOS regarding default time zones and
// NSTimeZone.knownTimeZoneNames. Unforunately, knownTimeZoneNames is not
// accurate. There is a chance the default timezone reported via
// NSTimeZone.defaultTimeZone is not present in knownTimeZoneNames. However,
// the NSTimeZone constructor can actually still understand this default TZ.
// Therefore, anytime the defaultTimeZone is requested we record it by
// manually inserting it into the timezones we consider valid. Note we have to
// do this every time the method is called, because there is a chance the
// default timezone can change while the program is running. In that case, we
// would want to accept both the old and new timezones.
std::u16string getDefaultTimeZone() {
  std::u16string tz = nsStringToU16String(NSTimeZone.defaultTimeZone.name);
  validTimeZoneNames().update(tz);
  return tz;
}

// https://402.ecma-international.org/8.0/#sec-getAvailableNamedTimeZoneIdentifier
std::u16string getAvailableNamedTimeZoneIdentifier(std::u16string_view tz) {
  auto ianaTimeZone = validTimeZoneNames().getCanonical(tz);

  return ianaTimeZone;
}

NSTimeZone *createTimeZone(std::u16string_view tz) {
  auto nsTimeZoneName = validTimeZoneNames().getNSName(tz);

  return [NSTimeZone timeZoneWithName:u16StringToNSString(nsTimeZoneName)];
}

// https://402.ecma-international.org/8.0/#sec-isvalidtimezonename
static bool isValidTimeZoneName(std::u16string_view tz) {
  return validTimeZoneNames().contains(tz);
}

struct TimeZoneOffsetParseResult {
  long long nanoseconds;
  std::u16string parsedComponents;
  bool hasSecondsComponent;
};

// 21.4.1.33.1 IsTimeZoneOffsetString ( offsetString )
// The abstract operation IsTimeZoneOffsetString takes argument offsetString
// (a String) and returns a Boolean. The return value indicates whether
// offsetString conforms to the grammar given by UTCOffset. It performs the
// following steps when called:
static std::optional<TimeZoneOffsetParseResult> getTimeZoneOffsetString(
    std::u16string_view offsetString) {
  // 1. Let parseResult be ParseText(StringToCodePoints(offsetString),
  // UTCOffset).
  // 2. If parseResult is a List of errors, return false.
  // 3. Return true.

  int length = offsetString.length();

  // Java supports "Z" and "+3" while JavaScript does not.
  // The shortest JavaScript zone offset is "+03" and the longest is "+22:00"
  // See
  // https://docs.oracle.com/javase/8/docs/api/java/time/ZoneOffset.html#of-java.lang.String-
  if (offsetString == u"Z" || length < 3 || length > 6) {
    return {};
  }

  // Normalize -00:00
  if (offsetString == u"-00" || offsetString == u"-0000" ||
      offsetString == u"-00:00" || offsetString == u"\u221200" ||
      offsetString == u"\u22120000" || offsetString == u"\u221200:00" ||
      offsetString == u"+00" || offsetString == u"+0000" ||
      offsetString == u"+00:00") {
    return TimeZoneOffsetParseResult{0, u"+00:00", false};
  }

  std::u16string parsedSign = u"";
  std::u16string parsedHour = u"";
  std::u16string parsedMinute = u"";
  std::u16string parsedSecond = u"";
  std::u16string parsedNanosecond = u"";

  int i = 0;
  int sign;
  int hours;
  int minutes;
  int seconds;
  long long nanoseconds;

  char16_t next = offsetString[i];

  if (next == u'+') {
    parsedSign += '+';
    sign = 1;
  } else if (next == u'-' || next == u'\u2212') {
    parsedSign += '-';
    sign = -1;
  } else {
    return {};
  }

  next = offsetString[++i];

  // + is invalid
  if (i == length) {
    return {};
  }

  bool isTwenty = next == u'2';
  if (next == u'0' || next == u'1' || isTwenty) {
    parsedHour += next;
  } else {
    return {};
  }

  next = offsetString[++i];

  // +2 is invalid
  if (i == length) {
    return {};
  }

  if (!isTwenty && next >= u'0' && next <= u'9') {
    parsedHour += next;
  } else if (isTwenty && next >= u'0' && next <= u'3') {
    parsedHour += next;
  } else {
    // +0A is invalid and +24 is invalid
    return {};
  }

  hours = u16StringToNSString(parsedHour).integerValue;

  next = offsetString[++i];
  if (i == length) {
    long long nanoseconds = sign * hours * 60 * 60 * 1e9;
    return TimeZoneOffsetParseResult{
        nanoseconds, parsedSign + parsedHour + u":00", false};
  }

  if (next == u':') {
    next = offsetString[++i];
  }

  // +12: is invalid
  if (i == length) {
    return {};
  }

  if (next >= u'0' && next <= u'5') {
    parsedMinute += next;
  } else {
    // +12:9 is invalid
    return {};
  }

  next = offsetString[++i];

  // +12:6 is invalid
  if (i == length) {
    return {};
  }

  if (next >= u'0' && next <= u'9') {
    parsedMinute += next;
  } else {
    // +12:5A is invalid
    return {};
  }

  minutes = u16StringToNSString(parsedMinute).integerValue;

  next = offsetString[++i];

  // +12:32
  if (i == length) {
    long long nanoseconds = sign * (((hours * 60 + minutes) * 60) * 1e9);
    return TimeZoneOffsetParseResult{
        nanoseconds, parsedSign + parsedHour + u":" + parsedMinute, false};
  }

  // +12532 or +12:532 is invalid
  if (next != u':') {
    return {};
  }

  if (next >= u'0' && next <= u'5') {
    parsedSecond += next;
  } else {
    // +12:54:9 is invalid
    return {};
  }

  next = offsetString[++i];

  // +12:54:6 is invalid
  if (i == length) {
    return {};
  }

  if (next >= u'0' && next <= u'9') {
    parsedSecond += next;
  } else {
    // +12:52:4A is invalid
    return {};
  }

  seconds = u16StringToNSString(parsedSecond).integerValue;
  next = offsetString[++i];

  // +12:32:10
  if (i == length) {
    long long nanoseconds =
        sign * (((hours * 60 + minutes) * 60 + seconds) * 1e9);
    std::u16string parsedComponents =
        parsedSign + parsedHour + u":" + parsedMinute + u":" + parsedSecond;
    return TimeZoneOffsetParseResult{nanoseconds, parsedComponents, true};
  }

  if (next != u'.' && next != u',') {
    // +12:32:10_ is invalid
    return {};
  }

  next = offsetString[++i];

  while (i < length) {
    if (next >= u'0' && next <= u'9') {
      parsedNanosecond += next;

      next = offsetString[++i];
      continue;
    }

    // +12:32:10.ABC is invalid
    return {};
  }

  std::u16string fraction = parsedNanosecond + u"000000000";
  std::u16string nanosecondsString = fraction.substr(1, 10);
  nanoseconds = u16StringToNSString(parsedSecond).longLongValue;

  // Return sign × (((hours × 60 + minutes) × 60 + seconds) × 10**9 +
  // nanoseconds).
  nanoseconds =
      sign * (((hours * 60 + minutes) * 60 + seconds) * 1e9 + nanoseconds);
  std::u16string parsedComponents = parsedSign + parsedHour + u":" +
      parsedMinute + u":" + parsedSecond + u"." + nanosecondsString;

  return TimeZoneOffsetParseResult{nanoseconds, parsedComponents, true};
}

// 22.1.3.17.3 ToZeroPaddedDecimalString ( n, minLength )
// The abstract operation ToZeroPaddedDecimalString takes arguments n (a
// non-negative integer) and minLength (a non-negative integer) and returns a
// String. It performs the following steps when called:
std::u16string toZeroPaddedDecimalString(int n, int minLength) {
  // 1. Let S be the String representation of n, formatted as a decimal
  // number.
  NSString *S = [@(n) stringValue];
  // 2. Return StringPad(S, minLength, "0", start).
  std::u16string s = nsStringToU16String(S);
  return s.insert(0, minLength - s.size(), u'0');
}

// The abstract operation FormatOffsetTimeZoneIdentifier takes argument
// offsetMinutes (an integer) and returns a String. It formats a UTC offset,
// in minutes, into a UTC offset string formatted like ±HH:MM. It performs the
// following steps when called:
std::u16string formatOffsetTimeZoneIdentifier(int offsetMinutes) {
  // 1. If offsetMinutes ≥ 0, let sign be the code unit 0x002B (PLUS SIGN);
  // otherwise, let sign be the code unit 0x002D (HYPHEN-MINUS).
  std::u16string sign;
  if (offsetMinutes >= 0) {
    sign = u"+";
  } else {
    sign = u"-";
  }
  // 2. Let absoluteMinutes be abs(offsetMinutes).
  auto absoluteMinutes = std::abs(offsetMinutes);
  // 3. Let hours be floor(absoluteMinutes / 60).
  auto hours = std::floor(absoluteMinutes / 60);
  // 4. Let minutes be absoluteMinutes modulo 60.
  auto minutes = absoluteMinutes % 60;
  // 5. Return the string-concatenation of sign,
  // ToZeroPaddedDecimalString(hours, 2), the code unit 0x003A (COLON), and
  // ToZeroPaddedDecimalString(minutes, 2).
  return u"" + sign + toZeroPaddedDecimalString(hours, 2) + u":" +
      toZeroPaddedDecimalString(minutes, 2);
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
    } else if (c16 == u'k') {
      return u"h24";
    }
  }

  return u"h23";
}

std::u16string getDefaultHourCycle24(NSLocale *locale) {
  auto *formatter = [[NSDateFormatter alloc] init];
  formatter.locale = locale;
  [formatter setLocalizedDateFormatFromTemplate:@"Hm"];
  auto dateFormatPattern = nsStringToU16String(formatter.dateFormat);

  for (char16_t c16 : dateFormatPattern) {
    if (c16 == u'k') {
      return u"h24";
    } else if (c16 == u'H') {
      return u"h23";
    }
  }
  return u"h23";
}

std::u16string getDefaultHourCycle12(NSLocale *locale) {
  auto *formatter = [[NSDateFormatter alloc] init];
  formatter.locale = locale;
  [formatter setLocalizedDateFormatFromTemplate:@"hm"];
  auto dateFormatPattern = nsStringToU16String(formatter.dateFormat);

  for (char16_t c16 : dateFormatPattern) {
    if (c16 == u'h') {
      return u"h12";
    } else if (c16 == u'K') {
      return u"h11";
    }
  }

  return u"h11";
}

/// Helper to lookup a \p key in a map represented as a sorted array of pairs.
template <typename K, typename V, size_t N>
std::optional<V> pairMapLookup(const std::pair<K, V> (&arr)[N], const K &key) {
  HERMES_SLOW_ASSERT(std::is_sorted(std::begin(arr), std::end(arr)));
  auto it = llvh::lower_bound(
      arr, key, [](const auto &a, const K &b) { return a.first < b; });
  if (it != std::end(arr) && it->first == key)
    return it->second;
  return std::nullopt;
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
  //  2. If the substring "-per-" does not occur exactly once in
  //  unitIdentifier, then a. Return false.
  std::u16string_view fractionDelimiter = u"-per-";
  auto foundPos = unitIdentifier.find(fractionDelimiter);
  if (foundPos == std::u16string_view::npos)
    return false;
  //  3. Let numerator be the substring of unitIdentifier from the beginning
  //  to just before "-per-".
  //  4. If the result of IsSanctionedSimpleUnitIdentifier(numerator) is
  //  false, then a. Return false.
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

/// Helper to convert from a well-formed unit identifier \p id, to one of the
/// supported built-in NSUnit types provided by Foundation.
NSUnit *unitIdentifierToNSUnit(const std::u16string &unitId) {
  static const std::pair<std::u16string_view, NSUnit *> units[] = {
    {u"acre", NSUnitArea.acres},
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_13_0
    {u"bit", NSUnitInformationStorage.bits},
    {u"byte", NSUnitInformationStorage.bytes},
#endif
    {u"celsius", NSUnitTemperature.celsius},
    {u"centimeter", NSUnitLength.centimeters},
    {u"degree", NSUnitAngle.degrees},
    {u"fahrenheit", NSUnitTemperature.fahrenheit},
    {u"fluid-ounce", NSUnitVolume.fluidOunces},
    {u"foot", NSUnitLength.feet},
    {u"gallon", NSUnitVolume.gallons},
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_13_0
    {u"gigabit", NSUnitInformationStorage.gigabits},
    {u"gigabyte", NSUnitInformationStorage.gigabytes},
#endif
    {u"gram", NSUnitMass.grams},
    {u"gram-per-liter", NSUnitConcentrationMass.gramsPerLiter},
    {u"hectare", NSUnitArea.hectares},
    {u"hour", NSUnitDuration.hours},
    {u"inch", NSUnitLength.inches},
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_13_0
    {u"kilobit", NSUnitInformationStorage.kilobits},
    {u"kilobyte", NSUnitInformationStorage.kilobytes},
#endif
    {u"kilogram", NSUnitMass.kilograms},
    {u"kilometer", NSUnitLength.kilometers},
    {u"kilometer-per-hour", NSUnitSpeed.kilometersPerHour},
    {u"liter", NSUnitVolume.liters},
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_13_0
    {u"megabit", NSUnitInformationStorage.megabits},
    {u"megabyte", NSUnitInformationStorage.megabytes},
#endif
    {u"meter", NSUnitLength.meters},
    {u"meter-per-second", NSUnitSpeed.metersPerSecond},
    {u"mile", NSUnitLength.miles},
    {u"mile-per-gallon", NSUnitFuelEfficiency.milesPerGallon},
    {u"mile-per-hour", NSUnitSpeed.milesPerHour},
    {u"mile-scandinavian", NSUnitLength.scandinavianMiles},
    {u"milliliter", NSUnitVolume.milliliters},
    {u"millimeter", NSUnitLength.millimeters},
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_13_0
    {u"millisecond", NSUnitDuration.milliseconds},
#endif
    {u"minute", NSUnitDuration.minutes},
    {u"ounce", NSUnitMass.ounces},
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_13_0
    {u"petabyte", NSUnitInformationStorage.petabytes},
#endif
    {u"pound", NSUnitMass.poundsMass},
    {u"second", NSUnitDuration.seconds},
    {u"stone", NSUnitMass.stones},
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_13_0
    {u"terabit", NSUnitInformationStorage.terabits},
    {u"terabyte", NSUnitInformationStorage.terabytes},
#endif
    {u"yard", NSUnitLength.yards}
  };
  if (auto nsUnitOpt = pairMapLookup(units, std::u16string_view(unitId)))
    return *nsUnitOpt;
  return [[NSUnit alloc] initWithSymbol:u16StringToNSString(unitId)];
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

uint8_t getCurrencyDigits(std::u16string_view code) {
  //  https://en.wikipedia.org/wiki/ISO_4217#Active_codes
  static constexpr std::pair<std::u16string_view, uint8_t> currencies[] = {
      {u"BHD", 3}, {u"BIF", 0}, {u"CLF", 4}, {u"CLP", 0}, {u"DJF", 0},
      {u"GNF", 0}, {u"IQD", 3}, {u"ISK", 0}, {u"JOD", 3}, {u"JPY", 0},
      {u"KMF", 0}, {u"KRW", 0}, {u"KWD", 3}, {u"LYD", 3}, {u"OMR", 3},
      {u"PYG", 0}, {u"RWF", 0}, {u"TND", 3}, {u"UGX", 0}, {u"UYI", 0},
      {u"UYW", 4}, {u"VND", 0}, {u"VUV", 0}, {u"XAF", 0}, {u"XOF", 0},
      {u"XPF", 0}};
  //  1. If the ISO 4217 currency and funds code list contains currency as an
  //  alphabetic code, return the minor unit value corresponding to the
  //  currency from the list; otherwise, return 2.
  if (auto digitsOpt = pairMapLookup(currencies, code))
    return *digitsOpt;
  return 2;
}
} // namespace

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

namespace {
class CollatorApple : public Collator {
 public:
  CollatorApple() = default;

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

  Options resolvedOptions() noexcept;

  double compare(const std::u16string &x, const std::u16string &y) noexcept;

 private:
  NSLocale *nsLocale_;
  NSStringCompareOptions nsCompareOptions_;
  std::u16string locale_;
  std::u16string usage_;
  std::u16string collation_;
  std::u16string caseFirst_;
  std::u16string sensitivity_;
  bool numeric_;
  bool ignorePunctuation_;
};
} // namespace

Collator::Collator() = default;
Collator::~Collator() = default;

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
  return supportedLocales(
      runtime, availableLocales, *requestedLocalesRes, options);
}

/// https://402.ecma-international.org/8.0/#sec-initializecollator
vm::ExecutionStatus CollatorApple::initialize(
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
  usage_ = std::move(**usageRes);
  // 5. If usage is "sort", then
  // a. Let localeData be %Collator%.[[SortLocaleData]].
  // 6. Else,
  // a. Let localeData be %Collator%.[[SearchLocaleData]].
  // 7. Let opt be a new Record.
  StringRecord opt;
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
  locale_ = std::move(r.locale);
  // 21. Let collation be r.[[co]].
  auto collation = r.extensions.at(u"co");
  // 22. If collation is null, let collation be "default".
  // 23. Set collator.[[Collation]] to collation.
  if (collation) {
    collation_ = *collation;
  }
  // 24. If relevantExtensionKeys contains "kn", then
  // a. Set collator.[[Numeric]] to ! SameValue(r.[[kn]], "true").
  auto numeric = r.extensions.at(u"kn") == u"true";
  numeric_ = numeric;

  // 25. If relevantExtensionKeys contains "kf", then
  // a. Set collator.[[CaseFirst]] to r.[[kf]].
  auto caseFirst = r.extensions.at(u"kf");
  if (caseFirst) {
    caseFirst_ = *caseFirst;
  }
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
    sensitivity_ = std::move(*sensitivityOpt);
  else
    sensitivity_ = u"variant";

  // 29. Let ignorePunctuation be ? GetOption(options, "ignorePunctuation",
  // "boolean", undefined,false).
  auto ignorePunctuationOpt =
      getOptionBool(runtime, options, u"ignorePunctuation", false);
  // 30. Set collator.[[IgnorePunctuation]] to ignorePunctuation.
  ignorePunctuation_ = *ignorePunctuationOpt;

  // Set up the state for calling into Obj-C APIs.
  NSStringCompareOptions cmpOpts = 0;
  if (numeric_)
    cmpOpts |= NSNumericSearch;
  if (sensitivity_ == u"base")
    cmpOpts |= (NSDiacriticInsensitiveSearch | NSCaseInsensitiveSearch);
  else if (sensitivity_ == u"accent")
    cmpOpts |= NSCaseInsensitiveSearch;
  else if (sensitivity_ == u"case")
    cmpOpts |= NSDiacriticInsensitiveSearch;
  nsCompareOptions_ = cmpOpts;

  std::u16string nsLocaleExtensions;
  if (collation_ != u"default")
    nsLocaleExtensions.append(u"-co-").append(collation_);
  else if (usage_ == u"search")
    nsLocaleExtensions.append(u"-co-search");
  if (caseFirst_ != u"false")
    nsLocaleExtensions.append(u"-kf-").append(caseFirst_);
  auto nsLocaleIdentifier = r.dataLocale;
  if (!nsLocaleExtensions.empty())
    nsLocaleIdentifier.append(u"-u").append(nsLocaleExtensions);
  nsLocale_ = [NSLocale
      localeWithLocaleIdentifier:u16StringToNSString(nsLocaleIdentifier)];
  // 31. Return collator.
  return vm::ExecutionStatus::RETURNED;
}

vm::CallResult<std::unique_ptr<Collator>> Collator::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  auto instance = std::make_unique<CollatorApple>();
  if (LLVM_UNLIKELY(
          instance->initialize(runtime, locales, options) ==
          vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return instance;
}

/// https://402.ecma-international.org/8.0/#sec-intl.collator.prototype.resolvedoptions
Options CollatorApple::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(locale_));
  options.emplace(u"usage", Option(usage_));
  options.emplace(u"sensitivity", Option(sensitivity_));
  options.emplace(u"ignorePunctuation", Option(ignorePunctuation_));
  options.emplace(u"collation", Option(collation_));
  options.emplace(u"numeric", Option(numeric_));
  options.emplace(u"caseFirst", Option(caseFirst_));
  return options;
}

Options Collator::resolvedOptions() noexcept {
  return static_cast<CollatorApple *>(this)->resolvedOptions();
}

/// https://402.ecma-international.org/8.0/#sec-intl.collator.prototype.compare
double CollatorApple::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  NSString *nsX = u16StringToNSString(x);
  NSString *nsY = u16StringToNSString(y);
  if (ignorePunctuation_) {
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
              options:nsCompareOptions_
                range:NSMakeRange(0, nsX.length)
               locale:nsLocale_];
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return static_cast<CollatorApple *>(this)->compare(x, y);
}

namespace {
// Implementation of
// https://402.ecma-international.org/8.0/#datetimeformat-objects
class DateTimeFormatApple : public DateTimeFormat {
 public:
  DateTimeFormatApple() = default;

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &inputOptions) noexcept;

  Options resolvedOptions() noexcept;

  std::u16string format(double jsTimeValue) noexcept;

  std::vector<Part> formatToParts(double x) noexcept;

  std::u16string formatRange(
      double jsTimeValueFrom,
      double jsTimeValueTo) noexcept;

  std::vector<Part> formatRangeToParts(
      double jsTimeValueFrom,
      double jsTimeValueTo) noexcept;

 private:
  void initializeNSDateFormatter() noexcept;

  // https://402.ecma-international.org/8.0/#sec-properties-of-intl-datetimeformat-instances
  // Intl.DateTimeFormat instances have an [[InitializedDateTimeFormat]]
  // internal slot.
  // NOTE: InitializedDateTimeFormat is not implemented.
  // Intl.DateTimeFormat instances also have several internal
  // slots that are computed by the constructor:
  // [[Locale]] is a String value with the language tag of the locale whose
  // localization is used for formatting.
  std::u16string locale_;
  std::u16string dataLocale_;
  // [[Calendar]] is a String value with the "type" given in Unicode Technical
  // Standard 35 for the calendar used for formatting.
  std::optional<std::u16string> calendar_;
  // [[NumberingSystem]] is a String value with the "type" given in Unicode
  // Technical Standard 35 for the numbering system used for formatting.
  // NOTE: Even though NSDateFormatter formats date and time using different
  // numbering systems based on its "locale" value, it does not allow to set/get
  // the numbering system value explicitly. So we consider this feature
  // unsupported.
  // [[TimeZone]] is a String value with the IANA time zone name of the time
  // zone used for formatting.
  std::u16string timeZone_;
  int timeZoneOffsetMinutes_;
  // [[Weekday]], [[Era]], [[Year]], [[Month]], [[Day]], [[DayPeriod]],
  // [[Hour]], [[Minute]], [[Second]], [[TimeZoneName]] are each either
  // undefined, indicating that the component is not used for formatting, or one
  // of the String values given in Table 4, indicating how the component should
  // be presented in the formatted output.
  std::optional<std::u16string> weekday_;
  std::optional<std::u16string> era_;
  std::optional<std::u16string> year_;
  std::optional<std::u16string> month_;
  std::optional<std::u16string> day_;
  std::optional<std::u16string> dayPeriod_;
  std::optional<std::u16string> hour_;
  std::optional<std::u16string> minute_;
  std::optional<std::u16string> second_;
  std::optional<std::u16string> timeZoneName_;
  // [[FractionalSecondDigits]] is either undefined or a positive, non-zero
  // integer Number value indicating the fraction digits to be used for
  // fractional seconds. Numbers will be rounded or padded with trailing zeroes
  // if necessary.
  std::optional<uint8_t> fractionalSecondDigits_;
  // [[HourCycle]] is a String value indicating whether the 12-hour format
  // ("h11", "h12") or the 24-hour format ("h23", "h24") should be used. "h11"
  // and "h23" start with hour 0 and go up to 11 and 23 respectively. "h12" and
  // "h24" start with hour 1 and go up to 12 and 24. [[HourCycle]] is only used
  // when [[Hour]] is not undefined.
  std::optional<std::u16string> hourCycle_;
  // [[DateStyle]], [[TimeStyle]] are each either undefined, or a String value
  // with values "full", "long", "medium", or "short".
  std::optional<std::u16string> dateStyle_;
  std::optional<std::u16string> timeStyle_;
  // [[Pattern]] is a String value as described in 11.3.3.
  // [[RangePatterns]] is a Record as described in 11.3.3.
  // Finally, Intl.DateTimeFormat instances have a [[BoundFormat]]
  // internal slot that caches the function returned by the format accessor
  // (11.4.3).
  // NOTE: Pattern and RangePatterns are not implemented. BoundFormat is
  // implemented in Intl.cpp.
  NSDateFormatter *nsDateFormatter_;
  NSDateIntervalFormatter *nsDateIntervalFormatter_;
};
} // namespace

DateTimeFormat::DateTimeFormat() = default;
DateTimeFormat::~DateTimeFormat() = default;

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
  return supportedLocales(
      runtime, availableLocales, requestedLocales.getValue(), options);
}

// Implementation of
// https://402.ecma-international.org/8.0/#sec-initializedatetimeformat
vm::ExecutionStatus DateTimeFormatApple::initialize(
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
  StringRecord opt;
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
  auto numberingSystemRes =
      getOptionString(runtime, options, u"numberingSystem", {}, {});
  // 10. If numberingSystem is not undefined, then
  if (numberingSystemRes->has_value()) {
    auto numberingSystemOpt = *numberingSystemRes;
    // a. If numberingSystem does not match the Unicode Locale Identifier
    // type nonterminal, throw a RangeError exception.
    if (!isUnicodeExtensionType(*numberingSystemOpt))
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid numbering system: ") +
          vm::TwineChar16(numberingSystemOpt->c_str()));
  }
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
  std::optional<std::u16string> hourCycleOpt = *hourCycleRes;
  // 14. If hour12 is not undefined, then
  if (hour12.has_value())
    // a. Let hourCycle be null.
    hourCycleOpt = std::nullopt;

  // 15. Set opt.[[hc]] to hourCycle.
  // _If_ hourCycle has a value, add it to opt
  // or add std::nullopt if hour12 has a value
  if (hourCycleOpt.has_value() || hour12.has_value())
    opt.emplace(u"hc", hourCycleOpt);
  // 16. Let localeData be %DateTimeFormat%.[[LocaleData]].
  // NOTE: We use NSLocale as "localeData", see 23.
  // 17. Let r be ResolveLocale(%DateTimeFormat%.[[AvailableLocales]],
  // requestedLocales, opt, %DateTimeFormat%.[[RelevantExtensionKeys]],
  // localeData).
  static constexpr std::u16string_view relevantExtensionKeys[] = {
      u"ca", u"nu", u"hc"};
  auto r = resolveLocale(
      getAvailableLocales(), *requestedLocalesRes, opt, relevantExtensionKeys);
  // 18. Set dateTimeFormat.[[Locale]] to r.[[locale]].
  locale_ = std::move(r.locale);
  dataLocale_ = r.dataLocale;
  // 19. Let calendar be r.[[ca]].
  auto ca = r.extensions.at(u"ca");
  // 20. Set dateTimeFormat.[[Calendar]] to calendar.
  calendar_ = ca;
  // 21. Set dateTimeFormat.[[HourCycle]] to r.[[hc]].
  auto hc = r.extensions.at(u"hc");
  hourCycle_ = hc;
  // 22. Set dateTimeFormat.[[NumberingSystem]] to r.[[nu]].
  // 23. Let dataLocale be r.[[dataLocale]].
  auto dataLocale = r.dataLocale;
  // 23. Let dataLocaleData be localeData.[[<dataLocale>]].
  auto dataLocaleData =
      [NSLocale localeWithLocaleIdentifier:u16StringToNSString(dataLocale_)];
  // 24. Let hcDefault be dataLocaleData.[[hourCycle]].
  auto hcDefault = getDefaultHourCycle(dataLocaleData);
  // If hour12 is true, then
  if (hour12.has_value() && *hour12 == true) {
    // a. If hcDefault is "h11" or "h23", let hc be "h11". Otherwise, let hc be
    // "h12".
    hc = getDefaultHourCycle12(dataLocaleData);
    // 26. Else if hour12 is false, then
  } else if (hour12.has_value() && *hour12 == false) {
    // a. If hcDefault is "h11" or "h23", let hc be "h23". Otherwise, let hc be
    // "h24".
    hc = getDefaultHourCycle24(dataLocaleData);
  } else {
    // 27. Else,
    //  a. Assert: hour12 is undefined.
    //  b. Let hc be r.[[hc]].
    hc = r.extensions.at(u"hc");
    // c. If hc is null, set hc to hcDefault.
    if (!hc) {
      hc = hcDefault;
    }
  }
  // 28. Set dateTimeFormat.[[HourCycle]] to hc.
  hourCycle_ = hc;
  // 29. Let timeZone be ? Get(options, "timeZone").
  auto timeZoneIt = options.find(u"timeZone");
  std::u16string timeZone;
  // 30. If timeZone is undefined, then
  if (timeZoneIt == options.end()) {
    // a. Let timeZone be DefaultTimeZone().
    timeZone = getDefaultTimeZone();
    // 31. Else,
  } else {
    // a. Let timeZone be ? ToString(timeZone).
    timeZone = timeZoneIt->second.getString();
    auto timeZoneOffsetStringParseResult = getTimeZoneOffsetString(timeZone);
    auto isTimeZoneOffsetString = timeZoneOffsetStringParseResult.has_value();
    if (isTimeZoneOffsetString) {
      // a. Let parseResult be ParseText(StringToCodePoints(timeZone),
      // UTCOffset).
      auto parseResult = timeZoneOffsetStringParseResult.value();
      // c. If parseResult contains more than one MinuteSecond Parse Node, throw
      // a RangeError exception.
      if (parseResult.hasSecondsComponent) {
        return runtime.raiseRangeError(
            "TimeZone offset has unexpected seconds components");
      }

      // d. Let offsetNanoseconds be ParseTimeZoneOffsetString(timeZone).
      long long offsetNanoseconds = parseResult.nanoseconds;
      // e. Let offsetMinutes be offsetNanoseconds / (6 × 10**10).
      long long offsetMinutes = offsetNanoseconds / (6 * 1e10);
      // f. Assert: offsetMinutes is an integer.
      assert(
          offsetMinutes >= NSIntegerMin && offsetMinutes <= NSIntegerMax &&
          "Offset minutes is within an integer.");
      // g. Set timeZone to FormatOffsetTimeZoneIdentifier(offsetMinutes).
      timeZoneOffsetMinutes_ = static_cast<int>(offsetMinutes);
      timeZone =
          formatOffsetTimeZoneIdentifier(static_cast<int>(offsetMinutes));
    } else if (isValidTimeZoneName(timeZone)) {
      timeZone = getAvailableNamedTimeZoneIdentifier(timeZone);
    } else {
      // Throw a RangeError exception.
      return runtime.raiseRangeError(
          vm::TwineChar16("Incorrect timeZone information provided: ") +
          vm::TwineChar16(timeZone.c_str()));
    }
  }

  // 32. Set dateTimeFormat.[[TimeZone]] to timeZone.
  timeZone_ = timeZone;
  // 33. Let formatOptions be a new Record.
  // NOTE: We use properties of this class like _<key> as
  // formatOptions.[[<key>]]
  // 34. Set formatOptions.[[hourCycle]] to hc.
  // 35.  Let hasExplicitFormatComponents be false.
  auto hasExplicitFormatComponents = false;

  // 36. For each row of Table 7, except the header row, in table order, do
  //   a. Let prop be the name given in the Property column of the row.
  //   b. If prop is "fractionalSecondDigits", then
  //     i. Let value be ? GetNumberOption(options, "fractionalSecondDigits", 1,
  //     3, undefined).
  auto fractionalSecondDigitsRes =
      getNumberOption(runtime, options, u"fractionalSecondDigits", 1, 3, {});
  if (LLVM_UNLIKELY(
          fractionalSecondDigitsRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // d. Set formatOptions.[[<prop>]] to value.
  fractionalSecondDigits_ = *fractionalSecondDigitsRes;
  // e. If value is not undefined, then
  if (fractionalSecondDigits_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;
  // c. Else,
  //   i. Let values be a List whose elements are the strings given in the
  //   Values column of the row. ii. Let value be ? GetOption(options, prop,
  //   string, values, undefined).
  static constexpr std::u16string_view weekdayValues[] = {
      u"narrow", u"short", u"long"};
  auto weekdayRes =
      getOptionString(runtime, options, u"weekday", weekdayValues, {});
  if (LLVM_UNLIKELY(weekdayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  weekday_ = *weekdayRes;
  // e. If value is not undefined, then
  if (weekday_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;

  // i. Let values be a List whose elements are the strings given in the Values
  // column of the row. ii. Let value be ? GetOption(options, prop, string,
  // values, undefined).
  static constexpr std::u16string_view eraValues[] = {
      u"narrow", u"short", u"long"};
  auto eraRes = getOptionString(runtime, options, u"era", eraValues, {});
  if (LLVM_UNLIKELY(eraRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  era_ = *eraRes;
  // e. If value is not undefined, then
  if (era_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;

  // i. Let values be a List whose elements are the strings given in the Values
  // column of the row. ii. Let value be ? GetOption(options, prop, string,
  // values, undefined).
  static constexpr std::u16string_view yearValues[] = {u"2-digit", u"numeric"};
  auto yearRes = getOptionString(runtime, options, u"year", yearValues, {});
  if (LLVM_UNLIKELY(yearRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  year_ = *yearRes;
  // e. If value is not undefined, then
  if (year_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;

  // i. Let values be a List whose elements are the strings given in the Values
  // column of the row. ii. Let value be ? GetOption(options, prop, string,
  // values, undefined).
  static constexpr std::u16string_view monthValues[] = {
      u"2-digit", u"numeric", u"narrow", u"short", u"long"};
  auto monthRes = getOptionString(runtime, options, u"month", monthValues, {});
  if (LLVM_UNLIKELY(monthRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  month_ = *monthRes;
  // e. If value is not undefined, then
  if (month_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;

  // i. Let values be a List whose elements are the strings given in the Values
  // column of the row. ii. Let value be ? GetOption(options, prop, string,
  // values, undefined).
  static constexpr std::u16string_view dayValues[] = {u"2-digit", u"numeric"};
  auto dayRes = getOptionString(runtime, options, u"day", dayValues, {});
  if (LLVM_UNLIKELY(dayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  day_ = *dayRes;
  // e. If value is not undefined, then
  if (day_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;

  // i. Let values be a List whose elements are the strings given in the Values
  // column of the row. ii. Let value be ? GetOption(options, prop, string,
  // values, undefined).
  static constexpr std::u16string_view dayPeriodValues[] = {
      u"narrow", u"short", u"long"};
  auto dayPeriodRes =
      getOptionString(runtime, options, u"dayPeriod", dayPeriodValues, {});
  if (LLVM_UNLIKELY(dayPeriodRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  dayPeriod_ = *dayPeriodRes;
  // e. If value is not undefined, then
  if (dayPeriod_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;

  // i. Let values be a List whose elements are the strings given in the Values
  // column of the row. ii. Let value be ? GetOption(options, prop, string,
  // values, undefined).
  static constexpr std::u16string_view hourValues[] = {u"2-digit", u"numeric"};
  auto hourRes = getOptionString(runtime, options, u"hour", hourValues, {});
  if (LLVM_UNLIKELY(hourRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  hour_ = *hourRes;
  // e. If value is not undefined, then
  if (hour_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;

  // i. Let values be a List whose elements are the strings given in the Values
  // column of the row. ii. Let value be ? GetOption(options, prop, string,
  // values, undefined).
  static constexpr std::u16string_view minuteValues[] = {
      u"2-digit", u"numeric"};
  auto minuteRes =
      getOptionString(runtime, options, u"minute", minuteValues, {});
  if (LLVM_UNLIKELY(minuteRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  minute_ = *minuteRes;
  // e. If value is not undefined, then
  if (minute_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;

  // i. Let values be a List whose elements are the strings given in the Values
  // column of the row. ii. Let value be ? GetOption(options, prop, string,
  // values, undefined).
  static constexpr std::u16string_view secondValues[] = {
      u"2-digit", u"numeric"};
  auto secondRes =
      getOptionString(runtime, options, u"second", secondValues, {});
  if (LLVM_UNLIKELY(secondRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  second_ = *secondRes;
  // e. If value is not undefined, then
  if (second_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;

  // i. Let values be a List whose elements are the strings given in the Values
  // column of the row. ii. Let value be ? GetOption(options, prop, string,
  // values, undefined). NOTE: "shortOffset", "longOffset", "shortGeneric",
  // "longGeneric" are specified here:
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
      runtime, options, u"timeZoneName", timeZoneNameValues, {});
  if (LLVM_UNLIKELY(timeZoneNameRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  timeZoneName_ = *timeZoneNameRes;
  // e. If value is not undefined, then
  if (timeZoneName_)
    // i. Set hasExplicitFormatComponents to true.
    hasExplicitFormatComponents = true;

  // 37. Let matcher be ? GetOption(options, "formatMatcher", string, « "basic",
  // "best fit" », "best fit").
  auto formatMatcherRes = getOptionString(
      runtime, options, u"formatMatcher", {u"basic", u"best fit"}, u"best fit");
  if (LLVM_UNLIKELY(formatMatcherRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;

  // 38. Let dateStyle be ? GetOption(options, "dateStyle", string, « "full",
  // "long", "medium", "short" », undefined).
  static constexpr std::u16string_view dateStyles[] = {
      u"full", u"long", u"medium", u"short"};
  auto dateStyleRes =
      getOptionString(runtime, options, u"dateStyle", dateStyles, {});
  if (LLVM_UNLIKELY(dateStyleRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;

  // 39. Set dateTimeFormat.[[DateStyle]] to dateStyle.
  dateStyle_ = *dateStyleRes;
  // 40. Let timeStyle be ? GetOption(options, "timeStyle", string, « "full",
  // "long", "medium", "short" », undefined).
  static constexpr std::u16string_view timeStyles[] = {
      u"full", u"long", u"medium", u"short"};
  auto timeStyleRes =
      getOptionString(runtime, options, u"timeStyle", timeStyles, {});
  if (LLVM_UNLIKELY(timeStyleRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 41. Set dateTimeFormat.[[TimeStyle]] to timeStyle.
  timeStyle_ = *timeStyleRes;

  // 42. If dateStyle is not undefined or timeStyle is not undefined, then
  if (dateStyle_.has_value() || timeStyle_.has_value()) {
    // a. If hasExplicitFormatComponents is true, then
    if (hasExplicitFormatComponents) {
      // i. Throw a TypeError exception.
      return runtime.raiseTypeError(
          "Cannot use explicit format options with dateStyle or timeStyle");
    }
    // b. Let styles be dataLocaleData.[[styles]].[[<resolvedCalendar>]].
    // c. Let bestFormat be DateTimeStyleFormat(dateStyle, timeStyle, styles).
  }
  // 43. Else,
  //   a. Let formats be dataLocaleData.[[formats]].[[<resolvedCalendar>]].
  //   b. If matcher is "basic", then
  //     i. Let bestFormat be BasicFormatMatcher(formatOptions, formats).
  //   c. Else,
  //     i. Let bestFormat be BestFitFormatMatcher(formatOptions, formats).
  // 44. For each row in Table 7, except the header row, in table order, do
  //   a. Let prop be the name given in the Property column of the row.
  //   b. If bestFormat has a field [[<prop>]], then
  //     i. Let p be bestFormat.[[<prop>]].
  //     ii. Set dateTimeFormat's internal slot whose name is the Internal Slot
  //     column of the row to p.
  // 45. If dateTimeFormat.[[Hour]] is undefined, then
  if (!hour_.has_value() && !timeStyle_.has_value()) {
    // a. Set dateTimeFormat.[[HourCycle]] to undefined.
    hourCycle_ = std::nullopt;
  }
  // 46. If dateTimeFormat.[[HourCycle]] is "h11" or "h12", then
  //   a. Let pattern be bestFormat.[[pattern12]].
  //   b. Let rangePatterns be bestFormat.[[rangePatterns12]].
  // 47. Else,
  //   a. Let pattern be bestFormat.[[pattern]].
  //   b. Let rangePatterns be bestFormat.[[rangePatterns]].
  // 48. Set dateTimeFormat.[[Pattern]] to pattern.
  // 49. Set dateTimeFormat.[[RangePatterns]] to rangePatterns.
  // 50. Return dateTimeFormat.
  initializeNSDateFormatter();
  return vm::ExecutionStatus::RETURNED;
}

vm::CallResult<std::unique_ptr<DateTimeFormat>> DateTimeFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &inputOptions) noexcept {
  auto instance = std::make_unique<DateTimeFormatApple>();
  if (LLVM_UNLIKELY(
          instance->initialize(runtime, locales, inputOptions) ==
          vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return instance;
}

// Implementer note: This method corresponds roughly to
// https://402.ecma-international.org/8.0/#sec-intl.datetimeformat.prototype.resolvedoptions
Options DateTimeFormatApple::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(locale_));
  options.emplace(u"timeZone", Option(timeZone_));
  if (calendar_)
    options.emplace(u"calendar", Option(*calendar_));
  if (hourCycle_.has_value()) {
    options.emplace(u"hourCycle", *hourCycle_);
    options.emplace(u"hour12", hourCycle_ == u"h11" || hourCycle_ == u"h12");
  }
  if (weekday_.has_value())
    options.emplace(u"weekday", *weekday_);
  if (era_.has_value())
    options.emplace(u"era", *era_);
  if (year_.has_value())
    options.emplace(u"year", *year_);
  if (month_.has_value())
    options.emplace(u"month", *month_);
  if (day_.has_value())
    options.emplace(u"day", *day_);
  if (hour_.has_value())
    options.emplace(u"hour", *hour_);
  if (minute_.has_value())
    options.emplace(u"minute", *minute_);
  if (second_.has_value())
    options.emplace(u"second", *second_);
  if (timeZoneName_.has_value())
    options.emplace(u"timeZoneName", *timeZoneName_);
  if (dateStyle_.has_value())
    options.emplace(u"dateStyle", *dateStyle_);
  if (timeStyle_.has_value())
    options.emplace(u"timeStyle", *timeStyle_);
  return options;
}

Options DateTimeFormat::resolvedOptions() noexcept {
  return static_cast<DateTimeFormatApple *>(this)->resolvedOptions();
}

void DateTimeFormatApple::initializeNSDateFormatter() noexcept {
  static constexpr std::u16string_view kLong = u"long", kShort = u"short",
                                       kNarrow = u"narrow", kMedium = u"medium",
                                       kFull = u"full", kNumeric = u"numeric",
                                       kTwoDigit = u"2-digit",
                                       kShortOffset = u"shortOffset",
                                       kLongOffset = u"longOffset",
                                       kShortGeneric = u"shortGeneric",
                                       kLongGeneric = u"longGeneric";

  nsDateFormatter_ = [[NSDateFormatter alloc] init];
  nsDateFormatter_.locale =
      [[NSLocale alloc] initWithLocaleIdentifier:u16StringToNSString(locale_)];

  nsDateIntervalFormatter_ = [[NSDateIntervalFormatter alloc] init];
  nsDateIntervalFormatter_.locale = nsDateFormatter_.locale;
  if (calendar_) {
    nsDateFormatter_.calendar = [[NSCalendar alloc]
        initWithCalendarIdentifier:u16StringToNSString(
                                       getNSCalendarIdentifier(*calendar_))];
    nsDateIntervalFormatter_.calendar = nsDateFormatter_.calendar;
  }

  if (timeStyle_.has_value()) {
    if (*timeStyle_ == kFull) {
      nsDateFormatter_.timeStyle = NSDateFormatterFullStyle;
      nsDateIntervalFormatter_.timeStyle = NSDateIntervalFormatterFullStyle;
    } else if (*timeStyle_ == kLong) {
      nsDateFormatter_.timeStyle = NSDateFormatterLongStyle;
      nsDateIntervalFormatter_.timeStyle = NSDateIntervalFormatterLongStyle;
    } else if (*timeStyle_ == kMedium) {
      nsDateFormatter_.timeStyle = NSDateFormatterMediumStyle;
      nsDateIntervalFormatter_.timeStyle = NSDateIntervalFormatterMediumStyle;
    } else {
      assert(*timeStyle_ == kShort && "No other valid timeStyle.");
      nsDateFormatter_.timeStyle = NSDateFormatterShortStyle;
      nsDateIntervalFormatter_.timeStyle = NSDateIntervalFormatterShortStyle;
    }
  }
  if (dateStyle_.has_value()) {
    if (*dateStyle_ == kFull) {
      nsDateFormatter_.dateStyle = NSDateFormatterFullStyle;
      nsDateIntervalFormatter_.dateStyle = NSDateIntervalFormatterFullStyle;
    } else if (*dateStyle_ == kLong) {
      nsDateFormatter_.dateStyle = NSDateFormatterLongStyle;
      nsDateIntervalFormatter_.dateStyle = NSDateIntervalFormatterLongStyle;
    } else if (*dateStyle_ == kMedium) {
      nsDateFormatter_.dateStyle = NSDateFormatterMediumStyle;
      nsDateIntervalFormatter_.dateStyle = NSDateIntervalFormatterMediumStyle;
    } else {
      assert(*dateStyle_ == kShort && "No other valid dateStyle.");
      nsDateFormatter_.dateStyle = NSDateFormatterShortStyle;
      nsDateIntervalFormatter_.dateStyle = NSDateIntervalFormatterShortStyle;
    }
  }

  if (timeZone_.size() > 0 && (timeZone_[0] == u'+' || timeZone_[0] == u'-')) {
    nsDateFormatter_.timeZone =
        [NSTimeZone timeZoneForSecondsFromGMT:60 * timeZoneOffsetMinutes_];
    nsDateIntervalFormatter_.timeZone =
        [NSTimeZone timeZoneForSecondsFromGMT:60 * timeZoneOffsetMinutes_];
  } else {
    nsDateFormatter_.timeZone = createTimeZone(timeZone_);
    nsDateIntervalFormatter_.timeZone = createTimeZone(timeZone_);
  }

  if (timeStyle_.has_value() || dateStyle_.has_value())
    return;
  // The following options cannot be used in conjunction with timeStyle or
  // dateStyle
  // Form a custom format string It will be reordered according to
  // locale later
  NSMutableString *customFormattedDate = [[NSMutableString alloc] init];
  if (timeZoneName_.has_value()) {
    if (*timeZoneName_ == kShort) {
      [customFormattedDate appendString:@"z"];
    } else if (*timeZoneName_ == kLong) {
      [customFormattedDate appendString:@"zzzz"];
    } else if (*timeZoneName_ == kShortOffset) {
      [customFormattedDate appendString:@"O"];
    } else if (*timeZoneName_ == kLongOffset) {
      [customFormattedDate appendString:@"OOOO"];
    } else if (*timeZoneName_ == kShortGeneric) {
      [customFormattedDate appendString:@"v"];
    } else {
      assert(*timeZoneName_ == kLongGeneric && "No other valid timeZoneName");
      [customFormattedDate appendString:@"vvvv"];
    }
  }
  if (era_.has_value()) {
    if (*era_ == kNarrow) {
      [customFormattedDate appendString:@"GGGGG"];
    } else if (*era_ == kShort) {
      [customFormattedDate appendString:@"GGG"];
    } else {
      assert(*era_ == kLong && "No other valid era.");
      [customFormattedDate appendString:@"GGGG"];
    }
  }
  if (year_.has_value()) {
    if (*year_ == kNumeric) {
      [customFormattedDate appendString:@"yyyy"];
    } else {
      assert(*year_ == kTwoDigit && "No other valid year.");
      [customFormattedDate appendString:@"yy"];
    }
  }
  if (month_.has_value()) {
    if (*month_ == kNarrow) {
      [customFormattedDate appendString:@"MMMMM"];
    } else if (*month_ == kNumeric) {
      [customFormattedDate appendString:@"M"];
    } else if (*month_ == kTwoDigit) {
      [customFormattedDate appendString:@"MM"];
    } else if (*month_ == kShort) {
      [customFormattedDate appendString:@"MMM"];
    } else {
      assert(*month_ == kLong && "No other valid month.");
      [customFormattedDate appendString:@"MMMM"];
    }
  }
  if (weekday_.has_value()) {
    if (*weekday_ == kNarrow) {
      [customFormattedDate appendString:@"EEEEE"];
    } else if (*weekday_ == kShort) {
      [customFormattedDate appendString:@"E"];
    } else {
      assert(*weekday_ == kLong && "No other valid weekday.");
      [customFormattedDate appendString:@"EEEE"];
    }
  }
  if (day_.has_value()) {
    if (*day_ == kNumeric) {
      [customFormattedDate appendString:@"d"];
    } else {
      assert(*day_ == kTwoDigit && "No other valid day.");
      [customFormattedDate appendString:@"dd"];
    }
  }
  if (hour_.has_value()) {
    // Ignore the hour12 bool in the impl_ struct
    // a = AM/PM for 12 hr clocks, automatically added depending on locale
    // AM/PM not multilingual, de-DE should be "03 Uhr" not "3 AM"
    // K = h11 = 0-11
    // h = h12 = 1-12
    // H = h23 = 0-23
    // k = h24 = 1-24
    if (hourCycle_ == u"h12") {
      if (*hour_ == kNumeric) {
        [customFormattedDate appendString:@"h"];
      } else {
        assert(*hour_ == kTwoDigit && "No other valid hour.");
        [customFormattedDate appendString:@"hh"];
      }
    } else if (hourCycle_ == u"h24") {
      if (*hour_ == kNumeric) {
        [customFormattedDate appendString:@"k"];
      } else {
        assert(*hour_ == kTwoDigit && "No other valid hour.");
        [customFormattedDate appendString:@"kk"];
      }
    } else if (hourCycle_ == u"h11") {
      if (*hour_ == kNumeric) {
        [customFormattedDate appendString:@"K"];
      } else {
        assert(*hour_ == kTwoDigit && "No other valid hour.");
        [customFormattedDate appendString:@"KK"];
      }
    } else { // h23
      if (*hour_ == kNumeric) {
        [customFormattedDate appendString:@"H"];
      } else {
        assert(*hour_ == kTwoDigit && "No other valid hour.");
        [customFormattedDate appendString:@"HH"];
      }
    }
  }
  if (minute_.has_value()) {
    if (*minute_ == kNumeric) {
      [customFormattedDate appendString:@"m"];
    } else {
      assert(*minute_ == kTwoDigit && "No other valid minute.");
      [customFormattedDate appendString:@"mm"];
    }
  }
  if (second_.has_value()) {
    if (*second_ == kNumeric) {
      [customFormattedDate appendString:@"s"];
    } else {
      assert(*second_ == kTwoDigit && "No other valid second.");
      [customFormattedDate appendString:@"ss"];
    }
  }
  if (fractionalSecondDigits_.has_value()) {
    // This currently outputs to 3 digits only with the date?
    switch (*fractionalSecondDigits_) {
      case 1:
        [customFormattedDate appendString:@"S"];
        break;
      case 2:
        [customFormattedDate appendString:@"SS"];
        break;
      case 3:
        [customFormattedDate appendString:@"SSS"];
        break;
    }
  }
  if (dayPeriod_.has_value()) {
    if (*dayPeriod_ == u"narrow") {
      [customFormattedDate appendString:@"BBBBB"];
    } else if (*dayPeriod_ == u"short") {
      [customFormattedDate appendString:@"B"];
    } else if (*dayPeriod_ == u"long") {
      [customFormattedDate appendString:@"BBBB"];
    }
  }
  // Set the custom date from all the concatonated NSStrings (locale will
  // automatically separate the order) Only set a template format if it isn't
  // empty
  if (customFormattedDate.length > 0) {
    [nsDateFormatter_ setLocalizedDateFormatFromTemplate:customFormattedDate];
    nsDateIntervalFormatter_.dateTemplate =
        [NSDateFormatter dateFormatFromTemplate:customFormattedDate
                                        options:0
                                         locale:nsDateFormatter_.locale];
  } else {
    nsDateFormatter_.dateStyle = NSDateFormatterShortStyle;
    nsDateIntervalFormatter_.dateStyle = NSDateIntervalFormatterShortStyle;
  }
}

std::u16string DateTimeFormatApple::format(double jsTimeValue) noexcept {
  auto timeInSeconds = jsTimeValue / 1000;
  NSDate *date = [NSDate dateWithTimeIntervalSince1970:timeInSeconds];
  return nsStringToU16String([nsDateFormatter_ stringFromDate:date]);
}

std::u16string DateTimeFormatApple::formatRange(
    double jsTimeValueFrom,
    double jsTimeValueTo) noexcept {
  auto timeInSecondsFrom = jsTimeValueFrom / 1000;
  NSDate *dateFrom = [NSDate dateWithTimeIntervalSince1970:timeInSecondsFrom];
  auto timeInSecondsTo = jsTimeValueTo / 1000;
  NSDate *dateTo = [NSDate dateWithTimeIntervalSince1970:timeInSecondsTo];
  return nsStringToU16String([nsDateIntervalFormatter_ stringFromDate:dateFrom
                                                               toDate:dateTo]);
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  return static_cast<DateTimeFormatApple *>(this)->format(jsTimeValue);
}

std::vector<Part> DateTimeFormat::formatToParts(double x) noexcept {
  return static_cast<DateTimeFormatApple *>(this)->formatToParts(x);
}

// Map date format patterns to Intl option types
// See
// https://developer.apple.com/documentation/foundation/nsdateformatter/1408112-dateformatfromtemplate#parameters
static std::u16string typeFromDateFormatPattern(const char16_t &c16) {
  if (c16 == u'a' || c16 == u'b' || c16 == u'B')
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
  if (c16 == u'U')
    return u"yearName";
  if (c16 == u'r')
    return u"relatedYear";

  // If the character is not known, return literal.
  return u"literal";
}

// Implementer note: This method corresponds roughly to
// https://402.ecma-international.org/8.0/#sec-formatdatetimetoparts
std::vector<Part> DateTimeFormatApple::formatToParts(double x) noexcept {
  // NOTE: We don't have access to localeData.patterns. Instead we use
  // NSDateFormatter's format string, and break it into components.

  // 1. Let parts be ? PartitionDateTimePattern(dateTimeFormat, x).
  std::vector<std::u16string> parts;
  auto dateTimeFormat = nsStringToU16String(nsDateFormatter_.dateFormat);
  auto timeInSeconds = x / 1000;
  NSDate *date = [NSDate dateWithTimeIntervalSince1970:timeInSeconds];
  // 2. Let result be ArrayCreate(0).
  std::vector<Part> result;
  // 3. Let n be 0.
  size_t n = 0;

  auto formattedString = format(x);

  char16_t formatCharacter = dateTimeFormat[n];

  std::u16string type = typeFromDateFormatPattern(formatCharacter);
  std::u16string currentType = type;

  std::u16string partFormatString = u"";
  partFormatString.reserve(6);

  for (size_t n = 0; n < dateTimeFormat.size(); n++) {
    formatCharacter = dateTimeFormat[n];
    type = typeFromDateFormatPattern(formatCharacter);

    if (type != currentType) {
      parts.push_back(partFormatString);

      partFormatString.clear();
      currentType = type;
    }

    partFormatString += formatCharacter;
  }

  if (partFormatString.size() > 0) {
    parts.push_back(std::move(partFormatString));
  }

  NSDateFormatter *nsDatePartFormatter = [[NSDateFormatter alloc] init];
  nsDatePartFormatter.locale = nsDateFormatter_.locale;
  nsDatePartFormatter.timeZone = nsDateFormatter_.timeZone;
  nsDatePartFormatter.calendar = nsDateFormatter_.calendar;

  // 4. For each Record { [[Type]], [[Value]] } part in parts, do
  for (auto part : parts) {
    if (part == u"")
      continue;

    auto type = typeFromDateFormatPattern(part[0]);

    if (type == u"literal") {
      result.push_back({{u"type", u"literal"}, {u"value", part}});
    } else {
      nsDatePartFormatter.dateFormat = u16StringToNSString(part);
      std::u16string formattedPart =
          nsStringToU16String([nsDatePartFormatter stringFromDate:date]);
      result.push_back({{u"type", type}, {u"value", formattedPart}});
    }
  }

  // 5. Return result.
  return result;
}

std::vector<Part> DateTimeFormatApple::formatRangeToParts(
    double x,
    double y) noexcept {
  // NOTE: NSDateIntervalFormatter does not expose details on what
  // parts of the formatted string belong to which date, so as a stub
  // we'll implement formatRangeToParts using a basic format of
  // [start] - [end], calling formatToParts on [start] and [end]
  // individually. While this meets the spec, it does not match
  // the formatting of formatRange.

  // A quick check if the outputs should be equal, in
  // that case we should just return the parts as "shared".
  auto isTheSameDateTimeOutput = format(x) == format(y);

  if (isTheSameDateTimeOutput) {
    std::vector<Part> parts = formatToParts(x);
    std::vector<Part> result;

    for (Part part : parts) {
      Part O(part);
      O.insert({u"source", u"shared"});
      result.push_back(O);
    }

    return result;
  }

  // 1. Let parts be ? PartitionDateTimeRangePattern(dateTimeFormat, x, y).
  std::vector<Part> xParts = formatToParts(x);
  std::vector<Part> yParts = formatToParts(y);

  // 2. Let result be ! ArrayCreate(0).
  std::vector<Part> result;

  // Reserve the expected space required, this is the parts
  // of [start], the parts of [end], and 3 literals (" ", "-", " ")
  result.reserve(xParts.size() + yParts.size() + 3);

  // Combine the parts
  for (auto [first, second] :
       std::vector<std::pair<std::u16string, std::vector<Part>>>{
           {u"startRange", xParts},
           {
               u"shared",
               {
                   {{u"type", u"literal"},
                    // thin space, en-dash, thin space to match formatRange
                    {u"value", u"\u2009\u2013\u2009"},
                    {u"source", u"shared"}},
               },
           },
           {u"endRange", yParts}}) {
    // 3. Let n be 0.
    for (Part part : second) {
      // 4. For each Record { [[Type]], [[Value]], [[Source]] } part in parts,
      // do

      //   a. Let O be OrdinaryObjectCreate(%Object.prototype%).
      //   d. Perform ! CreateDataPropertyOrThrow(O, "source", part.[[Source]]).
      Part O{{u"source", first}};
      //   b. Perform ! CreateDataPropertyOrThrow(O, "type", part.[[Type]]).
      //   c. Perform ! CreateDataPropertyOrThrow(O, "value", part.[[Value]]).
      O.merge(part);
      //   e. Perform ! CreateDataProperty(result, ! ToString(n), O).
      result.push_back(O);
      //   f. Increment n by 1.
    }
  }

  return result;
}

std::u16string DateTimeFormat::formatRange(
    double jsTimeValueFrom,
    double jsTimeValueTo) noexcept {
  return static_cast<DateTimeFormatApple *>(this)->formatRange(
      jsTimeValueFrom, jsTimeValueTo);
}

std::vector<Part> DateTimeFormat::formatRangeToParts(
    double jsTimeValueFrom,
    double jsTimeValueTo) noexcept {
  return static_cast<DateTimeFormatApple *>(this)->formatRangeToParts(
      jsTimeValueFrom, jsTimeValueTo);
}

class NumberFormatApple : public NumberFormat {
 public:
  NumberFormatApple() = default;

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

  Options resolvedOptions() noexcept;

  std::u16string format(double number) noexcept;

 private:
  // https://402.ecma-international.org/8.0/#sec-properties-of-intl-numberformat-instances
  // Intl.NumberFormat instances have an [[InitializedNumberFormat]] internal
  // slot.
  // NOTE: InitializedNumberFormat is not implemented.
  // [[Locale]] is a String value with the language tag of the locale whose
  // localization is used for formatting.
  std::u16string locale_;
  // [[DataLocale]] is a String value with the language tag of the nearest
  // locale for which the implementation has data to perform the formatting
  // operation. It will be a parent locale of [[Locale]].
  std::u16string dataLocale_;
  // [[NumberingSystem]] is a String value with the "type" given in Unicode
  // Technical Standard 35 for the numbering system used for formatting.
  // NOTE: Even though NSNumberFormatter formats numbers and time using
  // different numbering systems based on its "locale" value, it does not allow
  // to set/get the numbering system value explicitly. So we consider this
  // feature unsupported.
  // [[Style]] is one of the String values "decimal", "currency", "percent", or
  // "unit", identifying the type of quantity being measured.
  std::u16string style_;
  // [[Currency]] is a String value with the currency code identifying the
  // currency to be used if formatting with the "currency" unit type. It is only
  // used when [[Style]] has the value "currency".
  std::optional<std::u16string> currency_;
  // [[CurrencyDisplay]] is one of the String values "code", "symbol",
  // "narrowSymbol", or "name", specifying whether to display the currency as an
  // ISO 4217 alphabetic currency code, a localized currency symbol, or a
  // localized currency name if formatting with the "currency" style. It is only
  // used when [[Style]] has the value "currency".
  std::optional<std::u16string> currencyDisplay_;
  // [[CurrencySign]] is one of the String values "standard" or "accounting",
  // specifying whether to render negative numbers in accounting format, often
  // signified by parenthesis. It is only used when [[Style]] has the value
  // "currency" and when [[SignDisplay]] is not "never".
  std::optional<std::u16string> currencySign_;
  // [[Unit]] is a core unit identifier, as defined by Unicode Technical
  // Standard #35, Part 2, Section 6. It is only used when [[Style]] has the
  // value "unit".
  std::optional<std::u16string> unit_;
  // [[UnitDisplay]] is one of the String values "short", "narrow", or "long",
  // specifying whether to display the unit as a symbol, narrow symbol, or
  // localized long name if formatting with the "unit" style. It is only used
  // when [[Style]] has the value "unit".
  std::optional<std::u16string> unitDisplay_;
  // [[MinimumIntegerDigits]] is a non-negative integer Number value indicating
  // the minimum integer digits to be used. Numbers will be padded with leading
  // zeroes if necessary.
  uint8_t minimumIntegerDigits_;

  struct NumDigits {
    uint8_t minimum;
    uint8_t maximum;
  };
  // [[MinimumFractionDigits]] and [[MaximumFractionDigits]] are non-negative
  // integer Number values indicating the minimum and maximum fraction digits to
  // be used. Numbers will be rounded or padded with trailing zeroes if
  // necessary. These properties are only used when [[RoundingType]] is
  // fractionDigits.
  std::optional<NumDigits> fractionDigits_;
  // [[MinimumSignificantDigits]] and [[MaximumSignificantDigits]] are positive
  // integer Number values indicating the minimum and maximum fraction digits to
  // be shown. If present, the formatter uses however many fraction digits are
  // required to display the specified number of significant digits. These
  // properties are only used when [[RoundingType]] is significantDigits.
  std::optional<NumDigits> significantDigits_;
  // [[UseGrouping]] is a Boolean value indicating whether a grouping separator
  // should be used.
  bool useGrouping_;
  // [[RoundingType]] is one of the values fractionDigits, significantDigits, or
  // compactRounding, indicating which rounding strategy to use. If
  // fractionDigits, the number is rounded according to
  // [[MinimumFractionDigits]] and [[MaximumFractionDigits]], as described
  // above. If significantDigits, the number is rounded according to
  // [[MinimumSignificantDigits]] and [[MaximumSignificantDigits]] as described
  // above. If compactRounding, the number is rounded to 1 maximum fraction
  // digit if there is 1 digit before the decimal separator, and otherwise round
  // to 0 fraction digits.
  std::u16string roundingType_;
  // [[Notation]] is one of the String values "standard", "scientific",
  // "engineering", or "compact", specifying whether the number should be
  // displayed without scaling, scaled to the units place with the power of ten
  // in scientific notation, scaled to the nearest thousand with the power of
  // ten in scientific notation, or scaled to the nearest locale-dependent
  // compact decimal notation power of ten with the corresponding compact
  // decimal notation affix.
  std::u16string notation_;
  // [[CompactDisplay]] is one of the String values "short" or "long",
  // specifying whether to display compact notation affixes in short form ("5K")
  // or long form ("5 thousand") if formatting with the "compact" notation. It
  // is only used when [[Notation]] has the value "compact".
  std::optional<std::u16string> compactDisplay_;
  // [[SignDisplay]] is one of the String values "auto", "always", "never", or
  // "exceptZero", specifying whether to show the sign on negative numbers only,
  // positive and negative numbers including zero, neither positive nor negative
  // numbers, or positive and negative numbers but not zero. In scientific
  // notation, this slot affects the sign display of the mantissa but not the
  // exponent.
  std::u16string signDisplay_;
  // Finally, Intl.NumberFormat instances have a [[BoundFormat]] internal slot
  // that caches the function returned by the format accessor (15.4.3).
  // NOTE: BoundFormat is not implemented.
  NSNumberFormatter *nsNumberFormatter_;
  NSMeasurementFormatter *nsMeasurementFormatter_;
  NSUnit *nsUnit_;

  std::optional<std::u16string> numberingSystem_;

  vm::ExecutionStatus setNumberFormatUnitOptions(
      vm::Runtime &runtime,
      const Options &options) noexcept;
  vm::ExecutionStatus setNumberFormatDigitOptions(
      vm::Runtime &runtime,
      const Options &options,
      uint8_t mnfdDefault,
      uint8_t mxfdDefault,
      std::u16string_view notation) noexcept;
  void initializeNSFormatters() noexcept;
};

NumberFormat::NumberFormat() = default;
NumberFormat::~NumberFormat() = default;

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
  return supportedLocales(
      runtime, availableLocales, requestedLocales.getValue(), options);
}

// https://402.ecma-international.org/8.0/#sec-setnumberformatunitoptions
vm::ExecutionStatus NumberFormatApple::setNumberFormatUnitOptions(
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
  style_ = *styleOpt;
  //  5. Let currency be ? GetOption(options, "currency", "string", undefined,
  //  undefined).
  auto currencyRes = getOptionString(runtime, options, u"currency", {}, {});
  if (LLVM_UNLIKELY(currencyRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto currencyOpt = *currencyRes;
  //  6. If currency is undefined, then
  if (!currencyOpt) {
    //  a. If style is "currency", throw a TypeError exception.
    if (style_ == u"currency")
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
    if (style_ == u"unit")
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
  if (style_ == u"currency") {
    //  a. Let currency be the result of converting currency to upper case as
    //  specified in 6.1.
    //  b. Set intlObj.[[Currency]] to currency.
    currency_ = toASCIIUppercase(*currencyOpt);
    //  c. Set intlObj.[[CurrencyDisplay]] to currencyDisplay.
    currencyDisplay_ = *currencyDisplayOpt;
    //  d. Set intlObj.[[CurrencySign]] to currencySign.
    currencySign_ = *currencySignOpt;
  }
  //  15. If style is "unit", then
  if (style_ == u"unit") {
    //  a. Set intlObj.[[Unit]] to unit.
    //  b. Set intlObj.[[UnitDisplay]] to unitDisplay.
    unit_ = *unitOpt;
    unitDisplay_ = *unitDisplayOpt;
  }
  return vm::ExecutionStatus::RETURNED;
}

// https://402.ecma-international.org/8.0/#sec-setnfdigitoptions
vm::ExecutionStatus NumberFormatApple::setNumberFormatDigitOptions(
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
  minimumIntegerDigits_ = *mnidOpt;
  // 11. If mnsd is not undefined or mxsd is not undefined, then
  if (mnsdIt != options.end() || mxsdIt != options.end()) {
    // a. Set intlObj.[[RoundingType]] to significantDigits.
    roundingType_ = u"significantDigits";
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
    significantDigits_ = {*mnsdOpt, *mxsdOpt};
    // 12. Else if mnfd is not undefined or mxfd is not undefined, then
  } else if (mnfdIt != options.end() || mxfdIt != options.end()) {
    // a. Set intlObj.[[RoundingType]] to fractionDigits.
    roundingType_ = u"fractionDigits";
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
    fractionDigits_ = {*mnfdOpt, *mxfdOpt};
    // 13. Else if notation is "compact", then
  } else if (notation_ == u"compact") {
    // a. Set intlObj.[[RoundingType]] to compactRounding.
    roundingType_ = u"compactRounding";
    // 14. Else,
  } else {
    // a. Set intlObj.[[RoundingType]] to fractionDigits.
    roundingType_ = u"fractionDigits";
    // b. Set intlObj.[[MinimumFractionDigits]] to mnfdDefault.
    // c. Set intlObj.[[MaximumFractionDigits]] to mxfdDefault.
    fractionDigits_ = {mnfdDefault, mxfdDefault};
  }
  return vm::ExecutionStatus::RETURNED;
}

// https://402.ecma-international.org/8.0/#sec-initializenumberformat
vm::ExecutionStatus NumberFormatApple::initialize(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  const auto requestedLocales = canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 2. Set options to ? CoerceOptionsToObject(options).
  // 3. Let opt be a new Record.
  StringRecord opt;
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
  locale_ = r.locale;
  // 12. Set numberFormat.[[DataLocale]] to r.[[dataLocale]].
  dataLocale_ = r.dataLocale;
  // 13. Set numberFormat.[[NumberingSystem]] to r.[[nu]].
  numberingSystem_ = r.extensions.at(u"nu");

  // 14. Perform ? SetNumberFormatUnitOptions(numberFormat, options).
  auto setUnitOptionsRes = setNumberFormatUnitOptions(runtime, options);
  if (LLVM_UNLIKELY(setUnitOptionsRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 15. Let style be numberFormat.[[Style]].
  auto style = style_;
  uint8_t mnfdDefault, mxfdDefault;
  // 16. If style is "currency", then
  if (style == u"currency") {
    // a. Let currency be numberFormat.[[Currency]].
    // b. Let cDigits be CurrencyDigits(currency).
    auto cDigits = getCurrencyDigits(*currency_);
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
  notation_ = *notationOpt;
  // Perform ? SetNumberFormatDigitOptions(numberFormat, options, mnfdDefault,
  // mxfdDefault, notation).
  auto setDigitOptionsRes = setNumberFormatDigitOptions(
      runtime, options, mnfdDefault, mxfdDefault, notation_);
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
    compactDisplay_ = *compactDisplayOpt;
  // 23. Let useGrouping be ? GetOption(options, "useGrouping", "boolean",
  // undefined, true).
  auto useGroupingOpt = getOptionBool(runtime, options, u"useGrouping", true);
  // 24. Set numberFormat.[[UseGrouping]] to useGrouping.
  useGrouping_ = *useGroupingOpt;
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
  signDisplay_ = *signDisplayOpt;

  initializeNSFormatters();
  return vm::ExecutionStatus::RETURNED;
}

vm::CallResult<std::unique_ptr<NumberFormat>> NumberFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &inputOptions) noexcept {
  auto instance = std::make_unique<NumberFormatApple>();
  if (LLVM_UNLIKELY(
          instance->initialize(runtime, locales, inputOptions) ==
          vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return instance;
}

// https://402.ecma-international.org/8.0/#sec-intl.numberformat.prototype.resolvedoptions
Options NumberFormatApple::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", locale_);
  options.emplace(u"style", style_);
  if (currency_)
    options.emplace(u"currency", *currency_);
  if (currencyDisplay_)
    options.emplace(u"currencyDisplay", *currencyDisplay_);
  if (currencySign_)
    options.emplace(u"currencySign", *currencySign_);
  if (unit_)
    options.emplace(u"unit", *unit_);
  if (unitDisplay_)
    options.emplace(u"unitDisplay", *unitDisplay_);
  options.emplace(u"minimumIntegerDigits", (double)minimumIntegerDigits_);
  if (fractionDigits_) {
    options.emplace(u"minimumFractionDigits", (double)fractionDigits_->minimum);
    options.emplace(u"maximumFractionDigits", (double)fractionDigits_->maximum);
  }
  if (significantDigits_) {
    options.emplace(
        u"minimumSignificantDigits", (double)significantDigits_->minimum);
    options.emplace(
        u"maximumSignificantDigits", (double)significantDigits_->maximum);
  }
  options.emplace(u"useGrouping", useGrouping_);
  options.emplace(u"roundingType", roundingType_);
  options.emplace(u"notation", notation_);
  if (compactDisplay_)
    options.emplace(u"compactDisplay", *compactDisplay_);
  options.emplace(u"signDisplay", signDisplay_);
  return options;
}

Options NumberFormat::resolvedOptions() noexcept {
  return static_cast<NumberFormatApple *>(this)->resolvedOptions();
}

// https://402.ecma-international.org/8.0/#sec-formatnumber
void NumberFormatApple::initializeNSFormatters() noexcept {
  // NOTE: NSNumberFormatter has following limitations:
  // - "scientific" notation is supprted, "engineering" and "compact" are not.
  // - roundingType is not supported.
  // - compactDisplay is not supported.
  // - signDisplay is not supported.
  // - NSNumberFormatter has maximumIntegerDigits, which is 42 by default
  std::u16string nsLocaleStr = dataLocale_;
  nsNumberFormatter_ = [NSNumberFormatter new];
  if (style_ == u"decimal") {
    nsNumberFormatter_.numberStyle = NSNumberFormatterDecimalStyle;
    if (notation_ == u"scientific") {
      nsNumberFormatter_.numberStyle = NSNumberFormatterScientificStyle;
    }
  } else if (style_ == u"currency") {
    nsNumberFormatter_.numberStyle = NSNumberFormatterCurrencyStyle;
    nsLocaleStr.append(u"@currency=").append(*currency_);
    if (currencyDisplay_ == u"code") {
      nsNumberFormatter_.numberStyle = NSNumberFormatterCurrencyISOCodeStyle;
    } else if (currencyDisplay_ == u"symbol") {
      nsNumberFormatter_.numberStyle = NSNumberFormatterCurrencyStyle;
    } else if (currencyDisplay_ == u"narrowSymbol") {
      nsNumberFormatter_.numberStyle = NSNumberFormatterCurrencyStyle;
    } else if (currencyDisplay_ == u"name") {
      nsNumberFormatter_.numberStyle = NSNumberFormatterCurrencyPluralStyle;
    }
    if (signDisplay_ != u"never" && currencySign_ == u"accounting") {
      nsNumberFormatter_.numberStyle = NSNumberFormatterCurrencyAccountingStyle;
    }
  } else if (style_ == u"percent") {
    nsNumberFormatter_.numberStyle = NSNumberFormatterPercentStyle;
  } else if (style_ == u"unit") {
    nsNumberFormatter_.numberStyle = NSNumberFormatterNoStyle;
  }
  nsNumberFormatter_.minimumIntegerDigits = minimumIntegerDigits_;
  if (fractionDigits_) {
    nsNumberFormatter_.minimumFractionDigits = fractionDigits_->minimum;
    nsNumberFormatter_.maximumFractionDigits = fractionDigits_->maximum;
  }
  if (significantDigits_) {
    nsNumberFormatter_.minimumSignificantDigits = significantDigits_->minimum;
    nsNumberFormatter_.maximumSignificantDigits = significantDigits_->maximum;
  }
  nsNumberFormatter_.usesGroupingSeparator = useGrouping_;
  auto nsLocale =
      [NSLocale localeWithLocaleIdentifier:u16StringToNSString(nsLocaleStr)];
  nsNumberFormatter_.locale = nsLocale;
  if (style_ == u"unit") {
    nsMeasurementFormatter_ = [NSMeasurementFormatter new];
    nsMeasurementFormatter_.numberFormatter = nsNumberFormatter_;
    nsMeasurementFormatter_.locale = nsLocale;
    if (unitDisplay_ == u"short") {
      nsMeasurementFormatter_.unitStyle = NSFormattingUnitStyleShort;
    } else if (unitDisplay_ == u"narrow") {
      nsMeasurementFormatter_.unitStyle = NSFormattingUnitStyleMedium;
    } else if (unitDisplay_ == u"long") {
      nsMeasurementFormatter_.unitStyle = NSFormattingUnitStyleLong;
    }
    nsUnit_ = unitIdentifierToNSUnit(*unit_);
  }
}

std::u16string NumberFormatApple::format(double number) noexcept {
  if (nsMeasurementFormatter_) {
    assert(style_ == u"unit");
    auto m = [[NSMeasurement alloc] initWithDoubleValue:number unit:nsUnit_];
    return nsStringToU16String(
        [nsMeasurementFormatter_ stringFromMeasurement:m]);
  }
  return nsStringToU16String(
      [nsNumberFormatter_ stringFromNumber:[NSNumber numberWithDouble:number]]);
}

std::u16string NumberFormat::format(double number) noexcept {
  return static_cast<NumberFormatApple *>(this)->format(number);
}

std::vector<Part> NumberFormat::formatToParts(double number) noexcept {
  llvm_unreachable("formatToParts is unimplemented on Apple platforms");
}

} // namespace platform_intl
} // namespace hermes
