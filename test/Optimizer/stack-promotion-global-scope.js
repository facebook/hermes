/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheck --match-full-lines %s

// Ensure the stack promotion happens in global scope.
let x = 10;
print(x);

//CHECK-LABEL:function global{{.*}}
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 10 : number
//CHECK-NEXT:  %2 = ReturnInst %1
//CHECK-NEXT:function_end
