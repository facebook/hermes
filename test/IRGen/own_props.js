/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheck --match-full-lines %s

({10: 1, "11": 2, "10": 3})

//CHECK:       %2 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 2 : number, %2 : object, "11" : string, true : boolean
//CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 3 : number, %2 : object, "10" : string, true : boolean
//CHECK-NEXT:  %5 = StoreStackInst %2 : object, %0
//CHECK-NEXT:  %6 = LoadStackInst %0
//CHECK-NEXT:  %7 = ReturnInst %6
