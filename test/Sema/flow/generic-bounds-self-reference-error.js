/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Type parameters are bound left-to-right, so a bound that references its own
// parameter or a later parameter resolves to no visible type. Resolution
// happens at instantiation, so these are only checked once the generics are
// used. They must produce a clean "undefined type" error rather than crashing
// or silently resolving to some outer/any type.

class Base {}

// Self-reference: the bound names the parameter it belongs to.
function selfRef<T: T>(x: T): void {}
selfRef<Base>(new Base());

// Forward reference: the bound names a later parameter.
function fwdRef<T: U, U>(x: T, y: U): void {}
fwdRef<Base, Base>(new Base(), new Base());

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-bounds-self-reference-error.js:19:21: error: ft: undefined type T
// CHECK-NEXT:function selfRef<T: T>(x: T): void {}
// CHECK-NEXT:                    ^
// CHECK-NEXT:{{.*}}generic-bounds-self-reference-error.js:23:20: error: ft: undefined type U
// CHECK-NEXT:function fwdRef<T: U, U>(x: T, y: U): void {}
// CHECK-NEXT:                   ^
// CHECK-NEXT:Emitted 2 errors. exiting.
