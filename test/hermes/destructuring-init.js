/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines
// RUN: %hermes -O %s | %FileCheck %s --match-full-lines
// RUN: %hermesc %s -emit-binary -out %t.hbc && %hermes %t.hbc | %FileCheck %s --match-full-lines

print('initializers in destructuring');
// CHECK-LABEL: initializers in destructuring

var {abc = function() {}} = {};
print(abc.name);
// CHECK-NEXT: abc
var {['abc']: def = function() {}} = {};
print(def.name);
// CHECK-NEXT: def

var [foo = function() {}, bar = function() {}] = [];
print(foo.name, bar.name);
// CHECK-NEXT: foo bar

var {[undefined]: x = 1, ...x} = 1;
print(typeof(x));
// CHECK-NEXT: object

var {[null]: y = 1, ...y} = 1;
print(typeof(y));
// CHECK-NEXT: object
