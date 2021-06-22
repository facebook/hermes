/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheck --match-full-lines %s

({10: 1, "11": 2, "10": 3})

//CHECK:    %2 = AllocObjectLiteralInst "11" : string, 2 : number, "10" : string, 3 : number
//CHECK-NEXT:    %3 = StoreStackInst %2 : object, %0
