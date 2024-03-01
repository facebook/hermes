/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -O0 -target=HBC -dump-lir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermes -strict -O -target=HBC -dump-lir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKOPT %s

var a = 5;
a;

function bar(a) {
  return a;
}

function foo(a) {
  var b = bar(a);
  return b;
}

function daa(a) {
  var b = a + 1;
  function daa_capture() {
    return b;
  }
  return daa_capture;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "a": string
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "daa": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %bar(): functionCode
// CHECK-NEXT:  %6 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:       StorePropertyStrictInst %5: object, %6: object, "bar": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:  %9 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:        StorePropertyStrictInst %8: object, %9: object, "foo": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %daa(): functionCode
// CHECK-NEXT:  %12 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:        StorePropertyStrictInst %11: object, %12: object, "daa": string
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %15 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:        StoreStackInst %15: undefined, %14: any
// CHECK-NEXT:  %17 = HBCLoadConstInst (:number) 5: number
// CHECK-NEXT:  %18 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:        StorePropertyStrictInst %17: number, %18: object, "a": string
// CHECK-NEXT:  %20 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  %21 = LoadPropertyInst (:any) %20: object, "a": string
// CHECK-NEXT:        StoreStackInst %21: any, %14: any
// CHECK-NEXT:  %23 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        ReturnInst %23: any
// CHECK-NEXT:function_end

// CHECK:function bar(a: any): any
// CHECK-NEXT:frame = [a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %bar(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: undefined, [b]: any
// CHECK-NEXT:  %6 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: object, "bar": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:  %9 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %10 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %11 = HBCCallNInst (:any) %7: any, empty: any, empty: any, %9: undefined, %10: undefined, %8: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [b]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:function_end

// CHECK:function daa(a: any): any
// CHECK-NEXT:frame = [a: any, b: any, daa_capture: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %daa(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: undefined, [b]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %daa_capture(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [daa_capture]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:  %9 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  %10 = BinaryAddInst (:any) %8: any, %9: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: any, [b]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [daa_capture]: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function daa_capture(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %daa(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %daa_capture(): any, %0: environment
// CHECK-NEXT:  %2 = LIRResolveScopeInst (:environment) %daa(): any, %1: environment, 1: number
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %2: environment, [b@daa]: any
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHKOPT:function global(): any
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHKOPT-NEXT:       DeclareGlobalVarInst "a": string
// CHKOPT-NEXT:       DeclareGlobalVarInst "bar": string
// CHKOPT-NEXT:       DeclareGlobalVarInst "foo": string
// CHKOPT-NEXT:       DeclareGlobalVarInst "daa": string
// CHKOPT-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %bar(): functionCode
// CHKOPT-NEXT:  %6 = HBCGetGlobalObjectInst (:object)
// CHKOPT-NEXT:       StorePropertyStrictInst %5: object, %6: object, "bar": string
// CHKOPT-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHKOPT-NEXT:       StorePropertyStrictInst %8: object, %6: object, "foo": string
// CHKOPT-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %daa(): functionCode
// CHKOPT-NEXT:        StorePropertyStrictInst %10: object, %6: object, "daa": string
// CHKOPT-NEXT:  %12 = HBCLoadConstInst (:number) 5: number
// CHKOPT-NEXT:        StorePropertyStrictInst %12: number, %6: object, "a": string
// CHKOPT-NEXT:  %14 = LoadPropertyInst (:any) %6: object, "a": string
// CHKOPT-NEXT:        ReturnInst %14: any
// CHKOPT-NEXT:function_end

// CHKOPT:function bar(a: any): any
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHKOPT-NEXT:       ReturnInst %0: any
// CHKOPT-NEXT:function_end

// CHKOPT:function foo(a: any): any
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCGetGlobalObjectInst (:object)
// CHKOPT-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "bar": string
// CHKOPT-NEXT:  %2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKOPT-NEXT:  %3 = LoadParamInst (:any) %a: any
// CHKOPT-NEXT:  %4 = HBCCallNInst (:any) %1: any, empty: any, empty: any, %2: undefined, %2: undefined, %3: any
// CHKOPT-NEXT:       ReturnInst %4: any
// CHKOPT-NEXT:function_end

// CHKOPT:function daa(a: any): object
// CHKOPT-NEXT:frame = [b: undefined|string|number]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHKOPT-NEXT:  %1 = CreateScopeInst (:environment) %daa(): any, %0: environment
// CHKOPT-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHKOPT-NEXT:  %3 = HBCLoadConstInst (:number) 1: number
// CHKOPT-NEXT:  %4 = BinaryAddInst (:string|number) %2: any, %3: number
// CHKOPT-NEXT:       StoreFrameInst %1: environment, %4: string|number, [b]: undefined|string|number
// CHKOPT-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %daa_capture(): functionCode
// CHKOPT-NEXT:       ReturnInst %6: object
// CHKOPT-NEXT:function_end

// CHKOPT:function daa_capture(): undefined|string|number
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %daa(): any, %parentScope: environment
// CHKOPT-NEXT:  %1 = LoadFrameInst (:undefined|string|number) %0: environment, [b@daa]: undefined|string|number
// CHKOPT-NEXT:       ReturnInst %1: undefined|string|number
// CHKOPT-NEXT:function_end
