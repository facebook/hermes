/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-jsx -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

(<T>() => 3);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 3,
// CHECK-NEXT:           "raw": "3"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterDeclaration",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TypeParameter",
// CHECK-NEXT:               "name": "T",
// CHECK-NEXT:               "bound": null,
// CHECK-NEXT:               "variance": null,
// CHECK-NEXT:               "default": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(async<T>(): number => {1});
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ExpressionStatement",
// CHECK-NEXT:               "expression": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 1,
// CHECK-NEXT:                 "raw": "1"
// CHECK-NEXT:               },
// CHECK-NEXT:               "directive": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterDeclaration",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TypeParameter",
// CHECK-NEXT:               "name": "T",
// CHECK-NEXT:               "bound": null,
// CHECK-NEXT:               "variance": null,
// CHECK-NEXT:               "default": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": false,
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(<T>() => 3</T>);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "JSXElement",
// CHECK-NEXT:         "openingElement": {
// CHECK-NEXT:           "type": "JSXOpeningElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "T"
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": [],
// CHECK-NEXT:           "selfClosing": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "children": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "JSXText",
// CHECK-NEXT:             "value": "() => 3",
// CHECK-NEXT:             "raw": "() => 3"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "closingElement": {
// CHECK-NEXT:           "type": "JSXClosingElement",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "JSXIdentifier",
// CHECK-NEXT:             "name": "T"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(async (): A => B => C => { });
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "ArrowFunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "B"
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "ArrowFunctionExpression",
// CHECK-NEXT:             "id": null,
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "C"
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "body": {
// CHECK-NEXT:               "type": "BlockStatement",
// CHECK-NEXT:               "body": []
// CHECK-NEXT:             },
// CHECK-NEXT:             "expression": false,
// CHECK-NEXT:             "async": false
// CHECK-NEXT:           },
// CHECK-NEXT:           "expression": true,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "A"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
