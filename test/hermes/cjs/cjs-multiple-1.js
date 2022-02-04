/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs %S/cjs-multiple-1.js %S/cjs-multiple-2.js | %FileCheck --match-full-lines %s
// RUN: %hermes -O -commonjs %S/cjs-multiple-1.js %S/cjs-multiple-2.js | %FileCheck --match-full-lines %s
print('initializing module 1');

var module2 = require('./cjs-multiple-2.js')

print('module2.x=', module2.x);
print('module2.y=', module2.y);

// CHECK-LABEL: initializing module 1
// CHECK-NEXT: initializing module 2
// CHECK-NEXT: module2.x= 42
// CHECK-NEXT: module2.y= asdf
