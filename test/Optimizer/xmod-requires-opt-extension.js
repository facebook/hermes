/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// This tests simulates the Metro requires tranform, and shows that
// requires calls in a module are compiled to a bytecode instruction.

// The important case is the call in the module factory function
// modFact1, which does "require(0)" (twice).  These calls should be
// annotated with the [metro-require] attribute, and should be
// translated into the CallRequire/RequireCacheMiss bytecode instruction pair.

$SHBuiltin.moduleFactory(10, function(global, require) {
  // This just demonstrates that the require opt is in place.  The call
  // to require should have the [metro-require] attribute in the IR.
  function simple() {
    return require(17).x;
  }

  function testPhi(p) {
    var locReq;
    if (p) {
      locReq = require;
    } else {
      locReq = require;
    }
    // The phi function for locReq should be found to have inputs that
    // are all require.
    // So the call will have the [metro-require] attribute in the IR.
    return locReq(17).x;
  }

  var reqAlias = require;

  // The use of try/catch causes some variables to not be eligible for SSA.
  // But all stores to "req" are require (perhaps through a level of def-use
  // chain).  So the two calls vai "req" will get the [metro-require] attribute.
  function testStack(p) {
    var req = reqAlias;
    try {
      if (p) {
        req = require;
        throw 3;
      }
      return req(17).x;
    } catch (e) {
      return req(17).x;
    }
  }
  return {simple: simple, testPhi: testPhi, testStack: testStack};
})

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %""(): functionCode
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [require: any, reqAlias: any]

// CHECK:function ""(global: any, require: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %require: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [%VS1.require]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS1: any, %simple(): functionCode
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %VS1: any, %testPhi(): functionCode
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS1: any, %testStack(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [%VS1.reqAlias]: any
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) empty: any, "simple": string, null: null, "testPhi": string, null: null, "testStack": string, null: null
// CHECK-NEXT:       PrStoreInst %3: object, %7: object, 0: number, "simple": string, false: boolean
// CHECK-NEXT:       PrStoreInst %4: object, %7: object, 1: number, "testPhi": string, false: boolean
// CHECK-NEXT:        PrStoreInst %5: object, %7: object, 2: number, "testStack": string, false: boolean
// CHECK-NEXT:        ReturnInst %7: object
// CHECK-NEXT:function_end

// CHECK:function simple(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:any) %0: environment, [%VS1.require]: any
// CHECK-NEXT:  %2 = CallInst [metro-require] (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 17: number
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "x": string
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:function testPhi(p: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       CondBranchInst %1: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %0: environment, [%VS1.require]: any
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %0: environment, [%VS1.require]: any
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = PhiInst (:any) %3: any, %BB1, %5: any, %BB2
// CHECK-NEXT:  %8 = CallInst [metro-require] (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 17: number
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "x": string
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function testStack(p: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = AllocStackInst (:any) $req: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %0: environment, [%VS1.reqAlias]: any
// CHECK-NEXT:       StoreStackInst %3: any, %1: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = CatchInst (:any)
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %8 = CallInst [metro-require] (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 17: number
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "x": string
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        CondBranchInst %2: any, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %0: environment, [%VS1.require]: any
// CHECK-NEXT:        StoreStackInst %12: any, %1: any
// CHECK-NEXT:        ThrowInst 3: number, %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %16 = CallInst [metro-require] (:any) %15: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 17: number
// CHECK-NEXT:  %17 = LoadPropertyInst (:any) %16: any, "x": string
// CHECK-NEXT:        TryEndInst %BB1, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %17: any
// CHECK-NEXT:function_end
