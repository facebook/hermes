/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Wno-direct-eval -non-strict %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -Wno-direct-eval -non-strict -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Verify property access on transient objects.

print("transient-obj-props");
//CHECK-LABEL: transient-obj-props

var xname = "x";

Object.defineProperty(String.prototype, "x", {
    configurable: true,
    get: function() { print("get", typeof(this)); },
    set: function() { print("set", typeof(this)); }
});
'asdf'.x;
//CHECK-NEXT: get object
'asdf'[eval("xname")];
//CHECK-NEXT: get object
'asdf'.x = 10;
//CHECK-NEXT: set object
'asdf'[eval("xname")] = 20;
//CHECK-NEXT: set object

Object.defineProperty(String.prototype, "x", {
    configurable: true,
    get: function() { "use strict"; print("get", typeof(this)); },
    set: function() { "use strict"; print("set", typeof(this)); }
});
'asdf'.x;
//CHECK-NEXT: get string
'asdf'[eval("xname")];
//CHECK-NEXT: get string
'asdf'.x = 10;
//CHECK-NEXT: set string
'asdf'[eval("xname")] = 20;
//CHECK-NEXT: set string
