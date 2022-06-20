/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fn_dir/run_fnc.sh %fnc %s %t && %t | %FileCheck %s --match-full-lines

function println(x){
  print(x);
  print("\n");
}

function make1(){
    return this;
}

var obj = new make1();
println(obj.foo);
// CHECK: undefined

var proto = {foo: "banana"}
make1.prototype = proto;
obj = new make1();
println(obj.foo);
// CHECK-NEXT: banana

obj.foo = "apple";
println(obj.foo);
// CHECK-NEXT: apple

println(proto.foo);
// CHECK-NEXT: banana
