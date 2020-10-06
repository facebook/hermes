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

// Run "./gradlew :intltest:preparetest262" from the root to download and copy the test files to the APK assets.
public class HermesIntlDateFormatTest extends HermesIntlTest262Base {

    private static final String LOG_TAG = "HermesIntDaterFormatTest";

    public void testIntlDateTimeFormat() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat";

        Set<String> whilteList = new HashSet<>();

        Set<String> securityList = new HashSet<>(Arrays.asList(
                "taint-Object-prototype-date-time-components.js" // Client code can adversely affect behavior: setter for day.
        ));

        Set<String> deviationList = new HashSet<>(Arrays.asList(
                "constructor-options-order.js", //Expected [localeMatcher, hour12, hourCycle, timeZone, formatMatcher, weekday, era, year, month, day, hour, minute, second, timeZoneName] and [weekday, year, month, day, hour, minute, second, localeMatcher, hour12, hourCycle, timeZone, weekday, era, year, month, day, hour, minute, second, timeZoneName, formatMatcher] to have the same contents.
                "timezone-canonicalized.js", // Time zone name PRC was accepted, but incorrectly canonicalized. Expected SameValue(«China Standard Time», «Asia/Shanghai») to be true
                "timezone-utc.js" // Time zone name UTC was not correctly accepted. Expected SameValue(«GMT», «UTC») to be true
        ));

        // Our implementation corresponds to ECMA 402 2nd edition (https://www.ecma-international.org/ecma-402/2.0/index.html) which corresonds to ECMA-262 6th Edition (ES6)
        // We dont' support the following,
        //      dateStyle and timeStyle properties. These properties are in draft state AFAIK (https://tc39.es/proposal-intl-datetime-style/)
        //      dayPeriod property was introduced in ECMA 402 4th edition (https://www.ecma-international.org/ecma-402/4.0/index.html)
        //      fractionalSecondDigits property is still in PR ( https://github.com/tc39/ecma402/pull/347 )
        Set<String> unSupportedList = new HashSet<>(Arrays.asList(
                "constructor-options-throwing-getters-timedate-style.js", // : Exception from dateStyle getter should be propagated Expected a CustomError to be thrown but no exception was thrown at all
                "constructor-options-throwing-getters-dayPeriod.js", //Exception from dayPeriod getter should be propagated Expected a CustomError to be thrown but no exception was thrown at all
                "constructor-options-fractionalSecondDigits-valid.js", //Expected SameValue(«undefined», «1») to be true
                "constructor-options-fractionalSecondDigits-invalid.js", //new Intl.DateTimeFormat("en", { fractionalSecondDigits: "LONG" }) throws RangeError Expected a RangeError to be thrown but no exception was thrown at all
                "constructor-options-dayPeriod-valid.js", //Expected SameValue(«undefined», «long») to be true
                "constructor-options-order-fractionalSecondDigits.js", //Expected [localeMatcher, formatMatcher, second, timeZoneName] and [second, fractionalSecondDigits, localeMatcher, second, timeZoneName, fractionalSecondDigits, formatMatcher] to have the same contents.
                "constructor-options-throwing-getters-fractionalSecondDigits.js", //Exception from fractionalSecondDigits getter should be propagated Expected a CustomError to be thrown but no exception was thrown at all
                "constructor-options-order-timedate-style.js", //Expected [dateStyle, timeStyle, localeMatcher, hour12, hourCycle, timeZone, formatMatcher, weekday, era, year, month, day, hour, minute, second, timeZoneName] and [weekday, year, month, day, hour, minute, second, dateStyle, timeStyle, localeMatcher, hour12, hourCycle, timeZone, weekday, era, year, month, day, hour, minute, second, timeZoneName, formatMatcher, dateStyle, timeStyle] to have the same contents.
                "constructor-options-dayPeriod-invalid.js", //new Intl.DateTimeFormat("en", { dayPeriod: "" }) throws RangeError Expected a RangeError to be thrown but no exception was thrown at all
                "constructor-options-order-dayPeriod.js", //Expected [day, hour] and [day, dayPeriod, hour, day, dayPeriod, hour] to have the same contents.
                "constructor-options-dateStyle-invalid.js", //new Intl.DateTimeFormat("en", { dateStyle: "" }) throws RangeError Expected a RangeError to be thrown but no exception was thrown at all
                "constructor-options-dateStyle-valid.js", //Expected SameValue(«undefined», «full») to be true
                "constructor-options-timeStyle-invalid.js", //new Intl.DateTimeFormat("en", { timeStyle: "" }) throws RangeError Expected a RangeError to be thrown but no exception was thrown at all
                "constructor-options-timeStyle-valid.js" //Expected SameValue(«undefined», «full») to be true
        ));

        Set<String> testIssuesList = new HashSet<>(Arrays.asList(
                "subclassing.js", // Hermes doesn't support Javascript classes .. Compiling JS failed: 18:1:invalid statement encountered. Buffer size 1037 starts with: 2f2f20436f7079726967687420323031
                "proto-from-ctor-realm.js" // Hermes doesn' support realms (Isolated environments).
        ));

        Set<String> blackList = new HashSet<>();
        blackList.addAll(unSupportedList);
        blackList.addAll(testIssuesList);
        blackList.addAll(deviationList);
        blackList.addAll(securityList);

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlDateTimeFormat_supportedLocalesOf() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/supportedLocalesOf";

        Set<String> whilteList = new HashSet<>();
        Set<String> testIssueList = new HashSet<>();

        runTests(basePath, testIssueList, whilteList);
    }

    public void testIntlDateTimeFormat_prototype_constructor() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/constructor";

        Set<String> whilteList = new HashSet<>();
        Set<String> blackList = new HashSet<>();

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlDateTimeFormat_prototype_format() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/format";

        Set<String> whiteList = new HashSet<>();

        Set<String> deviations = new HashSet<>(Arrays.asList(
                "proleptic-gregorian-calendar.js" // Strangely icu4j returned date for -8640000000000000 (beginning of ECMAScript time) is November 271817 .. The expected is April 271822 .. Need to dig in.
        ));

        // timeStyle, dateStyle, dayPeriod, fractionalSecondDigits properties are not supported. Please find details above.
        Set<String> unSupportedList = new HashSet<>(Arrays.asList(
                "timedatestyle-en.js", //Result for full with {} Expected SameValue(«2:12:47 PM Coordinated Universal Time», «14:12:47 Coordinated Universal Time») to be true
                "dayPeriod-narrow-en.js", //00:00, narrow format Expected SameValue(«12/12/2017», «at night») to be true
                "dayPeriod-short-en.js", //00:00, short format Expected SameValue(«12/12/2017», «at night») to be true
                "dayPeriod-long-en.js", //00:00, long format Expected SameValue(«12/12/2017», «at night») to be true
                "fractionalSecondDigits.js" //1 fractionalSecondDigits round down Expected SameValue(«02:03», «02:03.2») to be true
        ));

        Set<String> blackList = new HashSet<>();
        blackList.addAll(deviations);
        blackList.addAll(unSupportedList);

        runTests(basePath, blackList, whiteList);
    }


    public void testIntlDateTimeFormat_prototype_formatToParts() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/formatToParts";

        Set<String> whilteList = new HashSet<>();

        // timeStyle, dateStyle, dayPeriod, fractionalSecondDigits properties are not supported. Please find details above.
        Set<String> unSupportedList = new HashSet<>(Arrays.asList(
                "fractionalSecondDigits.js", //length should be 5, 1 fractionalSecondDigits round down Expected SameValue(«3», «5») to be true
                "dayPeriod-narrow-en.js", //length should be 1, 00:00, narrow format Expected SameValue(«5», «1») to be true
                "dayPeriod-long-en.js", //length should be 1, 00:00, long format Expected SameValue(«5», «1») to be true
                "dayPeriod-short-en.js" //length should be 1, 00:00, short format Expected SameValue(«5», «1») to be true
        ));

        Set<String> blackList = new HashSet<>();
        blackList.addAll(unSupportedList);

        runTests(basePath, blackList, whilteList);
    }


    public void testIntlDateTimeFormat_prototype_resolvedOptions() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/resolvedOptions";

        Set<String> whilteList = new HashSet<>();

        Set<String> deviations = new HashSet<>(Arrays.asList(
                "order.js", // Expected true but got false .. We don't have deterministic ordering of keys.
                "order-style.js" // Expected true but got false .. We don't have deterministic ordering of keys.
        ));

        // timeStyle, dateStyle, dayPeriod, fractionalSecondDigits properties are not supported. Please find details above.
        Set<String> unSupportedList = new HashSet<>(Arrays.asList(
                "order-fractionalSecondDigits.js", // Expected true but got false
                "hourCycle-dateStyle.js", //Should support dateStyle=full Expected SameValue(«undefined», «full») to be true
                "hourCycle-timeStyle.js", //Should support timeStyle=full Expected SameValue(«undefined», «full») to be true
                "order-dayPeriod.js" // Expected true but got false .. We don't have deterministic ordering of keys.
        ));

        Set<String> blackList = new HashSet<>();
        blackList.addAll(deviations);
        blackList.addAll(unSupportedList);

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlDateTimeFormat_prototype_toStringTag() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype/toStringTag";

        Set<String> whilteList = new HashSet<>();

        Set<String> deviations = new HashSet<>(Arrays.asList(
                "toString.js", // Expected SameValue(«[object Object]», «[object Intl.DateTimeFormat]») to be true
                "toStringTag.js" // descriptor value should be Intl.DateTimeFormat
        ));

        Set<String> blackList = new HashSet<>();
        blackList.addAll(deviations);

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlDateTimeFormat_prototype() throws IOException {

        String basePath = "test262-main/test/intl402/DateTimeFormat/prototype";

        Set<String> whilteList = new HashSet<>();
        Set<String> blackList = new HashSet<>();

        runTests(basePath, blackList, whilteList);
    }
}