/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -fno-inline %s | %FileCheck --match-full-lines %s
"use strict";

function c(arg) {
  var error = {};
  Error.captureStackTrace(error, arg);
  return error.stack;
}

function b(arg) {
  var f = globalThis.f = function f() {
    return c(arg);
  };
  return f();
}

function a(arg) {
  return b(arg);
}

// Does not skip frames by default
print(a());
//CHECK: [object Object]
//CHECK-NEXT:  at c ({{.*/error-capture-stack-trace.js}}:13:26)
//CHECK-NEXT:  at f ({{.*/error-capture-stack-trace.js}}:19:13)
//CHECK-NEXT:  at b ({{.*/error-capture-stack-trace.js}}:21:11)
//CHECK-NEXT:  at a ({{.*/error-capture-stack-trace.js}}:25:11)
//CHECK-NEXT:  at global ({{.*/error-capture-stack-trace.js}}:29:8)

// Skips the top frame (`c`)
print(a(c));
//CHECK-NEXT: [object Object]
//CHECK-NEXT:  at f ({{.*/error-capture-stack-trace.js}}:19:13)
//CHECK-NEXT:  at b ({{.*/error-capture-stack-trace.js}}:21:11)
//CHECK-NEXT:  at a ({{.*/error-capture-stack-trace.js}}:25:11)
//CHECK-NEXT:  at global ({{.*/error-capture-stack-trace.js}}:38:8)

// Skips everything down to `b`
print(a(b));
//CHECK-NEXT: [object Object]
//CHECK-NEXT:  at a ({{.*/error-capture-stack-trace.js}}:25:11)
//CHECK-NEXT:  at global ({{.*/error-capture-stack-trace.js}}:46:8)

// Skips everything down to `a`
print(a(a));
//CHECK-NEXT: [object Object]
//CHECK-NEXT:  at global ({{.*/error-capture-stack-trace.js}}:52:8)

// Drills through a bound sentinel function to reach the underlying JSFunction
var bound_a = a.bind(null);
print(bound_a(bound_a));
//CHECK-NEXT: [object Object]
//CHECK-NEXT:  at global ({{.*}})

// Skips the entire stack if the sentinel is a function that's not found
print(a(() => {}));
//CHECK: [object Object]
//CHECK-EMPTY

// Calls toString() on the target object
var err = {toString() { return 'Foo'; }};
Error.captureStackTrace(err);
print(err.stack);
//CHECK-NEXT: Foo
//CHECK-NEXT:  at global ({{.*}})

// Calling with no args
function captureOnInvalid() {
  try {
    print(Error.captureStackTrace(...arguments));
  } catch(e) {
    print(e)
    return;
  }
  throw new Error("succeeded, but should have failed on " + arguments[0]);
}

captureOnInvalid()
//CHECK: TypeError: Invalid argument

captureOnInvalid(1)
//CHECK: TypeError: Invalid argument

captureOnInvalid("string")
//CHECK: TypeError: Invalid argument

captureOnInvalid(undefined)
//CHECK: TypeError: Invalid argument

captureOnInvalid(true)
//CHECK: TypeError: Invalid argument

// Formats the stack trace lazily and caches the result
var description = 'Foo';
var toStringCalled = false;
var err = {toString() { toStringCalled = true; return description; }};
Error.captureStackTrace(err);
print(toStringCalled);
//CHECK: false
print(err.stack);
//CHECK: Foo
description = 'Bar';
print(err.stack);
//CHECK: Foo

// Overrides a never-called stack getter on an Error instance
var err = (function makeErr() { return new Error(); })();
Error.captureStackTrace(err);
print(err.stack);
//CHECK: Error
//CHECK-NEXT:  at global ({{.*}})

// Overrides an already-called stack getter on an Error instance
var err = (function makeErr() { return new Error(); })();
print(err.stack);
//CHECK: Error
//CHECK-NEXT:  at makeErr ({{.*}})
Error.captureStackTrace(err);
print(err.stack);
//CHECK: Error
//CHECK-NEXT:  at global ({{.*}})

// Throws if the target object is frozen
try {
  Error.captureStackTrace(Object.freeze({}));
} catch (e) {
  print(e);
}
//CHECK: TypeError: Cannot add new properties to object

// Does not add a user-visible symbol to the target object
var target = {};
Error.captureStackTrace(target);
print(Object.getOwnPropertySymbols(target).length);
//CHECK: 0

// Captures a new stack with each call.
// Does not cache values of target.toString() between stacks
// Also, calls target.toString() whenever a new stack needs to be rendered.
var description = 'First uncached getter call';
var target = {toString() { return description; }};
(function firstCapture() { Error.captureStackTrace(target); })();
(function secondCapture() { Error.captureStackTrace(target); })();
print(target.stack);
//CHECK: First uncached getter call
//CHECK-NEXT:  at secondCapture ({{.*}})
description = 'Second uncached getter call';
(function thirdCapture() { Error.captureStackTrace(target); })();
print(target.stack);
//CHECK: Second uncached getter call
//CHECK-NEXT:  at thirdCapture ({{.*}})

// The stack getter throws if its receiver has no captured stack.
var dummy = {};
Error.captureStackTrace(dummy);
const stackGetter = Object.getOwnPropertyDescriptor(dummy, 'stack').get;
try { stackGetter.apply({});   } catch (e) { print(e); }
//CHECK: TypeError: Error.stack getter called with an invalid receiver
try { stackGetter.apply(null); } catch (e) { print(e); }
//CHECK: TypeError: Error.stack getter called with an invalid receiver
