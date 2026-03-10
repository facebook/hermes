/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc --transform-ts --dump-transformed-ast -pretty-json %s | %FileCheck %s --match-full-lines

// Type aliases and interfaces are removed from the body.

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

type MyString = string;
interface Greeter { greet(): void; }

// Only the variable declaration remains.
let x = 10;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "let",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 10,
// CHECK-NEXT:             "raw": "10"
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
