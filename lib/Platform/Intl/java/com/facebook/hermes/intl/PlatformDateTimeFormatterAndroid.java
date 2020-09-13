package com.facebook.hermes.intl;

import android.os.Build;

import java.text.AttributedCharacterIterator;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

public class PlatformDateTimeFormatterAndroid implements IPlatformDateTimeFormatter{
    private DateFormat mDateFormat = null;
    private ILocaleObject mLocale = null;

    @Override
    public String format(double n) throws JSRangeErrorException {
        String result = mDateFormat.format(new Date((long) n));
        return result;
    }

    @Override
    public String fieldToString(AttributedCharacterIterator.Attribute field) {
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

    @Override
    public AttributedCharacterIterator formatToParts(double n) throws JSRangeErrorException {
        return mDateFormat.formatToCharacterIterator(n);
    }

    @Override
    public String getCalendarName() {
        return mDateFormat.getCalendar().toString();
    }

    @Override
    public String getTimeZoneName() throws JSRangeErrorException {
        return mDateFormat.getTimeZone().getDisplayName(false, TimeZone.SHORT, (Locale) mLocale.getLocale());
    }

    private static class PatternUtils {

        public static String getPatternWithoutLiterals(String pattern) {

            StringBuffer segment = new StringBuffer();
            boolean literalSegmentRunning = false;
            for (int idx = 0; idx < pattern.length(); idx++) {
                char c = pattern.charAt(idx);
                if (c == '\'') {
                    if (literalSegmentRunning) {
                        literalSegmentRunning = false;
                    } else {
                        literalSegmentRunning = true;
                    }
                    continue;
                }

                if (literalSegmentRunning)
                    continue;

                // ['a'..'z'] and ['A'..'Z']
                if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
                    segment.append(pattern.charAt(idx));
            }

            return segment.toString();
        }
    }

    @Override
    public String getDefaultHourCycle(ILocaleObject localeObject) throws JSRangeErrorException {
        String hourCycle;
        try {
            String dateFormatPattern = ((SimpleDateFormat) DateFormat.getTimeInstance(DateFormat.FULL, (Locale) localeObject.getLocale())).toPattern();
            String dateFormatPatternWithoutLiterals = PatternUtils.getPatternWithoutLiterals(dateFormatPattern);
            if (dateFormatPatternWithoutLiterals.contains(String.valueOf('h')))
                hourCycle = "h12";
            else if (dateFormatPatternWithoutLiterals.contains(String.valueOf('K')))
                hourCycle = "h11";
            else if (dateFormatPatternWithoutLiterals.contains(String.valueOf('H')))
                hourCycle = "h23";
            else // TODO :: Make it more tight.
                hourCycle = "h24";
        } catch (ClassCastException ex) {
            hourCycle = "h24";
        }

        return hourCycle;
    }

    @Override
    public String getDefaultTimeZone(ILocaleObject localeObject) throws JSRangeErrorException {
        return Calendar.getInstance((Locale) localeObject.getLocale()).getTimeZone().getID();
    }

    @Override
    public String getDefaultNumberingSystem(ILocaleObject localeObject) throws JSRangeErrorException {
        return "latn";
    }

    public void configure (ILocaleObject resolvedLocaleObject, Object calendar, Object numberingSystem, FormatMatcher formatMatcher
            , WeekDay weekDay, Era era
            , Year year, Month month, Day day
            , Hour hour, Minute minute, Second second
            , TimeZoneName timeZoneName, Object hourCycle, Object timeZone) throws JSRangeErrorException {
        mLocale = resolvedLocaleObject;
        if (!JSObjects.isUndefined(calendar) && !JSObjects.isNull(calendar)) {
            ArrayList<String> calendarList = new ArrayList<>();
            calendarList.add(JSObjects.getJavaString(calendar));

            resolvedLocaleObject.setUnicodeExtensions("ca", calendarList);
        }

        if (!JSObjects.isUndefined(numberingSystem) && !JSObjects.isNull(numberingSystem)) {

            ArrayList<String> numberingSystemList = new ArrayList<>();
            numberingSystemList.add(JSObjects.getJavaString(numberingSystem));

            resolvedLocaleObject.setUnicodeExtensions("nu", numberingSystemList);
        }

        boolean needDate = year != null ||  month != null || day != null;
        boolean needTime = hour != null ||  minute != null || second != null;

        if(needDate && needTime)
            mDateFormat = DateFormat.getDateTimeInstance(DateFormat.FULL, DateFormat.FULL, (Locale)resolvedLocaleObject.getLocale());
        else if(needDate)
            mDateFormat = DateFormat.getDateInstance(DateFormat.FULL, (Locale)resolvedLocaleObject.getLocale());
        else if(needTime)
            mDateFormat = DateFormat.getTimeInstance(DateFormat.FULL, (Locale)resolvedLocaleObject.getLocale());

        if (!JSObjects.isUndefined(timeZone) && !JSObjects.isNull(timeZone)) {
            TimeZone timeZoneObject = TimeZone.getTimeZone(JSObjects.getJavaString(timeZone));
            mDateFormat.setTimeZone(timeZoneObject);
        }
    }

    @Override
    public boolean isValidTimeZone(String timeZone) {
        return TimeZone.getTimeZone(timeZone).getID().equals(timeZone);
    }

    @Override
    public String[] getAvailableLocales() {

        if(Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
            // Before L, Locale.toLanguageTag isn't available. Need to figure out how to get a locale id from locale object ... Currently resoring to support only en
            return new String []{"en"};
        }

        ArrayList<String> availableLocaleIds = new ArrayList<>();
        java.util.Locale[] availableLocales = java.text.DateFormat.getAvailableLocales();
        for(java.util.Locale locale: availableLocales) {
            availableLocaleIds.add(locale.toLanguageTag()); // TODO:: Not available on platforms <= 20
        }

        return availableLocaleIds.toArray(new String[availableLocaleIds.size()]);
    }

    PlatformDateTimeFormatterAndroid() { }
}
