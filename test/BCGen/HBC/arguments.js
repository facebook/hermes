/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheckOrRegen --match-full-lines %s

function count() {
  return arguments.length;
}

function select(x) {
  return arguments[x+1];
}

function build() {
  return arguments;
}

function buffalobuffalo() {
  if(arguments) {
    return arguments + arguments;
  }
  return arguments;
}

function check_phi_handling(x) {
  return x ? [1] : arguments;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "count": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "select": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "build": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "buffalobuffalo": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "check_phi_handling": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg0, %count(): functionCode
// CHECK-NEXT:  $Reg1 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "count": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg0, %select(): functionCode
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "select": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg0, %build(): functionCode
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "build": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg0, %buffalobuffalo(): functionCode
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "buffalobuffalo": string
// CHECK-NEXT:  $Reg0 = CreateFunctionInst (:object) $Reg0, %check_phi_handling(): functionCode
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg1, "check_phi_handling": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function count(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = AllocStackInst (:undefined) $arguments: any
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg1 = StoreStackInst $Reg1, $Reg0
// CHECK-NEXT:  $Reg0 = LoadStackInst (:undefined) $Reg0
// CHECK-NEXT:  $Reg0 = HBCGetArgumentsLengthInst (:number) $Reg0
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function select(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = AllocStackInst (:undefined|object) $arguments: any
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = StoreStackInst $Reg0, $Reg1
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg0 = BinaryAddInst (:string|number) $Reg2, $Reg0
// CHECK-NEXT:  $Reg0 = HBCGetArgumentsPropByValLooseInst (:any) $Reg0, $Reg1
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function build(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = AllocStackInst (:undefined|object) $arguments: any
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg1 = StoreStackInst $Reg1, $Reg0
// CHECK-NEXT:  $Reg1 = HBCReifyArgumentsLooseInst $Reg0
// CHECK-NEXT:  $Reg0 = LoadStackInst (:undefined|object) $Reg0
// CHECK-NEXT:  $Reg0 = UnionNarrowTrustedInst (:object) $Reg0
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function buffalobuffalo(): string|number|bigint
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = AllocStackInst (:undefined|object) $arguments: any
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg1 = StoreStackInst $Reg1, $Reg0
// CHECK-NEXT:  $Reg1 = HBCReifyArgumentsLooseInst $Reg0
// CHECK-NEXT:  $Reg0 = LoadStackInst (:undefined|object) $Reg0
// CHECK-NEXT:  $Reg0 = UnionNarrowTrustedInst (:object) $Reg0
// CHECK-NEXT:  $Reg0 = BinaryAddInst (:string|number|bigint) $Reg0, $Reg0
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function check_phi_handling(x: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = AllocStackInst (:undefined|object) $arguments: any
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg1 = StoreStackInst $Reg1, $Reg0
// CHECK-NEXT:  $Reg1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg1 = CondBranchInst $Reg1, %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg1 = AllocArrayInst (:object) 1: number, 1: number
// CHECK-NEXT:  $Reg0 = MovInst (:object) $Reg1
// CHECK-NEXT:  $Reg1 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = PhiInst (:object) $Reg0, %BB1, $Reg0, %BB3
// CHECK-NEXT:  $Reg0 = MovInst (:object) $Reg0
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg1 = HBCReifyArgumentsLooseInst $Reg0
// CHECK-NEXT:  $Reg0 = LoadStackInst (:undefined|object) $Reg0
// CHECK-NEXT:  $Reg0 = UnionNarrowTrustedInst (:object) $Reg0
// CHECK-NEXT:  $Reg0 = MovInst (:object) $Reg0
// CHECK-NEXT:  $Reg1 = BranchInst %BB2
// CHECK-NEXT:function_end
