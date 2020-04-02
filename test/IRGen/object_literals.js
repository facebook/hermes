/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

({prop1: 10});
({"prop1": 10});

//CHECK: function global()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = AllocStackInst $?anon_0_ret
//CHECK:     %1 = StoreStackInst undefined : undefined, %0
//CHECK:     %2 = AllocObjectInst 1 : number, empty
//CHECK:     %3 = StoreNewOwnPropertyInst 10 : number, %2 : object, "prop1" : string, true : boolean
//CHECK:     %4 = StoreStackInst %2 : object, %0
//CHECK:     %5 = AllocObjectInst 1 : number, empty
//CHECK:     %6 = StoreNewOwnPropertyInst 10 : number, %5 : object, "prop1" : string, true : boolean
//CHECK:     %7 = StoreStackInst %5 : object, %0
//CHECK:     %8 = LoadStackInst %0
//CHECK:     %9 = ReturnInst %8
//CHECK: function_end

