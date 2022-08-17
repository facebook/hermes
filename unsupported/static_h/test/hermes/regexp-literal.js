/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// Make sure RegExp literals.
"use strict";

print('RegExp Literal');
// CHECK-LABEL: RegExp Literal
var re = /aa/;
print(re.__proto__.constructor === RegExp);
// CHECK-NEXT: true
print(/abc/.toString());
// CHECK-NEXT: /abc/

var m = /\w (\d+)/.exec("abc 1234");
print(m.input + ", " + m.index + ", " + m.length + ", " + m[1]);
// CHECK-NEXT: abc 1234, 2, 2, 1234
print(/\w (\d+)/.exec("abc def"));
// CHECK-NEXT: null
print(/\w (\d+)/.exec("abc 1234"));
// CHECK-NEXT: c 1234,1234
