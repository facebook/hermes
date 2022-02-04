/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:function global() : undefined
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
switch (8) { case 2: case 9: 1?2:3}
