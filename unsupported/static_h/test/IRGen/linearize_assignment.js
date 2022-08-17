/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O0 | %FileCheck %s --match-full-lines

a.x = a = 42;

//CHECK-LABEL: %BB0:
//CHECK-NEXT:   %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:   %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:   %2 = TryLoadGlobalPropertyInst globalObject : object, "a" : string
//CHECK-NEXT:   %3 = StorePropertyInst 42 : number, globalObject : object, "a" : string
//CHECK-NEXT:   %4 = StorePropertyInst 42 : number, %2, "x" : string
//CHECK-NEXT:   %5 = StoreStackInst 42 : number, %0
//CHECK-NEXT:   %6 = LoadStackInst %0
//CHECK-NEXT:   %7 = ReturnInst %6
//CHECK-NEXT: function_end
