/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy -O -target=HBC %s | %FileCheck --match-full-lines %s

function stringSwitch(s) {
  'noinline'
  switch (s) {
  case "l0":
    return 0;
  case "l1":
    return 1;
  case "l2":
    return 2;
  case "l3":
    return 3;
  case "l4":
    return 4;
  case "l5":
    return 5;
  case "l6":
    return 6;
  case "l7":
    return 7;
  case "l8":
    return 8;
  case "l9":
    return 9;
  }
  return 1000;
}

print(stringSwitch("l0"));
//CHECK-LABEL: 0

// Do twice because the first execution does table initialization; make sure
// the second execution behaves the same.
print(stringSwitch("l0"));
//CHECK-LABEL: 0

print(stringSwitch("l9"));
//CHECK-NEXT: 9

print(stringSwitch("abc"));
//CHECK-NEXT: 1000

print(stringSwitch(100));
//CHECK-NEXT: 1000

print(stringSwitch(null));
//CHECK-NEXT: 1000

function stringSwitchXXX(s) {
  'noinline'
  switch (s) {
  case "xxxa":
    return 1;
  case "xxxb":
    return 2;
  case "xxxc":
    return 3;
  case "xxxd":
    return 4;
  case "xxxe":
    return 5;
  case "xxxf":
    return 6;
  case "xxxg":
    return 7;
  case "xxxh":
    return 8;
  case "xxxi":
    return 9;
  case "xxxj":
    return 10;
  default:
    return 0;
  }
}

print(stringSwitchXXX("xxxc"));
//CHECK-NEXT: 3

// The next test ensures that we are properly comparing
// StringPrimitive by value, rather than by pointer equality.  We
// construct a string that is equal to one of the case labels, but is
// a distinct StringPrimitive object.

// This is 'noinline' to defeat optimizations: to prevent the compiler
// from computing a (uniqued) constant string for the concatenation.
function concatStr(s0, s1) {
  'noinline'
  return s0 + s1;
}

print(stringSwitchXXX(concatStr("xx", "xc")));
//CHECK-NEXT: 3
