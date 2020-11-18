/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.test;

import android.content.res.AssetManager;
import android.test.InstrumentationTestCase;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.stream.Collectors;
import org.junit.Test;

public class HermesIntlAndroidTest extends InstrumentationTestCase {
  @Test
  public void testIntlFromAsset() throws IOException {
    AssetManager assets = getInstrumentation().getContext().getAssets();
    InputStream is = assets.open("intl.js");
    String script =
        new BufferedReader(new InputStreamReader(is)).lines().collect(Collectors.joining("\n"));
    try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
      rt.evaluateJavaScript(script);
    }
  }
}
