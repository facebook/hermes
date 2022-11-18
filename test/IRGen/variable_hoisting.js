/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

main()

function main() {

  function foo(x) { return capture_me; }

  var capture_me;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [main]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %main()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadPropertyInst globalObject : object, "main" : string
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined
// CHECK-NEXT:  %6 = StoreStackInst %5, %2
// CHECK-NEXT:  %7 = LoadStackInst %2
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function main()
// CHECK-NEXT:frame = [capture_me, foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [capture_me]
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [foo]
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = LoadFrameInst [capture_me@main]
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
