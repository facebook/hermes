/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy -O -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

// Each function compiles to a StringSwitchImm with its own runtime lookup
// table, stored by index in a vector on the RuntimeModule. Under -lazy, the
// table vector grows (and may reallocate) as each function is compiled on
// first call. The JIT must reference the table by (module, index) rather than
// baking its address in, otherwise the code for an earlier function ends up
// with a dangling pointer once a later function grows the vector.

function first(s) {
  switch (s) {
    case "l0": return 0;
    case "l1": return 1;
    case "l2": return 2;
    case "l3": return 3;
    case "l4": return 4;
    case "l5": return 5;
    case "l6": return 6;
    case "l7": return 7;
    case "l8": return 8;
    case "l9": return 9;
  }
  return 1000;
}

function second(s) {
  switch (s) {
    case "m0": return 10;
    case "m1": return 11;
    case "m2": return 12;
    case "m3": return 13;
    case "m4": return 14;
    case "m5": return 15;
    case "m6": return 16;
    case "m7": return 17;
    case "m8": return 18;
    case "m9": return 19;
  }
  return 2000;
}

function third(s) {
  switch (s) {
    case "n0": return 20;
    case "n1": return 21;
    case "n2": return 22;
    case "n3": return 23;
    case "n4": return 24;
    case "n5": return 25;
    case "n6": return 26;
    case "n7": return 27;
    case "n8": return 28;
    case "n9": return 29;
  }
  return 3000;
}

// Compile+JIT first, baking a reference to table index 0.
print(first("l3"));
// CHECK: 3

// Compile+JIT second and third; each grows the RuntimeModule's table vector,
// reallocating the storage that first's table used to live in.
print(second("m4"));
// CHECK-NEXT: 14
print(third("n7"));
// CHECK-NEXT: 27

// Re-enter first: its table must still be found via the (now reallocated)
// vector. A baked table pointer would be dangling here.
print(first("l5"));
// CHECK-NEXT: 5
print(first("nope"));
// CHECK-NEXT: 1000
