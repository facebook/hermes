/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc --dump-ast --pretty-json %s | %FileCheck --match-full-lines %s

function foo(a, [b = 1, c] = [], {c:d = 1, e:{f}}, ...[g,h = 1]) {}
//CHECK:      {
//CHECK-NEXT:     "type": "Program",
//CHECK-NEXT:     "body": [
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "FunctionDeclaration",
//CHECK-NEXT:         "id": {
//CHECK-NEXT:           "type": "Identifier",
//CHECK-NEXT:           "name": "foo"
//CHECK-NEXT:         },
//CHECK-NEXT:         "params": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "Identifier",
//CHECK-NEXT:             "name": "a"
//CHECK-NEXT:           },
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "AssignmentPattern",
//CHECK-NEXT:             "left": {
//CHECK-NEXT:               "type": "ArrayPattern",
//CHECK-NEXT:               "elements": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "AssignmentPattern",
//CHECK-NEXT:                   "left": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "b"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "right": {
//CHECK-NEXT:                     "type": "NumericLiteral",
//CHECK-NEXT:                     "value": 1,
//CHECK-NEXT:                     "raw": "1"
//CHECK-NEXT:                   }
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Identifier",
//CHECK-NEXT:                   "name": "c"
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             },
//CHECK-NEXT:             "right": {
//CHECK-NEXT:               "type": "ArrayExpression",
//CHECK-NEXT:               "elements": [],
//CHECK-NEXT:               "trailingComma": false
//CHECK-NEXT:             }
//CHECK-NEXT:           },
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "ObjectPattern",
//CHECK-NEXT:             "properties": [
//CHECK-NEXT:               {
//CHECK-NEXT:                 "type": "Property",
//CHECK-NEXT:                 "key": {
//CHECK-NEXT:                   "type": "Identifier",
//CHECK-NEXT:                   "name": "c"
//CHECK-NEXT:                 },
//CHECK-NEXT:                 "value": {
//CHECK-NEXT:                   "type": "AssignmentPattern",
//CHECK-NEXT:                   "left": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "d"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "right": {
//CHECK-NEXT:                     "type": "NumericLiteral",
//CHECK-NEXT:                     "value": 1,
//CHECK-NEXT:                     "raw": "1"
//CHECK-NEXT:                   }
//CHECK-NEXT:                 },
//CHECK-NEXT:                 "kind": "init",
//CHECK-NEXT:                 "computed": false,
//CHECK-NEXT:                 "method": false,
//CHECK-NEXT:                 "shorthand": false
//CHECK-NEXT:               },
//CHECK-NEXT:               {
//CHECK-NEXT:                 "type": "Property",
//CHECK-NEXT:                 "key": {
//CHECK-NEXT:                   "type": "Identifier",
//CHECK-NEXT:                   "name": "e"
//CHECK-NEXT:                 },
//CHECK-NEXT:                 "value": {
//CHECK-NEXT:                   "type": "ObjectPattern",
//CHECK-NEXT:                   "properties": [
//CHECK-NEXT:                     {
//CHECK-NEXT:                       "type": "Property",
//CHECK-NEXT:                       "key": {
//CHECK-NEXT:                         "type": "Identifier",
//CHECK-NEXT:                         "name": "f"
//CHECK-NEXT:                       },
//CHECK-NEXT:                       "value": {
//CHECK-NEXT:                         "type": "Identifier",
//CHECK-NEXT:                         "name": "f"
//CHECK-NEXT:                       },
//CHECK-NEXT:                       "kind": "init",
//CHECK-NEXT:                       "computed": false,
//CHECK-NEXT:                       "method": false,
//CHECK-NEXT:                       "shorthand": true
//CHECK-NEXT:                     }
//CHECK-NEXT:                   ]
//CHECK-NEXT:                 },
//CHECK-NEXT:                 "kind": "init",
//CHECK-NEXT:                 "computed": false,
//CHECK-NEXT:                 "method": false,
//CHECK-NEXT:                 "shorthand": false
//CHECK-NEXT:               }
//CHECK-NEXT:             ]
//CHECK-NEXT:           },
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "RestElement",
//CHECK-NEXT:             "argument": {
//CHECK-NEXT:               "type": "ArrayPattern",
//CHECK-NEXT:               "elements": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Identifier",
//CHECK-NEXT:                   "name": "g"
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "AssignmentPattern",
//CHECK-NEXT:                   "left": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "h"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "right": {
//CHECK-NEXT:                     "type": "NumericLiteral",
//CHECK-NEXT:                     "value": 1,
//CHECK-NEXT:                     "raw": "1"
//CHECK-NEXT:                   }
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ],
//CHECK-NEXT:         "body": {
//CHECK-NEXT:           "type": "BlockStatement",
//CHECK-NEXT:           "body": []
//CHECK-NEXT:         },
//CHECK-NEXT:         "generator": false,
//CHECK-NEXT:         "async": false
//CHECK-NEXT:       }
//CHECK-NEXT:     ]
//CHECK-NEXT:   }
