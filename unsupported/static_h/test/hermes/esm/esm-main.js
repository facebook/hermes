/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs %S/esm-main.js %S/esm-foo.js %S/esm-bar.js | %FileCheck --match-full-lines %s

print('esm');
// CHECK-LABEL: esm

import * as Foo from './esm-foo.js';

print(Foo.x);
// CHECK-NEXT: 42
print(Foo.y());
// CHECK-NEXT: 182
print(Foo.z);
// CHECK-NEXT: 472
print(Foo.shortVar);
// CHECK-NEXT: 157
print(Foo.a);
// CHECK-NEXT: 24
print(Foo.b);
// CHECK-NEXT: 1845

import { y, z as zAliased } from './esm-foo.js';

print(y(), zAliased);
// CHECK-NEXT: 182 472

print(FooDefault());
// CHECK-NEXT: 352

// Ensure imports are hoisted.
import FooDefault from './esm-foo.js';
