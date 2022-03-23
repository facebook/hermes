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
public class HermesIntlArrayPrototypeTest extends HermesIntlTest262Base {

  @Test
  public void testIntlArrayToLocaleString() throws IOException {

    String basePath = "test262/test/intl402/Array/prototype/toLocaleString";

    Set<String> skipList =
        new HashSet<>(
            Arrays.asList(
                "throws-same-exceptions-as-NumberFormat.js" // Number.prototype.toLocaleString
                // didn't throw exception for options
                // {"style":"currency"}. Expected a
                // TypeError but got a RangeError .. We
                // have the same bug in NumberFormat as
                // well.. This is because we don't throw
                // TypeError from java code yet.
                ));

    runTests(basePath, skipList);
  }
}
