/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -dump-ast -pretty-json %s | %FileCheckOrRegen %s --match-full-lines

declare module Foo {
  import Bar from "baz";
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "DeclareModule",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Foo"
// CHECK-NEXT:      },
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "BlockStatement",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ImportDeclaration",
// CHECK-NEXT:            "specifiers": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "ImportDefaultSpecifier",
// CHECK-NEXT:                "local": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "Bar"
// CHECK-NEXT:                }
// CHECK-NEXT:              }
// CHECK-NEXT:            ],
// CHECK-NEXT:            "source": {
// CHECK-NEXT:              "type": "StringLiteral",
// CHECK-NEXT:              "value": "baz"
// CHECK-NEXT:            },
// CHECK-NEXT:            "assertions": [],
// CHECK-NEXT:            "importKind": "value"
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
