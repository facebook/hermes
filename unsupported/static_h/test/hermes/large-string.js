/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -gc-sanitize-handles=0 -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

var MBString = "0123456789abcdef";
for(var i = 0; i < 16; ++i)
  MBString += MBString;

var arr = Array(512);
for(var i = 0; i < 512; ++i)
  arr[i] = MBString;

var largeString;

try {
    largeString = arr.join("");
} catch(e) {
    print("caught", e.name, e.message);
}
//CHECK: caught RangeError {{.*}}
