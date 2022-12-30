/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -fno-inline | %FileCheckOrRegen %s

"use strict";

function ctor_this_test() {
    function use_this(k) {
        this.k = k
        return this;
    }

    return new use_this(12)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : string
// CHECK-NEXT:frame = [], globals = [ctor_this_test]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %ctor_this_test() : object
// CHECK-NEXT:  %1 = StorePropertyStrictInst %0 : closure, globalObject : object, "ctor_this_test" : string
// CHECK-NEXT:  %2 = ReturnInst "use strict" : string
// CHECK-NEXT:function_end

// CHECK:function ctor_this_test() : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %use_this()
// CHECK-NEXT:  %1 = ConstructInst %0 : closure, undefined : undefined, 12 : number
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function use_this(k : number)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this
// CHECK-NEXT:  %1 = StorePropertyStrictInst 12 : number, %0, "k" : string
// CHECK-NEXT:  %2 = ReturnInst %0
// CHECK-NEXT:function_end
