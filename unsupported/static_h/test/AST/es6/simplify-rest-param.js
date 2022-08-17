/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-transformed-ast -pretty-json %s | %FileCheck --match-full-lines %s

// Transform (a, ...[b]) into (a, b)

//CHECK:        {
//CHECK-NEXT:     "type": "Program",
//CHECK-NEXT:     "body": [

function foo(a, ...[b, c]) {}
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
//CHECK-NEXT:             "type": "RestElement",
//CHECK-NEXT:             "argument": {
//CHECK-NEXT:               "type": "ArrayPattern",
//CHECK-NEXT:               "elements": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Identifier",
//CHECK-NEXT:                   "name": "b"
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Identifier",
//CHECK-NEXT:                   "name": "c"
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
