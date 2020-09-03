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

public class HermesIntlNumberFormatTest extends InstrumentationTestCase {

    private static final String LOG_TAG = "HermesIntlNumberFormatTest";

    private void evalScriptFromAsset(JSRuntime rt, String filename) throws IOException {
        AssetManager assets = getInstrumentation().getContext().getAssets();
        InputStream is = assets.open(filename);
        String script = new BufferedReader(new InputStreamReader(is))
                .lines().collect(Collectors.joining("\n"));
        rt.evaluateJavaScript(script);

    }

    private void evaluateCommonScriptsFromAsset(JSRuntime rt) throws IOException {
        evalScriptFromAsset(rt, "test262/intl/common/sta.js");
        evalScriptFromAsset(rt, "test262/intl/common/assert.js");
        evalScriptFromAsset(rt, "test262/intl/common/testintl.js");
        evalScriptFromAsset(rt, "test262/intl/common/propertyHelpers.js");
        evalScriptFromAsset(rt, "test262/intl/common/compareArray.js");
        evalScriptFromAsset(rt, "test262/intl/common/testintl.js");

    }

    private void runTests(String basePath, Set<String> blackList) throws IOException {
        String[] testFileList = getInstrumentation().getContext().getAssets().list(basePath);

        ArrayList<String> ranTests = new ArrayList<>();
        HashMap<String, String> failedTests = new HashMap<>();

        for (String testFileName : testFileList) {
            String testFilePath = basePath + testFileName;
            Log.d(LOG_TAG, "Evaluating " + testFilePath);

            try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
                evaluateCommonScriptsFromAsset(rt);
                ranTests.add(testFileName);
                try {
                    evalScriptFromAsset(rt, testFilePath);
                } catch (FileNotFoundException ex) {
                    if (testFilePath.endsWith(".js"))
                        throw ex;
                    // Skip, they are likely subdirectories
                } catch (com.facebook.jni.CppException ex) {
                    if (!blackList.contains(testFileName))
                        failedTests.put(testFilePath, ex.getMessage());
                } catch (Exception ex) {
                    if   (!blackList.contains(testFileName))
                        failedTests.put(testFilePath, ex.getMessage());
                }
            }
        }

        Log.v(LOG_TAG, "Passed Tests: " + TextUtils.join("\n", ranTests));

        for (
                Map.Entry<String, String> entry : failedTests.entrySet()) {
            Log.v(LOG_TAG, "Failed Tests: " + entry.getKey() + " : " + entry.getValue());
            assert (false);
        }

        assertThat(failedTests.entrySet().

                isEmpty()).

                isEqualTo(true);

    }

    public void testIntlNumberFormat() throws IOException {

        String basePath = "test262/NumberFormat/";

        Set<String> blackList = new HashSet<>(Arrays.asList(
                "proto-from-ctor-realm.js", // Property '$262' doesn't exist
                "constructor-locales-get-tostring.js", // Property 'Proxy' doesn't exist
                "subclassing.js", // Compiling JS failed: 18:1:invalid statement encountered. Buffer size 986 starts with: 2f2f20436f7079726967687420323031
                "constructor-order.js", // Expected a RangeError but got a TypeError
                "constructor-locales-hasproperty.js",  // Property 'Proxy' doesn't exist
                "currency-digits.js" // Didn't get correct minimumFractionDigits for currency AFN. Expected SameValue(«0», «2») to be true
        ));

        runTests3(basePath, blackList);
    }

    @Test
    public void testIntlNumberFormat_prototype() throws IOException {

        String basePath = "test262/NumberFormat/prototype/";

        Set<String> blackList = new HashSet<>();


        runTests(basePath, blackList);
    }

    @Test
    public void testIntlNumberFormat_supportedLocalesOf() throws IOException {

        String basePath = "test262/NumberFormat/supportedLocalesOf/";
        Set<String> blackList = new HashSet<>(Arrays.asList(
                "builtin.js" // Property 'isConstructor' doesn't exist
        ));

        runTests(basePath, blackList);
    }


    @Test
    public void testIntlNumberFormat_prototype_constructor() throws IOException {

        String basePath = "test262/NumberFormat/prototype/constructor/";

        Set<String> blackList = new HashSet<>();

        runTests(basePath, blackList);
    }


    @Test
    public void testIntlNumberFormat_prototype_toStringTag() throws IOException {

        String basePath = "test262/NumberFormat/prototype/toStringTag/";

        Set<String> blackList = new HashSet<>(Arrays.asList(
                "prop-desc.js", // Property 'isConstructor' doesn't exist
                "configurable.js" // Expected SameValue(«Object», «Intl.NumberFormat») to be true
        ));

        runTests(basePath, blackList);
    }

    @Test
    public void testIntlNumberFormat_prototype_resolvedOptions() throws IOException {

        String basePath = "test262/NumberFormat/prototype/resolvedOptions/";

        Set<String> blackList = new HashSet<>(Arrays.asList(
                "order.js", // Expected [notation, locale, minimumSignificantDigits, maximumSignificantDigits, minimumIntegerDigits, currency, signDisplay, numberingSystem, useGrouping, style, currencyDisplay, currencySign] and [locale, numberingSystem, style, currency, currencyDisplay, currencySign, minimumIntegerDigits, minimumSignificantDigits, maximumSignificantDigits, useGrouping, notation, signDisplay] to have the same contents.
                "builtin.js" // Property 'isConstructor' doesn't exist
        ));

        runTests(basePath, blackList);
    }
//    @Test

    @Test
    public void testIntlNumberFormat_prototype_format() throws IOException {

        String basePath = "test262/NumberFormat/prototype/format/";

        Set<String> blackList = new HashSet<>(Arrays.asList(
                "signDisplay-currency-zh-TW.js", //Expected SameValue(«US$0.00», «+US$0.00») to be true
                "signDisplay-rounding.js", //Expected SameValue(«-0», «0») to be true
                "signDisplay-currency-de-DE.js", //Expected SameValue(«0,00 $», «+0,00 $») to be true
                "signDisplay-de-DE.js", //0 (always) Expected SameValue(«0», «+0») to be true
                "signDisplay-ko-KR.js", //0 (always) Expected SameValue(«0», «+0») to be true
                "signDisplay-currency-en-US.js", //Expected SameValue(«$0.00», «+$0.00») to be true
                "signDisplay-currency-ja-JP.js", //Expected SameValue(«$0.00», «+$0.00») to be true
                "signDisplay-zh-TW.js", //NaN (auto) Expected SameValue(«NaN», «非數值») to be true
                "notation-compact-zh-TW.js", //Expected SameValue(«9.9亿», «9.9億») to be true
                "unit-zh-TW.js", //Expected SameValue(«每小时-987公里», «-987 公里/小時») to be true
                "builtin.js", //Property 'isConstructor' doesn't exist -- ReferenceError: Property 'isConstructor' doesn't exist
                "engineering-scientific-zh-TW.js", //Expected SameValue(«NaN», «非數值») to be true
                "signDisplay-en-US.js", //0 (always) Expected SameValue(«0», «+0») to be true
                "units.js", //Expected SameValue(«123», «123») to be false
                "format-function-length.js", //Expected SameValue(«0», «1») to be true
                "signDisplay-currency-ko-KR.js", //Expected SameValue(«US$0.00», «+US$0.00») to be true
                "signDisplay-ja-JP.js", //0 (always) Expected SameValue(«0», «+0») to be true
                "format-function-builtin.js" //Property 'isConstructor' doesn't exist
        ));

        runTests(basePath, blackList);
    }


    @Test
    public void testIntlNumberFormat_prototype_formatToParts() throws IOException {

        String basePath = "test262/NumberFormat/prototype/formatToParts/";

        Set<String> blackList = new HashSet<>(Arrays.asList(
                "signDisplay-zh-TW.js", //NaN (auto): parts[0].value Expected SameValue(«NaN», «非數值») to be true
                "unit-zh-TW.js", //undefined: length Expected SameValue(«1», «4») to be true
                "unit.js", //Expected SameValue(«false», «true») to be true
                "notation-compact-zh-TW.js", //Compact short: 987654321: parts[3].type Expected SameValue(«literal», «compact») to be true
                "length.js", //Expected SameValue(«0», «1») to be true
                "signDisplay-currency-de-DE.js", //undefined: length Expected SameValue(«5», «6») to be true
                "signDisplay-en-US.js", //0 (always): length Expected SameValue(«1», «2») to be true
                "notation-compact-ko-KR.js", //Compact short: 987654321: parts[3].type Expected SameValue(«literal», «compact») to be true
                "engineering-scientific-zh-TW.js", //NaN - engineering: parts[0].value Expected SameValue(«NaN», «非數值») to be true
                "unit-ko-KR.js", //undefined: length Expected SameValue(«1», «3») to be true
                "signDisplay-de-DE.js", //0 (always): length Expected SameValue(«1», «2») to be true
                "signDisplay-currency-ko-KR.js", //undefined: length Expected SameValue(«4», «5») to be true
                "notation-compact-ja-JP.js", //Compact short: 987654321: parts[3].type Expected SameValue(«literal», «compact») to be true
                "signDisplay-ja-JP.js", //0 (always): length Expected SameValue(«1», «2») to be true
                "signDisplay-currency-zh-TW.js", //undefined: length Expected SameValue(«4», «5») to be true
                "unit-de-DE.js", //undefined: length Expected SameValue(«1», «4») to be true
                "percent-en-US.js", //unit: length Expected SameValue(«1», «3») to be true
                "signDisplay-ko-KR.js", //0 (always): length Expected SameValue(«1», «2») to be true
                "unit-en-US.js", //undefined: length Expected SameValue(«1», «4») to be true
                "notation-compact-en-US.js", //Compact short: 987654321: parts[1].type Expected SameValue(«literal», «compact») to be true
                "unit-ja-JP.js", //undefined: length Expected SameValue(«1», «4») to be true
                "signDisplay-currency-en-US.js", //undefined: length Expected SameValue(«4», «5») to be true
                "notation-compact-de-DE.js", //Compact short: 987654321: parts[2].type Expected SameValue(«literal», «compact») to be true
                "signDisplay-currency-ja-JP.js" //undefined: length Expected SameValue(«4», «5») to be true//
        ));

        runTests(basePath, blackList);
    }
}