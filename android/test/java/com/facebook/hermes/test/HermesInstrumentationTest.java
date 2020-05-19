/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.test;

import static org.fest.assertions.api.Assertions.assertThat;

import android.test.InstrumentationTestCase;
import java.util.Locale;
import java.util.TimeZone;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.junit.Test;

public class HermesInstrumentationTest extends InstrumentationTestCase {

  @Test
  public void testEvaluatingJavaScript() {
    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      rt.evaluateJavaScript("x = 1;");
    }
  }

  @Test
  public void testLocaleCompare() {
    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      /* Reenable this once Intl is capable enough */
      rt.evaluateJavaScript("intlType = typeof Intl");
      if (rt.getGlobalStringProperty("intlType").equals("object")) {
        return;
      }

      rt.evaluateJavaScript("compareResult1 = 'a'.localeCompare('a');");
      rt.evaluateJavaScript("compareResult2 = 'a'.localeCompare('b');");
      rt.evaluateJavaScript("compareResult3 = 'a'.localeCompare('A');");
      rt.evaluateJavaScript("compareResult4 = 'b'.localeCompare('A');");
      rt.evaluateJavaScript("compareResult5 = '\u00FC'.localeCompare('\u00FC');");
      rt.evaluateJavaScript("compareResult6 = '\u00FC'.localeCompare('u');");

      int result1 = rt.getGlobalNumberProperty("compareResult1");
      assertThat(result1).isEqualTo(0);
      int result2 = rt.getGlobalNumberProperty("compareResult2");
      assertThat(result2).isEqualTo(-1);
      int result3 = rt.getGlobalNumberProperty("compareResult3");
      assertThat(result3).isEqualTo(-1);
      int result4 = rt.getGlobalNumberProperty("compareResult4");
      assertThat(result4).isEqualTo(1);
      int result5 = rt.getGlobalNumberProperty("compareResult5");
      assertThat(result5).isEqualTo(0);
      int result6 = rt.getGlobalNumberProperty("compareResult6");
      assertThat(result6).isEqualTo(1);
    }
  }

  @Test
  public void testDateFormat() {
    Locale defaultLocale = Locale.getDefault();
    Locale.setDefault(new Locale("en-US"));

    TimeZone defaultZone = TimeZone.getDefault();
    TimeZone.setDefault(TimeZone.getTimeZone("UTC"));

    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      /* Reenable this once Intl is capable enough */
      rt.evaluateJavaScript("intlType = typeof Intl");
      if (rt.getGlobalStringProperty("intlType").equals("object")) {
        return;
      }

      rt.evaluateJavaScript(
          new StringBuilder()
              .append("var date = new Date(Date.UTC(2012, 11, 20, 3, 0, 0));\n")
              .append("d1 = date.toLocaleDateString();\n")
              .append("d2 = date.toLocaleTimeString();\n")
              .append("d3 = date.toLocaleString();\n")
              .toString());

      String result1 = rt.getGlobalStringProperty("d1");
      assertThat(result1).isEqualTo("Dec 20, 2012");
      String result2 = rt.getGlobalStringProperty("d2");
      assertThat(result2).isEqualTo("3:00:00 AM");
      String result3 = rt.getGlobalStringProperty("d3");
      assertThat(result3).isEqualTo("Dec 20, 2012 3:00:00 AM");
    } finally {
      Locale.setDefault(defaultLocale);
      TimeZone.setDefault(defaultZone);
    }
  }

  @Test
  public void testCaseConversion() {
    Locale defaultLocale = Locale.getDefault();
    Locale.setDefault(new Locale("en-US"));

    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      rt.evaluateJavaScript(
          new StringBuilder()
              .append("var s1 = 'abc'.toUpperCase();")
              .append("var s2 = '\u00E0'.toUpperCase();")
              .toString());
      String result1 = rt.getGlobalStringProperty("s1");
      assertThat(result1).isEqualTo("ABC");
      String result2 = rt.getGlobalStringProperty("s2");
      assertThat(result2).isEqualTo("\u00C0");
    }
  }

  @Test
  public void testNormalize() {
    Locale defaultLocale = Locale.getDefault();
    Locale.setDefault(new Locale("en-US"));

    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      rt.evaluateJavaScript(
          new StringBuilder()
              .append("var s1 = '\u1E9B\u0323'.normalize('NFC');")
              .append("var s2 = '\u1E9B\u0323'.normalize('NFD');")
              .append("var s3 = '\u1E9B\u0323'.normalize('NFKC');")
              .append("var s4 = '\u1E9B\u0323'.normalize('NFKD');")
              .toString());
      String result1 = rt.getGlobalStringProperty("s1");
      String result2 = rt.getGlobalStringProperty("s2");
      String result3 = rt.getGlobalStringProperty("s3");
      String result4 = rt.getGlobalStringProperty("s4");
      assertThat(result1).isEqualTo("\u1E9B\u0323");
      assertThat(result2).isEqualTo("\u017F\u0323\u0307");
      assertThat(result3).isEqualTo("\u1E69");
      assertThat(result4).isEqualTo("\u0073\u0323\u0307");
    }
  }

  // Gets the recorded GC stats from the runtime, finds the reported heap size,
  // parses is a long, and returns that.
  private long getHeapSize(JSRuntime rt) {
    String pattern = "\"Heap size\": ([0-9]+),";
    Pattern r = Pattern.compile(pattern, Pattern.MULTILINE);
    Matcher m = r.matcher(rt.getRecordedGCStats());
    assertThat(m.find()).isTrue();
    return Long.parseLong(m.group(1));
  }

  @Test
  public void testCreateRuntimeWithHeapSpec() {
    long M = 1024 * 1024;
    // The current default initial heap size is 32M.  Verify that.
    // In doing such verifications, some heaps, especially NCGen, may round
    // the requested size up to meet various constraints -- but never by as much
    // as 8MB (the size of an NCGen segment).
    try (JSRuntime rt = JSRuntime.makeHermesRuntime(/* shouldRecordGCStats */ true)) {
      assertThat(getHeapSize(rt)).isGreaterThanOrEqualTo(32 * M);
      assertThat(getHeapSize(rt)).isLessThan(32 * M + 8 * M);
    }

    // Now show that we can make runtime with a smaller heap with the alternate
    // factory function:
    try (JSRuntime rt =
        JSRuntime.makeHermesRuntimeWithHeapSpec(8 * M, 8 * M, /* shouldRecordGCStats */ true)) {
      assertThat(getHeapSize(rt)).isGreaterThanOrEqualTo(8 * M);
      assertThat(getHeapSize(rt)).isLessThan(8 * M + 8 * M);
    }
  }

  @Test
  public void testGetHermesEpilogue() {
    final String EXPECTED_EPILOGUE = "{\"foo\" : \"bar\"}\n";
    byte[] s = HermesEpilogue.getHermesBytecodeMetadata(HermesEpilogueTestData.getBytecode());
    assertThat(s.length).isZero();

    byte[] epilogue =
        HermesEpilogue.getHermesBytecodeMetadata(
            HermesEpilogueTestData.getBytecodeWithEpilogue(EXPECTED_EPILOGUE));
    assertEquals(EXPECTED_EPILOGUE, new String(epilogue));
  }
}
