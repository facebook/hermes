/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import java.io.IOException;
import org.junit.Test;

// Run "./gradlew :intltest:prepareTests" from the root to copy the test files to the
// APK assets.
public class HermesIntlStringPrototypeTest extends HermesIntlTest262Base {

  @Test
  public void testIntlStringToLocaleLowerCase() throws IOException {
    String basePath = "test262/test/intl402/String/prototype/toLocaleLowerCase";
    runTests(basePath);
  }

  @Test
  public void testIntlStringToLocaleUpperCase() throws IOException {
    String basePath = "test262/test/intl402/String/prototype/toLocaleUpperCase";
    runTests(basePath);
  }

  @Test
  public void testIntlStringLocaleCompare() throws IOException {
    String basePath = "test262/test/intl402/String/prototype/localeCompare";
    runTests(basePath);
  }
}
