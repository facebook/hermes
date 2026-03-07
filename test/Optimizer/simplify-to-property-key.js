/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir -O %s | %FileCheckOrRegen --match-full-lines %s

// ToPropertyKey on a value known to be a string should be eliminated.
function toPropertyKeyString(s) {
  s = s + "";
  class C {
    [s]() { return 1; }
  }
  return C;
}

// ToPropertyKey on a value of unknown type should not be eliminated.
function toPropertyKeyAny(s) {
  class C {
    [s]() { return 2; }
  }
  return C;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "toPropertyKeyString": string
// CHECK-NEXT:       DeclareGlobalVarInst "toPropertyKeyAny": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) empty: any, empty: any, %toPropertyKeyString(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "toPropertyKeyString": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) empty: any, empty: any, %toPropertyKeyAny(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "toPropertyKeyAny": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function toPropertyKeyString(s: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %s: any
// CHECK-NEXT:  %1 = AddEmptyStringInst (:string) %0: any
// CHECK-NEXT:  %2 = AllocStackInst (:object) $?anon_0_clsPrototype: any
// CHECK-NEXT:  %3 = CreateClassInst (:object) empty: any, empty: any, %C(): functionCode, empty: any, %2: object
// CHECK-NEXT:  %4 = LoadStackInst (:object) %2: object
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) empty: any, empty: any, %""(): functionCode
// CHECK-NEXT:  %6 = CallBuiltinInst (:any) [HermesBuiltin.setFunctionName]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: object, %1: string, 0: number
// CHECK-NEXT:       DefineOwnPropertyInst %5: object, %4: object, %1: string, false: boolean
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:function toPropertyKeyAny(s: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %s: any
// CHECK-NEXT:  %1 = AllocStackInst (:object) $?anon_0_clsPrototype: any
// CHECK-NEXT:  %2 = CreateClassInst (:object) empty: any, empty: any, %"C 1#"(): functionCode, empty: any, %1: object
// CHECK-NEXT:  %3 = LoadStackInst (:object) %1: object
// CHECK-NEXT:  %4 = ToPropertyKeyInst (:string|symbol) %0: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) empty: any, empty: any, %" 1#"(): functionCode
// CHECK-NEXT:  %6 = CallBuiltinInst (:any) [HermesBuiltin.setFunctionName]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: object, %4: string|symbol, 0: number
// CHECK-NEXT:       DefineOwnPropertyInst %5: object, %3: object, %4: string|symbol, false: boolean
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:base constructor C(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) %1: any
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:method ""(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:base constructor "C 1#"(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) %1: any
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:method " 1#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end
