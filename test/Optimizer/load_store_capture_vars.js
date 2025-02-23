/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O -fno-inline | %FileCheckOrRegen %s --match-full-lines

function store_x_not_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { y  = 12; }
  k();
  x = 9;
  return x;
}

function store_x_is_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { y  = x; }
  k();
  x = 9;
  return x;
}

function load_x_not_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { y  = 12; }
  k();
  return x;
}

function load_x_is_captured() {
  var y = 3;
  var x = 4;
  var k = function foo() { x = 12; }
  k();
  return x;
}

function load_x_captured_as_load() {
  var y = 3;
  var x = 4;
  var k = function foo() {  print(x); }
  k();
  return x;
}

function abort() {
  throw 42;
}

function foo(x) {
  var a = x;
    try {
      a += 1;
      abort();
      a += 2;
    } catch (e) {
      a += 100;
    }
    return a;
}

function postponed_store_in_use_block(x) {
  switch (0) {
    // This caused a crash because StackPromotion inserted an invalid load.
    case 0: 0 + function() { return x; }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "store_x_not_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "store_x_is_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_x_not_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_x_is_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_x_captured_as_load": string
// CHECK-NEXT:       DeclareGlobalVarInst "abort": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "postponed_store_in_use_block": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %VS0: any, %store_x_not_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "store_x_not_captured": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %VS0: any, %store_x_is_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "store_x_is_captured": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %VS0: any, %load_x_not_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "load_x_not_captured": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %VS0: any, %load_x_is_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "load_x_is_captured": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %VS0: any, %load_x_captured_as_load(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "load_x_captured_as_load": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %0: environment, %VS0: any, %abort(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "abort": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "foo": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %0: environment, %VS0: any, %postponed_store_in_use_block(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %23: object, globalObject: object, "postponed_store_in_use_block": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function store_x_not_captured(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %"foo 1#"(): functionCode
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %"foo 1#"(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 9: number
// CHECK-NEXT:function_end

// CHECK:function store_x_is_captured(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %"foo 2#"(): functionCode
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %"foo 2#"(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 9: number
// CHECK-NEXT:function_end

// CHECK:function load_x_not_captured(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %"foo 3#"(): functionCode
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %"foo 3#"(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 4: number
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [x: number]

// CHECK:function load_x_is_captured(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, 4: number, [%VS1.x]: number
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %VS1: any, %"foo 4#"(): functionCode
// CHECK-NEXT:  %4 = CallInst (:undefined) %3: object, %"foo 4#"(): functionCode, true: boolean, %1: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %5 = LoadFrameInst (:number) %1: environment, [%VS1.x]: number
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function load_x_captured_as_load(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %"foo 5#"(): functionCode
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %"foo 5#"(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 4: number
// CHECK-NEXT:function_end

// CHECK:function abort(): any [noReturn]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ThrowInst 42: number
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $a: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreStackInst %1: any, %0: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst (:any)
// CHECK-NEXT:  %5 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %6 = BinaryAddInst (:string|number) %5: any, 100: number
// CHECK-NEXT:       StoreStackInst %6: string|number, %0: any
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = LoadStackInst (:any) %0: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %12 = BinaryAddInst (:string|number) %11: any, 1: number
// CHECK-NEXT:        StoreStackInst %12: string|number, %0: any
// CHECK-NEXT:  %14 = LoadPropertyInst (:any) globalObject: object, "abort": string
// CHECK-NEXT:  %15 = CallInst (:any) %14: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %16 = BinaryAddInst (:string|number) %12: string|number, 2: number
// CHECK-NEXT:        StoreStackInst %16: string|number, %0: any
// CHECK-NEXT:        TryEndInst %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: any]

// CHECK:function postponed_store_in_use_block(x: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.x]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %VS2: any, %""(): functionCode
// CHECK-NEXT:  %5 = BinaryAddInst (:string|number) 0: number, %4: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 1#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 2#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 3#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 4#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:       StoreFrameInst %0: environment, 12: number, [%VS1.x]: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 5#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:any) %0: environment, [%VS2.x]: any
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end
