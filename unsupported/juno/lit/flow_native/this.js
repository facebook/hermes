/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fn_dir/run_fnc.sh %fnc %s %t && %t | %FileCheck %s --match-full-lines

"use strict";

function println(x){
  print(x);
  print("\n");
}

function myFun(){
  println(this);
  if(this !== undefined){
    println(this.print);
    println(this.foo);
  }
  function bar(){
    println(this);
  }
  bar();
}
myFun();
// CHECK: undefined
// CHECK-NEXT: undefined

var x = {};
x.foo = myFun;
x.foo();
// CHECK-NEXT: [Object]
// CHECK-NEXT: undefined
// CHECK-NEXT: [Closure]
// CHECK-NEXT: undefined
