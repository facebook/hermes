/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


function same_func_name(same_param_name) {
  function same_func_name(same_param_name) {
     function same_func_name(same_param_name) {
      return same_param_name;
    }
  }
}

//CHECK: function sink(a, b, c)
//CHECK: frame = [a, b, c]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %a, [a]
//CHECK:     %1 = StoreFrameInst %b, [b]
//CHECK:     %2 = StoreFrameInst %c, [c]
//CHECK:     %3 = ReturnInst undefined : undefined

function sink(a,b,c) {}

//CHECK: function level0(x)
//CHECK: frame = [level1, x]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = CreateFunctionInst %level1()
//CHECK:     %2 = StoreFrameInst %1 : closure, [level1]
//CHECK:     %3 = ReturnInst undefined : undefined

//CHECK: function level1(y)
//CHECK: frame = [level2, y]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %y, [y]
//CHECK:     %1 = CreateFunctionInst %level2()
//CHECK:     %2 = StoreFrameInst %1 : closure, [level2]
//CHECK:     %3 = ReturnInst undefined : undefined

//CHECK: function level2(z)
//CHECK: frame = [z]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %z, [z]
//CHECK:     %1 = LoadPropertyInst globalObject : object, "sink" : string
//CHECK:     %2 = LoadFrameInst [x@level0]
//CHECK:     %3 = LoadFrameInst [y@level1]
//CHECK:     %4 = LoadFrameInst [z]
//CHECK:     %5 = CallInst %1, undefined : undefined, %2, %3, %4
//CHECK:     %6 = ReturnInst undefined : undefined


function level0(x) {
  function level1(y) {
    function level2(z) {
      sink(x,y,z)
    }
  }
}




