/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -emit-binary -out /dev/null -O0 %s 2>&1) | %FileCheck --match-full-lines %s
// RUN: (! %hermesc -emit-binary -out /dev/null -O %s 2>&1) | %FileCheck --match-full-lines %s

// Regression test: parsing `for (using x of y)` and `for [await] (await using
// x of y)` used to push a bare IdentifierNode into VariableDeclaration's
// `_declarations`. Sema helpers (`ScopedFunctionPromoter::extractDeclaredIdents`,
// `SemanticResolver::extractIdentsFromDecl`) then aborted via `Casting.h`
// `cast<VariableDeclaratorNode>` before reaching the explicit `using`
// rejection, masking the user-facing error and corrupting memory in release
// WASM (https://github.com/facebook/hermes/issues/1981).
//
// With the parser fix, the cast succeeds and Sema reaches the explicit
// rejection cleanly.

function for_of_using(arr) {
  for (using x of arr) {}
}
// CHECK: {{.*}}for-using-not-supported.js:[[@LINE-2]]:8: error: using declarations are not yet supported

function for_in_using(obj) {
  for (using x in obj) {}
}
// CHECK: {{.*}}for-using-not-supported.js:[[@LINE-2]]:8: error: using declarations are not yet supported

async function for_await_of_await_using(arr) {
  for await (await using x of arr) {}
}
// CHECK: {{.*}}for-using-not-supported.js:[[@LINE-2]]:14: error: using declarations are not yet supported
