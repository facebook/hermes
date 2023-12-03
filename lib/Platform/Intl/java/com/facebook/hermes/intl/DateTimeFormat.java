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
  // options are localeMatcher:string, calendar:string, numberingSystem:string, hour12:boolean,
  // hourCycle:string, timeZone:string, formatMatcher:string, weekday:string, era:string,
  // year:string, month:string, day:string, hour:string, minute:string, second:string,
  // timeZoneName:string
  //
  // Implementer note: The ctor corresponds roughly to
  // https://tc39.es/ecma402/#sec-initializedatetimeformat.
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

  // https://tc39.es/ecma402/#sec-todatetimeoptions
  private Object ToDateTimeOptions(Object options, String required, String defaults)
      throws JSRangeErrorException {
    if (!JSObjects.isObject(options)) {
      throw new JSRangeErrorException("Invalid options object !");
    }
    boolean needDefaults = true;

    if (required.equals("date") || required.equals("any")) {
      for (String property : new String[] {"dayPeriod", "weekday", "year", "month", "day"}) {
        if (!JSObjects.isUndefined(JSObjects.Get(options, property))) needDefaults = false;
      }
    }

    if (required.equals("time") || required.equals("any")) {
      for (String property : new String[] {"hour", "minute", "second"}) {
        if (!JSObjects.isUndefined(JSObjects.Get(options, property))) needDefaults = false;
      }
    }

    if (!JSObjects.isUndefined(JSObjects.Get(options, "dateStyle"))
        || !JSObjects.isUndefined(JSObjects.Get(options, "timeStyle"))) needDefaults = false;

    if (needDefaults && (defaults.equals("date") || defaults.equals("all"))) {
      for (String property : new String[] {"year", "month", "day"}) {
        JSObjects.Put(options, property, "numeric");
      }
    }

    if (needDefaults && (defaults.equals("time") || defaults.equals("all"))) {
      for (String property : new String[] {"hour", "minute", "second"}) {
        JSObjects.Put(options, property, "numeric");
      }
    }

    return options;
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


  // https://tc39.es/ecma402/#sec-initializedatetimeformat
  private void initializeDateTimeFormat(List<String> locales, Map<String, Object> inOptions)
      throws JSRangeErrorException {
    List<String> relevantExtensionKeys = Arrays.asList("ca", "nu", "hc");

    // 2.
    Object options = ToDateTimeOptions(inOptions, "any", "date");

    // 3 - 4.
    Object opt = JSObjects.newObject();

    // 5 - 6.
    Object matcher =
        OptionHelpers.GetOption(
            options,
            Constants.LOCALEMATCHER,
            OptionHelpers.OptionType.STRING,
            Constants.LOCALEMATCHER_POSSIBLE_VALUES,
            Constants.LOCALEMATCHER_BESTFIT);
    JSObjects.Put(opt, "localeMatcher", matcher);

    // 7 - 9.
    Object calendar =
        OptionHelpers.GetOption(
            options,
            "calendar",
            OptionHelpers.OptionType.STRING,
            JSObjects.Undefined(),
            JSObjects.Undefined());
    if (!JSObjects.isUndefined(calendar)) {
      if (!isLocaleIdType(JSObjects.getJavaString(calendar)))
        throw new JSRangeErrorException("Invalid calendar option !");
    }
    JSObjects.Put(opt, "ca", calendar);

    // 10 - 12.
    Object numberingSystem =
        OptionHelpers.GetOption(
            options,
            "numberingSystem",
            OptionHelpers.OptionType.STRING,
            JSObjects.Undefined(),
            JSObjects.Undefined());
    if (!JSObjects.isUndefined(numberingSystem)) {
      if (!isLocaleIdType(JSObjects.getJavaString(numberingSystem)))
        throw new JSRangeErrorException("Invalid numbering system !");
    }
    JSObjects.Put(opt, "nu", numberingSystem);

    // 13 - 16.
    Object hour12 =
        OptionHelpers.GetOption(
            options,
            "hour12",
            OptionHelpers.OptionType.BOOLEAN,
            JSObjects.Undefined(),
            JSObjects.Undefined());
    Object hourCycle =
        OptionHelpers.GetOption(
            options,
            "hourCycle",
            OptionHelpers.OptionType.STRING,
            new String[] {"h11", "h12", "h23", "h24"},
            JSObjects.Undefined());

    if (!JSObjects.isUndefined(hour12)) hourCycle = JSObjects.Null();

    mHour12 = hour12;

    JSObjects.Put(opt, "hc", hourCycle);
    // 18 - 20.
    HashMap<String, Object> r = LocaleResolver.resolveLocale(locales, opt, relevantExtensionKeys);

    mResolvedLocaleObject = (ILocaleObject<?>) JSObjects.getJavaMap(r).get("locale");
    mResolvedDataLocaleObject = (ILocaleObject<?>) JSObjects.getJavaMap(r).get("dataLocale");
    mResolvedLocaleObjectForResolvedOptions = mResolvedLocaleObject.cloneObject();

    // 21.
    Object calendarResolved = JSObjects.Get(r, "ca");
    if (!JSObjects.isNull(calendarResolved)) {
      useDefaultCalendar = false;
      mCalendar = JSObjects.getJavaString(calendarResolved);
    } else {
      useDefaultCalendar = true;
      mCalendar = mPlatformDateTimeFormatter.getDefaultCalendarName(mResolvedLocaleObject);
    }

    // 22.
    Object numeringSystemResolved = JSObjects.Get(r, "nu");
    if (!JSObjects.isNull(numeringSystemResolved)) {
      useDefaultNumberSystem = false;
      mNumberingSystem = JSObjects.getJavaString(numeringSystemResolved);
    } else {
      useDefaultNumberSystem = true;
      mNumberingSystem =
          mPlatformDateTimeFormatter.getDefaultNumberingSystem(mResolvedLocaleObject);
    }

    IPlatformDateTimeFormatter.HourCycle hc;

    if (JSObjects.isBoolean(hour12)) {
      if (JSObjects.getJavaBoolean(hour12)) {
        hc = mPlatformDateTimeFormatter.getHourCycle12(mResolvedDataLocaleObject);
      } else {
        hc = mPlatformDateTimeFormatter.getHourCycle24(mResolvedDataLocaleObject);
      }
    } else {
      Object hourCycleResolved = JSObjects.Get(r, "hc");
      if (JSObjects.isNull(hourCycleResolved)) {
        hc = mPlatformDateTimeFormatter.getHourCycle(mResolvedDataLocaleObject);
      } else {
        hc = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.HourCycle.class, hourCycleResolved);
      }
    }

    mHourCycle = hc;

    // 29 - 35.
    Object timeZone = JSObjects.Get(options, "timeZone");
    if (JSObjects.isUndefined(timeZone)) {
      timeZone = DefaultTimeZone();
    } else {
      timeZone = normalizeTimeZone(timeZone.toString());
    }
    mTimeZone = timeZone;

    // 36 - 38.
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

    // 39.
    Object weekDay =
        OptionHelpers.GetOption(
            options,
            "weekday",
            OptionHelpers.OptionType.STRING,
            new String[] {"long", "short", "narrow"},
            JSObjects.Undefined());
    mWeekDay = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.WeekDay.class, weekDay);

    Object era =
        OptionHelpers.GetOption(
            options,
            "era",
            OptionHelpers.OptionType.STRING,
            new String[] {"long", "short", "narrow"},
            JSObjects.Undefined());
    mEra = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Era.class, era);

    Object year =
        OptionHelpers.GetOption(
            options,
            "year",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit"},
            JSObjects.Undefined());
    mYear = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Year.class, year);

    Object month =
        OptionHelpers.GetOption(
            options,
            "month",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit", "long", "short", "narrow"},
            JSObjects.Undefined());
    mMonth = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Month.class, month);

    Object day =
        OptionHelpers.GetOption(
            options,
            "day",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit"},
            JSObjects.Undefined());
    mDay = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Day.class, day);

    Object hour =
        OptionHelpers.GetOption(
            options,
            "hour",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit"},
            JSObjects.Undefined());
    mHour = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Hour.class, hour);

    Object minute =
        OptionHelpers.GetOption(
            options,
            "minute",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit"},
            JSObjects.Undefined());
    mMinute = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Minute.class, minute);

    Object second =
        OptionHelpers.GetOption(
            options,
            "second",
            OptionHelpers.OptionType.STRING,
            new String[] {"numeric", "2-digit"},
            JSObjects.Undefined());
    mSecond = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Second.class, second);

    Object timeZoneName =
        OptionHelpers.GetOption(
            options,
            "timeZoneName",
            OptionHelpers.OptionType.STRING,
            new String[] {
              "long", "longOffset", "longGeneric", "short", "shortOffset", "shortGeneric"
            },
            JSObjects.Undefined());
    mTimeZoneName =
        OptionHelpers.searchEnum(IPlatformDateTimeFormatter.TimeZoneName.class, timeZoneName);

    Object dayPeriod =
        OptionHelpers.GetOption(
            options,
            "dayPeriod",
            OptionHelpers.OptionType.STRING,
            new String[] {"long", "short", "narrow"},
            JSObjects.Undefined());
    mDayPeriod =
        OptionHelpers.searchEnum(IPlatformDateTimeFormatter.DayPeriod.class, dayPeriod);

    mFractionalSecondDigits = OptionHelpers.GetNumberOption(
        options,
        "fractionalSecondDigits",
        1, 3,
        null);

    // 41.
    Object dateStyle =
        OptionHelpers.GetOption(
            options,
            "dateStyle",
            OptionHelpers.OptionType.STRING,
            new String[] {"full", "long", "medium", "short"},
            JSObjects.Undefined());
    mDateStyle = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.DateStyle.class, dateStyle);

    // 43.
    Object timeStyle =
        OptionHelpers.GetOption(
            options,
            "timeStyle",
            OptionHelpers.OptionType.STRING,
            new String[] {"full", "long", "medium", "short"},
            JSObjects.Undefined());
    mTimeStyle = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.TimeStyle.class, timeStyle);

    // 48.
    if (mTimeStyle == IPlatformDateTimeFormatter.TimeStyle.UNDEFINED && mHour == IPlatformDateTimeFormatter.Hour.UNDEFINED) {
      mHourCycle = IPlatformDateTimeFormatter.HourCycle.UNDEFINED;
    }
  }

  @DoNotStrip
  public DateTimeFormat(List<String> locales, Map<String, Object> options)
      throws JSRangeErrorException {

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N)
      mPlatformDateTimeFormatter = new PlatformDateTimeFormatterICU();
    else mPlatformDateTimeFormatter = new PlatformDateTimeFormatterAndroid();

    initializeDateTimeFormat(locales, options);

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
