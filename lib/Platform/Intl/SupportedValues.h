/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_SUPPORTEDVALUES_H
#define HERMES_PLATFORMINTL_SUPPORTEDVALUES_H

#ifdef HERMES_ENABLE_INTL

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace hermes {
namespace platform_intl {

inline void sortAndUnique(std::vector<std::u16string> &values) {
  std::sort(values.begin(), values.end());
  values.erase(std::unique(values.begin(), values.end()), values.end());
}

/// Canonicalize Etc/UTC and Etc/GMT to UTC, then sort, unique, and
/// ensure UTC is present.
/// https://tc39.es/ecma402/#sec-canonicalizetimezonename
inline void finalizeSupportedTimeZones(
    std::vector<std::u16string> &timeZones) {
  for (auto &tz : timeZones) {
    if (tz == u"Etc/UTC" || tz == u"Etc/GMT")
      tz = u"UTC";
  }
  sortAndUnique(timeZones);
  if (std::find(timeZones.begin(), timeZones.end(), u"UTC") ==
      timeZones.end()) {
    timeZones.emplace_back(u"UTC");
    std::sort(timeZones.begin(), timeZones.end());
  }
}

template <size_t N>
inline std::vector<std::u16string> copySorted(
    const char16_t *const (&values)[N]) {
  std::vector<std::u16string> result;
  result.reserve(N);
  for (const char16_t *v : values)
    result.emplace_back(v);
  std::sort(result.begin(), result.end());
  return result;
}

/// https://tc39.es/ecma402/#sec-availablecalendars
inline std::vector<std::u16string> availableCalendars() {
  // Matches the calendars Hermes already treats as valid.
  static constexpr const char16_t *kCalendars[] = {
      u"buddhist",
      u"chinese",
      u"coptic",
      u"dangi",
      u"ethioaa",
      u"ethiopic",
      u"gregory",
      u"hebrew",
      u"indian",
      u"islamic",
      u"islamic-civil",
      u"islamic-rgsa",
      u"islamic-tbla",
      u"islamic-umalqura",
      u"iso8601",
      u"japanese",
      u"persian",
      u"roc"};
  return copySorted(kCalendars);
}

/// https://tc39.es/ecma402/#sec-availablecanonicalcollations
inline std::vector<std::u16string> availableCollations() {
  // "standard" and "search" are excluded by the spec.
  static constexpr const char16_t *kCollations[] = {
      u"big5han",
      u"compat",
      u"dict",
      u"direct",
      u"ducet",
      u"emoji",
      u"eor",
      u"gb2312",
      u"phonebk",
      u"phonetic",
      u"pinyin",
      u"reformed",
      u"searchjl",
      u"stroke",
      u"trad",
      u"unihan",
      u"zhuyin"};
  return copySorted(kCollations);
}

/// https://tc39.es/ecma402/#sec-availablecanonicalnumberingsystems
inline std::vector<std::u16string> availableNumberingSystems() {
  // Simple numbering systems already treated as valid by Hermes.
  static constexpr const char16_t *kNumberingSystems[] = {
      u"adlm",     u"ahom",     u"arab",     u"arabext",  u"bali",
      u"beng",     u"bhks",     u"brah",     u"cakm",     u"cham",
      u"deva",     u"diak",     u"fullwide", u"gong",     u"gonm",
      u"gujr",     u"guru",     u"hanidec",  u"hmng",     u"hmnp",
      u"java",     u"kali",     u"khmr",     u"knda",     u"lana",
      u"lanatham", u"laoo",     u"latn",     u"lepc",     u"limb",
      u"mathbold", u"mathdbl",  u"mathmono", u"mathsanb", u"mathsans",
      u"mlym",     u"modi",     u"mong",     u"mroo",     u"mtei",
      u"mymr",     u"mymrshan", u"mymrtlng", u"newa",     u"nkoo",
      u"olck",     u"orya",     u"osma",     u"rohg",     u"saur",
      u"segment",  u"shrd",     u"sind",     u"sinh",     u"sora",
      u"sund",     u"takr",     u"talu",     u"tamldec",  u"telu",
      u"thai",     u"tibt",     u"tirh",     u"vaii",     u"wara",
      u"wcho"};
  return copySorted(kNumberingSystems);
}

/// https://tc39.es/ecma402/#sec-availablecanonicalunits
inline std::vector<std::u16string> availableUnits() {
  static constexpr const char16_t *kUnits[] = {
      u"acre",
      u"bit",
      u"byte",
      u"celsius",
      u"centimeter",
      u"day",
      u"degree",
      u"fahrenheit",
      u"fluid-ounce",
      u"foot",
      u"gallon",
      u"gigabit",
      u"gigabyte",
      u"gram",
      u"hectare",
      u"hour",
      u"inch",
      u"kilobit",
      u"kilobyte",
      u"kilogram",
      u"kilometer",
      u"liter",
      u"megabit",
      u"megabyte",
      u"meter",
      u"microsecond",
      u"mile",
      u"mile-scandinavian",
      u"milliliter",
      u"millimeter",
      u"millisecond",
      u"minute",
      u"month",
      u"nanosecond",
      u"ounce",
      u"percent",
      u"petabyte",
      u"pound",
      u"second",
      u"stone",
      u"terabit",
      u"terabyte",
      u"week",
      u"yard",
      u"year"};
  return copySorted(kUnits);
}

/// Handle keys whose values are not platform-specific. Returns
/// nullopt for timeZone and currency (platform must supply those) and
/// for invalid keys.
inline std::optional<std::vector<std::u16string>>
trySupportedValuesOfCommon(const std::u16string &key) {
  if (key == u"calendar")
    return availableCalendars();
  if (key == u"collation")
    return availableCollations();
  if (key == u"numberingSystem")
    return availableNumberingSystems();
  if (key == u"unit")
    return availableUnits();
  return std::nullopt;
}

} // namespace platform_intl
} // namespace hermes

#endif // HERMES_ENABLE_INTL

#endif // HERMES_PLATFORMINTL_SUPPORTEDVALUES_H
