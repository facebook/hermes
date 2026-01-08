/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

for (using x of y);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ForOfStatement",
// CHECK-NEXT:       "left": {
// CHECK-NEXT:         "type": "VariableDeclaration",
// CHECK-NEXT:         "kind": "using",
// CHECK-NEXT:         "declarations": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "y"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EmptyStatement"
// CHECK-NEXT:       },
// CHECK-NEXT:       "await": false
// CHECK-NEXT:     },

for (using x in y);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ForInStatement",
// CHECK-NEXT:       "left": {
// CHECK-NEXT:         "type": "VariableDeclaration",
// CHECK-NEXT:         "kind": "using",
// CHECK-NEXT:         "declarations": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "y"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EmptyStatement"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

for (using of x);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ForOfStatement",
// CHECK-NEXT:       "left": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "using"
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "x"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "EmptyStatement"
// CHECK-NEXT:       },
// CHECK-NEXT:       "await": false
// CHECK-NEXT:     },

async function f() {
  for await (await using x of y);
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "f"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ForOfStatement",
// CHECK-NEXT:             "left": {
// CHECK-NEXT:               "type": "VariableDeclaration",
// CHECK-NEXT:               "kind": "await using",
// CHECK-NEXT:               "declarations": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x"
// CHECK-NEXT:                 }
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "right": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "y"
// CHECK-NEXT:             },
// CHECK-NEXT:             "body": {
// CHECK-NEXT:               "type": "EmptyStatement"
// CHECK-NEXT:             },
// CHECK-NEXT:             "await": true
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
