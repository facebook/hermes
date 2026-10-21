/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ulimit -s 1024 && %hermes -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// RUN: ulimit -s 1024 && %shermes -exec -Wx,-gc-sanitize-handles=0 -Wx,-max-register-stack=1048576 %s | %FileCheck --match-full-lines %s
// UNSUPPORTED: windows

// JSON.stringify and the JSON.parse reviver recurse natively once per level of
// nesting. When they are called with the native stack nearly exhausted, they
// must throw a RangeError instead of overflowing it.

print('json-native-stack-overflow');
// CHECK-LABEL: json-native-stack-overflow

// Recurse through a native function until the native stack runs out, then
// call f() at each level on the way back until it stops throwing RangeError.
// \return 'ok' if f() returned true, or 'RangeError' if it never succeeded.
function withLittleStack(f) {
  var result;
  function recurse() {
    try {
      [0].forEach(recurse);
    } catch (e) {
      if (!(e instanceof RangeError)) throw e;
    }
    if (result !== undefined) return;
    try {
      result = f() ? 'ok' : 'wrong result';
    } catch (e) {
      if (!(e instanceof RangeError)) result = 'unexpected ' + e;
    }
  }
  recurse();
  return result === undefined ? 'RangeError' : result;
}

function nestedObjects(depth) {
  var o = {};
  for (var i = 1; i < depth; ++i) o = {o: o};
  return o;
}

function nestedArrays(depth) {
  var a = [];
  for (var i = 1; i < depth; ++i) a = [a];
  return a;
}

// Nest as deeply as the nesting limit and the native stack allow. The nesting
// limit is lower in some sanitizer builds, and in unoptimized builds 400
// nested objects do not fit in the 1 MB stack.
var depth = 400;
try {
  JSON.stringify(nestedObjects(depth));
  JSON.stringify(nestedArrays(depth));
} catch (e) {
  depth = 99;
}

var objects = nestedObjects(depth);
var objectsJSON = JSON.stringify(objects);
var arrays = nestedArrays(depth);
var arraysJSON = JSON.stringify(arrays);

print(
  'stringify objects:',
  withLittleStack(function () {
    return JSON.stringify(objects) === objectsJSON;
  }),
);
// CHECK-NEXT: stringify objects: {{ok|RangeError}}

print(
  'stringify arrays:',
  withLittleStack(function () {
    return JSON.stringify(arrays) === arraysJSON;
  }),
);
// CHECK-NEXT: stringify arrays: {{ok|RangeError}}

print(
  'parse with reviver:',
  withLittleStack(function () {
    var calls = 0;
    JSON.parse(arraysJSON, function (key, value) {
      ++calls;
      return value;
    });
    return calls === depth;
  }),
);
// CHECK-NEXT: parse with reviver: {{ok|RangeError}}
