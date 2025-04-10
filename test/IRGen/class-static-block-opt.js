/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

class A {
  static #f1 = 10;
  static {
    var x = this.#f1;
  }
  static {
    var x = this.#f1;
  }
  static {
    let y = this.#f1;
    print(y);
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreatePrivateNameInst (:privateName) "#f1": string
// CHECK-NEXT:  %1 = AllocStackInst (:object) $?anon_1_clsPrototype: any
// CHECK-NEXT:  %2 = CreateClassInst (:object) empty: any, empty: any, %A(): functionCode, empty: any, %1: object
// CHECK-NEXT:       AddOwnPrivateFieldInst 10: number, %2: object, %0: privateName
// CHECK-NEXT:  %4 = LoadOwnPrivateFieldInst (:any) %2: object, %0: privateName
// CHECK-NEXT:  %5 = LoadOwnPrivateFieldInst (:any) %2: object, %0: privateName
// CHECK-NEXT:  %6 = LoadOwnPrivateFieldInst (:any) %2: object, %0: privateName
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %6: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor A(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
