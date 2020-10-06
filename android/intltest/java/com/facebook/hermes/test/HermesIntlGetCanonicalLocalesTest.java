/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import android.content.res.AssetManager;
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
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

// Run "./gradlew :intltest:preparetest262" from the root to download and copy the test files to the APK assets.
public class HermesIntlGetCanonicalLocalesTest extends HermesIntlTest262Base {

    private static final String LOG_TAG = "HermesIntlGetCanonicalLocalesTest";

   @Test
   public void testIntlGetCanonicalLocales() throws IOException {
       String basePath = "test262-main/test/intl402/Intl/getCanonicalLocales";
       Set<String> whiteList = new HashSet<>();
       Set<String> blackList = new HashSet<>(Arrays.asList("Locale-object.js"
               , "canonicalized-tags.js" // All except one tag (cmn-hans-cn-u-ca-t-ca-x-t-u) passes. icu4j adds an extra 'yes' token to the unicode 'ca' extension!
               , "complex-region-subtag-replacement.js" // ICU4j don't do complex region replacement.
               , "non-iana-canon.js" // All except one tag (de-u-kf) passes. icu4j adds an extra 'yes' token to the unicode 'kf' extension !
               , "preferred-variant.js" // ICU4j don't canonicalize extensions
               , "transformed-ext-canonical.js" // ICU4j don't canonicalize extensions.
               , "transformed-ext-invalid.js"  // ICU4j don't canonicalize extensions.
               , "unicode-ext-canonicalize-region.js"  // ICU4j don't canonicalize extensions.
               , "unicode-ext-canonicalize-subdivision.js"  // ICU4j don't canonicalize extensions.
               , "unicode-ext-canonicalize-yes-to-true.js"  // ICU4j don't canonicalize extensions.
               , "unicode-ext-key-with-digit.js"  // ICU4j don't canonicalize extensions.
               , "grandfathered.js" // icu4j doesn't perform all grandfathered tag replacements.
               , "preferred-grandfathered.js" // icu4j doesn't perform all grandfathered tag replacements.
               // Note:: CLDR has a list of grandfathered/language/script/region/variant replacements that must happen along with the canonicalization. (https://github.com/unicode-org/cldr/blob/master/common/supplemental/supplementalMetadata.xml)
               // But, typically, the implementation can't/shouldn't lookup cldr while canonicaliation as it can be costly.
               // ICU typically hardcodes a small subset of translations in code .. for inst: https://github.com/unicode-org/icu/blob/12dc3772b1858c73bedd5cffdee0a5a41ce7c61a/icu4j/main/classes/core/src/com/ibm/icu/impl/locale/LanguageTag.java#L43
               // Which implies ICU APIs doesn't perform all translations based on spec.
               // Note that our canonicalization implementaton for pre-24 platform attemps to do most of the translations, based on tables generated from CLDR.
               // Another thing to note is that different version of android platform ships with different versions of ICU4J and CLDR. ( https://developer.android.com/guide/topics/resources/internationalization )
               // Which means, same locale id can potentially be localized to different canonical locale ids.
               , "invalid-tags.js" // icu4j canonicalization doesn't reject all the locales that are invalid based on spec.
               , "complex-language-subtag-replacement.js" // icu4j canonicalization doesn't perform complex subtag replacements.
       ));

       Set<String> testIssuesList = new HashSet<>(Arrays.asList(
               "has-property.js" // Hermes's proxy implementation doesn't call 'has' trap.
       ));

       Set<String> icuIssues = new HashSet<>();
       // ICU:58.2 	CLDR:30.0.3 	UniCode: 9.0
       if(android.os.Build.VERSION.SDK_INT < 28) {
           icuIssues.addAll(Arrays.asList(
                   "unicode-ext-canonicalize-timezone.js" // Expected SameValue(«und-u-tz-utc», «und-u-tz-gmt») to be true
           ));
       }

       // ICU:56 	CLDR:28 	UniCode: 8.0
       if(android.os.Build.VERSION.SDK_INT < 26) {
           icuIssues.addAll(Arrays.asList(
                   "unicode-ext-canonicalize-measurement-system.js" // Expected SameValue(«und-u-ms-imperial», «und-u-ms-uksystem») to be true
           ));
       }

       Set<String> pre24Issues = new HashSet<>();
       if(android.os.Build.VERSION.SDK_INT < 24) {
           pre24Issues.addAll(Arrays.asList(
                   "unicode-ext-canonicalize-calendar.js", // Expected SameValue(«und-u-ca-ethiopic-amete-alem», «und-u-ca-ethioaa») to be true
                   "unicode-ext-canonicalize-col-strength.js" // Expected SameValue(«und-u-ks-primary», «und-u-ks-level1») to be true
           ));
       }

       blackList.addAll(icuIssues);
       blackList.addAll(testIssuesList);
       blackList.addAll(pre24Issues);

       runTests(basePath, blackList, whiteList);
   }
}
