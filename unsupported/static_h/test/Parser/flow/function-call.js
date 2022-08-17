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

foo<number>(3);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "CallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeArguments": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 3,
// CHECK-NEXT:             "raw": "3"
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

async<number>(3);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "CallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "async"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeArguments": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 3,
// CHECK-NEXT:             "raw": "3"
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

foo().bar<{prop?: T}>();
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "CallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "MemberExpression",
// CHECK-NEXT:           "object": {
// CHECK-NEXT:             "type": "CallExpression",
// CHECK-NEXT:             "callee": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "arguments": []
// CHECK-NEXT:           },
// CHECK-NEXT:           "property": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "bar"
// CHECK-NEXT:           },
// CHECK-NEXT:           "computed": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeArguments": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ObjectTypeAnnotation",
// CHECK-NEXT:               "properties": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "ObjectTypeProperty",
// CHECK-NEXT:                   "key": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "prop"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "value": {
// CHECK-NEXT:                     "type": "GenericTypeAnnotation",
// CHECK-NEXT:                     "id": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "T"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "typeParameters": null
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "method": false,
// CHECK-NEXT:                   "optional": true,
// CHECK-NEXT:                   "static": false,
// CHECK-NEXT:                   "proto": false,
// CHECK-NEXT:                   "variance": null,
// CHECK-NEXT:                   "kind": "init"
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "indexers": [],
// CHECK-NEXT:               "callProperties": [],
// CHECK-NEXT:               "internalSlots": [],
// CHECK-NEXT:               "inexact": false,
// CHECK-NEXT:               "exact": false
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
