/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "DateTimeFormat.h"

#include "Constants.h"
#include "IntlUtils.h"
#include "LocaleConverter.h"
#include "LocaleResolver.h"
#include "NumberingSystem.h"
#include "OptionHelpers.h"

#include <unicode/ucal.h>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

DateTimeFormat::DateTimeFormat()
    : dateFormat_(nullptr, &udat_close),
      dateIntervalFormat_(nullptr, &udtitvfmt_close),
      // Corresponde to the table
      // https://tc39.es/ecma402/#table-datetimeformat-components.
      dateTimeFormatComponents_{
          {{constants::opt_name::weekday,
            constants::opt_value::style::validNameOnlyStyles,
            std::nullopt,
            u'E'},
           {constants::opt_name::era,
            constants::opt_value::style::validNameOnlyStyles,
            std::nullopt,
            u'G'},
           {constants::opt_name::year,
            constants::opt_value::style::validNumericOnlyStyles,
            std::nullopt,
            u'y'},
           {constants::opt_name::month,
            constants::opt_value::style::validNameAndNumericStyles,
            std::nullopt,
            u'M'},
           {constants::opt_name::day,
            constants::opt_value::style::validNumericOnlyStyles,
            std::nullopt,
            u'd'},
           {constants::opt_name::dayPeriod,
            constants::opt_value::style::validNameOnlyStyles,
            std::nullopt,
            u'B'},
           {constants::opt_name::hour,
            constants::opt_value::style::validNumericOnlyStyles,
            std::nullopt,
            // hour's icu skeleton symbol depends on the resolved hour cycle.
            // 'j' is populated here as a default, which is the hour skeleton
            // symbol to specify the use of locale default hour cycle symbol.
            // It's not used in buildSkeleton(), which gets the symbol based
            // on the resolved hour cycle.
            u'j'},
           {constants::opt_name::minute,
            constants::opt_value::style::validNumericOnlyStyles,
            std::nullopt,
            u'm'},
           {constants::opt_name::second,
            constants::opt_value::style::validNumericOnlyStyles,
            std::nullopt,
            u's'},
           {constants::opt_name::fractionalSecondDigits,
            // fractionalSecondDigits values isn't populated here. When calling
            // getNumberOption() for it, 1 as the minimum valid value and 3 as
            // the maximum valid value are used.
            {},
            std::nullopt,
            u'S'},
           {constants::opt_name::timeZoneName,
            constants::opt_value::style::validTimeZoneNameStyles,
            std::nullopt,
            // time zone name's icu skeleton symbol differs if the style is
            // offset or generic type. That will be handled in buildSkeleton().
            u'z'}}} {}

DateTimeFormat::~DateTimeFormat() = default;

vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return createDateTimeFormat(
      runtime,
      locales,
      options,
      DateTimeComponent::ANY,
      DateTimeComponent::DATE);
}

vm::ExecutionStatus DateTimeFormat::createDateTimeFormat(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options,
    const DateTimeComponent requiredComponent,
    const DateTimeComponent defaultComponent) noexcept {
  // 1. Let dateTimeFormat be ? OrdinaryCreateFromConstructor(newTarget,
  // "%DateTimeFormat.prototype%", « [[InitializedDateTimeFormat]],
  // [[Locale]], [[Calendar]], [[NumberingSystem]], [[TimeZone]],
  // [[Weekday]], [[Era]], [[Year]], [[Month]], [[Day]], [[DayPeriod]],
  // [[Hour]], [[Minute]], [[Second]], [[FractionalSecondDigits]],
  // [[TimeZoneName]], [[HourCycle]], [[DateStyle]], [[TimeStyle]],
  // [[Pattern]], [[RangePatterns]], [[BoundFormat]] »).
  // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  auto requestedLocalesRes =
      LocaleBCP47Object::canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(requestedLocalesRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }

  // 3. Set options to ? CoerceOptionsToObject(options).
  // 4. Let opt be a new Record.
  Options opt;

  // 5. Let matcher be ? GetOption(options, "localeMatcher", string, « "lookup",
  // "best fit" », "best fit").
  // 6. Set opt.[[localeMatcher]] to matcher.
  auto matcherRes = getStringOption(
      runtime,
      options,
      constants::opt_name::localeMatcher,
      constants::opt_value::locale_matcher::validLocaleMatchers,
      constants::opt_value::locale_matcher::bestFit);
  if (LLVM_UNLIKELY(matcherRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  opt.emplace(constants::opt_name::localeMatcher, **matcherRes);

  // 7. Let calendar be ? GetOption(options, "calendar", string, empty,
  // undefined).
  // 8. If calendar is not undefined, then
  // a. If calendar cannot be matched by the type Unicode locale nonterminal,
  // throw a RangeError exception.
  // 9. Set opt.[[ca]] to calendar.
  auto calendarRes = getStringOption(
      runtime, options, constants::opt_name::calendar, {}, std::nullopt);
  if (LLVM_UNLIKELY(calendarRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  if (calendarRes->has_value()) {
    if (!isUnicodeExtensionType(**calendarRes)) {
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid calendar: ") +
          vm::TwineChar16((*calendarRes)->c_str()));
    }
    opt.emplace(constants::extension_key::ca, **calendarRes);
  }

  // 10. Let numberingSystem be ? GetOption(options, "numberingSystem", string,
  // empty, undefined).
  // 11. If numberingSystem is not undefined, then
  // a. If numberingSystem cannot be matched by the type Unicode locale
  // nonterminal, throw a RangeError exception.
  // 12. Set opt.[[nu]] to numberingSystem.
  auto numberingSystemRes = getStringOption(
      runtime, options, constants::opt_name::numberingSystem, {}, std::nullopt);
  if (LLVM_UNLIKELY(numberingSystemRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  if (numberingSystemRes->has_value()) {
    if (!isUnicodeExtensionType(**numberingSystemRes)) {
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid numbering system: ") +
          vm::TwineChar16((*numberingSystemRes)->c_str()));
    }
    opt.emplace(constants::extension_key::nu, **numberingSystemRes);
  }

  // 13. Let hour12 be ? GetOption(options, "hour12", boolean, empty,
  // undefined).
  auto hour12 =
      getBoolOption(options, constants::opt_name::hour12, std::nullopt);

  // 14. Let hourCycle be ? GetOption(options, "hourCycle", string, « "h11",
  // "h12", "h23", "h24" », undefined).
  auto hourCycleRes = getStringOption(
      runtime,
      options,
      constants::opt_name::hourCycle,
      constants::opt_value::hour_cycle::validHourCycles,
      std::nullopt);
  if (LLVM_UNLIKELY(hourCycleRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }

  // 15. If hour12 is not undefined, then
  // a. Set hourCycle to null.
  // 16. Set opt.[[hc]] to hourCycle.
  // Implementation Note:
  // The spec is setting hourCycle to null here in order to get the effect of
  // overriding any -u-hc extension included in given locales in ResolveLocale()
  // We will implement that logic instead by checking for hour12 after
  // ResolveLocale() and delete the resolved hourCycle. Here we are only going
  // to pass valid hourCycle option to ResolveLocale if no hour12 option is set.
  if (!hour12.has_value() && hourCycleRes->has_value()) {
    opt.emplace(constants::extension_key::hc, **hourCycleRes);
  }

  // 17. Let localeData be %DateTimeFormat%.[[LocaleData]].
  // 18. Let r be ResolveLocale(%DateTimeFormat%.[[AvailableLocales]],
  // requestedLocales, opt, %DateTimeFormat%.[[RelevantExtensionKeys]],
  // localeData).
  static constexpr std::u16string_view relevantExtensionKeys[] = {
      constants::extension_key::ca,
      constants::extension_key::hc,
      constants::extension_key::nu};

  ResolvedResult result = resolveLocale(
      *requestedLocalesRes,
      opt,
      relevantExtensionKeys,
      isExtensionTypeSupported);

  LocaleBCP47Object resolvedBCP47Locale = result.localeBcp47Object;
  Options resolvedOpt = result.resolvedOpts;
  auto extensionMap = resolvedBCP47Locale.getExtensionMap();
  auto localeNoExtICU =
      convertBCP47toICULocale(resolvedBCP47Locale.getLocaleNoExt());

  // Implementation Note:
  // If hour12 option is set, hour12 overrides any resolved hc unicode
  // extension, so remove hc unicode extension from resolvedBCP47Locale and
  // resolvedOpt.
  if (hour12.has_value()) {
    extensionMap.erase(constants::extension_key::hc);
    resolvedBCP47Locale.updateExtensionMap(extensionMap);
    resolvedOpt.erase(constants::extension_key::hc);
  }

  // 19. Set dateTimeFormat.[[Locale]] to r.[[locale]].
  resolvedLocale_ = resolvedBCP47Locale.getCanonicalizedLocaleId();
  std::u16string internalLocale = resolvedLocale_;

  // 20. Let resolvedCalendar be r.[[ca]].
  // 21. Set dateTimeFormat.[[Calendar]] to resolvedCalendar.
  auto calendarEntry = resolvedOpt.find(constants::extension_key::ca);
  if (calendarEntry != resolvedOpt.end()) {
    resolvedCalendar_ = calendarEntry->second.getString();
    // The way to pass calendar option to ICU4C DateTimeFormat
    // is through setting 'ca' unicode extension in the locale used
    // when creating the instance.
    // resolvedLocale_ may not yet include calendar extension
    // if calendar is specified through options parameter.
    // Add the extension to internalLocale if that's the case.
    if (extensionMap.find(constants::extension_key::ca) == extensionMap.end()) {
      extensionMap.emplace(constants::extension_key::ca, resolvedCalendar_);
      resolvedBCP47Locale.updateExtensionMap(extensionMap);
      internalLocale = resolvedBCP47Locale.getCanonicalizedLocaleId();
    }
  } else {
    resolvedCalendar_ = getDefaultCalendar(localeNoExtICU);
  }

  // 22. Set dateTimeFormat.[[NumberingSystem]] to r.[[nu]].
  auto numberingSystemEntry = resolvedOpt.find(constants::extension_key::nu);
  if (numberingSystemEntry != resolvedOpt.end()) {
    resolvedNumberingSystem_ = numberingSystemEntry->second.getString();
    // The way to pass numbering system option to ICU4C DateTimeFormat
    // is through setting 'nu' unicode extension in the locale used
    // when creating the instance.
    // resolvedLocale_ may not yet include numbering system extension
    // if numbering system is specified through options parameter.
    // Add the extension to internalLocale if that's the case.
    if (extensionMap.find(constants::extension_key::nu) == extensionMap.end()) {
      extensionMap.emplace(
          constants::extension_key::nu, resolvedNumberingSystem_);
      resolvedBCP47Locale.updateExtensionMap(extensionMap);
      internalLocale = resolvedBCP47Locale.getCanonicalizedLocaleId();
    }
  } else {
    resolvedNumberingSystem_ = getDefaultNumberingSystem(localeNoExtICU);
  }

  // 23. Let dataLocale be r.[[localeData]].
  // 24. Let dataLocaleData be localeData.[[<dataLocale>]].
  // 25. If hour12 is true, then
  // a. Let hc be dataLocaleData.[[hourCycle12]].
  // 26. Else if hour12 is false, then
  // a. Let hc be dataLocaleData.[[hourCycle24]].
  // 27. Else,
  // a. Assert: hour12 is undefined.
  // b. Let hc be r.[[hc]].
  // c. If hc is null, set hc to dataLocaleData.[[hourCycle]].
  std::u16string hc;
  if (hour12.has_value()) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UDateTimePatternGenerator, decltype(&udatpg_close)>
        generator(udatpg_open(localeNoExtICU.c_str(), &status), &udatpg_close);
    std::u16string pattern(8, char16_t());
    if (*hour12) {
      // Default for 12-hour clock is h12.
      hc = constants::opt_value::hour_cycle::h12;
      if (U_SUCCESS(status) && generator) {
        if (getBestPattern(
                generator.get(), u"h", UDATPG_MATCH_NO_OPTIONS, pattern) &&
            findDateFieldSymbol(pattern, u'K').has_value()) {
          hc = constants::opt_value::hour_cycle::h11;
        }
      }
    } else {
      // Default for 24-hour clock is h23.
      hc = constants::opt_value::hour_cycle::h23;
      if (U_SUCCESS(status) && generator) {
        if (getBestPattern(
                generator.get(), u"H", UDATPG_MATCH_NO_OPTIONS, pattern) &&
            findDateFieldSymbol(pattern, u'k').has_value()) {
          hc = constants::opt_value::hour_cycle::h24;
        }
      }
    }
  } else {
    auto ptr = resolvedOpt.find(constants::extension_key::hc);
    if (ptr != resolvedOpt.end()) {
      hc = ptr->second.getString();
    } else {
      hc = getDefaultHourCycle(localeNoExtICU);
    }
  }

  // 28. Set dateTimeFormat.[[HourCycle]] to hc.
  resolvedHourCycle_ = hc;

  // For UDateIntervalFormat, there isn't a way to specify a date-time pattern
  // when constructing, rather only with date-time skeleton. The problem is
  // then the resolved hour cycle cannot be enforced through the pattern,
  // unlike with UDateFormat.
  //
  // ICU 67.1 has added support for the 'hc' unicode extension in locale, so
  // the hour cycle of UDateIntervalFormat can be controlled via the 'hc'
  // extension. See https://unicode-org.atlassian.net/browse/ICU-20887.
  //
  // Therefore, add 'hc' extension to internalLocale.
  if (resolvedHourCycle_.has_value() &&
      extensionMap.find(constants::extension_key::hc) == extensionMap.end()) {
    extensionMap.emplace(constants::extension_key::hc, *resolvedHourCycle_);
    resolvedBCP47Locale.updateExtensionMap(extensionMap);
    internalLocale = resolvedBCP47Locale.getCanonicalizedLocaleId();
  }

  // 29. Let timeZone be ? Get(options, "timeZone").
  // 30. If timeZone is undefined, then
  // a. Set timeZone to systemTimeZoneIdentifier().
  // 31. Else,
  // a. Set timeZone to ? ToString(timeZone).
  auto timeZoneRes = getStringOption(
      runtime, options, constants::opt_name::timeZone, {}, std::nullopt);
  if (LLVM_UNLIKELY(timeZoneRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  std::u16string internalTimeZone;
  if (!timeZoneRes->has_value()) {
    internalTimeZone = systemTimeZoneIdentifier();
    if (startsWith(internalTimeZone, u"GMT+") ||
        startsWith(internalTimeZone, u"GMT-")) {
      vm::CallResult<std::u16string> offsetRes =
          formatOffsetTimeZoneIdentifier(runtime, internalTimeZone);
      if (LLVM_UNLIKELY(offsetRes == vm::ExecutionStatus::EXCEPTION)) {
        // Set time zone to UTC in case system time zone is an offset time zone
        // id that is not allowed by the spec.
        resolvedTimeZone_ = u"UTC";
        internalTimeZone = u"UTC";
      } else {
        resolvedTimeZone_ = *offsetRes;
      }
    } else {
      resolvedTimeZone_ = internalTimeZone;
    }
  } else {
    // Implementation Note : We are keeping implemenation of steps 32-35 inside
    // else block as the check done in isTimeZoneOffsetString() and
    // isValidTimeZoneName() does not need to apply to system time zone.

    // 32. If IsTimeZoneOffsetString(timeZone) is true, then
    // a. Let parseResult be ParseText(StringToCodePoints(timeZone), UTCOffset).
    // b. Assert: parseResult is a Parse Node.
    // c. If parseResult contains more than one MinuteSecond Parse Node, throw a
    // RangeError exception.
    // d. Let offsetNanoseconds be ParseTimeZoneOffsetString(timeZone).
    // e. Let offsetMinutes be offsetNanoseconds / (6 × 10**10).
    // f. Assert: offsetMinutes is an integer.
    // g. Set timeZone to FormatOffsetTimeZoneIdentifier(offsetMinutes).
    // 33. Else if IsValidTimeZoneName(timeZone) is true, then
    // a. Set timeZone to CanonicalizeTimeZoneName(timeZone).
    // 34. Else,
    // a. Throw a RangeError exception.
    // 35. Set dateTimeFormat.[[TimeZone]] to timeZone.
    if (isTimeZoneOffsetString(**timeZoneRes, internalTimeZone) &&
        startsWith(internalTimeZone, u"GMT")) {
      vm::CallResult<std::u16string> offsetRes =
          formatOffsetTimeZoneIdentifier(runtime, internalTimeZone);
      if (LLVM_UNLIKELY(offsetRes == vm::ExecutionStatus::EXCEPTION)) {
        return vm::ExecutionStatus::EXCEPTION;
      }
      resolvedTimeZone_ = *offsetRes;
    } else if (isValidTimeZoneName(**timeZoneRes, internalTimeZone)) {
      resolvedTimeZone_ = internalTimeZone;
    } else {
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid time zone: ") +
          vm::TwineChar16((*timeZoneRes)->c_str()));
    }
  }

  // 36. Let formatOptions be a new Record.
  // 37. Set formatOptions.[[hourCycle]] to hc.
  // 38. Let hasExplicitFormatComponents be false.
  bool hasExplicitFormatComponents = false;
  // 39. For each row of Table 7, except the header row, in table order, do
  // a. Let prop be the name given in the Property column of the row.
  // b. If prop is "fractionalSecondDigits", then
  //   i. Let value be ? GetNumberOption(options, "fractionalSecondDigits", 1,
  //   3, undefined).
  // c. Else,
  //   i. Let values be a List whose elements are the strings given in the
  //   Values column of the row.
  //   ii. Let value be ? GetOption(options, prop, STRING, values, undefined).
  // d. Set formatOptions.[[<prop>]] to value.
  // e. If value is not undefined, then
  //   i. Set hasExplicitFormatComponents to true.
  for (auto &formatComponent : dateTimeFormatComponents_) {
    std::u16string prop(formatComponent.property_);
    if (prop == constants::opt_name::fractionalSecondDigits) {
      auto fractionalSecondDigitsRes =
          getNumberOption(runtime, options, prop, 1, 3, std::nullopt);
      if (LLVM_UNLIKELY(
              fractionalSecondDigitsRes == vm::ExecutionStatus::EXCEPTION)) {
        return vm::ExecutionStatus::EXCEPTION;
      }
      if (fractionalSecondDigitsRes->has_value()) {
        formatComponent.resolvedValue_ = **fractionalSecondDigitsRes;
        hasExplicitFormatComponents = true;
      }
    } else {
      auto valueRes = getStringOption(
          runtime, options, prop, formatComponent.values_, std::nullopt);
      if (LLVM_UNLIKELY(valueRes == vm::ExecutionStatus::EXCEPTION)) {
        return vm::ExecutionStatus::EXCEPTION;
      }
      if (valueRes->has_value()) {
        formatComponent.resolvedValue_ = **valueRes;
        hasExplicitFormatComponents = true;
      }
    }
  }

  // 40. Let formatMatcher be ? GetOption(options, "formatMatcher", STRING, «
  // "basic", "best fit" », "best fit").
  auto formatMatcherRes = getStringOption(
      runtime,
      options,
      constants::opt_name::formatMatcher,
      constants::opt_value::format_matcher::validFormatMatchers,
      constants::opt_value::format_matcher::bestFit);
  if (LLVM_UNLIKELY(formatMatcherRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }

  // 41. Let dateStyle be ? GetOption(options, "dateStyle", STRING, « "full",
  // "long", "medium", "short" », undefined).
  // 42. Set dateTimeFormat.[[DateStyle]] to dateStyle.
  auto dateStyleRes = getStringOption(
      runtime,
      options,
      constants::opt_name::dateStyle,
      constants::opt_value::style::validDateTimeStyles,
      std::nullopt);
  if (LLVM_UNLIKELY(dateStyleRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  resolvedDateStyle_ = *dateStyleRes;

  // 43. Let timeStyle be ? GetOption(options, "timeStyle", STRING, « "full",
  // "long", "medium", "short" », undefined).
  // 44. Set dateTimeFormat.[[TimeStyle]] to timeStyle.
  auto timeStyleRes = getStringOption(
      runtime,
      options,
      constants::opt_name::timeStyle,
      constants::opt_value::style::validDateTimeStyles,
      std::nullopt);
  if (LLVM_UNLIKELY(timeStyleRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  resolvedTimeStyle_ = *timeStyleRes;

  std::string localeICU = convertBCP47toICULocale(internalLocale);
  std::u16string bestFormatPattern;
  std::u16string bestFormatSkeleton;
  // 45. If dateStyle is not undefined or timeStyle is not undefined, then
  if (resolvedDateStyle_.has_value() || resolvedTimeStyle_.has_value()) {
    // a. If hasExplicitFormatComponents is true, then
    // i. Throw a TypeError exception.
    if (hasExplicitFormatComponents == true) {
      return runtime.raiseTypeError(
          "dateStyle and timeStyle cannot be specified with other date-time component options");
    }
    // b. If required is DATE and timeStyle is not undefined, then
    // i. Throw a TypeError exception.
    if (requiredComponent == DateTimeComponent::DATE &&
        resolvedTimeStyle_.has_value()) {
      return runtime.raiseTypeError(
          "timeStyle cannot be specified for date only format");
    }
    // c. If required is TIME and dateStyle is not undefined, then
    // i. Throw a TypeError exception.
    if (requiredComponent == DateTimeComponent::TIME &&
        resolvedDateStyle_.has_value()) {
      return runtime.raiseTypeError(
          "dateStyle cannot be specified for time only format");
    }
    // d. Let styles be dataLocaleData.[[styles]].[[<resolvedCalendar>]].
    // e. Let bestFormat be DateTimeStyleFormat(dateStyle, timeStyle, styles).
    //
    // Implementation Note: implementation relies on ICU DateFormat instance
    // constructed according to specified dateStyle and timeStyle in place of
    // DateTimeStyleFormat().
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UDateFormat, decltype(&udat_close)> dateFormat(
        udat_open(
            dateTimeStyleToICUDateFormatStyle(resolvedTimeStyle_),
            dateTimeStyleToICUDateFormatStyle(resolvedDateStyle_),
            localeICU.c_str(),
            nullptr,
            0,
            nullptr,
            0,
            &status),
        &udat_close);
    if (U_SUCCESS(status) && dateFormat) {
      bestFormatPattern.resize(64);
      int32_t bufferSize = bestFormatPattern.size();
      int32_t resultLength = 0;
      while ((resultLength = udat_toPattern(
                  dateFormat.get(),
                  false,
                  bestFormatPattern.data(),
                  bufferSize,
                  &status)) > bufferSize) {
        status = U_ZERO_ERROR;
        bestFormatPattern.resize(resultLength);
        bufferSize = bestFormatPattern.size();
      }
      bestFormatPattern.resize(resultLength);
    }

    bestFormatSkeleton.resize(bestFormatPattern.size());
    int32_t bufferSize = bestFormatSkeleton.size();
    int32_t resultLength = 0;
    while ((resultLength = udatpg_getBaseSkeleton(
                nullptr,
                bestFormatPattern.data(),
                bestFormatPattern.size(),
                bestFormatSkeleton.data(),
                bufferSize,
                &status)) > bufferSize) {
      status = U_ZERO_ERROR;
      bestFormatSkeleton.resize(resultLength);
      bufferSize = bestFormatSkeleton.size();
    }
    bestFormatSkeleton.resize(resultLength);
    // remove symbols 'a','b'and 'B'
    cleanUpBaseSkeleton(bestFormatSkeleton);

    // If the generated pattern's hour cycle does not match against
    // resolvedHourCycle_, replace hour cycle symbol in skeleton with
    // resolvedHourCycle_'s symbol, and re-generate the best pattern
    // from the modified skeleton.
    if ((resolvedTimeStyle_.has_value()) &&
        ((resolvedHourCycle_.has_value()))) {
      auto resultOpt = findHourSymbol(bestFormatPattern);
      char16_t desiredHourSymbol = getHourSymbol(*resolvedHourCycle_);
      if (resultOpt.has_value() && resultOpt->symbol != desiredHourSymbol) {
        auto findResultOpt = findHourSymbol(bestFormatSkeleton);
        if (findResultOpt.has_value()) {
          bestFormatSkeleton.replace(
              findResultOpt->startPos,
              findResultOpt->count,
              findResultOpt->count,
              desiredHourSymbol);
          std::unique_ptr<UDateTimePatternGenerator, decltype(&udatpg_close)>
              generator(udatpg_open(localeICU.c_str(), &status), &udatpg_close);
          std::u16string pattern(bestFormatPattern.size() + 8, char16_t());
          if (U_SUCCESS(status) && generator &&
              getBestPattern(
                  generator.get(),
                  bestFormatSkeleton,
                  UDATPG_MATCH_NO_OPTIONS,
                  pattern)) {
            bestFormatPattern = pattern;
          }
        }
      }
    }
  } else {
    // 46. Else,
    // a. Let needDefaults be true.
    bool needDefaults = true;
    // b. If required is DATE or ANY, then
    // i. For each property name prop of « "weekday", "year", "month", "day" »,
    // do
    //   1. Let value be formatOptions.[[<prop>]].
    //   2. If value is not undefined, set needDefaults to false.
    if (requiredComponent == DateTimeComponent::DATE ||
        requiredComponent == DateTimeComponent::ANY) {
      for (const auto &formatComponent : dateTimeFormatComponents_) {
        if ((formatComponent.property_ == constants::opt_name::weekday ||
             formatComponent.property_ == constants::opt_name::year ||
             formatComponent.property_ == constants::opt_name::month ||
             formatComponent.property_ == constants::opt_name::day) &&
            formatComponent.resolvedValue_.has_value()) {
          needDefaults = false;
          break;
        }
      }
    }

    // c. If required is TIME or ANY, then
    // i. For each property name prop of « "dayPeriod", "hour", "minute",
    // "second", "fractionalSecondDigits" », do
    //   1. Let value be formatOptions.[[<prop>]].
    //   2. If value is not undefined, set needDefaults to false.
    if (requiredComponent == DateTimeComponent::TIME ||
        requiredComponent == DateTimeComponent::ANY) {
      for (const auto &formatComponent : dateTimeFormatComponents_) {
        if ((formatComponent.property_ == constants::opt_name::dayPeriod ||
             formatComponent.property_ == constants::opt_name::hour ||
             formatComponent.property_ == constants::opt_name::minute ||
             formatComponent.property_ == constants::opt_name::second ||
             formatComponent.property_ ==
                 constants::opt_name::fractionalSecondDigits) &&
            formatComponent.resolvedValue_.has_value()) {
          needDefaults = false;
          break;
        }
      }
    }

    // d. If needDefaults is true and defaults is either DATE or ALL, then
    // i. For each property name prop of « "year", "month", "day" », do
    // 1. Set formatOptions.[[<prop>]] to "numeric".
    if (needDefaults &&
        (defaultComponent == DateTimeComponent::DATE ||
         defaultComponent == DateTimeComponent::ALL)) {
      for (auto &formatComponent : dateTimeFormatComponents_) {
        if (formatComponent.property_ == constants::opt_name::year ||
            formatComponent.property_ == constants::opt_name::month ||
            formatComponent.property_ == constants::opt_name::day) {
          formatComponent.resolvedValue_ =
              std::u16string(constants::opt_value::style::numeric);
        }
      }
    }

    // e. If needDefaults is true and defaults is either TIME or ALL, then
    // i. For each property name prop of « "hour", "minute", "second" », do
    // 1. Set formatOptions.[[<prop>]] to "numeric".
    if (needDefaults &&
        (defaultComponent == DateTimeComponent::TIME ||
         defaultComponent == DateTimeComponent::ALL)) {
      for (auto &formatComponent : dateTimeFormatComponents_) {
        if (formatComponent.property_ == constants::opt_name::hour ||
            formatComponent.property_ == constants::opt_name::minute ||
            formatComponent.property_ == constants::opt_name::second) {
          formatComponent.resolvedValue_ =
              std::u16string(constants::opt_value::style::numeric);
        }
      }
    }

    // f. Let formats be dataLocaleData.[[formats]].[[<resolvedCalendar>]].
    // g. If formatMatcher is "basic", then
    // i. Let bestFormat be BasicFormatMatcher(formatOptions, formats).
    // h. Else,
    // i. Let bestFormat be BestFitFormatMatcher(formatOptions, formats).
    //
    // Implementation Note: Only best fit format matcher is implemented and it
    // relies on creating a skeleton based on the specified date-time components
    // and then getting the best pattern from the skeleton using ICU.
    bestFormatSkeleton = buildSkeleton();
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UDateTimePatternGenerator, decltype(&udatpg_close)>
        generator(udatpg_open(localeICU.c_str(), &status), &udatpg_close);
    std::u16string pattern(bestFormatSkeleton.size() + 32, char16_t());
    if (U_SUCCESS(status) && generator &&
        getBestPattern(
            generator.get(),
            bestFormatSkeleton,
            UDATPG_MATCH_ALL_FIELDS_LENGTH,
            pattern)) {
      bestFormatPattern = pattern;
    }
  }

  // 47. For each row of Table 7, except the header row, in table order, do
  // a. Let prop be the name given in the Property column of the current row.
  // b. If bestFormat has a field [[<prop>]], then
  //   i. Let p be bestFormat.[[<prop>]].
  //   ii. Set dateTimeFormat's internal slot whose name is the Internal Slot
  //   column of the current row to p.
  //
  // Implementation Note : Step 47 can be skipped because the best pattern
  // generated by ICU respects all the styles from the skeleton as specified
  // from date-time component input options.

  // Replace hour cycle symbol in the resolvedPattern if it doesn't match
  // resolvedHourCycle_'s symbol.
  if (resolvedHourCycle_.has_value()) {
    char16_t desiredHourSymbol = getHourSymbol(*resolvedHourCycle_);
    auto resultOpt = findHourSymbol(bestFormatPattern);

    // 48. If dateTimeFormat.[[Hour]] is undefined, then
    // a. Set dateTimeFormat.[[HourCycle]] to undefined.
    if (!resultOpt.has_value()) {
      resolvedHourCycle_.reset();
    } else if (resultOpt->symbol != desiredHourSymbol) {
      bestFormatPattern.replace(
          resultOpt->startPos,
          resultOpt->count,
          resultOpt->count,
          desiredHourSymbol);
    }
  }

  // 49. If dateTimeFormat.[[HourCycle]] is "h11" or "h12", then
  // a. Let pattern be bestFormat.[[pattern12]].
  // b. Let rangePatterns be bestFormat.[[rangePatterns12]].
  // 50. Else,
  // a. Let pattern be bestFormat.[[pattern]].
  // b. Let rangePatterns be bestFormat.[[rangePatterns]].
  // 51. Set dateTimeFormat.[[Pattern]] to pattern.
  // 52. Set dateTimeFormat.[[RangePatterns]] to rangePatterns.
  // 53. Return dateTimeFormat.
  UErrorCode status = U_ZERO_ERROR;
  dateFormat_.reset(udat_open(
      UDAT_PATTERN,
      UDAT_PATTERN,
      localeICU.c_str(),
      internalTimeZone.data(),
      internalTimeZone.size(),
      bestFormatPattern.data(),
      bestFormatPattern.size(),
      &status));
  if (U_FAILURE(status) || !dateFormat_) {
    return runtime.raiseError(
        "Internal error: unable to create DateTimeFormat instance");
  }

  dateIntervalFormat_.reset(udtitvfmt_open(
      localeICU.c_str(),
      bestFormatSkeleton.data(),
      bestFormatSkeleton.size(),
      internalTimeZone.data(),
      internalTimeZone.size(),
      &status));
  if (U_FAILURE(status) || !dateIntervalFormat_) {
    return runtime.raiseError(
        "Internal error: unable to create DateTimeFormat instance for range formatting");
  }

  return vm::ExecutionStatus::RETURNED;
}

std::u16string DateTimeFormat::systemTimeZoneIdentifier() {
  UErrorCode status = U_ZERO_ERROR;
  std::u16string defaultTimeZone(32, char16_t());
  int32_t bufferSize = defaultTimeZone.size();
  int32_t resultLength = 0;
  while ((resultLength = ucal_getDefaultTimeZone(
              defaultTimeZone.data(), bufferSize, &status)) > bufferSize) {
    status = U_ZERO_ERROR;
    defaultTimeZone.resize(resultLength);
    bufferSize = defaultTimeZone.size();
  }
  defaultTimeZone.resize(resultLength);
  if (U_FAILURE(status) || defaultTimeZone == u"Etc/Unknown") {
    return u"UTC";
  }
  return canonicalizeTimeZoneName(defaultTimeZone);
}

const std::unordered_map<std::u16string, std::u16string> &
DateTimeFormat::getValidTimeZones() {
  // Intentionally leaked to avoid destruction order problems.
  static const auto *validTimeZones = [] {
    auto *validZones = new std::unordered_map<std::u16string, std::u16string>();
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UEnumeration, decltype(&uenum_close)> timeZones(
        ucal_openTimeZones(&status), &uenum_close);
    if (U_FAILURE(status) || !timeZones) {
      return validZones;
    }
    // Only IANA time zone identifiers are allowed by the spec.
    // See test262/test/intl402/DateTimeFormat/timezone-legacy-non-iana.js
    // Below is the list of non-IANA link names that ICU accepts.
    // Exclude adding the time zones in this list to validZones.
    constexpr std::u16string_view namesToExclude[] = {
        u"ACT", u"AET", u"AGT", u"ART", u"AST", u"BET", u"BST", u"CAT", u"CNT",
        u"CST", u"CTT", u"EAT", u"ECT", u"IET", u"IST", u"JST", u"MIT", u"NET",
        u"NST", u"PLT", u"PNT", u"PRT", u"PST", u"SST", u"VST"};
    const auto *namesToExcludeBegin = std::begin(namesToExclude);
    const auto *namesToExcludeEnd = std::end(namesToExclude);
    int32_t length;
    const UChar *timeZoneId;
    while ((timeZoneId = uenum_unext(timeZones.get(), &length, &status)) !=
               nullptr &&
           U_SUCCESS(status)) {
      std::u16string timeZoneIdStr(timeZoneId, length);
      if (std::find(namesToExcludeBegin, namesToExcludeEnd, timeZoneIdStr) ==
          namesToExcludeEnd) {
        validZones->emplace(
            toUpperASCII(timeZoneIdStr),
            canonicalizeTimeZoneName(timeZoneIdStr));
      }
    }
    validZones->emplace(u"UTC", u"UTC");
    return validZones;
  }();
  return *validTimeZones;
}

bool DateTimeFormat::isValidTimeZoneName(
    const std::u16string &timeZone,
    std::u16string &canonicalizedTimeZoneResult) {
  const std::unordered_map<std::u16string, std::u16string> &validTimeZones =
      getValidTimeZones();
  auto it = validTimeZones.find(toUpperASCII(timeZone));
  if (it != validTimeZones.end()) {
    canonicalizedTimeZoneResult = it->second;
    return true;
  }
  return false;
}

bool DateTimeFormat::isTimeZoneOffsetString(
    const std::u16string &timeZone,
    std::u16string &canonicalizedTimeZoneResult) {
  // Check if timeZone starts with "+", "-", or '\u2212'
  if (timeZone.empty() ||
      (timeZone[0] != u'+' && timeZone[0] != u'-' &&
       timeZone[0] != u'\u2212')) {
    return false;
  }
  if (timeZone.find(u':') == std::u16string::npos) {
    if (timeZone.size() != 3 && timeZone.size() != 5) {
      return false;
    }
  } else if (timeZone.size() != 6) {
    return false;
  }
  std::u16string offsetTimeZoneId = timeZone;
  // If offsetTimeZoneId starts with "\u2212", replace it with "-"
  if (offsetTimeZoneId[0] == u'\u2212') {
    offsetTimeZoneId[0] = u'-';
  }
  // Prepend "GMT" to offsetTimeZoneId
  offsetTimeZoneId.insert(0, u"GMT");
  auto resultOpt = getCanonicalTimeZoneId(offsetTimeZoneId);
  if (!resultOpt.has_value()) {
    return false;
  }
  canonicalizedTimeZoneResult = *resultOpt;
  if (canonicalizedTimeZoneResult == u"GMT") {
    canonicalizedTimeZoneResult.append(u"+00:00");
  }
  return true;
}

std::u16string DateTimeFormat::canonicalizeTimeZoneName(
    const std::u16string &timeZoneName) {
  std::u16string result;
  // ECMA-402 requires the canonical time zone name to be the ones defined by
  // IANA. Call ucal_getIanaTimeZoneID(), but it's only available since
  // ICU 74.
#if U_ICU_VERSION_MAJOR_NUM >= 74
  UErrorCode status = U_ZERO_ERROR;
  std::u16string ianaId(32, char16_t());
  int32_t bufferSize = ianaId.size();
  int32_t resultLength = 0;
  while ((resultLength = ucal_getIanaTimeZoneID(
              timeZoneName.data(),
              timeZoneName.size(),
              ianaId.data(),
              bufferSize,
              &status)) > bufferSize) {
    status = U_ZERO_ERROR;
    ianaId.resize(resultLength);
    bufferSize = ianaId.size();
  }
  ianaId.resize(resultLength);
  if (U_FAILURE(status)) {
    return u"UTC";
  }
  result = ianaId;
#else
  auto resultOpt = getCanonicalTimeZoneId(timeZoneName);
  if (!resultOpt.has_value()) {
    return u"UTC";
  }
  result = *resultOpt;
#endif
  // If the result is "Etc/UTC", "Etc/GMT", or "GMT", return "UTC"
  if (result == u"Etc/UTC" || result == u"Etc/GMT" || result == u"GMT") {
    return u"UTC";
  }
  return result;
}

std::optional<std::u16string> DateTimeFormat::getCanonicalTimeZoneId(
    const std::u16string &timeZoneId) {
  UErrorCode status = U_ZERO_ERROR;
  std::u16string canonicalId(32, char16_t());
  int32_t bufferSize = canonicalId.size();
  int32_t resultLength = 0;
  while ((resultLength = ucal_getCanonicalTimeZoneID(
              timeZoneId.data(),
              timeZoneId.size(),
              canonicalId.data(),
              bufferSize,
              nullptr,
              &status)) > bufferSize) {
    status = U_ZERO_ERROR;
    canonicalId.resize(resultLength);
    bufferSize = canonicalId.size();
  }
  canonicalId.resize(resultLength);
  if (U_FAILURE(status)) {
    return std::nullopt;
  }
  return canonicalId;
}

/**
 * Converts ICU offset time zone id in the form GMT+/-hh:mm[:ss] to ECMA-402
 * offset time zone id format +/-hh:mm.
 */
vm::CallResult<std::u16string> DateTimeFormat::formatOffsetTimeZoneIdentifier(
    vm::Runtime &runtime,
    const std::u16string &icuOffsetTimeZone) noexcept {
  // Seconds field in the offset is not allowed in the spec.
  // Check for seconds field in the offset time zone id.
  // Throw a RangeError exception if seconds field is present.
  if (icuOffsetTimeZone.length() > 9) {
    return runtime.raiseRangeError(
        "Offset time zone id cannot contain seconds field.");
  }
  // Remove "GMT" from the beginning
  if (icuOffsetTimeZone.size() >= 3) {
    return icuOffsetTimeZone.substr(3);
  }
  return icuOffsetTimeZone;
}

bool DateTimeFormat::isExtensionTypeSupported(
    std::u16string_view extensionKey,
    std::u16string_view extensionType,
    const LocaleBCP47Object &localeBCP47Object) {
  if (extensionKey == constants::extension_key::nu) {
    const std::unordered_set<std::u16string> &numberingSystems =
        getAvailableNumberingSystems();
    return numberingSystems.find(std::u16string(extensionType)) !=
        numberingSystems.end();
  }

  if (extensionKey == constants::extension_key::hc) {
    return std::find(
               std::begin(constants::opt_value::hour_cycle::validHourCycles),
               std::end(constants::opt_value::hour_cycle::validHourCycles),
               extensionType) !=
        std::end(constants::opt_value::hour_cycle::validHourCycles);
  }

  if (extensionKey == constants::extension_key::ca) {
    std::unordered_set<std::u16string> calendars = getAvailableCalendars(
        convertBCP47toICULocale(localeBCP47Object.getLocaleNoExt()));
    return calendars.find(std::u16string(extensionType)) != calendars.end();
  }

  return false;
}

std::u16string DateTimeFormat::getDefaultCalendar(
    const std::string &localeICU) {
  UErrorCode status = U_ZERO_ERROR;
  std::u16string defaultCalendar{u"gregory"};
  std::unique_ptr<UCalendar, decltype(&ucal_close)> calendar(
      ucal_open(nullptr, -1, localeICU.c_str(), UCAL_DEFAULT, &status),
      ucal_close);
  if (U_FAILURE(status) || !calendar) {
    return defaultCalendar;
  }
  std::string_view calType(ucal_getType(calendar.get(), &status));
  if (U_FAILURE(status) || calType.empty() || calType == "unknown") {
    return defaultCalendar;
  }
  return convertLegacyCalendar(calType);
}

std::unordered_set<std::u16string> DateTimeFormat::getAvailableCalendars(
    const std::string &localeICU) {
  std::unordered_set<std::u16string> availableCalendars;
  UErrorCode status = U_ZERO_ERROR;
  std::unique_ptr<UEnumeration, decltype(&uenum_close)> names(
      ucal_getKeywordValuesForLocale(
          "calendar", localeICU.c_str(), false, &status),
      &uenum_close);
  if (U_FAILURE(status) || !names) {
    return availableCalendars;
  }
  int32_t length;
  const char *name;
  while ((name = uenum_next(names.get(), &length, &status)) != nullptr &&
         U_SUCCESS(status)) {
    std::string_view nameStr(name, length);
    availableCalendars.emplace(convertLegacyCalendar(nameStr));
  }
  return availableCalendars;
}

std::u16string DateTimeFormat::convertLegacyCalendar(std::string_view cal) {
  if (cal == "gregorian") {
    return u"gregory";
  } else if (cal == "ethiopic-amete-alem") {
    return u"ethioaa";
  } else {
    return toUTF16ASCII(cal);
  }
}

char16_t DateTimeFormat::getHourSymbol(const std::u16string &hourCycle) {
  if (hourCycle == constants::opt_value::hour_cycle::h11) {
    return u'K';
  } else if (hourCycle == constants::opt_value::hour_cycle::h12) {
    return u'h';
  } else if (hourCycle == constants::opt_value::hour_cycle::h23) {
    return u'H';
  } else if (hourCycle == constants::opt_value::hour_cycle::h24) {
    return u'k';
  } else {
    return u'H';
  }
}

std::u16string DateTimeFormat::getDefaultHourCycle(
    const std::string &localeICU) {
  // Default to 24-hour cycle in case of an error
  auto defaultHourCycle = constants::opt_value::hour_cycle::h23;
  UErrorCode status = U_ZERO_ERROR;
  std::unique_ptr<UDateTimePatternGenerator, decltype(&udatpg_close)> generator(
      udatpg_open(localeICU.c_str(), &status), &udatpg_close);
  if (U_FAILURE(status) || !generator) {
    return defaultHourCycle;
  }
  std::u16string pattern(8, char16_t());
  if (!getBestPattern(
          generator.get(), u"j", UDATPG_MATCH_NO_OPTIONS, pattern)) {
    return defaultHourCycle;
  }
  auto resultOpt = findHourSymbol(pattern);
  if (!resultOpt.has_value()) {
    return defaultHourCycle;
  }
  switch (resultOpt->symbol) {
    case u'K':
      return constants::opt_value::hour_cycle::h11;
    case u'h':
      return constants::opt_value::hour_cycle::h12;
    case u'H':
      return constants::opt_value::hour_cycle::h23;
    case u'k':
      return constants::opt_value::hour_cycle::h24;
    default:
      return defaultHourCycle;
  }
}

std::optional<DateTimeFormat::SymbolFindResult> DateTimeFormat::findHourSymbol(
    const std::u16string &pattern) {
  for (char16_t symbol : {u'k', u'h', u'H', u'K', u'j'}) {
    auto resultOpt = findDateFieldSymbol(pattern, symbol);
    if (resultOpt.has_value()) {
      return resultOpt;
    }
  }
  return std::nullopt;
};

std::optional<DateTimeFormat::SymbolFindResult>
DateTimeFormat::findDateFieldSymbol(
    const std::u16string &pattern,
    char16_t symbol) {
  // Single quote.
  const char16_t quote = u'\u0027';
  SymbolFindResult result;
  bool found = false;
  bool inQuote = false;
  int32_t length = pattern.length();
  for (int32_t i = 0; i < length; i++) {
    char16_t ch = pattern[i];

    if (ch != symbol && result.count > 0) {
      break;
    }

    if (ch == quote) {
      // Consecutive single quotes are a single quote literal,
      // either outside of quotes or between quotes.
      if ((i + 1) < length && pattern[i + 1] == quote) {
        i += 1;
      } else {
        inQuote = !inQuote;
      }
    } else if (!inQuote && ch == symbol) {
      if (result.startPos == -1) {
        found = true;
        result.symbol = symbol;
        result.startPos = i;
      }
      result.count++;
    }
  }
  return found ? std::make_optional(result) : std::nullopt;
}

UDateFormatStyle DateTimeFormat::dateTimeStyleToICUDateFormatStyle(
    const std::optional<std::u16string> &styleOpt) {
  if (!styleOpt.has_value()) {
    return UDAT_NONE;
  }
  auto style = *styleOpt;
  if (style == constants::opt_value::style::full) {
    return UDAT_FULL;
  }
  if (style == constants::opt_value::style::longStr) {
    return UDAT_LONG;
  }
  if (style == constants::opt_value::style::medium) {
    return UDAT_MEDIUM;
  }
  if (style == constants::opt_value::style::shortStr) {
    return UDAT_SHORT;
  }
  return UDAT_DEFAULT;
}

// According to https://unicode-org.atlassian.net/browse/ICU-20437, there is a
// ICU bug with getBaseSkeleton that the symbols 'a', 'b', and 'B', are still
// present in the skeleton result. This function is to remove these symbols from
// the base skeleton.
void DateTimeFormat::cleanUpBaseSkeleton(std::u16string &skeleton) {
  for (char16_t symbol : {u'a', u'b', u'B'}) {
    // Remove 'a', 'b', and 'B'
    auto resultOpt = findDateFieldSymbol(skeleton, symbol);
    if (resultOpt.has_value()) {
      skeleton.erase(resultOpt->startPos, resultOpt->count);
    }
  }
}

std::u16string DateTimeFormat::buildSkeleton() {
  std::u16string skeleton;
  for (const auto &formatComponent : dateTimeFormatComponents_) {
    if (!formatComponent.resolvedValue_.has_value()) {
      continue;
    }
    Option value = *(formatComponent.resolvedValue_);
    char16_t symbol = formatComponent.icuSkeletonSymbol;
    if (formatComponent.property_ ==
        constants::opt_name::fractionalSecondDigits) {
      skeleton.append(value.getNumber(), symbol);
      continue;
    }
    // All other values are strings.
    auto valueStr = value.getString();
    if (formatComponent.property_ == constants::opt_name::timeZoneName) {
      if (valueStr == constants::opt_value::style::shortStr) {
        skeleton.append(u"z");
      } else if (valueStr == constants::opt_value::style::longStr) {
        skeleton.append(u"zzzz");
      } else if (valueStr == constants::opt_value::style::shortOffset) {
        skeleton.append(u"O");
      } else if (valueStr == constants::opt_value::style::longOffset) {
        skeleton.append(u"OOOO");
      } else if (valueStr == constants::opt_value::style::shortGeneric) {
        skeleton.append(u"v");
      } else if (valueStr == constants::opt_value::style::longGeneric) {
        skeleton.append(u"vvvv");
      }
      continue;
    }
    if (formatComponent.property_ == constants::opt_name::hour) {
      symbol = getHourSymbol(resolvedHourCycle_.value_or(u""));
    }
    int numSymbols = 0;
    if (valueStr == constants::opt_value::style::numeric) {
      numSymbols = 1;
    } else if (valueStr == constants::opt_value::style::twoDigit) {
      numSymbols = 2;
    } else if (valueStr == constants::opt_value::style::shortStr) {
      numSymbols = 3;
    } else if (valueStr == constants::opt_value::style::longStr) {
      numSymbols = 4;
    } else if (valueStr == constants::opt_value::style::narrow) {
      numSymbols = 5;
    }
    skeleton.append(numSymbols, symbol);
  }
  return skeleton;
}

bool DateTimeFormat::getBestPattern(
    UDateTimePatternGenerator *generator,
    std::u16string_view skeleton,
    UDateTimePatternMatchOptions options,
    std::u16string &bestPatternResult) {
  UErrorCode status = U_ZERO_ERROR;
  int32_t bufferSize = bestPatternResult.size();
  int32_t resultLength = 0;
  while ((resultLength = udatpg_getBestPatternWithOptions(
              generator,
              skeleton.data(),
              skeleton.size(),
              options,
              bestPatternResult.data(),
              bufferSize,
              &status)) > bufferSize) {
    status = U_ZERO_ERROR;
    bestPatternResult.resize(resultLength);
    bufferSize = bestPatternResult.size();
  }
  bestPatternResult.resize(resultLength);
  return U_SUCCESS(status);
}

// As mentioned in
// https://tc39.es/ecma402/#table-datetimeformat-resolvedoptions-properties
Options DateTimeFormat::resolvedOptions() noexcept {
  Options finalResolvedOptions;

  finalResolvedOptions.emplace(
      constants::opt_name::locale, Option(resolvedLocale_));
  finalResolvedOptions.emplace(
      constants::opt_name::calendar, Option(resolvedCalendar_));
  finalResolvedOptions.emplace(
      constants::opt_name::numberingSystem, Option(resolvedNumberingSystem_));
  finalResolvedOptions.emplace(
      constants::opt_name::timeZone, Option(resolvedTimeZone_));
  if (resolvedHourCycle_.has_value()) {
    finalResolvedOptions.emplace(
        constants::opt_name::hourCycle, Option(*resolvedHourCycle_));
    finalResolvedOptions.emplace(
        constants::opt_name::hour12,
        Option(
            (*resolvedHourCycle_ == constants::opt_value::hour_cycle::h11 ||
             *resolvedHourCycle_ == constants::opt_value::hour_cycle::h12)));
  }
  if (!resolvedDateStyle_.has_value() && !resolvedTimeStyle_.has_value()) {
    for (const auto &formatComponent : dateTimeFormatComponents_) {
      if (formatComponent.resolvedValue_.has_value()) {
        finalResolvedOptions.emplace(
            formatComponent.property_,
            Option(*(formatComponent.resolvedValue_)));
      }
    }
  } else {
    if (resolvedDateStyle_.has_value()) {
      finalResolvedOptions.emplace(
          constants::opt_name::dateStyle, Option(*resolvedDateStyle_));
    }
    if (resolvedTimeStyle_.has_value()) {
      finalResolvedOptions.emplace(
          constants::opt_name::timeStyle, Option(*resolvedTimeStyle_));
    }
  }

  return finalResolvedOptions;
}

vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return supportedLocales(runtime, locales, options);
}

std::u16string DateTimeFormat::format(double value) noexcept {
  UErrorCode status = U_ZERO_ERROR;
  std::u16string output(128, char16_t());
  int32_t bufferSize = output.size();
  int32_t resultLength = 0;
  while ((resultLength = udat_format(
              dateFormat_.get(),
              value,
              output.data(),
              bufferSize,
              nullptr,
              &status)) > bufferSize) {
    status = U_ZERO_ERROR;
    output.resize(resultLength);
    bufferSize = output.size();
  }
  output.resize(resultLength);
  if (U_FAILURE(status)) {
    return u""; // TODO : Throw error if failure
  }
  return output;
}

// Implementer note: This method corresponds roughly to
// https://402.ecma-international.org/8.0/#sec-formatdatetimetoparts
std::vector<Part> DateTimeFormat::formatToParts(double value) noexcept {
  UErrorCode status = U_ZERO_ERROR;
  std::unique_ptr<UFieldPositionIterator, decltype(&ufieldpositer_close)>
      posIter(ufieldpositer_open(&status), &ufieldpositer_close);
  if (U_FAILURE(status) || !posIter) {
    return {};
  }
  std::u16string formattedStr(128, char16_t());
  int32_t bufferSize = formattedStr.size();
  int32_t resultLength = 0;
  while ((resultLength = udat_formatForFields(
              dateFormat_.get(),
              value,
              formattedStr.data(),
              bufferSize,
              posIter.get(),
              &status)) > bufferSize) {
    status = U_ZERO_ERROR;
    formattedStr.resize(resultLength);
    bufferSize = formattedStr.size();
  }
  formattedStr.resize(resultLength);
  if (U_FAILURE(status)) {
    return {};
  }
  return partitionPattern(posIter.get(), formattedStr);
}

std::vector<Part> DateTimeFormat::partitionPattern(
    UFieldPositionIterator *posIter,
    const std::u16string &formattedStr) {
  int32_t begin = 0;
  int32_t end = 0;
  int32_t previousEnd = 0;
  int32_t fieldType = -1;
  std::vector<Part> result{};

  while ((fieldType = ufieldpositer_next(posIter, &begin, &end)) >= 0) {
    if (previousEnd < begin) {
      result.push_back(
          {{constants::part_key::type, constants::part_type::literal},
           {constants::part_key::value,
            formattedStr.substr(previousEnd, begin - previousEnd)}});
    }
    result.push_back(
        {{constants::part_key::type, icuDateFieldTypeToPartType(fieldType)},
         {constants::part_key::value,
          formattedStr.substr(begin, end - begin)}});
    previousEnd = end;
  }
  int32_t formattedStrLength = formattedStr.length();
  if (previousEnd < formattedStrLength) {
    result.push_back(
        {{constants::part_key::type, constants::part_type::literal},
         {constants::part_key::value, formattedStr.substr(previousEnd)}});
  }
  return result;
}

std::u16string DateTimeFormat::icuDateFieldTypeToPartType(int32_t fieldType) {
  // Mapping ICU DateFormat field type to DateTimeFormat part types.
  switch (fieldType) {
    case UDAT_DAY_OF_WEEK_FIELD:
    case UDAT_DOW_LOCAL_FIELD:
    case UDAT_STANDALONE_DAY_FIELD:
      return u"weekday";
    case UDAT_ERA_FIELD:
      return u"era";
    case UDAT_YEAR_FIELD:
    case UDAT_EXTENDED_YEAR_FIELD:
      return u"year";
    case UDAT_YEAR_NAME_FIELD:
      return u"yearName";
    case UDAT_MONTH_FIELD:
    case UDAT_STANDALONE_MONTH_FIELD:
      return u"month";
    case UDAT_DATE_FIELD:
      return u"day";
    case UDAT_AM_PM_FIELD:
    case UDAT_AM_PM_MIDNIGHT_NOON_FIELD:
    case UDAT_FLEXIBLE_DAY_PERIOD_FIELD:
      return u"dayPeriod";
    case UDAT_HOUR_OF_DAY1_FIELD:
    case UDAT_HOUR_OF_DAY0_FIELD:
    case UDAT_HOUR1_FIELD:
    case UDAT_HOUR0_FIELD:
      return u"hour";
    case UDAT_MINUTE_FIELD:
      return u"minute";
    case UDAT_SECOND_FIELD:
      return u"second";
    case UDAT_FRACTIONAL_SECOND_FIELD:
      return u"fractionalSecond";
    case UDAT_TIMEZONE_FIELD:
    case UDAT_TIMEZONE_RFC_FIELD:
    case UDAT_TIMEZONE_GENERIC_FIELD:
    case UDAT_TIMEZONE_SPECIAL_FIELD:
    case UDAT_TIMEZONE_LOCALIZED_GMT_OFFSET_FIELD:
    case UDAT_TIMEZONE_ISO_FIELD:
    case UDAT_TIMEZONE_ISO_LOCAL_FIELD:
      return u"timeZoneName";
    // Since ICU 53, UDAT_RELATED_YEAR_FIELD is marked internal only,
    // but it is needed ECMA-402 spec. test262 test case:
    // intl402/DateTimeFormat/prototype/formatToParts/related-year.js
    case UDAT_RELATED_YEAR_FIELD:
      return u"relatedYear";
    default:
      // Handle unsupported or unknown fields
      return constants::part_type::literal;
  }
}

vm::CallResult<std::u16string> DateTimeFormat::formatRange(
    vm::Runtime &runtime,
    double startDate,
    double endDate) noexcept {
  UErrorCode status = U_ZERO_ERROR;
  std::u16string output(256, char16_t());
  int32_t bufferSize = output.size();
  int32_t resultLength = 0;
  while ((resultLength = udtitvfmt_format(
              dateIntervalFormat_.get(),
              startDate,
              endDate,
              output.data(),
              bufferSize,
              nullptr,
              &status)) > bufferSize) {
    status = U_ZERO_ERROR;
    output.resize(resultLength);
    bufferSize = output.size();
  }
  output.resize(resultLength);
  if (U_FAILURE(status)) {
    return runtime.raiseError(
        "Internal error: unable to format the date range.");
  }
  return output;
}

vm::CallResult<std::vector<Part>> DateTimeFormat::formatRangeToParts(
    vm::Runtime &runtime,
    double startDate,
    double endDate) noexcept {
  UErrorCode status = U_ZERO_ERROR;
  std::unique_ptr<UFormattedDateInterval, decltype(&udtitvfmt_closeResult)>
      formattedDateInterval(
          udtitvfmt_openResult(&status), &udtitvfmt_closeResult);
// The argument order changed when udtitvfmt_formatToResult() became stable in
// ICU 67
#if U_ICU_VERSION_MAJOR_NUM >= 67
  udtitvfmt_formatToResult(
      dateIntervalFormat_.get(),
      startDate,
      endDate,
      formattedDateInterval.get(),
      &status);
#else
  udtitvfmt_formatToResult(
      dateIntervalFormat_.get(),
      formattedDateInterval.get(),
      startDate,
      endDate,
      &status);
#endif
  if (U_FAILURE(status)) {
    return runtime.raiseError(
        "Internal error: unable to format the date range");
  }
  return partitionDateTimeRangePattern(runtime, formattedDateInterval.get());
}

vm::CallResult<std::vector<Part>> DateTimeFormat::partitionDateTimeRangePattern(
    vm::Runtime &runtime,
    const UFormattedDateInterval *formattedDateInterval) {
  std::vector<Part> result{};
  int32_t previousEnd = 0;
  std::pair<int32_t, int32_t> startDateRange{-1, -1};
  std::pair<int32_t, int32_t> endDateRange{-1, -1};

  UErrorCode status = U_ZERO_ERROR;
  const UFormattedValue *formattedValue =
      udtitvfmt_resultAsValue(formattedDateInterval, &status);
  int32_t formattedStrLength;
  const char16_t *formattedStr =
      ufmtval_getString(formattedValue, &formattedStrLength, &status);
  std::unique_ptr<UConstrainedFieldPosition, decltype(&ucfpos_close)> cfp(
      ucfpos_open(&status), &ucfpos_close);
  UConstrainedFieldPosition *constrainedFieldPosition = cfp.get();
  while (
      ufmtval_nextPosition(formattedValue, constrainedFieldPosition, &status) &&
      U_SUCCESS(status)) {
    int32_t begin;
    int32_t end;
    ucfpos_getIndexes(constrainedFieldPosition, &begin, &end, &status);
    int32_t fieldType = ucfpos_getField(constrainedFieldPosition, &status);
    int32_t category = ucfpos_getCategory(constrainedFieldPosition, &status);
    if (category == UFIELD_CATEGORY_DATE_INTERVAL_SPAN) {
      // According to udtitvfmt_resultAsValue() documentation, the field value
      // for category UFIELD_CATEGORY_DATE_INTERVAL_SPAN is either 0 or 1.
      // 0 means the span is attributed to the start date. 1 means the span is
      // attributed to the end date. Thus, we use this field value to set
      // startDateRange and endDateRange accordingly.
      if (fieldType == 0) {
        startDateRange.first = begin;
        startDateRange.second = end;
      } else if (fieldType == 1) {
        endDateRange.first = begin;
        endDateRange.second = end;
      }
    } else {
      if (begin > previousEnd) {
        result.push_back(
            {{constants::part_key::type, constants::part_type::literal},
             {constants::part_key::value,
              std::u16string(formattedStr + previousEnd, begin - previousEnd)},
             {constants::part_key::source,
              getPartSource(
                  {previousEnd, begin}, startDateRange, endDateRange)}});
      }
      result.push_back(
          {{constants::part_key::type, icuDateFieldTypeToPartType(fieldType)},
           {constants::part_key::value,
            std::u16string(formattedStr + begin, end - begin)},
           {constants::part_key::source,
            getPartSource({begin, end}, startDateRange, endDateRange)}});
      previousEnd = end;
    }
  }
  if (U_FAILURE(status)) {
    return runtime.raiseError(
        "Internal error: unable to format the date range");
  }
  if (previousEnd < formattedStrLength) {
    result.push_back(
        {{constants::part_key::type, constants::part_type::literal},
         {constants::part_key::value,
          std::u16string(
              formattedStr + previousEnd, formattedStrLength - previousEnd)},
         {constants::part_key::source,
          getPartSource(
              {previousEnd, formattedStrLength},
              startDateRange,
              endDateRange)}});
  }
  return result;
}

bool DateTimeFormat::isSpanInRange(
    std::pair<int32_t, int32_t> span,
    std::pair<int32_t, int32_t> range) {
  return (span.first >= range.first) && (span.first <= range.second) &&
      (span.second >= range.first) && (span.second <= range.second);
}

std::u16string DateTimeFormat::getPartSource(
    std::pair<int32_t, int32_t> span,
    std::pair<int32_t, int32_t> startDateRange,
    std::pair<int32_t, int32_t> endDateRange) {
  if (isSpanInRange(span, startDateRange)) {
    return u"startRange";
  } else if (isSpanInRange(span, endDateRange)) {
    return u"endRange";
  }
  return u"shared";
}

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes
