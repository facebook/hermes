/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:function global() : number
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT: {{.*}} %0 = HBCLoadConstInst 42 : number
//CHECK-NEXT: {{.*}} %1 = ReturnInst %0 : number

42;
