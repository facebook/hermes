/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -gc-max-heap=8M %s | %FileCheck --match-full-lines %s

print('ArrayBuffer')
// CHECK-LABEL: ArrayBuffer

// Smaller than max heap succeeds.
a = new ArrayBuffer(1000000);

// Even if done multiple times; GC should collect unused ones.
a = new ArrayBuffer(1000000);
a = new ArrayBuffer(1000000);
a = new ArrayBuffer(1000000);
a = new ArrayBuffer(1000000);

a = null;

// Larger than max heap fails
try {
    a = new ArrayBuffer(12000000);
} catch (x) {
    print(x)
    // CHECK-NEXT: RangeError: Cannot allocate a data block for the ArrayBuffer
}
a = null;

print('ExternalStringPrimitive')
// CHECK-LABEL: ExternalStringPrimitive

// Note: use Array.join() instead of string concatenation to avoid the relative
// unpredictability of fast buffered concatenation.
var s10 = 'aaaaaaaaaa';
var s100 = [s10, s10, s10, s10, s10, s10, s10, s10, s10, s10].join("")

function strOfSize(n) {
    if (n < 100) {
        return s100.substring(0, n);
    } else {
        var leftSize = Math.floor(n / 2);
        var left = strOfSize(leftSize);
        // Don't allocate a string for rightSize -- that would mean exponential work,
        // and live memory proportional to n before the final allocation.
        if (leftSize * 2 == n) {
            return [left, left].join("");
        } else {
            // n == 2 * leftSize + 1:
            return [left, left, 'a'].join("");
        }
    }
}

// Smaller than max heap succeeds.
var s = strOfSize(1000000);

// Even if done multiple times; GC should collect unused ones.
s = strOfSize(1000000);
s = strOfSize(1000000);
s = strOfSize(1000000);
s = strOfSize(1000000);
s = null;

// Larger than max heap fails.
try {
    s = strOfSize(6000000);
    print('no exception');
} catch (x) {
    print(x)
    // CHECK-NEXT: RangeError: Cannot allocate an external string primitive.
}
