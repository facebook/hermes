/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.os.Build;

import androidx.annotation.Nullable;

import java.text.AttributedCharacterIterator;
import java.text.AttributedString;
import java.text.CharacterIterator;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.TimeZone;

public class PlatformDateTimeFormatterAndroid implements IPlatformDateTimeFormatter {
  public static String SEPARATOR = " - ";
  private DateFormat mDateFormat = null;

  @Override
  public String format(double n) {
    return mDateFormat.format(new Date((long) n));
  }

  @Override
  public String formatRange(double from, double to) {
    return mDateFormat.format(new Date((long) from)) + SEPARATOR + mDateFormat.format(new Date((long) to));
  }

  public String fieldAttrsToSourceString(Set<Map.Entry<AttributedCharacterIterator.Attribute, Object>> attrs) {
    Iterator<Map.Entry<AttributedCharacterIterator.Attribute, Object>> iterator = attrs.iterator();

    if (!iterator.hasNext()) {
      return "shared";
    }

    Map.Entry<AttributedCharacterIterator.Attribute, Object> entry;
    Object value = null;
    do {
      entry = iterator.next();

      if (entry.getKey() == SpanField.DATE_INTERVAL_SPAN) {
        value = iterator.next().getValue();
      }
    } while (iterator.hasNext());

    if (!(value instanceof Integer)) {
      return "shared";
    }

    if ((Integer) value == 0) {
      return "startRange";
    }

    if ((Integer) value == 1) {
      return "endRange";
    }

    return "shared";
  }

  @Override
  public String fieldAttrsToTypeString(Set<Map.Entry<AttributedCharacterIterator.Attribute, Object>> attrs, String fieldValue) {
    Iterator<Map.Entry<AttributedCharacterIterator.Attribute, Object>> iterator = attrs.iterator();

    if (!iterator.hasNext()) {
      return "literal";
    }

    Map.Entry<AttributedCharacterIterator.Attribute, Object> entry;
    do {
      entry = iterator.next();

      if (entry.getKey() instanceof DateFormat.Field || entry.getKey().toString().startsWith("java.text.DateFormat$Field")) {
        return this.fieldToString(entry.getKey(), fieldValue);
      }
    } while (iterator.hasNext());

    return "literal";
  }

  private String fieldToString(AttributedCharacterIterator.Attribute field, String fieldValue) {
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
    if (field == DateFormat.Field.MILLISECOND) {
      return "fractionalSecond";
    }
    // TODO:: There must be a better way to do this.
    if (field.toString().equals("java.text.DateFormat$Field(related year)"))
      return "relatedYear";

    // Report unsupported/unexpected date fields as literals.
    return "literal";
  }

  @Override
  public AttributedCharacterIterator formatToParts(double n) {
    return mDateFormat.formatToCharacterIterator(n);
  }

  enum DateTimeFormatterSource {
    START_RANGE,
    SHARED,
    END_RANGE
  }


  @Override
  public AttributedCharacterIterator formatRangeToParts(double from, double to) {
    AttributedCharacterIterator fromIterator = mDateFormat.formatToCharacterIterator(from);
    AttributedCharacterIterator toIterator = mDateFormat.formatToCharacterIterator(to);

    StringBuilder sb = new StringBuilder();

    char ch = fromIterator.current();
    while (ch != CharacterIterator.DONE)
    {
      sb.append(ch);
      ch = fromIterator.next();
    }

    int separatorLength = SEPARATOR.length();
    sb.append(SEPARATOR);

    ch = toIterator.current();
    while (ch != CharacterIterator.DONE)
    {
      sb.append(ch);

      ch = toIterator.next();
    }

    AttributedString combined = new AttributedString(sb.toString());

    fromIterator = mDateFormat.formatToCharacterIterator(from);
    toIterator = mDateFormat.formatToCharacterIterator(to);

    HashMap<AttributedCharacterIterator.Attribute, Object> map = new HashMap<AttributedCharacterIterator.Attribute, Object>();
    map.put(SpanField.DATE_INTERVAL_SPAN, 0);
    combined.addAttributes(map, 0, fromIterator.getEndIndex());

    int fromIndex = fromIterator.getEndIndex();

    map = new HashMap<>();
    map.put(SpanField.DATE_INTERVAL_SPAN, 2);
    combined.addAttributes(map, fromIndex, fromIndex + separatorLength);

    map = new HashMap<>();
    map.put(SpanField.DATE_INTERVAL_SPAN, 1);

    int toIndex = toIterator.getEndIndex();
    combined.addAttributes(map, fromIndex + separatorLength, fromIndex + separatorLength + toIndex);

    return combined.getIterator();
  }


  @Override
  public String getDefaultCalendarName(ILocaleObject<?> mResolvedLocaleObject)
      throws JSRangeErrorException {
    return DateFormat.getDateInstance(
            java.text.DateFormat.SHORT, (Locale) mResolvedLocaleObject.getLocale())
        .getCalendar()
        .toString();
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

  @Override
  public HourCycle getHourCycle(ILocaleObject<?> localeObject) throws JSRangeErrorException {
    List<String> exts = localeObject.getUnicodeExtensions("hc");
    if (exts.size() > 0) {
      if (exts.contains("h11")) {
        return HourCycle.H11;
      } else if (exts.contains("h12")) {
        return HourCycle.H12;
      } else if (exts.contains("h23")) {
        return HourCycle.H23;
      } else if (exts.contains("h24")) {
        return HourCycle.H24;
      }
    }

    HourCycle hourCycle;

    try {
      String dateFormatPattern =
              ((SimpleDateFormat)
                      DateFormat.getTimeInstance(DateFormat.FULL, (Locale) localeObject.getLocale()))
                      .toPattern();
      String dateFormatPatternWithoutLiterals = PatternUtils.getPatternWithoutLiterals(dateFormatPattern);

      if (dateFormatPatternWithoutLiterals.contains(String.valueOf('h')))
        hourCycle = HourCycle.H12;
      else if (dateFormatPatternWithoutLiterals.contains(String.valueOf('K')))
        hourCycle = HourCycle.H11;
      else if (dateFormatPatternWithoutLiterals.contains(String.valueOf('H')))
        hourCycle = HourCycle.H23;
      else if (dateFormatPatternWithoutLiterals.contains(String.valueOf('k')))
        hourCycle = HourCycle.H24;
      else
        hourCycle = HourCycle.H23;
    } catch (ClassCastException ex) {
      hourCycle = HourCycle.H23;
    }

    return hourCycle;
  }

  @Override
  public HourCycle getHourCycle12(ILocaleObject<?> localeObject) throws JSRangeErrorException {
    List<String> exts = localeObject.getUnicodeExtensions("hc");
    if (exts.size() > 0) {
      if (exts.contains("h11") || exts.contains("h23")) {
        return HourCycle.H11;
      } else if (exts.contains("h12") || exts.contains("h24")) {
        return HourCycle.H12;
      }
    }

    try {
      SimpleDateFormat sdf =
              ((SimpleDateFormat)
                      DateFormat.getTimeInstance(DateFormat.FULL, (Locale) localeObject.getLocale()));

      String pattern = PatternUtils.getPatternWithoutLiterals(sdf.toPattern());

      if (pattern.contains("K") || pattern.contains("H")) {
        return HourCycle.H11;
      } else if (pattern.contains("k") || pattern.contains("h")) {
        return HourCycle.H12;
      }
    } catch (IllegalArgumentException ex) {
    } catch (ClassCastException ex) {
    }

    return HourCycle.H11;
  }

  @Override
  public HourCycle getHourCycle24(ILocaleObject<?> localeObject) throws JSRangeErrorException {
    List<String> exts = localeObject.getUnicodeExtensions("hc");

    if (exts.size() > 0) {
      if (exts.contains("h23") || exts.contains("h11")) {
        return HourCycle.H23;
      } else if (exts.contains("h24") || exts.contains("h12")) {
        return HourCycle.H24;
      }
    }


    try {
      SimpleDateFormat sdf =
              ((SimpleDateFormat)
                      DateFormat.getTimeInstance(DateFormat.FULL, (Locale) localeObject.getLocale()));

      String pattern = PatternUtils.getPatternWithoutLiterals(sdf.toPattern());

      if (pattern.contains("K") || pattern.contains("H")) {
        return HourCycle.H23;
      } else if (pattern.contains("k") || pattern.contains("h")) {
        return HourCycle.H24;
      }
    } catch (IllegalArgumentException ex) {
    } catch (ClassCastException ex) {
    }

    return HourCycle.H23;
  }



  @Override
  public String getDefaultTimeZone(ILocaleObject<?> localeObject) throws JSRangeErrorException {
    return Calendar.getInstance((Locale) localeObject.getLocale()).getTimeZone().getID();
  }

  @Override
  public String getDefaultNumberingSystem(ILocaleObject<?> localeObject) {
    return "latn";
  }

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
      Object timeZone,
      DateStyle dateStyle,
      TimeStyle timeStyle,
      Object hour12,
      DayPeriod dayPeriod,
      @Nullable Integer fractionalSecondDigits)
      throws JSRangeErrorException {
    if (!calendar.isEmpty()) {
      ArrayList<String> calendarList = new ArrayList<>();
      calendarList.add(JSObjects.getJavaString(calendar));

      resolvedLocaleObject.setUnicodeExtensions("ca", calendarList);
    }

    if (!numberingSystem.isEmpty()) {

      ArrayList<String> numberingSystemList = new ArrayList<>();
      numberingSystemList.add(JSObjects.getJavaString(numberingSystem));

      resolvedLocaleObject.setUnicodeExtensions("nu", numberingSystemList);
    }

    boolean needDate = year != null || month != null || day != null;
    boolean needTime = hour != null || minute != null || second != null;

    if (needDate && needTime)
      mDateFormat =
          DateFormat.getDateTimeInstance(
              DateFormat.FULL, DateFormat.FULL, (Locale) resolvedLocaleObject.getLocale());
    else if (needDate)
      mDateFormat =
          DateFormat.getDateInstance(DateFormat.FULL, (Locale) resolvedLocaleObject.getLocale());
    else if (needTime)
      mDateFormat =
          DateFormat.getTimeInstance(DateFormat.FULL, (Locale) resolvedLocaleObject.getLocale());

    if (!JSObjects.isUndefined(timeZone) && !JSObjects.isNull(timeZone)) {
      TimeZone timeZoneObject = this.parseTimeZoneIdentifierFallback(JSObjects.getJavaString(timeZone));
      mDateFormat.setTimeZone(timeZoneObject);
    }
  }

  private TimeZone parseTimeZoneIdentifierFallback(String timeZone) {

    // TimeZone.getTimeZone doesn't support raw offset strings
    if (timeZone.startsWith("-") || timeZone.startsWith("+")) {
      return TimeZone.getTimeZone("GMT" + timeZone);
    } else {
      return TimeZone.getTimeZone(timeZone);
    }

  }

  @Override
  public String[] getAvailableLocales() {

    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
      // Before L, Locale.toLanguageTag isn't available. Need to figure out how to get a locale id
      // from locale object ... Currently resoring to support only en
      return new String[] {"en"};
    }

    ArrayList<String> availableLocaleIds = new ArrayList<>();
    java.util.Locale[] availableLocales = java.text.DateFormat.getAvailableLocales();
    for (java.util.Locale locale : availableLocales) {
      availableLocaleIds.add(locale.toLanguageTag()); // TODO:: Not available on platforms <= 20
    }

    String[] availableLocaleIdsArray = new String[availableLocaleIds.size()];
    return availableLocaleIds.toArray(availableLocaleIdsArray);
  }

  PlatformDateTimeFormatterAndroid() {}
}
