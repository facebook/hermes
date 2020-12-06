/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import java.io.IOException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import org.junit.Test;

// Run "./gradlew :intltest:preparetest262" from the root to download and copy the test files to the
// APK assets.
public class HermesIntlCollatorTest extends HermesIntlTest262Base {

  private static final String LOG_TAG = "HermesIntlCollatorTest";

  @Test
  public void testIntlCollator() throws IOException {

    String basePath = "test262-main/test/intl402/Collator";
    Set<String> whilteList = new HashSet<>();

    Set<String> deviations =
        new HashSet<>(
            Arrays.asList(
                "constructor-options-throwing-getters.js", // TODO -- Need to investigate how this
                // regressed.
                "ignore-invalid-unicode-ext-values.js" // TODO [Follow-up] Failing because Hermes
                // array.sort sort is not stable. In-place
                // sorting changes the order when the input
                // is already sorted with call to array.sort.
                ));

    Set<String> testIssueList =
        new HashSet<>(
            Arrays.asList(
                "subclassing.js", // Test requires Javascript classes
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

    Set<String> deviations =
        new HashSet<>(
            Arrays.asList(
                "order.js" // Expected [locale, sensitivity, ignorePunctuation, caseFirst,
                // collation, numeric, usage] and [locale, usage, sensitivity,
                // ignorePunctuation, collation, numeric, caseFirst] to have the same
                // contents.
                // TODO :: [Follow-up] We fail the above test above we use std::unordered_map to
                // hold the options in the C++ binding layer between java/platform code and VM
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
    Set<String> blackList = new HashSet<>();

    // ICU APIs not available prior to 24.
    Set<String> pre24Issues = new HashSet<>();
    if (android.os.Build.VERSION.SDK_INT < 24) {
      pre24Issues.addAll(
          Arrays.asList(
              "non-normative-sensitivity.js", // Expected [Aa] and [Aa, Aã] to have the same
              // contents.. Pre-24 collator object doesn't expose an
              // API to specifiy decomposition mode.
              "canonically-equivalent-strings.js" // Collator.compare considers ạ̈ (\u00e4\u0323) ≠
              // ạ̈ (\u0061\u0323\u0308). Expected
              // SameValue(«-1», «0») to be true. Pre-24
              // collator object doesn't expose an API to
              // specifiy decomposition mode.
              ));
    }

    blackList.addAll(pre24Issues);

    runTests(basePath, blackList, whilteList);
  }
}
