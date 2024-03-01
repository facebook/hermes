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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "store_x_not_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "store_x_is_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_x_not_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_x_is_captured": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_x_captured_as_load": string
// CHECK-NEXT:       DeclareGlobalVarInst "abort": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "postponed_store_in_use_block": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %store_x_not_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "store_x_not_captured": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %store_x_is_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "store_x_is_captured": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %load_x_not_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "load_x_not_captured": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %load_x_is_captured(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "load_x_is_captured": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %load_x_captured_as_load(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "load_x_captured_as_load": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %0: environment, %abort(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "abort": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "foo": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %0: environment, %postponed_store_in_use_block(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %23: object, globalObject: object, "postponed_store_in_use_block": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function store_x_not_captured(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %store_x_not_captured(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %"foo 1#"(): functionCode
// CHECK-NEXT:  %3 = CallInst (:undefined) %2: object, %"foo 1#"(): functionCode, %1: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 9: number
// CHECK-NEXT:function_end

// CHECK:function store_x_is_captured(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %store_x_is_captured(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %"foo 2#"(): functionCode
// CHECK-NEXT:  %3 = CallInst (:undefined) %2: object, %"foo 2#"(): functionCode, %1: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 9: number
// CHECK-NEXT:function_end

// CHECK:function load_x_not_captured(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %load_x_not_captured(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %"foo 3#"(): functionCode
// CHECK-NEXT:  %3 = CallInst (:undefined) %2: object, %"foo 3#"(): functionCode, %1: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 4: number
// CHECK-NEXT:function_end

// CHECK:function load_x_is_captured(): number
// CHECK-NEXT:frame = [x: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %load_x_is_captured(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, 4: number, [x]: number
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %"foo 4#"(): functionCode
// CHECK-NEXT:  %4 = CallInst (:undefined) %3: object, %"foo 4#"(): functionCode, %1: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %5 = LoadFrameInst (:number) %1: environment, [x]: number
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function load_x_captured_as_load(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %load_x_captured_as_load(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %"foo 5#"(): functionCode
// CHECK-NEXT:  %3 = CallInst (:undefined) %2: object, %"foo 5#"(): functionCode, %1: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst 4: number
// CHECK-NEXT:function_end

// CHECK:function abort(): any [noReturn]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ThrowInst 42: number
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): any
// CHECK-NEXT:frame = []
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
// CHECK-NEXT:  %15 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %16 = BinaryAddInst (:string|number) %12: string|number, 2: number
// CHECK-NEXT:        StoreStackInst %16: string|number, %0: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function postponed_store_in_use_block(x: any): undefined
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %postponed_store_in_use_block(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %""(): functionCode
// CHECK-NEXT:  %5 = BinaryAddInst (:string|number) 0: number, %4: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 1#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 2#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 3#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 4#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %load_x_is_captured(): any, %parentScope: environment
// CHECK-NEXT:       StoreFrameInst %0: environment, 12: number, [x@load_x_is_captured]: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 5#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %postponed_store_in_use_block(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:any) %0: environment, [x@postponed_store_in_use_block]: any
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end
