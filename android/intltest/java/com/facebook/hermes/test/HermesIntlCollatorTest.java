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

public class HermesIntlCollatorTest extends InstrumentationTestCase {

    private static final String LOG_TAG = "HermesIntlCollatorTest";

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
                    //if (testFilePath.endsWith(".js"))
                    //    throw ex;
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

        for (Map.Entry<String, String> entry : failedTests.entrySet()) {
            Log.v(LOG_TAG, "Failed Tests: " + entry.getKey() + " : " + entry.getValue());
            assert (false);
        }

        assertThat(failedTests.entrySet().isEmpty()).isEqualTo(true);

    }

    @Test
    public void testIntlCollator262() throws IOException {

        String basePath = "test262/Collator/";

        Set<String> blackList = new HashSet<>(Arrays.asList(
                "subclassing.js",  // Test requires Javascript classes
                "proto-from-ctor-realm.js", // ReferenceError: Property '$262' doesn't exist .. Test requires Reflect
                "unicode-ext-value-collation.js", // Collation for "standard" should be default, but is search. Expected SameValue(«-1», «-1») to be false << TODO >>
                "ignore-invalid-unicode-ext-values.js", // Locale en is affected by key co; value standard. Expected SameValue(«en», «en-US») to be true << TODO >>
                "missing-unicode-ext-value-defaults-to-true.js", // "kn-true" is returned in locale, but shouldn't be. Expected SameValue(«15», «-1») to be true << TODO >>
                "missing-unicode-ext-value-defaults-to-true.js", // "kn-true" is returned in locale, but shouldn't be. Expected SameValue(«15», «-1») to be true
                "numeric-and-caseFirst.js" // "Property numeric couldn't be set through locale extension key kn. Expected SameValue(«undefined», «true») to be true // This is because icu4j collator doesn't support the extension
        ));

//        Passed Tests:
//        builtin.js
//        constructor-options-throwing-getters.js
//        default-options-object-prototype.js
//        ignore-invalid-unicode-ext-values.js
//        instance-proto-and-extensible.js
//        legacy-regexp-statics-not-modified.js
//        length.js
//        missing-unicode-ext-value-defaults-to-true.js
//        name.js
//        numeric-and-caseFirst.js
//        prop-desc.js
//        taint-Object-prototype.js
//        test-option-ignorePunctuation.js
//        test-option-localeMatcher.js
//        test-option-numeric-and-caseFirst.js
//        test-option-sensitivity.js
//        test-option-usage.js
//        this-value-ignored.js
//        unicode-ext-seq-in-private-tag.js
//        unicode-ext-seq-with-attribute.js
//        unicode-ext-value-collation.js
//        usage-de.js

        runTests(basePath, blackList);
    }

    public void test262_prototype() throws IOException {
        String basePath = "test262/Collator/prototype";
        Set<String> blackList = new HashSet<>(Arrays.asList(""
        ));

        runTests(basePath, blackList);
    }

    public void test262_prototype_resolvedOptions() throws IOException {
        String basePath = "test262/Collator/prototype/resolvedOptions";
        Set<String> blackList = new HashSet<>(Arrays.asList("order.js" // Expected [locale, sensitivity, ignorePunctuation, caseFirst, collation, numeric, usage] and [locale, usage, sensitivity, ignorePunctuation, collation, numeric, caseFirst] to have the same contents.
                // TODO :: We fail the above test above we use std::unordered_map to hold the options in the C++ binding layer between java/platform code and VM
                , "builtin.js" // Property 'isConstructor' doesn't exist // needs Reflect.construct
        ));

//        Passed Tests:
//        basic.js
//        length.js
//        name.js
//        prop-desc.js

        runTests(basePath, blackList);
    }

    public void test262_prototype_constructor() throws IOException {
        String basePath = "test262/Collator/prototype/constructor";
        Set<String> blackList = new HashSet<>(Arrays.asList(""
        ));

//        Passed Tests:
//        prop-desc.js
//        value.js

        runTests(basePath, blackList);
    }

    public void test262_prototype_compare() throws IOException {
        String basePath = "test262/Collator/prototype/compare";
        Set<String> blackList = new HashSet<>(Arrays.asList("compare-function-length.js" // Expected SameValue(«0», «2») to be true .. Hermes Function.length doesn't seem to work correctly
                , "canonically-equivalent-strings.js" // Collator.compare considers ö (\u006f\u0308) ≠ ö (\u00f6). Expected SameValue(«-135», «0») to be true
                , "non-normative-sensitivity.js" // Expected [Aa] and [Aa, aA, áÁ, Aã] to have the same contents.
                , "non-normative-basic.js" // Expected [A, C, E, H, J, L, N, V, X, Z, b, d, f, g, i, k, m, w, y] and [A, b, C, d, E, f, g, H, i, J, k, L, m, N, V, w, X, y, Z] to have the same contents.
                , "compare-function-builtin.js" // Property 'isConstructor' doesn't exist
                , "builtin.js" // Property 'isConstructor' doesn't exist
        ));

//        Passed Tests:
//        bound-to-collator-instance.js
//        compare-function-name.js
//        length.js
//        name.js
//        non-normative-phonebook.js
//        prop-desc.js

        runTests(basePath, blackList);
    }
}
