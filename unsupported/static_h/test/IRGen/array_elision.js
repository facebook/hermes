/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


//CHECK-LABEL:function func()
//CHECK-NEXT:frame = [foo]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [foo]
//CHECK-NEXT:  %1 = AllocArrayInst 3 : number
//CHECK-NEXT:  %2 = StoreOwnPropertyInst "a" : string, %1 : object, 2 : number, true : boolean
//CHECK-NEXT:  %3 = StoreFrameInst %1 : object, [foo]
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function func() {
  var foo = [,,"a"];
}


