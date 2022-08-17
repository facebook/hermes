/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function doAlloc() {
    var x0 = null;
    var x1 = null;
    var x2 = null;
    var x3 = null;
    var x4 = null;
    var x5 = null;
    var x6 = null;
    var x7 = null;
    var x8 = null;
    var x9 = null;
    for (var i = 0; i < 100000; i++) {
        x0 = new Object();
        x1 = new Object();
        x2 = new Object();
        x3 = new Object();
        x4 = new Object();
        x5 = new Object();
        x6 = new Object();
        x7 = new Object();
        x8 = new Object();
        x9 = new Object();
    }
    // We must prevent the Hermes (or other) compiler from optimizing away the allocations
    // in the loop above.  So we use the most recent objects allocated, in a way where we
    // return the result.  (A compiler *could* be smart enough to realize that only the last
    // loop iteration is needed, but Hermes, at least, is not that smart today.)
    x0.p = 0;
    x1.p = 1;
    x2.p = 2;
    x3.p = 3;
    x4.p = 4;
    x5.p = 5;
    x6.p = 6;
    x7.p = 7;
    x8.p = 8;
    x9.p = 9;
    return x0.p + x1.p + x2.p + x3.p + x4.p +
        x5.p + x6.p + x7.p + x8.p + x9.p;
}

function allocNTimes(n) {
    var sum = 0;
    for (var i = 0; i < n; i++) {
        sum += doAlloc()
    }
    return sum;
}

print(allocNTimes(500));

