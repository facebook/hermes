/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

async function foo() {
  await 1;
}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExpressionStatement",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "AwaitExpression",
// CHECK-NEXT:               "argument": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 1,
// CHECK-NEXT:                 "raw": "1"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "directive": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     },

(async function() {
  await 1;
});

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "FunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ExpressionStatement",
// CHECK-NEXT:               "expression": {
// CHECK-NEXT:                 "type": "AwaitExpression",
// CHECK-NEXT:                 "argument": {
// CHECK-NEXT:                   "type": "NumericLiteral",
// CHECK-NEXT:                   "value": 1,
// CHECK-NEXT:                   "raw": "1"
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "directive": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "generator": false,
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

({
  async foo() { await 1; }
});

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
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ExpressionStatement",
// CHECK-NEXT:                     "expression": {
// CHECK-NEXT:                       "type": "AwaitExpression",
// CHECK-NEXT:                       "argument": {
// CHECK-NEXT:                         "type": "NumericLiteral",
// CHECK-NEXT:                         "value": 1,
// CHECK-NEXT:                         "raw": "1"
// CHECK-NEXT:                       }
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "directive": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": true
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

(async () => { await 1; });
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
// CHECK-NEXT:                 "type": "AwaitExpression",
// CHECK-NEXT:                 "argument": {
// CHECK-NEXT:                   "type": "NumericLiteral",
// CHECK-NEXT:                   "value": 1,
// CHECK-NEXT:                   "raw": "1"
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "directive": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": false,
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
