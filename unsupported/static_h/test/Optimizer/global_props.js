/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -strict -dump-ir -O0 -include-globals=%s.d %s | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -strict -dump-ir -O -include-globals=%s.d %s | %FileCheck %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermes -hermes-parser -non-strict -dump-ir -O -include-globals=%s.d %s | %FileCheck %s --match-full-lines --check-prefix=OPT-NONSTRICT

// Ensure that global properties are not promoted.
var a = 10;
print(a, process);
process = null;


//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [a]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:  %2 = StorePropertyInst 10 : number, globalObject : object, "a" : string
//CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %4 = LoadPropertyInst globalObject : object, "a" : string
//CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "process" : string
//CHECK-NEXT:  %6 = CallInst %3, undefined : undefined, %4, %5
//CHECK-NEXT:  %7 = StoreStackInst %6, %0
//CHECK-NEXT:  %8 = TryStoreGlobalPropertyInst null : null, globalObject : object, "process" : string
//CHECK-NEXT:  %9 = StoreStackInst null : null, %0
//CHECK-NEXT:  %10 = LoadStackInst %0
//CHECK-NEXT:  %11 = ReturnInst %10
//CHECK-NEXT:function_end

//OPT-CHECK-LABEL:function global() : null
//OPT-CHECK-NEXT:frame = [], globals = [a]
//OPT-CHECK-NEXT:%BB0:
//OPT-CHECK-NEXT:  %0 = StorePropertyInst 10 : number, globalObject : object, "a" : string
//OPT-CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//OPT-CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "a" : string
//OPT-CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "process" : string
//OPT-CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, %2, %3
//OPT-CHECK-NEXT:  %5 = TryStoreGlobalPropertyInst null : null, globalObject : object, "process" : string
//OPT-CHECK-NEXT:  %6 = ReturnInst null : null
//OPT-CHECK-NEXT:function_end

//OPT-NONSTRICT-LABEL:function global() : null
//OPT-NONSTRICT-NEXT:frame = [], globals = [a]
//OPT-NONSTRICT-NEXT:%BB0:
//OPT-NONSTRICT-NEXT:  %0 = StorePropertyInst 10 : number, globalObject : object, "a" : string
//OPT-NONSTRICT-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//OPT-NONSTRICT-NEXT:  %2 = LoadPropertyInst globalObject : object, "a" : string
//OPT-NONSTRICT-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "process" : string
//OPT-NONSTRICT-NEXT:  %4 = CallInst %1, undefined : undefined, %2, %3
//OPT-NONSTRICT-NEXT:  %5 = StorePropertyInst null : null, globalObject : object, "process" : string
//OPT-NONSTRICT-NEXT:  %6 = ReturnInst null : null
//OPT-NONSTRICT-NEXT:function_end
