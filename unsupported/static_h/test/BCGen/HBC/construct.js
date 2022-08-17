/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s

function foo(x) {
  this.x = x;
}

//CHECK-LABEL:Function<bar>(1 params, {{[0-9]+}} registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:{{.*}} GetGlobalObject 0<Reg8>
//CHECK-NEXT:{{.*}} GetByIdShort 2<Reg8>, 0<Reg8>, 1<UInt8>, 2<UInt8>
//CHECK-NEXT:{{.*}} GetByIdShort 0<Reg8>, 2<Reg8>, 2<UInt8>, 3<UInt8>
//CHECK-NEXT:{{.*}} CreateThis 1<Reg8>, 0<Reg8>, 2<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUInt8 3<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} Mov 4<Reg8>, 1<Reg8>
//CHECK-NEXT:{{.*}} Construct 0<Reg8>, 2<Reg8>, 2<UInt8>
//CHECK-NEXT:{{.*}} SelectObject 0<Reg8>, 1<Reg8>, 0<Reg8>
//CHECK-NEXT:{{.*}} Ret 0<Reg8>

function bar() {
  return new foo(1);
}
