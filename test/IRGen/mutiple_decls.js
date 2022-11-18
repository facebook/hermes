/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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
// CHECK-NEXT:frame = [], globals = [a, foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined
// CHECK-NEXT:  %6 = StoreStackInst %5, %2
// CHECK-NEXT:  %7 = LoadStackInst %2
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function foo()
// CHECK-NEXT:frame = [b, a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [b]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
