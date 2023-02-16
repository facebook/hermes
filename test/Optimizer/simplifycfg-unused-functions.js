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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %outer(): closure
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "outer": string
// CHECK-NEXT:  %3 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer(): closure
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %simplified(): undefined
// CHECK-NEXT:  %1 = ReturnInst (:closure) %0: closure
// CHECK-NEXT:function_end

// CHECK:function simplified(list: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end
