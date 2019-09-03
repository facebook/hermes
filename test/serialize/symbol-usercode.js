// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict";

var sym = Symbol(44912);
var sym2 = Symbol();
var sym3 = Symbol(undefined);
var sym4 = Symbol('asdf');
var sym5 = Symbol({toString: function() {return 'asdf'}})

var obj = {};
var a = Symbol('x');
var b = Symbol('y');
obj[a] = 1;
obj[b] = 2;
var syms = Object.getOwnPropertySymbols(obj);

var x = Symbol.for('x');
var x2 = Symbol.for('x');
var y = Symbol.for('y');

serializeVM(function() {
  print('Symbol()');
  // CHECK-LABEL: Symbol()
  print(typeof Symbol());
  // CHECK-NEXT: symbol

  print(String(sym));
  // CHECK-NEXT: Symbol(44912)
  print(sym2.toString());
  // CHECK-NEXT: Symbol()
  print(sym3.toString());
  // CHECK-NEXT: Symbol()
  print(sym4.toString());
  // CHECK-NEXT: Symbol(asdf)
  print(sym5.toString());
  // CHECK-NEXT: Symbol(asdf)

  print(syms.length, String(syms[0]), String(syms[1]));
  // CHECK-NEXT: 2 Symbol(x) Symbol(y)
  print(obj[syms[0]], obj[syms[1]]);
  // CHECK-NEXT: 1 2

  print(x === x2);
  // CHECK-NEXT: true
  print(x === y);
  // CHECK-NEXT: false
  print(String(x), String(x2), String(y));
  // CHECK-NEXT: Symbol(x) Symbol(x) Symbol(y)
  print(Symbol.keyFor(x));
  // CHECK-NEXT: x
  print(Symbol.keyFor(x2));
  // CHECK-NEXT: x
  print(Symbol.keyFor(y));
  // CHECK-NEXT: y
})
