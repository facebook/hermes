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

// CHECK-LABEL: Function<foo>(3 params, 28 registers, 2 symbols):
// CHECK-NEXT: Offset in debug table: source 0x{{.*}}, lexical 0x{{.*}}
// CHECK-NEXT:     CreateEnvironment r0
// CHECK-NEXT:     LoadParam         r1, 1
// CHECK-NEXT:     LoadParam         r2, 2
// CHECK-NEXT:     LoadConstZero     r3
// CHECK-NEXT:     LoadConstUndefined r4
// CHECK-NEXT:     StoreToEnvironment r0, 0, r1
// CHECK-NEXT:     StoreToEnvironment r0, 1, r2
// CHECK-NEXT:     LoadFromEnvironment r5, r0, 0
// CHECK-NEXT:     Mov               r6, r3
// CHECK-NEXT:     LoadFromEnvironment r7, r0, 1
// CHECK-NEXT:     NewArray          r8, 0
// CHECK-NEXT:     Mov               r9, r6
// CHECK-NEXT:     Mov               r20, r8
// CHECK-NEXT:     Mov               r19, r7
// CHECK-NEXT:     Mov               r18, r9
// CHECK-NEXT:     CallBuiltin       r10, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:     Mov               r6, r10
// CHECK-NEXT:     Mov               r20, r5
// CHECK-NEXT:     Mov               r19, r8
// CHECK-NEXT:     Mov               r18, r4
// CHECK-NEXT:     CallBuiltin       r11, "HermesBuiltin.apply", 4
// CHECK-NEXT:     LoadFromEnvironment r11, r0, 0
// CHECK-NEXT:     Mov               r12, r3
// CHECK-NEXT:     LoadFromEnvironment r13, r0, 1
// CHECK-NEXT:     NewArray          r14, 0
// CHECK-NEXT:     Mov               r15, r12
// CHECK-NEXT:     Mov               r20, r14
// CHECK-NEXT:     Mov               r19, r13
// CHECK-NEXT:     Mov               r18, r15
// CHECK-NEXT:     CallBuiltin       r16, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:     Mov               r12, r16
// CHECK-NEXT:     Mov               r20, r11
// CHECK-NEXT:     Mov               r19, r14
// CHECK-NEXT:     CallBuiltin       r17, "HermesBuiltin.apply", 3
// CHECK-NEXT:     Ret               r4
