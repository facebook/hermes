/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheckOrRegen --match-full-lines --check-prefix=RA   %s
// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines --check-prefix=EXEC %s

// This test failed with an assert because we tried to resolve the global scope
// instead of using GetGlobalScope. However, exception variables have limited
// scope, even in the global function, so a resolve is necessary.

var inner, e="global";

try {
  throw "local";
} catch (e) {
  local = function() { return e; };
}

// EXEC-LABEL: local
print(local());
// EXEC-NEXT: global
print(e);

// Auto-generated content below. Please do not modify manually.

// RA:scope %VS0 [e: any]

// RA:function global(): any
// RA-NEXT:%BB0:
// RA-NEXT:  $Reg1 = CreateScopeInst (:environment) %VS0: any, empty: any
// RA-NEXT:  $Reg0 = DeclareGlobalVarInst "inner": string
// RA-NEXT:  $Reg0 = DeclareGlobalVarInst "e": string
// RA-NEXT:  $Reg2 = HBCLoadConstInst (:string) "global": string
// RA-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// RA-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg0, "e": string
// RA-NEXT:  $Reg2 = TryStartInst %BB1, %BB2
// RA-NEXT:%BB1:
// RA-NEXT:  $Reg2 = CatchInst (:any)
// RA-NEXT:  $Reg2 = StoreFrameInst $Reg1, $Reg2, [%VS0.e]: any
// RA-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg1, %local(): functionCode
// RA-NEXT:  $Reg1 = StorePropertyLooseInst $Reg1, $Reg0, "local": string
// RA-NEXT:  $Reg3 = TryLoadGlobalPropertyInst (:any) $Reg0, "print": string
// RA-NEXT:  $Reg1 = TryLoadGlobalPropertyInst (:any) $Reg0, "local": string
// RA-NEXT:  $Reg2 = HBCLoadConstInst (:undefined) undefined: undefined
// RA-NEXT:  $Reg1 = HBCCallNInst (:any) $Reg1, empty: any, false: boolean, empty: any, undefined: undefined, $Reg2
// RA-NEXT:  $Reg1 = HBCCallNInst (:any) $Reg3, empty: any, false: boolean, empty: any, undefined: undefined, $Reg2, $Reg1
// RA-NEXT:  $Reg1 = TryLoadGlobalPropertyInst (:any) $Reg0, "print": string
// RA-NEXT:  $Reg0 = LoadPropertyInst (:any) $Reg0, "e": string
// RA-NEXT:  $Reg0 = HBCCallNInst (:any) $Reg1, empty: any, false: boolean, empty: any, undefined: undefined, $Reg2, $Reg0
// RA-NEXT:  $Reg0 = ReturnInst $Reg0
// RA-NEXT:%BB2:
// RA-NEXT:  $Reg2 = HBCLoadConstInst (:string) "local": string
// RA-NEXT:  $Reg2 = ThrowInst $Reg2, %BB1
// RA-NEXT:function_end

// RA:scope %VS0 [e: any]

// RA:function local(): any
// RA-NEXT:%BB0:
// RA-NEXT:  $Reg0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// RA-NEXT:  $Reg0 = LoadFrameInst (:any) $Reg0, [%VS0.e]: any
// RA-NEXT:  $Reg0 = ReturnInst $Reg0
// RA-NEXT:function_end
