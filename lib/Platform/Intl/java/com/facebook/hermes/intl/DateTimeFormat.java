/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.os.Build;

import androidx.annotation.Nullable;

import com.facebook.proguard.annotations.DoNotStrip;

import java.text.AttributedCharacterIterator;
import java.text.CharacterIterator;
import java.text.StringCharacterIterator;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;

/**
 * This class represents the Java part of the Android Intl.DateTimeFormat implementation. The
 * interaction with the Hermes JavaScript internals are implemented in C++ and should not generally
 * need to be changed. Implementers' notes here will describe what parts of the ECMA 402 spec remain
 * to be implemented.
 *
 * <p>Implementer notes:
 *
 * <p>Internal slots: In the ECMA 402 spec, there are a number of references to internal slots.
 * These are generally expressed in terms of JavaScript objects, but the semantics do not generally
 * depend on this. For example, where the spec says "Intl.DateTimeFormat instances have an
 * [[InitializedDateTimeFormat]] internal slot", this would not be a literal artifact of the
 * implementation. Internal slots, where necessary, should be represented as members of this java
 * DateTimeFormat object.
 *
 * <p>ICU4J vs Unicode: The ECMA 402 spec makes reference to Unicode documents and data, such as
 * Unicode Technical Standard 35 and the Common Locale Data Repository. However, in practice,
 * platform Unicode implementations are based on the ICU libraries, which encapsulate the concepts
 * in the Unicode documents and data, and do not provide direct access. The Android Intl platform
 * code is expected to be implemented in terms of the Android ICU libraries such as android.icu.text
 * <https://developer.android.com/reference/android/icu/text/package-summary> and android.icu.number
 * <https://developer.android.com/reference/android/icu/number/package-summary>, which are
 * themselves derived from the icu4j API
 * <https://unicode-org.github.io/icu-docs/apidoc/released/icu4j/>. This strategy allows for a more
 * space-efficient implementation than one which ships its own copy of the locale data, which can be
 * 20mb or more. That said, it can be difficult to understand precisely how to implement ECMA 402 in
 * terms of icu4j, and I've left much of the specifics of this up to the implementer. Comparison
 * with existing implementations may help.
 */
@DoNotStrip
public class DateTimeFormat {
  // Implementer note: The constructor corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.datetimeformat.
  //
  // Locales and Options: But, some of the steps described in the
  // algorithm in the spec have already been performed.  Most of
  // CanonicalizeLocaleList has already been done; only steps
  // 7.c.v-vii remain to be done for each element in the input locales
  // list.  The options argument will only contain the correct option
  // strings, whose names have already been converted to appropriate
  // Java types (Boolean, Number, or String).  Where the spec dictates
  // a set of possible values, that check has not yet been done.  It
  // is the responsibility of the ctor to store whatever locale and
  // option data is needed to implement the other members of this
  // class.

  IPlatformDateTimeFormatter mPlatformDateTimeFormatter;

  private ILocaleObject<?> mResolvedLocaleObject = null;

  // We need to preserve the matched locale without options
  // to handle hourCycle formatting
  private ILocaleObject<?> mResolvedDataLocaleObject = null;

  // This is a hacky way to avoid the extensions that we add from being shown in "resolvedOptions"
  // ..
  private ILocaleObject<?> mResolvedLocaleObjectForResolvedOptions = null;

  private boolean useDefaultCalendar;
  private String mCalendar;

  private boolean useDefaultNumberSystem;
  private String mNumberingSystem;

  private Object mHour12;
  private IPlatformDateTimeFormatter.HourCycle mHourCycle;

  private IPlatformDateTimeFormatter.FormatMatcher mFormatMatcher;
  private IPlatformDateTimeFormatter.WeekDay mWeekDay;
  private IPlatformDateTimeFormatter.Era mEra;
  private IPlatformDateTimeFormatter.Year mYear;
  private IPlatformDateTimeFormatter.Month mMonth;
  private IPlatformDateTimeFormatter.Day mDay;
  private IPlatformDateTimeFormatter.Hour mHour;
  private IPlatformDateTimeFormatter.Minute mMinute;
  private IPlatformDateTimeFormatter.Second mSecond;
  private IPlatformDateTimeFormatter.TimeZoneName mTimeZoneName;
  private IPlatformDateTimeFormatter.DateStyle mDateStyle;
  private IPlatformDateTimeFormatter.TimeStyle mTimeStyle;
  private IPlatformDateTimeFormatter.DayPeriod mDayPeriod;
  @Nullable
  private Integer mFractionalSecondDigits;

  private Object mTimeZone = null;

  private boolean isLocaleIdType(String token) {
    return IntlTextUtils.isUnicodeExtensionKeyTypeItem(token, 0, token.length() - 1);
  }

  public String normalizeTimeZoneName(String timeZoneName) {
    StringBuilder normalized = new StringBuilder(timeZoneName.length());
    int offset = 'a' - 'A';
    for (int idx = 0; idx < timeZoneName.length(); idx++) {
      char c = timeZoneName.charAt(idx);
      if (c >= 'A' && c <= 'Z') {
        normalized.append((char) (c + offset));
      } else {
        normalized.append(c);
      }
    }
    return normalized.toString();
  }

  public String normalizeTimeZone(String timeZone) throws JSRangeErrorException {
    for (String id : TimeZone.getAvailableIDs()) {
      if (normalizeTimeZoneName(id).equals(normalizeTimeZoneName(timeZone))) {
        return id;
      }
    }

    String offset = getTimeZoneOffset(timeZone);
    if (offset != null) {
      return offset;
    }

    throw new JSRangeErrorException("Invalid timezone name!");
  }

  private Object DefaultTimeZone() throws JSRangeErrorException {
    return mPlatformDateTimeFormatter.getDefaultTimeZone(mResolvedLocaleObject);
  }

  // 21.4.1.33.1 IsTimeZoneOffsetString ( offsetString )
  // The abstract operation IsTimeZoneOffsetString takes argument offsetString (a String) and returns a Boolean.
  // The return value indicates whether offsetString conforms to the grammar given by UTCOffset.
  // It performs the following steps when called:
  private String getTimeZoneOffset(String offsetString) {
    // 1. Let parseResult be ParseText(StringToCodePoints(offsetString), UTCOffset).
    // 2. If parseResult is a List of errors, return false.
    // 3. Return true.

    int length = offsetString.length();

    boolean isInvalidInJava = false;

    // Java supports "Z" and "+3" while JavaScript does not.
    // The shortest JavaScript zone offset is "+03" and the longest is "+22:00"
    // See https://docs.oracle.com/javase/8/docs/api/java/time/ZoneOffset.html#of-java.lang.String-
    if (offsetString.equals("Z") || length < 3 || length > 6) {
      return null;
    }

    // Normalize -00:00
    if (offsetString.equals("-00") || offsetString.equals("-0000") || offsetString.equals("-00:00") ||
        offsetString.equals("\u221200") || offsetString.equals("\u22120000") || offsetString.equals("\u221200:00")
        || offsetString.equals("+00") || offsetString.equals("+0000") || offsetString.equals("+00:00")) {
      return "+00:00";
    }

    StringBuilder builder = new StringBuilder();
    StringCharacterIterator i = new StringCharacterIterator(offsetString);

    char next = i.first();
    if (next == StringCharacterIterator.DONE) {
      return null;
    }

    if (next == '+') {
      builder.append('+');
    } else if (next == '-' || next == '\u2212') {
      builder.append('-');
    } else {
      return null;
    }

    next = i.next();

    // + is invalid
    if (next == StringCharacterIterator.DONE) {
      return null;
    }

    boolean isTwenty = next == '2';
    if (next == '0' || next == '1' || isTwenty) {
      builder.append(next);
    } else {
      return null;
    }

    next = i.next();

    // +2 is invalid
    if (next == StringCharacterIterator.DONE) {
      return null;
    }


    if (!isTwenty && next >= '0' && next <= '9') {
      if (next >= '8') {
        isInvalidInJava = true;
      }
      builder.append(next);
    } else if (isTwenty && next >= '0' && next <= '3') {
      isInvalidInJava = true;
      builder.append(next);
    } else {
      // +0A is invalid and +24 is invalid
      return null;
    }

    next = i.next();
    if (next == StringCharacterIterator.DONE) {
      return builder + ":00";
    }

    if (next == ':') {
      next = i.next();
    }

    builder.append(':');


    // +12: is invalid
    if (next == StringCharacterIterator.DONE) {
      return null;
    }

    if (next >= '0' && next <= '5') {
      builder.append(next);
    } else {
      // +12:9 is invalid
      return null;
    }

    next = i.next();

    // +12:6 is invalid
    if (next == StringCharacterIterator.DONE) {
      return null;
    }

    if (next >= '0' && next <= '9') {
      builder.append(next);
    } else {
      // +12:5A is invalid
      return null;
    }

    next = i.next();

    // +12532 or +12:532 is invalid
    if (next != StringCharacterIterator.DONE) {
      return null;
    }

    return builder.toString();
  }


  // https://tc39.es/ecma402/#sec-createdatetimeformat
  private void createDateTimeFormat(List<String> locales, Map<String, Object> options, String required, String defaults)
      throws JSRangeErrorException {
    List<String> relevantExtensionKeys = Arrays.asList("ca", "nu", "hc");

    // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
    List<String> requestedLocales = LocaleMatcher.canonicalizeLocaleList(locales);
    // 3. Set options to ? CoerceOptionsToObject(options).
    // NOTE: options is already an object at this point.
    // 4. Let opt be a new Record.
    Object opt = JSObjects.newObject();
    // 5. Let matcher be ? GetOption(options, "localeMatcher", string, « "lookup", "best fit" », "best fit").
    Object matcher =
        OptionHelpers.GetOption(
            options,
            Constants.LOCALEMATCHER,
            OptionHelpers.OptionType.STRING,
            Constants.LOCALEMATCHER_POSSIBLE_VALUES,
            Constants.LOCALEMATCHER_BESTFIT);
    // 6. Set opt.[[localeMatcher]] to matcher.
    JSObjects.Put(opt, "localeMatcher", matcher);
    // 7. Let calendar be ? GetOption(options, "calendar", string, empty, undefined).
    Object calendar =
        OptionHelpers.GetOption(
            options,
            "calendar",
            OptionHelpers.OptionType.STRING,
            JSObjects.Undefined(),
            JSObjects.Undefined());
    // 8. If calendar is not undefined, then
    if (!JSObjects.isUndefined(calendar)) {
      // a. If calendar cannot be matched by the type Unicode locale nonterminal, throw a RangeError exception.
      if (!isLocaleIdType(JSObjects.getJavaString(calendar)))
        throw new JSRangeErrorException("Invalid calendar option !");
    }
    // 9. Set opt.[[ca]] to calendar.
    JSObjects.Put(opt, "ca", calendar);
    // 10. Let numberingSystem be ? GetOption(options, "numberingSystem", string, empty, undefined).
    Object numberingSystem =
        OptionHelpers.GetOption(
            options,
            "numberingSystem",
            OptionHelpers.OptionType.STRING,
            JSObjects.Undefined(),
            JSObjects.Undefined());
    // 11. If numberingSystem is not undefined, then
    if (!JSObjects.isUndefined(numberingSystem)) {
      // a. If numberingSystem cannot be matched by the type Unicode locale nonterminal, throw a RangeError exception.
      if (!isLocaleIdType(JSObjects.getJavaString(numberingSystem)))
        throw new JSRangeErrorException("Invalid numbering system !");
    }
    // 12. Set opt.[[nu]] to numberingSystem.
    JSObjects.Put(opt, "nu", numberingSystem);
    // 13. Let hour12 be ? GetOption(options, "hour12", boolean, empty, undefined).
    Object hour12 =
        OptionHelpers.GetOption(
            options,
            "hour12",
            OptionHelpers.OptionType.BOOLEAN,
            JSObjects.Undefined(),
            JSObjects.Undefined());
    // 14. Let hourCycle be ? GetOption(options, "hourCycle", string, « "h11", "h12", "h23", "h24" », undefined).
    Object hourCycle =
        OptionHelpers.GetOption(
            options,
            "hourCycle",
            OptionHelpers.OptionType.STRING,
            new String[] {"h11", "h12", "h23", "h24"},
            JSObjects.Undefined());
    // 15. If hour12 is not undefined, then
    if (!JSObjects.isUndefined(hour12))
      // a. Set hourCycle to null.
      hourCycle = JSObjects.Null();
    // 16. Set opt.[[hc]] to hourCycle.
    JSObjects.Put(opt, "hc", hourCycle);
    // 17. Let localeData be %DateTimeFormat%.[[LocaleData]].

    // 18. Let r be ResolveLocale(%DateTimeFormat%.[[AvailableLocales]], requestedLocales, opt, %DateTimeFormat%.[[RelevantExtensionKeys]], localeData).
    HashMap<String, Object> r = LocaleResolver.resolveLocale(requestedLocales, opt, relevantExtensionKeys);
    // 19. Set dateTimeFormat.[[Locale]] to r.[[locale]].
    mResolvedLocaleObject = (ILocaleObject<?>) JSObjects.getJavaMap(r).get("locale");
    // 20. Let resolvedCalendar be r.[[ca]].
    mResolvedDataLocaleObject = (ILocaleObject<?>) JSObjects.getJavaMap(r).get("dataLocale");
    mResolvedLocaleObjectForResolvedOptions = mResolvedLocaleObject.cloneObject();
    // 21. Set dateTimeFormat.[[Calendar]] to resolvedCalendar.
    Object calendarResolved = JSObjects.Get(r, "ca");
    if (!JSObjects.isNull(calendarResolved)) {
      useDefaultCalendar = false;
      mCalendar = JSObjects.getJavaString(calendarResolved);
    } else {
      useDefaultCalendar = true;
      mCalendar = mPlatformDateTimeFormatter.getDefaultCalendarName(mResolvedLocaleObject);
    }
    // 22. Set dateTimeFormat.[[NumberingSystem]] to r.[[nu]].
    Object numeringSystemResolved = JSObjects.Get(r, "nu");
    if (!JSObjects.isNull(numeringSystemResolved)) {
      useDefaultNumberSystem = false;
      mNumberingSystem = JSObjects.getJavaString(numeringSystemResolved);
    } else {
      useDefaultNumberSystem = true;
      mNumberingSystem =
          mPlatformDateTimeFormatter.getDefaultNumberingSystem(mResolvedLocaleObject);
    }
    // 23. Let dataLocale be r.[[dataLocale]].
    // 24. Let dataLocaleData be localeData.[[<dataLocale>]].
    IPlatformDateTimeFormatter.HourCycle hc;
    if (JSObjects.isBoolean(hour12)) {
      // 25. If hour12 is true, then
      if (JSObjects.getJavaBoolean(hour12)) {
        // a. Let hc be dataLocaleData.[[hourCycle12]].
        hc = mPlatformDateTimeFormatter.getHourCycle12(mResolvedDataLocaleObject);
      } else {
        // 26. Else if hour12 is false, then
        // a. Let hc be dataLocaleData.[[hourCycle24]].
        hc = mPlatformDateTimeFormatter.getHourCycle24(mResolvedDataLocaleObject);
      }
    } else {
      // 27. Else,
      // a. Assert: hour12 is undefined.
      //     b. Let hc be r.[[hc]].
      hc = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.HourCycle.class, JSObjects.Get(r, "hc"));
      // c. If hc is null, set hc to dataLocaleData.[[hourCycle]].
      if (hc == null) {
        hc = mPlatformDateTimeFormatter.getHourCycle(mResolvedDataLocaleObject);
      }
    }

    // 28. Set dateTimeFormat.[[HourCycle]] to hc.
    mHourCycle = hc;
    // 29. Let timeZone be ? Get(options, "timeZone").
    Object timeZone = JSObjects.Get(options, "timeZone");
    // 30. If timeZone is undefined, then
    if (JSObjects.isUndefined(timeZone)) {
      // a. Set timeZone to SystemTimeZoneIdentifier().
      timeZone = DefaultTimeZone();
    } else {
      // 31. Else,
      //   a. Set timeZone to ? ToString(timeZone).
      timeZone = normalizeTimeZone(timeZone.toString());
    }
    // 32. If IsTimeZoneOffsetString(timeZone) is true, then
    // TODO: Implement zone offset strings fully
    // a. Let parseResult be ParseText(StringToCodePoints(timeZone), UTCOffset).
    // b. Assert: parseResult is a Parse Node.
    //     c. If parseResult contains more than one MinuteSecond Parse Node, throw a RangeError exception.
    // d. Let offsetNanoseconds be ParseTimeZoneOffsetString(timeZone).
    //     e. Let offsetMinutes be offsetNanoseconds / (6 × 10**10).
    // f. Assert: offsetMinutes is an integer.
    // g. Set timeZone to FormatOffsetTimeZoneIdentifier(offsetMinutes).
    // 33. Else if IsValidTimeZoneName(timeZone) is true, then
    // a. Set timeZone to CanonicalizeTimeZoneName(timeZone).
    mTimeZone = timeZone;
    // 34. Else,
    //     a. Throw a RangeError exception.
    // 35. Set dateTimeFormat.[[TimeZone]] to timeZone.
    // 36. Let formatOptions be a new Record.
    // 37. Set formatOptions.[[hourCycle]] to hc.
    // 38. Let hasExplicitFormatComponents be false.
    boolean hasExplicitFormatComponents = false;
    // 39. For each row of Table 7, except the header row, in table order, do
    //   a. Let prop be the name given in the Property column of the row.
    //   b. If prop is "fractionalSecondDigits", then
    //     i. Let value be ? GetNumberOption(options, "fractionalSecondDigits", 1, 3, undefined).
    // d. Set formatOptions.[[<prop>]] to value.
    mFractionalSecondDigits = OptionHelpers.GetNumberOption(
        options,
        "fractionalSecondDigits",
        1, 3,
        null);
    // e. If value is not undefined, then
    if (mFractionalSecondDigits != null) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // c. Else,
    //   i. Let values be a List whose elements are the strings given in the Values column of the row.
    //   ii. Let value be ? GetOption(options, prop, string, values, undefined).
    Object value =
        OptionHelpers.GetOption(
            options,
            "era",
            OptionHelpers.OptionType.STRING,
            new String[] {"long", "short", "narrow"},
            JSObjects.Undefined());
    // d. Set formatOptions.[[<prop>]] to value.
    mEra = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Era.class, value);
    // e. If value is not undefined, then
    if (!JSObjects.isUndefined(value)) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // c. Else,
    //   i. Let values be a List whose elements are the strings given in the Values column of the row.
    //   ii. Let value be ? GetOption(options, prop, string, values, undefined).
    value =
        OptionHelpers.GetOption(
            options,
            "year",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit"},
            JSObjects.Undefined());
    // d. Set formatOptions.[[<prop>]] to value.
    mYear = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Year.class, value);
    // e. If value is not undefined, then
    if (!JSObjects.isUndefined(value)) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // c. Else,
    //   i. Let values be a List whose elements are the strings given in the Values column of the row.
    //   ii. Let value be ? GetOption(options, prop, string, values, undefined).
    value =
        OptionHelpers.GetOption(
            options,
            "month",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit", "long", "short", "narrow"},
            JSObjects.Undefined());
    // d. Set formatOptions.[[<prop>]] to value.
    mMonth = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Month.class, value);
    // e. If value is not undefined, then
    if (!JSObjects.isUndefined(value)) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // c. Else,
    //   i. Let values be a List whose elements are the strings given in the Values column of the row.
    //   ii. Let value be ? GetOption(options, prop, string, values, undefined).
    value =
        OptionHelpers.GetOption(
            options,
            "day",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit"},
            JSObjects.Undefined());
    // d. Set formatOptions.[[<prop>]] to value.
    mDay = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Day.class, value);
    // e. If value is not undefined, then
    if (!JSObjects.isUndefined(value)) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // c. Else,
    //   i. Let values be a List whose elements are the strings given in the Values column of the row.
    //   ii. Let value be ? GetOption(options, prop, string, values, undefined).
    value =
        OptionHelpers.GetOption(
            options,
            "hour",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit"},
            JSObjects.Undefined());
    // d. Set formatOptions.[[<prop>]] to value.
    mHour = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Hour.class, value);
    // e. If value is not undefined, then
    if (!JSObjects.isUndefined(value)) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // c. Else,
    //   i. Let values be a List whose elements are the strings given in the Values column of the row.
    //   ii. Let value be ? GetOption(options, prop, string, values, undefined).
    value =
        OptionHelpers.GetOption(
            options,
            "minute",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit"},
            JSObjects.Undefined());
    // d. Set formatOptions.[[<prop>]] to value.
    mMinute = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Minute.class, value);
    // e. If value is not undefined, then
    if (!JSObjects.isUndefined(value)) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // c. Else,
    //   i. Let values be a List whose elements are the strings given in the Values column of the row.
    //   ii. Let value be ? GetOption(options, prop, string, values, undefined).
    value =
        OptionHelpers.GetOption(
            options,
            "second",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit"},
            JSObjects.Undefined());
    // d. Set formatOptions.[[<prop>]] to value.
    mSecond = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Second.class, value);
    // e. If value is not undefined, then
    if (!JSObjects.isUndefined(value)) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // c. Else,
    //   i. Let values be a List whose elements are the strings given in the Values column of the row.
    //   ii. Let value be ? GetOption(options, prop, string, values, undefined).
    value =
        OptionHelpers.GetOption(
            options,
            "timeZoneName",
            OptionHelpers.OptionType.STRING,
            new String[] {
                "long", "longOffset", "longGeneric", "short", "shortOffset", "shortGeneric"
            },
            JSObjects.Undefined());
    // d. Set formatOptions.[[<prop>]] to value.
    mTimeZoneName =
        OptionHelpers.searchEnum(IPlatformDateTimeFormatter.TimeZoneName.class, value);
    // e. If value is not undefined, then
    if (!JSObjects.isUndefined(value)) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // c. Else,
    //   i. Let values be a List whose elements are the strings given in the Values column of the row.
    //   ii. Let value be ? GetOption(options, prop, string, values, undefined).
    value =
        OptionHelpers.GetOption(
            options,
            "dayPeriod",
            OptionHelpers.OptionType.STRING,
            new String[] {"long", "short", "narrow"},
            JSObjects.Undefined());
    // d. Set formatOptions.[[<prop>]] to value.
    mDayPeriod =
        OptionHelpers.searchEnum(IPlatformDateTimeFormatter.DayPeriod.class, value);
    // e. If value is not undefined, then
    if (!JSObjects.isUndefined(value)) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // c. Else,
    //   i. Let values be a List whose elements are the strings given in the Values column of the row.
    //   ii. Let value be ? GetOption(options, prop, string, values, undefined).
    value =
        OptionHelpers.GetOption(
            options,
            "weekday",
            OptionHelpers.OptionType.STRING,
            new String[] {"long", "short", "narrow"},
            JSObjects.Undefined());
    // d. Set formatOptions.[[<prop>]] to value.
    mWeekDay = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.WeekDay.class, value);
    // e. If value is not undefined, then
    if (!JSObjects.isUndefined(value)) {
      // i. Set hasExplicitFormatComponents to true.
      hasExplicitFormatComponents = true;
    }

    // 40. Let formatMatcher be ? GetOption(options, "formatMatcher", string, « "basic", "best fit" », "best fit").
    Object formatMatcher =
        OptionHelpers.GetOption(
            options,
            "formatMatcher",
            OptionHelpers.OptionType.STRING,
            new String[] {"basic", "best fit"},
            "best fit");
    mFormatMatcher =
        OptionHelpers.searchEnum(
            IPlatformDateTimeFormatter.FormatMatcher.class, JSObjects.getJavaString(formatMatcher));

    // 41. Let dateStyle be ? GetOption(options, "dateStyle", string, « "full", "long", "medium", "short" », undefined).
    Object dateStyle =
        OptionHelpers.GetOption(
            options,
            "dateStyle",
            OptionHelpers.OptionType.STRING,
            new String[] {"full", "long", "medium", "short"},
            JSObjects.Undefined());
    // 42. Set dateTimeFormat.[[DateStyle]] to dateStyle.
    mDateStyle = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.DateStyle.class, dateStyle);

    // 43. Let timeStyle be ? GetOption(options, "timeStyle", string, « "full", "long", "medium", "short" », undefined).
    Object timeStyle =
        OptionHelpers.GetOption(
            options,
            "timeStyle",
            OptionHelpers.OptionType.STRING,
            new String[] {"full", "long", "medium", "short"},
            JSObjects.Undefined());
    // 44. Set dateTimeFormat.[[TimeStyle]] to timeStyle.
    mTimeStyle = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.TimeStyle.class, timeStyle);

    // 45. If dateStyle is not undefined or timeStyle is not undefined, then
    if (mTimeStyle != IPlatformDateTimeFormatter.TimeStyle.UNDEFINED || mDateStyle != IPlatformDateTimeFormatter.DateStyle.UNDEFINED) {
      // a. If hasExplicitFormatComponents is true, then
      if (hasExplicitFormatComponents) {
        // i. Throw a TypeError exception.
        // TODO: We don't support TypeError from Java code yet
        throw new JSRangeErrorException("Has explicit format components");
      }

      // b. If required is date and timeStyle is not undefined, then
      if (required.equals("date") && timeStyle != JSObjects.Undefined()) {
        // i. Throw a TypeError exception.
        // TODO: We don't support TypeError from Java code yet
        throw new JSRangeErrorException("Date is required with time style");
      }

      // c. If required is time and dateStyle is not undefined, then
      if (required.equals("time") && dateStyle != JSObjects.Undefined()) {
        // i. Throw a TypeError exception.
        // TODO: We don't support TypeError from Java code yet
        throw new JSRangeErrorException();
      }

      // d. Let styles be dataLocaleData.[[styles]].[[<resolvedCalendar>]].
      // e. Let bestFormat be DateTimeStyleFormat(dateStyle, timeStyle, styles).
    // 46. Else,
    } else {
      // a. Let needDefaults be true.
      boolean needDefaults = true;
      // b. If required is date or any, then
      if (required.equals("date") || required.equals("any")) {
        // i. For each property name prop of « "weekday", "year", "month", "day" », do
        //   1. Let value be formatOptions.[[<prop>]].
        if (mDayPeriod != IPlatformDateTimeFormatter.DayPeriod.UNDEFINED ||
          mWeekDay != IPlatformDateTimeFormatter.WeekDay.UNDEFINED ||
          mYear != IPlatformDateTimeFormatter.Year.UNDEFINED ||
          mMonth != IPlatformDateTimeFormatter.Month.UNDEFINED ||
          mDay != IPlatformDateTimeFormatter.Day.UNDEFINED) {
            // 2. If value is not undefined, set needDefaults to false.
            needDefaults = false;
        }
      }


      // c. If required is time or any, then
      if (required.equals("time") || required.equals("any")) {
        // i. For each property name prop of « "dayPeriod", "hour", "minute", "second", "fractionalSecondDigits" », do
        for (String property : new String[] {"hour", "minute", "second"}) {
          // 1. Let value be formatOptions.[[<prop>]].
          if (!JSObjects.isUndefined(JSObjects.Get(options, property))) {
            // 2. If value is not undefined, set needDefaults to false.
            needDefaults = false;
          }
        }
      }

      // d. If needDefaults is true and defaults is either date or all, then
      if (needDefaults && (defaults.equals("date") || defaults.equals("all"))) {
        // i. For each property name prop of « "year", "month", "day" », do
        //   1. Set formatOptions.[[<prop>]] to "numeric".
        mYear = IPlatformDateTimeFormatter.Year.NUMERIC;
        mMonth = IPlatformDateTimeFormatter.Month.NUMERIC;
        mDay = IPlatformDateTimeFormatter.Day.NUMERIC;
      }

      // e. If needDefaults is true and defaults is either time or all, then
      if (needDefaults && (defaults.equals("time") || defaults.equals("all"))) {
        // i. For each property name prop of « "hour", "minute", "second" », do
        //   1. Set formatOptions.[[<prop>]] to "numeric".
        mHour = IPlatformDateTimeFormatter.Hour.NUMERIC;
        mMinute = IPlatformDateTimeFormatter.Minute.NUMERIC;
        mSecond = IPlatformDateTimeFormatter.Second.NUMERIC;
      }

      // f. Let formats be dataLocaleData.[[formats]].[[<resolvedCalendar>]].
      // g. If formatMatcher is "basic", then
      // i. Let bestFormat be BasicFormatMatcher(formatOptions, formats).
      // h. Else,
      // i. Let bestFormat be BestFitFormatMatcher(formatOptions, formats).
    }

    // 47. For each row of Table 7, except the header row, in table order, do
    //   a. Let prop be the name given in the Property column of the row.
    //     b. If bestFormat has a field [[<prop>]], then
    // i. Let p be bestFormat.[[<prop>]].
    // ii. Set dateTimeFormat's internal slot whose name is the Internal Slot column of the row to p.
    // 48. If dateTimeFormat.[[Hour]] is undefined, then
    if (mTimeStyle == IPlatformDateTimeFormatter.TimeStyle.UNDEFINED && mHour == IPlatformDateTimeFormatter.Hour.UNDEFINED) {
      // a. Set dateTimeFormat.[[HourCycle]] to undefined.
      mHourCycle = IPlatformDateTimeFormatter.HourCycle.UNDEFINED;
    }
    // 49. If dateTimeFormat.[[HourCycle]] is "h11" or "h12", then
    // a. Let pattern be bestFormat.[[pattern12]].
    // b. Let rangePatterns be bestFormat.[[rangePatterns12]].
    // 50. Else,
    //     a. Let pattern be bestFormat.[[pattern]].
    // b. Let rangePatterns be bestFormat.[[rangePatterns]].
    // 51. Set dateTimeFormat.[[Pattern]] to pattern.
    // 52. Set dateTimeFormat.[[RangePatterns]] to rangePatterns.
    // 53. Return dateTimeFormat.
  }

  @DoNotStrip
  public DateTimeFormat(List<String> locales, Map<String, Object> options)
      throws JSRangeErrorException {
    // 1. If NewTarget is undefined, let newTarget be the active function object, else let newTarget be NewTarget.

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N)
      mPlatformDateTimeFormatter = new PlatformDateTimeFormatterICU();
    else mPlatformDateTimeFormatter = new PlatformDateTimeFormatterAndroid();

    // 2. Let dateTimeFormat be ? CreateDateTimeFormat(newTarget, locales, options, any, date).
    createDateTimeFormat(locales, options, "any", "date");

    // TODO: We don't support passing objects in...
    // 3. If the implementation supports the normative optional constructor mode of 4.3 Note 1, then
    //   a. Let this be the this value.
    //   b. Return ? ChainDateTimeFormat(dateTimeFormat, NewTarget, this).

    mPlatformDateTimeFormatter.configure(
        mResolvedLocaleObject,
        useDefaultCalendar ? "" : mCalendar,
        useDefaultNumberSystem ? "" : mNumberingSystem,
        mFormatMatcher,
        mWeekDay,
        mEra,
        mYear,
        mMonth,
        mDay,
        mHour,
        mMinute,
        mSecond,
        mTimeZoneName,
        mHourCycle,
        mTimeZone,
        mDateStyle,
        mTimeStyle,
        mHour12,
        mDayPeriod,
        mFractionalSecondDigits);
  }

  // options are localeMatcher:string
  //
  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.datetimeformat.supportedlocalesof.
  //
  // The notes on the ctor for Locales and Options also apply here.
  @DoNotStrip
  public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options)
      throws JSRangeErrorException {
    String matcher =
        JSObjects.getJavaString(
            OptionHelpers.GetOption(
                options,
                Constants.LOCALEMATCHER,
                OptionHelpers.OptionType.STRING,
                Constants.LOCALEMATCHER_POSSIBLE_VALUES,
                Constants.LOCALEMATCHER_BESTFIT));
    String[] localeArray = new String[locales.size()];
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N && matcher.equals("best fit")) {
      return Arrays.asList(LocaleMatcher.bestFitSupportedLocales(locales.toArray(localeArray)));
    } else {
      return Arrays.asList(LocaleMatcher.lookupSupportedLocales(locales.toArray(localeArray)));
    }
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.datetimeformat.prototype.resolvedoptions
  //
  // Steps 1-4 are implied; all that is necessary is to perform step
  // 5, except that CreateDataPropertyOrThrow needs to be mapped into
  // Java.  Instead, a Java Map must be returned, where keys are the
  // expected property names, and values are Boolean, Number, or
  // String.  Note that the types are implied when the "internal
  // slots" are set (in the ctor), but in practice each "slot" should
  // correspond to a member with a well-defined Java type.
  @DoNotStrip
  public Map<String, Object> resolvedOptions() throws JSRangeErrorException {
    HashMap<String, Object> finalResolvedOptions = new LinkedHashMap<>();
    finalResolvedOptions.put(
        Constants.LOCALE, mResolvedLocaleObjectForResolvedOptions.toCanonicalTag());

    finalResolvedOptions.put("numberingSystem", mNumberingSystem);
    finalResolvedOptions.put("calendar", mCalendar);
    finalResolvedOptions.put("timeZone", mTimeZone);

    if (mHourCycle != IPlatformDateTimeFormatter.HourCycle.UNDEFINED) {
      finalResolvedOptions.put("hourCycle", mHourCycle.toString());

      if (mHourCycle == IPlatformDateTimeFormatter.HourCycle.H11
          || mHourCycle == IPlatformDateTimeFormatter.HourCycle.H12)
        finalResolvedOptions.put("hour12", true);
      else finalResolvedOptions.put("hour12", false);
    }

    if (mWeekDay != IPlatformDateTimeFormatter.WeekDay.UNDEFINED)
      finalResolvedOptions.put("weekday", mWeekDay.toString());

    if (mEra != IPlatformDateTimeFormatter.Era.UNDEFINED)
      finalResolvedOptions.put("era", mEra.toString());

    if (mYear != IPlatformDateTimeFormatter.Year.UNDEFINED)
      finalResolvedOptions.put("year", mYear.toString());

    if (mMonth != IPlatformDateTimeFormatter.Month.UNDEFINED)
      finalResolvedOptions.put("month", mMonth.toString());

    if (mDay != IPlatformDateTimeFormatter.Day.UNDEFINED)
      finalResolvedOptions.put("day", mDay.toString());

    if (mHour != IPlatformDateTimeFormatter.Hour.UNDEFINED)
      finalResolvedOptions.put("hour", mHour.toString());

    if (mMinute != IPlatformDateTimeFormatter.Minute.UNDEFINED)
      finalResolvedOptions.put("minute", mMinute.toString());

    if (mSecond != IPlatformDateTimeFormatter.Second.UNDEFINED)
      finalResolvedOptions.put("second", mSecond.toString());

    if (mTimeZoneName != IPlatformDateTimeFormatter.TimeZoneName.UNDEFINED) {
      finalResolvedOptions.put("timeZoneName", mTimeZoneName.toString());
    }

    if (mDayPeriod != IPlatformDateTimeFormatter.DayPeriod.UNDEFINED) {
      finalResolvedOptions.put("dayPeriod", mDayPeriod.toString());
    }

    if (mFractionalSecondDigits != null) {
      finalResolvedOptions.put("fractionalSecondDigits", mFractionalSecondDigits);
    }

    if (mDateStyle != IPlatformDateTimeFormatter.DateStyle.UNDEFINED) {
      finalResolvedOptions.put("dateStyle", mDateStyle.toString());
    }

    if (mTimeStyle != IPlatformDateTimeFormatter.TimeStyle.UNDEFINED) {
      finalResolvedOptions.put("timeStyle", mTimeStyle.toString());
    }

    return finalResolvedOptions;
  }

  private List<Map<String, String>> mapAttributedCharacterIteratorToParts(AttributedCharacterIterator iterator) {
    ArrayList<Map<String, String>> ret = new ArrayList<>();
    StringBuilder sb = new StringBuilder();
    for (char ch = iterator.first(); ch != CharacterIterator.DONE; ch = iterator.next()) {
      sb.append(ch);
      if (iterator.getIndex() + 1 == iterator.getRunLimit()) {
        Map<AttributedCharacterIterator.Attribute, Object> attributes =
            iterator.getAttributes();

        String value = sb.toString();
        sb.setLength(0);

        String type;
        if (attributes.size() > 0) {
          type = mPlatformDateTimeFormatter.fieldAttrsToTypeString(attributes.entrySet(), value);
        } else {
          type = "literal";
        }

        HashMap<String, String> part = new HashMap<>();
        part.put("type", type);
        part.put("value", value);
        ret.add(part);
      }
    }

    return ret;
  }

  private List<Map<String, String>> mapAttributedCharacterIteratorToSourceParts(AttributedCharacterIterator iterator) {
    ArrayList<Map<String, String>> ret = new ArrayList<>();
    StringBuilder sb = new StringBuilder();

    for (char ch = iterator.first(); ch != CharacterIterator.DONE; ch = iterator.next()) {
      sb.append(ch);
      if (iterator.getIndex() + 1 == iterator.getRunLimit()) {
        HashMap<String, String> part = new HashMap<>();

        String value = sb.toString();
        sb.setLength(0);

        part.put("value", value);

        Map<AttributedCharacterIterator.Attribute, Object> attributes = iterator.getAttributes();
        String type, source;

        if (attributes.size() == 0) {
          type = "literal";
          source = "shared";
        } else {
          type = mPlatformDateTimeFormatter.fieldAttrsToTypeString(attributes.entrySet(), value);
          source = mPlatformDateTimeFormatter.fieldAttrsToSourceString(attributes.entrySet());
        }

        part.put("type", type);
        part.put("source", source);
        ret.add(part);
      }
    }

    return ret;
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatdatetime
  //
  // Steps 1 and 2 of PartitionDateTimePattern are already done.  The
  // formal steps require construction of several internal
  // NumberFormat JavaScript objects, but these objects are never
  // exposed; it should be possible to create and use java
  // NumberFormat objects only.
  @DoNotStrip
  public String format(double jsTimeValue) throws JSRangeErrorException {
    return mPlatformDateTimeFormatter.format(jsTimeValue);
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatdatetimetoparts
  @DoNotStrip
  public List<Map<String, String>> formatToParts(double jsTimeValue) throws JSRangeErrorException {
    AttributedCharacterIterator iterator = mPlatformDateTimeFormatter.formatToParts(jsTimeValue);

    return this.mapAttributedCharacterIteratorToParts(iterator);
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatdatetimerange
  @DoNotStrip
  public String formatRange(double jsTimeValueFrom, double jsTimeValueTo) throws JSRangeErrorException {
    return mPlatformDateTimeFormatter.formatRange(jsTimeValueFrom, jsTimeValueTo);
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatdatetimerangetoparts
  @DoNotStrip
  public List<Map<String, String>> formatRangeToParts(double jsTimeValueFrom, double jsTimeValueTo) throws JSRangeErrorException {
    AttributedCharacterIterator iterator = mPlatformDateTimeFormatter.formatRangeToParts(jsTimeValueFrom, jsTimeValueTo);

    return this.mapAttributedCharacterIteratorToSourceParts(iterator);
  }
}
