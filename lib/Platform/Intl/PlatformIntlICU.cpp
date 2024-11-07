/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/BCP47Parser.h"
#include "hermes/Platform/Intl/PlatformIntl.h"
#include "hermes/Platform/Intl/PlatformIntlShared.h"
#include "impl_icu/Collator.h"
#include "impl_icu/IntlUtils.h"
#include "impl_icu/LocaleBCP47Object.h"
#include "impl_icu/LocaleResolver.h"
#include "impl_icu/OptionHelpers.h"

#include <shared_mutex>
#include <string>
#include <unordered_map>

// ICU changed the default UChar type on version 59, but we still need to
// support 52+ However, ICU allows us to manually set a type for UChar using
// UCHAR_TYPE so UChar isn't platform dependent.
#define UCHAR_TYPE char16_t

#include "unicode/dtptngen.h"
#include "unicode/strenum.h"
#include "unicode/timezone.h"
#include "unicode/udat.h"
#include "unicode/unistr.h"

using namespace U_ICU_NAMESPACE;

namespace hermes {
namespace platform_intl {
namespace {

/// Thread safe management of time zone names map.
class TimeZoneNames {
 public:
  /// Initializing the underlying map with all known time zone names in
  /// ICU::TimeZone
  TimeZoneNames() {
    std::unique_ptr<StringEnumeration> icuTimeZones(
        TimeZone::createEnumeration());
    UErrorCode status = U_ZERO_ERROR;
    auto *zoneId = icuTimeZones->unext(nullptr, status);

    while (zoneId != nullptr && status == U_ZERO_ERROR) {
      auto upper = toASCIIUppercase(zoneId);
      timeZoneNamesMap_.emplace(std::move(upper), std::move(zoneId));
      zoneId = icuTimeZones->unext(nullptr, status);
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

static TimeZoneNames &validTimeZoneNames() {
  static TimeZoneNames validTimeZoneNames;
  return validTimeZoneNames;
}

/// https://402.ecma-international.org/8.0/#sec-isvalidtimezonename
static bool isValidTimeZoneName(std::u16string_view tz) {
  return validTimeZoneNames().contains(tz);
}

/// https://402.ecma-international.org/8.0/#sec-defaulttimezone
std::u16string getDefaultTimeZone(vm::Runtime &runtime) {
  std::unique_ptr<TimeZone> timeZone(TimeZone::createDefault());
  UnicodeString unicodeTz;
  timeZone->getID(unicodeTz);
  std::u16string tz(unicodeTz.getBuffer(), unicodeTz.length());
  validTimeZoneNames().update(tz);
  return tz;
}

/// https://402.ecma-international.org/8.0/#sec-canonicalizetimezonename
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

// T is a type of the intl service implementation classes.
// BaseT is the base platform_intl class that the intl service
// implementation class inherits from.
// Instance creation and initialization across the intl service
// implementation classes is the same. This function template
// provides the common code.
template <typename T, typename BaseT>
vm::CallResult<std::unique_ptr<BaseT>> createInstance(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  auto instance = std::make_unique<T>();
  if (LLVM_UNLIKELY(
          instance->initialize(runtime, locales, options) ==
          vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return std::move(instance);
}

} // namespace

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

Collator::Collator() = default;
Collator::~Collator() = default;

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return impl_icu::Collator::supportedLocalesOf(runtime, locales, options);
}

vm::CallResult<std::unique_ptr<Collator>> Collator::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return createInstance<impl_icu::Collator, Collator>(
      runtime, locales, options);
}

Options Collator::resolvedOptions() noexcept {
  return static_cast<impl_icu::Collator *>(this)->resolvedOptions();
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return static_cast<impl_icu::Collator *>(this)->compare(x, y);
}

namespace {
/// Implementation of
/// https://402.ecma-international.org/8.0/#datetimeformat-objects
struct DateTimeFormatICU : DateTimeFormat {
 public:
  DateTimeFormatICU() = default;
  ~DateTimeFormatICU() {
    udat_close(dateTimeFormatter_);
  };

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &inputOptions) noexcept;
  Options resolvedOptions() noexcept;

  std::u16string format(double jsTimeValue) noexcept;

 private:
  UDateFormat *getUDateFormatter(vm::Runtime &runtime);
  std::u16string getDefaultHourCycle();

  /// https://402.ecma-international.org/8.0/#sec-properties-of-intl-datetimeformat-instances
  /// Intl.DateTimeFormat instances have an [[InitializedDateTimeFormat]]
  /// internal slot.
  /// NOTE: InitializedDateTimeFormat is not implemented.
  /// Intl.DateTimeFormat instances also have several internal
  /// slots that are computed by the constructor:
  /// [[Locale]] is a String value with the language tag of the locale whose
  /// localization is used for formatting.
  std::u16string locale_;
  /// [[Calendar]] is a String value with the "type" given in Unicode Technical
  /// Standard 35 for the calendar used for formatting.
  std::optional<std::u16string> calendar_;
  /// [[NumberingSystem]] is a String value with the "type" given in Unicode
  /// Technical Standard 35 for the numbering system used for formatting.
  /// [[TimeZone]] is a String value with the IANA time zone name of the time
  /// zone used for formatting.
  std::u16string timeZone_;
  /// [[Weekday]], [[Era]], [[Year]], [[Month]], [[Day]], [[DayPeriod]],
  /// [[Hour]], [[Minute]], [[Second]], [[TimeZoneName]] are each either
  /// undefined, indicating that the component is not used for formatting, or
  /// one of the String values given in Table 4, indicating how the component
  /// should be presented in the formatted output.
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
  /// [[FractionalSecondDigits]] is either undefined or a positive, non-zero
  /// integer Number value indicating the fraction digits to be used for
  /// fractional seconds. Numbers will be rounded or padded with trailing zeroes
  /// if necessary.
  std::optional<uint8_t> fractionalSecondDigits_;
  /// [[HourCycle]] is a String value indicating whether the 12-hour format
  /// ("h11", "h12") or the 24-hour format ("h23", "h24") should be used. "h11"
  /// and "h23" start with hour 0 and go up to 11 and 23 respectively. "h12" and
  /// "h24" start with hour 1 and go up to 12 and 24. [[HourCycle]] is only used
  /// when [[Hour]] is not undefined.
  std::optional<std::u16string> hourCycle_;
  /// [[DateStyle]], [[TimeStyle]] are each either undefined, or a String value
  /// with values "full", "long", "medium", or "short".
  std::optional<std::u16string> dateStyle_;
  std::optional<std::u16string> timeStyle_;
  /// UTF-8 version of locale_. Used for ICU calls.
  std::string locale8_;
  /// Internal initialized ICU date formatter.
  UDateFormat *dateTimeFormatter_;
};
} // namespace

DateTimeFormat::DateTimeFormat() = default;
DateTimeFormat::~DateTimeFormat() = default;

/// https://402.ecma-international.org/8.0/#sec-intl.datetimeformat.supportedlocalesof
vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return impl_icu::supportedLocales(runtime, locales, options);
}

vm::ExecutionStatus DateTimeFormatICU::initialize(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &inputOptions) noexcept {
  // 1. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  auto requestedLocalesRes =
      impl_icu::LocaleBCP47Object::canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocalesRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 2. Let options be ? ToDateTimeOptions(options, "any", "date").
  auto optionsRes = toDateTimeOptions(runtime, inputOptions, u"any", u"date");
  if (LLVM_UNLIKELY(optionsRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  auto options = *optionsRes;
  // 3. Let opt be a new Record.
  Options opt;
  // 4. Let matcher be ? GetOption(options, "localeMatcher", "string",
  // «"lookup", "best fit" », "best fit").
  auto matcherRes = impl_icu::getStringOption(
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
  auto calendarRes =
      impl_icu::getStringOption(runtime, options, u"calendar", {}, {});
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
  auto hour12 = impl_icu::getBoolOption(options, u"hour12", {});
  // 13. Let hourCycle be ? GetOption(options, "hourCycle", "string", «
  // "h11", "h12", "h23", "h24" », undefined).
  auto hourCycleRes = impl_icu::getStringOption(
      runtime, options, u"hourCycle", {u"h11", u"h12", u"h23", u"h24"}, {});
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
  auto r = impl_icu::resolveLocale(
      *requestedLocalesRes,
      opt,
      relevantExtensionKeys,
      [](auto key, auto type, auto locale) { return true; });
  // 18. Set dateTimeFormat.[[Locale]] to r.[[locale]].
  locale_ = r.localeBcp47Object.getCanonicalizedLocaleId();

  // store the UTF8 version of locale since it is used in almost all other
  // functions
  locale8_ = impl_icu::toUTF8ASCII(locale_);

  // 19. Let calendar be r.[[ca]].
  auto caIt = r.resolvedOpts.find(u"ca");
  // 20. Set dateTimeFormat.[[Calendar]] to calendar.
  if (caIt != r.resolvedOpts.end())
    calendar_ = caIt->second.getString();
  // 21. Set dateTimeFormat.[[HourCycle]] to r.[[hc]].
  auto hcIt = r.resolvedOpts.find(u"hc");
  if (hcIt != r.resolvedOpts.end())
    hourCycle_ = hcIt->second.getString();
  // 22. Set dateTimeFormat.[[NumberingSystem]] to r.[[nu]].
  // 23. Let dataLocale be r.[[dataLocale]].
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
  auto dateStyleRes = impl_icu::getStringOption(
      runtime,
      options,
      u"dateStyle",
      {u"full", u"long", u"medium", u"short"},
      {});
  if (LLVM_UNLIKELY(dateStyleRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 33. Set dateTimeFormat.[[DateStyle]] to dateStyle.
  dateStyle_ = *dateStyleRes;
  // 34. Let timeStyle be ? GetOption(options, "timeStyle", "string", « "full",
  // "long", "medium", "short" », undefined).
  auto timeStyleRes = impl_icu::getStringOption(
      runtime,
      options,
      u"timeStyle",
      {u"full", u"long", u"medium", u"short"},
      {});
  if (LLVM_UNLIKELY(timeStyleRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  // 35. Set dateTimeFormat.[[TimeStyle]] to timeStyle.
  timeStyle_ = *timeStyleRes;

  // Initialize properties using values from the input options.
  auto weekdayRes = impl_icu::getStringOption(
      runtime, options, u"weekday", {u"narrow", u"short", u"long"}, {});
  if (LLVM_UNLIKELY(weekdayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  weekday_ = *weekdayRes;

  auto eraRes = impl_icu::getStringOption(
      runtime, options, u"era", {u"narrow", u"short", u"long"}, {});
  if (LLVM_UNLIKELY(eraRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  era_ = *eraRes;

  auto yearRes = impl_icu::getStringOption(
      runtime, options, u"year", {u"2-digit", u"numeric"}, {});
  if (LLVM_UNLIKELY(yearRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  year_ = *yearRes;

  auto monthRes = impl_icu::getStringOption(
      runtime,
      options,
      u"month",
      {u"2-digit", u"numeric", u"narrow", u"short", u"long"},
      {});
  if (LLVM_UNLIKELY(monthRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  month_ = *monthRes;

  auto dayRes = impl_icu::getStringOption(
      runtime, options, u"day", {u"2-digit", u"numeric"}, {});
  if (LLVM_UNLIKELY(dayRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  day_ = *dayRes;

  auto dayPeriodRes = impl_icu::getStringOption(
      runtime, options, u"dayPeriod", {u"narrow", u"short", u"long"}, {});
  if (LLVM_UNLIKELY(dayPeriodRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  dayPeriod_ = *dayPeriodRes;

  auto hourRes = impl_icu::getStringOption(
      runtime, options, u"hour", {u"2-digit", u"numeric"}, {});
  if (LLVM_UNLIKELY(hourRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  hour_ = *hourRes;

  auto minuteRes = impl_icu::getStringOption(
      runtime, options, u"minute", {u"2-digit", u"numeric"}, {});
  if (LLVM_UNLIKELY(minuteRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  minute_ = *minuteRes;

  auto secondRes = impl_icu::getStringOption(
      runtime, options, u"second", {u"2-digit", u"numeric"}, {});
  if (LLVM_UNLIKELY(secondRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  second_ = *secondRes;

  auto fractionalSecondDigitsRes = impl_icu::getNumberOption(
      runtime, options, u"fractionalSecondDigits", 1, 3, {});
  if (LLVM_UNLIKELY(
          fractionalSecondDigitsRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  fractionalSecondDigits_ = *fractionalSecondDigitsRes;

  // NOTE: "shortOffset", "longOffset", "shortGeneric", "longGeneric"
  // are specified here:
  // https://tc39.es/proposal-intl-extend-timezonename
  // they are not in ecma402 spec, but there is a test for them:
  // "test262/test/intl402/DateTimeFormat/constructor-options-timeZoneName-valid.js"
  auto timeZoneNameRes = impl_icu::getStringOption(
      runtime,
      options,
      u"timeZoneName",
      {u"short",
       u"long",
       u"shortOffset",
       u"longOffset",
       u"shortGeneric",
       u"longGeneric"},
      {});
  if (LLVM_UNLIKELY(timeZoneNameRes == vm::ExecutionStatus::EXCEPTION))
    return vm::ExecutionStatus::EXCEPTION;
  timeZoneName_ = *timeZoneNameRes;
  // NOTE: We don't have access to localeData, instead we'll defer to
  // ICU::Locale
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
    auto hcDefault = getDefaultHourCycle();
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

/// Gets the UDateFormat with options set in initialize
UDateFormat *DateTimeFormatICU::getUDateFormatter(vm::Runtime &runtime) {
  static constexpr std::u16string_view kLong = u"long", kShort = u"short",
                                       kNarrow = u"narrow",
                                       keMedium = u"medium", kFull = u"full",
                                       kNumeric = u"numeric",
                                       kTwoDigit = u"2-digit",
                                       kShortOffset = u"shortOffset",
                                       kLongOffset = u"longOffset",
                                       kShortGeneric = u"shortGeneric",
                                       kLongGeneric = u"longGeneric";

  // timeStyle and dateStyle cannot be used in conjunction with the other
  // options.
  if (timeStyle_.has_value() || dateStyle_.has_value()) {
    UDateFormatStyle dateStyleRes = UDAT_NONE;
    UDateFormatStyle timeStyleRes = UDAT_NONE;

    if (dateStyle_.has_value()) {
      if (dateStyle_ == kFull)
        dateStyleRes = UDAT_FULL;
      else if (dateStyle_ == kLong)
        dateStyleRes = UDAT_LONG;
      else if (dateStyle_ == keMedium)
        dateStyleRes = UDAT_MEDIUM;
      else if (dateStyle_ == kShort)
        dateStyleRes = UDAT_SHORT;
    }

    if (timeStyle_.has_value()) {
      if (timeStyle_ == kFull)
        timeStyleRes = UDAT_FULL;
      else if (timeStyle_ == kLong)
        timeStyleRes = UDAT_LONG;
      else if (timeStyle_ == keMedium)
        timeStyleRes = UDAT_MEDIUM;
      else if (timeStyle_ == kShort)
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
    if (weekday_ == kNarrow)
      skeleton += u"EEEEE";
    else if (weekday_ == kLong)
      skeleton += u"EEEE";
    else if (weekday_ == kShort)
      skeleton += u"EEE";
  }

  if (timeZoneName_.has_value()) {
    if (timeZoneName_ == kShort)
      skeleton += u"z";
    else if (timeZoneName_ == kLong)
      skeleton += u"zzzz";
    else if (timeZoneName_ == kShortOffset)
      skeleton += u"O";
    else if (timeZoneName_ == kLongOffset)
      skeleton += u"OOOO";
    else if (timeZoneName_ == kShortGeneric)
      skeleton += u"v";
    else if (timeZoneName_ == kLongGeneric)
      skeleton += u"vvvv";
  }

  if (era_.has_value()) {
    if (era_ == kNarrow)
      skeleton += u"GGGGG";
    else if (era_ == kShort)
      skeleton += u"GG";
    else if (era_ == kLong)
      skeleton += u"GGGG";
  }

  if (year_.has_value()) {
    if (year_ == kNumeric)
      skeleton += u"y";
    else if (year_ == kTwoDigit)
      skeleton += u"yy";
  }

  if (month_.has_value()) {
    if (month_ == kTwoDigit)
      skeleton += u"MM";
    else if (month_ == kNumeric)
      skeleton += u'M';
    else if (month_ == kNarrow)
      skeleton += u"MMMMM";
    else if (month_ == kShort)
      skeleton += u"MMM";
    else if (month_ == kLong)
      skeleton += u"MMMM";
  }

  if (day_.has_value()) {
    if (day_ == kNumeric)
      skeleton += u"d";
    else if (day_ == kTwoDigit)
      skeleton += u"dd";
  }

  if (hour_.has_value()) {
    if (hourCycle_ == u"h12") {
      if (hour_ == kNumeric)
        skeleton += u"h";
      else if (hour_ == kTwoDigit)
        skeleton += u"hh";
    } else if (hourCycle_ == u"h24") {
      if (hour_ == kNumeric)
        skeleton += u"k";
      else if (hour_ == kTwoDigit)
        skeleton += u"kk";
    } else if (hourCycle_ == u"h23") {
      if (hour_ == kNumeric)
        skeleton += u"H";
      else if (hour_ == kTwoDigit)
        skeleton += u"HH";
    } else {
      if (hour_ == kNumeric)
        skeleton += u"h";
      else if (hour_ == kTwoDigit)
        skeleton += u"HH";
    }
  }

  if (minute_.has_value()) {
    if (minute_ == kNumeric)
      skeleton += u"m";
    else if (minute_ == kTwoDigit)
      skeleton += u"mm";
  }

  if (second_.has_value()) {
    if (second_ == kNumeric)
      skeleton += u"s";
    else if (second_ == kTwoDigit)
      skeleton += u"ss";
  }

  UErrorCode status = U_ZERO_ERROR;
  std::u16string bestpattern;
  int32_t patternLength;

  std::unique_ptr<UDateTimePatternGenerator, decltype(&udatpg_close)>
      dtpGenerator(udatpg_open(&locale8_[0], &status), &udatpg_close);
  patternLength = udatpg_getBestPatternWithOptions(
      dtpGenerator.get(),
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
        dtpGenerator.get(),
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

std::u16string DateTimeFormatICU::getDefaultHourCycle() {
  UErrorCode status = U_ZERO_ERROR;
  std::u16string myString;
  // open the default UDateFormat and Pattern of locale
  UDateFormat *defaultDTF = udat_open(
      UDAT_DEFAULT,
      UDAT_DEFAULT,
      &locale8_[0],
      nullptr,
      -1,
      nullptr,
      -1,
      &status);
  int32_t size = udat_toPattern(defaultDTF, true, nullptr, 0, &status);
  if (status == U_BUFFER_OVERFLOW_ERROR) {
    status = U_ZERO_ERROR;
    myString.resize(size + 1);
    udat_toPattern(defaultDTF, true, &myString[0], 40, &status);
    assert(status <= 0); // Check for errors
    udat_close(defaultDTF);
    // find the default hour cycle and return it
    for (int32_t i = 0; i < size; i++) {
      char16_t ch = myString[i];
      switch (ch) {
        case 'K':
          return u"h11";
          break;
        case 'h':
          return u"h12";
          break;
        case 'H':
          return u"h23";
          break;
        case 'k':
          return u"h24";
          break;
      }
    }
  }

  return u"h24";
}

vm::CallResult<std::unique_ptr<DateTimeFormat>> DateTimeFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &inputOptions) noexcept {
  auto instance = std::make_unique<DateTimeFormatICU>();
  if (LLVM_UNLIKELY(
          instance->initialize(runtime, locales, inputOptions) ==
          vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return instance;
}

/// Implementer note: This method corresponds roughly to
/// https://402.ecma-international.org/8.0/#sec-intl.datetimeformat.prototype.resolvedoptions
Options DateTimeFormatICU::resolvedOptions() noexcept {
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
  return static_cast<DateTimeFormatICU *>(this)->resolvedOptions();
}

std::u16string DateTimeFormatICU::format(double jsTimeValue) noexcept {
  auto timeInSeconds = jsTimeValue;
  UDate date = UDate(timeInSeconds);
  UErrorCode status = U_ZERO_ERROR;
  std::u16string formattedDate;
  int32_t myStrlen = 0;

  myStrlen = udat_format(
      dateTimeFormatter_, date, nullptr, myStrlen, nullptr, &status);
  if (status == U_BUFFER_OVERFLOW_ERROR) {
    status = U_ZERO_ERROR;
    formattedDate.resize(myStrlen);
    udat_format(
        dateTimeFormatter_,
        date,
        &formattedDate[0],
        myStrlen,
        nullptr,
        &status);
  }

  assert(status <= 0); // Check for errors
  return formattedDate;
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  return static_cast<DateTimeFormatICU *>(this)->format(jsTimeValue);
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
