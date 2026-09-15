/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Regression: a MemberExpression buried in a destructuring assignment LHS
// must be classified as a write target, not a read. Before, the classifier
// looked only at the immediate parent (PropertyNode / ArrayPattern element),
// so readonly/writeonly diagnostics fired on the wrong side.

function objDestrReadonly(o: {+x: number}, src: {a: number}): void {
  // Inside the destructuring pattern, `o.x` is a write of the readonly
  // field — must error.
  ({a: o.x} = src);
}

function objDestrWriteonly(o: {-y: number}, src: {a: number}): void {
  // `o.y` is a pure write here, so the writeonly field is fine — no error.
  ({a: o.y} = src);
}

function arrDestrReadonly(o: {+x: number}, src: [number]): void {
  // Array destructuring slot is a write.
  [o.x] = src;
}

function arrDestrWriteonly(o: {-y: number}, src: [number]): void {
  // Array destructuring slot is a write — writeonly is fine.
  [o.y] = src;
}

function nestedDestrReadonly(
  o: {+x: number},
  src: {a: {b: number}},
): void {
  // Nested pattern: still a write.
  ({a: {b: o.x}} = src);
}

function defaultDestrReadonly(
  o: {+x: number},
  src: {a: number | void},
): void {
  // Default-value pattern: the binding side is still a write.
  ({a: o.x = 1} = src);
}

function defaultReadsWriteonly(
  o: {-y: number},
  src: {a: number | void},
): void {
  // The default value itself is a normal read context. Using a writeonly
  // field as the default expression must error.
  ({a: o.y = o.y} = src);
}

function restDestrReadonly(
  o: {+x: Array<number>},
  src: Array<number>,
): void {
  // Rest binding is a write of the array type itself.
  [...o.x] = src;
}

// Declaration destructuring reads the named source fields, so a
// writeonly source field can't be picked.
function letDestrWriteonly(o: {-y: number}): void {
  let {y} = o;
}

// The rest binding of a declaration destructuring reads every
// remaining source field; writeonly fields can't be carried into the
// rest.
function letRestDestrWriteonly(o: {-y: number, x: number}): void {
  let {x, ...rest} = o;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}variance-destructure-error.js:18:10: error: ft: cannot assign to readonly property x
// CHECK-NEXT:  ({a: o.x} = src);
// CHECK-NEXT:         ^
// CHECK-NEXT:{{.*}}variance-destructure-error.js:28:6: error: ft: cannot assign to readonly property x
// CHECK-NEXT:  [o.x] = src;
// CHECK-NEXT:     ^
// CHECK-NEXT:{{.*}}variance-destructure-error.js:41:14: error: ft: cannot assign to readonly property x
// CHECK-NEXT:  ({a: {b: o.x}} = src);
// CHECK-NEXT:             ^
// CHECK-NEXT:{{.*}}variance-destructure-error.js:49:10: error: ft: cannot assign to readonly property x
// CHECK-NEXT:  ({a: o.x = 1} = src);
// CHECK-NEXT:         ^
// CHECK-NEXT:{{.*}}variance-destructure-error.js:58:16: error: ft: cannot read writeonly property y
// CHECK-NEXT:  ({a: o.y = o.y} = src);
// CHECK-NEXT:               ^
// CHECK-NEXT:{{.*}}variance-destructure-error.js:66:9: error: ft: cannot assign to readonly property x
// CHECK-NEXT:  [...o.x] = src;
// CHECK-NEXT:        ^
// CHECK-NEXT:{{.*}}variance-destructure-error.js:72:8: error: ft: cannot read writeonly property y
// CHECK-NEXT:  let {y} = o;
// CHECK-NEXT:       ^
// CHECK-NEXT:{{.*}}variance-destructure-error.js:79:11: error: ft: cannot read writeonly property y
// CHECK-NEXT:  let {x, ...rest} = o;
// CHECK-NEXT:          ^~~~~~~
// CHECK-NEXT:Emitted 8 errors. exiting.
