/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs %S/cjs-circle-1.js %S/cjs-circle-2.js %S/cjs-circle-3.js | %FileCheck --match-full-lines %s
// RUN: %hermes -O -commonjs %S/cjs-circle-1.js %S/cjs-circle-2.js %S/cjs-circle-3.js | %FileCheck --match-full-lines %s
// RUN: %hermes -O -fstatic-builtins -fstatic-require -commonjs %S/cjs-circle-1.js %S/cjs-circle-2.js %S/cjs-circle-3.js | %FileCheck --match-full-lines %s
// RUN: %hermes -O -fstatic-builtins -fstatic-require -commonjs %S/cjs-circle-1.js %S/cjs-circle-2.js %S/cjs-circle-3.js -emit-binary -out %t.hbc && %hermes %t.hbc | %FileCheck --match-full-lines %s

print('1: init');
// CHECK-LABEL: 1: init

var mod2 = require('./cjs-circle-2.js');
// CHECK-NEXT: 2: init
// CHECK-NEXT: 3: init
// CHECK-NEXT: 3: mod2.x = 1
// CHECK-NEXT: 3: mod2.y = undefined
// CHECK-NEXT: 2: mod3.z = 42

print('1: mod2.x =', mod2.x);
// CHECK-NEXT: 1: mod2.x = undefined
print('1: mod2.y =', mod2.y);
// CHECK-NEXT: 1: mod2.y = 2
print('1: mod2.z =', mod2.z);
// CHECK-NEXT: 1: mod2.z = 42
