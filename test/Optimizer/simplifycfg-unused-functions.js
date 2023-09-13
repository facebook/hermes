/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir -fno-inline %s | %FileCheckOrRegen %s

function outer() {
    function simplified(list) {
      if (true === true) {
        return;
      }

      // SimplifyCFG can eliminate creating and calling this inner function, since
      // it is unreachable. It should ensure that it also deletes the now unused
      // function, which loads from foo, which is now also unused.
      (function () {
        foo();
      })();
    }

    function foo() {}

    return simplified;
  }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %outer(): object
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "outer": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %simplified(): undefined
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function simplified(list: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
