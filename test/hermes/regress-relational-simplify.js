/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -exec -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -exec -O %s | %FileCheck --match-full-lines %s

// Verify that relational comparisons between an identical value that could
// be converted to NaN are not optimized away.

// The try/catch is needed to force the optimizer to introduce a Phi between
// the initializing "undefined" and another value. Without it, "v" is just
// replaced with a literal.

// A function to convince the compiler that the try/catch is necessary.
function maybeThrow() {
    if (globalThis.XXX)
        throw new Error();
}

function le() {
    try { maybeThrow(); } catch { var v = ""; }
    return v <= v;
}
function ge() {
    try { maybeThrow(); } catch { var v = ""; }
    return v <= v;
}
// These two weren't broken, but including for completeness.
function lt() {
    try { maybeThrow(); } catch { var v = ""; }
    return v < v;
}
function gt() {
    try { maybeThrow(); } catch { var v = ""; }
    return v > v;
}

print(le(), ge(), lt(), gt());
// CHECK: false false false false
