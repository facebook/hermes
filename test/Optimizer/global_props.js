/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -strict -dump-ir -O0 -include-globals=%s.d %s | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -strict -dump-ir -O -include-globals=%s.d %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermes -hermes-parser -non-strict -dump-ir -O -include-globals=%s.d %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-NONSTRICT

// Ensure that global properties are not promoted.
var a = 10;
print(a, process);
process = null;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = StorePropertyInst 10 : number, globalObject : object, "a" : string
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %5 = LoadPropertyInst globalObject : object, "a" : string
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "process" : string
// CHECK-NEXT:  %7 = CallInst %4, undefined : undefined, %5, %6
// CHECK-NEXT:  %8 = StoreStackInst %7, %1
// CHECK-NEXT:  %9 = TryStoreGlobalPropertyInst null : null, globalObject : object, "process" : string
// CHECK-NEXT:  %10 = StoreStackInst null : null, %1
// CHECK-NEXT:  %11 = LoadStackInst %1
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// OPT-CHECK:function global#0()#1 : null
// OPT-CHECK-NEXT:frame = [], globals = [a]
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// OPT-CHECK-NEXT:  %1 = StorePropertyInst 10 : number, globalObject : object, "a" : string
// OPT-CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// OPT-CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "a" : string
// OPT-CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "process" : string
// OPT-CHECK-NEXT:  %5 = CallInst %2, undefined : undefined, %3, %4
// OPT-CHECK-NEXT:  %6 = TryStoreGlobalPropertyInst null : null, globalObject : object, "process" : string
// OPT-CHECK-NEXT:  %7 = ReturnInst null : null
// OPT-CHECK-NEXT:function_end

// OPT-NONSTRICT:function global#0()#1 : null
// OPT-NONSTRICT-NEXT:frame = [], globals = [a]
// OPT-NONSTRICT-NEXT:%BB0:
// OPT-NONSTRICT-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// OPT-NONSTRICT-NEXT:  %1 = StorePropertyInst 10 : number, globalObject : object, "a" : string
// OPT-NONSTRICT-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// OPT-NONSTRICT-NEXT:  %3 = LoadPropertyInst globalObject : object, "a" : string
// OPT-NONSTRICT-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "process" : string
// OPT-NONSTRICT-NEXT:  %5 = CallInst %2, undefined : undefined, %3, %4
// OPT-NONSTRICT-NEXT:  %6 = StorePropertyInst null : null, globalObject : object, "process" : string
// OPT-NONSTRICT-NEXT:  %7 = ReturnInst null : null
// OPT-NONSTRICT-NEXT:function_end
