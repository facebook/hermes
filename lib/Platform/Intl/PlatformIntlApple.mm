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
  result.reserve(size);
  for (size_t i = 0; i < size; i++) {
    result[i] = nsStringToU16String(array[i]);
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
// Funcion split into two depending on which data type is needed
vm::CallResult<Option> getOptionBool(
    const Options &options,
    const std::u16string property,
    const std::u16string type,
    const std::vector<std::u16string> values,
    const Option &fallback) {
  // 1. Assert type(options) is object
  // 2. Let value be ? Get(options, property).
  auto value = options.find(property);
  // 3. If value is undefined, return fallback.
  if (value == options.end()) {
    return fallback;
  }
  // 4. Assert: type is "boolean" or "string".
  // 5. If type is "boolean", then
  if (type == u"boolean") {
    if(!value->second.isBool()) {
      // TODO: in what layer do we call into runtime to throw exceptions
      //return runtime->raiseRangeError(u"Boolean option expected but not found");
      return vm::ExecutionStatus::EXCEPTION;
    }
  }
  // 7. If values is not undefined and values does not contain an element equal to value, throw a RangeError exception.
  if (!values.empty() && llvh::find(values, value->second) == values.end()) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  // 8. Return value.
  return value->second;
}
vm::CallResult<Option> getOptionString(
     const Options &options,
     const std::u16string property,
     const std::u16string type,
     const std::vector<std::u16string> values,
     const Option &fallback) {
  // 1. Assert type(options) is object
  // 2. Let value be ? Get(options, property).
  auto value = options.find(property);
  // 3. If value is undefined, return fallback.
  if (value == options.end()) {
    return fallback;
  }
  if (type == u"string") {
    if(!value->second.isString()) {
      //return runtime->raiseRangeError(u"String option expected but not found");
      return vm::ExecutionStatus::EXCEPTION;
    }
    // 7. If values is not undefined and values does not contain an element equal to value, throw a RangeError exception.
    if (!values.empty() && llvh::find(values, value->second) == values.end()) {
      return vm::ExecutionStatus::EXCEPTION;
    }
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
    const Option &fallback) {
//  1. Assert: Type(options) is Object.
//  2. Let value be ? Get(options, property).
  auto value = options.find(property);
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
    //std::u16string noExtensionsLocale = toNoExtensionsLocale(locale);
    // b. Let availableLocale be BestAvailableLocale(availableLocales, noExtensionsLocale).
    //std::u16string availableLocale = bestAvailableLocale(availableLocales, noExtensionsLocale);
    // c. If availableLocale is not undefined, append locale to the end of subset.
    // if (!availableLocale.empty()) { subset.push_back(locale); }
  }

  // 3. Return subset.
  return subset;
}

// Implementation of
// https://tc39.es/ecma402/#sec-supportedlocales
vm::CallResult<std::vector<std::u16string>> supportedLocales(
    const std::vector<std::u16string> availableLocales,
    const std::vector<std::u16string> requestedLocales,
    const Options &options) {
  // 1. Set options to ? CoerceOptionsToObject(options).
  // 2. Let matcher be ? GetOption(options, "localeMatcher", "string", « "lookup", "best fit" », "best fit").
  std::vector<std::u16string> vectorForMatcher = {u"lookup", u"best fit"};
  vm::CallResult<std::u16string> matcher = getOptionString(options, u"localeMatcher", u"string", vectorForMatcher, u"best fit").getValue().getString();
  if (LLVM_UNLIKELY(matcher == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  
  std::vector<std::u16string> supportedLocales;
  // 3. If matcher is "best fit", then
  if (matcher.getValue() == u"best fit") {
    // a. Let supportedLocales be BestFitSupportedLocales(availableLocales, requestedLocales).
    // TODO: Is bestFitSupportedLocales required?
    supportedLocales = lookupSupportedLocales(availableLocales, requestedLocales);
  } else { // 4. Else,
    // a. Let supportedLocales be LookupSupportedLocales(availableLocales, requestedLocales).
    supportedLocales = lookupSupportedLocales(availableLocales, requestedLocales);
  }
  
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
  // Enum declaration for DateTimeFormat internal slots
  // https://tc39.es/ecma402/#datetimeformat-objects
  enum class FormatMatcher : size_t {Undefined = 0, Bestfit, Basic};
  enum class HourCycle : size_t {Undefined = 0, H11, H12, H23, H24};
  enum class Weekday : size_t {Undefined = 0, Narrow, Short, Long};
  enum class Era : size_t {Undefined = 0, Narrow, Short, Long};
  enum class Year : size_t {Undefined = 0, TwoDigit, Numeric};
  enum class Month : size_t {Undefined = 0, TwoDigit, Numeric, Narrow, Short, Long};
  enum class Day : size_t {Undefined = 0, TwoDigit, Numeric};
  enum class DayPeriod : size_t {Undefined = 0, Narrow, Short, Long};
  enum class Hour : size_t {Undefined = 0, TwoDigit, Numeric};
  enum class Minute : size_t {Undefined = 0, TwoDigit, Numeric};
  enum class Second : size_t {Undefined = 0, TwoDigit, Numeric};
  enum class TimeZoneName : size_t {Undefined = 0, Short, Long};
  enum class DateTimeStyle : size_t {Undefined = 0, Full, Long, Medium, Short};

  static std::u16string formatMatcherString(const FormatMatcher &formatMatcher) {
    switch (formatMatcher) {
      case FormatMatcher::Bestfit:
        return u"best fit";
      case FormatMatcher::Basic:
        return u"basic";
      case FormatMatcher::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  static std::u16string hourCycleString(const HourCycle &hourCycle) {
    switch (hourCycle) {
      case HourCycle::H11:
        return u"h11";
      case HourCycle::H12:
        return u"h12";
      case HourCycle::H23:
        return u"h23";
      case HourCycle::H24:
        return u"h24";
      case HourCycle::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  static std::u16string weekdayString(const Weekday &weekday) {
    switch (weekday) {
      case Weekday::Narrow:
        return u"narrow";
      case Weekday::Short:
        return u"short";
      case Weekday::Long:
        return u"long";
      case Weekday::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  static std::u16string eraString(const Era &era) {
    switch (era) {
      case Era::Narrow:
        return u"narrow";
      case Era::Short:
        return u"short";
      case Era::Long:
        return u"long";
      case Era::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  static std::u16string yearString(const Year &year) {
    switch (year) {
      case Year::Numeric:
        return u"numeric";
      case Year::TwoDigit:
        return u"2-digit";
      case Year::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  static std::u16string monthString(const Month &month) {
    switch (month) {
      case Month::Numeric:
        return u"numeric";
      case Month::TwoDigit:
        return u"2-digit";
      case Month::Narrow:
        return u"narrow";
      case Month::Short:
        return u"short";
      case Month::Long:
        return u"long";
      case Month::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  static std::u16string dayString(const Day &day) {
    switch (day) {
      case Day::Numeric:
        return u"numeric";
      case Day::TwoDigit:
        return u"2-digit";
      case Day::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  static std::u16string hourString(const Hour &hour) {
    switch (hour) {
      case Hour::Numeric:
        return u"numeric";
      case Hour::TwoDigit:
        return u"2-digit";
      case Hour::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  static std::u16string minuteString(const Minute &minute) {
    switch (minute) {
      case Minute::Numeric:
        return u"numeric";
      case Minute::TwoDigit:
        return u"2-digit";
      case Minute::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  static std::u16string secondString(const Second &second) {
    switch (second) {
      case Second::Numeric:
        return u"numeric";
      case Second::TwoDigit:
        return u"2-digit";
      case Second::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  static std::u16string timeZoneNameString(const TimeZoneName &timeZoneName) {
    switch (timeZoneName) {
      case TimeZoneName::Short:
        return u"short";
      case TimeZoneName::Long:
        return u"long";
      case TimeZoneName::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  
  // https://tc39.es/ecma402/#sec-date-time-style-format
  static std::u16string dateTimeStyleString(const DateTimeStyle &dateTimeStyle) {
    switch (dateTimeStyle) {
      case DateTimeStyle::Short:
        return u"short";
      case DateTimeStyle::Long:
        return u"long";
      case DateTimeStyle::Full:
        return u"full";
      case DateTimeStyle::Medium:
        return u"medium";
      case DateTimeStyle::Undefined:
        return u"";
      default:
        return u"";
    }
  }
  // Table 4 from https://tc39.es/ecma402/#table-datetimeformat-components
  FormatMatcher mFormatMatcher {FormatMatcher::Undefined};
  HourCycle mHourCycle {HourCycle::Undefined};
  Weekday mWeekday {Weekday::Undefined};
  Era mEra {Era::Undefined};
  Year mYear {Year::Undefined};
  Month mMonth {Month::Undefined};
  Day mDay {Day::Undefined};
  DayPeriod mDayPeriod {DayPeriod::Undefined};
  Hour mHour {Hour::Undefined};
  Minute mMinute {Minute::Undefined};
  Second mSecond {Second::Undefined};
  TimeZoneName mTimeZoneName {TimeZoneName::Undefined};
  DateTimeStyle mDateStyle {DateTimeStyle::Undefined};
  DateTimeStyle mTimeStyle {DateTimeStyle::Undefined};
  
  // For opt in initialize
  std::u16string localeMatcher;
  std::u16string ca;
  std::u16string nu;
  std::u16string hc;
  
  // For dateTimeFormat in initialize
  std::vector<std::u16string> localeData;
  std::u16string Locale;
  std::u16string Calendar;
  std::u16string HourCycle;
  std::u16string NumberingSystem;
  std::u16string TimeZone;
  std::u16string DateStyle;
  std::u16string TimeStyle;
  std::u16string Hour;
};

DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}
DateTimeFormat::~DateTimeFormat() {}

// Implementation of
// https://tc39.es/ecma402/#sec-resolvelocale
struct ResolveLocale {
  std::u16string dataLocale;
  std::u16string value;
  
  std::u16string resolveLocale;
  std::u16string locale;
  std::u16string ca;
  std::u16string nu;
  std::u16string hc;
};
ResolveLocale resolveLocale(
    vm::CallResult<std::vector<std::u16string>> &requestedLocales,
    const Options &options,
    const std::vector<std::u16string> &relevantExtensionKeys
//  In line with Java, haven't included availableLocales or LocaleData
    ){
//  1. Let matcher be options.[[localeMatcher]].
    auto matcher = options.find(u"localeMatcher");
//  2. If matcher is "lookup", then
      if (matcher->second.getString() == u"lookup") {
//  a. Let r be LookupMatcher(availableLocales, requestedLocales).
//  Call function to get availableLocales once merged?
        auto localeMatchResult = lookupMatcher(availableLocales, requestedLocales);
      }
//  3. Else,
      else {
//  TODO: Will bestFitMatcher also be needed?
//  a. Let r be BestFitMatcher(availableLocales, requestedLocales).
        auto localeMatchResult = bestFitMatcher(availableLocales, requestedLocales);
      }
//  5. Let result be a new Record.
    ResolveLocale result;

//  9. For each element key of relevantExtensionKeys, do
  for (std::u16string key : relevantExtensionKeys) {
    result.value = u"null";
   
  }
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
  return supportedLocales(availableLocales, requestedLocales.getValue(), options);
}

// Implementation of 
// https://tc39.es/ecma402/#sec-todatetimeoptions
vm::CallResult<Options> toDateTimeOptions(const Options &options, std::u16string required, std::u16string defaults) {
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
  // 11. If needDefaults is true and defaults is either "date" or "all", then
  if (needDefaults && (defaults == u"date" || defaults == u"all")) {
    // a. For each property name prop of « "year", "month", "day" », do
    for (std::u16string prop : {u"year", u"month", u"day"}) {
      // TODO: implement createDataPropertyOrThrow
      // i. Perform ? CreateDataPropertyOrThrow(options, prop, "numeric").
    }
  }
  // 12. If needDefaults is true and defaults is either "time" or "all", then
  if (needDefaults && (defaults == u"time" || defaults == u"all")) {
    // a. For each property name prop of « "hour", "minute", "second" », do
    for (std::u16string prop : {u"hour", u"minute", u"second"}) {
      // i. Perform ? CreateDataPropertyOrThrow(options, prop, "numeric").
    }
  }
  // 13. return options
  return options;
}

// Implementation of
// https://tc39.es/ecma402/#sec-initializedatetimeformat
vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  vm::CallResult<std::vector<std::u16string>> requestedLocales =
      getCanonicalLocales(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocales == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  // 2. Let options be ? ToDateTimeOptions(options, "any", "date").
  auto o = toDateTimeOptions(options, u"any", u"date");
  // 3. Let opt be a new Record.
  Impl opt;
  // 4. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  //   "lookup", "best fit" », "best fit").
  std::vector<std::u16string> values = {u"lookup", u"best fit"};
  Option matcherFallback = new Option(u"best fit");
  auto matcher = getOptionString(options, u"localeMatcher", u"string", values, matcherFallback);
  // 5. Set opt.[[localeMatcher]] to matcher.
  opt.localeMatcher = matcher->getString();

  // 6. Let calendar be ? GetOption(options, "calendar", "string",
  //    undefined, undefined).
  std::vector<std::u16string> emptyVector;
  Option undefinedFallback = new Option(u"und"); // TODO: Pass undefined.
  auto calendar = getOptionString(options, u"calendar", u"string", emptyVector, undefinedFallback);
  // 7. If calendar is not undefined, then
  //    a. If calendar does not match the Unicode Locale Identifier type
  //       nonterminal, throw a RangeError exception.
  // 8. Set opt.[[ca]] to calendar.
  if (calendar->getString() != u"und") {
      opt.ca = calendar->getString();
  }
  else {
    return vm::ExecutionStatus::EXCEPTION;
  }
        
  // 9. Let numberingSystem be ? GetOption(options, "numberingSystem",
  //    "string", undefined, undefined).
  auto numberingSystem = getOptionString(options, u"numberingSystem", u"string", emptyVector, undefinedFallback);
  // 10. If numberingSystem is not undefined, then
  //     a. If numberingSystem does not match the Unicode Locale Identifier
  //        type nonterminal, throw a RangeError exception.
  // 11. Set opt.[[nu]] to numberingSystem.
  if (numberingSystem->getString() != u"und") {
      opt.nu = numberingSystem->getString();
  }
  else {
    return vm::ExecutionStatus::EXCEPTION;
  }
        
  // 12. Let hour12 be ? GetOption(options, "hour12", "boolean",
  //     undefined, undefined).
  auto hour12 = getOptionBool(options, u"hour12", u"boolean", emptyVector, undefinedFallback);
  // 13. Let hourCycle be ? GetOption(options, "hourCycle", "string", «
  //     "h11", "h12", "h23", "h24" », undefined).
  std::vector<std::u16string> hourCycleVector = {u"h11", u"h12", u"h23", u"h24"};
  auto hourCycle = getOptionString(options, u"hourCycle", u"string", hourCycleVector, undefinedFallback);
  // 14. If hour12 is not undefined, then
  //     a. Let hourCycle be null.
  if (hour12->getString() != u"und") {
      hourCycle->getString() = u"null";
  }
  // 15. Set opt.[[hc]] to hourCycle.
  opt.hc = hourCycle->getString();
        
//  16. Let localeData be %DateTimeFormat%.[[LocaleData]].
    Impl dateTimeFormat;
//  TODO: resolveLocale? https://tc39.es/ecma402/#sec-resolvelocale
//  17. Let r be ResolveLocale(%DateTimeFormat%.[[AvailableLocales]], requestedLocales, opt, %DateTimeFormat%.[[RelevantExtensionKeys]], localeData).
    std::vector<std::u16string> relevantExtensionKeys = {u"ca", u"nu", u"hc"};
    auto r = resolveLocale(requestedLocales, options, relevantExtensionKeys);
//  18. Set dateTimeFormat.[[Locale]] to r.[[locale]].
    dateTimeFormat.Locale = r.locale;
//  19. Let calendar be r.[[ca]].
    r.ca = calendar->getString();
//  20. Set dateTimeFormat.[[Calendar]] to calendar.
    dateTimeFormat.Calendar = calendar->getString();
//  21. Set dateTimeFormat.[[HourCycle]] to r.[[hc]].
    dateTimeFormat.HourCycle = r.hc;
//  22. Set dateTimeFormat.[[NumberingSystem]] to r.[[nu]].
    dateTimeFormat.NumberingSystem = r.nu;
//  23. Let dataLocale be r.[[dataLocale]].
    auto dataLocale = r.dataLocale;
//  24. Let timeZone be ? Get(options, "timeZone").
    auto timeZone = options.find(u"timeZone");
    std::u16string timeZoneValue;
//  25. If timeZone is undefined, then
    if (timeZone == options.end()) {
//  a. Let timeZone be DefaultTimeZone().
      NSString *nstrTime = (NSString*)[NSTimeZone defaultTimeZone];
      std::u16string defaultTime = nsStringToU16String(nstrTime);
      timeZoneValue = defaultTime;
    }
//  26. Else,
    else {
//  a. Let timeZone be ? ToString(timeZone).
      timeZoneValue = timeZone->second.getString();
//  TODO: Find a way to get timezone validity
//  b. If the result of IsValidTimeZoneName(timeZone) is false, then
//  i. Throw a RangeError exception.
//  c. Let timeZone be CanonicalizeTimeZoneName(timeZone).
    }
//  27. Set dateTimeFormat.[[TimeZone]] to timeZone.
    dateTimeFormat.TimeZone = timeZoneValue;
//  28. Let opt be a new Record.
//  29. For each row of Table 4, except the header row, in table order, do
//  a. Let prop be the name given in the Property column of the row.
      std::vector<std::u16string> tableFourProps = {u"Weekday", u"Era", u"Year", u"Month", u"Day", u"DayPeriod", u"Hour", u"Minute", u"Second", u"FractionalSecondDigits", u"TimeZoneName"};
      for (std::u16string prop : tableFourProps) {
        //  b. If prop is "fractionalSecondDigits", then
        if (prop == u"FractionalSecondDigits") {
        //  i. Let value be ? GetNumberOption(options, "fractionalSecondDigits", 1, 3, undefined).
          auto value = getOptionNumber(options, u"fractionalSecondDigits", 1, 3, undefinedFallback);
        }
        else {
        //  c. Else,
        //  i. Let value be ? GetOption(options, prop, "string", « the strings given in the Values column of the row », undefined).
        // Not sure if this should be string every time....
          auto value = getOptionString(options, prop, u"string", tableFourProps, undefinedFallback);
        }
      //  d. Set opt.[[<prop>]] to value.
          opt.prop = value->getString();
      }
//   30. Let dataLocaleData be localeData.[[<dataLocale>]].
//   31. Let matcher be ? GetOption(options, "formatMatcher", "string", « "basic", "best fit" », "best fit").
      const std::vector<std::u16string> matcherVector = {u"basic", u"best fit"};
      auto mformatMatcher = getOptionString(options, u"formatMatcher", u"string", matcherVector, undefinedFallback);
//   32. Let dateStyle be ? GetOption(options, "dateStyle", "string", « "full", "long", "medium", "short" », undefined).
      const std::vector<std::u16string> dateStyleVector  = {u"full", u"long", u"medium", u"short"};
      auto mdateStyle = getOptionString(options, u"dateStyle", u"string", dateStyleVector, undefinedFallback);
//   33. Set dateTimeFormat.[[DateStyle]] to dateStyle.
      dateTimeFormat.DateStyle = mdateStyle->getString();
//   34. Let timeStyle be ? GetOption(options, "timeStyle", "string", « "full", "long", "medium", "short" », undefined).
      const std::vector<std::u16string> timeStyleVector = {u"full", u"long", u"medium", u"short"};
      auto mtimeStyle = getOptionString(options, u"timeStyle", u"string", timeStyleVector, undefinedFallback);
//   35. Set dateTimeFormat.[[TimeStyle]] to timeStyle.
      dateTimeFormat.TimeStyle = mtimeStyle->getString();
//  36. If dateStyle is not undefined or timeStyle is not undefined, then
      if (mdateStyle->getString() != u"und" || mtimeStyle->getString() != u"und") {
//  a. For each row in Table 4, except the header row, do
//  i. Let prop be the name given in the Property column of the row.
        for (std::u16string prop : {u"Weekday", u"Era", u"Year", u"Month", u"Day", u"DayPeriod", u"Hour", u"Minute", u"Second", u"FractionalSecondDigits", u"TimeZoneName"}) {
//  ii. Let p be opt.[[<prop>]].
          auto p = opt.prop;
//  iii. If p is not undefined, then
          if (p != u"und") {
//  1. Throw a TypeError exception.
            return vm::ExecutionStatus::EXCEPTION;
          }
//  Else,
//  a. Let formats be dataLocaleData.[[formats]].[[<calendar>]].
//  b. If matcher is "basic", then
//  i. Let bestFormat be BasicFormatMatcher(opt, formats).
// TODO: BasicFormatMatcher? https://tc39.es/ecma402/#sec-basicformatmatcher

//  c. Else,
//  i. Let bestFormat be BestFitFormatMatcher(opt, formats).

        }
      }
//  Skip 38?
//  39. If dateTimeFormat.[[Hour]] is undefined, then
      if (dateTimeFormat.Hour == u"und") {
//  a. Set dateTimeFormat.[[HourCycle]] to undefined.
        dateTimeFormat.HourCycle = u"und";
      }
      else {
//  b. Let hc be dateTimeFormat.[[HourCycle]].
        auto hc = dateTimeFormat.HourCycle;
//  c. If hc is null, then
        if (hc == u"") {
//  i. Set hc to hcDefault.
        }
//  d. If hour12 is not undefined, then
//  i. If hour12 is true, then
//  1. If hcDefault is "h11" or "h23", then
//  a. Set hc to "h11".
//  2. Else,
//  a. Set hc to "h12".
//  ii. Else,
//  1. Assert: hour12 is false.
//  2. If hcDefault is "h11" or "h23", then
//  a. Set hc to "h23".
//  3. Else,
//  a. Set hc to "h24".
//  e. Set dateTimeFormat.[[HourCycle]] to hc.
      }
//  43. Return dateTimeFormat.
  return dateTimeFormat;
}

Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  // Implementation of
  // https://tc39.es/ecma402/#sec-todatetimeoptions
  //        2. Let options be ? ToDateTimeOptions(options, "any", "date").
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
