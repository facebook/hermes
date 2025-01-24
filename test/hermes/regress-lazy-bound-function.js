/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// We previously implemented BoundFunction as a lazy object if the target was
// lazy. This resulted in incorrect behaviour if the target was updated after
// the BoundFunction was created.

function a(){}
// Looking up `a.bind` would make `a` non-lazy, so this should always work.
var boundA = a.bind();
Object.defineProperty(a, "length", {value: 5});
print(boundA.length);
// CHECK: 0

function b(arg1, arg2){}
// `b` remains lazy here because we didn't look up anything on it.
// Previously, this would have created a lazy `boundB`.
var boundB = Function.prototype.bind.call(b, undefined, 1);
// Modify the length property, making `b` non-lazy.
Object.defineProperty(b, "length", {value: 5});
// Check that `boundB` preserves the original length minus the bound arg.
print(boundB.length);
// CHECK-NEXT: 1

function c(arg1, arg2, arg3){}
// Modify the length property, making it a getter.
Object.defineProperty(c, "length", {get: function(){print("c.length called"); return 3;}});
// Check that binding the function calls the getter.
var boundC = c.bind(undefined, 1);
// CHECK-NEXT: c.length called

// Check that the new length is the returned length minus the bound arg.
print(boundC.length);
// CHECK-NEXT: 2

function d(arg1, arg2, arg3){}
// As with `b` above, `d` remains lazy here.
var boundD = Function.prototype.bind.call(d, undefined, 1);
// Modify the length property, making it a throwing getter.
Object.defineProperty(d, "length", {get: function(){throw new Error("d.length called"); }});
// Check that the getter is not called.
print(boundD.length);
// CHECK-NEXT: 2
