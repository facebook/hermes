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

({
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ObjectExpression",
// CHECK-NEXT:         "properties": [

  get foo(): number {},
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Property",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "TSTypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSNumberKeyword"
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "get",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "shorthand": false
// CHECK-NEXT:           },

  set foo(v: number) {},
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
// CHECK-NEXT:                   "name": "v",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSTypeAnnotation",
// CHECK-NEXT:                     "typeAnnotation": {
// CHECK-NEXT:                       "type": "TSNumberKeyword"
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
// CHECK-NEXT:             "kind": "set",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "method": false,
// CHECK-NEXT:             "shorthand": false
// CHECK-NEXT:           },

  get<T>() {},
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Property",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "get"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": {
// CHECK-NEXT:                 "type": "TSTypeParameterDeclaration",
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "TSTypeParameter",
// CHECK-NEXT:                     "name": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "T"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "constraint": null,
// CHECK-NEXT:                     "default": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "init",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "method": true,
// CHECK-NEXT:             "shorthand": false
// CHECK-NEXT:           },

  set<T>() {},
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Property",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "set"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": {
// CHECK-NEXT:                 "type": "TSTypeParameterDeclaration",
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "TSTypeParameter",
// CHECK-NEXT:                     "name": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "T"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "constraint": null,
// CHECK-NEXT:                     "default": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "init",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "method": true,
// CHECK-NEXT:             "shorthand": false
// CHECK-NEXT:           }

});
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
