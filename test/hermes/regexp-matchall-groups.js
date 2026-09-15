/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// RegExp.prototype[Symbol.matchAll] copies the receiver via the species
// constructor. When the flags are unchanged, that copy reuses the compiled
// bytecode and must also carry over the capture group name mapping, or the
// matches lose their groups object.

print('matchAll groups');
// CHECK-LABEL: matchAll groups

var ms = [...'x1 x2'.matchAll(/x(?<d>\d)/g)];
print(ms.length, JSON.stringify(ms[0].groups), JSON.stringify(ms[1].groups));
// CHECK-NEXT: 2 {"d":"1"} {"d":"2"}

// The same applies to regexp literals, whose mapping is built by the compiler.
var lit = [...'ab'.matchAll(/(?<a>a)|(?<b>b)/g)];
print(JSON.stringify(lit[0].groups), JSON.stringify(lit[1].groups));
// CHECK-NEXT: {"a":"a"} {"b":"b"}

// And to the indices array under the d flag.
var d = [...'ab'.matchAll(/(?<a>a)|(?<b>b)/dg)];
print(JSON.stringify(d[1].indices.groups));
// CHECK-NEXT: {"b":[1,2]}

// A copy made with different flags recompiles the pattern and keeps groups too.
var re = /(?<a>a)/;
var g = new RegExp(re, 'g');
print(JSON.stringify([...'aa'.matchAll(g)].map(function (m) { return m.groups; })));
// CHECK-NEXT: [{"a":"a"},{"a":"a"}]

// Without named groups, groups stays undefined.
print([...'aa'.matchAll(/a/g)][0].groups);
// CHECK-NEXT: undefined
