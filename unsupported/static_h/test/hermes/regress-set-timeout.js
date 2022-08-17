/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck %s

function foo() {}
Object.defineProperty(foo, "length", {get: function(){ throw "oops"; }});
try {
    setTimeout(foo);
} catch (e) {
    print("did not crash");
}
//CHECK: did not crash
