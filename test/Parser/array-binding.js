/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s

//CHECK:        {
//CHECK-NEXT:     "type": "Program",
//CHECK-NEXT:     "body": [

var [a, [b = 1, c] = [,2]] = x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "VariableDeclaration",
//CHECK-NEXT:         "kind": "var",
//CHECK-NEXT:         "declarations": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "VariableDeclarator",
//CHECK-NEXT:             "init": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "x"
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "ArrayPattern",
//CHECK-NEXT:               "elements": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Identifier",
//CHECK-NEXT:                   "name": "a"
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "AssignmentPattern",
//CHECK-NEXT:                   "left": {
//CHECK-NEXT:                     "type": "ArrayPattern",
//CHECK-NEXT:                     "elements": [
//CHECK-NEXT:                       {
//CHECK-NEXT:                         "type": "AssignmentPattern",
//CHECK-NEXT:                         "left": {
//CHECK-NEXT:                           "type": "Identifier",
//CHECK-NEXT:                           "name": "b"
//CHECK-NEXT:                         },
//CHECK-NEXT:                         "right": {
//CHECK-NEXT:                           "type": "NumericLiteral",
//CHECK-NEXT:                           "value": 1,
//CHECK-NEXT:                           "raw": "1"
//CHECK-NEXT:                         }
//CHECK-NEXT:                       },
//CHECK-NEXT:                       {
//CHECK-NEXT:                         "type": "Identifier",
//CHECK-NEXT:                         "name": "c"
//CHECK-NEXT:                       }
//CHECK-NEXT:                     ]
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "right": {
//CHECK-NEXT:                     "type": "ArrayExpression",
//CHECK-NEXT:                     "elements": [
//CHECK-NEXT:                       {
//CHECK-NEXT:                         "type": "Empty"
//CHECK-NEXT:                       },
//CHECK-NEXT:                       {
//CHECK-NEXT:                         "type": "NumericLiteral",
//CHECK-NEXT:                         "value": 2,
//CHECK-NEXT:                         "raw": "2"
//CHECK-NEXT:                       }
//CHECK-NEXT:                     ],
//CHECK-NEXT:                     "trailingComma": false
//CHECK-NEXT:                   }
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ]
//CHECK-NEXT:       },

[a, [b = 1, c] = [,2]] = x;
//CHECK-NEXT:       {
//CHECK-NEXT:         "type": "ExpressionStatement",
//CHECK-NEXT:         "expression": {
//CHECK-NEXT:           "type": "AssignmentExpression",
//CHECK-NEXT:           "operator": "=",
//CHECK-NEXT:           "left": {
//CHECK-NEXT:             "type": "ArrayPattern",
//CHECK-NEXT:             "elements": [
//CHECK-NEXT:               {
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "a"
//CHECK-NEXT:               },
//CHECK-NEXT:               {
//CHECK-NEXT:                 "type": "AssignmentPattern",
//CHECK-NEXT:                 "left": {
//CHECK-NEXT:                   "type": "ArrayPattern",
//CHECK-NEXT:                   "elements": [
//CHECK-NEXT:                     {
//CHECK-NEXT:                       "type": "AssignmentPattern",
//CHECK-NEXT:                       "left": {
//CHECK-NEXT:                         "type": "Identifier",
//CHECK-NEXT:                         "name": "b"
//CHECK-NEXT:                       },
//CHECK-NEXT:                       "right": {
//CHECK-NEXT:                         "type": "NumericLiteral",
//CHECK-NEXT:                         "value": 1,
//CHECK-NEXT:                         "raw": "1"
//CHECK-NEXT:                       }
//CHECK-NEXT:                     },
//CHECK-NEXT:                     {
//CHECK-NEXT:                       "type": "Identifier",
//CHECK-NEXT:                       "name": "c"
//CHECK-NEXT:                     }
//CHECK-NEXT:                   ]
//CHECK-NEXT:                 },
//CHECK-NEXT:                 "right": {
//CHECK-NEXT:                   "type": "ArrayExpression",
//CHECK-NEXT:                   "elements": [
//CHECK-NEXT:                     {
//CHECK-NEXT:                       "type": "Empty"
//CHECK-NEXT:                     },
//CHECK-NEXT:                     {
//CHECK-NEXT:                       "type": "NumericLiteral",
//CHECK-NEXT:                       "value": 2,
//CHECK-NEXT:                       "raw": "2"
//CHECK-NEXT:                     }
//CHECK-NEXT:                   ],
//CHECK-NEXT:                   "trailingComma": false
//CHECK-NEXT:                 }
//CHECK-NEXT:               }
//CHECK-NEXT:             ]
//CHECK-NEXT:           },
//CHECK-NEXT:           "right": {
//CHECK-NEXT:             "type": "Identifier",
//CHECK-NEXT:             "name": "x"
//CHECK-NEXT:           }
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       }

//CHECK-NEXT:     ]
//CHECK-NEXT:   }
