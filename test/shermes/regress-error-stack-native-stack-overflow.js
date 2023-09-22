/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s  | %FileCheck --match-full-lines %s

"use strict";

function test(description, createErr) {
    print(description);
    print(createErr().stack);
}

test("overriding toString", () => {
    var err = new Error("overriding toString");
    err.toString = function() { return this.stack; };
    return err;
});
// CHECK-LABEL: overriding toString
// CHECK-NEXT: Error: overriding toString

test("overriding name getter", () => {
    var err = new Error("overriding name getter");
    Object.defineProperty(err, "name", {get: function() { return this.stack; }});
    return err;
});
// CHECK-LABEL: overriding name getter
// CHECK-NEXT: <while converting error to string: RangeError: Maximum call stack size exceeded (native stack depth)>

test("overriding message getter", () => {
    var err = new Error("overriding message getter");
    Object.defineProperty(err, "message", {get: function() { return this.stack; }});
    return err;
});
// CHECK-LABEL: overriding message getter
// CHECK-NEXT: Error: Error: {{.*}}: <while converting error to string: RangeError: Maximum call stack size exceeded (native stack depth)>

test("Object.toString => this.stack", () => {
    var o = { toString: function() { return this.stack; } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.name => this.stack", () => {
    var o = { get name() { return this.stack; } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.message => this.stack", () => {
    var o = { get message() { return this.stack; } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.toString throws", () => {
    var o = { toString: function() { throw new Error("toString throws"); } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.name throws", () => {
    var o = { get name() { throw new Error("name getter throws"); } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.message throws", () => {
    var o = { get message() { throw new Error("message getter throws"); } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.__proto__.toString => this.stack", () => {
    var p = { toString: function() { return this.stack; } };
    var o = { __proto__: p };
    Error.captureStackTrace(o);
    return o;
});

test("Object.__proto__.name => this.stack", () => {
    var p = { get name() { return this.stack; } };
    var o = { __proto__: p };
    Error.captureStackTrace(o);
    return o;
});

test("Object.__proto__.message => this.stack", () => {
    var p = { get name() { return this.stack; } };
    var o = { __proto__: p };
    Error.captureStackTrace(o);
    return o;
});

test("Object.name getter throws err; err.name is string", () => {
    var o = { get name() { throw  {name: "123" }; } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.name getter throws err; err.name is not string", () => {
    var o = { get name() { throw { name: 123 }; } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.name getter throws err; err.message is string", () => {
    var o = { get name() { throw  {message: "123" }; } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.name getter throws err; err.message is not string", () => {
    var o = { get name() { throw { message: 123 }; } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.message getter throws err; err.message is string", () => {
    var o = { get message() { throw  {message: "123" }; } };
    Error.captureStackTrace(o);
    return o;
});

test("Object.message getter throws err; err.name is string, err.message is not string", () => {
    var o = { get message() { throw { name: "object name", message: 123 }; } };
    Error.captureStackTrace(o);
    return o;
});
