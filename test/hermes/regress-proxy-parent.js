/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// We previously set the parent of a proxy to Object.prototype, which is
// incorrect and may be observed by the prototype traversal in the error stack
// getter. Test this by installing stack trace information on Object.prototype
// and verifying that the stack trace getter does not find it.

// Install stack trace information on Object.prototype.
Error.captureStackTrace(Object.prototype);

// Check that lookup on a regular proxy returns undefined.
var p =  new Proxy(new Error(), {});
print(p.stack);
// CHECK: undefined

// Check that lookup on a callable proxy fails.
function foo(){}
foo.__proto__ = new Error();
var cp = new Proxy(foo, {});
print(cp.stack);
// CHECK-NEXT: undefined
