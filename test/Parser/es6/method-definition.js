// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

({
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ObjectExpression",
// CHECK-NEXT:         "properties": [

  foo(x,y) {return 3},
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Property",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo",
// CHECK-NEXT:               "typeAnnotation": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x",
// CHECK-NEXT:                   "typeAnnotation": null
// CHECK-NEXT:                 },
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "y",
// CHECK-NEXT:                   "typeAnnotation": null
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ReturnStatement",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "NumericLiteral",
// CHECK-NEXT:                       "value": 3
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "init",
// CHECK-NEXT:             "computed": false
// CHECK-NEXT:           },

  ['bar']() {},
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Property",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "StringLiteral",
// CHECK-NEXT:               "value": "bar"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "init",
// CHECK-NEXT:             "computed": true
// CHECK-NEXT:           },

  'baz'() {},
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Property",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "StringLiteral",
// CHECK-NEXT:               "value": "baz"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "init",
// CHECK-NEXT:             "computed": false
// CHECK-NEXT:           },

  123() {},
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Property",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 123
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": []
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "init",
// CHECK-NEXT:             "computed": false
// CHECK-NEXT:           },

  *gen() { yield* 3 },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Property",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "gen",
// CHECK-NEXT:               "typeAnnotation": null
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ExpressionStatement",
// CHECK-NEXT:                     "expression": {
// CHECK-NEXT:                       "type": "YieldExpression",
// CHECK-NEXT:                       "argument": {
// CHECK-NEXT:                         "type": "NumericLiteral",
// CHECK-NEXT:                         "value": 3
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "delegate": true
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "directive": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": true
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "init",
// CHECK-NEXT:             "computed": false
// CHECK-NEXT:           }

});
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
