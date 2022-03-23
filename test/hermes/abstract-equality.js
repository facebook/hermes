/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

"use strict";

var str1 = "";
var str2 = "a string";
var str3 = "another string";
var obj1 = { field0: "1", toString: () => "obj1" };
var obj2 = { field0: "1", toString: () => "obj2" };
var sym1 =  Symbol("Symbol1");
var sym2 =  Symbol("Symbol2");

const values = [
    undefined,
    null,
    str1, str2, str3,
    obj1, obj2,
    false, true,
    sym1, sym2,
    0.0, 1.0, -NaN, -Infinity,
];

function printValuesAndEqualityValue(v0, v1) {
    var s0 = "'" + String(v0) + "'";
    var s1 = "'" + String(v1) + "'";

    print(s0, s1, v0 == v1);
}

print("Abstract Equality Test");
for (var v0 of values) {
    for (var v1 of values) {
        printValuesAndEqualityValue(v0, v1);
    }
}

// CHECK-LABEL: Abstract Equality Test
// CHECK-NEXT: 'undefined' 'undefined' true
// CHECK-NEXT: 'undefined' 'null' true
// CHECK-NEXT: 'undefined' '' false
// CHECK-NEXT: 'undefined' 'a string' false
// CHECK-NEXT: 'undefined' 'another string' false
// CHECK-NEXT: 'undefined' 'obj1' false
// CHECK-NEXT: 'undefined' 'obj2' false
// CHECK-NEXT: 'undefined' 'false' false
// CHECK-NEXT: 'undefined' 'true' false
// CHECK-NEXT: 'undefined' 'Symbol(Symbol1)' false
// CHECK-NEXT: 'undefined' 'Symbol(Symbol2)' false
// CHECK-NEXT: 'undefined' '0' false
// CHECK-NEXT: 'undefined' '1' false
// CHECK-NEXT: 'undefined' 'NaN' false
// CHECK-NEXT: 'undefined' '-Infinity' false
// CHECK-NEXT: 'null' 'undefined' true
// CHECK-NEXT: 'null' 'null' true
// CHECK-NEXT: 'null' '' false
// CHECK-NEXT: 'null' 'a string' false
// CHECK-NEXT: 'null' 'another string' false
// CHECK-NEXT: 'null' 'obj1' false
// CHECK-NEXT: 'null' 'obj2' false
// CHECK-NEXT: 'null' 'false' false
// CHECK-NEXT: 'null' 'true' false
// CHECK-NEXT: 'null' 'Symbol(Symbol1)' false
// CHECK-NEXT: 'null' 'Symbol(Symbol2)' false
// CHECK-NEXT: 'null' '0' false
// CHECK-NEXT: 'null' '1' false
// CHECK-NEXT: 'null' 'NaN' false
// CHECK-NEXT: 'null' '-Infinity' false
// CHECK-NEXT: '' 'undefined' false
// CHECK-NEXT: '' 'null' false
// CHECK-NEXT: '' '' true
// CHECK-NEXT: '' 'a string' false
// CHECK-NEXT: '' 'another string' false
// CHECK-NEXT: '' 'obj1' false
// CHECK-NEXT: '' 'obj2' false
// CHECK-NEXT: '' 'false' true
// CHECK-NEXT: '' 'true' false
// CHECK-NEXT: '' 'Symbol(Symbol1)' false
// CHECK-NEXT: '' 'Symbol(Symbol2)' false
// CHECK-NEXT: '' '0' true
// CHECK-NEXT: '' '1' false
// CHECK-NEXT: '' 'NaN' false
// CHECK-NEXT: '' '-Infinity' false
// CHECK-NEXT: 'a string' 'undefined' false
// CHECK-NEXT: 'a string' 'null' false
// CHECK-NEXT: 'a string' '' false
// CHECK-NEXT: 'a string' 'a string' true
// CHECK-NEXT: 'a string' 'another string' false
// CHECK-NEXT: 'a string' 'obj1' false
// CHECK-NEXT: 'a string' 'obj2' false
// CHECK-NEXT: 'a string' 'false' false
// CHECK-NEXT: 'a string' 'true' false
// CHECK-NEXT: 'a string' 'Symbol(Symbol1)' false
// CHECK-NEXT: 'a string' 'Symbol(Symbol2)' false
// CHECK-NEXT: 'a string' '0' false
// CHECK-NEXT: 'a string' '1' false
// CHECK-NEXT: 'a string' 'NaN' false
// CHECK-NEXT: 'a string' '-Infinity' false
// CHECK-NEXT: 'another string' 'undefined' false
// CHECK-NEXT: 'another string' 'null' false
// CHECK-NEXT: 'another string' '' false
// CHECK-NEXT: 'another string' 'a string' false
// CHECK-NEXT: 'another string' 'another string' true
// CHECK-NEXT: 'another string' 'obj1' false
// CHECK-NEXT: 'another string' 'obj2' false
// CHECK-NEXT: 'another string' 'false' false
// CHECK-NEXT: 'another string' 'true' false
// CHECK-NEXT: 'another string' 'Symbol(Symbol1)' false
// CHECK-NEXT: 'another string' 'Symbol(Symbol2)' false
// CHECK-NEXT: 'another string' '0' false
// CHECK-NEXT: 'another string' '1' false
// CHECK-NEXT: 'another string' 'NaN' false
// CHECK-NEXT: 'another string' '-Infinity' false
// CHECK-NEXT: 'obj1' 'undefined' false
// CHECK-NEXT: 'obj1' 'null' false
// CHECK-NEXT: 'obj1' '' false
// CHECK-NEXT: 'obj1' 'a string' false
// CHECK-NEXT: 'obj1' 'another string' false
// CHECK-NEXT: 'obj1' 'obj1' true
// CHECK-NEXT: 'obj1' 'obj2' false
// CHECK-NEXT: 'obj1' 'false' false
// CHECK-NEXT: 'obj1' 'true' false
// CHECK-NEXT: 'obj1' 'Symbol(Symbol1)' false
// CHECK-NEXT: 'obj1' 'Symbol(Symbol2)' false
// CHECK-NEXT: 'obj1' '0' false
// CHECK-NEXT: 'obj1' '1' false
// CHECK-NEXT: 'obj1' 'NaN' false
// CHECK-NEXT: 'obj1' '-Infinity' false
// CHECK-NEXT: 'obj2' 'undefined' false
// CHECK-NEXT: 'obj2' 'null' false
// CHECK-NEXT: 'obj2' '' false
// CHECK-NEXT: 'obj2' 'a string' false
// CHECK-NEXT: 'obj2' 'another string' false
// CHECK-NEXT: 'obj2' 'obj1' false
// CHECK-NEXT: 'obj2' 'obj2' true
// CHECK-NEXT: 'obj2' 'false' false
// CHECK-NEXT: 'obj2' 'true' false
// CHECK-NEXT: 'obj2' 'Symbol(Symbol1)' false
// CHECK-NEXT: 'obj2' 'Symbol(Symbol2)' false
// CHECK-NEXT: 'obj2' '0' false
// CHECK-NEXT: 'obj2' '1' false
// CHECK-NEXT: 'obj2' 'NaN' false
// CHECK-NEXT: 'obj2' '-Infinity' false
// CHECK-NEXT: 'false' 'undefined' false
// CHECK-NEXT: 'false' 'null' false
// CHECK-NEXT: 'false' '' true
// CHECK-NEXT: 'false' 'a string' false
// CHECK-NEXT: 'false' 'another string' false
// CHECK-NEXT: 'false' 'obj1' false
// CHECK-NEXT: 'false' 'obj2' false
// CHECK-NEXT: 'false' 'false' true
// CHECK-NEXT: 'false' 'true' false
// CHECK-NEXT: 'false' 'Symbol(Symbol1)' false
// CHECK-NEXT: 'false' 'Symbol(Symbol2)' false
// CHECK-NEXT: 'false' '0' true
// CHECK-NEXT: 'false' '1' false
// CHECK-NEXT: 'false' 'NaN' false
// CHECK-NEXT: 'false' '-Infinity' false
// CHECK-NEXT: 'true' 'undefined' false
// CHECK-NEXT: 'true' 'null' false
// CHECK-NEXT: 'true' '' false
// CHECK-NEXT: 'true' 'a string' false
// CHECK-NEXT: 'true' 'another string' false
// CHECK-NEXT: 'true' 'obj1' false
// CHECK-NEXT: 'true' 'obj2' false
// CHECK-NEXT: 'true' 'false' false
// CHECK-NEXT: 'true' 'true' true
// CHECK-NEXT: 'true' 'Symbol(Symbol1)' false
// CHECK-NEXT: 'true' 'Symbol(Symbol2)' false
// CHECK-NEXT: 'true' '0' false
// CHECK-NEXT: 'true' '1' true
// CHECK-NEXT: 'true' 'NaN' false
// CHECK-NEXT: 'true' '-Infinity' false
// CHECK-NEXT: 'Symbol(Symbol1)' 'undefined' false
// CHECK-NEXT: 'Symbol(Symbol1)' 'null' false
// CHECK-NEXT: 'Symbol(Symbol1)' '' false
// CHECK-NEXT: 'Symbol(Symbol1)' 'a string' false
// CHECK-NEXT: 'Symbol(Symbol1)' 'another string' false
// CHECK-NEXT: 'Symbol(Symbol1)' 'obj1' false
// CHECK-NEXT: 'Symbol(Symbol1)' 'obj2' false
// CHECK-NEXT: 'Symbol(Symbol1)' 'false' false
// CHECK-NEXT: 'Symbol(Symbol1)' 'true' false
// CHECK-NEXT: 'Symbol(Symbol1)' 'Symbol(Symbol1)' true
// CHECK-NEXT: 'Symbol(Symbol1)' 'Symbol(Symbol2)' false
// CHECK-NEXT: 'Symbol(Symbol1)' '0' false
// CHECK-NEXT: 'Symbol(Symbol1)' '1' false
// CHECK-NEXT: 'Symbol(Symbol1)' 'NaN' false
// CHECK-NEXT: 'Symbol(Symbol1)' '-Infinity' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'undefined' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'null' false
// CHECK-NEXT: 'Symbol(Symbol2)' '' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'a string' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'another string' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'obj1' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'obj2' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'false' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'true' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'Symbol(Symbol1)' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'Symbol(Symbol2)' true
// CHECK-NEXT: 'Symbol(Symbol2)' '0' false
// CHECK-NEXT: 'Symbol(Symbol2)' '1' false
// CHECK-NEXT: 'Symbol(Symbol2)' 'NaN' false
// CHECK-NEXT: 'Symbol(Symbol2)' '-Infinity' false
// CHECK-NEXT: '0' 'undefined' false
// CHECK-NEXT: '0' 'null' false
// CHECK-NEXT: '0' '' true
// CHECK-NEXT: '0' 'a string' false
// CHECK-NEXT: '0' 'another string' false
// CHECK-NEXT: '0' 'obj1' false
// CHECK-NEXT: '0' 'obj2' false
// CHECK-NEXT: '0' 'false' true
// CHECK-NEXT: '0' 'true' false
// CHECK-NEXT: '0' 'Symbol(Symbol1)' false
// CHECK-NEXT: '0' 'Symbol(Symbol2)' false
// CHECK-NEXT: '0' '0' true
// CHECK-NEXT: '0' '1' false
// CHECK-NEXT: '0' 'NaN' false
// CHECK-NEXT: '0' '-Infinity' false
// CHECK-NEXT: '1' 'undefined' false
// CHECK-NEXT: '1' 'null' false
// CHECK-NEXT: '1' '' false
// CHECK-NEXT: '1' 'a string' false
// CHECK-NEXT: '1' 'another string' false
// CHECK-NEXT: '1' 'obj1' false
// CHECK-NEXT: '1' 'obj2' false
// CHECK-NEXT: '1' 'false' false
// CHECK-NEXT: '1' 'true' true
// CHECK-NEXT: '1' 'Symbol(Symbol1)' false
// CHECK-NEXT: '1' 'Symbol(Symbol2)' false
// CHECK-NEXT: '1' '0' false
// CHECK-NEXT: '1' '1' true
// CHECK-NEXT: '1' 'NaN' false
// CHECK-NEXT: '1' '-Infinity' false
// CHECK-NEXT: 'NaN' 'undefined' false
// CHECK-NEXT: 'NaN' 'null' false
// CHECK-NEXT: 'NaN' '' false
// CHECK-NEXT: 'NaN' 'a string' false
// CHECK-NEXT: 'NaN' 'another string' false
// CHECK-NEXT: 'NaN' 'obj1' false
// CHECK-NEXT: 'NaN' 'obj2' false
// CHECK-NEXT: 'NaN' 'false' false
// CHECK-NEXT: 'NaN' 'true' false
// CHECK-NEXT: 'NaN' 'Symbol(Symbol1)' false
// CHECK-NEXT: 'NaN' 'Symbol(Symbol2)' false
// CHECK-NEXT: 'NaN' '0' false
// CHECK-NEXT: 'NaN' '1' false
// CHECK-NEXT: 'NaN' 'NaN' false
// CHECK-NEXT: 'NaN' '-Infinity' false
// CHECK-NEXT: '-Infinity' 'undefined' false
// CHECK-NEXT: '-Infinity' 'null' false
// CHECK-NEXT: '-Infinity' '' false
// CHECK-NEXT: '-Infinity' 'a string' false
// CHECK-NEXT: '-Infinity' 'another string' false
// CHECK-NEXT: '-Infinity' 'obj1' false
// CHECK-NEXT: '-Infinity' 'obj2' false
// CHECK-NEXT: '-Infinity' 'false' false
// CHECK-NEXT: '-Infinity' 'true' false
// CHECK-NEXT: '-Infinity' 'Symbol(Symbol1)' false
// CHECK-NEXT: '-Infinity' 'Symbol(Symbol2)' false
// CHECK-NEXT: '-Infinity' '0' false
// CHECK-NEXT: '-Infinity' '1' false
// CHECK-NEXT: '-Infinity' 'NaN' false
// CHECK-NEXT: '-Infinity' '-Infinity' true
