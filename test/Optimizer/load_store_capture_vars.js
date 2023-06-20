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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "store_x_not_captured": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "store_x_is_captured": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "load_x_not_captured": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "load_x_is_captured": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "load_x_captured_as_load": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "abort": string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "postponed_store_in_use_block": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %store_x_not_captured(): number
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: object, globalObject: object, "store_x_not_captured": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %store_x_is_captured(): number
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: object, globalObject: object, "store_x_is_captured": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %load_x_not_captured(): number
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12: object, globalObject: object, "load_x_not_captured": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %load_x_is_captured(): number
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14: object, globalObject: object, "load_x_is_captured": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %load_x_captured_as_load(): number
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16: object, globalObject: object, "load_x_captured_as_load": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %abort(): any
// CHECK-NEXT:  %19 = StorePropertyLooseInst %18: object, globalObject: object, "abort": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:  %21 = StorePropertyLooseInst %20: object, globalObject: object, "foo": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %postponed_store_in_use_block(): undefined
// CHECK-NEXT:  %23 = StorePropertyLooseInst %22: object, globalObject: object, "postponed_store_in_use_block": string
// CHECK-NEXT:  %24 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function store_x_not_captured(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %"foo 1#"(): undefined
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %"foo 1#"(): undefined, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst 9: number
// CHECK-NEXT:function_end

// CHECK:function store_x_is_captured(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %"foo 2#"(): undefined
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %"foo 2#"(): undefined, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst 9: number
// CHECK-NEXT:function_end

// CHECK:function load_x_not_captured(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %"foo 3#"(): undefined
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %"foo 3#"(): undefined, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst 4: number
// CHECK-NEXT:function_end

// CHECK:function load_x_is_captured(): number
// CHECK-NEXT:frame = [x: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst 4: number, [x]: number
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %"foo 4#"(): undefined
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %"foo 4#"(): undefined, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %3 = LoadFrameInst (:number) [x]: number
// CHECK-NEXT:  %4 = ReturnInst %3: number
// CHECK-NEXT:function_end

// CHECK:function load_x_captured_as_load(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %"foo 5#"(): undefined
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %"foo 5#"(): undefined, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst 4: number
// CHECK-NEXT:function_end

// CHECK:function abort(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ThrowInst 42: number
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $a: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %2 = StoreStackInst %1: any, %0: any
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst (:any)
// CHECK-NEXT:  %5 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %6 = BinaryAddInst (:string|number) %5: any, 100: number
// CHECK-NEXT:  %7 = StoreStackInst %6: string|number, %0: any
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %10 = ReturnInst %9: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %12 = BinaryAddInst (:string|number) %11: any, 1: number
// CHECK-NEXT:  %13 = StoreStackInst %12: string|number, %0: any
// CHECK-NEXT:  %14 = LoadPropertyInst (:any) globalObject: object, "abort": string
// CHECK-NEXT:  %15 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %16 = BinaryAddInst (:string|number) %12: string|number, 2: number
// CHECK-NEXT:  %17 = StoreStackInst %16: string|number, %0: any
// CHECK-NEXT:  %18 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = TryEndInst
// CHECK-NEXT:  %20 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function postponed_store_in_use_block(x: any): undefined
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:  %3 = BinaryAddInst (:string|number) 0: number, %2: object
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 1#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 2#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 3#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 4#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst 12: number, [x@load_x_is_captured]: number
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "foo 5#"(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [x@postponed_store_in_use_block]: any
// CHECK-NEXT:  %1 = ReturnInst %0: any
// CHECK-NEXT:function_end
