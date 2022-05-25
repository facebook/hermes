/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno --dialect=flow-detect --gen-ast %s | %FileCheck %s --match-full-lines

// @flow

foo<T>(1);

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "CallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo",
// CHECK-NEXT:           "typeAnnotation": null,
// CHECK-NEXT:           "optional": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeArguments": {
// CHECK-NEXT:           "type": "TypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "T",
// CHECK-NEXT:                 "typeAnnotation": null,
// CHECK-NEXT:                 "optional": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 1
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
