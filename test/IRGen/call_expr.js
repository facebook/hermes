/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK: function bar(x, y, z)
//CHECK: frame = [x, y, z]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = StoreFrameInst %y, [y]
//CHECK:     %2 = StoreFrameInst %z, [z]
//CHECK:     %3 = LoadPropertyInst globalObject : object, "foo" : string
//CHECK:     %4 = LoadFrameInst [x]
//CHECK:     %5 = LoadFrameInst [y]
//CHECK:     %6 = LoadFrameInst [z]
//CHECK:     %7 = CallInst %3, undefined : undefined, %4, %5, %6
//CHECK:     %8 = ReturnInst undefined : undefined
function foo(a, b, c) {

}

function bar(x, y, z) {
  foo(x, y, z)
}

// Test that we are passing the 'obj' as the 'this' for the method.
//CHECK: function test_member_access(obj, param)
//CHECK: frame = [obj, param]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %obj, [obj]
//CHECK:     %1 = StoreFrameInst %param, [param]
//CHECK:     %2 = LoadFrameInst [obj]
//CHECK:     %3 = LoadPropertyInst %2, "foo" : string
//CHECK:     %4 = LoadFrameInst [param]
//CHECK:     %5 = CallInst %3, %2, %4
//CHECK:     %6 = ReturnInst undefined : undefined
function test_member_access(obj, param) {
  obj.foo(param)
}

