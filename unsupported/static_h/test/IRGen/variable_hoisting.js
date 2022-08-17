/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


main()

function main() {

  function foo(x) { return capture_me; }

  var capture_me;
}


//CHECK: function global()
//CHECK: frame = [], globals = [main]
//CHECK:   %BB0:
//CHECK:     %0 = CreateFunctionInst %main()
//CHECK:     %1 = StorePropertyInst %0 : closure, globalObject : object, "main" : string
//CHECK:     %2 = AllocStackInst $?anon_0_ret
//CHECK:     %3 = StoreStackInst undefined : undefined, %2
//CHECK:     %4 = LoadPropertyInst globalObject : object, "main" : string
//CHECK:     %5 = CallInst %4, undefined : undefined
//CHECK:     %6 = StoreStackInst %5, %2
//CHECK:     %7 = LoadStackInst %2
//CHECK:     %8 = ReturnInst %7
//CHECK: function_end

//CHECK: function main()
//CHECK: frame = [capture_me, foo]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst undefined : undefined, [capture_me]
//CHECK:     %1 = CreateFunctionInst %foo()
//CHECK:     %2 = StoreFrameInst %1 : closure, [foo]
//CHECK:     %3 = ReturnInst undefined : undefined
//CHECK: function_end

//CHECK: function foo(x)
//CHECK: frame = [x]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = LoadFrameInst [capture_me@main]
//CHECK:     %2 = ReturnInst %1
//CHECK:   %BB1:
//CHECK:     %3 = ReturnInst undefined : undefined
//CHECK: function_end
