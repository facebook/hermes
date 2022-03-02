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

// Define a proxy that will prevent new properties from being added by
// always throwing in the defineProperty trap. This causes
// Error.captureStackTrace to fail to add the [[CapturedError]] internal
// property, but should not cause crashes in the VM.
var cantDefineProperties = new Proxy({ count: 0 }, {
    defineProperty(target) {
        target.count += 1;
        throw "no props";
    }
});

// Define a proxy that will not allow any properties to be read until
// stack is defined in the target object. This caused T113008543.
var cantReadUntilHasStackObj = { count: 0 };
var cantReadUntilHasStack = new Proxy(cantReadUntilHasStackObj, {
    get(target) {
        if (!("stack" in target)) {
            throw "can't get anything until stack is set";
        }
        return Reflect.get(...arguments);
    },

    defineProperty(target, property) {
        target.count += 1;
        return Reflect.defineProperty(...arguments);
    }
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

print("can't define properties");
// CHECK-LABEL: can't define properties
try { Error.captureStackTrace(cantDefineProperties) } catch (e) {
    print(typeof e);
// CHECK-NEXT: string
    print(e);
// CHECK-NEXT: no props
    print(cantDefineProperties.stack)
// CHECK-NEXT: undefined
    print(cantDefineProperties.count)
// CHECK-NEXT: 1
}

print("can't read until object has stack");
// CHECK-LABEL: can't read until object has stack
print(Error.captureStackTrace(cantReadUntilHasStack))
// CHECK-NEXT: undefined
print(cantReadUntilHasStackObj.stack);
// CHECK-NEXT: [object Object]
// CHECK-NEXT:    at global ({{.*}}/error-capture-stack-trace-define-property-stack-fails.js{{.*}})
print(cantReadUntilHasStackObj.count);
// CHECK-NEXT: 2
