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
// RUN: %hermes -commonjs %S -O -fstatic-require -fstatic-builtins -dump-bytecode | %FileCheck --match-full-lines %s --check-prefix BC

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

// BC-LABEL: CommonJS Modules (Static):
// BC-NEXT: Module ID 99 -> function ID 1
// BC-NEXT: Module ID 10 -> function ID 2
// BC-NEXT: Module ID 20 -> function ID 3
// BC-NEXT: Module ID 30 -> function ID 4
