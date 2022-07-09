/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheck %s --match-full-lines

function *loop(x) {
  var i = 0;
  while (y) {
    yield x[i++];
  }
  return 'DONE LOOPING';
}

// CHECK-LABEL: NCFunction<loop>(2 params, 4 registers, 1 symbols):
// CHECK-NEXT:     CreateEnvironment r0
// CHECK-NEXT:     LoadConstUndefined r1
// CHECK-NEXT:     StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:     CreateGenerator   r2, r0, 2
// CHECK-NEXT:     Ret               r2

// CHECK-LABEL: Function<?anon_0_loop>(2 params, 14 registers, 2 symbols):
// CHECK-NEXT: Offset in debug table: source 0x{{.*}}, lexical 0x0000
// CHECK-NEXT:     StartGenerator
// CHECK-NEXT:     CreateEnvironment r0
// CHECK-NEXT:     LoadParam         r1, 1
// CHECK-NEXT:     LoadConstUndefined r2
// CHECK-NEXT:     LoadConstZero     r3
// CHECK-NEXT:     LoadConstString   r4, "DONE LOOPING"
// CHECK-NEXT:     GetGlobalObject   r5
// CHECK-NEXT:     ResumeGenerator   r7, r6
// CHECK-NEXT:     Mov               r8, r6
// CHECK-NEXT:     JmpTrue           L1, r8
// CHECK-NEXT:     StoreNPToEnvironment r0, 0, r2
// CHECK-NEXT:     StoreToEnvironment r0, 1, r1
// CHECK-NEXT:     StoreNPToEnvironment r0, 0, r3
// CHECK-NEXT:     TryGetById        r6, r5, 1, "y"
// CHECK-NEXT:     JmpFalse          L2, r6
// CHECK-NEXT: L5:
// CHECK-NEXT:     LoadFromEnvironment r6, r0, 1
// CHECK-NEXT:     LoadFromEnvironment r8, r0, 0
// CHECK-NEXT:     ToNumeric         r9, r8
// CHECK-NEXT:     Inc              r10, r9
// CHECK-NEXT:     StoreToEnvironment r0, 0, r10
// CHECK-NEXT:     GetByVal          r11, r6, r9
// CHECK-NEXT:     SaveGenerator     L3
// CHECK-NEXT:     Ret               r11
// CHECK-NEXT: L3:
// CHECK-NEXT:     ResumeGenerator   r6, r12
// CHECK-NEXT:     Mov               r8, r12
// CHECK-NEXT:     JmpTrue           L4, r8
// CHECK-NEXT:     TryGetById        r8, r5, 1, "y"
// CHECK-NEXT:     JmpTrue           L5, r8
// CHECK-NEXT: L2:
// CHECK-NEXT:     CompleteGenerator
// CHECK-NEXT:     Ret               r4
// CHECK-NEXT: L4:
// CHECK-NEXT:     CompleteGenerator
// CHECK-NEXT:     Ret               r6
// CHECK-NEXT: L1:
// CHECK-NEXT:     CompleteGenerator
// CHECK-NEXT:     Ret               r7

function *args() {
  yield arguments[0];
}

// CHECK-LABEL: NCFunction<args>(1 params, 3 registers, 0 symbols):
// CHECK-NEXT:     CreateEnvironment r0
// CHECK-NEXT:     CreateGenerator   r1, r0, 4
// CHECK-NEXT:     Ret               r1

// CHECK-LABEL: Function<?anon_0_args>(1 params, 7 registers, 0 symbols):
// CHECK-NEXT: Offset in debug table: source 0x{{.*}}, lexical 0x0000
// CHECK-NEXT:     StartGenerator
// CHECK-NEXT:     CreateEnvironment r0
// CHECK-NEXT:     LoadConstUndefined r0
// CHECK-NEXT:     LoadConstZero     r1
// CHECK-NEXT:     ResumeGenerator   r3, r2
// CHECK-NEXT:     Mov               r4, r2
// CHECK-NEXT:     JmpTrue           L1, r4
// CHECK-NEXT:     Mov               r2, r0
// CHECK-NEXT:     GetArgumentsPropByVal r4, r1, r2
// CHECK-NEXT:     SaveGenerator     L2
// CHECK-NEXT:     Ret               r4
// CHECK-NEXT: L2:
// CHECK-NEXT:     ResumeGenerator   r2, r5
// CHECK-NEXT:     Mov               r4, r5
// CHECK-NEXT:     JmpTrue           L3, r4
// CHECK-NEXT:     CompleteGenerator
// CHECK-NEXT:     Ret               r0
// CHECK-NEXT: L3:
// CHECK-NEXT:     CompleteGenerator
// CHECK-NEXT:     Ret               r2
// CHECK-NEXT: L1:
// CHECK-NEXT:     CompleteGenerator
// CHECK-NEXT:     Ret               r3
