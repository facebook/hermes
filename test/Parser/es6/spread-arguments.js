// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: (! %hermesc -dump-ast -pretty-json %s 2>/dev/null ) | %FileCheck %s --match-full-lines

// CHECK: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

f(...x, ...y);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "CallExpression",
// CHECK-NEXT:         "callee": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "f",
// CHECK-NEXT:           "typeAnnotation": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "arguments": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "SpreadElement",
// CHECK-NEXT:             "argument": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x",
// CHECK-NEXT:               "typeAnnotation": null
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "SpreadElement",
// CHECK-NEXT:             "argument": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "y",
// CHECK-NEXT:               "typeAnnotation": null
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
