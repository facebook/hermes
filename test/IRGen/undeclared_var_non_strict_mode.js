/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -hermes-parser -dump-ir %s -non-strict 2>&1 | %FileCheckOrRegen %s --match-full-lines

function one() { return s; return s; }

function two() { return s; return t;}

function three() { return z; return z;}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "one" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "two" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "three" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %one()
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3 : closure, globalObject : object, "one" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %two()
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5 : closure, globalObject : object, "two" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %three()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "three" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function one()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "s" : string
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "s" : string
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function two()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "s" : string
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "t" : string
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function three()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "z" : string
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "z" : string
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
