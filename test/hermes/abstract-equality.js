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
var str4 = "another string";
var str5 = str3;
var bigint1 = BigInt(0);
var bigint2 = BigInt(1);
var bigint3 = BigInt(0);
var bigint4 = bigint1;
var obj1 = { field0: "1", toString: () => "obj1" };
var obj2 = { field0: "1", toString: () => "obj2" };
var obj3 = { field0: "1", toString: () => "obj2" };
var obj4 = obj1;
var sym1 =  Symbol("Symbol1");
var sym2 =  Symbol("Symbol2");
var sym3 =  Symbol("Symbol1");
var sym4 =  sym1;

const values = [
    undefined,
    null,
    str1, str2, str3, str4, str5,
    bigint1, bigint2, bigint3, bigint4,
    obj1, obj2, obj3, obj4,
    false, true,
    sym1, sym2, sym3, sym4,
    0.0, 1.0, -NaN, -Infinity,
];

function printValuesAndEqualityValue(i, j) {
    var v0 = values[i];
    var v1 = values[j];
    var s0 = "'" + String(v0) + "'";
    var s1 = "'" + String(v1) + "'";

    print(s0, "("+i+")", s1, "("+j+")", v0 == v1);
}

print("Abstract Equality Test");
for (var i in values) {
    for (var j in values) {
        printValuesAndEqualityValue(i, j);
    }
}

// CHECK-LABEL: Abstract Equality Test
// CHECK-NEXT: 'undefined' (0) 'undefined' (0) true
// CHECK-NEXT: 'undefined' (0) 'null' (1) true
// CHECK-NEXT: 'undefined' (0) '' (2) false
// CHECK-NEXT: 'undefined' (0) 'a string' (3) false
// CHECK-NEXT: 'undefined' (0) 'another string' (4) false
// CHECK-NEXT: 'undefined' (0) 'another string' (5) false
// CHECK-NEXT: 'undefined' (0) 'another string' (6) false
// CHECK-NEXT: 'undefined' (0) '0' (7) false
// CHECK-NEXT: 'undefined' (0) '1' (8) false
// CHECK-NEXT: 'undefined' (0) '0' (9) false
// CHECK-NEXT: 'undefined' (0) '0' (10) false
// CHECK-NEXT: 'undefined' (0) 'obj1' (11) false
// CHECK-NEXT: 'undefined' (0) 'obj2' (12) false
// CHECK-NEXT: 'undefined' (0) 'obj2' (13) false
// CHECK-NEXT: 'undefined' (0) 'obj1' (14) false
// CHECK-NEXT: 'undefined' (0) 'false' (15) false
// CHECK-NEXT: 'undefined' (0) 'true' (16) false
// CHECK-NEXT: 'undefined' (0) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'undefined' (0) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'undefined' (0) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'undefined' (0) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'undefined' (0) '0' (21) false
// CHECK-NEXT: 'undefined' (0) '1' (22) false
// CHECK-NEXT: 'undefined' (0) 'NaN' (23) false
// CHECK-NEXT: 'undefined' (0) '-Infinity' (24) false
// CHECK-NEXT: 'null' (1) 'undefined' (0) true
// CHECK-NEXT: 'null' (1) 'null' (1) true
// CHECK-NEXT: 'null' (1) '' (2) false
// CHECK-NEXT: 'null' (1) 'a string' (3) false
// CHECK-NEXT: 'null' (1) 'another string' (4) false
// CHECK-NEXT: 'null' (1) 'another string' (5) false
// CHECK-NEXT: 'null' (1) 'another string' (6) false
// CHECK-NEXT: 'null' (1) '0' (7) false
// CHECK-NEXT: 'null' (1) '1' (8) false
// CHECK-NEXT: 'null' (1) '0' (9) false
// CHECK-NEXT: 'null' (1) '0' (10) false
// CHECK-NEXT: 'null' (1) 'obj1' (11) false
// CHECK-NEXT: 'null' (1) 'obj2' (12) false
// CHECK-NEXT: 'null' (1) 'obj2' (13) false
// CHECK-NEXT: 'null' (1) 'obj1' (14) false
// CHECK-NEXT: 'null' (1) 'false' (15) false
// CHECK-NEXT: 'null' (1) 'true' (16) false
// CHECK-NEXT: 'null' (1) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'null' (1) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'null' (1) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'null' (1) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'null' (1) '0' (21) false
// CHECK-NEXT: 'null' (1) '1' (22) false
// CHECK-NEXT: 'null' (1) 'NaN' (23) false
// CHECK-NEXT: 'null' (1) '-Infinity' (24) false
// CHECK-NEXT: '' (2) 'undefined' (0) false
// CHECK-NEXT: '' (2) 'null' (1) false
// CHECK-NEXT: '' (2) '' (2) true
// CHECK-NEXT: '' (2) 'a string' (3) false
// CHECK-NEXT: '' (2) 'another string' (4) false
// CHECK-NEXT: '' (2) 'another string' (5) false
// CHECK-NEXT: '' (2) 'another string' (6) false
// CHECK-NEXT: '' (2) '0' (7) true
// CHECK-NEXT: '' (2) '1' (8) false
// CHECK-NEXT: '' (2) '0' (9) true
// CHECK-NEXT: '' (2) '0' (10) true
// CHECK-NEXT: '' (2) 'obj1' (11) false
// CHECK-NEXT: '' (2) 'obj2' (12) false
// CHECK-NEXT: '' (2) 'obj2' (13) false
// CHECK-NEXT: '' (2) 'obj1' (14) false
// CHECK-NEXT: '' (2) 'false' (15) true
// CHECK-NEXT: '' (2) 'true' (16) false
// CHECK-NEXT: '' (2) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: '' (2) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: '' (2) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: '' (2) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: '' (2) '0' (21) true
// CHECK-NEXT: '' (2) '1' (22) false
// CHECK-NEXT: '' (2) 'NaN' (23) false
// CHECK-NEXT: '' (2) '-Infinity' (24) false
// CHECK-NEXT: 'a string' (3) 'undefined' (0) false
// CHECK-NEXT: 'a string' (3) 'null' (1) false
// CHECK-NEXT: 'a string' (3) '' (2) false
// CHECK-NEXT: 'a string' (3) 'a string' (3) true
// CHECK-NEXT: 'a string' (3) 'another string' (4) false
// CHECK-NEXT: 'a string' (3) 'another string' (5) false
// CHECK-NEXT: 'a string' (3) 'another string' (6) false
// CHECK-NEXT: 'a string' (3) '0' (7) false
// CHECK-NEXT: 'a string' (3) '1' (8) false
// CHECK-NEXT: 'a string' (3) '0' (9) false
// CHECK-NEXT: 'a string' (3) '0' (10) false
// CHECK-NEXT: 'a string' (3) 'obj1' (11) false
// CHECK-NEXT: 'a string' (3) 'obj2' (12) false
// CHECK-NEXT: 'a string' (3) 'obj2' (13) false
// CHECK-NEXT: 'a string' (3) 'obj1' (14) false
// CHECK-NEXT: 'a string' (3) 'false' (15) false
// CHECK-NEXT: 'a string' (3) 'true' (16) false
// CHECK-NEXT: 'a string' (3) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'a string' (3) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'a string' (3) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'a string' (3) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'a string' (3) '0' (21) false
// CHECK-NEXT: 'a string' (3) '1' (22) false
// CHECK-NEXT: 'a string' (3) 'NaN' (23) false
// CHECK-NEXT: 'a string' (3) '-Infinity' (24) false
// CHECK-NEXT: 'another string' (4) 'undefined' (0) false
// CHECK-NEXT: 'another string' (4) 'null' (1) false
// CHECK-NEXT: 'another string' (4) '' (2) false
// CHECK-NEXT: 'another string' (4) 'a string' (3) false
// CHECK-NEXT: 'another string' (4) 'another string' (4) true
// CHECK-NEXT: 'another string' (4) 'another string' (5) true
// CHECK-NEXT: 'another string' (4) 'another string' (6) true
// CHECK-NEXT: 'another string' (4) '0' (7) false
// CHECK-NEXT: 'another string' (4) '1' (8) false
// CHECK-NEXT: 'another string' (4) '0' (9) false
// CHECK-NEXT: 'another string' (4) '0' (10) false
// CHECK-NEXT: 'another string' (4) 'obj1' (11) false
// CHECK-NEXT: 'another string' (4) 'obj2' (12) false
// CHECK-NEXT: 'another string' (4) 'obj2' (13) false
// CHECK-NEXT: 'another string' (4) 'obj1' (14) false
// CHECK-NEXT: 'another string' (4) 'false' (15) false
// CHECK-NEXT: 'another string' (4) 'true' (16) false
// CHECK-NEXT: 'another string' (4) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'another string' (4) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'another string' (4) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'another string' (4) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'another string' (4) '0' (21) false
// CHECK-NEXT: 'another string' (4) '1' (22) false
// CHECK-NEXT: 'another string' (4) 'NaN' (23) false
// CHECK-NEXT: 'another string' (4) '-Infinity' (24) false
// CHECK-NEXT: 'another string' (5) 'undefined' (0) false
// CHECK-NEXT: 'another string' (5) 'null' (1) false
// CHECK-NEXT: 'another string' (5) '' (2) false
// CHECK-NEXT: 'another string' (5) 'a string' (3) false
// CHECK-NEXT: 'another string' (5) 'another string' (4) true
// CHECK-NEXT: 'another string' (5) 'another string' (5) true
// CHECK-NEXT: 'another string' (5) 'another string' (6) true
// CHECK-NEXT: 'another string' (5) '0' (7) false
// CHECK-NEXT: 'another string' (5) '1' (8) false
// CHECK-NEXT: 'another string' (5) '0' (9) false
// CHECK-NEXT: 'another string' (5) '0' (10) false
// CHECK-NEXT: 'another string' (5) 'obj1' (11) false
// CHECK-NEXT: 'another string' (5) 'obj2' (12) false
// CHECK-NEXT: 'another string' (5) 'obj2' (13) false
// CHECK-NEXT: 'another string' (5) 'obj1' (14) false
// CHECK-NEXT: 'another string' (5) 'false' (15) false
// CHECK-NEXT: 'another string' (5) 'true' (16) false
// CHECK-NEXT: 'another string' (5) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'another string' (5) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'another string' (5) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'another string' (5) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'another string' (5) '0' (21) false
// CHECK-NEXT: 'another string' (5) '1' (22) false
// CHECK-NEXT: 'another string' (5) 'NaN' (23) false
// CHECK-NEXT: 'another string' (5) '-Infinity' (24) false
// CHECK-NEXT: 'another string' (6) 'undefined' (0) false
// CHECK-NEXT: 'another string' (6) 'null' (1) false
// CHECK-NEXT: 'another string' (6) '' (2) false
// CHECK-NEXT: 'another string' (6) 'a string' (3) false
// CHECK-NEXT: 'another string' (6) 'another string' (4) true
// CHECK-NEXT: 'another string' (6) 'another string' (5) true
// CHECK-NEXT: 'another string' (6) 'another string' (6) true
// CHECK-NEXT: 'another string' (6) '0' (7) false
// CHECK-NEXT: 'another string' (6) '1' (8) false
// CHECK-NEXT: 'another string' (6) '0' (9) false
// CHECK-NEXT: 'another string' (6) '0' (10) false
// CHECK-NEXT: 'another string' (6) 'obj1' (11) false
// CHECK-NEXT: 'another string' (6) 'obj2' (12) false
// CHECK-NEXT: 'another string' (6) 'obj2' (13) false
// CHECK-NEXT: 'another string' (6) 'obj1' (14) false
// CHECK-NEXT: 'another string' (6) 'false' (15) false
// CHECK-NEXT: 'another string' (6) 'true' (16) false
// CHECK-NEXT: 'another string' (6) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'another string' (6) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'another string' (6) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'another string' (6) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'another string' (6) '0' (21) false
// CHECK-NEXT: 'another string' (6) '1' (22) false
// CHECK-NEXT: 'another string' (6) 'NaN' (23) false
// CHECK-NEXT: 'another string' (6) '-Infinity' (24) false
// CHECK-NEXT: '0' (7) 'undefined' (0) false
// CHECK-NEXT: '0' (7) 'null' (1) false
// CHECK-NEXT: '0' (7) '' (2) true
// CHECK-NEXT: '0' (7) 'a string' (3) false
// CHECK-NEXT: '0' (7) 'another string' (4) false
// CHECK-NEXT: '0' (7) 'another string' (5) false
// CHECK-NEXT: '0' (7) 'another string' (6) false
// CHECK-NEXT: '0' (7) '0' (7) true
// CHECK-NEXT: '0' (7) '1' (8) false
// CHECK-NEXT: '0' (7) '0' (9) true
// CHECK-NEXT: '0' (7) '0' (10) true
// CHECK-NEXT: '0' (7) 'obj1' (11) false
// CHECK-NEXT: '0' (7) 'obj2' (12) false
// CHECK-NEXT: '0' (7) 'obj2' (13) false
// CHECK-NEXT: '0' (7) 'obj1' (14) false
// CHECK-NEXT: '0' (7) 'false' (15) true
// CHECK-NEXT: '0' (7) 'true' (16) false
// CHECK-NEXT: '0' (7) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: '0' (7) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: '0' (7) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: '0' (7) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: '0' (7) '0' (21) true
// CHECK-NEXT: '0' (7) '1' (22) false
// CHECK-NEXT: '0' (7) 'NaN' (23) false
// CHECK-NEXT: '0' (7) '-Infinity' (24) false
// CHECK-NEXT: '1' (8) 'undefined' (0) false
// CHECK-NEXT: '1' (8) 'null' (1) false
// CHECK-NEXT: '1' (8) '' (2) false
// CHECK-NEXT: '1' (8) 'a string' (3) false
// CHECK-NEXT: '1' (8) 'another string' (4) false
// CHECK-NEXT: '1' (8) 'another string' (5) false
// CHECK-NEXT: '1' (8) 'another string' (6) false
// CHECK-NEXT: '1' (8) '0' (7) false
// CHECK-NEXT: '1' (8) '1' (8) true
// CHECK-NEXT: '1' (8) '0' (9) false
// CHECK-NEXT: '1' (8) '0' (10) false
// CHECK-NEXT: '1' (8) 'obj1' (11) false
// CHECK-NEXT: '1' (8) 'obj2' (12) false
// CHECK-NEXT: '1' (8) 'obj2' (13) false
// CHECK-NEXT: '1' (8) 'obj1' (14) false
// CHECK-NEXT: '1' (8) 'false' (15) false
// CHECK-NEXT: '1' (8) 'true' (16) true
// CHECK-NEXT: '1' (8) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: '1' (8) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: '1' (8) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: '1' (8) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: '1' (8) '0' (21) false
// CHECK-NEXT: '1' (8) '1' (22) true
// CHECK-NEXT: '1' (8) 'NaN' (23) false
// CHECK-NEXT: '1' (8) '-Infinity' (24) false
// CHECK-NEXT: '0' (9) 'undefined' (0) false
// CHECK-NEXT: '0' (9) 'null' (1) false
// CHECK-NEXT: '0' (9) '' (2) true
// CHECK-NEXT: '0' (9) 'a string' (3) false
// CHECK-NEXT: '0' (9) 'another string' (4) false
// CHECK-NEXT: '0' (9) 'another string' (5) false
// CHECK-NEXT: '0' (9) 'another string' (6) false
// CHECK-NEXT: '0' (9) '0' (7) true
// CHECK-NEXT: '0' (9) '1' (8) false
// CHECK-NEXT: '0' (9) '0' (9) true
// CHECK-NEXT: '0' (9) '0' (10) true
// CHECK-NEXT: '0' (9) 'obj1' (11) false
// CHECK-NEXT: '0' (9) 'obj2' (12) false
// CHECK-NEXT: '0' (9) 'obj2' (13) false
// CHECK-NEXT: '0' (9) 'obj1' (14) false
// CHECK-NEXT: '0' (9) 'false' (15) true
// CHECK-NEXT: '0' (9) 'true' (16) false
// CHECK-NEXT: '0' (9) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: '0' (9) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: '0' (9) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: '0' (9) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: '0' (9) '0' (21) true
// CHECK-NEXT: '0' (9) '1' (22) false
// CHECK-NEXT: '0' (9) 'NaN' (23) false
// CHECK-NEXT: '0' (9) '-Infinity' (24) false
// CHECK-NEXT: '0' (10) 'undefined' (0) false
// CHECK-NEXT: '0' (10) 'null' (1) false
// CHECK-NEXT: '0' (10) '' (2) true
// CHECK-NEXT: '0' (10) 'a string' (3) false
// CHECK-NEXT: '0' (10) 'another string' (4) false
// CHECK-NEXT: '0' (10) 'another string' (5) false
// CHECK-NEXT: '0' (10) 'another string' (6) false
// CHECK-NEXT: '0' (10) '0' (7) true
// CHECK-NEXT: '0' (10) '1' (8) false
// CHECK-NEXT: '0' (10) '0' (9) true
// CHECK-NEXT: '0' (10) '0' (10) true
// CHECK-NEXT: '0' (10) 'obj1' (11) false
// CHECK-NEXT: '0' (10) 'obj2' (12) false
// CHECK-NEXT: '0' (10) 'obj2' (13) false
// CHECK-NEXT: '0' (10) 'obj1' (14) false
// CHECK-NEXT: '0' (10) 'false' (15) true
// CHECK-NEXT: '0' (10) 'true' (16) false
// CHECK-NEXT: '0' (10) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: '0' (10) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: '0' (10) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: '0' (10) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: '0' (10) '0' (21) true
// CHECK-NEXT: '0' (10) '1' (22) false
// CHECK-NEXT: '0' (10) 'NaN' (23) false
// CHECK-NEXT: '0' (10) '-Infinity' (24) false
// CHECK-NEXT: 'obj1' (11) 'undefined' (0) false
// CHECK-NEXT: 'obj1' (11) 'null' (1) false
// CHECK-NEXT: 'obj1' (11) '' (2) false
// CHECK-NEXT: 'obj1' (11) 'a string' (3) false
// CHECK-NEXT: 'obj1' (11) 'another string' (4) false
// CHECK-NEXT: 'obj1' (11) 'another string' (5) false
// CHECK-NEXT: 'obj1' (11) 'another string' (6) false
// CHECK-NEXT: 'obj1' (11) '0' (7) false
// CHECK-NEXT: 'obj1' (11) '1' (8) false
// CHECK-NEXT: 'obj1' (11) '0' (9) false
// CHECK-NEXT: 'obj1' (11) '0' (10) false
// CHECK-NEXT: 'obj1' (11) 'obj1' (11) true
// CHECK-NEXT: 'obj1' (11) 'obj2' (12) false
// CHECK-NEXT: 'obj1' (11) 'obj2' (13) false
// CHECK-NEXT: 'obj1' (11) 'obj1' (14) true
// CHECK-NEXT: 'obj1' (11) 'false' (15) false
// CHECK-NEXT: 'obj1' (11) 'true' (16) false
// CHECK-NEXT: 'obj1' (11) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'obj1' (11) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'obj1' (11) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'obj1' (11) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'obj1' (11) '0' (21) false
// CHECK-NEXT: 'obj1' (11) '1' (22) false
// CHECK-NEXT: 'obj1' (11) 'NaN' (23) false
// CHECK-NEXT: 'obj1' (11) '-Infinity' (24) false
// CHECK-NEXT: 'obj2' (12) 'undefined' (0) false
// CHECK-NEXT: 'obj2' (12) 'null' (1) false
// CHECK-NEXT: 'obj2' (12) '' (2) false
// CHECK-NEXT: 'obj2' (12) 'a string' (3) false
// CHECK-NEXT: 'obj2' (12) 'another string' (4) false
// CHECK-NEXT: 'obj2' (12) 'another string' (5) false
// CHECK-NEXT: 'obj2' (12) 'another string' (6) false
// CHECK-NEXT: 'obj2' (12) '0' (7) false
// CHECK-NEXT: 'obj2' (12) '1' (8) false
// CHECK-NEXT: 'obj2' (12) '0' (9) false
// CHECK-NEXT: 'obj2' (12) '0' (10) false
// CHECK-NEXT: 'obj2' (12) 'obj1' (11) false
// CHECK-NEXT: 'obj2' (12) 'obj2' (12) true
// CHECK-NEXT: 'obj2' (12) 'obj2' (13) false
// CHECK-NEXT: 'obj2' (12) 'obj1' (14) false
// CHECK-NEXT: 'obj2' (12) 'false' (15) false
// CHECK-NEXT: 'obj2' (12) 'true' (16) false
// CHECK-NEXT: 'obj2' (12) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'obj2' (12) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'obj2' (12) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'obj2' (12) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'obj2' (12) '0' (21) false
// CHECK-NEXT: 'obj2' (12) '1' (22) false
// CHECK-NEXT: 'obj2' (12) 'NaN' (23) false
// CHECK-NEXT: 'obj2' (12) '-Infinity' (24) false
// CHECK-NEXT: 'obj2' (13) 'undefined' (0) false
// CHECK-NEXT: 'obj2' (13) 'null' (1) false
// CHECK-NEXT: 'obj2' (13) '' (2) false
// CHECK-NEXT: 'obj2' (13) 'a string' (3) false
// CHECK-NEXT: 'obj2' (13) 'another string' (4) false
// CHECK-NEXT: 'obj2' (13) 'another string' (5) false
// CHECK-NEXT: 'obj2' (13) 'another string' (6) false
// CHECK-NEXT: 'obj2' (13) '0' (7) false
// CHECK-NEXT: 'obj2' (13) '1' (8) false
// CHECK-NEXT: 'obj2' (13) '0' (9) false
// CHECK-NEXT: 'obj2' (13) '0' (10) false
// CHECK-NEXT: 'obj2' (13) 'obj1' (11) false
// CHECK-NEXT: 'obj2' (13) 'obj2' (12) false
// CHECK-NEXT: 'obj2' (13) 'obj2' (13) true
// CHECK-NEXT: 'obj2' (13) 'obj1' (14) false
// CHECK-NEXT: 'obj2' (13) 'false' (15) false
// CHECK-NEXT: 'obj2' (13) 'true' (16) false
// CHECK-NEXT: 'obj2' (13) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'obj2' (13) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'obj2' (13) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'obj2' (13) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'obj2' (13) '0' (21) false
// CHECK-NEXT: 'obj2' (13) '1' (22) false
// CHECK-NEXT: 'obj2' (13) 'NaN' (23) false
// CHECK-NEXT: 'obj2' (13) '-Infinity' (24) false
// CHECK-NEXT: 'obj1' (14) 'undefined' (0) false
// CHECK-NEXT: 'obj1' (14) 'null' (1) false
// CHECK-NEXT: 'obj1' (14) '' (2) false
// CHECK-NEXT: 'obj1' (14) 'a string' (3) false
// CHECK-NEXT: 'obj1' (14) 'another string' (4) false
// CHECK-NEXT: 'obj1' (14) 'another string' (5) false
// CHECK-NEXT: 'obj1' (14) 'another string' (6) false
// CHECK-NEXT: 'obj1' (14) '0' (7) false
// CHECK-NEXT: 'obj1' (14) '1' (8) false
// CHECK-NEXT: 'obj1' (14) '0' (9) false
// CHECK-NEXT: 'obj1' (14) '0' (10) false
// CHECK-NEXT: 'obj1' (14) 'obj1' (11) true
// CHECK-NEXT: 'obj1' (14) 'obj2' (12) false
// CHECK-NEXT: 'obj1' (14) 'obj2' (13) false
// CHECK-NEXT: 'obj1' (14) 'obj1' (14) true
// CHECK-NEXT: 'obj1' (14) 'false' (15) false
// CHECK-NEXT: 'obj1' (14) 'true' (16) false
// CHECK-NEXT: 'obj1' (14) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'obj1' (14) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'obj1' (14) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'obj1' (14) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'obj1' (14) '0' (21) false
// CHECK-NEXT: 'obj1' (14) '1' (22) false
// CHECK-NEXT: 'obj1' (14) 'NaN' (23) false
// CHECK-NEXT: 'obj1' (14) '-Infinity' (24) false
// CHECK-NEXT: 'false' (15) 'undefined' (0) false
// CHECK-NEXT: 'false' (15) 'null' (1) false
// CHECK-NEXT: 'false' (15) '' (2) true
// CHECK-NEXT: 'false' (15) 'a string' (3) false
// CHECK-NEXT: 'false' (15) 'another string' (4) false
// CHECK-NEXT: 'false' (15) 'another string' (5) false
// CHECK-NEXT: 'false' (15) 'another string' (6) false
// CHECK-NEXT: 'false' (15) '0' (7) true
// CHECK-NEXT: 'false' (15) '1' (8) false
// CHECK-NEXT: 'false' (15) '0' (9) true
// CHECK-NEXT: 'false' (15) '0' (10) true
// CHECK-NEXT: 'false' (15) 'obj1' (11) false
// CHECK-NEXT: 'false' (15) 'obj2' (12) false
// CHECK-NEXT: 'false' (15) 'obj2' (13) false
// CHECK-NEXT: 'false' (15) 'obj1' (14) false
// CHECK-NEXT: 'false' (15) 'false' (15) true
// CHECK-NEXT: 'false' (15) 'true' (16) false
// CHECK-NEXT: 'false' (15) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'false' (15) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'false' (15) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'false' (15) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'false' (15) '0' (21) true
// CHECK-NEXT: 'false' (15) '1' (22) false
// CHECK-NEXT: 'false' (15) 'NaN' (23) false
// CHECK-NEXT: 'false' (15) '-Infinity' (24) false
// CHECK-NEXT: 'true' (16) 'undefined' (0) false
// CHECK-NEXT: 'true' (16) 'null' (1) false
// CHECK-NEXT: 'true' (16) '' (2) false
// CHECK-NEXT: 'true' (16) 'a string' (3) false
// CHECK-NEXT: 'true' (16) 'another string' (4) false
// CHECK-NEXT: 'true' (16) 'another string' (5) false
// CHECK-NEXT: 'true' (16) 'another string' (6) false
// CHECK-NEXT: 'true' (16) '0' (7) false
// CHECK-NEXT: 'true' (16) '1' (8) true
// CHECK-NEXT: 'true' (16) '0' (9) false
// CHECK-NEXT: 'true' (16) '0' (10) false
// CHECK-NEXT: 'true' (16) 'obj1' (11) false
// CHECK-NEXT: 'true' (16) 'obj2' (12) false
// CHECK-NEXT: 'true' (16) 'obj2' (13) false
// CHECK-NEXT: 'true' (16) 'obj1' (14) false
// CHECK-NEXT: 'true' (16) 'false' (15) false
// CHECK-NEXT: 'true' (16) 'true' (16) true
// CHECK-NEXT: 'true' (16) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'true' (16) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'true' (16) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'true' (16) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'true' (16) '0' (21) false
// CHECK-NEXT: 'true' (16) '1' (22) true
// CHECK-NEXT: 'true' (16) 'NaN' (23) false
// CHECK-NEXT: 'true' (16) '-Infinity' (24) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'undefined' (0) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'null' (1) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) '' (2) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'a string' (3) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'another string' (4) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'another string' (5) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'another string' (6) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) '0' (7) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) '1' (8) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) '0' (9) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) '0' (10) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'obj1' (11) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'obj2' (12) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'obj2' (13) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'obj1' (14) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'false' (15) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'true' (16) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'Symbol(Symbol1)' (17) true
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'Symbol(Symbol1)' (20) true
// CHECK-NEXT: 'Symbol(Symbol1)' (17) '0' (21) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) '1' (22) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) 'NaN' (23) false
// CHECK-NEXT: 'Symbol(Symbol1)' (17) '-Infinity' (24) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'undefined' (0) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'null' (1) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) '' (2) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'a string' (3) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'another string' (4) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'another string' (5) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'another string' (6) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) '0' (7) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) '1' (8) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) '0' (9) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) '0' (10) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'obj1' (11) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'obj2' (12) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'obj2' (13) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'obj1' (14) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'false' (15) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'true' (16) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'Symbol(Symbol2)' (18) true
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) '0' (21) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) '1' (22) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) 'NaN' (23) false
// CHECK-NEXT: 'Symbol(Symbol2)' (18) '-Infinity' (24) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'undefined' (0) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'null' (1) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) '' (2) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'a string' (3) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'another string' (4) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'another string' (5) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'another string' (6) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) '0' (7) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) '1' (8) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) '0' (9) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) '0' (10) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'obj1' (11) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'obj2' (12) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'obj2' (13) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'obj1' (14) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'false' (15) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'true' (16) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'Symbol(Symbol1)' (19) true
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) '0' (21) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) '1' (22) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) 'NaN' (23) false
// CHECK-NEXT: 'Symbol(Symbol1)' (19) '-Infinity' (24) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'undefined' (0) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'null' (1) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) '' (2) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'a string' (3) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'another string' (4) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'another string' (5) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'another string' (6) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) '0' (7) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) '1' (8) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) '0' (9) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) '0' (10) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'obj1' (11) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'obj2' (12) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'obj2' (13) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'obj1' (14) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'false' (15) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'true' (16) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'Symbol(Symbol1)' (17) true
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'Symbol(Symbol1)' (20) true
// CHECK-NEXT: 'Symbol(Symbol1)' (20) '0' (21) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) '1' (22) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) 'NaN' (23) false
// CHECK-NEXT: 'Symbol(Symbol1)' (20) '-Infinity' (24) false
// CHECK-NEXT: '0' (21) 'undefined' (0) false
// CHECK-NEXT: '0' (21) 'null' (1) false
// CHECK-NEXT: '0' (21) '' (2) true
// CHECK-NEXT: '0' (21) 'a string' (3) false
// CHECK-NEXT: '0' (21) 'another string' (4) false
// CHECK-NEXT: '0' (21) 'another string' (5) false
// CHECK-NEXT: '0' (21) 'another string' (6) false
// CHECK-NEXT: '0' (21) '0' (7) true
// CHECK-NEXT: '0' (21) '1' (8) false
// CHECK-NEXT: '0' (21) '0' (9) true
// CHECK-NEXT: '0' (21) '0' (10) true
// CHECK-NEXT: '0' (21) 'obj1' (11) false
// CHECK-NEXT: '0' (21) 'obj2' (12) false
// CHECK-NEXT: '0' (21) 'obj2' (13) false
// CHECK-NEXT: '0' (21) 'obj1' (14) false
// CHECK-NEXT: '0' (21) 'false' (15) true
// CHECK-NEXT: '0' (21) 'true' (16) false
// CHECK-NEXT: '0' (21) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: '0' (21) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: '0' (21) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: '0' (21) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: '0' (21) '0' (21) true
// CHECK-NEXT: '0' (21) '1' (22) false
// CHECK-NEXT: '0' (21) 'NaN' (23) false
// CHECK-NEXT: '0' (21) '-Infinity' (24) false
// CHECK-NEXT: '1' (22) 'undefined' (0) false
// CHECK-NEXT: '1' (22) 'null' (1) false
// CHECK-NEXT: '1' (22) '' (2) false
// CHECK-NEXT: '1' (22) 'a string' (3) false
// CHECK-NEXT: '1' (22) 'another string' (4) false
// CHECK-NEXT: '1' (22) 'another string' (5) false
// CHECK-NEXT: '1' (22) 'another string' (6) false
// CHECK-NEXT: '1' (22) '0' (7) false
// CHECK-NEXT: '1' (22) '1' (8) true
// CHECK-NEXT: '1' (22) '0' (9) false
// CHECK-NEXT: '1' (22) '0' (10) false
// CHECK-NEXT: '1' (22) 'obj1' (11) false
// CHECK-NEXT: '1' (22) 'obj2' (12) false
// CHECK-NEXT: '1' (22) 'obj2' (13) false
// CHECK-NEXT: '1' (22) 'obj1' (14) false
// CHECK-NEXT: '1' (22) 'false' (15) false
// CHECK-NEXT: '1' (22) 'true' (16) true
// CHECK-NEXT: '1' (22) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: '1' (22) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: '1' (22) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: '1' (22) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: '1' (22) '0' (21) false
// CHECK-NEXT: '1' (22) '1' (22) true
// CHECK-NEXT: '1' (22) 'NaN' (23) false
// CHECK-NEXT: '1' (22) '-Infinity' (24) false
// CHECK-NEXT: 'NaN' (23) 'undefined' (0) false
// CHECK-NEXT: 'NaN' (23) 'null' (1) false
// CHECK-NEXT: 'NaN' (23) '' (2) false
// CHECK-NEXT: 'NaN' (23) 'a string' (3) false
// CHECK-NEXT: 'NaN' (23) 'another string' (4) false
// CHECK-NEXT: 'NaN' (23) 'another string' (5) false
// CHECK-NEXT: 'NaN' (23) 'another string' (6) false
// CHECK-NEXT: 'NaN' (23) '0' (7) false
// CHECK-NEXT: 'NaN' (23) '1' (8) false
// CHECK-NEXT: 'NaN' (23) '0' (9) false
// CHECK-NEXT: 'NaN' (23) '0' (10) false
// CHECK-NEXT: 'NaN' (23) 'obj1' (11) false
// CHECK-NEXT: 'NaN' (23) 'obj2' (12) false
// CHECK-NEXT: 'NaN' (23) 'obj2' (13) false
// CHECK-NEXT: 'NaN' (23) 'obj1' (14) false
// CHECK-NEXT: 'NaN' (23) 'false' (15) false
// CHECK-NEXT: 'NaN' (23) 'true' (16) false
// CHECK-NEXT: 'NaN' (23) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: 'NaN' (23) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: 'NaN' (23) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: 'NaN' (23) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: 'NaN' (23) '0' (21) false
// CHECK-NEXT: 'NaN' (23) '1' (22) false
// CHECK-NEXT: 'NaN' (23) 'NaN' (23) false
// CHECK-NEXT: 'NaN' (23) '-Infinity' (24) false
// CHECK-NEXT: '-Infinity' (24) 'undefined' (0) false
// CHECK-NEXT: '-Infinity' (24) 'null' (1) false
// CHECK-NEXT: '-Infinity' (24) '' (2) false
// CHECK-NEXT: '-Infinity' (24) 'a string' (3) false
// CHECK-NEXT: '-Infinity' (24) 'another string' (4) false
// CHECK-NEXT: '-Infinity' (24) 'another string' (5) false
// CHECK-NEXT: '-Infinity' (24) 'another string' (6) false
// CHECK-NEXT: '-Infinity' (24) '0' (7) false
// CHECK-NEXT: '-Infinity' (24) '1' (8) false
// CHECK-NEXT: '-Infinity' (24) '0' (9) false
// CHECK-NEXT: '-Infinity' (24) '0' (10) false
// CHECK-NEXT: '-Infinity' (24) 'obj1' (11) false
// CHECK-NEXT: '-Infinity' (24) 'obj2' (12) false
// CHECK-NEXT: '-Infinity' (24) 'obj2' (13) false
// CHECK-NEXT: '-Infinity' (24) 'obj1' (14) false
// CHECK-NEXT: '-Infinity' (24) 'false' (15) false
// CHECK-NEXT: '-Infinity' (24) 'true' (16) false
// CHECK-NEXT: '-Infinity' (24) 'Symbol(Symbol1)' (17) false
// CHECK-NEXT: '-Infinity' (24) 'Symbol(Symbol2)' (18) false
// CHECK-NEXT: '-Infinity' (24) 'Symbol(Symbol1)' (19) false
// CHECK-NEXT: '-Infinity' (24) 'Symbol(Symbol1)' (20) false
// CHECK-NEXT: '-Infinity' (24) '0' (21) false
// CHECK-NEXT: '-Infinity' (24) '1' (22) false
// CHECK-NEXT: '-Infinity' (24) 'NaN' (23) false
// CHECK-NEXT: '-Infinity' (24) '-Infinity' (24) true
