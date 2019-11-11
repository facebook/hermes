/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s

function f1(a, ...b) {}
//CHECK-LABEL: function f1(a)
//CHECK-NEXT: frame = [a, b]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
//CHECK-NEXT:   %1 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %2 = LoadPropertyInst %1, "copyRestArgs" : string
//CHECK-NEXT:   %3 = CallInst %2, undefined : undefined, 1 : number
//CHECK-NEXT:   %4 = StoreFrameInst %3, [b]
//CHECK-NEXT:   %5 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
