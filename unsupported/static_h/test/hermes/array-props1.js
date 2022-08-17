/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -non-strict -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Make sure that we can redefine a index property.
var a = [];
a[0] = 1;
var acc = {};
acc.get = function() {return 'hello'};
Object.defineProperty(a, 0, acc);
print(a[0]);
//CHECK: hello

var a = [1];
Object.defineProperty(a, "5", {configurable:true, value:'prop5'});
print(a.length, a, a[5]);
//CHECK-NEXT: 6 1,,,,,prop5 prop5

// Make sure that setting '.length' deletes "special" properties.
a.length=3;
print(a.length, a, a[5]);
//CHECK-NEXT: 3 1,, undefined

// Make sure that setting '.length' fails if a prorperty can't be deleted.
Object.defineProperty(a, "2", {configurable:true, value:'prop2'});
Object.defineProperty(a, "1", {value:'prop1'});
a.length=1;
print(a.length, a, a[1], a[2]);
//CHECK-NEXT: 2 1,prop1 prop1 undefined

// Now check it in strict-mode.
try {
    (function() {
        "use strict";
        Object.defineProperty(a, "2", {configurable:true, value:'prop2'});
        Object.defineProperty(a, "1", {value:'prop1'});
        a.length=1;
    })();
} catch(e) {
    print("caught exception");
    print(a.length, a, a[1], a[2]);
}
//CHECK-NEXT: caught exception
//CHECK-NEXT: 2 1,prop1 prop1 undefined
