/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-ts -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

namespace Foo {
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSModuleMember",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "initializer": {
// CHECK-NEXT:         "type": "TSModuleBlock",
// CHECK-NEXT:         "body": [

let x = 3;
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "VariableDeclaration",
// CHECK-NEXT:             "kind": "let",
// CHECK-NEXT:             "declarations": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "VariableDeclarator",
// CHECK-NEXT:                 "init": {
// CHECK-NEXT:                   "type": "NumericLiteral",
// CHECK-NEXT:                   "value": 3,
// CHECK-NEXT:                   "raw": "3"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "id": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },

export function bar() {}
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExportNamedDeclaration",
// CHECK-NEXT:             "declaration": {
// CHECK-NEXT:               "type": "FunctionDeclaration",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "bar"
// CHECK-NEXT:               },
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "specifiers": [],
// CHECK-NEXT:             "source": null,
// CHECK-NEXT:             "exportKind": "value"
// CHECK-NEXT:           }

}
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
