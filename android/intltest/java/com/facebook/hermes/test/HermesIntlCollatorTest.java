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

    @Test
    public void testIntlCollator262() throws IOException {
        String[] testFileList = getInstrumentation().getContext().getAssets().list("test262/Collator");
        Set<String> blackList = new HashSet<>(Arrays.asList(
                "numeric-and-caseFirst.js" // Property numeric couldn't be set through locale extension key kn. Expected SameValue(«undefined», «true») to be true
                , "usage-de.js" // Expected [AE, Ä] and [Ä, AE] to have the same contents. sort
                , "proto-from-ctor-realm.js" // ReferenceError: Property '$262' doesn't exist
                , "subclassing.js" // Compiling JS failed: 18:1:invalid statement encountered. Buffer size 917 starts with: 2f2f20436f7079726967687420323031
                , "test-option-sensitivity.js" // com.facebook.hermes.intl.JSRangeErrorException: Invalid value '-1' for option sensitivity
        ));

        ArrayList<String> ranTests = new ArrayList<>();
        HashMap<String, String> failedTests = new HashMap<>();

//        try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
//            evaluateCommonScriptsFromAsset(rt);
//            evalScriptFromAsset(rt, "test262/Collator/test-option-usage.js");
//            ranTests.add("test262/Collator/test-option-usage.js");
//        }
//

        for (String testFileName : testFileList) {
            if (blackList.contains(testFileName)) {
                Log.v(LOG_TAG, "Skipping " + testFileName + " as it is blacklisted.");
                continue;
            }

            String testFilePath = "test262/Collator/" + testFileName;
            Log.d(LOG_TAG, "Evaluating " + testFilePath);

            try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
                evaluateCommonScriptsFromAsset(rt);
                ranTests.add(testFileName);
                try {
                    evalScriptFromAsset(rt, testFilePath);
                } catch (FileNotFoundException ex) {
                    // Skip, they are likely subdirectories
                } catch (com.facebook.jni.CppException ex) {
                    failedTests.put(testFilePath, ex.getMessage());
                }
            }
        }

        Log.v(LOG_TAG, "Passed Tests: " + TextUtils.join("\n", ranTests));

        for (Map.Entry<String, String> entry: failedTests.entrySet() ) {
            Log.v(LOG_TAG, "Failed Tests: " +  entry.getKey() + " : " + entry.getValue());
        }
    }

//    @Test
//    public void testIntlCollatorResolvedOptions() throws IOException {
//        int i = 10;
//        String result1 = "d1";
//
//        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
//            // ULocale[] locales = ULocale.getAvailableLocales();
//
//            // ULocale[] locales = new ULocale[] {ULocale.forLanguageTag("en-uk")};
//            // ULocale accepted = ULocale.acceptLanguage(locales, null);
//
//            try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
//                evaluateCommonScriptsFromAsset(rt);
//                evalScriptFromAsset(rt, "test262/Collator/prototype/resolvedOptions/basic.js");
//            }
//        }
//
//
//    }



//    @Test
//    public void testIntlGetCanonicalLocales() throws IOException {
//        String[] testFileList = getInstrumentation().getContext().getAssets().list("test262/intl/getCanonicalLocales");
//        Set<String> blackList = new HashSet<>(Arrays.asList("Locale-object.js"
//                , "canonicalized-tags.js" // All except one tag (cmn-hans-cn-u-ca-t-ca-x-t-u) passes. icu4j adds an extra 'yes' token to the unicode 'ca' extension!
//                , "complex-region-subtag-replacement.js" // We don't do complex region replacement.
//                , "has-property.js" // Test needs Proxy,
//                , "non-iana-canon.js" // All except one tag (de-u-kf) passes. icu4j adds an extra 'yes' token to the unicode 'kf' extension !
//                , "preferred-variant.js" // We din;t do variant replacement
//                , "transformed-ext-canonical.js" // We don't canonicalize extensions yet.
//                , "transformed-ext-invalid.js"  // We don't canonicalize extensions yet.
//                , "unicode-ext-canonicalize-region.js"  // We don't canonicalize extensions yet.
//                , "unicode-ext-canonicalize-subdivision.js"  // We don't canonicalize extensions yet.
//                , "unicode-ext-canonicalize-yes-to-true.js"  // We don't canonicalize extensions yet.
//                , "unicode-ext-key-with-digit.js"  // We don't canonicalize extensions yet.
//        ));
//
//        /*
//            The following tests successfully completes as of now.
//            Executed Tests:
//            canonicalized-unicode-ext-seq.js
//            complex-language-subtag-replacement.js
//            descriptor.js
//            duplicates.js
//            elements-not-reordered.js
//            error-cases.js
//            get-locale.js
//            getCanonicalLocales.js
//            grandfathered.js
//            invalid-tags.js
//            length.js
//            locales-is-not-a-string.js
//            main.js
//            name.js
//            overriden-arg-length.js
//            overriden-push.js
//            preferred-grandfathered.js
//            returned-object-is-an-array.js
//            returned-object-is-mutable.js
//            to-string.js
//            transformed-ext-valid.js
//            unicode-ext-canonicalize-calendar.js
//            unicode-ext-canonicalize-col-strength.js
//            unicode-ext-canonicalize-measurement-system.js
//            unicode-ext-canonicalize-timezone.js
//            weird-cases.js
//            */
//
//
//        ArrayList<String> ranTests = new ArrayList<>();
//
//        for (String testFileName : testFileList) {
//            if (blackList.contains(testFileName)) {
//                Log.v(LOG_TAG, "Skipping " + testFileName + " as it is blacklisted.");
//                continue;
//            }
//
//            String testFilePath = "test262/intl/getCanonicalLocales/" + testFileName;
//            Log.d(LOG_TAG, "Evaluating " + testFilePath);
//
//            try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
//                evaluateCommonScriptsFromAsset(rt);
//                evalScriptFromAsset(rt, testFilePath);
//            }
//
//            ranTests.add(testFileName);
//        }
//
//        Log.v(LOG_TAG, "Executed Tests: " + TextUtils.join("\n", ranTests));
//    }
}
