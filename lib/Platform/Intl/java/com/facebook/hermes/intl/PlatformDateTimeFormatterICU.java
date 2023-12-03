/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.DateStyle.UNDEFINED;

import android.icu.text.DateFormat;
import android.icu.text.DateIntervalFormat;
import android.icu.text.NumberingSystem;
import android.icu.text.SimpleDateFormat;
import android.icu.util.Calendar;
import android.icu.util.DateInterval;
import android.icu.util.TimeZone;
import android.icu.util.ULocale;
import android.os.Build;

import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import java.text.AttributedCharacterIterator;
import java.text.AttributedString;
import java.text.CharacterIterator;
import java.text.FieldPosition;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class PlatformDateTimeFormatterICU implements IPlatformDateTimeFormatter {
  public static String FALLBACK_SEPARATOR = " - ";

  private DateFormat mDateFormat = null;
  private DateIntervalFormat mDateIntervalFormat = null;

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String format(double n) {
    return mDateFormat.format(new Date((long) n));
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  public String formatRange(double from, double to) {
    DateInterval interval = new DateInterval((long) from, (long) to);
    StringBuffer buf = new StringBuffer();

    mDateIntervalFormat.format(
      interval,
      buf,
      // Ignore the position (0 is the beginning)
      new FieldPosition(0)
    );

    return buf.toString();
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String fieldAttrsToTypeString(Set<Map.Entry<AttributedCharacterIterator.Attribute, Object>> attrs, String fieldValue) {
    Iterator<Map.Entry<AttributedCharacterIterator.Attribute, Object>> iterator = attrs.iterator();

    if (!iterator.hasNext()) {
      return "literal";
    }

    Map.Entry<AttributedCharacterIterator.Attribute, Object> entry;
    do {
      entry = iterator.next();

      if (entry.getKey() instanceof DateFormat.Field || entry.getKey().toString().startsWith("android.icu.text.DateFormat$Field")) {
        return this.fieldToString(entry.getKey(), fieldValue);
      }
    } while (iterator.hasNext());

    return "literal";
  }

  @Override
  public String fieldAttrsToSourceString(Set<Map.Entry<AttributedCharacterIterator.Attribute, Object>> attrs) {
    Iterator<Map.Entry<AttributedCharacterIterator.Attribute, Object>> iterator = attrs.iterator();

    if (!iterator.hasNext()) {
      return "shared";
    }

    Map.Entry<AttributedCharacterIterator.Attribute, Object> entry;
    Object value = null;
    do {
      entry = iterator.next();

      if (entry.getKey().toString().equals("android.icu.text.DateIntervalFormat$SpanField(date-interval-span)")) {
        value = entry.getValue();
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

  @RequiresApi(api = Build.VERSION_CODES.N)
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

    // Flexible day period is only supported on API 28+
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P && field == DateFormat.Field.FLEXIBLE_DAY_PERIOD) {
        return "dayPeriod";
    }

    if (field == DateFormat.Field.AM_PM) {
      return "dayPeriod";
    }

    if (field == DateFormat.Field.MILLISECOND) {
      return "fractionalSecond";
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
  private AttributedCharacterIterator formatIntervalToCharacterIteratorFallback(double from, double to) {
    AttributedCharacterIterator fromIterator = mDateFormat.formatToCharacterIterator(from);
    AttributedCharacterIterator toIterator = mDateFormat.formatToCharacterIterator(to);

    StringBuilder sb = new StringBuilder();

    char ch = fromIterator.current();
    while (ch != CharacterIterator.DONE)
    {
      sb.append(ch);
      ch = fromIterator.next();
    }

    int separatorLength = FALLBACK_SEPARATOR.length();
    sb.append(FALLBACK_SEPARATOR);

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



  @RequiresApi(api = Build.VERSION_CODES.S)
  private AttributedCharacterIterator formatIntervalToCharacterIterator(double from, double to) {
    // Only formatToValue annotates the character iterator with "from" and "to" attributes
    DateIntervalFormat.FormattedDateInterval interval = mDateIntervalFormat.formatToValue(new DateInterval((long) from, (long) to));

    return interval.toCharacterIterator();
  }


  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public AttributedCharacterIterator formatRangeToParts(double from, double to) {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
      return this.formatIntervalToCharacterIterator(from, to);
    } else {
      return this.formatIntervalToCharacterIteratorFallback(from, to);
    }
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
          ((android.icu.text.SimpleDateFormat)
                  DateFormat.getTimeInstance(DateFormat.FULL, (ULocale) localeObject.getLocale()))
              .toPattern();
      String dateFormatPatternWithoutLiterals =
          PatternUtils.getPatternWithoutLiterals(dateFormatPattern);

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

  @RequiresApi(api = Build.VERSION_CODES.N)
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

      String skeleton = "hm";

      android.icu.text.SimpleDateFormat sdf =
              ((android.icu.text.SimpleDateFormat)
                      DateFormat.getInstanceForSkeleton(skeleton, (ULocale) localeObject.getLocale()));

      String pattern = PatternUtils.getPatternWithoutLiterals(sdf.toPattern());

      if (pattern.contains("K")) {
        return HourCycle.H11;
      } else if (pattern.contains("h")) {
        return HourCycle.H12;
      }
    } catch (IllegalArgumentException ex) {
      return HourCycle.H11;
    } catch (ClassCastException ex) {
      return HourCycle.H11;
    }

    return HourCycle.H11;
  }


  @RequiresApi(api = Build.VERSION_CODES.N)
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
      String skeleton = "Hm";

      android.icu.text.SimpleDateFormat sdf =
              ((android.icu.text.SimpleDateFormat)
                      DateFormat.getInstanceForSkeleton(skeleton, (ULocale) localeObject.getLocale()));

      String pattern = PatternUtils.getPatternWithoutLiterals(sdf.toPattern());

      if (pattern.contains("k")) {
        return HourCycle.H24;
      } else if (pattern.contains("H")) {
        return HourCycle.H23;
      }
    } catch (IllegalArgumentException ex) {
      return HourCycle.H23;
    } catch (ClassCastException ex) {
      return HourCycle.H23;
    }

    return HourCycle.H23;
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

  @RequiresApi(api = Build.VERSION_CODES.N)
  static int toICUDateStyle(DateStyle dateStyle) throws JSRangeErrorException {
    switch (dateStyle) {
      case FULL:
        return DateFormat.FULL;
      case LONG:
        return DateFormat.LONG;
      case MEDIUM:
        return DateFormat.MEDIUM;
      case SHORT:
        return DateFormat.SHORT;
      case UNDEFINED:
      default:
        throw new JSRangeErrorException("Invalid DateStyle: " + dateStyle.toString());
    }
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  static int toICUTimeStyle(TimeStyle timeStyle) throws JSRangeErrorException {
    switch (timeStyle) {
      case FULL:
        return DateFormat.FULL;
      case LONG:
        return DateFormat.LONG;
      case MEDIUM:
        return DateFormat.MEDIUM;
      case SHORT:
        return DateFormat.SHORT;
      case UNDEFINED:
      default:
        throw new JSRangeErrorException("Invalid DateStyle: " + timeStyle.toString());
    }
  }

  private static void replaceChars(StringBuilder builder, String from, String to) {
    int index = builder.indexOf(from);
    if (index != -1) {
      builder.replace(index, index + from.length(), to);
    }
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  private static String getPatternForStyle(
      ILocaleObject<?> resolvedLocaleObject, DateStyle dateStyle, TimeStyle timeStyle)
      throws JSRangeErrorException {
    if (dateStyle == UNDEFINED) {
      return ((SimpleDateFormat)
              DateFormat.getTimeInstance(
                  toICUTimeStyle(timeStyle), (ULocale) resolvedLocaleObject.getLocale()))
          .toLocalizedPattern();
    } else if (timeStyle == TimeStyle.UNDEFINED) {
      return ((SimpleDateFormat)
              DateFormat.getDateInstance(
                  toICUDateStyle(dateStyle), (ULocale) resolvedLocaleObject.getLocale()))
          .toLocalizedPattern();
    } else {
      return ((SimpleDateFormat)
              DateFormat.getDateTimeInstance(
                  toICUDateStyle(dateStyle),
                  toICUTimeStyle(timeStyle),
                  (ULocale) resolvedLocaleObject.getLocale()))
          .toLocalizedPattern();
    }
  }

  // Workaround for ICU-21939 based on Oracle's workaround for GraalJS:
  // https://github.com/oracle/graaljs/blob/release/graal-vm/22.2/graal-js/src/com.oracle.truffle.js/src/com/oracle/truffle/js/runtime/builtins/intl/JSDateTimeFormat.java#L367
  // workaround for ICU-21939, replaces less common year-related parts
  // of the skeleton by the most common pattern symbol for year
  // and removes day-period-related pattern symbols
  private static String patchSkeletonToAvoidICU21939(String skeleton) {
    StringBuilder sb = new StringBuilder();
    for (char c : skeleton.toCharArray()) {
      switch (c) {
        case 'a':
        case 'b':
        case 'B':
          continue;
        case 'Y':
        case 'u':
        case 'U':
        case 'r':
          sb.append('y');
          break;
        default:
          sb.append(c);
          break;
      }
    }
    return sb.toString();
  }

  private static void replacePatternChars(StringBuilder skeletonBuffer, char[] fromSet, char to) {
    for (int idx = 0; idx < skeletonBuffer.length(); idx++) {
      for (char fromChar : fromSet) {
        if (skeletonBuffer.charAt(idx) == fromChar) {
          skeletonBuffer.setCharAt(idx, to);
          break; // we don't expect more than once fromChar to be present in the pattern.
        }
      }
    }
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  private static String getSkeleton(
      ILocaleObject<?> resolvedLocaleObject,
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
      DateStyle dateStyle,
      TimeStyle timeStyle,
      Object hour12,
      DayPeriod dayPeriod,
      @Nullable Integer fractionalSecondDigits)
      throws JSRangeErrorException {

    StringBuilder skeletonBuffer = new StringBuilder();

    // For reference on patterns/skeleton:
    // https://docs.oracle.com/javase/8/docs/api/java/text/SimpleDateFormat.html

    if (dateStyle != DateStyle.UNDEFINED || timeStyle != TimeStyle.UNDEFINED) {
      skeletonBuffer.append(patchSkeletonToAvoidICU21939(getPatternForStyle(resolvedLocaleObject, dateStyle, timeStyle)));

       if (hourCycle != HourCycle.UNDEFINED) {
        replacePatternChars(skeletonBuffer, new char[] {
                HourCycle.H11.getSkeletonSymbol(),
                HourCycle.H12.getSkeletonSymbol(),
                HourCycle.H23.getSkeletonSymbol(),
                HourCycle.H24.getSkeletonSymbol()
        }, hourCycle.getSkeletonSymbol());
      }
    } else {
      skeletonBuffer.append(weekDay.getSkeletonSymbol());
      skeletonBuffer.append(era.getSkeletonSymbol());
      skeletonBuffer.append(year.getSkeletonSymbol());
      skeletonBuffer.append(month.getSkeletonSymbol());
      skeletonBuffer.append(day.getSkeletonSymbol());
      skeletonBuffer.append(hour.getSkeletonSymbol(hourCycle));
      skeletonBuffer.append(minute.getSkeletonSymbol());
      skeletonBuffer.append(second.getSkeletonSymbol());
      skeletonBuffer.append(timeZoneName.getSkeletonSymbol());

      // Day period is only technically supported on API 28+,
      // so we fallback to the AM/PM field
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
        skeletonBuffer.append(dayPeriod.getSkeletonSymbol());
      } else {
        skeletonBuffer.append(dayPeriod.getSkeletonSymbolFallback());
      }
    }

    if (fractionalSecondDigits != null) {
      switch (fractionalSecondDigits) {
        case 1:
          skeletonBuffer.append("S");
          break;
        case 2:
          skeletonBuffer.append("SS");
          break;
        case 3:
          skeletonBuffer.append("SSS");
          break;
      }
    }

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
      Object timeZone,
      DateStyle dateStyle,
      TimeStyle timeStyle,
      Object hour12,
      DayPeriod dayPeriod,
      Integer fractionalSecondDigits)
      throws JSRangeErrorException {
    String skeleton =
        getSkeleton(
            resolvedLocaleObject,
            weekDay,
            era,
            year,
            month,
            day,
            hour,
            minute,
            second,
            timeZoneName,
            hourCycle,
            dateStyle,
            timeStyle,
            hour12,
            dayPeriod,
            fractionalSecondDigits);

    ILocaleObject<?> modifiedLocaleObjectWithCalendar = resolvedLocaleObject.cloneObject();
    Calendar calendarInstance = null;
    if (!calendar.isEmpty()) {
      ArrayList<String> calendarList = new ArrayList<>();
      calendarList.add(JSObjects.getJavaString(calendar));

      modifiedLocaleObjectWithCalendar.setUnicodeExtensions("ca", calendarList);

      calendarInstance = Calendar.getInstance((ULocale) modifiedLocaleObjectWithCalendar.getLocale());
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

    mDateIntervalFormat = DateIntervalFormat.getInstance(skeleton, (ULocale) modifiedLocaleObjectWithCalendar.getLocale());

    if (!JSObjects.isUndefined(timeZone) && !JSObjects.isNull(timeZone)) {

      TimeZone timeZoneObject;
      String timeZoneString  = JSObjects.getJavaString(timeZone);
      if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
       timeZoneObject= this.parseTimeZoneIdentifierWithZoneId(timeZoneString);
      } else {
        timeZoneObject = this.parseTimeZoneIdentifierFallback(timeZoneString);
      }

      mDateFormat.setTimeZone(timeZoneObject);
      mDateIntervalFormat.setTimeZone(timeZoneObject);

    }
  }

    @RequiresApi(api = Build.VERSION_CODES.O)
  private TimeZone parseTimeZoneIdentifierWithZoneId(String timeZone) {
      try {
        ZoneId timeZoneObject = ZoneId.of(timeZone);
        if (timeZoneObject instanceof ZoneOffset) {
          timeZoneObject = ZoneId.ofOffset("GMT", (ZoneOffset) timeZoneObject);
        }

        return TimeZone.getTimeZone(timeZoneObject.getId());
    } catch (RuntimeException exc) {
        // TODO: We can't load more specific exceptions below API Level 19
        return TimeZone.UNKNOWN_ZONE;
      }
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  private TimeZone parseTimeZoneIdentifierFallback(String timeZone) {

    // TimeZone.getTimeZone doesn't support raw offset strings
    if (timeZone.startsWith("-") || timeZone.startsWith("+")) {
     return TimeZone.getTimeZone("GMT" + timeZone);
    } else {
    return TimeZone.getTimeZone(timeZone);
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
