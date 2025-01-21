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
// CHECK-NEXT:  {r0}      %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:                 DeclareGlobalVarInst "count": string
// CHECK-NEXT:                 DeclareGlobalVarInst "select": string
// CHECK-NEXT:                 DeclareGlobalVarInst "build": string
// CHECK-NEXT:                 DeclareGlobalVarInst "buffalobuffalo": string
// CHECK-NEXT:                 DeclareGlobalVarInst "check_phi_handling": string
// CHECK-NEXT:  {r2}      %6 = CreateFunctionInst (:object) {r0} %0: environment, %count(): functionCode
// CHECK-NEXT:  {r1}      %7 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {r2} %6: object, {r1} %7: object, "count": string
// CHECK-NEXT:  {r2}      %9 = CreateFunctionInst (:object) {r0} %0: environment, %select(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r2} %9: object, {r1} %7: object, "select": string
// CHECK-NEXT:  {r2}     %11 = CreateFunctionInst (:object) {r0} %0: environment, %build(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r2} %11: object, {r1} %7: object, "build": string
// CHECK-NEXT:  {r2}     %13 = CreateFunctionInst (:object) {r0} %0: environment, %buffalobuffalo(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r2} %13: object, {r1} %7: object, "buffalobuffalo": string
// CHECK-NEXT:  {r0}     %15 = CreateFunctionInst (:object) {r0} %0: environment, %check_phi_handling(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %15: object, {r1} %7: object, "check_phi_handling": string
// CHECK-NEXT:  {np0}    %17 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %17: undefined
// CHECK-NEXT:function_end

// CHECK:function count(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {np0}     %0 = AllocStackInst (:undefined) $arguments: any
// CHECK-NEXT:  {np1}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 StoreStackInst {np1} %1: undefined, {np0} %0: undefined
// CHECK-NEXT:  {np0}     %3 = LoadStackInst (:undefined) {np0} %0: undefined
// CHECK-NEXT:  {n0}      %4 = HBCGetArgumentsLengthInst (:number) {np0} %3: undefined
// CHECK-NEXT:                 ReturnInst {n0} %4: number
// CHECK-NEXT:function_end

// CHECK:function select(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = AllocStackInst (:undefined|object) $arguments: any
// CHECK-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 StoreStackInst {np0} %1: undefined, {r1} %0: undefined|object
// CHECK-NEXT:  {r0}      %3 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  {n0}      %4 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  {r0}      %5 = BinaryAddInst (:string|number) {r0} %3: any, {n0} %4: number
// CHECK-NEXT:  {r0}      %6 = HBCGetArgumentsPropByValLooseInst (:any) {r0} %5: string|number, {r1} %0: undefined|object
// CHECK-NEXT:                 ReturnInst {r0} %6: any
// CHECK-NEXT:function_end

// CHECK:function build(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = AllocStackInst (:undefined|object) $arguments: any
// CHECK-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 StoreStackInst {np0} %1: undefined, {r0} %0: undefined|object
// CHECK-NEXT:                 HBCReifyArgumentsLooseInst {r0} %0: undefined|object
// CHECK-NEXT:  {r0}      %4 = LoadStackInst (:undefined|object) {r0} %0: undefined|object
// CHECK-NEXT:  {r0}      %5 = UnionNarrowTrustedInst (:object) {r0} %4: undefined|object
// CHECK-NEXT:                 ReturnInst {r0} %5: object
// CHECK-NEXT:function_end

// CHECK:function buffalobuffalo(): string|number|bigint
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = AllocStackInst (:undefined|object) $arguments: any
// CHECK-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 StoreStackInst {np0} %1: undefined, {r0} %0: undefined|object
// CHECK-NEXT:                 HBCReifyArgumentsLooseInst {r0} %0: undefined|object
// CHECK-NEXT:  {r0}      %4 = LoadStackInst (:undefined|object) {r0} %0: undefined|object
// CHECK-NEXT:  {r0}      %5 = UnionNarrowTrustedInst (:object) {r0} %4: undefined|object
// CHECK-NEXT:  {r0}      %6 = BinaryAddInst (:string|number|bigint) {r0} %5: object, {r0} %5: object
// CHECK-NEXT:                 ReturnInst {r0} %6: string|number|bigint
// CHECK-NEXT:function_end

// CHECK:function check_phi_handling(x: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = AllocStackInst (:undefined|object) $arguments: any
// CHECK-NEXT:  {np0}     %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 StoreStackInst {np0} %1: undefined, {r0} %0: undefined|object
// CHECK-NEXT:  {r1}      %3 = LoadParamInst (:any) %x: any
// CHECK-NEXT:                 CondBranchInst {r1} %3: any, %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {r1}      %5 = AllocArrayInst (:object) 1: number, 1: number
// CHECK-NEXT:  {r0}      %6 = MovInst (:object) {r1} %5: object
// CHECK-NEXT:                 BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {r0}      %8 = PhiInst (:object) {r0} %6: object, %BB1, {r0} %14: object, %BB3
// CHECK-NEXT:  {r0}      %9 = MovInst (:object) {r0} %8: object
// CHECK-NEXT:                 ReturnInst {r0} %9: object
// CHECK-NEXT:%BB3:
// CHECK-NEXT:                 HBCReifyArgumentsLooseInst {r0} %0: undefined|object
// CHECK-NEXT:  {r0}     %12 = LoadStackInst (:undefined|object) {r0} %0: undefined|object
// CHECK-NEXT:  {r0}     %13 = UnionNarrowTrustedInst (:object) {r0} %12: undefined|object
// CHECK-NEXT:  {r0}     %14 = MovInst (:object) {r0} %13: object
// CHECK-NEXT:                 BranchInst %BB2
// CHECK-NEXT:function_end
