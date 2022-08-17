/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck --match-full-lines %s
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [x]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:    %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:    %2 = LoadPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:    %3 = StoreStackInst %2, %0
//CHECK-NEXT:    %4 = LoadStackInst %0
//CHECK-NEXT:    %5 = ReturnInst %4
//CHECK-NEXT:function_end

var x;
x;
