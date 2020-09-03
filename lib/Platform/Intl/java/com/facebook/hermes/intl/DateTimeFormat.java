/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.icu.text.DateFormat;
import android.icu.text.DateIntervalFormat;
import android.icu.text.SimpleDateFormat;
import android.icu.util.Calendar;
import android.icu.util.TimeZone;
import android.icu.util.ULocale;
import android.util.TimeFormatException;

import java.lang.reflect.Array;
import java.text.AttributedCharacterIterator;
import java.text.CharacterIterator;
import java.text.FieldPosition;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;

/**
 * This class represents the Java part of the Android
 * Intl.DateTimeFormat implementation.  The interaction with the
 * Hermes JaveScript internals are implemented in C++ and should not
 * generally need to be changed.  Implementers' notes here will
 * describe what parts of the ECMA 402 spec remain to be implemented.
 *
 * Implementer notes:
 *
 * Internal slots: In the ECMA 402 spec, there are a number of
 * references to internal slots.  These are gneerally expressed in
 * terms of JavaScript objects, but the semantics do not generally
 * depend on this.  For example, where the spec says
 * "Intl.DateTimeFormat instances have an
 * [[InitializedDateTimeFormat]] internal slot", this would not be a
 * literal artifact of the implementation.  Internal slots, where
 * necessary, should be represented as members of this java
 * DateTimeFormat object.
 *
 * ICU4J vs Unicode: The ECMA 402 spec makes reference to Unicode
 * documents and data, such as Unicode Technical Standard 35 and the
 * Common Locale Data Repository.  However, in practice, platform
 * Unicode implementations are based on the ICU libraries, which
 * encapsulate the concepts in the Unicode documents and data, and do
 * not provide direct access.  The Android Intl platform code is
 * expected to be implemented in terms of the Android ICU libraries
 * such as android.icu.text
 * <https://developer.android.com/reference/android/icu/text/package-summary>
 * and android.icu.number
 * <https://developer.android.com/reference/android/icu/number/package-summary>,
 * which are themselves derived from the icu4j API
 * <https://unicode-org.github.io/icu-docs/apidoc/released/icu4j/>.
 * This strategy allows for a more space-efficient implementation than
 * one which ships its own copy of the locale data, which can be 20mb
 * or more.  That said, it can be difficult to understand precisely
 * how to implement ECMA 402 in terms of icu4j, and I've left much of
 * the specifics of this up to the implementer.  Comparison with
 * existing implementations may help.
 */
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

  private ULocale mLocale = null;

  private DateFormat mDateFormat = null;

  private String mCalendar = null;
  private Calendar mPlatformCalendarInstance = null;

  private String mDateStyle = null;
  private String mTimeStyle = null;

  private String mWeekDay = null;
  private String mEra = null;
  private String mYear = null;
  private String mMonth = null;
  private String mDay = null;
  private String mHour = null;
  private String mMinute = null;
  private String mSecond = null;
  private String mTimeZoneName = null;

  private String mTimeZone = null;

  private int getDateStyleValue (String styleStr) throws JSRangeErrorException {
    switch (styleStr) {
      case "full":
        return DateFormat.FULL;
      case "long":
        return DateFormat.LONG;
      case "medium":
        return DateFormat.MEDIUM;
      case "short":
        return DateFormat.SHORT;
      default:
        throw new JSRangeErrorException("Unknow date style !");
    }
  }

  private int getTimeStyleValue (String styleStr) throws JSRangeErrorException {
    switch (styleStr) {
      case "full":
        return DateFormat.FULL;
      case "long":
        return DateFormat.LONG;
      case "medium":
        return DateFormat.MEDIUM;
      case "short":
        return DateFormat.SHORT;
      default:
        throw new JSRangeErrorException("Unknow date style !");
    }
  }

  private void initializeFormatterFromDateTimeStyles() throws JSRangeErrorException {

    // TODO :: use setCalendar to simplify this code.
    if(mPlatformCalendarInstance != null ) {
      if(!mDateStyle.isEmpty() && !mTimeStyle.isEmpty()) {
        mDateFormat = DateFormat.getDateTimeInstance(mPlatformCalendarInstance, getDateStyleValue(mDateStyle), getTimeStyleValue(mDateStyle), mLocale);
      } else if (!mDateStyle.isEmpty()) {
        mDateFormat = DateFormat.getDateInstance(mPlatformCalendarInstance, getDateStyleValue(mDateStyle), mLocale);
      }  else if (!mTimeStyle.isEmpty()) {
        mDateFormat = DateFormat.getTimeInstance(mPlatformCalendarInstance, getTimeStyleValue(mTimeStyle), mLocale);
      } else {
        throw new JSRangeErrorException("Expected date or time style");
      }

    } else {
      if(!mDateStyle.isEmpty() && !mTimeStyle.isEmpty()) {
        mDateFormat = DateFormat.getDateTimeInstance(getDateStyleValue(mDateStyle), getTimeStyleValue(mDateStyle), mLocale);
      } else if (!mDateStyle.isEmpty()) {
        mDateFormat = DateFormat.getDateInstance(getDateStyleValue(mDateStyle), mLocale);
      }  else if (!mTimeStyle.isEmpty()) {
        mDateFormat = DateFormat.getTimeInstance(getTimeStyleValue(mTimeStyle), mLocale);
      } else {
        throw new JSRangeErrorException("Expected date or time style");
      }
    }
  }

  private void initializeFormatterFromComponents(ILocaleObject resolvedLocale, String weekDay, String era, String year, String month, String day, String hour, String minute, String second, String timeZoneName) throws JSRangeErrorException {

    StringBuffer skeletonBuffer = new StringBuffer();

    if(!weekDay.isEmpty()) {
      switch (weekDay) {
        case "long":
          skeletonBuffer.append("EEEE");
          break;
        case "short":
          skeletonBuffer.append("EEE");
          break;
        case "narrow":
          skeletonBuffer.append("EEEEE");
          break;
      }
    }

    if(!era.isEmpty()) {
      switch (era) {
        case "long":
          skeletonBuffer.append("GGGG");
          break;
        case "short":
          skeletonBuffer.append("GGG");
          break;
        case "narrow":
          skeletonBuffer.append("G5");
          break;
      }
    }

    if(!year.isEmpty()) {
      switch (year) {
        case "numeric":
          skeletonBuffer.append("yyyy");
          break;
        case "2-digit":
          skeletonBuffer.append("yy");
          break;
      }
    }

    if(!month.isEmpty()) {
      switch (month) {
        case "numeric":
          skeletonBuffer.append("M");
          break;
        case "2-digit":
          skeletonBuffer.append("MM");
          break;
      }
    }

    if(!day.isEmpty()) {
      switch (day) {
        case "numeric":
          skeletonBuffer.append("d");
          break;
        case "2-digit":
          skeletonBuffer.append("dd");
          break;
      }
    }

    // TODO
    if(!hour.isEmpty()) {
      switch (hour) {
        case "numeric":
          skeletonBuffer.append("h");
          break;
        case "2-digit":
          skeletonBuffer.append("HH");
          break;
      }
    }

    if(!minute.isEmpty()) {
      switch (minute) {
        case "numeric":
          skeletonBuffer.append("m");
          break;
        case "2-digit":
          skeletonBuffer.append("mm");
          break;
      }
    }

    if(!second.isEmpty()) {
      switch (second) {
        case "numeric":
          skeletonBuffer.append("s");
          break;
        case "2-digit":
          skeletonBuffer.append("ss");
          break;
      }
    }

    if(!timeZoneName.isEmpty()) {
      switch (timeZoneName) {
        case "long":
          skeletonBuffer.append("VV");
          break;
        case "short":
          skeletonBuffer.append("O");
          break;
      }
    }

    mDateFormat  = DateFormat.getPatternInstance(skeletonBuffer.toString(), (ULocale) resolvedLocale.getLocale());
  }

  public DateTimeFormat(List<String> locales, Map<String, Object> options)
    throws JSRangeErrorException
  {
    String desiredLocaleMatcher = OptionHelpers.resolveStringOption(options, Constants.LOCALEMATCHER, Constants.LOCALEMATCHER_POSSIBLE_VALUES, Constants.LOCALEMATCHER_BESTFIT);
    PlatformCollator.LocaleResolutionResult localeResolutionResult = PlatformNumberFormatter.resolveLocales(locales, desiredLocaleMatcher);

    mLocale = (ULocale) localeResolutionResult.resolvedLocale.getLocale();

    mCalendar = OptionHelpers.resolveStringOption(options, "calendar", new String [] { "buddhist", "chinese", "coptic", "ethiopia", "ethiopic", "gregory", "hebrew", "indian", "islamic", "iso8601", "japanese", "persian", "roc"}, "");

    if(!mCalendar.isEmpty()) {
      ILocaleObject locale = localeResolutionResult.resolvedLocale.cloneObject();
      ArrayList<String> calendarList = new ArrayList<>();
      calendarList.add(mCalendar);
      locale.setUnicodeExtensions("ca", calendarList); // TODO

      mPlatformCalendarInstance = Calendar.getInstance((ULocale) locale.getLocale());
    }

    mDateStyle = OptionHelpers.resolveStringOption(options, "dateStyle", new String [] { "full", "long", "medium", "short"}, "");
    mTimeStyle = OptionHelpers.resolveStringOption(options, "timeStyle", new String [] { "full", "long", "medium", "short"}, "");

    mTimeZone = OptionHelpers.resolveStringOption(options, "timeZone", new String [] {}, "");

    if(!mDateStyle.isEmpty() || !mTimeStyle.isEmpty()) {
      initializeFormatterFromDateTimeStyles();
    } else {
      mWeekDay = OptionHelpers.resolveStringOption(options, "weekday", new String[]{"long", "short", "narrow"}, "");
      mEra = OptionHelpers.resolveStringOption(options, "era", new String[]{"long", "short", "narrow"}, "");
      mYear = OptionHelpers.resolveStringOption(options, "year", new String[]{"numeric", "2-digit"}, "");
      mMonth = OptionHelpers.resolveStringOption(options, "month", new String[]{"numeric", "2-digit", "long", "short", "narrow"}, "");
      mDay = OptionHelpers.resolveStringOption(options, "day", new String[]{"numeric", "2-digit"}, "");
      mHour = OptionHelpers.resolveStringOption(options, "hour", new String[]{"numeric", "2-digit"}, "");
      mMinute = OptionHelpers.resolveStringOption(options, "minute", new String[]{"numeric", "2-digit"}, "");
      mSecond = OptionHelpers.resolveStringOption(options, "second", new String[]{"numeric", "2-digit"}, "");
      mTimeZoneName = OptionHelpers.resolveStringOption(options, "timeZoneName", new String[]{"long", "short"}, "");

      if (mWeekDay.isEmpty() &&
              mEra.isEmpty() &&
              mYear.isEmpty() &&
              mMonth.isEmpty() &&
              mDay.isEmpty() &&
              mHour.isEmpty() &&
              mMinute.isEmpty() &&
              mSecond.isEmpty() &&
              mTimeZoneName.isEmpty()) {
        mYear = "numeric";
        mMonth = "numeric";
        mDay = "numeric";
      }
      initializeFormatterFromComponents(localeResolutionResult.resolvedLocale ,
              mWeekDay ,
              mEra,
              mYear,
              mMonth,
              mDay,
              mHour, mMinute, mSecond, mTimeZoneName);
    }

    if(!mTimeZone.isEmpty()) {
      TimeZone timeZone = TimeZone.getTimeZone(mTimeZone);
      mDateFormat.setTimeZone(timeZone);
    }

  }

  // options are localeMatcher:string
  //
  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.datetimeformat.supportedlocalesof.
  //
  // The notes on the ctor for Locales and Options also apply here.
  public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options) {
    return locales;
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
  public Map<String, Object> resolvedOptions() {
    return new HashMap<String, Object>();
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatdatetime
  //
  // Steps 1 and 2 of PartitionDateTimePattern are already done.  The
  // formal steps require construction of several internal
  // NumberFormat JavaScript objects, but these objects are never
  // exposed; it should be possible to create and use java
  // NumberFormat objects only.
  public String format(double jsTimeValue) {
    String result = mDateFormat.format(new Date((long) jsTimeValue));
    return result;
  }

  // Copied from https://github.com/anba/es6draft/blob/5487e074da2f9489fa2d8c653f037c0969bb2a68/src/main/java/com/github/anba/es6draft/runtime/objects/intl/DateTimeFormatConstructor.java
  private static String fieldToString(DateFormat.Field field) {
    if (field == DateFormat.Field.DAY_OF_WEEK) {
      return "weekday";
    }
    if (field == DateFormat.Field.ERA) {
      return "era";
    }
    if (field == DateFormat.Field.YEAR) {
      return "year";
    }
    if (field == DateFormat.Field.MONTH) {
      return "month";
    }
    if (field == DateFormat.Field.DAY_OF_MONTH) {
      return "day";
    }
    if (field == DateFormat.Field.HOUR0) {
      return "hour";
    }
    if (field == DateFormat.Field.HOUR1) {
      return "hour";
    }
    if (field == DateFormat.Field.HOUR_OF_DAY0) {
      return "hour";
    }
    if (field == DateFormat.Field.HOUR_OF_DAY1) {
      return "hour";
    }
    if (field == DateFormat.Field.MINUTE) {
      return "minute";
    }
    if (field == DateFormat.Field.SECOND) {
      return "second";
    }
    if (field == DateFormat.Field.TIME_ZONE) {
      return "timeZoneName";
    }
    if (field == DateFormat.Field.AM_PM) {
      return "dayPeriod";
    }
    // Report unsupported/unexpected date fields as literals.
    return "literal";
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatdatetimetoparts
  public List<Map<String, String>> formatToParts(double jsTimeValue) {
    ArrayList<Map<String, String>> ret = new ArrayList<Map<String, String>>();

    // TODO :: Redo the code.
    AttributedCharacterIterator iterator = mDateFormat.formatToCharacterIterator(jsTimeValue);
    StringBuilder sb = new StringBuilder();
    for (char ch = iterator.first(); ch != CharacterIterator.DONE; ch = iterator.next()) {
      sb.append(ch);
      if (iterator.getIndex() + 1 == iterator.getRunLimit()) {
        Iterator<AttributedCharacterIterator.Attribute> keyIterator = iterator.getAttributes().keySet().iterator();
        String key;
        if (keyIterator.hasNext()) {
          key  = fieldToString((DateFormat.Field) keyIterator.next());
        } else {
          key = "literal";
        }
        String value = sb.toString();
        sb.setLength(0);

        HashMap<String, String> part = new HashMap();
        part.put(key, value);
        ret.add(part);
      }
    }

    return ret;
  }
}
