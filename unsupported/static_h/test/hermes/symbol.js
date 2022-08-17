/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
"use strict";

print('Symbol()');
// CHECK-LABEL: Symbol()
print(typeof Symbol());
// CHECK-NEXT: symbol

// Ensure that dynamically allocated descriptions are marked during GC.
var sym = Symbol(44912);
print(String(sym));
// CHECK-NEXT: Symbol(44912)
gc();
print(String(sym));
// CHECK-NEXT: Symbol(44912)

print('description');
// CHECK-LABEL: description
print(Symbol().description);
// CHECK-EMPTY:
print(Symbol('asdf').description);
// CHECK-NEXT: asdf
print(Symbol({toString: function() {return 'asdf'}}).description)
// CHECK-NEXT: asdf
print(Object(Symbol('asdf')).description)
// CHECK-NEXT: asdf
try { print(Symbol.prototype.description.call(3)); }
catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught TypeError
print(Symbol.iterator.description)  // well-known Symbols
// CHECK-NEXT: Symbol.iterator

// TODO(The below cases need to return `undefined` to be fully conformant.)
// print(Symbol().description)
// print(Symbol(undefined).description)

print('toString');
// CHECK-LABEL: toString
print(Symbol().toString());
// CHECK-NEXT: Symbol()
print(Symbol(undefined).toString());
// CHECK-NEXT: Symbol()
print(Symbol('asdf').toString());
// CHECK-NEXT: Symbol(asdf)
print(Symbol({toString: function() {return 'asdf'}}).toString());
// CHECK-NEXT: Symbol(asdf)
print(Object(Symbol('asdf')).toString());
// CHECK-NEXT: Symbol(asdf)
try { print(Symbol.prototype.toString.call(3)); }
catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught TypeError

print('valueOf');
// CHECK-LABEL: valueOf
var sym = Symbol();
print(sym === sym.valueOf());
// CHECK-NEXT: true
print(sym === Object(sym).valueOf());
// CHECK-NEXT: true

print('equality');
// CHECK-LABEL: equality
print(Symbol() === Symbol());
// CHECK-NEXT: false
var x = Symbol();
print(x === x);
// CHECK-NEXT: true

print('object properties');
// CHECK-LABEL: object properties
var obj = {};
var x = Symbol('asdf');
var y = Symbol('asdf');
obj[x] = 1;
obj[y] = 2;
obj['asdf'] = 3;
print(obj[x], obj[y], obj['asdf']);
// CHECK-NEXT: 1 2 3
gc();
print(obj[x], obj[y], obj['asdf']);
// CHECK-NEXT: 1 2 3
delete obj[x];
print(obj[x]);
// CHECK-NEXT: undefined
var sym = Symbol();
var obj = {};
var wrapped = {
  toString: function() {
    return sym;
  }
}
obj[sym] = 3;
print(obj[sym], obj[wrapped]);
// CHECK-NEXT: 3 3
function foo() {
  print(arguments[Symbol()]);
}
foo();
// CHECK-NEXT: undefined

print('getOwnPropertySymbols');
// CHECK-LABEL: getOwnPropertySymbols
var obj = {};
var x = Symbol('x');
var y = Symbol('y');
obj[x] = 1;
obj[y] = 2;
var syms = Object.getOwnPropertySymbols(obj);
print(syms.length, String(syms[0]), String(syms[1]));
// CHECK-NEXT: 2 Symbol(x) Symbol(y)
print(obj[syms[0]], obj[syms[1]]);
// CHECK-NEXT: 1 2

print('registry');
// CHECK-LABEL: registry
var x = Symbol.for('x');
var x2 = Symbol.for('x');
var y = Symbol.for('y');
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
print(Symbol.keyFor(Symbol('asdf')));
// CHECK-NEXT: undefined
print(Symbol.keyFor(Symbol()));
// CHECK-NEXT: undefined
try { print(Symbol.keyFor('a')); } catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught TypeError

print('well-known Symbols');
// CHECK-LABEL: well-known Symbols
print(String(Symbol.iterator));
// CHECK-NEXT: Symbol(Symbol.iterator)
print(String(Symbol.toStringTag));
// CHECK-NEXT: Symbol(Symbol.toStringTag)
print(Array.prototype[Symbol.iterator] === Array.prototype.values);
// CHECK-NEXT: true
print(Uint32Array.prototype[Symbol.iterator] === Uint32Array.prototype.values);
// CHECK-NEXT: true
print(Set.prototype[Symbol.iterator] === Set.prototype.values);
// CHECK-NEXT: true
print(Map.prototype[Symbol.iterator] === Map.prototype.entries);
// CHECK-NEXT: true
print(String(Symbol.hasInstance));
// CHECK-NEXT: Symbol(Symbol.hasInstance)
print(String(Symbol.match));
// CHECK-NEXT: Symbol(Symbol.match)
print(String(Symbol.matchAll));
// CHECK-NEXT: Symbol(Symbol.matchAll)
print(String(Symbol.search));
// CHECK-NEXT: Symbol(Symbol.search)
print(String(Symbol.replace));
// CHECK-NEXT: Symbol(Symbol.replace)
print(String(Symbol.split));
// CHECK-NEXT: Symbol(Symbol.split)
