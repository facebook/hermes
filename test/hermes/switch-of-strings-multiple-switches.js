/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC %s | %FileCheck %s --match-full-lines

function stringSwitchSeveralSwitches(b, s, i) {
  'noinline'
  var res = 0;
  switch (i) {
  case 1:
    res = 10000;
    break;
  case 2:
    res = 20000;
    break;
  case 3:
    res = 30000;
    break;
  case 4:
    res = 40000;
    break;
  case 5:
    res = 50000;
    break;
  case 6:
    res = 60000;
    break;
  case 7:
    res = 70000;
    break;
  case 8:
    res = 80000;
    break;
  case 9:
    res = 90000;
    break;
  case 10:
    res = 100000;
    break;
  }
  if (b) {
    switch (s) {
      case "l0":
        res += 10;
        break;
      case "l1":
        res += 11;
        break;
      case "l2":
        res += 12;
        break;
      case "l3":
        res += 13;
        break;
      case "l4":
        res += 14;
        break;
      case "l5":
        res += 15;
        break;
      case "l6":
        res += 16;
        break;
      case "l7":
        res += 17;
        break;
      case "l8":
        res += 18;
        break;
      case "l9":
        res += 19;
        break;
    }
  } else {
    switch (s) {
      case "l1000":
        res += 1000;
        break;
      case "l1100":
        res += 1100;
        break;
      case "l1200":
        res += 1200;
        break;
      case "l1300":
        res += 1300;
        break;
      case "l1400":
        res += 1400;
        break;
      case "l1500":
        res += 1500;
        break;
      case "l1600":
        res += 1600;
        break;
      case "l1700":
        res += 1700;
        break;
      case "l1800":
        res += 1800;
        break;
      case "l1900":
        res += 1900;
        break;
    }
  }
  return res;
}

print(stringSwitchSeveralSwitches(true, "l5", 1));
//CHECK-LABEL: 10015

// Do twice because the first execution does table initialization; make sure
// the second execution behaves the same.
print(stringSwitchSeveralSwitches(true, "l5", 1));
//CHECK-LABEL: 10015

print(stringSwitchSeveralSwitches(false, "l1500", 2));
//CHECK-NEXT: 21500

// default case for int switch.
print(stringSwitchSeveralSwitches(false, "l1500", 1000));
//CHECK-NEXT: 1500

// default case for one of the string switches.
print(stringSwitchSeveralSwitches(false, "not-a-label", 3));
//CHECK-NEXT: 30000

// First and last cases for the string switches.
print(stringSwitchSeveralSwitches(true, "l0", 4));
//CHECK-NEXT: 40010
print(stringSwitchSeveralSwitches(true, "l9", 4));
//CHECK-NEXT: 40019
print(stringSwitchSeveralSwitches(false, "l1000", 4));
//CHECK-NEXT: 41000
print(stringSwitchSeveralSwitches(false, "l1900", 4));
//CHECK-NEXT: 41900

