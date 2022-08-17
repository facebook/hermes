/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir -strict -Wno-undefined-variable %s 2>&1 | %FileCheck %s --match-full-lines

var x = y;
//CHECK: function global()
//CHECK-NEXT: frame = [], globals = [x]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:   %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:   %2 = TryLoadGlobalPropertyInst globalObject : object, "y" : string
//CHECK-NEXT:   %3 = StorePropertyInst %2, globalObject : object, "x" : string
//CHECK-NEXT:   %4 = LoadStackInst %0
//CHECK-NEXT:   %5 = ReturnInst %4
//CHECK-NEXT: function_end
