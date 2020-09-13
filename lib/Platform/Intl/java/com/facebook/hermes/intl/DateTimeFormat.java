/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.os.Build;

import java.text.AttributedCharacterIterator;
import java.text.CharacterIterator;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * This class represents the Java part of the Android
 * Intl.DateTimeFormat implementation.  The interaction with the
 * Hermes JaveScript internals are implemented in C++ and should not
 * generally need to be changed.  Implementers' notes here will
 * describe what parts of the ECMA 402 spec remain to be implemented.
 * <p>
 * Implementer notes:
 * <p>
 * Internal slots: In the ECMA 402 spec, there are a number of
 * references to internal slots.  These are gneerally expressed in
 * terms of JavaScript objects, but the semantics do not generally
 * depend on this.  For example, where the spec says
 * "Intl.DateTimeFormat instances have an
 * [[InitializedDateTimeFormat]] internal slot", this would not be a
 * literal artifact of the implementation.  Internal slots, where
 * necessary, should be represented as members of this java
 * DateTimeFormat object.
 * <p>
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


    IPlatformDateTimeFormatter mPlatformDateTimeFormatter;

    private ILocaleObject mResolvedLocaleObject = null;

    // This is a hacky way to avoid the extensions that we add from being shown in "resolvedOptions" ..
    private ILocaleObject mResolvedLocaleObjectForResolvedOptions = null;

    private Object mCalendar = JSObjects.Undefined();
    private Object mNumberingSystem = JSObjects.Undefined();
    private Object mHourCycle = JSObjects.Undefined();

    private IPlatformDateTimeFormatter.FormatMatcher mFormatMatcher = null;
    private IPlatformDateTimeFormatter.WeekDay mWeekDay = null;
    private IPlatformDateTimeFormatter.Era mEra = null;
    private IPlatformDateTimeFormatter.Year mYear = null;
    private IPlatformDateTimeFormatter.Month mMonth = null;
    private IPlatformDateTimeFormatter.Day mDay = null;
    private IPlatformDateTimeFormatter.Hour mHour = null;
    private IPlatformDateTimeFormatter.Minute mMinute = null;
    private IPlatformDateTimeFormatter.Second mSecond = null;
    private IPlatformDateTimeFormatter.TimeZoneName mTimeZoneName = null;

    private Object mTimeZone = null;

    private boolean isLocaleIdType(String token) {
        return IntlTextUtils.isUnicodeExtensionKeyTypeItem(token, 0, token.length() - 1);
    }

    // https://tc39.es/ecma402/#sec-todatetimeoptions
    private Object ToDateTimeOptions(Object options, String required, String defaults) throws JSRangeErrorException {
        if(!JSObjects.isObject(options)) {
            throw new JSRangeErrorException("Invalid options object !");
        }
        boolean needDefaults = true;

        if(required.equals("date") || required.equals("any")) {
            for (String property : new String[]{"weekday", "year", "month", "day"}) {
                if(!JSObjects.isUndefined(JSObjects.Get(options, property)))
                    needDefaults = false;
            }
        }

        if(required.equals("time") || required.equals("any")) {
            for (String property : new String[]{"hour", "minute", "second"}) {
                if(!JSObjects.isUndefined(JSObjects.Get(options, property)))
                    needDefaults = false;
            }
        }

        if(needDefaults && (defaults.equals("date") || defaults.equals("all"))) {
            for (String property : new String[]{"year", "month", "day"}) {
                JSObjects.Put(options, property, "numeric");
            }
        }

        if(needDefaults && (defaults.equals("time") || defaults.equals("all"))) {
            for (String property : new String[]{"hour", "minute", "second"}) {
                JSObjects.Put(options, property, "numeric");
            }
        }

        return options;
    }

    private Object DefaultTimeZone() throws JSRangeErrorException {
        return mPlatformDateTimeFormatter.getDefaultTimeZone(mResolvedLocaleObject);
    }

    // https://tc39.es/ecma402/#sec-initializedatetimeformat
    private void initializeDateTimeFormat(List<String> locales, Map<String, Object> inOptions) throws JSRangeErrorException {

        List<String> relevantExtensionKeys = Arrays.asList(new String[]{"ca", "nu", "hc"});

        // 2
        Object options = ToDateTimeOptions(inOptions, "any", "date");

        //3
        Object opt = JSObjects.newObject();

        // 4,5,
        Object matcher = OptionHelpers.GetOption(options, Constants.LOCALEMATCHER, OptionHelpers.OptionType.STRING, Constants.LOCALEMATCHER_POSSIBLE_VALUES, Constants.LOCALEMATCHER_BESTFIT);
        JSObjects.Put(opt, "localeMatcher", matcher);

        // 6 - 8
        Object calendar = OptionHelpers.GetOption(options,"calendar", OptionHelpers.OptionType.STRING, JSObjects.Undefined(), JSObjects.Undefined());
        if (!JSObjects.isUndefined(calendar)) {
            if (!isLocaleIdType(JSObjects.getJavaString(calendar)))
                throw new JSRangeErrorException("Invalid calendar option !");
        }
        JSObjects.Put(opt, "ca", calendar);

        // 9 - 11
        Object numberingSystem = OptionHelpers.GetOption(options, "numberingSystem", OptionHelpers.OptionType.STRING, JSObjects.Undefined(), JSObjects.Undefined());
        if (!JSObjects.isUndefined(numberingSystem)) {
            if (!isLocaleIdType(JSObjects.getJavaString(numberingSystem)))
                throw new JSRangeErrorException("Invalid numbering system !");
        }
        JSObjects.Put(opt, "nu", numberingSystem);

        // 12 - 15
        Object hour12 = OptionHelpers.GetOption(options, "hour12", OptionHelpers.OptionType.BOOLEAN, JSObjects.Undefined(), JSObjects.Undefined());
        Object hourCycle = OptionHelpers.GetOption(options, "hourCycle", OptionHelpers.OptionType.STRING, new String[]{"h11", "h12", "h23", "h24"}, JSObjects.Undefined());

        if(!JSObjects.isUndefined(hour12))
            hourCycle = JSObjects.Null();

        JSObjects.Put(opt, "hc", hourCycle);

        // 16 - 23
        HashMap<String, Object> r = LocaleResolver.resolveLocale(mPlatformDateTimeFormatter.getAvailableLocales(), locales, opt, relevantExtensionKeys);

        mResolvedLocaleObject = (ILocaleObject) JSObjects.getJavaMap(r).get("locale");
        mResolvedLocaleObjectForResolvedOptions = mResolvedLocaleObject.cloneObject();

        mCalendar = JSObjects.Get(r, "ca");
        mNumberingSystem = JSObjects.Get(r, "nu");
        mHourCycle = JSObjects.Get(r, "hc");

        // 24 - 27
        Object timeZone = JSObjects.Get(options, "timeZone");
        if(JSObjects.isUndefined(timeZone)) {
            timeZone = DefaultTimeZone();
        } else {
            String normalizedTimeZone = normalizeTimeZoneName(JSObjects.getJavaString(timeZone));
            if (!isValidTimeZoneName(normalizedTimeZone)) {
                throw new JSRangeErrorException("Invalid timezone name!");
            }
        }
        mTimeZone = timeZone;

        // 28 - 34
        Object formatMatcher = OptionHelpers.GetOption(options, "formatMatcher", OptionHelpers.OptionType.STRING,  new String[]{"basic", "best fit"}, "best fit");
        mFormatMatcher = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.FormatMatcher.class, JSObjects.getJavaString(formatMatcher));

        // 29, 35
        Object weekDay = OptionHelpers.GetOption(options, "weekday", OptionHelpers.OptionType.STRING,  new String[]{"long", "short", "narrow"}, JSObjects.Undefined());
        if(!JSObjects.isUndefined(weekDay)) {
            mWeekDay = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.WeekDay.class, JSObjects.getJavaString(weekDay));
        }

        Object era = OptionHelpers.GetOption(options, "era", OptionHelpers.OptionType.STRING,  new String[]{"long", "short", "narrow"}, JSObjects.Undefined());
        if(!JSObjects.isUndefined(era)) {
            mEra = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Era.class, JSObjects.getJavaString(era));
        }

        Object year = OptionHelpers.GetOption(options, "year", OptionHelpers.OptionType.STRING,  new String[]{"numeric", "2-digit"}, JSObjects.Undefined());
        if(!JSObjects.isUndefined(year)) {
            mYear = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Year.class, JSObjects.getJavaString(year));
        }

        Object month = OptionHelpers.GetOption(options, "month", OptionHelpers.OptionType.STRING,  new String[]{"numeric", "2-digit", "long", "short", "narrow"}, JSObjects.Undefined());
        if(!JSObjects.isUndefined(month)) {
            mMonth = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Month.class, JSObjects.getJavaString(month));
        }

        Object day = OptionHelpers.GetOption(options, "day", OptionHelpers.OptionType.STRING,  new String[]{"numeric", "2-digit"}, JSObjects.Undefined());
        if(!JSObjects.isUndefined(day)) {
            mDay = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Day.class, JSObjects.getJavaString(day));
        }

        Object hour = OptionHelpers.GetOption(options, "hour", OptionHelpers.OptionType.STRING,  new String[]{"numeric", "2-digit"}, JSObjects.Undefined());
        if(!JSObjects.isUndefined(hour)) {
            mHour = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Hour.class, JSObjects.getJavaString(hour));
        }

        Object minute = OptionHelpers.GetOption(options, "minute", OptionHelpers.OptionType.STRING,  new String[]{"numeric", "2-digit"}, JSObjects.Undefined());
        if(!JSObjects.isUndefined(minute)) {
            mMinute = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Minute.class, JSObjects.getJavaString(minute));
        }

        Object second = OptionHelpers.GetOption(options, "second", OptionHelpers.OptionType.STRING,  new String[]{"numeric", "2-digit"}, JSObjects.Undefined());
        if(!JSObjects.isUndefined(second)) {
            mSecond = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.Second.class, JSObjects.getJavaString(second));
        }

        Object timeZoneName = OptionHelpers.GetOption(options, "timeZoneName", OptionHelpers.OptionType.STRING,  new String[]{"long", "short"}, JSObjects.Undefined());
        if(!JSObjects.isUndefined(timeZoneName)) {
            mTimeZoneName = OptionHelpers.searchEnum(IPlatformDateTimeFormatter.TimeZoneName.class, JSObjects.getJavaString(timeZoneName));
        }

        // 36
        if (JSObjects.isUndefined(hour)) {
            mHourCycle = JSObjects.Undefined();
        } else {
            Object hcDefault = mPlatformDateTimeFormatter.getDefaultHourCycle(mResolvedLocaleObject);
            Object hc = mHourCycle;

            if (JSObjects.isNull(hc)) {
                hc = hcDefault;
            }

            if(!JSObjects.isUndefined(hour12)) {
                if(JSObjects.getJavaBoolean(hour12)) { // true
                    if(JSObjects.getJavaString(hcDefault).equals("h11") || JSObjects.getJavaString(hcDefault).equals("h23"))
                        hc = JSObjects.newString("h11");
                    else
                        hc = JSObjects.newString("h12");
                } else {
                    if(JSObjects.getJavaString(hcDefault).equals("h11") || JSObjects.getJavaString(hcDefault).equals("h23"))
                        hc = JSObjects.newString("h23");
                    else
                        hc = JSObjects.newString("h24");
                }
            }

            mHourCycle = hc;
        }
    }

    private String normalizeTimeZoneName(String timeZoneName) {
        // https://tc39.es/ecma402/#sec-case-sensitivity-and-case-mapping
        // Note that we should convert only upper case translation in ASCII range.
        StringBuffer normalized = new StringBuffer(timeZoneName.length());
        int offset = 'a' - 'A';
        for (int idx = 0; idx < timeZoneName.length(); idx++) {
            char c = timeZoneName.charAt(idx);
            if (c >= 'a' && c <= 'z') {
                normalized.append((char) (c - offset));
            } else {
                normalized.append(c);
            }
        }

        return normalized.toString();
    }


    private boolean isValidTimeZoneName(String timeZone) {
        return mPlatformDateTimeFormatter.isValidTimeZone(timeZone);
    }

    public DateTimeFormat(List<String> locales, Map<String, Object> options)
            throws JSRangeErrorException {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N)
            mPlatformDateTimeFormatter = new PlatformDateTimeFormatterICU();
        else
            mPlatformDateTimeFormatter = new PlatformDateTimeFormatterAndroid();

        initializeDateTimeFormat(locales, options);

        mPlatformDateTimeFormatter.configure(mResolvedLocaleObject, mCalendar, mNumberingSystem, mFormatMatcher, mWeekDay,
                mEra, mYear, mMonth, mDay, mHour, mMinute, mSecond, mTimeZoneName, mHourCycle, mTimeZone);

    }

    // options are localeMatcher:string
    //
    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sec-intl.datetimeformat.supportedlocalesof.
    //
    // The notes on the ctor for Locales and Options also apply here.
    public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options) throws JSRangeErrorException {
        String[] availableLocales;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N)
            availableLocales = new PlatformDateTimeFormatterICU().getAvailableLocales();
        else
            availableLocales = new PlatformDateTimeFormatterAndroid().getAvailableLocales();

        return Arrays.asList(LocaleMatcher.lookupSupportedLocales(availableLocales, locales.toArray(new String[locales.size()])));
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
    public Map<String, Object> resolvedOptions() throws JSRangeErrorException {
        HashMap<String, Object> finalResolvedOptions = new HashMap<String, Object>();
        finalResolvedOptions.put(Constants.LOCALE, mResolvedLocaleObjectForResolvedOptions.toCanonicalTag());

        if (!JSObjects.isUndefined(mNumberingSystem) && !JSObjects.isNull(mNumberingSystem)) {
            finalResolvedOptions.put("numberingSystem", mNumberingSystem);
        } else {
            String effectiveNumberingSystem = mPlatformDateTimeFormatter.getDefaultNumberingSystem(mResolvedLocaleObject);
            finalResolvedOptions.put("numberingSystem", effectiveNumberingSystem);
        }

        String effectiveCalendar;
        if (!JSObjects.isUndefined(mCalendar) && !JSObjects.isNull(mCalendar)) {
            effectiveCalendar = JSObjects.getJavaString(mCalendar);
        } else {
            effectiveCalendar = mPlatformDateTimeFormatter.getCalendarName();
        }

        effectiveCalendar = UnicodeLocaleKeywordUtils.resolveCalendarAlias(effectiveCalendar);

        finalResolvedOptions.put("calendar", effectiveCalendar);

        String effectiveTimeZone = mPlatformDateTimeFormatter.getTimeZoneName();
        finalResolvedOptions.put("timeZone", effectiveTimeZone);

        if (!JSObjects.isUndefined(mHourCycle)) {
            finalResolvedOptions.put("hourCycle", JSObjects.getJavaString(mHourCycle));

            if (JSObjects.getJavaString(mHourCycle).equals("h11") || JSObjects.getJavaString(mHourCycle).equals("h12"))
                finalResolvedOptions.put("hour12", true);
            else
                finalResolvedOptions.put("hour12", false);
        }

        if (mWeekDay != null)
            finalResolvedOptions.put("weekday", mWeekDay.toString());

        if (mEra != null)
            finalResolvedOptions.put("era", mEra.toString());

        if (mYear != null)
            finalResolvedOptions.put("year", mYear.toString());

        if (mMonth != null)
            finalResolvedOptions.put("month", mMonth.toString());

        if (mDay != null)
            finalResolvedOptions.put("day", mDay.toString());

        if (mHour != null)
            finalResolvedOptions.put("hour", mHour.toString());

        if (mMinute != null)
            finalResolvedOptions.put("minute", mMinute.toString());

        if (mSecond != null)
            finalResolvedOptions.put("second", mSecond.toString());

        if (mTimeZoneName != null) {
            finalResolvedOptions.put("timeZoneName", mTimeZoneName.toString());
        }

        return finalResolvedOptions;
    }

    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sec-formatdatetime
    //
    // Steps 1 and 2 of PartitionDateTimePattern are already done.  The
    // formal steps require construction of several internal
    // NumberFormat JavaScript objects, but these objects are never
    // exposed; it should be possible to create and use java
    // NumberFormat objects only.
    public String format(double jsTimeValue) throws JSRangeErrorException {
        return mPlatformDateTimeFormatter.format(jsTimeValue);
    }

    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sec-formatdatetimetoparts
    public List<Map<String, String>> formatToParts(double jsTimeValue) throws JSRangeErrorException {
        ArrayList<Map<String, String>> ret = new ArrayList<Map<String, String>>();
        AttributedCharacterIterator iterator = mPlatformDateTimeFormatter.formatToParts(jsTimeValue);
        StringBuilder sb = new StringBuilder();
        for (char ch = iterator.first(); ch != CharacterIterator.DONE; ch = iterator.next()) {
            sb.append(ch);
            if (iterator.getIndex() + 1 == iterator.getRunLimit()) {
                Iterator<AttributedCharacterIterator.Attribute> keyIterator = iterator.getAttributes().keySet().iterator();
                String key;
                if (keyIterator.hasNext()) {
                    key = mPlatformDateTimeFormatter.fieldToString( keyIterator.next());
                } else {
                    key = "literal";
                }
                String value = sb.toString();
                sb.setLength(0);

                HashMap<String, String> part = new HashMap<>();
                part.put("type", key);
                part.put("value", value);
                ret.add(part);
            }
        }

        return ret;
    }
}