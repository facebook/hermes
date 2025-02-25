/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

(function (){
  function foo(sink) {
    "inline";
    let x = 10;
    sink(() => x++);
    return x;
  }

  // Make sure foo escapes, so x appears to escape as well.
  globalThis.foo = foo;

  // After inlining, we know that the scope instance in the inlined function
  // does not escape, even though x in general does escape.
  return foo(()=>{});
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:       StorePropertyLooseInst %1: object, %2: any, "foo": string
// CHECK-NEXT:       ReturnInst 10: number
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [x: number]

// CHECK:function foo(sink: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:       StoreFrameInst %0: environment, 10: number, [%VS1.x]: number
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS1: any, %" 2#"(): functionCode
// CHECK-NEXT:  %4 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %3: object
// CHECK-NEXT:  %5 = LoadFrameInst (:number) %0: environment, [%VS1.x]: number
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:arrow " 2#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS1.x]: number
// CHECK-NEXT:  %2 = FAddInst (:number) %1: number, 1: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %2: number, [%VS1.x]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
