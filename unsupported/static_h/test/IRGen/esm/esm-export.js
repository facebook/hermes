/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -commonjs -dump-ir %s | %FileCheck --match-full-lines %s

// CHECK-LABEL: function cjs_module(exports, require, module)
// CHECK-NEXT: frame = [x, y, z, longVariableName, a, myFun, exports, require, module]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:   %1 = StoreFrameInst undefined : undefined, [y]
// CHECK-NEXT:   %2 = StoreFrameInst undefined : undefined, [z]
// CHECK-NEXT:   %3 = StoreFrameInst undefined : undefined, [longVariableName]
// CHECK-NEXT:   %4 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:   %5 = StoreFrameInst %exports, [exports]
// CHECK-NEXT:   %6 = StoreFrameInst %require, [require]
// CHECK-NEXT:   %7 = StoreFrameInst %module, [module]
// CHECK-NEXT:   %8 = CreateFunctionInst %myFun()
// CHECK-NEXT:   %9 = StoreFrameInst %8 : closure, [myFun]

export default function myFun() {
  return 1038;
}
// CHECK-NEXT:   %10 = LoadFrameInst [myFun]
// CHECK-NEXT:   %11 = StorePropertyInst %10, %exports, "myFun" : string

export var x = 1;
// CHECK-NEXT:   %12 = StoreFrameInst 1 : number, [x]
// CHECK-NEXT:   %13 = LoadFrameInst [x]
// CHECK-NEXT:   %14 = StorePropertyInst %13, %exports, "x" : string

export var y = 2, z = 4;
// CHECK-NEXT:   %15 = StoreFrameInst 2 : number, [y]
// CHECK-NEXT:   %16 = LoadFrameInst [y]
// CHECK-NEXT:   %17 = StorePropertyInst %16, %exports, "y" : string
// CHECK-NEXT:   %18 = StoreFrameInst 4 : number, [z]
// CHECK-NEXT:   %19 = LoadFrameInst [z]
// CHECK-NEXT:   %20 = StorePropertyInst %19, %exports, "z" : string

var longVariableName = 3;
var a = 4;
// CHECK-NEXT:   %21 = StoreFrameInst 3 : number, [longVariableName]
// CHECK-NEXT:   %22 = StoreFrameInst 4 : number, [a]

export { a, longVariableName as b }
// CHECK-NEXT:   %23 = LoadFrameInst [a]
// CHECK-NEXT:   %24 = StorePropertyInst %23, %exports, "a" : string
// CHECK-NEXT:   %25 = LoadFrameInst [longVariableName]
// CHECK-NEXT:   %26 = StorePropertyInst %25, %exports, "b" : string

export * from 'foo.js';
// CHECK-NEXT:   %27 = CallInst %require, undefined : undefined, "foo.js" : string
// CHECK-NEXT:   %28 = CallBuiltinInst [HermesBuiltin.exportAll] : number, undefined : undefined, %exports, %27
// CHECK-NEXT:   %29 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

// CHECK-LABEL: function myFun()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = ReturnInst 1038 : number
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %1 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

