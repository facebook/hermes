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
public class HermesIntlCollatorTest extends HermesIntlTest262Base {

    private static final String LOG_TAG = "HermesIntlCollatorTest";

    @Test
    public void testIntlCollator() throws IOException {

        String basePath = "test262-main/test/intl402/Collator";
        Set<String> whilteList = new HashSet<>();

        Set<String> deviations = new HashSet<>(Arrays.asList(
                "ignore-invalid-unicode-ext-values.js" // TODO [Follow-up] Failing because Hermes array.sort sort is not stable. In-place sorting changes the order when the input is already sorted with call to array.sort.
        ));

        Set<String> testIssueList = new HashSet<>(Arrays.asList(
                "subclassing.js",  // Test requires Javascript classes
                "proto-from-ctor-realm.js" // Hermes doesn't support Realms (Isolated Environments).
        ));

        Set<String> blackList = new HashSet<>();
        blackList.addAll(testIssueList);
        blackList.addAll(deviations);

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlCollator_prototype() throws IOException {
        String basePath = "test262-main/test/intl402/Collator/prototype";
        Set<String> whilteList = new HashSet<>();
        Set<String> blackList = new HashSet<>();

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlCollator_prototype_resolvedOptions() throws IOException {
        String basePath = "test262-main/test/intl402/Collator/prototype/resolvedOptions";

        Set<String> whilteList = new HashSet<>();

        Set<String> deviations = new HashSet<>(Arrays.asList(
                "order.js"  // Expected [locale, sensitivity, ignorePunctuation, caseFirst, collation, numeric, usage] and [locale, usage, sensitivity, ignorePunctuation, collation, numeric, caseFirst] to have the same contents.
                // TODO :: [Follow-up] We fail the above test above we use std::unordered_map to hold the options in the C++ binding layer between java/platform code and VM
        ));

        Set<String> testIssueList = new HashSet<>();

        Set<String> blackList = new HashSet<>();
        blackList.addAll(deviations);
        blackList.addAll(testIssueList);

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlCollator_prototype_constructor() throws IOException {
        String basePath = "test262-main/test/intl402/Collator/prototype/constructor";
        Set<String> whilteList = new HashSet<>();
        Set<String> blackList = new HashSet<>();

        runTests(basePath, blackList, whilteList);
    }

    public void testIntlCollator_prototype_compare() throws IOException {
        String basePath = "test262-main/test/intl402/Collator/prototype/compare";
        Set<String> whilteList = new HashSet<>();
        Set<String> testIssueList = new HashSet<>();

        runTests(basePath, testIssueList, whilteList);
    }
}
