/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -parse-flow --pretty-json %s | %FileCheck %s --match-full-lines

// CHECK: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

a?.<string>();
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalCallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "a"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeArguments": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": [],
// CHECK-NEXT:         "optional": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a?.b<string>();
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalCallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "OptionalMemberExpression",
// CHECK-NEXT:           "object": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "a"
// CHECK-NEXT:           },
// CHECK-NEXT:           "property": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "b"
// CHECK-NEXT:           },
// CHECK-NEXT:           "computed": false,
// CHECK-NEXT:           "optional": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeArguments": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": [],
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a?.b<string>()?.c<string>();
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "OptionalCallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "OptionalMemberExpression",
// CHECK-NEXT:           "object": {
// CHECK-NEXT:             "type": "OptionalCallExpression",
// CHECK-NEXT:             "callee": {
// CHECK-NEXT:               "type": "OptionalMemberExpression",
// CHECK-NEXT:               "object": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "a"
// CHECK-NEXT:               },
// CHECK-NEXT:               "property": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "b"
// CHECK-NEXT:               },
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "optional": true
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeArguments": {
// CHECK-NEXT:               "type": "TypeParameterInstantiation",
// CHECK-NEXT:               "params": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "StringTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               ]
// CHECK-NEXT:             },
// CHECK-NEXT:             "arguments": [],
// CHECK-NEXT:             "optional": false
// CHECK-NEXT:           },
// CHECK-NEXT:           "property": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "c"
// CHECK-NEXT:           },
// CHECK-NEXT:           "computed": false,
// CHECK-NEXT:           "optional": true
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeArguments": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": [],
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
