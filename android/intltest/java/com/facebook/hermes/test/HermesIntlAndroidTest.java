/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.test;

import static org.assertj.core.api.Java6Assertions.assertThat;

import android.content.res.AssetManager;
import android.os.Build;
import androidx.test.platform.app.InstrumentationRegistry;
import androidx.test.runner.AndroidJUnit4;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.stream.Collectors;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class HermesIntlAndroidTest {

  @Test
  public void testIntlFromAsset() throws IOException {
    AssetManager assets =
        InstrumentationRegistry.getInstrumentation().getTargetContext().getAssets();
    InputStream is = assets.open("intl.js");
    String script =
        new BufferedReader(new InputStreamReader(is)).lines().collect(Collectors.joining("\n"));
    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      rt.evaluateJavaScript(script);
    }
  }

  @Test
  public void testNumberFormatFractionDigitsFromAsset() throws IOException {
    AssetManager assets =
        InstrumentationRegistry.getInstrumentation().getTargetContext().getAssets();
    InputStream is = assets.open("number-format-fraction-digits.js");
    String script =
        new BufferedReader(new InputStreamReader(is)).lines().collect(Collectors.joining("\n"));
    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      rt.evaluateJavaScript(script);
    }
  }

  @Test
  public void testNumberFormatSignificantDigitsFromAsset() throws IOException {
    AssetManager assets =
        InstrumentationRegistry.getInstrumentation().getTargetContext().getAssets();
    InputStream is = assets.open("number-format-significant-digits.js");
    String script =
        new BufferedReader(new InputStreamReader(is)).lines().collect(Collectors.joining("\n"));
    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      rt.evaluateJavaScript(script);
    }
  }

  @Test
  public void testDateTimeFormat() {
    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      rt.evaluateJavaScript(
          new StringBuilder()
              .append("var date = new Date('2021-08-13T14:00:00Z');\n")
              .append("var formattedDate = Intl.DateTimeFormat('en-US', {\n")
              .append("timeZone: 'America/New_York',\n")
              .append("day: 'numeric',\n")
              .append("month: 'numeric',\n")
              .append("hour: 'numeric',\n")
              .append("minute: 'numeric'\n")
              .append("}).format(date);\n")
              .toString());

      String result = rt.getGlobalStringProperty("formattedDate");
      assertThat(result).isEqualTo("8/13, 10:00 AM");
    }
  }

  @Test
  public void testDateTimeFormatCaseInsensitivity() {
    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      rt.evaluateJavaScript(
          new StringBuilder()
              .append("var date = new Date('2021-09-24T22:00:00Z');\n")
              .append("var formattedDate = Intl.DateTimeFormat('en-US', {\n")
              .append("timeZone: 'AmeRiCa/new_YORK',\n")
              .append("day: 'numeric',\n")
              .append("month: 'numeric',\n")
              .append("hour: 'numeric',\n")
              .append("minute: 'numeric'\n")
              .append("}).format(date);\n")
              .toString());

      String result = rt.getGlobalStringProperty("formattedDate");
      assertThat(result).isEqualTo("9/24, 6:00 PM");
    }
  }

  @Test
  public void testSignDisplayAlwaysForApiLevelAbove31() {
    if (Build.VERSION.SDK_INT >= 31) {
      try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
        rt.evaluateJavaScript(
            "var nf = new Intl.NumberFormat('de-DE', { signDisplay: 'always', currency: 'EUR',"
                + " style: 'currency' });\n"
                + "var result = nf.format(8537.71);");

        String result = rt.getGlobalStringProperty("result");
        assertThat(result).isEqualTo("+8.537,71 €");
      }
    }
  }

  @Test
  public void testSignDisplayNeverForApiLevelAbove31() {
    if (Build.VERSION.SDK_INT >= 31) {
      try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
        rt.evaluateJavaScript(
            "var nf = new Intl.NumberFormat('de-DE', { signDisplay: 'never', currency: 'EUR',"
                + " style: 'currency' });\n"
                + "var result = nf.format(8537.71);");

        String result = rt.getGlobalStringProperty("result");
        assertThat(result).isEqualTo("8.537,71 €");
      }
    }
  }

  @Test
  public void testSignDisplayExceptZeroForApiLevelAbove31() {
    if (Build.VERSION.SDK_INT >= 31) {
      try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
        rt.evaluateJavaScript(
            "var nf = new Intl.NumberFormat('de-DE', { signDisplay: 'exceptZero', currency: 'EUR',"
                + " style: 'currency'});\n"
                + "var result = nf.format(8537.71);");

        String result = rt.getGlobalStringProperty("result");
        assertThat(result).isEqualTo("+8.537,71 €");
      }
    }
  }
}
