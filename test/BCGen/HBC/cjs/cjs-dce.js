/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -commonjs -dump-bytecode %s | %FileCheck --match-full-lines %s

var x = encodeURIComponent('asdf');
function foo() {
  // Need to ensure that foo retains the ability to run,
  // and doesn't have an UnreachableInst inside it.
  return x;
}

try {
  // The try-catch is here to ensure foo isn't inlined here.
  foo();
} catch (e) {}

foo();

// CHECK: Global String Table:
// CHECK-NEXT:   s0[ASCII, {{.*}}]: asdf
// CHECK-NEXT:   s1[ASCII, {{.*}}]: foo
// CHECK-NEXT:   s2[ASCII, {{.*}}]: cjs-dce.js
// CHECK-NEXT:   s3[ASCII, {{.*}}]: cjs_module
// CHECK-NEXT:   s4[ASCII, {{.*}}]: global
// CHECK-NEXT:   i5[ASCII, {{.*}}] #{{[0-9A-F]+}}: encodeURIComponent

// CHECK: CommonJS Modules:
// CHECK-NEXT:   File ID 2 -> function ID 1

// CHECK: Function<global>(1 params, 1 registers, 0 symbols):
// CHECK-NEXT:     LoadConstUndefined r0
// CHECK-NEXT:     Ret               r0

//CHECK:Function<cjs_module>(4 params, 13 registers, 1 symbols):
//CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
//CHECK-NEXT:    CreateEnvironment r3
//CHECK-NEXT:    CreateClosure     r1, r3, 2
//CHECK-NEXT:    GetGlobalObject   r0
//CHECK-NEXT:    TryGetById        r4, r0, 1, "encodeURIComponen"...
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    LoadConstString   r2, "asdf"
//CHECK-NEXT:    Call2             r2, r4, r0, r2
//CHECK-NEXT:    StoreToEnvironment r3, 0, r2
//CHECK-NEXT:L4:
//CHECK-NEXT:    Mov               r2, r1
//CHECK-NEXT:    Call1             r2, r2, r0
//CHECK-NEXT:L5:
//CHECK-NEXT:    Jmp               L2
//CHECK-NEXT:L1:
//CHECK-NEXT:    Catch             r2
//CHECK-NEXT:L2:
//CHECK-NEXT:    Call1             r1, r1, r0
//CHECK-NEXT:    Ret               r0

//CHECK:Exception Handlers:
//CHECK-NEXT:0: start = L4, end = L5, target = L1

// CHECK: Function<foo>(1 params, 1 registers, 0 symbols):
// CHECK-NEXT:     GetEnvironment    r0, 0
// CHECK-NEXT:     LoadFromEnvironment r0, r0, 0
// CHECK-NEXT:     Ret               r0
