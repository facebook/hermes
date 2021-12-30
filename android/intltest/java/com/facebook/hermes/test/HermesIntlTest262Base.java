/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import static org.fest.assertions.api.Assertions.assertThat;

import android.content.res.AssetManager;
import android.test.InstrumentationTestCase;
import android.text.TextUtils;
import android.util.Log;
import com.facebook.hermes.test.JSRuntime;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

// Run "./gradlew :intltest:prepareTests" from the root to copy the test files to the
// APK assets.
public class HermesIntlTest262Base extends InstrumentationTestCase {

  private static final String LOG_TAG = "HermesIntlTest";

  protected void evalScriptFromAsset(JSRuntime rt, String filename) throws IOException {
    AssetManager assets = getInstrumentation().getContext().getAssets();
    InputStream is = assets.open(filename);

    BufferedReader r = new BufferedReader(new InputStreamReader(is));
    StringBuilder total = new StringBuilder();
    for (String line; (line = r.readLine()) != null; ) {
      total.append(line).append('\n');
    }

    String script = total.toString();

    rt.evaluateJavaScript(script);
  }

  protected void evaluateCommonScriptsFromAsset(JSRuntime rt) throws IOException {
    evalScriptFromAsset(rt, "test262/harness/sta.js");
    evalScriptFromAsset(rt, "test262/harness/assert.js");
    evalScriptFromAsset(rt, "test262/harness/testIntl.js");
    evalScriptFromAsset(rt, "test262/harness/propertyHelper.js");
    evalScriptFromAsset(rt, "test262/harness/compareArray.js");
    evalScriptFromAsset(rt, "test262/harness/dateConstants.js");
    evalScriptFromAsset(rt, "test262/harness/isConstructor.js");
  }

  protected void runTests(String basePath) throws IOException {
    runTests(basePath, new HashSet<String>());
  }

  protected void runTests(String basePath, Set<String> skipList) throws IOException {

    List<String> testFileList =
        Arrays.asList(getInstrumentation().getContext().getAssets().list(basePath));

    ArrayList<String> ranTests = new ArrayList<>();
    HashMap<String, String> failedTests = new HashMap<>();

    if (testFileList.isEmpty()) {
      Log.e(LOG_TAG, "No test files found at : " + basePath);
      assertThat(!testFileList.isEmpty()).isEqualTo(true);
    }

    for (String testFileName : testFileList) {
      String testFilePath = basePath + "/" + testFileName;
      Log.d(LOG_TAG, "Evaluating " + testFilePath);

      try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
        evaluateCommonScriptsFromAsset(rt);
        try {
          evalScriptFromAsset(rt, testFilePath);
          ranTests.add(testFileName);
        } catch (FileNotFoundException ex) {
          if (testFilePath.endsWith(".js")) throw ex;
          // Skip, they are likely subdirectories or non-javascript files.
        } catch (com.facebook.jni.CppException ex) {
          if (!skipList.contains(testFileName)) failedTests.put(testFilePath, ex.getMessage());
        } catch (Exception ex) {
          if (!skipList.contains(testFileName)) failedTests.put(testFilePath, ex.getMessage());
        }
      }
    }

    Log.v(LOG_TAG, "Passed Tests: " + TextUtils.join("\n", ranTests));

    for (Map.Entry<String, String> entry : failedTests.entrySet()) {
      Log.v(LOG_TAG, "Failed Tests: " + entry.getKey() + " : " + entry.getValue());
    }

    assertThat(failedTests.entrySet().isEmpty()).isEqualTo(true);
  }
}
