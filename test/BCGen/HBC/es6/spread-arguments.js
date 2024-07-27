/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheck %s --match-full-lines

function foo(fn, x) {
  fn(...x);
  new fn(...x);
}

// CHECK-LABEL:Function<foo>(3 params, 17 registers, 2 symbols):
// CHECK-NEXT: Offset in debug table: source 0x{{.*}}, scope 0x{{.*}}
// CHECK-NEXT:     CreateEnvironment r3
// CHECK-NEXT:     LoadParam         r5, 1
// CHECK-NEXT:     LoadParam         r1, 2
// CHECK-NEXT:     StoreToEnvironment r3, 0, r5
// CHECK-NEXT:     StoreToEnvironment r3, 1, r1
// CHECK-NEXT:     LoadFromEnvironment r1, r3, 0
// CHECK-NEXT:     LoadConstZero     r2
// CHECK-NEXT:     Mov               r5, r2
// CHECK-NEXT:     LoadFromEnvironment r2, r3, 1
// CHECK-NEXT:     NewArray          r4, 0
// CHECK-NEXT:     Mov               r6, r5
// CHECK-NEXT:     Mov               r9, r4
// CHECK-NEXT:     Mov               r8, r2
// CHECK-NEXT:     Mov               r7, r6
// CHECK-NEXT:     CallBuiltin       r2, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:     Mov               r5, r2
// CHECK-NEXT:     LoadConstUndefined r2
// CHECK-NEXT:     Mov               r9, r1
// CHECK-NEXT:     Mov               r8, r4
// CHECK-NEXT:     Mov               r7, r2
// CHECK-NEXT:     CallBuiltin       r0, "HermesBuiltin.apply", 4
// CHECK-NEXT:     LoadFromEnvironment r1, r3, 0
// CHECK-NEXT:     LoadConstZero     r2
// CHECK-NEXT:     Mov               r4, r2
// CHECK-NEXT:     LoadFromEnvironment r3, r3, 1
// CHECK-NEXT:     NewArray          r2, 0
// CHECK-NEXT:     Mov               r5, r4
// CHECK-NEXT:     Mov               r9, r2
// CHECK-NEXT:     Mov               r8, r3
// CHECK-NEXT:     Mov               r7, r5
// CHECK-NEXT:     CallBuiltin       r3, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:     Mov               r4, r3
// CHECK-NEXT:     Mov               r9, r1
// CHECK-NEXT:     Mov               r8, r2
// CHECK-NEXT:     CallBuiltin       r0, "HermesBuiltin.apply", 3
// CHECK-NEXT:     LoadConstUndefined r1
// CHECK-NEXT:     Ret               r1
