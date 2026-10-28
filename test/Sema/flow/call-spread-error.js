/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

function f(a: number, b: string): void {}
function one(a: number): void {}

// An array's length is not known at compile time, so the call's arity would
// be unknown.
function testArray(a: number[]): void {
  f(...a);
}

function testNotSpreadable(x: any, o: {p: number}): void {
  f(...x);
  f(...o);
}

// Arity is checked after the tuple is expanded.
function testTooFew(t: [number]): void {
  f(...t);
}
function testTooMany(t: [number, string]): void {
  one(...t);
}

function testElementType(t: [number, number]): void {
  f(...t);
}

// A tuple element is loaded directly, so there is no node to cast.
function testCheckedCast(t: [any]): void {
  one(...t);
}

// 'this' is consumed positionally.
function testThisPosition(t: [number]): void {
  one.call(...t);
  $SHBuiltin.call(one, ...t);
}

// An untyped receiver skips argument typechecking, but the spread must still
// be expandable, because IRGen has no dynamic fallback for these two forms.
function untyped(a, b) {}
function testUntypedReceiver(a: number[]): void {
  $SHBuiltin.call(untyped, undefined, ...a);
  untyped.call(undefined, ...a);
}

class Foo {
  @Hermes.final @Hermes.overload
  bar(x: number): void {}
  @Hermes.final @Hermes.overload
  bar(x: string): void {}

  // Reported once, not once per overload candidate.
  testOverload(a: number[]): void {
    this.bar(...a);
  }
}

// Generic type arguments are inferred from the expanded tuple elements.
function id<T>(a: T): T {
  return a;
}
function testGenericInference(t: [string]): number {
  return id(...t);
}

// Once one spread needs the dynamic path, a tuple spread would have to be
// iterated, and tuples are not iterable.
function testMixedSpreads(t: [number], a: number[], u: any): void {
  u(...t, ...a);
  new u(...a, ...t);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}call-spread-error.js:16:5: error: ft: cannot spread an array into a call, because its length is not known at compile time; use a tuple
// CHECK-NEXT:  f(...a);
// CHECK-NEXT:    ^~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:20:5: error: ft: spread argument must be a tuple
// CHECK-NEXT:  f(...x);
// CHECK-NEXT:    ^~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:21:5: error: ft: spread argument must be a tuple
// CHECK-NEXT:  f(...o);
// CHECK-NEXT:    ^~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:26:3: error: ft: function expects 2 arguments, but 1 supplied
// CHECK-NEXT:  f(...t);
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:29:3: error: ft: function expects at most 1 arguments, but 2 supplied
// CHECK-NEXT:  one(...t);
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:33:5: error: ft: function parameter 'b' type mismatch: cannot assign number to string
// CHECK-NEXT:  f(...t);
// CHECK-NEXT:    ^~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:38:7: error: ft: spread element with checked cast not supported
// CHECK-NEXT:  one(...t);
// CHECK-NEXT:      ^~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:43:12: error: ft: spread element not allowed in this argument position
// CHECK-NEXT:  one.call(...t);
// CHECK-NEXT:           ^~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:44:24: error: ft: spread element not allowed in this argument position
// CHECK-NEXT:  $SHBuiltin.call(one, ...t);
// CHECK-NEXT:                       ^~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:51:39: error: ft: cannot spread an array into a call, because its length is not known at compile time; use a tuple
// CHECK-NEXT:  $SHBuiltin.call(untyped, undefined, ...a);
// CHECK-NEXT:                                      ^~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:52:27: error: ft: cannot spread an array into a call, because its length is not known at compile time; use a tuple
// CHECK-NEXT:  untyped.call(undefined, ...a);
// CHECK-NEXT:                          ^~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:63:5: error: ft: no matching overload for call
// CHECK-NEXT:    this.bar(...a);
// CHECK-NEXT:    ^~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:72:3: error: ft: return value incompatible with return type: cannot return string as number
// CHECK-NEXT:  return id(...t);
// CHECK-NEXT:  ^~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:78:5: error: ft: cannot spread a tuple alongside a spread that is not a tuple
// CHECK-NEXT:  u(...t, ...a);
// CHECK-NEXT:    ^~~~
// CHECK-NEXT:{{.*}}call-spread-error.js:79:15: error: ft: cannot spread a tuple alongside a spread that is not a tuple
// CHECK-NEXT:  new u(...a, ...t);
// CHECK-NEXT:              ^~~~
// CHECK-NEXT:Emitted 15 errors. exiting.
