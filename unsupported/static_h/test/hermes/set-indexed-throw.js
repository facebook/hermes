/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// Check that exceptions in indexed setters are propagated.
"use strict";

var a = new Int8Array(4);
var o = {
    valueOf: function() { throw Error("Surprise!"); }
}

try {
a[0] = o;
} catch(e) {
    print("caught", e.name, e.message);
}
//CHECK: caught Error Surprise!
