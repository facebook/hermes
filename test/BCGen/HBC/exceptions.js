/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s

function foo(a) {
  try {
    a();
  } catch (e) {
    try {
      e();
    } catch (e) {
      e();
    }
    finally {
      e();
    }
  }
  finally {
    a();
  }
}

//CHECK-LABEL:Function<foo>(2 params, 11 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:{{.*}} LoadParam 2<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 1<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 3<Reg8>
//CHECK-NEXT:{{.*}} Call1 0<Reg8>, 2<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} Jmp 26<Addr8>
//CHECK-NEXT:{{.*}} Catch 3<Reg8>
//CHECK-NEXT:{{.*}} Mov 0<Reg8>, 3<Reg8>
//CHECK-NEXT:{{.*}} Call1 0<Reg8>, 0<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} Jmp 8<Addr8>
//CHECK-NEXT:{{.*}} Catch 0<Reg8>
//CHECK-NEXT:{{.*}} Call1 0<Reg8>, 0<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} Mov 0<Reg8>, 3<Reg8>
//CHECK-NEXT:{{.*}} Call1 0<Reg8>, 0<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} Call1 0<Reg8>, 2<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} Ret 1<Reg8>
//CHECK-NEXT:{{.*}} Catch 0<Reg8>
//CHECK-NEXT:{{.*}} Call1 3<Reg8>, 3<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} Throw 0<Reg8>
//CHECK-NEXT:{{.*}} Catch 0<Reg8>
//CHECK-NEXT:{{.*}} Call1 1<Reg8>, 2<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} Throw 0<Reg8>

//CHECK-LABEL: Exception Handlers:
//CHECK-NEXT: 0: start = 15, end = 22, target = 24
//CHECK-NEXT: 1: start = 7, end = 11, target = 13
//CHECK-NEXT: 2: start = 15, end = 30, target = 43
//CHECK-NEXT: 3: start = 7, end = 37, target = 51
//CHECK-NEXT: 4: start = 43, end = 51, target = 51
