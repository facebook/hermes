/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import android.content.res.AssetManager;
import android.icu.util.ULocale;
import android.test.InstrumentationTestCase;
import android.text.TextUtils;
import android.util.Log;

import static org.fest.assertions.api.Assertions.assertThat;

import com.facebook.hermes.test.JSRuntime;

import org.junit.Test;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

public class HermesIntlDateFormatTest extends HermesIntlTest262Base {

    private static final String LOG_TAG = "HermesIntDaterFormatTest";

    public void testIntlDateTimeFormat() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/";

        Set<String> whilteList = new HashSet<>(
        );
        Set<String> blackList = new HashSet<>(Arrays.asList(
                "subclassing.js", //Compiling JS failed: 18:1:invalid statement encountered. Buffer size 1037 starts with: 2f2f20436f7079726967687420323031
                "constructor-options-throwing-getters-dayPeriod.js", //Exception from dayPeriod getter should be propagated Expected a CustomError to be thrown but no exception was thrown at all
                "constructor-options-fractionalSecondDigits-valid.js", //Expected SameValue(«undefined», «1») to be true
                "constructor-options-fractionalSecondDigits-invalid.js", //new Intl.DateTimeFormat("en", { fractionalSecondDigits: "LONG" }) throws RangeError Expected a RangeError to be thrown but no exception was thrown at all
                "taint-Object-prototype-date-time-components.js", //Client code can adversely affect behavior: setter for day.
                "constructor-options-dayPeriod-valid.js", //Expected SameValue(«undefined», «long») to be true
                "constructor-options-order-fractionalSecondDigits.js", //Expected [localeMatcher, formatMatcher, second, timeZoneName] and [second, fractionalSecondDigits, localeMatcher, second, timeZoneName, fractionalSecondDigits, formatMatcher] to have the same contents.
                "constructor-options-throwing-getters-fractionalSecondDigits.js", //Exception from fractionalSecondDigits getter should be propagated Expected a CustomError to be thrown but no exception was thrown at all
                "constructor-options-order.js", //Expected [localeMatcher, hour12, hourCycle, timeZone, formatMatcher, weekday, era, year, month, day, hour, minute, second, timeZoneName] and [weekday, year, month, day, hour, minute, second, localeMatcher, hour12, hourCycle, timeZone, weekday, era, year, month, day, hour, minute, second, timeZoneName, formatMatcher] to have the same contents.
                "proto-from-ctor-realm.js", //Property '$262' doesn't exist ReferenceError: Property '$262' doesn't exist at global (:32:13)
                "constructor-options-order-timedate-style.js", //Expected [dateStyle, timeStyle, localeMatcher, hour12, hourCycle, timeZone, formatMatcher, weekday, era, year, month, day, hour, minute, second, timeZoneName] and [weekday, year, month, day, hour, minute, second, dateStyle, timeStyle, localeMatcher, hour12, hourCycle, timeZone, weekday, era, year, month, day, hour, minute, second, timeZoneName, formatMatcher, dateStyle, timeStyle] to have the same contents.
                "timezone-canonicalized.js", //Time zone name PRC was accepted, but incorrectly canonicalized. Expected SameValue(«China Standard Time», «Asia/Shanghai») to be true
                "constructor-options-dayPeriod-invalid.js", //new Intl.DateTimeFormat("en", { dayPeriod: "" }) throws RangeError Expected a RangeError to be thrown but no exception was thrown at all
                "constructor-options-order-dayPeriod.js", //Expected [day, hour] and [day, dayPeriod, hour, day, dayPeriod, hour] to have the same contents.
                "timezone-utc.js", //Time zone name UTC was not correctly accepted. Expected SameValue(«GMT», «UTC») to be true
                "constructor-options-dateStyle-invalid.js", //new Intl.DateTimeFormat("en", { dateStyle: "" }) throws RangeError Expected a RangeError to be thrown but no exception was thrown at all
                "constructor-options-dateStyle-valid.js", //Expected SameValue(«undefined», «full») to be true
                "constructor-options-timeStyle-invalid.js", //new Intl.DateTimeFormat("en", { timeStyle: "" }) throws RangeError Expected a RangeError to be thrown but no exception was thrown at all
                "constructor-options-timeStyle-valid.js", //Expected SameValue(«undefined», «full») to be true
                "constructor-options-numberingSystem-invalid.js" //new Intl.DateTimeFormat("en", {numberingSystem: "latné"}) throws RangeError Expected a RangeError to be thrown but no exception was thrown at all
                ));

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlDateTimeFormat_supportedLocalesOf() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/supportedLocalesOf/";

        Set<String> whilteList = new HashSet<>(
        );

        Set<String> blackList = new HashSet<>(Arrays.asList("builtin.js" // : Property 'isConstructor' doesn't exist
        ));

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlDateTimeFormat_prototype_constructor() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/constructor/";

        Set<String> whilteList = new HashSet<>(
        );

        Set<String> blackList = new HashSet<>(Arrays.asList(""
        ));

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlDateTimeFormat_prototype_format() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/format/";

        Set<String> whilteList = new HashSet<>(
        );

        Set<String> blackList = new HashSet<>(Arrays.asList("time-clip-near-time-boundaries.js", //Expected a RangeError but got a ReferenceError
                "timedatestyle-en.js", //Result for full with {} Expected SameValue(«2:12:47 PM Coordinated Universal Time», «14:12:47 Coordinated Universal Time») to be true
                "format-function-length.js", //Expected SameValue(«0», «1») to be true
                "dayPeriod-narrow-en.js", //00:00, narrow format Expected SameValue(«12/12/2017», «at night») to be true
                "dayPeriod-short-en.js", //00:00, short format Expected SameValue(«12/12/2017», «at night») to be true
                "dayPeriod-long-en.js", //00:00, long format Expected SameValue(«12/12/2017», «at night») to be true
                "proleptic-gregorian-calendar.js", //Internal error: Didn't find Gregorian calendar Expected SameValue(«undefined», «gregory») to be true
                "builtin.js", //Property 'isConstructor' doesn't exist ReferenceError: Property 'isConstructor' doesn't exist         at global (:30:18)
                "format-function-builtin.js", //Property 'isConstructor' doesn't exist ReferenceError: Property 'isConstructor' doesn't exist at global (:30:18)
                "fractionalSecondDigits.js", //1 fractionalSecondDigits round down Expected SameValue(«02:03», «02:03.2») to be true
                "related-year-zh.js", //Expected SameValue(«2019年», «2019己亥年») to be true
                "bound-to-datetimeformat-instance.js" // com.facebook.hermes.intl.JSRangeErrorException: Ill-formed Unicode locale keyword type: Thai Digits [at index 0]
        ));

        runTests(basePath, blackList, whilteList);
    }


    public void testIntlDateTimeFormat_prototype_formatToParts() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/formatToParts/";

        Set<String> whilteList = new HashSet<>(
        );

        Set<String> blackList = new HashSet<>(Arrays.asList("fractionalSecondDigits.js", //length should be 5, 1 fractionalSecondDigits round down Expected SameValue(«3», «5») to be true
                "dayPeriod-narrow-en.js", //length should be 1, 00:00, narrow format Expected SameValue(«5», «1») to be true
                "related-year-zh.js", //undefined: length Expected SameValue(«2», «3») to be true
                "time-clip-near-time-boundaries.js", //Expected a RangeError but got a ReferenceError
                "dayPeriod-long-en.js", //length should be 1, 00:00, long format Expected SameValue(«5», «1») to be true
                "length.js", //descriptor value should be 1
                "pattern-on-calendar.js", //Expected SameValue(«false», «true») to be true
                "related-year.js", //Expected SameValue(«false», «true») to be true
                "dayPeriod-short-en.js" //length should be 1, 00:00, short format Expected SameValue(«5», «1») to be true
        ));

        runTests(basePath, blackList, whilteList);
    }


    public void testIntlDateTimeFormat_prototype_resolvedOptions() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/resolvedOptions/";

        Set<String> whilteList = new HashSet<>(
        );

        Set<String> blackList = new HashSet<>(Arrays.asList("order-fractionalSecondDigits.js", //Property 'arrayContains' doesn't exist ReferenceError: Property 'arrayContains' doesn't exist at global (:31:8)
                "order.js", //Property 'arrayContains' doesn't exist ReferenceError: Property 'arrayContains' doesn't exist at global (:45:8)
                "hourCycle-dateStyle.js", //Should support dateStyle=full Expected SameValue(«undefined», «full») to be true
                "order-style.js", //Property 'arrayContains' doesn't exist ReferenceError: Property 'arrayContains' doesn't exist at global (:37:8)
                "hourCycle-timeStyle.js", //Should support timeStyle=full Expected SameValue(«undefined», «full») to be true
                "builtin.js", //Property 'isConstructor' doesn't exist ReferenceError: Property 'isConstructor' doesn't exist at global (:27:18)
                "order-dayPeriod.js" //Property 'arrayContains' doesn't exist ReferenceError: Property 'arrayContains' doesn't exist at global (:33:8)
        ));

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlDateTimeFormat_prototype_toStringTag() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/toStringTag/";

        Set<String> whilteList = new HashSet<>(
        );

        Set<String> blackList = new HashSet<>(Arrays.asList(
                "toString.js", //Expected SameValue(«[object Object]», «[object Intl.DateTimeFormat]») to be true
                "toStringTag.js" //descriptor value should be Intl.DateTimeFormat
        ));

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlDateTimeFormat_prototype() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/";

        Set<String> whilteList = new HashSet<>(
        );

        Set<String> blackList = new HashSet<>(Arrays.asList(""
        ));

        runTests(basePath, blackList, whilteList);
    }
}