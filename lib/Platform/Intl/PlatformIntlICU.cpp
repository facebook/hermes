/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/BCP47Parser.h"
#include "hermes/Platform/Intl/PlatformIntl.h"
#include "hermes/Platform/Intl/PlatformIntlShared.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include "unicode/strenum.h"
#include "unicode/timezone.h"
#include "unicode/unistr.h"
#include "unicode/udat.h"
#include "unicode/dtptngen.h"
#include "llvh/Support/ConvertUTF.h"

using namespace ::facebook;
using namespace ::hermes;
using namespace U_ICU_NAMESPACE;

namespace hermes {
namespace platform_intl {
namespace {
vm::CallResult<std::u16string> UTF8toUTF16(vm::Runtime &runtime, std::string_view in) {
  std::u16string out;
  size_t length = in.length();
  out.resize(length);
  const llvh::UTF8 *sourceStart = reinterpret_cast<const llvh::UTF8 *>(&in[0]);
  const llvh::UTF8 *sourceEnd = sourceStart + length;
  llvh::UTF16 *targetStart = reinterpret_cast<llvh::UTF16 *>(&out[0]);
  llvh::UTF16 *targetEnd = targetStart + out.size();
  llvh::ConversionResult convRes = ConvertUTF8toUTF16(
      &sourceStart,
      sourceEnd,
      &targetStart,
      targetEnd,
      llvh::lenientConversion);
  if (convRes != llvh::ConversionResult::conversionOK) {
    return runtime.raiseRangeError("utf8 to utf16 conversion failed");
  }
  out.resize(reinterpret_cast<char16_t *>(targetStart) - &out[0]);
  return out;
}

vm::CallResult<std::string> UTF16toUTF8(
    vm::Runtime &runtime,
    std::u16string in) {
  std::string out;
  size_t length = in.length();
  out.resize(length);
  const llvh::UTF16 *sourceStart =
      reinterpret_cast<const llvh::UTF16 *>(&in[0]);
  const llvh::UTF16 *sourceEnd = sourceStart + length;
  llvh::UTF8 *targetStart = reinterpret_cast<llvh::UTF8 *>(&out[0]);
  llvh::UTF8 *targetEnd = targetStart + out.size();
  llvh::ConversionResult convRes = ConvertUTF16toUTF8(
      &sourceStart,
      sourceEnd,
      &targetStart,
      targetEnd,
      llvh::lenientConversion);
  if (convRes != llvh::ConversionResult::conversionOK) {
    return runtime.raiseRangeError("utf16 to utf8 conversion failed");
  }
  out.resize(reinterpret_cast<char *>(targetStart) - &out[0]);
  return out;
}

const std::vector<std::u16string> &getAvailableLocales(vm::Runtime &runtime) {
  static const std::vector<std::u16string> *availableLocales = [&runtime] {
    auto *vec = new std::vector<std::u16string>();

    for (int32_t i = 0, count = uloc_countAvailable(); i < count; i++) {
      auto locale = uloc_getAvailable(i);
      vec->push_back(UTF8toUTF16(runtime, locale).getValue());
    }

    return vec;
  }();

  return *availableLocales;
}

/// https://402.ecma-international.org/8.0/#sec-lookupmatcher
LocaleMatch lookupMatcher(
    vm::Runtime &runtime,
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
  result.locale = UTF8toUTF16(runtime, uloc_getDefault()).getValue();
  // 5. Return result.
  return result;
}

/// https://402.ecma-international.org/8.0/#sec-resolvelocale
ResolvedLocale resolveLocale(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &availableLocales,
    const std::vector<std::u16string> &requestedLocales,
    const std::unordered_map<std::u16string, std::u16string> &options,
    llvh::ArrayRef<std::u16string_view> relevantExtensionKeys) {
  // 1. Let matcher be options.[[localeMatcher]].
  // 2. If matcher is "lookup", then
  // a. Let r be LookupMatcher(availableLocales, requestedLocales).
  // 3. Else,
  // a. Let r be BestFitMatcher(availableLocales, requestedLocales).
  auto r = lookupMatcher(runtime, availableLocales, requestedLocales);
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
} // namespace

namespace {
/// Thread safe management of time zone names map.
class TimeZoneNames {
 public:
  /// Initializing the underlying map with all known time zone names in
  /// NSTimeZone.
  TimeZoneNames() {
    StringEnumeration *icuTimeZones = TimeZone::createEnumeration();
    UErrorCode status = U_ZERO_ERROR;
    int32_t *resultLength;
    auto *zoneId = icuTimeZones->unext(resultLength, status);

    while (zoneId != NULL && status == U_ZERO_ERROR) {
      auto upper = toASCIIUppercase(zoneId);
      timeZoneNamesMap_.emplace(std::move(upper), std::move(zoneId));
    }
  }

  /// Check if \p tz is a valid time zone name.
  bool contains(std::u16string_view tz) const {
    std::shared_lock lock(mutex_);
    return timeZoneNamesMap_.find(toASCIIUppercase(tz)) !=
        timeZoneNamesMap_.end();
  }

  /// Get canonical time zone name for \p tz. Note that \p tz must
  /// be a valid key in the map.
  std::u16string getCanonical(std::u16string_view tz) const {
    std::shared_lock lock(mutex_);
    auto ianaTimeZoneIt = timeZoneNamesMap_.find(toASCIIUppercase(tz));
    assert(
        ianaTimeZoneIt != timeZoneNamesMap_.end() &&
        "getCanonical() must be called on valid time zone name.");
    return ianaTimeZoneIt->second;
  }

  /// Update the time zone name map with \p tz if it does not exist yet.
  void update(std::u16string_view tz) {
    auto upper = toASCIIUppercase(tz);
    // Read lock and check if tz is already in the map.
    {
      std::shared_lock lock(mutex_);
      if (timeZoneNamesMap_.find(upper) != timeZoneNamesMap_.end()) {
        return;
      }
    }
    // If not, write lock and insert it into the map.
    {
      std::unique_lock lock(mutex_);
      timeZoneNamesMap_.emplace(upper, tz);
    }
  }

 private:
  /// Map from upper case time zone name to canonical time zone name.
  std::unordered_map<std::u16string, std::u16string> timeZoneNamesMap_;
  mutable std::shared_mutex mutex_;
};
} // namespace

static TimeZoneNames &validTimeZoneNames() {
  static TimeZoneNames validTimeZoneNames;
  return validTimeZoneNames;
}

// https://402.ecma-international.org/8.0/#sec-isvalidtimezonename
static bool isValidTimeZoneName(std::u16string_view tz) {
  return validTimeZoneNames().contains(tz);
}

// https://402.ecma-international.org/8.0/#sec-defaulttimezone
std::u16string getDefaultTimeZone(vm::Runtime &runtime) {
  auto *timeZone = TimeZone::createDefault();
  UnicodeString unicodeTz;
  timeZone->getID(unicodeTz);
  std::string timeZoneId;
  unicodeTz.toUTF8String(timeZoneId);


  std::u16string tz = UTF8toUTF16(runtime, timeZoneId).getValue();

  validTimeZoneNames().update(tz);
  return tz;
}

vm::CallResult<std::u16string> getDefaultHourCycle(vm::Runtime &runtime) {
  Locale locale;
  auto status = U_ZERO_ERROR;
  DateTimePatternGenerator *generator = DateTimePatternGenerator::createInstance(locale, status);

  if (U_FAILURE(status)) {
    return runtime.raiseRangeError("failed to get default hour cycle.");
  }

  auto dateFormatPattern = generator->getDefaultHourCycle(status);
  if (U_FAILURE(status)) {
    return runtime.raiseRangeError("failed to get default hour cycle.");
  }


  if (dateFormatPattern == UDAT_HOUR_CYCLE_11) {
    return u"h11";
  } else if (dateFormatPattern == UDAT_HOUR_CYCLE_12) {
    return u"h12";
  } else if (dateFormatPattern == UDAT_HOUR_CYCLE_23) {
    return u"h23";
  }

  return u"h24";
}

// https://402.ecma-international.org/8.0/#sec-canonicalizetimezonename
std::u16string canonicalizeTimeZoneName(std::u16string_view tz) {
  // 1. Let ianaTimeZone be the Zone or Link name of the IANA Time Zone Database
  // such that timeZone, converted to upper case as described in 6.1, is equal
  // to ianaTimeZone, converted to upper case as described in 6.1.
  auto ianaTimeZone = validTimeZoneNames().getCanonical(tz);
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

// https://tc39.es/ecma402/#sec-intl.getcanonicallocales
vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales) {
  return canonicalizeLocaleList(runtime, locales);
}

// Not yet implemented.
vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"lowered");
}
// Not yet implemented.
vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"uppered");
}

namespace {
struct CollatorDummy : Collator {
  CollatorDummy(const char16_t *l) : locale(l) {}
  std::u16string locale;
};
} // namespace

Collator::Collator() = default;
Collator::~Collator() = default;

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::CallResult<std::unique_ptr<Collator>> Collator::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::make_unique<CollatorDummy>(u"en-US");
}

Options Collator::resolvedOptions() noexcept {
  Options options;
  options.emplace(
      u"locale", Option(static_cast<CollatorDummy *>(this)->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return x.compare(y);
}

namespace {
// Implementation of
// https://402.ecma-international.org/8.0/#datetimeformat-objects
struct DateTimeFormatICU : DateTimeFormat {
 public:
  DateTimeFormatICU(const char16_t *l) : locale(l) {}
  std::u16string locale;

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &inputOptions) noexcept;
 private:
  UDateFormat *getUDateFormatter(vm::Runtime &runtime);

  // https://402.ecma-international.org/8.0/#sec-properties-of-intl-datetimeformat-instances
  // Intl.DateTimeFormat instances have an [[InitializedDateTimeFormat]]
  // internal slot.
  // NOTE: InitializedDateTimeFormat is not implemented.
  // Intl.DateTimeFormat instances also have several internal
  // slots that are computed by the constructor:
  // [[Locale]] is a String value with the language tag of the locale whose
  // localization is used for formatting.
  std::u16string locale_;
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
  std::string locale8_;
  UDateFormat *dateTimeFormatter_;
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
  const std::vector<std::u16string> &availableLocales = getAvailableLocales(runtime);
  // 3. Return ? (availableLocales, requestedLocales, options).
  return supportedLocales(availableLocales, requestedLocales.getValue());
}

vm::ExecutionStatus DateTimeFormatICU::initialize(
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
  // will use ICU's default locale as a substitute
  // 17. Let r be ResolveLocale(%DateTimeFormat%.[[AvailableLocales]],
  // requestedLocales, opt, %DateTimeFormat%.[[RelevantExtensionKeys]],
  // localeData).
  static constexpr std::u16string_view relevantExtensionKeys[] = {
      u"ca", u"nu", u"hc"};
  auto r = resolveLocale(
      runtime, getAvailableLocales(runtime), *requestedLocalesRes, opt, relevantExtensionKeys);
  // 18. Set dateTimeFormat.[[Locale]] to r.[[locale]].
  locale_ = std::move(r.locale);

  auto conversion = UTF16toUTF8(runtime, locale_);
  if (conversion.getStatus() == vm::ExecutionStatus::EXCEPTION) {
    return conversion.getStatus();
  }
  locale8_ = conversion.getValue(); // store the UTF8 version of locale since it
                                    // is used in almost all other functions
  // 19. Let calendar be r.[[ca]].
  auto caIt = r.extensions.find(u"ca");
  // 20. Set dateTimeFormat.[[Calendar]] to calendar.
  if (caIt != r.extensions.end())
    calendar_ = std::move(caIt->second);
  // 21. Set dateTimeFormat.[[HourCycle]] to r.[[hc]].
  auto hcIt = r.extensions.find(u"hc");
  if (hcIt != r.extensions.end())
    hourCycle_ = std::move(hcIt->second);
  // 22. Set dateTimeFormat.[[NumberingSystem]] to r.[[nu]].
  // 23. Let dataLocale be r.[[dataLocale]].
  auto dataLocale = r.dataLocale;
  // 24. Let timeZone be ? Get(options, "timeZone").
  auto timeZoneIt = options.find(u"timeZone");
  std::u16string timeZone;
  //  25. If timeZone is undefined, then
  if (timeZoneIt == options.end()) {
    // a. Let timeZone be DefaultTimeZone().
    timeZone = getDefaultTimeZone(runtime);
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
  timeZone_ = timeZone;
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
  dateStyle_ = *dateStyleRes;
  // 34. Let timeStyle be ? GetOption(options, "timeStyle", "string", « "full",
  // "long", "medium", "short" », undefined).
  static constexpr std::u16string_view timeStyles[] = {
      u"full", u"long", u"medium", u"short"};
  auto timeStyleRes =
      getOptionString(runtime, options, u"timeStyle", timeStyles, {});
  if (LLVM_UNLIKELY(timeStyleRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 35. Set dateTimeFormat.[[TimeStyle]] to timeStyle.
  timeStyle_ = *timeStyleRes;

  // Initialize properties using values from the input options.
  static constexpr std::u16string_view weekdayValues[] = {
      u"narrow", u"short", u"long"};
  auto weekdayRes =
      getOptionString(runtime, options, u"weekday", weekdayValues, {});
  if (LLVM_UNLIKELY(weekdayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  weekday_ = *weekdayRes;

  static constexpr std::u16string_view eraValues[] = {
      u"narrow", u"short", u"long"};
  auto eraRes = getOptionString(runtime, options, u"era", eraValues, {});
  if (LLVM_UNLIKELY(eraRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  era_ = *eraRes;

  static constexpr std::u16string_view yearValues[] = {u"2-digit", u"numeric"};
  auto yearRes = getOptionString(runtime, options, u"year", yearValues, {});
  if (LLVM_UNLIKELY(yearRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  year_ = *yearRes;

  static constexpr std::u16string_view monthValues[] = {
      u"2-digit", u"numeric", u"narrow", u"short", u"long"};
  auto monthRes = getOptionString(runtime, options, u"month", monthValues, {});
  if (LLVM_UNLIKELY(monthRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  month_ = *monthRes;

  static constexpr std::u16string_view dayValues[] = {u"2-digit", u"numeric"};
  auto dayRes = getOptionString(runtime, options, u"day", dayValues, {});
  if (LLVM_UNLIKELY(dayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  day_ = *dayRes;

  static constexpr std::u16string_view dayPeriodValues[] = {
      u"narrow", u"short", u"long"};
  auto dayPeriodRes =
      getOptionString(runtime, options, u"dayPeriod", dayPeriodValues, {});
  if (LLVM_UNLIKELY(dayPeriodRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  dayPeriod_ = *dayPeriodRes;

  static constexpr std::u16string_view hourValues[] = {u"2-digit", u"numeric"};
  auto hourRes = getOptionString(runtime, options, u"hour", hourValues, {});
  if (LLVM_UNLIKELY(hourRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  hour_ = *hourRes;

  static constexpr std::u16string_view minuteValues[] = {
      u"2-digit", u"numeric"};
  auto minuteRes =
      getOptionString(runtime, options, u"minute", minuteValues, {});
  if (LLVM_UNLIKELY(minuteRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  minute_ = *minuteRes;

  static constexpr std::u16string_view secondValues[] = {
      u"2-digit", u"numeric"};
  auto secondRes =
      getOptionString(runtime, options, u"second", secondValues, {});
  if (LLVM_UNLIKELY(secondRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  second_ = *secondRes;

  auto fractionalSecondDigitsRes =
      getNumberOption(runtime, options, u"fractionalSecondDigits", 1, 3, {});
  if (LLVM_UNLIKELY(
          fractionalSecondDigitsRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  fractionalSecondDigits_ = *fractionalSecondDigitsRes;

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
      runtime, options, u"timeZoneName", timeZoneNameValues, {});
  if (LLVM_UNLIKELY(timeZoneNameRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  timeZoneName_ = *timeZoneNameRes;
  // NOTE: We don't have access to localeData, instead we'll defer to ICU::Locale
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
  if (!hour_.has_value()) {
    // a. Set dateTimeFormat.[[HourCycle]] to undefined.
    hourCycle_ = std::nullopt;
    // b. Let pattern be bestFormat.[[pattern]].
    // c. Let rangePatterns be bestFormat.[[rangePatterns]].
    // 40. Else,
  } else {
    // a. Let hcDefault be dataLocaleData.[[hourCycle]].
    auto hcDefault = getDefaultHourCycle(runtime).getValue();
    // b. Let hc be dateTimeFormat.[[HourCycle]].
    auto hc = hourCycle_;
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
    hourCycle_ = hc;
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
  dateTimeFormatter_ = getUDateFormatter(runtime);
  return vm::ExecutionStatus::RETURNED;
}

// gets the UDateFormat with options set in initialize
UDateFormat *DateTimeFormatICU::getUDateFormatter(vm::Runtime &runtime) {
  static std::u16string eLong = u"long", eShort = u"short", eNarrow = u"narrow",
                        eMedium = u"medium", eFull = u"full",
                        eNumeric = u"numeric", eTwoDigit = u"2-digit",
                        eShortOffset = u"shortOffset",
                        eLongOffset = u"longOffset",
                        eShortGeneric = u"shortGeneric",
                        eLongGeneric = u"longGeneric";

  // timeStyle and dateStyle cannot be used in conjunction with the other
  // options.
  if (timeStyle_.has_value() || dateStyle_.has_value()) {
    UDateFormatStyle dateStyleRes = UDAT_DEFAULT;
    UDateFormatStyle timeStyleRes = UDAT_DEFAULT;

    if (dateStyle_.has_value()) {
      if (dateStyle_ == eFull)
        dateStyleRes = UDAT_FULL;
      else if (dateStyle_ == eLong)
        dateStyleRes = UDAT_LONG;
      else if (dateStyle_ == eMedium)
        dateStyleRes = UDAT_MEDIUM;
      else if (dateStyle_ == eShort)
        dateStyleRes = UDAT_SHORT;
    }

    if (timeStyle_.has_value()) {
      if (timeStyle_ == eFull)
        timeStyleRes = UDAT_FULL;
      else if (timeStyle_ == eLong)
        timeStyleRes = UDAT_LONG;
      else if (timeStyle_ == eMedium)
        timeStyleRes = UDAT_MEDIUM;
      else if (timeStyle_ == eShort)
        timeStyleRes = UDAT_SHORT;
    }

    UErrorCode status = U_ZERO_ERROR;
    UDateFormat *dtf;
    // if timezone is specified, use that instead, else use default
    if (!timeZone_.empty()) {
      const UChar *timeZoneRes =
          reinterpret_cast<const UChar *>(timeZone_.c_str());
      int32_t timeZoneLength = timeZone_.length();
      dtf = udat_open(
          timeStyleRes,
          dateStyleRes,
          &locale8_[0],
          timeZoneRes,
          timeZoneLength,
          nullptr,
          -1,
          &status);
    } else {
      dtf = udat_open(
          timeStyleRes,
          dateStyleRes,
          &locale8_[0],
          nullptr,
          -1,
          nullptr,
          -1,
          &status);
    }
    assert(status == U_ZERO_ERROR);
    return dtf;
  }

  // Else: lets create the skeleton
  std::u16string skeleton = u"";
  if (weekday_.has_value()) {
    if (weekday_ == eNarrow)
      skeleton += u"EEEEE";
    else if (weekday_ == eLong)
      skeleton += u"EEEE";
    else if (weekday_ == eShort)
      skeleton += u"EEE";
  }

  if (timeZoneName_.has_value()) {
    if (timeZoneName_ == eShort)
      skeleton += u"z";
    else if (timeZoneName_ == eLong)
      skeleton += u"zzzz";
    else if (timeZoneName_ == eShortOffset)
      skeleton += u"O";
    else if (timeZoneName_ == eLongOffset)
      skeleton += u"OOOO";
    else if (timeZoneName_ == eShortGeneric)
      skeleton += u"v";
    else if (timeZoneName_ == eLongGeneric)
      skeleton += u"vvvv";
  }

  if (era_.has_value()) {
    if (era_ == eNarrow)
      skeleton += u"GGGGG";
    else if (era_ == eShort)
      skeleton += u"G";
    else if (era_ == eLong)
      skeleton += u"GGGG";
  }

  if (year_.has_value()) {
    if (year_ == eNumeric)
      skeleton += u"y";
    else if (year_ == eTwoDigit)
      skeleton += u"yy";
  }

  if (month_.has_value()) {
    if (month_ == eTwoDigit)
      skeleton += u"MM";
    else if (month_ == eNumeric)
      skeleton += u'M';
    else if (month_ == eNarrow)
      skeleton += u"MMMMM";
    else if (month_ == eShort)
      skeleton += u"MMM";
    else if (month_ == eLong)
      skeleton += u"MMMM";
  }

  if (day_.has_value()) {
    if (day_ == eNumeric)
      skeleton += u"d";
    else if (day_ == eTwoDigit)
      skeleton += u"dd";
  }

  if (hour_.has_value()) {
    if (hourCycle_ == u"h12") {
      if (hour_ == eNumeric)
        skeleton += u"h";
      else if (hour_ == eTwoDigit)
        skeleton += u"hh";
    } else if (hourCycle_ == u"h24") {
      if (hour_ == eNumeric)
        skeleton += u"k";
      else if (hour_ == eTwoDigit)
        skeleton += u"kk";
    } else if (hourCycle_ == u"h23") {
      if (hour_ == eNumeric)
        skeleton += u"k";
      else if (hour_ == eTwoDigit)
        skeleton += u"KK";
    } else {
      if (hour_ == eNumeric)
        skeleton += u"h";
      else if (hour_ == eTwoDigit)
        skeleton += u"HH";
    }
  }

  if (minute_.has_value()) {
    if (minute_ == eNumeric)
      skeleton += u"m";
    else if (minute_ == eTwoDigit)
      skeleton += u"mm";
  }

  if (second_.has_value()) {
    if (second_ == eNumeric)
      skeleton += u"s";
    else if (second_ == eTwoDigit)
      skeleton += u"ss";
  }

  UErrorCode status = U_ZERO_ERROR;
  std::u16string bestpattern;
  int32_t patternLength;

  UDateTimePatternGenerator *dtpGenerator = udatpg_open(&locale8_[0], &status);
  patternLength = udatpg_getBestPatternWithOptions(
      dtpGenerator,
      &skeleton[0],
      -1,
      UDATPG_MATCH_ALL_FIELDS_LENGTH,
      nullptr,
      0,
      &status);

  if (status == U_BUFFER_OVERFLOW_ERROR) {
    status = U_ZERO_ERROR;
    bestpattern.resize(patternLength);
    udatpg_getBestPatternWithOptions(
        dtpGenerator,
        &skeleton[0],
        skeleton.length(),
        UDATPG_MATCH_ALL_FIELDS_LENGTH,
        &bestpattern[0],
        patternLength,
        &status);
  }

  // if timezone is specified, use that instead, else use default
  if (!timeZone_.empty()) {
    const UChar *timeZoneRes =
        reinterpret_cast<const UChar *>(timeZone_.c_str());
    int32_t timeZoneLength = timeZone_.length();
    return udat_open(
        UDAT_PATTERN,
        UDAT_PATTERN,
        &locale8_[0],
        timeZoneRes,
        timeZoneLength,
        &bestpattern[0],
        patternLength,
        &status);
  } else {
    return udat_open(
        UDAT_PATTERN,
        UDAT_PATTERN,
        &locale8_[0],
        nullptr,
        -1,
        &bestpattern[0],
        patternLength,
        &status);
  }
}

vm::CallResult<std::unique_ptr<DateTimeFormat>> DateTimeFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::make_unique<DateTimeFormatICU>(u"en-US");
}

Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(
      u"locale", Option(static_cast<DateTimeFormatICU *>(this)->locale));
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

namespace {
struct NumberFormatDummy : NumberFormat {
  NumberFormatDummy(const char16_t *l) : locale(l) {}
  std::u16string locale;
};
} // namespace

NumberFormat::NumberFormat() = default;
NumberFormat::~NumberFormat() = default;

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::CallResult<std::unique_ptr<NumberFormat>> NumberFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::make_unique<NumberFormatDummy>(u"en-US");
}

Options NumberFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(
      u"locale", Option(static_cast<NumberFormatDummy *>(this)->locale));
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
