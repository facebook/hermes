/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo(a, b, c) {
    return {a, ...b, c};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo(a, b, c)
// CHECK-NEXT:frame = [a, b, c]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadParamInst %b
// CHECK-NEXT:  %3 = StoreFrameInst %2, [b]
// CHECK-NEXT:  %4 = LoadParamInst %c
// CHECK-NEXT:  %5 = StoreFrameInst %4, [c]
// CHECK-NEXT:  %6 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %7 = LoadFrameInst [a]
// CHECK-NEXT:  %8 = StoreNewOwnPropertyInst %7, %6 : object, "a" : string, true : boolean
// CHECK-NEXT:  %9 = LoadFrameInst [b]
// CHECK-NEXT:  %10 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %6 : object, %9
// CHECK-NEXT:  %11 = LoadFrameInst [c]
// CHECK-NEXT:  %12 = StoreOwnPropertyInst %11, %6 : object, "c" : string, true : boolean
// CHECK-NEXT:  %13 = ReturnInst %6 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
