/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

({
  foo(this: string) {}
})
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ObjectExpression",
// CHECK-NEXT:         "properties": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Property",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "this",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TypeAnnotation",
// CHECK-NEXT:                     "typeAnnotation": {
// CHECK-NEXT:                       "type": "StringTypeAnnotation"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "init",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "method": true,
// CHECK-NEXT:             "shorthand": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

function foo(this: string) {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "this",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     },

function foo(this: string, a: number) {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "this",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "a",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
