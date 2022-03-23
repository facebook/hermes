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
public class HermesIntlNumberFormatTest extends HermesIntlTest262Base {

  public void testIntlNumberFormat() throws IOException {

    String basePath = "test262/test/intl402/NumberFormat";

    Set<String> deviations =
        new HashSet<>(
            Arrays.asList(
                "constructor-locales-hasproperty.js", // Currently We/Hermes don't call the "has"
                // trap in proxy.
                "dft-currency-mnfd-range-check-mxfd.js", // This test is not correct based on spec.
                // With ths currency in test, the default
                // minfractiondigits is 2, and the
                // maxfractiondigita cannot be set to 1.
                // Both Firefox and Chrome also fails the
                // test.
                "constructor-order.js", // When strictly following spec, the currency checks comes
                // before unit check and the test will throw RangeError on
                // currency validation before reaching the cdoe which throws
                // TypeError on seeing undefined unit. But, we have a part
                // of option validation in C++ code which throws all
                // TypeErrors, which results in the TypeError getting
                // thrown.
                "currency-digits.js", // Didn't get correct minimumFractionDigts for currency AFN.
                // Expected SameValue(«0», «2») to be true
                "constructor-unit.js", // com.facebook.hermes.intl.JSRangeErrorException: Unknown
                // unit: acre-per-acre .. We support only units directly known
                // to
                // https://developer.android.com/reference/android/icu/util/MeasureUnit ..
                // MeasureFormat.format requires an instance of MeasureUnit.
                "constructor-options-roundingMode-invalid.js",
                "constructor-options-throwing-getters-rounding-mode.js"));

    Set<String> testIssues =
        new HashSet<>(
            Arrays.asList(
                "proto-from-ctor-realm.js", // Hermes doesn't support realms
                "subclassing.js" // Hermes doesn't support classes. Compiling JS failed:
                // 18:1:invalid statement encountered. Buffer size 986 starts with:
                // 2f2f20436f7079726967687420323031
                ));

    Set<String> icuIssues = new HashSet<>();

    // Requires Android 10 (API level 29) ICU: 63.2 	CLDR: 34 	UniCode: 11.0
    // (https://developer.android.com/guide/topics/resources/internationalization)
    if (android.os.Build.VERSION.SDK_INT < 29) {
      icuIssues.addAll(
          Arrays.asList(
              "constructor-unitDisplay.js" // com.facebook.hermes.intl.JSRangeErrorException:
              // Unknown unit: percent
              ));
    }

    Set<String> skipList = testIssues;
    skipList.addAll(deviations);
    skipList.addAll(icuIssues);

    runTests(basePath, skipList);
  }

  @Test
  public void testIntlNumberFormat_prototype() throws IOException {
    String basePath = "test262/test/intl402/NumberFormat/prototype";
    runTests(basePath);
  }

  @Test
  public void testIntlNumberFormat_supportedLocalesOf() throws IOException {
    String basePath = "test262/test/intl402/NumberFormat/supportedLocalesOf";
    runTests(basePath);
  }

  @Test
  public void testIntlNumberFormat_prototype_constructor() throws IOException {
    String basePath = "test262/test/intl402/NumberFormat/prototype/constructor";
    runTests(basePath);
  }

  @Test
  public void testIntlNumberFormat_prototype_toStringTag() throws IOException {
    String basePath = "test262/test/intl402/NumberFormat/prototype/toStringTag";
    runTests(basePath);
  }

  @Test
  public void testIntlNumberFormat_prototype_resolvedOptions() throws IOException {

    String basePath = "test262/test/intl402/NumberFormat/prototype/resolvedOptions";

    Set<String> deviations =
        new HashSet<>(
            Arrays.asList(
                "order.js", // Expected SameValue(«Object», «Intl.NumberFormat») to be true ..
                // Test expects new Intl.NumberFormat().toString() to return "[object
                // Intl.NumberFormat]" which Firefox does.. but hermes (and Chrome)
                // returns "[object Object]:
                "roundingMode.js" // Not implemented
                ));

    Set<String> skipList = deviations;

    runTests(basePath, skipList);
  }

  @Test
  public void testIntlNumberFormat_prototype_format() throws IOException {

    String basePath = "test262/test/intl402/NumberFormat/prototype/format";

    // Our implementation doesn't support signDisplay as implementing with ICU APIs available in
    // Android before API 30 is very involved and tricky (Requires explicit manipulation of patterns
    // ) ..
    Set<String> signDisplayList =
        new HashSet<>(
            Arrays.asList(
                "signDisplay-currency-zh-TW.js", // Expected SameValue(«US$0.00», «+US$0.00») to be
                // true
                "signDisplay-rounding.js", // Expected SameValue(«-0», «0») to be true
                "signDisplay-currency-de-DE.js", // Expected SameValue(«0,00 $», «+0,00 $») to be
                // true
                "signDisplay-de-DE.js", // 0 (always) Expected SameValue(«0», «+0») to be true
                "signDisplay-ko-KR.js", // 0 (always) Expected SameValue(«0», «+0») to be true
                "signDisplay-currency-en-US.js", // Expected SameValue(«$0.00», «+$0.00») to be true
                "signDisplay-currency-ja-JP.js", // Expected SameValue(«$0.00», «+$0.00») to be true
                "signDisplay-zh-TW.js", //  -0.0001 (exceptZero) Expected SameValue(«-0», «0») to be
                // true
                "signDisplay-en-US.js", // 0 (always) Expected SameValue(«0», «+0») to be true
                "signDisplay-currency-ko-KR.js", // Expected SameValue(«US$0.00», «+US$0.00») to be
                // true
                "signDisplay-ja-JP.js" // 0 (always) Expected SameValue(«0», «+0») to be true
                ));

    Set<String> unitIssues =
        new HashSet<>(
            Arrays.asList(
                "units.js" // com.facebook.hermes.intl.JSRangeErrorException: Unknown unit:
                // acre-per-acre .. We support only units directly known to
                // https://developer.android.com/reference/android/icu/util/MeasureUnit
                // .. MeasureFormat.format requires an instance of MeasureUnit.
                ));

    Set<String> icuIssues = new HashSet<>();
    // Requires Android 11 (API level 30)
    if (android.os.Build.VERSION.SDK_INT < 30) {
      icuIssues.addAll(
          Arrays.asList(
              "engineering-scientific-de-DE.js", // Expected SameValue(«-∞E0», «-∞») to be true
              "engineering-scientific-ja-JP.js", // Expected SameValue(«-∞E0», «-∞») to be true
              "numbering-systems.js", // numberingSystem: diak, digit: 0 Expected SameValue(«0»,
              // «𑥐») to be true
              "engineering-scientific-ko-KR.js", // Expected SameValue(«-∞E0», «-∞») to be true
              "engineering-scientific-en-US.js" // Expected SameValue(«-∞E0», «-∞») to be true
              ));
    }

    // Requires Android 10 (API level 29) ICU: 63.2 	CLDR: 34 	UniCode: 11.0
    // (https://developer.android.com/guide/topics/resources/internationalization)
    if (android.os.Build.VERSION.SDK_INT < 29) {
      icuIssues.addAll(
          Arrays.asList(
              "format-significant-digits.js", // Formatted value for 0, en-US-u-nu-arab and options
              // {"useGrouping":false,"minimumSignificantDigits":3,"maximumSignificantDigits":5} is
              // ٠; expected ٠٫٠٠.
              "unit-en-US.js", // Expected SameValue(«-987 kph», «-987 km/h») to be true
              "notation-compact-de-DE.js", // Expected SameValue(«99 Tsd.», «98.765») to be true
              "format-significant-digits-precision.js" // Formatted value for 123.44500,
              // en-US-u-nu-arab and options
              // {"useGrouping":false,"minimumSignificantDigits":3,"maximumSignificantDigits":5} is
              // ١٢٣٫٤٤٥; expected ١٢٣٫٤٥.
              ));
    }

    // Requires Android 9 (API level 28) ICU: 60.2 	CLDR: 32.0.1 	UniCode: 10.0
    // (https://developer.android.com/guide/topics/resources/internationalization)
    if (android.os.Build.VERSION.SDK_INT < 28) {
      icuIssues.addAll(
          Arrays.asList(
              "notation-compact-ko-KR.js", // Expected SameValue(«9,900만», «9877만») to be true
              "notation-compact-en-US.js", // Expected SameValue(«990M», «988M») to be true
              "notation-compact-ja-JP.js" // Expected SameValue(«9,900万», «9877万») to be true
              ));
    }

    // ICU APIs not available prior to 24.
    Set<String> pre24Issues = new HashSet<>();
    if (android.os.Build.VERSION.SDK_INT < 24) {
      pre24Issues.addAll(
          Arrays.asList(
              "unit-ko-KR.js", // : Expected SameValue(«-987», «-987km/h») to be true
              "unit-de-DE.js", // : Expected SameValue(«-987», «-987 km/h») to be true
              "format-fraction-digits-precision.js", // : Unexpected formatted 1.1 for
              // en-US-u-nu-hanidec and options
              // {"useGrouping":false,"minimumIntegerDigits":3,"minimumFractionDigits":1,"maximumFractionDigits":3}: 〇〇〈.〈
              "percent-formatter.js", // : Intl.NumberFormat's formatting of 20% does not include a
              // formatting of 20 as a substring. Expected SameValue(«-1»,
              // «-1») to be false
              "format-fraction-digits.js", // : Unexpected formatted 1.1 for en-US-u-nu-hanidec and
              // options
              // {"useGrouping":false,"minimumIntegerDigits":3,"minimumFractionDigits":1,"maximumFractionDigits":3}: 〇〇〈.〈
              "unit-ja-JP.js" // : Expected SameValue(«-987», «-987 km/h») to be true
              ));
    }

    Set<String> roundingMode = new HashSet<>();
    roundingMode.addAll(
        Arrays.asList(
            "format-rounding-mode-trunc.js",
            "format-rounding-mode-ceil.js",
            "format-rounding-mode-floor.js",
            "format-rounding-mode-half-floor.js",
            "format-rounding-mode-half-even.js",
            "format-rounding-mode-half-trunc.js",
            "format-rounding-mode-half-ceil.js",
            "format-rounding-mode-expand.js"));

    // There seem to be significant gaps in the implementation for zh-TW, even
    // with the latest API.
    Set<String> zh_TW = new HashSet<>();
    zh_TW.addAll(
        Arrays.asList(
            "engineering-scientific-zh-TW.js", "unit-zh-TW.js", "notation-compact-zh-TW.js"));

    Set<String> skipList = new HashSet<>();
    skipList.addAll(signDisplayList);
    skipList.addAll(unitIssues);
    skipList.addAll(icuIssues);
    skipList.addAll(pre24Issues);
    skipList.addAll(roundingMode);
    skipList.addAll(zh_TW);

    runTests(basePath, skipList);
  }

  @Test
  public void testIntlNumberFormat_prototype_formatToParts() throws IOException {

    String basePath = "test262/test/intl402/NumberFormat/prototype/formatToParts";

    // Our implementation doesn't support signDisplay as implementing with ICU APIs available in
    // Android before API 30 is very involved and tricky (Requires explicit manipulation of patterns
    // ) ..
    Set<String> signDisplayList =
        new HashSet<>(
            Arrays.asList(
                "signDisplay-zh-TW.js", // NaN (auto): parts[0].value Expected SameValue(«NaN»,
                // «非數值») to be true
                "signDisplay-currency-de-DE.js", // undefined: length Expected SameValue(«5», «6»)
                // to be true
                "signDisplay-de-DE.js", // 0 (always): length Expected SameValue(«1», «2») to be
                // true
                "signDisplay-currency-ko-KR.js", // undefined: length Expected SameValue(«4», «5»)
                // to be true
                "signDisplay-ko-KR.js", // 0 (always): length Expected SameValue(«1», «2») to be
                // true
                "signDisplay-currency-en-US.js", // undefined: length Expected SameValue(«4», «5»)
                // to be true
                "signDisplay-currency-ja-JP.js", // undefined: length Expected SameValue(«4», «5»)
                // to be true//"unit-zh-TW.js", //undefined: length
                // Expected SameValue(«1», «4») to be true
                "signDisplay-ja-JP.js", // 0 (always): length Expected SameValue(«1», «2») to be
                // true
                "signDisplay-currency-zh-TW.js", // undefined: length Expected SameValue(«4», «5»)
                // to be true
                "signDisplay-en-US.js" // 0 (always): length Expected SameValue(«1», «2») to be true
                ));

    // https://developer.android.com/reference/android/icu/text/MeasureFormat doesn't implement
    // "formatToCharacterIterator" method. It always returns the whole formatted text as a single
    // literal.
    Set<String> unitList =
        new HashSet<>(
            Arrays.asList(
                "unit.js", // Expected SameValue(«false», «true») to be true
                "unit-ja-JP.js", // undefined: length Expected SameValue(«1», «4») to be true
                "unit-ko-KR.js", // undefined: length Expected SameValue(«1», «3») to be true
                "unit-de-DE.js", // undefined: length Expected SameValue(«1», «4») to be true
                "unit-en-US.js", // undefined: length Expected SameValue(«1», «4») to be true
                "unit-zh-TW.js", // undefined: length Expected SameValue(«1», «4») to be true
                "percent-en-US.js" // unit: length Expected SameValue(«1», «3») to be true .. We
                // have issue when formatting percentage as unit .. i.e. {style:
                // "unit", unit: "percent"} .. We could hack to create a
                // formatter in percent style, but it's tricky to deal with the
                // x100 multiplier.
                ));

    Set<String> icuIssues = new HashSet<>();
    // Requires Android 11 (API level 30)
    if (android.os.Build.VERSION.SDK_INT < 30) {
      icuIssues.addAll(
          Arrays.asList(
              "engineering-scientific-ko-KR.js", // -Infinity - engineering: length Expected
              // SameValue(«4», «2») to be true
              "notation-compact-zh-TW.js", // Compact short: 987654321: parts[3].type Expected
              // SameValue(«literal», «compact») to be true
              "engineering-scientific-en-US.js", // -Infinity - engineering: length Expected
              // SameValue(«4», «2») to be true
              "notation-compact-de-DE.js", // Compact short: 987654321: length Expected
              // SameValue(«2», «3») to be true
              "engineering-scientific-de-DE.js", // -Infinity - engineering: length Expected
              // SameValue(«4», «2») to be true
              "notation-compact-en-US.js", // Compact short: 987654321: parts[1].type Expected
              // SameValue(«literal», «compact») to be true
              "engineering-scientific-ja-JP.js", // -Infinity - engineering: length Expected
              // SameValue(«4», «2») to be true
              "notation-compact-ja-JP.js", // Compact short: 987654321: parts[3].type Expected
              // SameValue(«literal», «compact») to be true
              "notation-compact-ko-KR.js" // Compact short: 987654321: parts[3].type Expected
              // SameValue(«literal», «compact») to be true
              ));
    }

    // ICU APIs not available prior to 24.
    Set<String> pre24Issues = new HashSet<>();
    if (android.os.Build.VERSION.SDK_INT < 24) {
      pre24Issues.addAll(
          Arrays.asList(
              "default-parameter.js" // : Both implicit and explicit calls should have the correct
              // result
              ));
    }

    Set<String> zh_TW = new HashSet<>();
    zh_TW.addAll(Arrays.asList("engineering-scientific-zh-TW.js", "notation-compact-zh-TW.js"));

    Set<String> skipList = new HashSet<>();

    skipList.addAll(signDisplayList);
    skipList.addAll(unitList);
    skipList.addAll(icuIssues);
    skipList.addAll(pre24Issues);
    skipList.addAll(zh_TW);

    runTests(basePath, skipList);
  }
}
