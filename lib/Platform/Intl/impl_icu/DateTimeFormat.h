/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_DATETIMEFORMAT_H
#define HERMES_PLATFORMINTL_IMPLICU_DATETIMEFORMAT_H

#include "LocaleBCP47Object.h"
#include "hermes/Platform/Intl/PlatformIntl.h"
#include "llvh/ADT/ArrayRef.h"

#include <unicode/udat.h>
#include <unicode/udateintervalformat.h>
#include <unicode/udatpg.h>

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

class DateTimeFormat : public platform_intl::DateTimeFormat {
 public:
  /**
   * @brief Creates a DateTimeFormat.
   */
  DateTimeFormat();

  /**
   * @brief Destructs the DateTimeFormat.
   */
  ~DateTimeFormat() override;

  /**
   * Initializes the DateTimeFormat.
   *
   * See https://tc39.es/ecma402/#sec-intl-datetimeformat-constructor.
   *
   * @param runtime runtime object
   * @param locales locales passed from JS
   * @param options options passed from JS
   * @return ExecutionStatus.RETURNED on success, ExecutionStatus.EXCEPTION
   * on failure.
   */
  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

  /**
   * Returns the resolved options.
   *
   * See
   * https://tc39.es/ecma402/#sec-intl.datetimeformat.prototype.resolvedoptions.
   *
   * @return a new options object with properties reflecting the locale and
   * date-time-format options computed during initialization of this
   * DateTimeFormat instance.
   */
  Options resolvedOptions() noexcept;

  /**
   * Returns provided locales that DateTimeFormat supports.
   *
   * See https://tc39.es/ecma402/#sec-intl.datetimeformat.supportedlocalesof.
   *
   * @param runtime runtime object
   * @param locales locales passed from JS
   * @param options options passed from JS
   * @return CallResult with a vector of provided locales that are supported on
   * success, with ExecutionStatus.EXCEPTION on failure.
   */
  static vm::CallResult<std::vector<std::u16string>> supportedLocalesOf(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

  /**
   * Formats an epoch date value to a string according to locale and options of
   * this DateTimeFormat instance.
   *
   * See https://tc39.es/ecma402/#sec-formatdatetime.
   *
   * @param value epoch date value
   * @return a formatted date-time string
   */
  std::u16string format(double value) noexcept;

  /**
   * Formats an epoch date value to a vector of objects containing the formatted
   * date-time in parts according to locale and options of this DateTimeFormat
   * instance.
   *
   * See https://tc39.es/ecma402/#sec-formatdatetimetoparts.
   *
   * @param value epoch date value
   * @return a vector of formatted date-time string parts
   */
  std::vector<Part> formatToParts(double value) noexcept;

  /**
   * Formats a date range to a string according to locale and
   * options of this DateTimeFormat instance.
   *
   * See https://tc39.es/ecma402/#sec-formatdatetimerange.
   *
   * @param runtime runtime object
   * @param startDate start of date range, an epoch date value
   * @param endDate end of date range, an epoch date value
   * @return a formatted date-time range string
   */
  vm::CallResult<std::u16string>
  formatRange(vm::Runtime &runtime, double startDate, double endDate) noexcept;

  /**
   * Formats a date range to a vector of objects containing the formatted
   * date-time range in parts according to locale and options of this
   * DateTimeFormat instance.
   *
   * See https://tc39.es/ecma402/#sec-formatdatetimerangetoparts.
   *
   * @param runtime runtime object
   * @param startDate start of date range, an epoch date value
   * @param endDate end of date range, an epoch date value
   * @return a vector of formatted date-time range string parts
   */
  vm::CallResult<std::vector<Part>> formatRangeToParts(
      vm::Runtime &runtime,
      double startDate,
      double endDate) noexcept;

 private:
  enum class DateTimeComponent : int { DATE, TIME, ALL, ANY };

  // Corresponds to table headers of
  // https://tc39.es/ecma402/#table-datetimeformat-components.
  // Additionally, include the date time format component's corresponding
  // icu skeleton symbol.
  struct DateTimeFormatComponent {
    const std::u16string_view property_;
    const llvh::ArrayRef<const char16_t *> values_;
    std::optional<Option> resolvedValue_;
    const char16_t icuSkeletonSymbol;
  };

  /**
   * SymbolFindResult is used to store the search result of a symbol
   * in a date-time format pattern.
   */
  struct SymbolFindResult {
    /** The symbol found */
    UChar symbol;
    /** The index of the symbol's first occurrence in the pattern if found */
    int startPos;
    /** The total number of occurrences of the symbol in the pattern if found */
    int count;
    SymbolFindResult() : symbol(u'\0'), startPos(-1), count(0) {}
  };

  // https://tc39.es/ecma402/#sec-createdatetimeformat
  vm::ExecutionStatus createDateTimeFormat(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options,
      DateTimeComponent requiredComponent,
      DateTimeComponent defaultComponent) noexcept;

  static std::u16string systemTimeZoneIdentifier();

  static const std::unordered_map<std::u16string, std::u16string> &
  getValidTimeZones();

  static bool isValidTimeZoneName(
      const std::u16string &timeZone,
      std::u16string &canonicalizedTimeZoneResult);

  static bool isTimeZoneOffsetString(
      const std::u16string &timeZone,
      std::u16string &canonicalizedTimeZoneResult);

  static std::u16string canonicalizeTimeZoneName(
      const std::u16string &timeZoneName);

  static std::optional<std::u16string> getCanonicalTimeZoneId(
      const std::u16string &timeZoneId);

  static vm::CallResult<std::u16string> formatOffsetTimeZoneIdentifier(
      vm::Runtime &runtime,
      const std::u16string &icuOffsetTimeZone) noexcept;

  static bool isExtensionTypeSupported(
      std::u16string_view extensionKey,
      std::u16string_view extensionType,
      const LocaleBCP47Object &localeBCP47Object);

  static std::u16string getDefaultCalendar(const std::string &localeICU);

  static std::unordered_set<std::u16string> getAvailableCalendars(
      const std::string &localeICU);

  static std::u16string convertLegacyCalendar(std::string_view cal);

  static char16_t getHourSymbol(const std::u16string &hourCycle);

  static std::u16string getDefaultHourCycle(const std::string &localeICU);

  static std::optional<SymbolFindResult> findHourSymbol(
      const std::u16string &pattern);

  /**
   * Find a date field symbol in the given pattern.
   *
   * Date Field Symbol Table are defined by CLDR; for more information see
   * https://www.unicode.org/reports/tr35/tr35-dates.html#Date_Field_Symbol_Table
   *
   * @param pattern[in]      A date pattern.
   * @param symbol[in]       A date field symbol.
   * @return a SymbolFindResult object if symbol is found in the pattern,
   * otherwise std::nullopt.
   */
  static std::optional<SymbolFindResult> findDateFieldSymbol(
      const std::u16string &pattern,
      char16_t symbol);

  static UDateFormatStyle dateTimeStyleToICUDateFormatStyle(
      const std::optional<std::u16string> &styleOpt);

  static void cleanUpBaseSkeleton(std::u16string &skeleton);

  std::u16string buildSkeleton();

  static bool getBestPattern(
      UDateTimePatternGenerator *generator,
      std::u16string_view skeleton,
      UDateTimePatternMatchOptions options,
      std::u16string &bestPatternResult);

  static std::vector<Part> partitionPattern(
      UFieldPositionIterator *posIter,
      const std::u16string &formattedStr);

  static std::u16string icuDateFieldTypeToPartType(int32_t fieldType);

  static vm::CallResult<std::vector<Part>> partitionDateTimeRangePattern(
      vm::Runtime &runtime,
      const UFormattedDateInterval *formattedDateInterval);

  static bool isSpanInRange(
      std::pair<int32_t, int32_t> span,
      std::pair<int32_t, int32_t> range);

  static std::u16string getPartSource(
      std::pair<int32_t, int32_t> span,
      std::pair<int32_t, int32_t> startDateRange,
      std::pair<int32_t, int32_t> endDateRange);

  std::u16string resolvedLocale_;
  std::u16string resolvedCalendar_;
  std::u16string resolvedNumberingSystem_;
  std::u16string resolvedTimeZone_;
  std::optional<std::u16string> resolvedHourCycle_;
  std::optional<std::u16string> resolvedDateStyle_;
  std::optional<std::u16string> resolvedTimeStyle_;
  std::unique_ptr<UDateFormat, decltype(&udat_close)> dateFormat_;
  std::unique_ptr<UDateIntervalFormat, decltype(&udtitvfmt_close)>
      dateIntervalFormat_;
  std::array<DateTimeFormatComponent, 11> dateTimeFormatComponents_;
};

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_DATETIMEFORMAT_H