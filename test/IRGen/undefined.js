/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function foo() {
  var undefined = 5;
  return undefined;
}

var undefined = 5;
foo();
undefined;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "undefined" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %foo()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %5 = StoreStackInst undefined : undefined, %4
// CHECK-NEXT:  %6 = StorePropertyLooseInst 5 : number, globalObject : object, "undefined" : string
// CHECK-NEXT:  %7 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %8 = CallInst %7, undefined : undefined
// CHECK-NEXT:  %9 = StoreStackInst %8, %4
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %4
// CHECK-NEXT:  %11 = LoadStackInst %4
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function foo()
// CHECK-NEXT:frame = [undefined]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [undefined]
// CHECK-NEXT:  %1 = StoreFrameInst 5 : number, [undefined]
// CHECK-NEXT:  %2 = LoadFrameInst [undefined]
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
