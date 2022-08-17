/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:Function<binary>(1 params, 10 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:    GetGlobalObject   r0
//CHECK-NEXT:    GetByIdShort      r1, r0, 1, "foo"
//CHECK-NEXT:    LoadConstUndefined r2
//CHECK-NEXT:    Call1             r1, r1, r2
//CHECK-NEXT:    GetByIdShort      r0, r0, 1, "foo"
//CHECK-NEXT:    Call1             r0, r0, r2
//CHECK-NEXT:    Eq                r2, r1, r0
//CHECK-NEXT:    Neq               r2, r1, r0
//CHECK-NEXT:    Neq               r2, r1, r0
//CHECK-NEXT:    Less              r2, r1, r0
//CHECK-NEXT:    LessEq            r2, r1, r0
//CHECK-NEXT:    Greater           r2, r1, r0
//CHECK-NEXT:    GreaterEq         r2, r1, r0
//CHECK-NEXT:    LShift            r2, r1, r0
//CHECK-NEXT:    RShift            r2, r1, r0
//CHECK-NEXT:    URshift           r2, r1, r0
//CHECK-NEXT:    Add               r2, r1, r0
//CHECK-NEXT:    Sub               r2, r1, r0
//CHECK-NEXT:    Mul               r2, r1, r0
//CHECK-NEXT:    Div               r2, r1, r0
//CHECK-NEXT:    Mod               r2, r1, r0
//CHECK-NEXT:    BitOr             r2, r1, r0
//CHECK-NEXT:    BitXor            r2, r1, r0
//CHECK-NEXT:    BitAnd            r2, r1, r0
//CHECK-NEXT:    IsIn              r2, r1, r0
//CHECK-NEXT:    StrictNeq         r0, r1, r0
//CHECK-NEXT:    Ret               r0


function binary() {
  var x = foo(), y = foo(), z;
  z = x == y;
  z = x != y;
  z = x === y;
  z = x != y;
  z = x<y;
  z = x <= y;
  z = x>y;
  z = x >= y;
  z = x << y;
  z = x >> y;
  z = x >>> y;
  z = x + y;
  z = x - y;
  z = x * y;
  z = x / y;
  z = x % y;
  z = x | y;
  z = x ^ y;
  z = x & y;
  z = x in y;
  return x !== y;
}

function foo() { return; }
