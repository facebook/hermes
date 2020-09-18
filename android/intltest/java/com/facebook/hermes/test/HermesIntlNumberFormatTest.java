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
public class HermesIntlNumberFormatTest extends HermesIntlTest262Base {

    private static final String LOG_TAG = "HermesIntlNumberFormatTest";

    public void testIntlNumberFormat() throws IOException {

        String basePath = "test262-main/test/intl402/NumberFormat/";

        Set<String> whiteList = new HashSet<>();

        Set<String> deviations = new HashSet<>(Arrays.asList(
                "dft-currency-mnfd-range-check-mxfd.js", // This test is not correct based on spec. With ths currency in test, the default minfractiondigits is 2, and the maxfractiondigita cannot be set to 1. Both Firefox and Chrome also fails the test.
                "constructor-order.js",  // When strictly following spec, the currency checks comes before unit check and the test will throw RangeError on currency validation before reaching the cdoe which throws TypeError on seeing undefined unit. But, we have a part of option validation in C++ code which throws all TypeErrors, which results in the TypeError getting thrown.
                "currency-digits.js", // Didn't get correct minimumFractionDigts for currency AFN. Expected SameValue(«0», «2») to be true
                "constructor-unit.js" // com.facebook.hermes.intl.JSRangeErrorException: Unknown unit: acre-per-acre .. We support only units directly known to https://developer.android.com/reference/android/icu/util/MeasureUnit .. MeasureFormat.format requires an instance of MeasureUnit.
        ));

        Set<String> testIssues = new HashSet<>(Arrays.asList(
                "proto-from-ctor-realm.js", // Property '$262' doesn't exist
                "constructor-locales-get-tostring.js", // Property 'Proxy' doesn't exist
                "subclassing.js", // Compiling JS failed: 18:1:invalid statement encountered. Buffer size 986 starts with: 2f2f20436f7079726967687420323031
                "constructor-locales-hasproperty.js"  // Property 'Proxy' doesn't exist
        ));

        Set<String> blackList = testIssues;
        blackList.addAll(deviations);

        runTests(basePath, blackList, whiteList);
    }

    @Test
    public void testIntlNumberFormat_prototype() throws IOException {

        String basePath = "test262-main/test/intl402/NumberFormat/prototype/";

        Set<String> whiteList = new HashSet<>();
        Set<String> blackList = new HashSet<>();

        runTests(basePath, blackList, whiteList);
    }

    @Test
    public void testIntlNumberFormat_supportedLocalesOf() throws IOException {

        String basePath = "test262-main/test/intl402/NumberFormat/supportedLocalesOf/";

        Set<String> whiteList = new HashSet<>();
        Set<String> testIssues = new HashSet<>(Arrays.asList(
                "builtin.js" // Property 'isConstructor' doesn't exist
        ));

        Set<String> blackList = testIssues;
        runTests(basePath, blackList, whiteList);
    }

    @Test
    public void testIntlNumberFormat_prototype_constructor() throws IOException {

        String basePath = "test262-main/test/intl402/NumberFormat/prototype/constructor/";

        Set<String> whiteList = new HashSet<>();
        Set<String> blackList = new HashSet<>();

        runTests(basePath, blackList, whiteList);
    }

    @Test
    public void testIntlNumberFormat_prototype_toStringTag() throws IOException {

        String basePath = "test262-main/test/intl402/NumberFormat/prototype/toStringTag/";

        Set<String> whiteList = new HashSet<>();
        Set<String> deviations = new HashSet<>(Arrays.asList(
                "configurable.js" // // Expected SameValue(«Object», «Intl.NumberFormat») to be true .. Test expects new Intl.NumberFormat().toString() to return "[object Intl.NumberFormat]" which Firefox does.. but hermes (and Chrome) returns "[object Object]:
        ));

        Set<String> testIssues = new HashSet<>(Arrays.asList(
                "prop-desc.js" // Property 'isConstructor' doesn't exist
        ));

        Set<String> blackList = testIssues;
        blackList.addAll(deviations);

        runTests(basePath, blackList, whiteList);
    }

    @Test
    public void testIntlNumberFormat_prototype_resolvedOptions() throws IOException {

        String basePath = "test262-main/test/intl402/NumberFormat/prototype/resolvedOptions/";

        Set<String> whiteList = new HashSet<>();
        Set<String> deviations = new HashSet<>(Arrays.asList(
                "order.js" // // Expected SameValue(«Object», «Intl.NumberFormat») to be true .. Test expects new Intl.NumberFormat().toString() to return "[object Intl.NumberFormat]" which Firefox does.. but hermes (and Chrome) returns "[object Object]:
        ));

        Set<String> testIssues = new HashSet<>(Arrays.asList(
                "builtin.js" // Property 'isConstructor' doesn't exist
        ));

        Set<String> blackList = testIssues;
        blackList.addAll(deviations);

        runTests(basePath, blackList, whiteList);
    }

    @Test
    public void testIntlNumberFormat_prototype_format() throws IOException {

        String basePath = "test262-main/test/intl402/NumberFormat/prototype/format/";

        Set<String> whiteList = new HashSet<>();

        // Our implementation doesn't support signDisplay.
        Set<String> signDisplayList = new HashSet<>(Arrays.asList(
                "signDisplay-currency-zh-TW.js", //Expected SameValue(«US$0.00», «+US$0.00») to be true
                "signDisplay-rounding.js", //Expected SameValue(«-0», «0») to be true
                "signDisplay-currency-de-DE.js", //Expected SameValue(«0,00 $», «+0,00 $») to be true
                "signDisplay-de-DE.js", //0 (always) Expected SameValue(«0», «+0») to be true
                "signDisplay-ko-KR.js", //0 (always) Expected SameValue(«0», «+0») to be true
                "signDisplay-currency-en-US.js", //Expected SameValue(«$0.00», «+$0.00») to be true
                "signDisplay-currency-ja-JP.js", //Expected SameValue(«$0.00», «+$0.00») to be true
                "signDisplay-zh-TW.js", //  -0.0001 (exceptZero) Expected SameValue(«-0», «0») to be true
                "signDisplay-en-US.js", //0 (always) Expected SameValue(«0», «+0») to be true
                "signDisplay-currency-ko-KR.js", //Expected SameValue(«US$0.00», «+US$0.00») to be true
                "signDisplay-ja-JP.js" //0 (always) Expected SameValue(«0», «+0») to be true
        ));

        Set<String> testIssues = new HashSet<>(Arrays.asList(
                "builtin.js", //Property 'isConstructor' doesn't exist -- ReferenceError: Property 'isConstructor' doesn't exist
                "format-function-builtin.js" //Property 'isConstructor' doesn't exist
        ));

        Set<String> unit = new HashSet<>(Arrays.asList(
            "units.js" // com.facebook.hermes.intl.JSRangeErrorException: Unknown unit: acre-per-acre .. We support only units directly known to https://developer.android.com/reference/android/icu/util/MeasureUnit .. MeasureFormat.format requires an instance of MeasureUnit.
        ));

        Set<String> blackList = new HashSet<>();
        blackList.addAll(signDisplayList);
        blackList.addAll(testIssues);
        blackList.addAll(unit);

        runTests(basePath, blackList, whiteList);
    }

    @Test
    public void testIntlNumberFormat_prototype_formatToParts() throws IOException {

        String basePath = "test262-main/test/intl402/NumberFormat/prototype/formatToParts/";

        Set<String> whiteList = new HashSet<>();

        // Our implementation doesn't support signDisplay.
        Set<String> signDisplayList = new HashSet<>(Arrays.asList(
                "signDisplay-zh-TW.js", //NaN (auto): parts[0].value Expected SameValue(«NaN», «非數值») to be true
                "signDisplay-currency-de-DE.js", //undefined: length Expected SameValue(«5», «6») to be true
                "signDisplay-de-DE.js", //0 (always): length Expected SameValue(«1», «2») to be true
                "signDisplay-currency-ko-KR.js", //undefined: length Expected SameValue(«4», «5») to be true
                "signDisplay-ko-KR.js", //0 (always): length Expected SameValue(«1», «2») to be true
                "signDisplay-currency-en-US.js", //undefined: length Expected SameValue(«4», «5») to be true
                "signDisplay-currency-ja-JP.js", //undefined: length Expected SameValue(«4», «5») to be true//"unit-zh-TW.js", //undefined: length Expected SameValue(«1», «4») to be true
                "signDisplay-ja-JP.js", //0 (always): length Expected SameValue(«1», «2») to be true
                "signDisplay-currency-zh-TW.js", //undefined: length Expected SameValue(«4», «5») to be true
                "signDisplay-en-US.js" //0 (always): length Expected SameValue(«1», «2») to be true
        ));

        // https://developer.android.com/reference/android/icu/text/MeasureFormat doesn't implementat "formatToCharacterIterator" method correctly. It always returns the whole formatted text as a single literal.
        Set<String> unitList = new HashSet<>(Arrays.asList(
                "unit.js", //Expected SameValue(«false», «true») to be true
                "unit-ja-JP.js", //undefined: length Expected SameValue(«1», «4») to be true
                "unit-ko-KR.js", //undefined: length Expected SameValue(«1», «3») to be true
                "unit-de-DE.js", //undefined: length Expected SameValue(«1», «4») to be true
                "unit-en-US.js", //undefined: length Expected SameValue(«1», «4») to be true
                "unit-zh-TW.js", //undefined: length Expected SameValue(«1», «4») to be true
                "percent-en-US.js" // unit: length Expected SameValue(«1», «3») to be true .. We have issue when formatting percentage as unit .. i.e. {style: "unit", unit: "percent"} .. We could hack to create a formatter in percent style, but it's tricky to deal with the x100 multiplier.
        ));

        Set<String> blackList = new HashSet<>();

        blackList.addAll(signDisplayList);
        blackList.addAll(unitList);

        runTests(basePath, blackList, whiteList);
    }
}