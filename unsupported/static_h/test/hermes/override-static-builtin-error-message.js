/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -fstatic-builtins %s | %FileCheck --match-full-lines %s

try {
  Math.sin = 2;
} catch (e) {
  print(e.toString());
}
// CHECK: TypeError: Attempting to override read-only builtin method 'sin'

try {
  Array.isArray = 2;
} catch (e) {
  print(e.toString());
}
// CHECK: TypeError: Attempting to override read-only builtin method 'Array.isArray'

try {
  var isArray = 'isArray';
  Array[isArray] = 2;
} catch (e) {
  print(e.toString());
}
// CHECK: TypeError: Attempting to override read-only builtin method 'Array.isArray'

Object.defineProperty(Array, 'name', {configurable : true});
Object.defineProperty(Array, 'name', {writable : true});
Array.name = undefined;
try {
  Array.isArray = 2;
} catch (e) {
  print(e.toString());
}
// CHECK: TypeError: Attempting to override read-only builtin method 'isArray'

Object.defineProperty(Array, 'name', {get: function () {return 1;}, set: function() { return ;}});
try {
  Array.isArray = 2;
} catch (e) {
  print(e.toString());
}
// CHECK: TypeError: Attempting to override read-only builtin method 'isArray'
