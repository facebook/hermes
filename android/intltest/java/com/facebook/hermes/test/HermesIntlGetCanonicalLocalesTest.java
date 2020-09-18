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
        String basePath = "test262-main/test/intl402/Intl/getCanonicalLocales/";
        Set<String> whiteList = new HashSet<>();
        Set<String> blackList = new HashSet<>(Arrays.asList("Locale-object.js"
                , "canonicalized-tags.js" // All except one tag (cmn-hans-cn-u-ca-t-ca-x-t-u) passes. icu4j adds an extra 'yes' token to the unicode 'ca' extension!
                , "complex-region-subtag-replacement.js" // We don't do complex region replacement.
                , "has-property.js" // Test needs Proxy,
                , "non-iana-canon.js" // All except one tag (de-u-kf) passes. icu4j adds an extra 'yes' token to the unicode 'kf' extension !
                , "preferred-variant.js" // We don;t do variant replacement
                , "transformed-ext-canonical.js" // We don't canonicalize extensions yet.
                , "transformed-ext-invalid.js"  // We don't canonicalize extensions yet.
                , "unicode-ext-canonicalize-region.js"  // We don't canonicalize extensions yet.
                , "unicode-ext-canonicalize-subdivision.js"  // We don't canonicalize extensions yet.
                , "unicode-ext-canonicalize-yes-to-true.js"  // We don't canonicalize extensions yet.
                , "unicode-ext-key-with-digit.js"  // We don't canonicalize extensions yet.
                , "grandfathered.js" // icu4j doesn't perform all grandfathered tag replacements.
                , "preferred-grandfathered.js" // icu4j doesn't perform all grandfathered tag replacements.
                , "invalid-tags.js" // // icu4j !
                , "complex-language-subtag-replacement.js" // icu4j canonicalization doesn't perform complex subtag replacements.
        ));

        /*
            The following tests successfully completes as of now with ICU implementation.
            Our own parsing implementation which is used for platform version before Lollypop passes most tests !
            Executed Tests:
            canonicalized-unicode-ext-seq.js
            complex-language-subtag-replacement.js
            descriptor.js
            duplicates.js
            elements-not-reordered.js
            error-cases.js
            get-locale.js
            getCanonicalLocales.js
            grandfathered.js
            invalid-tags.js
            length.js
            locales-is-not-a-string.js
            main.js
            name.js
            overriden-arg-length.js
            overriden-push.js
            preferred-grandfathered.js
            returned-object-is-an-array.js
            returned-object-is-mutable.js
            to-string.js
            transformed-ext-valid.js
            unicode-ext-canonicalize-calendar.js
            unicode-ext-canonicalize-col-strength.js
            unicode-ext-canonicalize-measurement-system.js
            unicode-ext-canonicalize-timezone.js
            weird-cases.js
        */

        runTests(basePath, blackList, whiteList);
    }
}
