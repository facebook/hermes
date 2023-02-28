/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import './index.js';

const printDiff = false;

if (printDiff) {
    print(JSON.stringify(runFullTest(true)));
} else {
    print("Running...");
    // Warmup.
    var i;
    for(i = 0; i < 50; ++i)
      runFullTest(false);
    // The actual execution.
    let t1 = Date.now();
    for(i = 0; i < 5000; ++i)
      runFullTest(false);
    print(Date.now() - t1, "ms", i, "iterations");
}

