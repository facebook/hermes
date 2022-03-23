/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

var x = Array(5);
print(x);
//CHECK: ,,,,

Object.preventExtensions(x);

try {
    x[0] = 1;
} catch(e) {
    print(e.name, e.message);
}
//CHECK-NEXT: TypeError {{.*}}

try {
    Object.defineProperty(x, 0, {writable:true, configurable:true, enumerable:true, value:1});
} catch(e) {
    print(e.name, e.message);
}
//CHECK-NEXT: TypeError {{.*}}
