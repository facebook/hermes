/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs %S/cjs-subdir-main.js %S/cjs-subdir-2.js %S/foo/cjs-subdir-foo.js %S/bar/cjs-subdir-bar.js | %FileCheck --match-full-lines %s
// RUN: %hermes -O -fstatic-builtins -fstatic-require -commonjs %S/cjs-subdir-main.js %S/cjs-subdir-2.js %S/foo/cjs-subdir-foo.js %S/bar/cjs-subdir-bar.js | %FileCheck --match-full-lines %s
// RUN: %hermes -commonjs %S | %FileCheck --match-full-lines %s
// RUN: %hermes -commonjs %S -O -fstatic-require -fstatic-builtins | %FileCheck --match-full-lines %s
// RUN: cd %S && zip %T/subdir.zip metadata.json cjs-subdir-main.js cjs-subdir-2.js foo/cjs-subdir-foo.js bar/cjs-subdir-bar.js && %hermes -commonjs %T/subdir.zip | %FileCheck --match-full-lines %s
// TODO(T53144040) Fix LIT tests on Windows
// XFAIL: windows

print('main: init');
// CHECK-LABEL: main: init

var foo = require('./foo/cjs-subdir-foo.js');
// CHECK-NEXT: foo: init
// CHECK-NEXT: bar: init
// CHECK-NEXT: foo: bar.y = 15
// CHECK-NEXT: foo: absolute bar.y = 15

print('main: foo.x =', foo.x);
// CHECK-NEXT: main: foo.x = 15

print('main: absolute foo.x =', require('/foo/cjs-subdir-foo.js').x);
// CHECK-NEXT: main: absolute foo.x = 15

print(typeof require.context);
// CHECK-NEXT: object

print(require('./cjs-subdir-2.js').alpha);
// CHECK-NEXT: 2: init
// CHECK-NEXT: 144
