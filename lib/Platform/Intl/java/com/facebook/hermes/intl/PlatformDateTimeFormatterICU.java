package com.facebook.hermes.intl;

import android.icu.text.DateFormat;
import android.icu.text.NumberingSystem;
import android.icu.text.SimpleDateFormat;
import android.icu.util.Calendar;
import android.icu.util.TimeZone;
import android.icu.util.ULocale;

import java.text.AttributedCharacterIterator;
import java.util.ArrayList;
import java.util.Date;

import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.Day.*;
import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.Era.*;
import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.Hour.*;
import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.Minute.*;
import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.Month.*;
import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.Second.*;
import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.TimeZoneName.*;
import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.WeekDay.*;
import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.WeekDay.LONG;
import static com.facebook.hermes.intl.IPlatformDateTimeFormatter.Year.*;

public class PlatformDateTimeFormatterICU implements IPlatformDateTimeFormatter{
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
        return mDateFormat.getCalendar().getType();
    }

    @Override
    public String getTimeZoneName() throws JSRangeErrorException {
        return mDateFormat.getTimeZone().getDisplayName(false, TimeZone.SHORT, (ULocale) mLocale.getLocale());
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
            String dateFormatPattern = ((SimpleDateFormat) DateFormat.getTimeInstance(DateFormat.FULL, (ULocale) localeObject.getLocale())).toPattern();
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
        return Calendar.getInstance((ULocale) localeObject.getLocale()).getTimeZone().getID();
    }

    @Override
    public String getDefaultNumberingSystem(ILocaleObject localeObject) throws JSRangeErrorException {
        return NumberingSystem.getInstance((ULocale) localeObject.getLocale()).getName();
    }

    private static String getSkeleton(FormatMatcher mFormatMatcher
            , WeekDay mWeekDay, Era mEra
            , Year mYear, Month mMonth, Day mDay
            , Hour mHour, Minute mMinute, Second mSecond
            , TimeZoneName mTimeZoneName, boolean hour12) {

        StringBuffer skeletonBuffer = new StringBuffer();

        if (mWeekDay != null) {
            switch (mWeekDay) {
                case LONG:
                    skeletonBuffer.append("EEEE");
                    break;
                case SHORT:
                    skeletonBuffer.append("EEE");
                    break;
                case NARROW:
                    skeletonBuffer.append("EEEEE");
                    break;
            }
        }

        if (mEra != null) {
            switch (mEra) {
                case LONG:
                    skeletonBuffer.append("GGGG");
                    break;
                case SHORT:
                    skeletonBuffer.append("GGG");
                    break;
                case NARROW:
                    skeletonBuffer.append("G5");
                    break;
            }
        }

        if (mYear != null) {
            switch (mYear) {
                case NUMERIC:
                    skeletonBuffer.append("yyyy");
                    break;
                case DIGIT2:
                    skeletonBuffer.append("yy");
                    break;
            }
        }

        if (mMonth != null) {
            switch (mMonth) {
                case NUMERIC:
                    skeletonBuffer.append("M");
                    break;
                case DIGIT2:
                    skeletonBuffer.append("MM");
                    break;
                case LONG:
                    skeletonBuffer.append("MMMM");
                    break;
                case SHORT:
                    skeletonBuffer.append("MMM");
                    break;
                case NARROW:
                    skeletonBuffer.append("MMMMM");
                    break;
            }
        }

        if (mDay != null) {
            switch (mDay) {
                case NUMERIC:
                    skeletonBuffer.append("d");
                    break;
                case DIGIT2:
                    skeletonBuffer.append("dd");
                    break;
            }
        }

        if (mHour != null) {
            switch (mHour) {
                case NUMERIC:
                    if (hour12)
                        skeletonBuffer.append("h");
                    else
                        skeletonBuffer.append("k");
                    break;
                case DIGIT2:
                    if (hour12)
                        skeletonBuffer.append("hh");
                    else
                        skeletonBuffer.append("kk");
                    break;
            }
        }

        if (mMinute != null) {
            switch (mMinute) {
                case NUMERIC:
                    skeletonBuffer.append("m");
                    break;
                case DIGIT2:
                    skeletonBuffer.append("mm");
                    break;
            }
        }

        if (mSecond != null) {
            switch (mSecond) {
                case NUMERIC:
                    skeletonBuffer.append("s");
                    break;
                case DIGIT2:
                    skeletonBuffer.append("ss");
                    break;
            }
        }

        if (mTimeZoneName != null) {
            switch (mTimeZoneName) {
                case LONG:
                    skeletonBuffer.append("VV");
                    break;
                case SHORT:
                    skeletonBuffer.append("O");
                    break;
            }
        }

        return skeletonBuffer.toString();
    }

    public void configure (ILocaleObject resolvedLocaleObject, Object calendar, Object numberingSystem
            , FormatMatcher mFormatMatcher
            , WeekDay mWeekDay, Era mEra
            , Year mYear, Month mMonth, Day mDay
            , Hour mHour, Minute mMinute, Second mSecond
            , TimeZoneName mTimeZoneName, Object hourCycle, Object timeZone) throws JSRangeErrorException {
        mLocale = resolvedLocaleObject;
        String skeleton = getSkeleton(mFormatMatcher, mWeekDay, mEra, mYear, mMonth, mDay, mHour, mMinute, mSecond, mTimeZoneName, hourCycle.equals("h11") || hourCycle.equals("h12"));

        Calendar calendarInstance = null;
        if (!JSObjects.isUndefined(calendar) && !JSObjects.isNull(calendar)) {
            ArrayList<String> calendarList = new ArrayList<>();
            calendarList.add(JSObjects.getJavaString(calendar));

            ILocaleObject modifiedLocaleObject = resolvedLocaleObject.cloneObject();
            modifiedLocaleObject.setUnicodeExtensions("ca", calendarList);

            calendarInstance = Calendar.getInstance((ULocale) modifiedLocaleObject.getLocale());
        }

        if (!JSObjects.isUndefined(numberingSystem) && !JSObjects.isNull(numberingSystem)) {
            NumberingSystem numberingSystemObject;
            try {
                numberingSystemObject = NumberingSystem.getInstanceByName(JSObjects.getJavaString(numberingSystem));
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

        DateFormat dateFormat;
        if(calendarInstance != null)
            mDateFormat = DateFormat.getPatternInstance(calendarInstance, skeleton, (ULocale) resolvedLocaleObject.getLocale());
        else
            mDateFormat = DateFormat.getPatternInstance(skeleton, (ULocale) resolvedLocaleObject.getLocale());

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
        ArrayList<String> availableLocaleIds = new ArrayList<>();
        java.util.Locale[] availableLocales = android.icu.text.DateFormat.getAvailableLocales();
        for(java.util.Locale locale: availableLocales) {
            availableLocaleIds.add(locale.toLanguageTag()); // TODO:: Not available on platforms <= 20
        }

        return availableLocaleIds.toArray(new String[availableLocaleIds.size()]);
    }


    PlatformDateTimeFormatterICU() { }
}
