/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-transformed-ast -pretty-json %s | %FileCheck --match-full-lines %s

// Transform [a, ...[b]] into [a, b]

//CHECK:        {
//CHECK-NEXT:     "type": "Program",
//CHECK-NEXT:     "body": [

[a, b, ...[c, ...d]] = [];
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
//CHECK-NEXT:                 "type": "Identifier",
//CHECK-NEXT:                 "name": "b"
//CHECK-NEXT:               },
//CHECK-NEXT:               {
//CHECK-NEXT:                 "type": "RestElement",
//CHECK-NEXT:                 "argument": {
//CHECK-NEXT:                   "type": "ArrayPattern",
//CHECK-NEXT:                   "elements": [
//CHECK-NEXT:                     {
//CHECK-NEXT:                       "type": "Identifier",
//CHECK-NEXT:                       "name": "c"
//CHECK-NEXT:                     },
//CHECK-NEXT:                     {
//CHECK-NEXT:                       "type": "RestElement",
//CHECK-NEXT:                       "argument": {
//CHECK-NEXT:                         "type": "Identifier",
//CHECK-NEXT:                         "name": "d"
//CHECK-NEXT:                       }
//CHECK-NEXT:                     }
//CHECK-NEXT:                   ]
//CHECK-NEXT:                 }
//CHECK-NEXT:               }
//CHECK-NEXT:             ]
//CHECK-NEXT:           },
//CHECK-NEXT:           "right": {
//CHECK-NEXT:             "type": "ArrayExpression",
//CHECK-NEXT:             "elements": [],
//CHECK-NEXT:             "trailingComma": false
//CHECK-NEXT:           }
//CHECK-NEXT:         },
//CHECK-NEXT:         "directive": null
//CHECK-NEXT:       }

//CHECK-NEXT:     ]
//CHECK-NEXT:   }
