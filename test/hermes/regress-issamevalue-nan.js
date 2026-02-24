/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Test that different NaN representations are considered equal by isSameValue.
function foo(){
    // This will get optimized to the NaN constant used by the compiler.
    var staticNan = 0/0;
    globalThis.zero = 0;
    // This will produce the default NaN on the platform.
    var dynamicNan = 0/globalThis.zero;
    print([staticNan].includes(dynamicNan));
}

foo();

//CHECK: true
