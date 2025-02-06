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
// RA-NEXT:                 DeclareGlobalVarInst "inner": string
// RA-NEXT:                 DeclareGlobalVarInst "e": string
// RA-NEXT:  {r0}      %2 = HBCGetGlobalObjectInst (:object)
// RA-NEXT:  {r1}      %3 = HBCLoadConstInst (:string) "global": string
// RA-NEXT:                 StorePropertyLooseInst {r1} %3: string, {r0} %2: object, "e": string
// RA-NEXT:                 TryStartInst %BB1, %BB2
// RA-NEXT:%BB1:
// RA-NEXT:  {r2}      %6 = CatchInst (:any)
// RA-NEXT:  {r1}      %7 = CreateScopeInst (:environment) %VS0: any, empty: any
// RA-NEXT:                 StoreFrameInst {r1} %7: environment, {r2} %6: any, [%VS0.e]: any
// RA-NEXT:  {r1}      %9 = CreateFunctionInst (:object) {r1} %7: environment, %VS0: any, %local(): functionCode
// RA-NEXT:                 StorePropertyLooseInst {r1} %9: object, {r0} %2: object, "local": string
// RA-NEXT:  {r3}     %11 = TryLoadGlobalPropertyInst (:any) {r0} %2: object, "print": string
// RA-NEXT:  {r1}     %12 = TryLoadGlobalPropertyInst (:any) {r0} %2: object, "local": string
// RA-NEXT:  {r2}     %13 = HBCLoadConstInst (:undefined) undefined: undefined
// RA-NEXT:  {r1}     %14 = HBCCallNInst (:any) {r1} %12: any, empty: any, false: boolean, empty: any, undefined: undefined, {r2} %13: undefined
// RA-NEXT:  {r1}     %15 = HBCCallNInst (:any) {r3} %11: any, empty: any, false: boolean, empty: any, undefined: undefined, {r2} %13: undefined, {r1} %14: any
// RA-NEXT:  {r1}     %16 = TryLoadGlobalPropertyInst (:any) {r0} %2: object, "print": string
// RA-NEXT:  {r0}     %17 = LoadPropertyInst (:any) {r0} %2: object, "e": string
// RA-NEXT:  {r0}     %18 = HBCCallNInst (:any) {r1} %16: any, empty: any, false: boolean, empty: any, undefined: undefined, {r2} %13: undefined, {r0} %17: any
// RA-NEXT:                 ReturnInst {r0} %18: any
// RA-NEXT:%BB2:
// RA-NEXT:  {r1}     %20 = HBCLoadConstInst (:string) "local": string
// RA-NEXT:                 ThrowInst {r1} %20: string, %BB1
// RA-NEXT:function_end

// RA:scope %VS0 [e: any]

// RA:function local(): any
// RA-NEXT:%BB0:
// RA-NEXT:  {r0}      %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// RA-NEXT:  {r0}      %1 = LoadFrameInst (:any) {r0} %0: environment, [%VS0.e]: any
// RA-NEXT:                 ReturnInst {r0} %1: any
// RA-NEXT:function_end
