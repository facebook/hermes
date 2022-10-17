/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Make sure that we are not inlining if copyRestArgs() is used.

function outer1() {
    return (function dontInline(...rest) {
        return rest;
    })(1);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [outer1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %outer1#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "outer1" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function outer1#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{outer1#0#1()#2}
// CHECK-NEXT:  %1 = CreateFunctionInst %dontInline#1#2()#3, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined, 1 : number
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:function_end

// CHECK:function dontInline#1#2()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{dontInline#1#2()#3}
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.copyRestArgs] : number, undefined : undefined, 0 : number
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:function_end
