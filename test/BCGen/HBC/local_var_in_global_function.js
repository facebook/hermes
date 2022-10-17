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

// RA:function global#0()#1
// RA-NEXT:frame = [?anon_1_e#1], globals = [inner, e]
// RA-NEXT:%BB0:
// RA-NEXT:  $Reg1 @0 [1...10) 	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// RA-NEXT:  $Reg2 @1 [2...4) 	%1 = HBCLoadConstInst "global" : string
// RA-NEXT:  $Reg0 @2 [3...18) 	%2 = HBCGetGlobalObjectInst
// RA-NEXT:  $Reg2 @3 [empty]	%3 = StorePropertyInst %1 : string, %2 : object, "e" : string
// RA-NEXT:  $Reg2 @4 [empty]	%4 = TryStartInst %BB1, %BB2
// RA-NEXT:%BB1:
// RA-NEXT:  $Reg2 @7 [8...9) 	%5 = CatchInst
// RA-NEXT:  $Reg2 @8 [empty]	%6 = HBCStoreToEnvironmentInst %0, %5, [?anon_1_e#1]
// RA-NEXT:  $Reg1 @9 [10...11) 	%7 = HBCCreateFunctionInst %local#0#1()#2, %0
// RA-NEXT:  $Reg1 @10 [empty]	%8 = StorePropertyInst %7 : closure, %2 : object, "local" : string
// RA-NEXT:  $Reg3 @11 [12...16) 	%9 = TryLoadGlobalPropertyInst %2 : object, "print" : string
// RA-NEXT:  $Reg1 @12 [13...15) 	%10 = TryLoadGlobalPropertyInst %2 : object, "local" : string
// RA-NEXT:  $Reg2 @13 [14...19) 	%11 = HBCLoadConstInst undefined : undefined
// RA-NEXT:  $Reg1 @14 [15...16) 	%12 = HBCCallNInst %10, %11 : undefined
// RA-NEXT:  $Reg1 @15 [empty]	%13 = HBCCallNInst %9, %11 : undefined, %12
// RA-NEXT:  $Reg1 @16 [17...19) 	%14 = TryLoadGlobalPropertyInst %2 : object, "print" : string
// RA-NEXT:  $Reg0 @17 [18...19) 	%15 = LoadPropertyInst %2 : object, "e" : string
// RA-NEXT:  $Reg0 @18 [19...20) 	%16 = HBCCallNInst %14, %11 : undefined, %15
// RA-NEXT:  $Reg0 @19 [empty]	%17 = ReturnInst %16
// RA-NEXT:%BB2:
// RA-NEXT:  $Reg2 @5 [6...7) 	%18 = HBCLoadConstInst "local" : string
// RA-NEXT:  $Reg2 @6 [empty]	%19 = ThrowInst %18 : string
// RA-NEXT:function_end

// RA:function local#0#1()#2
// RA-NEXT:frame = []
// RA-NEXT:%BB0:
// RA-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCResolveEnvironment %S{global#0()#1}, %S{local#0#1()#2}
// RA-NEXT:  $Reg0 @1 [2...3) 	%1 = HBCLoadFromEnvironmentInst %0, [?anon_1_e#1@global]
// RA-NEXT:  $Reg0 @2 [empty]	%2 = ReturnInst %1
// RA-NEXT:function_end
