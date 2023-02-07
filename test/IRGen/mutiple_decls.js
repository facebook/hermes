/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

// Make sure that multiple declarations are collapsed into one.
var a;
var a;
var a;

function foo () {
    var b, b, a;
    var b;
}

var foo;
foo(); // This is still a valid call.

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "a" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %foo()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %5 = StoreStackInst undefined : undefined, %4
// CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %7 = CallInst %6, empty, empty, undefined : undefined
// CHECK-NEXT:  %8 = StoreStackInst %7, %4
// CHECK-NEXT:  %9 = LoadStackInst %4
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function foo()
// CHECK-NEXT:frame = [b, a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [b]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
