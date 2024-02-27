/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen %s

function bar() { }

// The first stores to k1, k2, k3 can be eliminated since the variables haven't
// been captured yet.
function main(p) {
  var k1 = bar();
  var k2 = bar();
  var k3 = bar();

  return function () { return k1 + k2 + k3 }
}

// Make sure that we are not eliminating the "store-42" in test2, because the
// call to o() may observe it.
function outer() {
    var envVar;

    function setValue(v) {
        envVar = v;
    }

    function getValue() {
      return envVar;
    }

    function test1() {
      envVar = 42;
      envVar = 87;
    }

    function test2(o) {
      envVar = 42;
      o();
      envVar = 87;
    }

    return [getValue, setValue, test1, test2, envVar]
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %bar(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "bar": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %main(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "main": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %outer(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "outer": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(p: any): object
// CHECK-NEXT:frame = [k1: any, k2: any, k3: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %main(): any, %0: environment
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "bar": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: any, [k1]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) globalObject: object, "bar": string
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [k2]: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "bar": string
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: any, [k3]: any
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %1: environment, %""(): functionCode
// CHECK-NEXT:        ReturnInst %11: object
// CHECK-NEXT:function_end

// CHECK:function outer(): object
// CHECK-NEXT:frame = [envVar: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %outer(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [envVar]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %setValue(): functionCode
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %getValue(): functionCode
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %test1(): functionCode
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %test2(): functionCode
// CHECK-NEXT:  %7 = AllocArrayInst (:object) 5: number
// CHECK-NEXT:       StoreOwnPropertyInst %4: object, %7: object, 0: number, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst %3: object, %7: object, 1: number, true: boolean
// CHECK-NEXT:        StoreOwnPropertyInst %5: object, %7: object, 2: number, true: boolean
// CHECK-NEXT:        StoreOwnPropertyInst %6: object, %7: object, 3: number, true: boolean
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [envVar]: any
// CHECK-NEXT:        StoreOwnPropertyInst %12: any, %7: object, 4: number, true: boolean
// CHECK-NEXT:        ReturnInst %7: object
// CHECK-NEXT:function_end

// CHECK:function ""(): string|number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %main(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:any) %0: environment, [k1@main]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [k2@main]: any
// CHECK-NEXT:  %3 = BinaryAddInst (:string|number|bigint) %1: any, %2: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %0: environment, [k3@main]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:string|number|bigint) %3: string|number|bigint, %4: any
// CHECK-NEXT:       ReturnInst %5: string|number|bigint
// CHECK-NEXT:function_end

// CHECK:function setValue(v: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %outer(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %v: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [envVar@outer]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function getValue(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %outer(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:any) %0: environment, [envVar@outer]: any
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:function test1(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %outer(): any, %parentScope: environment
// CHECK-NEXT:       StoreFrameInst %0: environment, 87: number, [envVar@outer]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test2(o: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %outer(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %0: environment, 42: number, [envVar@outer]: any
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %0: environment, 87: number, [envVar@outer]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
