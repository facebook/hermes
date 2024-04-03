/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

(function () {
  var arrow1 = () => { print("in arrow"); }

  try {
    new arrow1();
  } catch (e) {
    print("caught", e.name, e.message);
//CHECK: caught TypeError Function is not a constructor
  }

  arrow1();
//CHECK-NEXT: in arrow
})();

// Methods on object literals cannot be called with new.
globalThis.a = "a";
(function () {
  var computedKey = globalThis.a;
  var obj = {
    m1() {
      return 5;
    },
    [computedKey]() {
      return 5;
    },
  };

  try {
    new obj.m1();
  } catch (e) {
    print(e);
//CHECK-NEXT: TypeError: Function is not a constructor
  }

  try {
    new obj[computedKey]();
  } catch (e) {
    print(e);
//CHECK-NEXT: TypeError: Function is not a constructor
  }
})();

