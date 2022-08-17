/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

// Make sure that multiple declarations are collapsed into one.
var a;
var a;
var a;

function foo () {
    var b, b, a;
    var b;
}

var foo;
foo(); // This is still a valid call.


//CHECK: function global()
//CHECK: frame = [], globals = [a, foo]
//CHECK:   %BB0:
//CHECK:     %0 = CreateFunctionInst %foo()
//CHECK:     %1 = StorePropertyInst %0 : closure, globalObject : object, "foo" : string
//CHECK:     %2 = AllocStackInst $?anon_0_ret
//CHECK:     %3 = StoreStackInst undefined : undefined, %2
//CHECK:     %4 = LoadPropertyInst globalObject : object, "foo" : string
//CHECK:     %5 = CallInst %4, undefined : undefined
//CHECK:     %6 = StoreStackInst %5, %2
//CHECK:     %7 = LoadStackInst %2
//CHECK:     %8 = ReturnInst %7
//CHECK:   function_end

//CHECK: function foo()
//CHECK: frame = [b, a]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst undefined : undefined, [b]
//CHECK:     %1 = StoreFrameInst undefined : undefined, [a]
//CHECK:     %2 = ReturnInst undefined : undefined
//CHECK: function_end

