/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s


({a, ...foo(10), b, ...d})
//CHECK:      {
//CHECK-NEXT:     "type": "Program",
//CHECK-NEXT:     "body": [
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "ObjectExpression",
//CHECK-NEXT:           "properties": [
//CHECK-NEXT:             {
//CHECK-NEXT:               "type": "Property",
//CHECK-NEXT:               "key": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "a"
//CHECK-NEXT:               },
//CHECK-NEXT:               "value": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "a"
//CHECK-NEXT:               },
//CHECK-NEXT:               "kind": "init",
//CHECK-NEXT:               "computed": false,
//CHECK-NEXT:               "method": false,
//CHECK-NEXT:               "shorthand": true
//CHECK-NEXT:             },
//CHECK-NEXT:             {
//CHECK-NEXT:               "type": "SpreadElement",
//CHECK-NEXT:               "argument": {
//CHECK-NEXT:                 "type": "CallExpression",
//CHECK-NEXT:                 "callee": {
//CHECK-NEXT:                   "type": "Identifier",
//CHECK-NEXT:                   "name": "foo"
//CHECK-NEXT:                 },
//CHECK-NEXT:                 "arguments": [
//CHECK-NEXT:                   {
//CHECK-NEXT:                     "type": "NumericLiteral",
//CHECK-NEXT:                     "value": 10,
//CHECK-NEXT:                     "raw": "10"
//CHECK-NEXT:                   }
//CHECK-NEXT:                 ]
//CHECK-NEXT:               }
//CHECK-NEXT:             },
//CHECK-NEXT:             {
//CHECK-NEXT:               "type": "Property",
//CHECK-NEXT:               "key": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "b"
//CHECK-NEXT:               },
//CHECK-NEXT:               "value": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "b"
//CHECK-NEXT:               },
//CHECK-NEXT:               "kind": "init",
//CHECK-NEXT:               "computed": false,
//CHECK-NEXT:               "method": false,
//CHECK-NEXT:               "shorthand": true
//CHECK-NEXT:             },
//CHECK-NEXT:             {
//CHECK-NEXT:               "type": "SpreadElement",
//CHECK-NEXT:               "argument": {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "d"
//CHECK-NEXT:               }
//CHECK-NEXT:             }
//CHECK-NEXT:           ]
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       }
//CHECK-NEXT:     ]
//CHECK-NEXT:   }
