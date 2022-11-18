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
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function foo(a, b, c)
// CHECK-NEXT:frame = [a, b, c]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:  %1 = StoreFrameInst %b, [b]
// CHECK-NEXT:  %2 = StoreFrameInst %c, [c]
// CHECK-NEXT:  %3 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %4 = LoadFrameInst [a]
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst %4, %3 : object, "a" : string, true : boolean
// CHECK-NEXT:  %6 = LoadFrameInst [b]
// CHECK-NEXT:  %7 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %3 : object, %6
// CHECK-NEXT:  %8 = LoadFrameInst [c]
// CHECK-NEXT:  %9 = StoreOwnPropertyInst %8, %3 : object, "c" : string, true : boolean
// CHECK-NEXT:  %10 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
