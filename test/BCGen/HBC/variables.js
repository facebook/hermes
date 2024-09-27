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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
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

// CHECK:scope %VS1 [a: any]

// CHECK:function bar(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.a]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS1.a]: any
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [a: any, b: any]

// CHECK:function foo(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.a]: any
// CHECK-NEXT:  %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: undefined, [%VS2.b]: any
// CHECK-NEXT:  %6 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: object, "bar": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS2.a]: any
// CHECK-NEXT:  %9 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %10 = HBCCallNInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, %9: undefined, %8: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: any, [%VS2.b]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS2.b]: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [a: any, b: any, daa_capture: any]

// CHECK:function daa(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS3.a]: any
// CHECK-NEXT:  %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: undefined, [%VS3.b]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %daa_capture(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS3.daa_capture]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS3.a]: any
// CHECK-NEXT:  %9 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  %10 = BinaryAddInst (:any) %8: any, %9: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: any, [%VS3.b]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS3.daa_capture]: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:function daa_capture(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [%VS3.b]: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHKOPT:scope %VS0 []

// CHKOPT:function global(): any
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
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
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHKOPT-NEXT:       ReturnInst %0: any
// CHKOPT-NEXT:function_end

// CHKOPT:function foo(a: any): any
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCGetGlobalObjectInst (:object)
// CHKOPT-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "bar": string
// CHKOPT-NEXT:  %2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKOPT-NEXT:  %3 = LoadParamInst (:any) %a: any
// CHKOPT-NEXT:  %4 = HBCCallNInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, %2: undefined, %3: any
// CHKOPT-NEXT:       ReturnInst %4: any
// CHKOPT-NEXT:function_end

// CHKOPT:scope %VS1 [b: undefined|string|number]

// CHKOPT:function daa(a: any): object
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCCreateFunctionEnvironmentInst (:environment) %VS1: any, %parentScope: environment
// CHKOPT-NEXT:  %1 = LoadParamInst (:any) %a: any
// CHKOPT-NEXT:  %2 = HBCLoadConstInst (:number) 1: number
// CHKOPT-NEXT:  %3 = BinaryAddInst (:string|number) %1: any, %2: number
// CHKOPT-NEXT:       StoreFrameInst %0: environment, %3: string|number, [%VS1.b]: undefined|string|number
// CHKOPT-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %daa_capture(): functionCode
// CHKOPT-NEXT:       ReturnInst %5: object
// CHKOPT-NEXT:function_end

// CHKOPT:function daa_capture(): undefined|string|number
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHKOPT-NEXT:  %1 = LoadFrameInst (:undefined|string|number) %0: environment, [%VS1.b]: undefined|string|number
// CHKOPT-NEXT:       ReturnInst %1: undefined|string|number
// CHKOPT-NEXT:function_end
