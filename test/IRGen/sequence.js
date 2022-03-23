/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function sink0(a) { }
function sink1(a) { }


//CHECK: function test1(x, y)
//CHECK: frame = [x, y]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = StoreFrameInst %y, [y]
//CHECK:     %2 = ReturnInst 3 : number
//CHECK:   %BB1:
//CHECK:     %3 = ReturnInst undefined : undefined
//CHECK: function_end
function test1(x,y) {
  return (1,2,3);
}

//CHECK: function test2(x, y)
//CHECK: frame = [x, y]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = StoreFrameInst %y, [y]
//CHECK:     %2 = LoadPropertyInst globalObject : object, "sink0" : string
//CHECK:     %3 = LoadFrameInst [x]
//CHECK:     %4 = LoadFrameInst [y]
//CHECK:     %5 = CallInst %2, undefined : undefined, %3, %4
//CHECK:     %6 = LoadPropertyInst globalObject : object, "sink1" : string
//CHECK:     %7 = LoadFrameInst [x]
//CHECK:     %8 = LoadFrameInst [y]
//CHECK:     %9 = CallInst %6, undefined : undefined, %7, %8
//CHECK:     %10 = ReturnInst %9
//CHECK:   %BB1:
//CHECK:     %11 = ReturnInst undefined : undefined
//CHECK: function_end
function test2(x,y) {
  return (sink0(x,y), sink1(x,y));
}

