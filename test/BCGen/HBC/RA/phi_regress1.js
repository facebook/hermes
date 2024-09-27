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
// CHKRA-NEXT:  $Reg0 = DeclareGlobalVarInst "glob": string
// CHKRA-NEXT:  $Reg0 = DeclareGlobalVarInst "bad": string
// CHKRA-NEXT:  $Reg0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKRA-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg0, %bad(): functionCode
// CHKRA-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg1 = StorePropertyLooseInst $Reg1, $Reg0, "bad": string
// CHKRA-NEXT:  $Reg4 = HBCLoadConstInst (:null) null: null
// CHKRA-NEXT:  $Reg1 = StorePropertyLooseInst $Reg4, $Reg0, "glob": string
// CHKRA-NEXT:  $Reg3 = LoadPropertyInst (:any) $Reg0, "bad": string
// CHKRA-NEXT:  $Reg2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg1 = HBCLoadConstInst (:string) "foo": string
// CHKRA-NEXT:  $Reg1 = HBCCallNInst (:any) $Reg3, empty: any, false: boolean, empty: any, undefined: undefined, $Reg2, $Reg1, $Reg4
// CHKRA-NEXT:  $Reg3 = TryLoadGlobalPropertyInst (:any) $Reg0, "print": string
// CHKRA-NEXT:  $Reg1 = HBCLoadConstInst (:string) "phi": string
// CHKRA-NEXT:  $Reg1 = HBCCallNInst (:any) $Reg3, empty: any, false: boolean, empty: any, undefined: undefined, $Reg2, $Reg1
// CHKRA-NEXT:  $Reg1 = TryLoadGlobalPropertyInst (:any) $Reg0, "print": string
// CHKRA-NEXT:  $Reg0 = LoadPropertyInst (:any) $Reg0, "glob": string
// CHKRA-NEXT:  $Reg0 = HBCCallNInst (:any) $Reg1, empty: any, false: boolean, empty: any, undefined: undefined, $Reg2, $Reg0
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end

// CHKRA:function bad(param1: any, param2: any): null
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg3 = LoadParamInst (:any) %param1: any
// CHKRA-NEXT:  $Reg2 = LoadParamInst (:any) %param2: any
// CHKRA-NEXT:  $Reg0 = HBCLoadConstInst (:number) 0: number
// CHKRA-NEXT:  $Reg3 = MovInst (:any) $Reg3
// CHKRA-NEXT:  $Reg1 = BranchInst %BB1
// CHKRA-NEXT:%BB1:
// CHKRA-NEXT:  $Reg3 = PhiInst (:any) $Reg3, %BB0, $Reg3, %BB3
// CHKRA-NEXT:  $Reg1 = MovInst (:any) $Reg3
// CHKRA-NEXT:  $Reg4 = CondBranchInst $Reg2, %BB2, %BB3
// CHKRA-NEXT:%BB2:
// CHKRA-NEXT:  $Reg4 = StorePropertyLooseInst $Reg0, $Reg2, "foo": string
// CHKRA-NEXT:  $Reg4 = BranchInst %BB3
// CHKRA-NEXT:%BB3:
// CHKRA-NEXT:  $Reg3 = MovInst (:any) $Reg2
// CHKRA-NEXT:  $Reg0 = CondBranchInst $Reg3, %BB1, %BB4
// CHKRA-NEXT:%BB4:
// CHKRA-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg0, "glob": string
// CHKRA-NEXT:  $Reg0 = HBCLoadConstInst (:null) null: null
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end
