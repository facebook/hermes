/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

// Regression test: in `for (using x of y)` and `for [await] (await using x of
// y)`, the binding must be wrapped in a `VariableDeclarator` (with `init:
// null`) inside `VariableDeclaration._declarations`, matching what
// `for (var x of y)` produces. Pushing a bare Identifier corrupts later
// stages that `cast<VariableDeclaratorNode>` over the list
// (https://github.com/facebook/hermes/issues/1981).

for (using x of y);
// CHECK:           "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "using",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": null,
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]

async function f() {
  for await (await using x of y);
}
// CHECK:             "type": "VariableDeclaration",
// CHECK-NEXT:               "kind": "await using",
// CHECK-NEXT:               "declarations": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "VariableDeclarator",
// CHECK-NEXT:                   "init": null,
// CHECK-NEXT:                   "id": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "x"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               ]
