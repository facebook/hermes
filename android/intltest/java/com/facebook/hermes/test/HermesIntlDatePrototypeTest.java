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
public class HermesIntlDatePrototypeTest extends HermesIntlTest262Base {

  private static final String LOG_TAG = "HermesIntlDatePrototypeTest";

  @Test
  public void testIntlDatePrototype() throws IOException {

    String basePath = "test262/test/intl402/Date/prototype";
    Set<String> whilteList = new HashSet<>();
    Set<String> blackList = new HashSet<>();

    runTests(basePath, blackList, whilteList);
  }

  @Test
  public void testIntlDatePrototypeToLocaleString() throws IOException {

    String basePath = "test262/test/intl402/Date/prototype/toLocaleString";
    Set<String> whilteList = new HashSet<>();
    Set<String> blackList = new HashSet<>();

    runTests(basePath, blackList, whilteList);
  }

  @Test
  public void testIntlDatePrototypeToLocaleDateString() throws IOException {

    String basePath = "test262/test/intl402/Date/prototype/toLocaleDateString";
    Set<String> whilteList = new HashSet<>();
    Set<String> blackList = new HashSet<>();

    runTests(basePath, blackList, whilteList);
  }

  @Test
  public void testIntlDatePrototypeToLocaleTimeString() throws IOException {

    String basePath = "test262/test/intl402/Date/prototype/toLocaleTimeString";
    Set<String> whilteList = new HashSet<>();
    Set<String> blackList = new HashSet<>();

    runTests(basePath, blackList, whilteList);
  }
}
