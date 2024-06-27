/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_CONSTANTS_H
#define HERMES_PLATFORMINTL_CONSTANTS_H

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace hermes {
namespace platform_intl {
namespace Constants {

const struct {
  std::u16string true_;
  std::u16string false_;
} boolStr = {u"true", u"false"};

const struct {
  std::u16string matcher_;
  std::u16string locale_;
  std::u16string usage_;
  std::u16string collation_;
  std::u16string numeric_;
  std::u16string caseFirst_;
  std::u16string sensitivity_;
  std::u16string ignorePunctuation_;
  std::u16string numberingSystem_;
  std::u16string style_;
  std::u16string years_;
  std::u16string months_;
  std::u16string weeks_;
  std::u16string days_;
  std::u16string hours_;
  std::u16string minutes_;
  std::u16string seconds_;
  std::u16string milliseconds_;
  std::u16string microseconds_;
  std::u16string nanoseconds_;
  std::u16string fractionalDigits_;
  std::u16string dayPeriod_;
  std::u16string fractionalSecondDigits_;
  std::u16string calendar_;
  std::u16string hour12_;
  std::u16string hourCycle_;
  std::u16string timeZone_;
  std::u16string formatMatcher_;
  std::u16string dateStyle_;
  std::u16string timeStyle_;
  std::u16string weekday_;
  std::u16string era_;
  std::u16string year_;
  std::u16string month_;
  std::u16string day_;
  std::u16string hour_;
  std::u16string minute_;
  std::u16string second_;
  std::u16string timeZoneName_;
} optName = {
    u"localeMatcher",
    u"locale",
    u"usage",
    u"collation",
    u"numeric",
    u"caseFirst",
    u"sensitivity",
    u"ignorePunctuation",
    u"numberingSystem",
    u"style",
    u"years",
    u"months",
    u"weeks",
    u"days",
    u"hours",
    u"minutes",
    u"seconds",
    u"milliseconds",
    u"microseconds",
    u"nanoseconds",
    u"fractionalDigits",
    u"dayPeriod",
    u"fractionalSecondDigits",
    u"calendar",
    u"hour12",
    u"hourCycle",
    u"timeZone",
    u"formatMatcher",
    u"dateStyle",
    u"timeStyle",
    u"weekday",
    u"era",
    u"year",
    u"month",
    u"day",
    u"hour",
    u"minute",
    u"second",
    u"timeZoneName"};

const struct {
  const struct {
    std::u16string upper_;
    std::u16string lower_;
    std::u16string false_;
  } caseFirst = {u"upper", u"lower", boolStr.false_};
  const struct {
    std::u16string true_;
    std::u16string false_;
  } numeric = {boolStr.true_, boolStr.false_};
  const struct {
    std::u16string base_;
    std::u16string accent_;
    std::u16string case_;
    std::u16string variant_;
  } sensitivity = {u"base", u"accent", u"case", u"variant"};
  const struct {
    std::u16string sort_;
    std::u16string search_;
  } usage = {u"sort", u"search"};
  const struct {
    std::u16string lookup_;
    std::u16string best_fit_;
  } matcher = {u"lookup", u"best fit"};
  const struct {
    std::u16string basic_;
    std::u16string best_fit_;
  } formatMatcher = {u"basic", u"best fit"};
  const struct {
    std::u16string h11_;
    std::u16string h12_;
    std::u16string h23_;
    std::u16string h24_;
  } hourCycle = {u"h11", u"h12", u"h23", u"h24"};
  const struct {
    std::u16string long_;
    std::u16string full_;
    std::u16string medium_;
    std::u16string short_;
    std::u16string narrow_;
    std::u16string digital_;
    std::u16string numeric_;
    std::u16string twoDigit_;
    std::u16string shortOffset_;
    std::u16string longOffset_;
    std::u16string shortGeneric_;
    std::u16string longGeneric_;
  } style = {
      u"long",
      u"full",
      u"medium",
      u"short",
      u"narrow",
      u"digital",
      u"numeric",
      u"2-digit",
      u"shortOffset",
      u"longOffset",
      u"shortGeneric",
      u"longGeneric"};

  const struct {
    std::u16string auto_;
    std::u16string always_;
  } unitDisplay = {u"auto", u"always"};

} optValue;

// All available extension keys defined by Unicode are described in
// https://unicode.org/reports/tr35/#Key_Type_Definitions
const struct {
  std::u16string co_;
  std::u16string kn_;
  std::u16string kf_;
  std::u16string nu_;
  std::u16string ca_;
  std::u16string hc_;
} extKey = {u"co", u"kn", u"kf", u"nu", u"ca", u"hc"};

const struct {
  std::u16string type_;
  std::u16string value_;
  std::u16string unit_;
} partKey = {u"type", u"value", u"unit"};

const struct { std::u16string literal_; } partType = {u"literal"};

// valid option values are matched with option section of
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Collator/Collator
const std::unordered_set<std::u16string> validSensitivities = {
    optValue.sensitivity.base_,
    optValue.sensitivity.accent_,
    optValue.sensitivity.case_,
    optValue.sensitivity.variant_};

// valid option values are matched with option section of
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Collator/Collator
const std::unordered_set<std::u16string> validUsages = {
    optValue.usage.sort_,
    optValue.usage.search_};

// valid option values are matched with option section of
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Collator/Collator
const std::unordered_set<std::u16string> validMatchers = {
    optValue.matcher.lookup_,
    optValue.matcher.best_fit_};

const std::unordered_set<std::u16string> validFormatMatchers = {
    optValue.formatMatcher.basic_,
    optValue.formatMatcher.best_fit_};

// valid option values are matched with option section of
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Collator/Collator
const std::unordered_set<std::u16string> validCaseFirsts = {
    optValue.caseFirst.upper_,
    optValue.caseFirst.lower_,
    optValue.caseFirst.false_};

const std::unordered_set<std::u16string> validHourCycles = {
    optValue.hourCycle.h11_,
    optValue.hourCycle.h12_,
    optValue.hourCycle.h23_,
    optValue.hourCycle.h24_};

const std::unordered_set<std::u16string> validNumeric = {
    boolStr.true_,
    boolStr.false_};

const std::unordered_set<std::u16string> validHour12 = {
    boolStr.true_,
    boolStr.false_};

const std::unordered_set<std::u16string> validDurationFormatStyles = {
    optValue.style.long_,
    optValue.style.short_,
    optValue.style.narrow_,
    optValue.style.digital_};

const std::unordered_set<std::u16string> validDateTimeStyles = {
    optValue.style.long_,
    optValue.style.full_,
    optValue.style.medium_,
    optValue.style.short_};

const std::unordered_set<std::u16string> validUnitDisplays = {
    optValue.unitDisplay.auto_,
    optValue.unitDisplay.always_};

const std::unordered_set<std::u16string> validWeekdayStyle = {
    optValue.style.long_,
    optValue.style.short_,
    optValue.style.narrow_};

const std::unordered_set<std::u16string> validEra = {
    optValue.style.long_,
    optValue.style.short_,
    optValue.style.narrow_};

const std::unordered_set<std::u16string> validYearStyle = {
    optValue.style.numeric_,
    optValue.style.twoDigit_};

const std::unordered_set<std::u16string> validMonthStyle = {
    optValue.style.numeric_,
    optValue.style.twoDigit_,
    optValue.style.long_,
    optValue.style.short_,
    optValue.style.narrow_};

const std::unordered_set<std::u16string> validDayStyle = {
    optValue.style.numeric_,
    optValue.style.twoDigit_};

const std::unordered_set<std::u16string> validDayPeriod = {
    optValue.style.narrow_,
    optValue.style.short_,
    optValue.style.long_};

const std::unordered_set<std::u16string> validHourStyle = {
    optValue.style.numeric_,
    optValue.style.twoDigit_};

const std::unordered_set<std::u16string> validMinuteStyle = {
    optValue.style.numeric_,
    optValue.style.twoDigit_};

const std::unordered_set<std::u16string> validSecondStyle = {
    optValue.style.numeric_,
    optValue.style.twoDigit_};

const std::unordered_set<std::u16string> validTimeZoneNames = {
    optValue.style.long_,
    optValue.style.short_,
    optValue.style.longOffset_,
    optValue.style.shortOffset_,
    optValue.style.longGeneric_,
    optValue.style.shortGeneric_};

// Most locales use ":" as separator for digital style duration, with a few
// using "." as separator, see
// https://www.unicode.org/cldr/charts/42/by_type/units.duration.html#Duration_Patterns
const std::unordered_map<std::string, std::u16string>
    langToDurationDigitalSeparatorMap = {
        {"", u":"}, // empty string key as default value
        {"da", u"."},
        {"fi", u"."},
        {"id", u"."},
        {"nds", u"."},
        {"si", u"."},
        {"sl", u"."},
        {"sr", u"."},
        {"su", u"."}};

} // namespace Constants
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_CONSTANTS_H