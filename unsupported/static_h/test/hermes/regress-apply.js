/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s

// Check that Function.prototype.apply() doesn't crash because it
// forgot to initialize the argument registers before a GC.
// Before the fix this would crash with the following assertion:
//
// Assertion failed: (!hv.isInvalid() && "HermesValue with InvalidTag encountered by GC."),
//   function accept, file hermes/lib/VM/CheckHeapWellFormedAcceptor.cpp, line 22.

function foo(a) {
    print(a);
}

// A "fake" array, which will cause apply() to call getComputed().
var p = {
    // Pretend there are 10 arguments.
    length: 10,
    // Force a GC when returning argument 0.
    get 0 () {
        gc();
        return 10;
    }
}

foo.apply(undefined, p);
