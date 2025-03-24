/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_CONSTANTS_H
#define HERMES_PLATFORMINTL_IMPLICU_CONSTANTS_H

namespace hermes {
namespace platform_intl {
namespace impl_icu {
namespace constants {

namespace opt_name {
inline constexpr char16_t localeMatcher[] = u"localeMatcher";
inline constexpr char16_t locale[] = u"locale";
inline constexpr char16_t usage[] = u"usage";
inline constexpr char16_t collation[] = u"collation";
inline constexpr char16_t numeric[] = u"numeric";
inline constexpr char16_t caseFirst[] = u"caseFirst";
inline constexpr char16_t sensitivity[] = u"sensitivity";
inline constexpr char16_t ignorePunctuation[] = u"ignorePunctuation";
inline constexpr char16_t calendar[] = u"calendar";
inline constexpr char16_t numberingSystem[] = u"numberingSystem";
inline constexpr char16_t hour12[] = u"hour12";
inline constexpr char16_t hourCycle[] = u"hourCycle";
inline constexpr char16_t timeZone[] = u"timeZone";
inline constexpr char16_t formatMatcher[] = u"formatMatcher";
inline constexpr char16_t dateStyle[] = u"dateStyle";
inline constexpr char16_t timeStyle[] = u"timeStyle";
inline constexpr char16_t weekday[] = u"weekday";
inline constexpr char16_t era[] = u"era";
inline constexpr char16_t year[] = u"year";
inline constexpr char16_t years[] = u"years";
inline constexpr char16_t month[] = u"month";
inline constexpr char16_t months[] = u"months";
inline constexpr char16_t weeks[] = u"weeks";
inline constexpr char16_t day[] = u"day";
inline constexpr char16_t days[] = u"days";
inline constexpr char16_t dayPeriod[] = u"dayPeriod";
inline constexpr char16_t hour[] = u"hour";
inline constexpr char16_t hours[] = u"hours";
inline constexpr char16_t minute[] = u"minute";
inline constexpr char16_t minutes[] = u"minutes";
inline constexpr char16_t second[] = u"second";
inline constexpr char16_t seconds[] = u"seconds";
inline constexpr char16_t fractionalSecondDigits[] = u"fractionalSecondDigits";
inline constexpr char16_t milliseconds[] = u"milliseconds";
inline constexpr char16_t microseconds[] = u"microseconds";
inline constexpr char16_t nanoseconds[] = u"nanoseconds";
inline constexpr char16_t timeZoneName[] = u"timeZoneName";
inline constexpr char16_t style[] = u"style";
inline constexpr char16_t fractionalDigits[] = u"fractionalDigits";
} // namespace opt_name

namespace opt_value {

inline constexpr char16_t trueStr[] = u"true";
inline constexpr char16_t falseStr[] = u"false";

namespace locale_matcher {
inline constexpr char16_t lookup[] = u"lookup";
inline constexpr char16_t best_fit[] = u"best fit";
inline constexpr const char16_t *validLocaleMatchers[] = {lookup, best_fit};
} // namespace locale_matcher

namespace case_first {
inline constexpr char16_t upper[] = u"upper";
inline constexpr char16_t lower[] = u"lower";
inline constexpr const char16_t *validCaseFirsts[] = {upper, lower, falseStr};
} // namespace case_first

namespace sensitivity {
inline constexpr char16_t base[] = u"base";
inline constexpr char16_t accent[] = u"accent";
inline constexpr char16_t caseStr[] = u"case";
inline constexpr char16_t variant[] = u"variant";
inline constexpr const char16_t *validSensitivities[] = {
    base,
    accent,
    caseStr,
    variant};
} // namespace sensitivity

namespace usage {
inline constexpr char16_t sort[] = u"sort";
inline constexpr char16_t search[] = u"search";
inline constexpr const char16_t *validUsages[] = {sort, search};
} // namespace usage

namespace numeric {
inline constexpr const char16_t *validNumerics[] = {trueStr, falseStr};
} // namespace numeric

namespace format_matcher {
inline constexpr char16_t basic[] = u"basic";
inline constexpr char16_t best_fit[] = u"best fit";
inline constexpr const char16_t *validFormatMatchers[] = {basic, best_fit};
} // namespace format_matcher

namespace hour_cycle {
inline constexpr char16_t h11[] = u"h11";
inline constexpr char16_t h12[] = u"h12";
inline constexpr char16_t h23[] = u"h23";
inline constexpr char16_t h24[] = u"h24";
inline constexpr const char16_t *validHourCycles[] = {h11, h12, h23, h24};
} // namespace hour_cycle

namespace hour12 {
inline constexpr const char16_t *validHour12s[] = {trueStr, falseStr};
} // namespace hour12

namespace style {
inline constexpr char16_t full[] = u"full";
inline constexpr char16_t longStr[] = u"long";
inline constexpr char16_t medium[] = u"medium";
inline constexpr char16_t shortStr[] = u"short";
inline constexpr char16_t narrow[] = u"narrow";
inline constexpr char16_t digital[] = u"digital";
inline constexpr char16_t numeric[] = u"numeric";
inline constexpr char16_t twoDigit[] = u"2-digit";
inline constexpr char16_t shortOffset[] = u"shortOffset";
inline constexpr char16_t longOffset[] = u"longOffset";
inline constexpr char16_t shortGeneric[] = u"shortGeneric";
inline constexpr char16_t longGeneric[] = u"longGeneric";
inline constexpr const char16_t *validDateTimeStyles[] = {
    full,
    longStr,
    medium,
    shortStr};
inline constexpr const char16_t *validNameOnlyStyles[] = {
    longStr,
    shortStr,
    narrow};
inline constexpr const char16_t *validNumericOnlyStyles[] = {numeric, twoDigit};
inline constexpr const char16_t *validNameAndNumericStyles[] =
    {longStr, shortStr, narrow, numeric, twoDigit};
inline constexpr const char16_t *validTimeZoneNameStyles[] =
    {longStr, shortStr, longOffset, shortOffset, longGeneric, shortGeneric};
inline constexpr const char16_t *validDurationFormatStyles[] = {
    longStr,
    shortStr,
    narrow,
    digital};
} // namespace style

namespace display {
inline constexpr char16_t autoStr[] = u"auto";
inline constexpr char16_t always[] = u"always";
inline constexpr const char16_t *validDurationUnitDisplays[] = {
    autoStr,
    always};
} // namespace display

} // namespace opt_value

// All available extension keys defined by Unicode are described in
// https://unicode.org/reports/tr35/#Key_Type_Definitions
namespace extension_key {
inline constexpr char16_t ca[] = u"ca";
inline constexpr char16_t co[] = u"co";
inline constexpr char16_t hc[] = u"hc";
inline constexpr char16_t kf[] = u"kf";
inline constexpr char16_t kn[] = u"kn";
inline constexpr char16_t nu[] = u"nu";
} // namespace extension_key

namespace part_key {
inline constexpr char16_t type[] = u"type";
inline constexpr char16_t value[] = u"value";
inline constexpr char16_t unit[] = u"unit";
} // namespace part_key

namespace part_type {
inline constexpr char16_t literal[] = u"literal";
} // namespace part_type

} // namespace constants
} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_CONSTANTS_H
