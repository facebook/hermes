/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

(function main(x, y, z) {

// Every function here should error due to invalid implicit return undefined.

function f1(): number {
  if (x) { return 1; }
}

function f2(): number {
  if (x) { return 1; }
  else {}
}

function f3(): number {
  try { x(); return 1; }
  catch {}
}

function f4(): number {
  try { x(); }
  catch { return 1; }
}

function f5(): number {
  while (x) { }
}

function f6(): number {
  for (var i of x) { }
}

function f7(): number {
  switch(x) {
    case 1:
      return 1;
    default:
  }
}

function f8(): number {
  try { throw 1; }
  catch { }
  finally { }
}

function f9(): number {
  switch (x) {
    case 1:
      if (y) { break; }
      else {}
    default:
      return 2;
  }
}

function f10(): number {
  switch (x) {
    case 1:
      if (y) break;
      else return 2;
    default:
      return 2;
  }
}

function f11(): number {
  try {
    try {
      return 1;
    } finally {
      throw 2;
    }
  } catch {}
}

function f12(): number {
  label:
    try {
      return 1;
    } finally {
      break label;
    }
}

function f13(): number {
  try {
    {
      {
        throw 1;
      }
      return 2;
    }
    return 3;
  } catch {
  }
}

function f14(): number {
  outer: {
    inner: {
      if (x) break outer;
      break inner;
    }
    return 1;
  }
}

function f15(): number {
  try {
    try {
      return 1;
    } finally {}
  } catch {}
}

})();

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}implicit-return-error.js:14:14: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f1(): number {
// CHECK-NEXT:             ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:18:14: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f2(): number {
// CHECK-NEXT:             ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:23:14: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f3(): number {
// CHECK-NEXT:             ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:28:14: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f4(): number {
// CHECK-NEXT:             ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:33:14: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f5(): number {
// CHECK-NEXT:             ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:37:14: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f6(): number {
// CHECK-NEXT:             ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:41:14: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f7(): number {
// CHECK-NEXT:             ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:49:14: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f8(): number {
// CHECK-NEXT:             ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:55:14: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f9(): number {
// CHECK-NEXT:             ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:65:15: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f10(): number {
// CHECK-NEXT:              ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:75:15: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f11(): number {
// CHECK-NEXT:              ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:85:15: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f12(): number {
// CHECK-NEXT:              ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:94:15: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f13(): number {
// CHECK-NEXT:              ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:107:15: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f14(): number {
// CHECK-NEXT:              ^~~~~~~~
// CHECK-NEXT:{{.*}}implicit-return-error.js:117:15: error: ft: implicitly-returned 'undefined' incompatible with return type
// CHECK-NEXT:function f15(): number {
// CHECK-NEXT:              ^~~~~~~~
// CHECK-NEXT:Emitted 15 errors. exiting.
