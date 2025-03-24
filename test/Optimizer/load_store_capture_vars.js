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

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "store_x_not_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "store_x_is_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_x_not_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_x_is_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_x_captured_as_load": string
// CHECK-NEXT:       DeclareGlobalVarInst "abort": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "postponed_store_in_use_block": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) empty: any, empty: any, %store_x_not_captured(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "store_x_not_captured": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) empty: any, empty: any, %store_x_is_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "store_x_is_captured": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) empty: any, empty: any, %load_x_not_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "load_x_not_captured": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) empty: any, empty: any, %load_x_is_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "load_x_is_captured": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) empty: any, empty: any, %load_x_captured_as_load(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "load_x_captured_as_load": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) empty: any, empty: any, %abort(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "abort": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %20: object, globalObject: object, "foo": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) empty: any, empty: any, %postponed_store_in_use_block(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %22: object, globalObject: object, "postponed_store_in_use_block": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function store_x_not_captured(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %"foo 1#"(): functionCode
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %"foo 1#"(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 9: number
// CHECK-NEXT:function_end

// CHECK:function store_x_is_captured(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %"foo 2#"(): functionCode
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %"foo 2#"(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 9: number
// CHECK-NEXT:function_end

// CHECK:function load_x_not_captured(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %"foo 3#"(): functionCode
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %"foo 3#"(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 4: number
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [x: number]

// CHECK:function load_x_is_captured(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       StoreFrameInst %0: environment, 4: number, [%VS0.x]: number
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %"foo 4#"(): functionCode
// CHECK-NEXT:  %3 = CallInst (:undefined) %2: object, %"foo 4#"(): functionCode, true: boolean, %0: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %4 = LoadFrameInst (:number) %0: environment, [%VS0.x]: number
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:function_end

// CHECK:function load_x_captured_as_load(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %"foo 5#"(): functionCode
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %"foo 5#"(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined
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

// CHECK:scope %VS1 [x: any]

// CHECK:function postponed_store_in_use_block(x: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [%VS1.x]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS1: any, %""(): functionCode
// CHECK-NEXT:  %4 = BinaryAddInst (:string|number) 0: number, %3: object
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
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:       StoreFrameInst %0: environment, 12: number, [%VS0.x]: number
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
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:any) %0: environment, [%VS1.x]: any
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end
