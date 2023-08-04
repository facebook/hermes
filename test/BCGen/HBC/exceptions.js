/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -bs -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s

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
//CHECK-NEXT:{{.*}} LoadParam 1<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} Mov 3<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 0<Reg8>
//CHECK-NEXT:{{.*}} Call1 0<Reg8>, 3<Reg8>, 0<Reg8>
//CHECK-NEXT:{{.*}} Jmp 32<Addr8>
//CHECK-NEXT:{{.*}} Catch 2<Reg8>
//CHECK-NEXT:{{.*}} Mov 3<Reg8>, 2<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 0<Reg8>
//CHECK-NEXT:{{.*}} Call1 0<Reg8>, 3<Reg8>, 0<Reg8>
//CHECK-NEXT:{{.*}} Jmp 10<Addr8>
//CHECK-NEXT:{{.*}} Catch 3<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 0<Reg8>
//CHECK-NEXT:{{.*}} Call1 0<Reg8>, 3<Reg8>, 0<Reg8>
//CHECK-NEXT:{{.*}} Mov 3<Reg8>, 2<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 0<Reg8>
//CHECK-NEXT:{{.*}} Call1 0<Reg8>, 3<Reg8>, 0<Reg8>
//CHECK-NEXT:{{.*}} Mov 3<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 0<Reg8>
//CHECK-NEXT:{{.*}} Call1 3<Reg8>, 3<Reg8>, 0<Reg8>
//CHECK-NEXT:{{.*}} Ret 0<Reg8>
//CHECK-NEXT:{{.*}} Catch 0<Reg8>
//CHECK-NEXT:{{.*}} Mov 3<Reg8>, 2<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 2<Reg8>
//CHECK-NEXT:{{.*}} Call1 2<Reg8>, 3<Reg8>, 2<Reg8>
//CHECK-NEXT:{{.*}} Throw 0<Reg8>
//CHECK-NEXT:{{.*}} Catch 0<Reg8>
//CHECK-NEXT:{{.*}} Mov 2<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 1<Reg8>
//CHECK-NEXT:{{.*}} Call1 1<Reg8>, 2<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} Throw 0<Reg8>

//CHECK-LABEL: Exception Handlers:
//CHECK-NEXT: 0: start = 16, end = 25, target = 27
//CHECK-NEXT: 1: start = 3, end = 12, target = 14
//CHECK-NEXT: 2: start = 16, end = 35, target = 55
//CHECK-NEXT: 3: start = 3, end = 44, target = 68
//CHECK-NEXT: 4: start = 55, end = 68, target = 68
