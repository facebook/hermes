/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.icu.text.DateFormat;
import android.icu.text.NumberingSystem;
import android.icu.util.Calendar;
import android.icu.util.TimeZone;
import android.icu.util.ULocale;
import android.os.Build;
import androidx.annotation.RequiresApi;
import java.text.AttributedCharacterIterator;
import java.util.ArrayList;
import java.util.Date;

public class PlatformDateTimeFormatterICU implements IPlatformDateTimeFormatter {
  private DateFormat mDateFormat = null;

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String format(double n) {
    return mDateFormat.format(new Date((long) n));
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String fieldToString(AttributedCharacterIterator.Attribute field, String fieldValue) {
    if (field == DateFormat.Field.DAY_OF_WEEK) {
      return "weekday";
    }
    if (field == DateFormat.Field.ERA) {
      return "era";
    }
    if (field == DateFormat.Field.YEAR) {
      // TODO:: I can't find the right rules to mark the type of the an year part as "yearName".
      // Likely, the presense of another part with "relatedYear" is the decider ?
      // Currently, i'm differentiating based on whether the value is numeric or not.
      try {
        Double.parseDouble(fieldValue);
        return "year";
      } catch (NumberFormatException nfe) {
        return "yearName";
      }
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
    // TODO:: There must be a better way to do this.
    if (field.toString().equals("android.icu.text.DateFormat$Field(related year)"))
      return "relatedYear";

    // Report unsupported/unexpected date fields as literals.
    return "literal";
  }

  @Override
  public AttributedCharacterIterator formatToParts(double n) {
    return mDateFormat.formatToCharacterIterator(n);
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String getDefaultCalendarName(ILocaleObject<?> mResolvedLocaleObject)
      throws JSRangeErrorException {
    String defaultCalendar =
        DateFormat.getDateInstance(DateFormat.SHORT, (ULocale) mResolvedLocaleObject.getLocale())
            .getCalendar()
            .getType();
    defaultCalendar = UnicodeExtensionKeys.resolveCalendarAlias(defaultCalendar);
    return defaultCalendar;
  }

  private static class PatternUtils {

    public static String getPatternWithoutLiterals(String pattern) {

      StringBuilder segment = new StringBuilder();
      boolean literalSegmentRunning = false;
      for (int idx = 0; idx < pattern.length(); idx++) {
        char c = pattern.charAt(idx);
        if (c == '\'') {
          literalSegmentRunning = !literalSegmentRunning;
          continue;
        }

        if (literalSegmentRunning) continue;

        // ['a'..'z'] and ['A'..'Z']
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) segment.append(pattern.charAt(idx));
      }

      return segment.toString();
    }
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public HourCycle getDefaultHourCycle(ILocaleObject<?> localeObject) throws JSRangeErrorException {
    HourCycle hourCycle;
    try {
      String dateFormatPattern =
          ((android.icu.text.SimpleDateFormat)
                  DateFormat.getTimeInstance(DateFormat.FULL, (ULocale) localeObject.getLocale()))
              .toPattern();
      String dateFormatPatternWithoutLiterals =
          PatternUtils.getPatternWithoutLiterals(dateFormatPattern);
      if (dateFormatPatternWithoutLiterals.contains(String.valueOf('h'))) hourCycle = HourCycle.H12;
      else if (dateFormatPatternWithoutLiterals.contains(String.valueOf('K')))
        hourCycle = HourCycle.H11;
      else if (dateFormatPatternWithoutLiterals.contains(String.valueOf('H')))
        hourCycle = HourCycle.H23;
      else // TODO :: Make it more tight.
      hourCycle = HourCycle.H24;
    } catch (ClassCastException ex) {
      hourCycle = HourCycle.H24;
    }

    return hourCycle;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String getDefaultTimeZone(ILocaleObject<?> localeObject) throws JSRangeErrorException {
    return Calendar.getInstance((ULocale) localeObject.getLocale()).getTimeZone().getID();
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String getDefaultNumberingSystem(ILocaleObject<?> localeObject)
      throws JSRangeErrorException {
    return NumberingSystem.getInstance((ULocale) localeObject.getLocale()).getName();
  }

  private static String getSkeleton(
      WeekDay weekDay,
      Era era,
      Year year,
      Month month,
      Day day,
      Hour hour,
      Minute minute,
      Second second,
      TimeZoneName timeZoneName,
      boolean hour12) {

    StringBuilder skeletonBuffer = new StringBuilder();
    skeletonBuffer.append(weekDay.getSkeleonSymbol());
    skeletonBuffer.append(era.getSkeleonSymbol());
    skeletonBuffer.append(year.getSkeleonSymbol());
    skeletonBuffer.append(month.getSkeleonSymbol());
    skeletonBuffer.append(day.getSkeleonSymbol());

    if (hour12) skeletonBuffer.append(hour.getSkeleonSymbol12());
    else skeletonBuffer.append(hour.getSkeleonSymbol24());

    skeletonBuffer.append(minute.getSkeleonSymbol());
    skeletonBuffer.append(second.getSkeleonSymbol());
    skeletonBuffer.append(timeZoneName.getSkeleonSymbol());

    return skeletonBuffer.toString();
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  public void configure(
      ILocaleObject<?> resolvedLocaleObject,
      String calendar,
      String numberingSystem,
      FormatMatcher formatMatcher,
      WeekDay weekDay,
      Era era,
      Year year,
      Month month,
      Day day,
      Hour hour,
      Minute minute,
      Second second,
      TimeZoneName timeZoneName,
      HourCycle hourCycle,
      Object timeZone)
      throws JSRangeErrorException {
    String skeleton =
        getSkeleton(
            weekDay,
            era,
            year,
            month,
            day,
            hour,
            minute,
            second,
            timeZoneName,
            hourCycle == HourCycle.H11 || hourCycle == HourCycle.H12);

    Calendar calendarInstance = null;
    if (!calendar.isEmpty()) {
      ArrayList<String> calendarList = new ArrayList<>();
      calendarList.add(JSObjects.getJavaString(calendar));

      ILocaleObject<?> modifiedLocaleObject = resolvedLocaleObject.cloneObject();
      modifiedLocaleObject.setUnicodeExtensions("ca", calendarList);

      calendarInstance = Calendar.getInstance((ULocale) modifiedLocaleObject.getLocale());
    }

    if (!numberingSystem.isEmpty()) {
      NumberingSystem numberingSystemObject;
      try {
        numberingSystemObject =
            NumberingSystem.getInstanceByName(JSObjects.getJavaString(numberingSystem));
      } catch (RuntimeException ex) {
        throw new JSRangeErrorException("Invalid numbering system: " + numberingSystem);
      }

      if (numberingSystemObject == null)
        throw new JSRangeErrorException("Invalid numbering system: " + numberingSystem);

      ArrayList<String> numberingSystemList = new ArrayList<>();
      numberingSystemList.add(JSObjects.getJavaString(numberingSystem));

      // TODO : Make sure that this is not shown through the resolvedOptions
      resolvedLocaleObject.setUnicodeExtensions("nu", numberingSystemList);
    }

    if (calendarInstance != null)
      mDateFormat =
          DateFormat.getPatternInstance(
              calendarInstance, skeleton, (ULocale) resolvedLocaleObject.getLocale());
    else
      mDateFormat =
          DateFormat.getPatternInstance(skeleton, (ULocale) resolvedLocaleObject.getLocale());

    if (!JSObjects.isUndefined(timeZone) && !JSObjects.isNull(timeZone)) {
      TimeZone timeZoneObject = TimeZone.getTimeZone(JSObjects.getJavaString(timeZone));
      mDateFormat.setTimeZone(timeZoneObject);
    }
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String[] getAvailableLocales() {
    ArrayList<String> availableLocaleIds = new ArrayList<>();

    // [[Comment copied here from NumberFormat]]
    // NumberFormat.getAvailableLocales() returns a shorter list compared to
    // ULocale.getAvailableLocales.
    // For e.g. "zh-TW" is missing in the list returned by NumberFormat.getAvailableLocales() in my
    // emulator.
    // But, NumberFormatter is able to format specific to "zh-TW" .. for instance "NaN" is expected
    // to be formatted as "非數值" in "zh-TW" by as "NaN" in "zh"
    // In short, NumberFormat.getAvailableLocales() doesn't contain all the locales as the
    // NumberFormat can format. Hence, using ULocale.getAvailableLocales()
    //

    // java.util.Locale[] availableLocales = android.icu.text.DateFormat.getAvailableLocales();
    android.icu.util.ULocale[] availableLocales = ULocale.getAvailableLocales();
    for (android.icu.util.ULocale locale : availableLocales) {
      availableLocaleIds.add(locale.toLanguageTag());
    }

    String[] availableLocaleIdsArray = new String[availableLocaleIds.size()];
    return availableLocaleIds.toArray(availableLocaleIdsArray);
  }

  PlatformDateTimeFormatterICU() {}
}
