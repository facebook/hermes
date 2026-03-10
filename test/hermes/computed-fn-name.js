/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec %s | %FileCheck %s --match-full-lines

// Tests for SetFunctionName with computed property names.

print("computed function names");
// CHECK-LABEL: computed function names

// Opaque identity function to prevent the optimizer from constant-folding
// computed string keys back into static property names.
function k(s) { return s; }

var sym = Symbol("desc");
var symEmpty = Symbol();

var obj = {
  [k("f")]: function() {},
  [sym]: () => {},
  get [k("g")]() {},
  set [k("s")](v) {},
};
print(obj["f"].name);
// CHECK-NEXT: f
print(obj[sym].name);
// CHECK-NEXT: [desc]
print(JSON.stringify(Object.getOwnPropertyDescriptor(obj, "g").get.name));
// CHECK-NEXT: "get g"
print(JSON.stringify(Object.getOwnPropertyDescriptor(obj, "s").set.name));
// CHECK-NEXT: "set s"

// Symbol() and Symbol("") edge cases (tested once here).
var symEmptyDesc = Symbol("");
var objSym = {
  [symEmpty]: function() {},
  [symEmptyDesc]: function() {},
};
print(JSON.stringify(objSym[symEmpty].name));
// CHECK-NEXT: ""
print(objSym[symEmptyDesc].name);
// CHECK-NEXT: []

class C1 {
  [k("m")]() {}
  [sym]() {}
  static [k("sm")]() {}
  get [k("g")]() {}
  set [k("s")](v) {}
}
print(C1.prototype["m"].name);
// CHECK-NEXT: m
print(C1.prototype[sym].name);
// CHECK-NEXT: [desc]
print(C1["sm"].name);
// CHECK-NEXT: sm
print(Object.getOwnPropertyDescriptor(C1.prototype, "g").get.name);
// CHECK-NEXT: get g
print(Object.getOwnPropertyDescriptor(C1.prototype, "s").set.name);
// CHECK-NEXT: set s

// Instance fields with computed key.
class C2 {
  [k("strFunc")] = function() {};
  [k("strArrow")] = () => {};
  [k("strClass")] = class {};
  [k("strNamedFunc")] = function myFunc() {};
  [sym] = function() {};
}
var c2 = new C2();

print(c2["strFunc"].name);
// CHECK-NEXT: strFunc
print(c2["strArrow"].name);
// CHECK-NEXT: strArrow
print(c2["strClass"].name);
// CHECK-NEXT: strClass
print(c2["strNamedFunc"].name);
// CHECK-NEXT: myFunc
print(c2[sym].name);
// CHECK-NEXT: [desc]

// Static fields with computed key.
class C3 {
  static [k("strStaticFunc")] = function() {};
  static [sym] = () => {};
  static [k("strStaticNamedFunc")] = function myFunc() {};
}

print(C3["strStaticFunc"].name);
// CHECK-NEXT: strStaticFunc
print(C3[sym].name);
// CHECK-NEXT: [desc]
print(C3["strStaticNamedFunc"].name);
// CHECK-NEXT: myFunc
