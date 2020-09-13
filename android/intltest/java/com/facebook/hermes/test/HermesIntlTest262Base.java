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
import java.util.List;
import java.util.stream.Collectors;

public class HermesIntlTest262Base extends InstrumentationTestCase {

    private static final String LOG_TAG = "HermesIntlNumberFormatTest";

    protected void evalScriptFromAsset(JSRuntime rt, String filename) throws IOException {
        AssetManager assets = getInstrumentation().getContext().getAssets();
        InputStream is = assets.open(filename);

//        String script = new BufferedReader(new InputStreamReader(is))
//                .lines().collect(Collectors.joining("\n"));

        BufferedReader r = new BufferedReader(new InputStreamReader(is));
        StringBuilder total = new StringBuilder();
        for (String line; (line = r.readLine()) != null; ) {
            total.append(line).append('\n');
        }

        String script = total.toString();

        rt.evaluateJavaScript(script);

    }

    protected void evaluateCommonScriptsFromAsset(JSRuntime rt) throws IOException {
        evalScriptFromAsset(rt, "test262-main/harness/sta.js");
        evalScriptFromAsset(rt, "test262-main/harness/assert.js");
        evalScriptFromAsset(rt, "test262-main/harness/testIntl.js");
        evalScriptFromAsset(rt, "test262-main/harness/propertyHelper.js");
        evalScriptFromAsset(rt, "test262-main/harness/compareArray.js");
        // evalScriptFromAsset(rt, "test262/intl/common/testintl.js");

    }

    protected void runTests(String basePath, Set<String> blackList, Set<String> whiteList) throws IOException {

        List<String> testFileList;
        if(whiteList.isEmpty()) {
            testFileList = Arrays.asList(getInstrumentation().getContext().getAssets().list(basePath));
        } else {
            testFileList= new ArrayList<String>();
            testFileList.addAll(whiteList);
        }

        ArrayList<String> ranTests = new ArrayList<>();
        HashMap<String, String> failedTests = new HashMap<>();

        if(testFileList.isEmpty()) {
            Log.e(LOG_TAG, "No test files found at : " + basePath);
            assertThat(!testFileList.isEmpty()).isEqualTo(true);
        }

        for (String testFileName : testFileList) {
            String testFilePath = basePath + testFileName;
            Log.d(LOG_TAG, "Evaluating " + testFilePath);

            try (JSRuntime rt = JSRuntime.makeHermesRuntime()) {
                evaluateCommonScriptsFromAsset(rt);
                try {
                    evalScriptFromAsset(rt, testFilePath);
                    ranTests.add(testFileName);
                } catch (FileNotFoundException ex) {
                    if (testFilePath.endsWith(".js"))
                        throw ex;
                    // Skip, they are likely subdirectories or non-javascript files.
                } catch (com.facebook.jni.CppException ex) {
                    if (!blackList.contains(testFileName))
                        failedTests.put(testFilePath, ex.getMessage());
                } catch (Exception ex) {
                    if (!blackList.contains(testFileName))
                        failedTests.put(testFilePath, ex.getMessage());
                }
            }
        }

        Log.v(LOG_TAG, "Passed Tests: " + TextUtils.join("\n", ranTests));

        for (Map.Entry<String, String> entry : failedTests.entrySet()) {
            Log.v(LOG_TAG, "Failed Tests: " + entry.getKey() + " : " + entry.getValue());

        }

        assertThat(failedTests.entrySet().isEmpty()).isEqualTo(true);

        if(ranTests.isEmpty()) {
            Log.e(LOG_TAG, "No test was successfully executed at : " + basePath);
            assertThat(!ranTests.isEmpty()).isEqualTo(true);
        }

    }
}