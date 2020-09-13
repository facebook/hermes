package com.facebook.hermes.intl;

import java.text.AttributedCharacterIterator;

public interface IPlatformDateTimeFormatter
{
    enum FormatMatcher {
        BESTFIT,
        BASIC;

        @Override
        public String toString() {
            switch (this) {
                case BESTFIT:
                    return "best fit";
                case BASIC:
                    return "basic";
                default:
                    throw new IllegalArgumentException();
            }
        }

    }

    enum WeekDay {
        LONG,
        SHORT,
        NARROW;

        @Override
        public String toString() {
            switch (this) {
                case LONG:
                    return "long";
                case SHORT:
                    return "short";
                case NARROW:
                    return "narrow";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    enum Era {
        LONG,
        SHORT,
        NARROW;

        @Override
        public String toString() {
            switch (this) {
                case LONG:
                    return "long";
                case SHORT:
                    return "short";
                case NARROW:
                    return "narrow";
                default:
                    throw new IllegalArgumentException();
            }
        }

    }

    enum Year {
        NUMERIC,
        DIGIT2;

        @Override
        public String toString() {
            switch (this) {
                case NUMERIC:
                    return "numeric";
                case DIGIT2:
                    return "2-digit";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    enum Month {
        NUMERIC,
        DIGIT2,
        LONG,
        SHORT,
        NARROW;

        @Override
        public String toString() {
            switch (this) {
                case NUMERIC:
                    return "numeric";
                case DIGIT2:
                    return "2-digit";
                case LONG:
                    return "long";
                case SHORT:
                    return "short";
                case NARROW:
                    return "narrow";

                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    enum Day {
        NUMERIC,
        DIGIT2;

        @Override
        public String toString() {
            switch (this) {
                case NUMERIC:
                    return "numeric";
                case DIGIT2:
                    return "2-digit";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    enum Hour {
        NUMERIC,
        DIGIT2;

        @Override
        public String toString() {
            switch (this) {
                case NUMERIC:
                    return "numeric";
                case DIGIT2:
                    return "2-digit";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    enum Minute {
        NUMERIC,
        DIGIT2;

        @Override
        public String toString() {
            switch (this) {
                case NUMERIC:
                    return "numeric";
                case DIGIT2:
                    return "2-digit";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    enum Second {
        NUMERIC,
        DIGIT2;

        @Override
        public String toString() {
            switch (this) {
                case NUMERIC:
                    return "numeric";
                case DIGIT2:
                    return "2-digit";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    enum TimeZoneName {
        LONG,
        SHORT;

        @Override
        public String toString() {
            switch (this) {
                case LONG:
                    return "long";
                case SHORT:
                    return "short";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    void configure(ILocaleObject mResolvedLocaleObject, Object mCalendar, Object mNumberingSystem
            , FormatMatcher mFormatMatcher
            , WeekDay mWeekDay, Era mEra
            , Year mYear, Month mMonth, Day mDay
            , Hour mHour, Minute mMinute, Second mSecond
            , TimeZoneName mTimeZoneName
            , Object hourCycle, Object timeZone) throws JSRangeErrorException;

    String format(double n) throws JSRangeErrorException;
    String fieldToString(AttributedCharacterIterator.Attribute attribute);
    AttributedCharacterIterator formatToParts(double n) throws JSRangeErrorException;

    String getCalendarName();
    String getTimeZoneName() throws JSRangeErrorException;

    String getDefaultHourCycle(ILocaleObject localeObject) throws JSRangeErrorException;
    String getDefaultTimeZone(ILocaleObject localeObject) throws JSRangeErrorException;
    String getDefaultNumberingSystem(ILocaleObject localeObject) throws JSRangeErrorException;

    boolean isValidTimeZone(String timeZone);

    String[] getAvailableLocales();
}
