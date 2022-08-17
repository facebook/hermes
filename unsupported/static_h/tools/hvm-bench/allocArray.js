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
        x0 = [];
        x1 = [];
        x2 = [];
        x3 = [];
        x4 = [];
        x5 = [];
        x6 = [];
        x7 = [];
        x8 = [];
        x9 = [];
    }
    // We must prevent the Hermes (or other) compiler from optimizing away the allocations
    // in the loop above.  So we use the most recent objects allocated, in a way where we
    // return the result.  (A compiler *could* be smart enough to realize that only the last
    // loop iteration is needed, but Hermes, at least, is not that smart today.)
    x0.push(0);
    x1.push(1);
    x2.push(2);
    x3.push(3);
    x4.push(4);
    x5.push(5);
    x6.push(6);
    x7.push(7);
    x8.push(8);
    x9.push(9);
    return x0[0] + x1[0] + x2[0] + x3[0] + x4[0] +
        x5[0] + x6[0] + x7[0] + x8[0] + x9[0];
}

function allocNTimes(n) {
    var sum = 0;
    for (var i = 0; i < n; i++) {
        sum += doAlloc()
    }
    return sum;
}

print(allocNTimes(500));
