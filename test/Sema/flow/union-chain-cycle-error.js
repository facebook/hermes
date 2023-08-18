/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -typed -dump-sema %s 2>&1 ) | %FileCheck --match-full-lines %s

type A = B | B;
type B = C | C;
type C = A | number;

// CHECK:{{.*}}:11:1: error: ft: type contains a circular reference to itself
// CHECK-NEXT:type B = C | C;
// CHECK-NEXT:^~~~~~~~~~~~~~~
