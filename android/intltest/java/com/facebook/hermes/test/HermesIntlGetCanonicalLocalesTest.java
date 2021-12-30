/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import java.io.IOException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import org.junit.Test;

// Run "./gradlew :intltest:prepareTests" from the root to copy the test files to the
// APK assets.
public class HermesIntlGetCanonicalLocalesTest extends HermesIntlTest262Base {

  @Test
  public void testIntlGetCanonicalLocales() throws IOException {
    String basePath = "test262/test/intl402/Intl/getCanonicalLocales";

    Set<String> skipList =
        new HashSet<>(
            Arrays.asList(
                "Locale-object.js",
                "canonicalized-tags.js" // All except one tag (cmn-hans-cn-u-ca-t-ca-x-t-u) passes.
                // icu4j adds an extra 'yes' token to the unicode 'ca'
                // extension!
                ,
                "complex-region-subtag-replacement.js" // Expected SameValue(«ru-SU», «ru-RU») to be
                // true; ICU4j don't do complex region
                // replacement.
                ,
                "non-iana-canon.js" // The value of Intl.getCanonicalLocales(tag)[0] equals the
                // value of `canonical` Expected SameValue(«mo», «ro») to be
                // true; All except one tag (de-u-kf) passes. icu4j adds an
                // extra 'yes' token to the unicode 'kf' extension !
                ,
                "preferred-variant.js" // Expected SameValue(«ja-Latn-hepburn-heploc»,
                // «ja-Latn-alalc97-hepburn») to be true; ICU4j don't do
                // variant replacement.
                ,
                "transformed-ext-canonical.js" // SameValue(«sl-t-sl-rozaj-biske-1994»,
                // «sl-t-sl-1994-biske-rozaj») to be true; ICU4j
                // don't canonicalize extensions.
                ,
                "transformed-ext-invalid.js" // en-t-root Expected a RangeError to be thrown but no
                // exception was thrown at all; ICU4j don't
                // canonicalize extensions.
                ,
                "unicode-ext-canonicalize-region.js" // Expected SameValue(«und-u-rg-no23»,
                // «und-u-rg-no50») to be true; ICU4j don't
                // canonicalize extensions.
                ,
                "unicode-ext-canonicalize-subdivision.js" // Expected SameValue(«und-NO-u-sd-no23»,
                // «und-NO-u-sd-no50») to be true; ICU4j
                // don't canonicalize extensions.
                ,
                "unicode-ext-canonicalize-yes-to-true.js" // Expected SameValue(«und-u-kb-true»,
                // «und-u-kb») to be true; ICU4j don't
                // canonicalize extensions.
                ,
                "unicode-ext-key-with-digit.js" // Expected a RangeError to be thrown but no
                // exception was thrown at all; ICU4j don't
                // canonicalize extensions.
                ,
                "grandfathered.js" // iExpected SameValue(«cmn», «zh») cu4j doesn't perform all
                // grandfathered tag replacements.
                ,
                "preferred-grandfathered.js" // Expected SameValue(«cmn», «zh») to be true .. icu4j
                // doesn't perform all grandfathered tag replacements.
                // Note:: CLDR has a list of grandfathered/language/script/region/variant
                // replacements that must happen along with the canonicalization.
                // (https://github.com/unicode-org/cldr/blob/master/common/supplemental/supplementalMetadata.xml)
                // But, typically, the implementation can't/shouldn't lookup cldr while
                // canonicaliation as it can be costly.
                // ICU typically hardcodes a small subset of translations in code .. for inst:
                // https://github.com/unicode-org/icu/blob/12dc3772b1858c73bedd5cffdee0a5a41ce7c61a/icu4j/main/classes/core/src/com/ibm/icu/impl/locale/LanguageTag.java#L43
                // Which implies ICU APIs doesn't perform all translations based on spec.
                // Note that our canonicalization implementaton for pre-24 platform attemps to do
                // most of the translations, based on tables generated from CLDR.
                // Another thing to note is that different version of android platform ships with
                // different versions of ICU4J and CLDR. (
                // https://developer.android.com/guide/topics/resources/internationalization )
                // Which means, same locale id can potentially be localized to different canonical
                // locale ids.
                ,
                "invalid-tags.js" // Language tag: de-gregory-gregory Expected a RangeError to be
                // thrown but no exception was thrown at all; icu4j
                // canonicalization doesn't reject all the locales that are
                // invalid based on spec.
                ,
                "complex-language-subtag-replacement.js" // Expected SameValue(«ru-SU», «ru-RU») to
                // be true; icu4j canonicalization doesn't
                // perform complex subtag replacements.
                ));

    Set<String> testIssuesList =
        new HashSet<>(
            Arrays.asList(
                "has-property.js" // Hermes's proxy implementation doesn't call 'has' trap.
                ));

    Set<String> icuIssues = new HashSet<>();
    // ICU:58.2 	CLDR:30.0.3 	UniCode: 9.0
    if (android.os.Build.VERSION.SDK_INT < 28) {
      icuIssues.addAll(
          Arrays.asList(
              "unicode-ext-canonicalize-timezone.js" // Expected SameValue(«und-u-tz-utc»,
              // «und-u-tz-gmt») to be true
              ));
    }

    // ICU:56 	CLDR:28 	UniCode: 8.0
    if (android.os.Build.VERSION.SDK_INT < 26) {
      icuIssues.addAll(
          Arrays.asList(
              "unicode-ext-canonicalize-measurement-system.js" // Expected
              // SameValue(«und-u-ms-imperial»,
              // «und-u-ms-uksystem») to be true
              ));
    }

    Set<String> pre24Issues = new HashSet<>();
    if (android.os.Build.VERSION.SDK_INT < 24) {
      pre24Issues.addAll(
          Arrays.asList(
              "unicode-ext-canonicalize-calendar.js", // Expected
              // SameValue(«und-u-ca-ethiopic-amete-alem»,
              // «und-u-ca-ethioaa») to be true
              "unicode-ext-canonicalize-col-strength.js" // Expected SameValue(«und-u-ks-primary»,
              // «und-u-ks-level1») to be true
              ));
    }

    skipList.addAll(icuIssues);
    skipList.addAll(testIssuesList);
    skipList.addAll(pre24Issues);

    runTests(basePath, skipList);

    // Passed Tests:
    //       descriptor.js
    //       duplicates.js
    //       elements-not-reordered.js
    //       error-cases.js
    //       get-locale.js
    //       getCanonicalLocales.js
    //       length.js
    //       locales-is-not-a-string.js
    //       main.js
    //       name.js
    //       overriden-arg-length.js
    //       overriden-push.js
    //       returned-object-is-an-array.js
    //       returned-object-is-mutable.js
    //       to-string.js
    //       transformed-ext-valid.js
    //       unicode-ext-canonicalize-calendar.js
    //       unicode-ext-canonicalize-col-strength.js
    //       unicode-ext-canonicalize-measurement-system.js
    //       unicode-ext-canonicalize-timezone.js
    //       weird-cases.js
  }
}
