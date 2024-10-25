/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes --target=HBC -O -dump-ra %s | %FileCheckOrRegen %s --check-prefix=CHKRA --match-full-lines

// Using a Phi in a successor of a Phi predecessor block:
// B0:
//   ...
// B1:
//   %p = PhiInst %a, B0, %b, B2
//   ...
// B2:
//   CondBranchInst ..., B1, B3
// B3:
//   Use %p

var glob = null;

function bad(param1, param2) {
    for (;;) {
        if (param2)
            param2.foo = 0;
        if (!param2) {
            glob = param1;
            return null;
        }
        param1 = param2;
    }
    return null;
}

bad("foo", null);
print("phi");
print(glob);

// Auto-generated content below. Please do not modify manually.

// CHKRA:scope %VS0 []

// CHKRA:function global(): any
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:                 DeclareGlobalVarInst "glob": string
// CHKRA-NEXT:                 DeclareGlobalVarInst "bad": string
// CHKRA-NEXT:  {r0}      %2 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKRA-NEXT:  {r1}      %3 = CreateFunctionInst (:object) {r0} %2: environment, %bad(): functionCode
// CHKRA-NEXT:  {r0}      %4 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:                 StorePropertyLooseInst {r1} %3: object, {r0} %4: object, "bad": string
// CHKRA-NEXT:  {np1}     %6 = HBCLoadConstInst (:null) null: null
// CHKRA-NEXT:                 StorePropertyLooseInst {np1} %6: null, {r0} %4: object, "glob": string
// CHKRA-NEXT:  {r2}      %8 = LoadPropertyInst (:any) {r0} %4: object, "bad": string
// CHKRA-NEXT:  {np0}     %9 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  {r1}     %10 = HBCLoadConstInst (:string) "foo": string
// CHKRA-NEXT:  {r1}     %11 = HBCCallNInst (:any) {r2} %8: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %9: undefined, {r1} %10: string, {np1} %6: null
// CHKRA-NEXT:  {r2}     %12 = TryLoadGlobalPropertyInst (:any) {r0} %4: object, "print": string
// CHKRA-NEXT:  {r1}     %13 = HBCLoadConstInst (:string) "phi": string
// CHKRA-NEXT:  {r1}     %14 = HBCCallNInst (:any) {r2} %12: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %9: undefined, {r1} %13: string
// CHKRA-NEXT:  {r1}     %15 = TryLoadGlobalPropertyInst (:any) {r0} %4: object, "print": string
// CHKRA-NEXT:  {r0}     %16 = LoadPropertyInst (:any) {r0} %4: object, "glob": string
// CHKRA-NEXT:  {r0}     %17 = HBCCallNInst (:any) {r1} %15: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %9: undefined, {r0} %16: any
// CHKRA-NEXT:                 ReturnInst {r0} %17: any
// CHKRA-NEXT:function_end

// CHKRA:function bad(param1: any, param2: any): null
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  {r2}      %0 = LoadParamInst (:any) %param1: any
// CHKRA-NEXT:  {r0}      %1 = LoadParamInst (:any) %param2: any
// CHKRA-NEXT:  {n0}      %2 = HBCLoadConstInst (:number) 0: number
// CHKRA-NEXT:  {r2}      %3 = MovInst (:any) {r2} %0: any
// CHKRA-NEXT:                 BranchInst %BB1
// CHKRA-NEXT:%BB1:
// CHKRA-NEXT:  {r2}      %5 = PhiInst (:any) {r2} %3: any, %BB0, {r2} %10: any, %BB3
// CHKRA-NEXT:  {r1}      %6 = MovInst (:any) {r2} %5: any
// CHKRA-NEXT:                 CondBranchInst {r0} %1: any, %BB2, %BB3
// CHKRA-NEXT:%BB2:
// CHKRA-NEXT:                 StorePropertyLooseInst {n0} %2: number, {r0} %1: any, "foo": string
// CHKRA-NEXT:                 BranchInst %BB3
// CHKRA-NEXT:%BB3:
// CHKRA-NEXT:  {r2}     %10 = MovInst (:any) {r0} %1: any
// CHKRA-NEXT:                 CondBranchInst {r2} %10: any, %BB1, %BB4
// CHKRA-NEXT:%BB4:
// CHKRA-NEXT:  {r0}     %12 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:                 StorePropertyLooseInst {r1} %6: any, {r0} %12: object, "glob": string
// CHKRA-NEXT:  {np0}    %14 = HBCLoadConstInst (:null) null: null
// CHKRA-NEXT:                 ReturnInst {np0} %14: null
// CHKRA-NEXT:function_end
