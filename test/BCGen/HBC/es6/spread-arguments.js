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

// CHECK-LABEL: Function<foo>(3 params, 30 registers, 2 symbols):
// CHECK-NEXT: Offset in debug table: source 0x{{.*}}, scope 0x{{.*}}
// CHECK-NEXT:     CreateEnvironment r0
// CHECK-NEXT:     LoadParam         r1, 1
// CHECK-NEXT:     LoadParam         r2, 2
// CHECK-NEXT:     StoreToEnvironment r0, 0, r1
// CHECK-NEXT:     StoreToEnvironment r0, 1, r2
// CHECK-NEXT:     LoadFromEnvironment r3, r0, 0
// CHECK-NEXT:     LoadConstZero     r5
// CHECK-NEXT:     Mov               r4, r5
// CHECK-NEXT:     LoadFromEnvironment r6, r0, 1
// CHECK-NEXT:     NewArray          r7, 0
// CHECK-NEXT:     Mov               r8, r4
// CHECK-NEXT:     Mov               r22, r7
// CHECK-NEXT:     Mov               r21, r6
// CHECK-NEXT:     Mov               r20, r8
// CHECK-NEXT:     CallBuiltin       r9, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:     Mov               r4, r9
// CHECK-NEXT:     LoadConstUndefined r10
// CHECK-NEXT:     Mov               r22, r3
// CHECK-NEXT:     Mov               r21, r7
// CHECK-NEXT:     Mov               r20, r10
// CHECK-NEXT:     CallBuiltin       r11, "HermesBuiltin.apply", 4
// CHECK-NEXT:     LoadFromEnvironment r11, r0, 0
// CHECK-NEXT:     LoadConstZero     r13
// CHECK-NEXT:     Mov               r12, r13
// CHECK-NEXT:     LoadFromEnvironment r14, r0, 1
// CHECK-NEXT:     NewArray          r15, 0
// CHECK-NEXT:     Mov               r16, r12
// CHECK-NEXT:     Mov               r22, r15
// CHECK-NEXT:     Mov               r21, r14
// CHECK-NEXT:     Mov               r20, r16
// CHECK-NEXT:     CallBuiltin       r17, "HermesBuiltin.arraySpread", 4
// CHECK-NEXT:     Mov               r12, r17
// CHECK-NEXT:     Mov               r22, r11
// CHECK-NEXT:     Mov               r21, r15
// CHECK-NEXT:     CallBuiltin       r18, "HermesBuiltin.apply", 3
// CHECK-NEXT:     LoadConstUndefined r18
// CHECK-NEXT:     Ret               r18
