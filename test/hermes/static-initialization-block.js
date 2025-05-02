/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -lazy -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes %s -exec | %FileCheck --match-full-lines %s

print("static blocks");
// CHECK: static blocks

// Correct interleaving of static blocks and fields.
(function () {
  var x = 0;
  class A {
    static f1 = ++x;
    static #f1 = ++x;
    static {
      A.staticBlockVal = ++x;
    }
    static f2 = ++x;
    static #f2 = ++x;
    static dumpProps() {
      print(A.f1);
// CHECK-NEXT: 1
      print(A.#f1);
// CHECK-NEXT: 2
      print(A.staticBlockVal);
// CHECK-NEXT: 3
      print(A.f2);
// CHECK-NEXT: 4
      print(A.#f2);
// CHECK-NEXT: 5
    }
  }
  A.dumpProps();
})();

// Static blocks can read private names, and correct `this` binding.
(function () {
  class B {
    static #f1 = 12;
    static {
      print(this.#f1);
// CHECK-NEXT: 12
    }
  }
})();

// Arrow functions in static blocks work.
(function () {
  class C {
    static #f1 = 13;
    static {
      let arr1 = () => {
        let arr2 = () => {
          let arr3 = () => {
            print(this.#f1);
  // CHECK-NEXT: 13
          }
          arr3();
        }
        arr2();
      };
      arr1();
    }
  }
})();
