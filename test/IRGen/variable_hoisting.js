/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

main()

function main() {

  function foo(x) { return capture_me; }

  var capture_me;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %main(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) globalObject: object, "main": string
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %7 = StoreStackInst %6: any, %3: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %9 = ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function main(): any
// CHECK-NEXT:frame = [foo: any, capture_me: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [foo]: any
// CHECK-NEXT:  %1 = StoreFrameInst undefined: undefined, [capture_me]: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:  %3 = StoreFrameInst %2: object, [foo]: any
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [capture_me@main]: any
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = UnreachableInst
// CHECK-NEXT:function_end
