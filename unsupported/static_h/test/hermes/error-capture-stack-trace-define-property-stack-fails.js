/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s
"use strict";

// Define an object with a read-only property stack. This should cause a
// failure when prepareStackTrace tries to define stack which should not
// be bubbled to the caller.
var readOnly = {};
Object.defineProperty(readOnly, "stack", {
    value: "not a stack",
    writable: false
});


// Define an object with an always throwing setter for property stack.
// This should cause a failure when prepareStackTrace tries to define
// stack which should not be bubbled to the caller.
var throwingSet = {};
Object.defineProperty(throwingSet, "stack", {
    get() { return "not a stack"; },
    set() { throw "nope"; }
});

print("read-only");
// CHECK-LABEL: read-only
print(Error.captureStackTrace(readOnly));
// CHECK-NEXT: undefined
print(readOnly.stack)
// CHECK-NEXT: not a stack

print("always throws");
// CHECK-LABEL: always throws
print(Error.captureStackTrace(throwingSet));
// CHECK-NEXT: undefined
print(throwingSet.stack)
// CHECK-NEXT: not a stack

print("proxy target");
// CHECK-LABEL: proxy target
try { Error.captureStackTrace(new Proxy({}, {})); } catch(e) { print(e); }
// CHECK-LABEL: TypeError: Invalid argument
