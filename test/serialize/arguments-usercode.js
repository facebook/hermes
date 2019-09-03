// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s || %FileCheck --match-full-lines %s
// REQUIRES: serializer

"use strict";

var empty = "";

function simpleArgs() {
    for(var i = 0; i < arguments.length; ++i)
        print("arg [" + i + "]=" + arguments[i]);
    print("invalid arg [" + i + "]=" + arguments[i]);
    i = "foo" + i;
    print("invalid arg [" + i + "]=" + arguments[i]);
    i = empty + "length";
    print("indirect length [" + i + "]=" + arguments[i]);
    i = empty + "foo";
    print("parent prop foo [" + i + "]=" + arguments[i]);
    // This will cause reification since it is an accessor.
    i = empty + "__proto__";
    print("indirect __proto__ [" + i + "]=" + arguments[i]);
    // Read the same parent property again.
    i = empty + "foo";
    print("parent prop foo [" + i + "]=" + arguments[i]);
}

Object.prototype.foo = "foo-val";

function modifyArgs(i) {
    ++arguments[i];
    print(arguments[i]);
    print(arguments[i+1]);
    ++arguments.length;
    print(arguments.length);
    arguments.foo = "bar";
    print(arguments.foo);
}

serializeVM(function() {
  simpleArgs(10,20,30);
  //CHECK:arg [0]=10
  //CHECK-NEXT:arg [1]=20
  //CHECK-NEXT:arg [2]=30
  //CHECK-NEXT:invalid arg [3]=undefined
  //CHECK-NEXT:invalid arg [foo3]=undefined
  //CHECK-NEXT:indirect length [length]=3
  //CHECK-NEXT:parent prop foo [foo]=foo-val
  //CHECK-NEXT:indirect __proto__ [__proto__]=[object Object]
  //CHECK-NEXT:parent prop foo [foo]=foo-val

  modifyArgs(1, 20, 30);
  //CHECK-NEXT:21
  //CHECK-NEXT:30
  //CHECK-NEXT:4
  //CHECK-NEXT:bar
})
