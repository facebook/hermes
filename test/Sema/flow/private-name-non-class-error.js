/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen %s --match-full-lines

class A {
  #x: number = 1;
  tuple(t: [number]): any { return t.#x; }
  str(t: string): any { return t.#x; }
  strLit(t: 'lit'): any { return t.#x; }
  num(t: number): any { return t.#x; }
  numLit(t: 5): any { return t.#x; }
  obj(t: {a: number}): any { return t.#x; }
  objUnion(t: {a: number} | {a: string}): any { return t.#x; }
  classUnion(t: A | B): any { return t.#x; }
  arrWrite(t: number[]): void { t.#x = 1; }
  anyRecv(t: any): any { return t.#x; }
  optional(t: ?A): any { return t?.#x; }
}

class B {
  #x: number = 1;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}private-name-non-class-error.js:12:38: error: ft: private name access only allowed on class instances, found [number]
// CHECK-NEXT:  tuple(t: [number]): any { return t.#x; }
// CHECK-NEXT:                                     ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:13:34: error: ft: private name access only allowed on class instances, found string
// CHECK-NEXT:  str(t: string): any { return t.#x; }
// CHECK-NEXT:                                 ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:14:36: error: ft: private name access only allowed on class instances, found "lit"
// CHECK-NEXT:  strLit(t: 'lit'): any { return t.#x; }
// CHECK-NEXT:                                   ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:15:34: error: ft: private name access only allowed on class instances, found number
// CHECK-NEXT:  num(t: number): any { return t.#x; }
// CHECK-NEXT:                                 ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:16:32: error: ft: private name access only allowed on class instances, found 5
// CHECK-NEXT:  numLit(t: 5): any { return t.#x; }
// CHECK-NEXT:                               ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:17:39: error: ft: private name access only allowed on class instances, found object
// CHECK-NEXT:  obj(t: {a: number}): any { return t.#x; }
// CHECK-NEXT:                                      ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:18:58: error: ft: private name access only allowed on class instances, found (object | object)
// CHECK-NEXT:  objUnion(t: {a: number} | {a: string}): any { return t.#x; }
// CHECK-NEXT:                                                         ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:19:40: error: ft: private name access only allowed on class instances, found (class A | class B)
// CHECK-NEXT:  classUnion(t: A | B): any { return t.#x; }
// CHECK-NEXT:                                       ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:20:35: error: ft: private field #x not visible outside class Array
// CHECK-NEXT:  arrWrite(t: number[]): void { t.#x = 1; }
// CHECK-NEXT:                                  ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:20:35: error: ft: property #x not defined in class Array
// CHECK-NEXT:  arrWrite(t: number[]): void { t.#x = 1; }
// CHECK-NEXT:                                  ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:21:35: error: ft: private name access only allowed on class instances, found any
// CHECK-NEXT:  anyRecv(t: any): any { return t.#x; }
// CHECK-NEXT:                                  ^~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:22:33: error: ft: optional member expression not implemented
// CHECK-NEXT:  optional(t: ?A): any { return t?.#x; }
// CHECK-NEXT:                                ^~~~~
// CHECK-NEXT:{{.*}}private-name-non-class-error.js:22:36: error: ft: private name access not supported in optional member expressions
// CHECK-NEXT:  optional(t: ?A): any { return t?.#x; }
// CHECK-NEXT:                                   ^~
// CHECK-NEXT:Emitted 13 errors. exiting.
