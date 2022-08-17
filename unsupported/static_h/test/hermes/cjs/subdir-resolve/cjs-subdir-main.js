/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs %S -O -fstatic-require -fstatic-builtins | %FileCheck --match-full-lines %s
// RUN: cd %S && zip %T/subdir.zip metadata.json cjs-subdir-main.js cjs-subdir-2.js foo/cjs-subdir-foo.js bar/cjs-subdir-bar.js && %hermes -commonjs %T/subdir.zip -O -fstatic-builtins -fstatic-require | %FileCheck --match-full-lines %s
// TODO(T53144040) Fix LIT tests on Windows
// XFAIL: windows

// Use directory and Zip inputs to allow for metadata.json resolution.

print('main: init');
// CHECK-LABEL: main: init

var foo = require('foo');
// CHECK-NEXT: foo: init
// CHECK-NEXT: bar: init
// CHECK-NEXT: 2: init
// CHECK-NEXT: foo: bar.y = 144
// CHECK-NEXT: foo: bar.y = 144

print('main: foo.x =', foo.x);
// CHECK-NEXT: main: foo.x = 144

print(require('two').alpha);
// CHECK-NEXT: 144
