/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xjit=force -Xjit-crash-on-error -Xjit-emit-type-asserts %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -Xjit=force -Xjit-crash-on-error -Xjit-emit-type-asserts %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function addNumbers(a, b) {
  var sum = 0;
  for (var i = 0; i < 100; ++i)
    sum += a * b - i;
  return sum;
}

print(addNumbers(3, 4));
// CHECK: -3750

// Materializes a genuine boolean value (the print() use stops it from being
// fused into the comparison's own branch), then branches on it, exercising
// jmpTrueFalse's known-Bool tier (TypePred::IsBool).
function boolCond(x, y) {
  var b = (x === y);
  print(b);
  if (b)
    print("eq");
  else
    print("neq");
}
boolCond(3, 3);
// CHECK-NEXT: true
// CHECK-NEXT: eq
boolCond(3, 4);
// CHECK-NEXT: false
// CHECK-NEXT: neq

// The `null` operand is a compile-time-known OtherNonPtr constant, driving
// strictEqualImpl's raw-bit tier (TypePred::BitComparable).
function eqNull(x) {
  return x === null;
}
print(eqNull(null));
// CHECK-NEXT: true
print(eqNull(3));
// CHECK-NEXT: false

// Same elision, but as a conditional jump: the `true` operand is a
// compile-time-known Bool constant, driving jStrictEqual's raw-bit tier
// (TypePred::BitComparable), a distinct emitter from strictEqualImpl.
function eqTrue(x) {
  if (x === true)
    return 1;
  return 0;
}
print(eqTrue(true));
// CHECK-NEXT: 1
print(eqTrue(false));
// CHECK-NEXT: 0
print(eqTrue(3));
// CHECK-NEXT: 0

// Regression: under -O0, HBC's register allocator can coalesce
// StrictEqual's result register with the register of the *unknown*
// operand (x.p here), not just the known constant's, since coalescing is
// just lowest-free-register reuse and has no notion of which operand is
// "known". strictEqualImpl's per-operand guards must be evaluated before
// frUpdatedWithHW(frRes, ...) overwrites that shared register's
// localType, or the guard goes spuriously true (or false) against a
// register that still holds the original, differently-typed operand.
// Under -O0 this compiles to `StrictEq rX, rX, rY` with rX = x.p, and
// previously crashed here with a bogus "expected non-pointer non-number"
// type assert failure on x.p's own (non-null) value.
function eqNullAlias(x) {
  return x.p === null;
}
print(eqNullAlias({p: {}}));
// CHECK-NEXT: false
print(eqNullAlias({p: null}));
// CHECK-NEXT: true

// Class C: the boolean-typed values here earn a non-pointer-classed
// global register (FRType::UnknownNonPtr), whose write is checked against
// TypePred::NotPointer at the following instruction boundary. Under -O
// the loop is rotated and `flag` is folded into the branch, so exactly
// one such check survives, in the loop pre-header. The numbers earn
// Number-classed global registers, checked the same way at many more
// sites, including inside the loop body. The -O0 RUN line contributes no
// Class C coverage at all: without optimization no FR gets a global
// register.
function nonPtrGlobalReg(n) {
  var flag = false;
  var total = 0;
  for (var i = 0; i < n; ++i) {
    flag = (i % 2) === 0;
    if (flag)
      total += i;
  }
  return total;
}
print(nonPtrGlobalReg(10));
// CHECK-NEXT: 20
