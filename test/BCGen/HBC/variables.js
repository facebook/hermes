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
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHECK-NEXT:       DeclareGlobalVarInst "a": string
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "daa": string
// CHECK-NEXT:  %5 = HBCCreateFunctionInst (:object) %bar(): any, %0: any
// CHECK-NEXT:  %6 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:       StorePropertyStrictInst %5: object, %6: object, "bar": string
// CHECK-NEXT:  %8 = HBCCreateFunctionInst (:object) %foo(): any, %0: any
// CHECK-NEXT:  %9 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:        StorePropertyStrictInst %8: object, %9: object, "foo": string
// CHECK-NEXT:  %11 = HBCCreateFunctionInst (:object) %daa(): any, %0: any
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
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       HBCStoreToEnvironmentInst %0: any, %1: any, [a]: any
// CHECK-NEXT:  %3 = HBCLoadFromEnvironmentInst (:any) %0: any, [a]: any
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       HBCStoreToEnvironmentInst %0: any, %1: any, [a]: any
// CHECK-NEXT:  %3 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       HBCStoreToEnvironmentInst %0: any, %3: undefined, [b]: any
// CHECK-NEXT:  %5 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: object, "bar": string
// CHECK-NEXT:  %7 = HBCLoadFromEnvironmentInst (:any) %0: any, [a]: any
// CHECK-NEXT:  %8 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %9 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %10 = HBCCallNInst (:any) %6: any, empty: any, empty: any, %8: undefined, %9: undefined, %7: any
// CHECK-NEXT:        HBCStoreToEnvironmentInst %0: any, %10: any, [b]: any
// CHECK-NEXT:  %12 = HBCLoadFromEnvironmentInst (:any) %0: any, [b]: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function daa(a: any): any
// CHECK-NEXT:frame = [a: any, b: any, daa_capture: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       HBCStoreToEnvironmentInst %0: any, %1: any, [a]: any
// CHECK-NEXT:  %3 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       HBCStoreToEnvironmentInst %0: any, %3: undefined, [b]: any
// CHECK-NEXT:  %5 = HBCCreateFunctionInst (:object) %daa_capture(): any, %0: any
// CHECK-NEXT:       HBCStoreToEnvironmentInst %0: any, %5: object, [daa_capture]: any
// CHECK-NEXT:  %7 = HBCLoadFromEnvironmentInst (:any) %0: any, [a]: any
// CHECK-NEXT:  %8 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %7: any, %8: number
// CHECK-NEXT:        HBCStoreToEnvironmentInst %0: any, %9: any, [b]: any
// CHECK-NEXT:  %11 = HBCLoadFromEnvironmentInst (:any) %0: any, [daa_capture]: any
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:function daa_capture(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHECK-NEXT:  %1 = HBCResolveEnvironment (:any) %daa(): any
// CHECK-NEXT:  %2 = HBCLoadFromEnvironmentInst (:any) %1: any, [b@daa]: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHKOPT:function global(): any
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKOPT-NEXT:       DeclareGlobalVarInst "a": string
// CHKOPT-NEXT:       DeclareGlobalVarInst "bar": string
// CHKOPT-NEXT:       DeclareGlobalVarInst "foo": string
// CHKOPT-NEXT:       DeclareGlobalVarInst "daa": string
// CHKOPT-NEXT:  %5 = HBCCreateFunctionInst (:object) %bar(): any, %0: any
// CHKOPT-NEXT:  %6 = HBCGetGlobalObjectInst (:object)
// CHKOPT-NEXT:       StorePropertyStrictInst %5: object, %6: object, "bar": string
// CHKOPT-NEXT:  %8 = HBCCreateFunctionInst (:object) %foo(): any, %0: any
// CHKOPT-NEXT:       StorePropertyStrictInst %8: object, %6: object, "foo": string
// CHKOPT-NEXT:  %10 = HBCCreateFunctionInst (:object) %daa(): object, %0: any
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
// CHKOPT-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKOPT-NEXT:  %1 = LoadParamInst (:any) %a: any
// CHKOPT-NEXT:  %2 = HBCLoadConstInst (:number) 1: number
// CHKOPT-NEXT:  %3 = BinaryAddInst (:string|number) %1: any, %2: number
// CHKOPT-NEXT:       HBCStoreToEnvironmentInst %0: any, %3: string|number, [b]: undefined|string|number
// CHKOPT-NEXT:  %5 = HBCCreateFunctionInst (:object) %daa_capture(): undefined|string|number, %0: any
// CHKOPT-NEXT:       ReturnInst %5: object
// CHKOPT-NEXT:function_end

// CHKOPT:function daa_capture(): undefined|string|number
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCResolveEnvironment (:any) %daa(): any
// CHKOPT-NEXT:  %1 = HBCLoadFromEnvironmentInst (:undefined|string|number) %0: any, [b@daa]: undefined|string|number
// CHKOPT-NEXT:       ReturnInst %1: undefined|string|number
// CHKOPT-NEXT:function_end
