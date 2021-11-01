/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import java.io.IOException;
import java.util.HashSet;
import java.util.Set;
import org.junit.Test;

// Run "./gradlew :intltest:prepareTests" from the root to copy the test files to the
// APK assets.
public class HermesIntlStringPrototypeTest extends HermesIntlTest262Base {

  private static final String LOG_TAG = "HermesIntlStringPrototypeTest";

  @Test
  public void testIntlStringToLocaleLowerCase() throws IOException {

    String basePath = "test262/test/intl402/String/prototype/toLocaleLowerCase";
    Set<String> whilteList = new HashSet<>();
    Set<String> blackList = new HashSet<>();

    runTests(basePath, blackList, whilteList);
  }

  @Test
  public void testIntlStringToLocaleUpperCase() throws IOException {

    String basePath = "test262/test/intl402/String/prototype/toLocaleUpperCase";
    Set<String> whilteList = new HashSet<>();
    Set<String> blackList = new HashSet<>();

    runTests(basePath, blackList, whilteList);
  }

  @Test
  public void testIntlStringLocaleCompare() throws IOException {

    String basePath = "test262/test/intl402/String/prototype/localeCompare";
    Set<String> whilteList = new HashSet<>();
    Set<String> blackList = new HashSet<>();

    runTests(basePath, blackList, whilteList);
  }
}
